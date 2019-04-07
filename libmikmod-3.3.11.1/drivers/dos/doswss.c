/*	MikMod sound library
	(c) 1998, 1999 Miodrag Vallat and others - see file AUTHORS for
	complete list.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.
*/

/*==============================================================================

  Windows Sound System I/O routines (CS42XX, ESS18XX, GUS+DaughterBoard etc)
  Written by Andrew Zabolotny <bit@eltech.ru>

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DRV_WSS

#include <stdlib.h>
#include <dpmi.h>
#include <go32.h>
#include <dos.h>
#include <sys/nearptr.h>
#include <sys/farptr.h>
#include <string.h>

#include "doswss.h"

/********************************************* Private variables/routines *****/

__wss_state wss;

/* WSS frequency rates... lower bit selects one of two frequency generators */
static unsigned int wss_rates[14][2] = {
	{5510, 0x00 | WSSM_XTAL2},
	{6620, 0x0E | WSSM_XTAL2},
	{8000, 0x00 | WSSM_XTAL1},
	{9600, 0x0E | WSSM_XTAL1},
	{11025, 0x02 | WSSM_XTAL2},
	{16000, 0x02 | WSSM_XTAL1},
	{18900, 0x04 | WSSM_XTAL2},
	{22050, 0x06 | WSSM_XTAL2},
	{27420, 0x04 | WSSM_XTAL1},
	{32000, 0x06 | WSSM_XTAL1},
	{33075, 0x0C | WSSM_XTAL2},
	{37800, 0x08 | WSSM_XTAL2},
	{44100, 0x0A | WSSM_XTAL2},
	{48000, 0x0C | WSSM_XTAL1}
};

static void wss_irq()
{
	/* Make sure its not a spurious IRQ */
	if (!irq_check(wss.irq_handle))
		return;

	wss.irqcount++;

	/* Clear IRQ status */
	outportb(WSS_STATUS, 0);

	/* Write transfer count again */
	__wss_outreg(WSSR_COUNT_LOW, wss.samples & 0xff);
	__wss_outreg(WSSR_COUNT_HIGH, wss.samples >> 8);
	irq_ack(wss.irq_handle);

	enable();
	if (wss.timer_callback)
		wss.timer_callback();
}

static void wss_irq_end()
{
}

/* WSS accepts some conventional values instead of frequency in Hz... */
static unsigned char __wss_getrate(unsigned int *freq)
{
	int i, best = -1, delta = 0xffff;

	for (i = 0; i < 14; i++) {
		int newdelta = abs(wss_rates[i][0] - *freq);
		if (newdelta < delta)
			best = i, delta = newdelta;
	}

	*freq = wss_rates[best][0];
	return wss_rates[best][1];
}

/* Check if we really have a WSS compatible card on given address */
static boolean __wss_ping()
{
	/* Disable CODEC operations first */
	__wss_regbit_reset(WSSR_IFACE_CTRL, WSSM_PLAYBACK_ENABLE);
	/* Now put some harmless values in registers and check them */
	__wss_outreg(WSSR_COUNT_LOW, 0xaa);
	__wss_outreg(WSSR_COUNT_HIGH, 0x55);
	return (__wss_inreg(WSSR_COUNT_LOW) == 0xaa)
	  && (__wss_inreg(WSSR_COUNT_HIGH) == 0x55);
}

static boolean __wss_reset()
{
	int count;

	/* Disable output */
	wss_output(FALSE);

	/* Now select the test/initialization register */
	count = 10000;
	while (inportb(WSS_ADDR) != WSSR_TEST_INIT) {
		outportb(WSS_ADDR, WSSR_TEST_INIT);
		if (!--count)
			return FALSE;
	}

	count = 10000;
	while (inportb(WSS_DATA) & WSSM_CALIB_IN_PROGRESS) {
		outportb(WSS_ADDR, WSSR_TEST_INIT);
		if (!--count)
			return FALSE;
	}

	/* Enable playback IRQ */
	__wss_regbit_set(WSSR_PIN_CTRL, WSSM_IRQ_ENABLE);
	__wss_outreg(WSSR_IRQ_STATUS, WSSM_PLAYBACK_IRQ);

	/* Clear IRQ status */
	outportb(WSS_STATUS, 0);

	return TRUE;
}

static boolean __wss_setformat(unsigned char format)
{
	int count;

	outportb(WSS_ADDR, WSSM_MCE | WSSR_PLAY_FORMAT);
	outportb(WSS_DATA, format);
	inportb(WSS_DATA);			/* ERRATA SHEETS ... */
	inportb(WSS_DATA);			/* ERRATA SHEETS ... */

	/* Wait end of syncronization ... */
	if (!__wss_wait())
		return FALSE;

	/* Turn off the ModeChangeEnable bit: do it until it works */
	count = 10000;
	while (inportb(WSS_ADDR) != WSSR_PLAY_FORMAT) {
		outportb(WSS_ADDR, WSSR_PLAY_FORMAT);
		if (!--count)
			return FALSE;
	}

	return __wss_reset();
}

/**************************************************** WSS detection stuff *****/

static int __wss_irq_irqdetect(int irqno)
{
	unsigned char status = inportb(WSS_STATUS);
	/* Clear IRQ status */
	outportb(WSS_STATUS, 0);
	/* Reset transfer counter */
	__wss_outreg(WSSR_COUNT_LOW, 0);
	__wss_outreg(WSSR_COUNT_HIGH, 0);
	return (status & WSSM_INT);
}

static boolean __wss_detect()
{
	/* First find the port number */
	if (!wss.port) {
		static unsigned int wss_ports[] =
		  { 0x32c, 0x530, 0x604, 0xE80, 0xF40 };
		int i;
		for (i = 0; i < 5; i++) {
			wss.port = wss_ports[i];
			if (__wss_ping())
				break;
		}
		if (i < 0) {
			wss.port = 0;
			return FALSE;
		}
	}

	/* Now disable output */
	wss_output(FALSE);

	/* Detect the DMA channel */
	if (!wss.dma) {
		static int __dma[] = { 0, 1, 3 };
		int i;

		/* Enable playback IRQ */
		__wss_regbit_set(WSSR_PIN_CTRL, WSSM_IRQ_ENABLE);
		__wss_outreg(WSSR_IRQ_STATUS, WSSM_PLAYBACK_IRQ);

		/* Start a short DMA transfer and check if DMA count is zero */
		for (i = 0; i < 3; i++) {
			unsigned int timer, status, freq = 44100;

			wss.dma = __dma[i];

			dma_disable(wss.dma);
			dma_set_mode(wss.dma, DMA_MODE_WRITE);
			dma_clear_ff(wss.dma);
			dma_set_count(wss.dma, 10);
			dma_enable(wss.dma);

			/* Clear IRQ status */
			outportb(WSS_STATUS, 0);

			__wss_setformat(__wss_getrate(&freq));
			__wss_outreg(WSSR_COUNT_LOW, 1);
			__wss_outreg(WSSR_COUNT_HIGH, 0);
			/* Tell codec to start transfer */
			__wss_regbit_set(WSSR_IFACE_CTRL, WSSM_PLAYBACK_ENABLE);

			_farsetsel(_dos_ds);
			timer = _farnspeekl(0x46c);

			while (_farnspeekl(0x46c) - timer <= 2)
				if (dma_get_count(wss.dma) == 0)
					break;
			__wss_regbit_reset(WSSR_IFACE_CTRL, WSSM_PLAYBACK_ENABLE);
			dma_disable(wss.dma);

			/* Now check if DMA transfer count is zero and an IRQ is pending */
			status = inportb(WSS_STATUS);
			outportb(WSS_STATUS, 0);
			if ((dma_get_count(wss.dma) == 0) && (status & WSSM_INT))
				break;

			wss.dma = 0;
		}

		if (!wss.dma)
			return FALSE;
	}

	/* Now detect the IRQ number */
	if (!wss.irq) {
		unsigned int i, irqmask, freq = 5510;
		unsigned long timer, delta = 0x7fffffff;

		/* IRQ can be one of 2,3,5,7,10 */
		irq_detect_start(0x04ac, __wss_irq_irqdetect);

		dma_disable(wss.dma);
		dma_set_mode(wss.dma, DMA_MODE_WRITE | DMA_MODE_AUTOINIT);
		dma_clear_ff(wss.dma);
		dma_set_count(wss.dma, 1);
		dma_enable(wss.dma);

		__wss_setformat(__wss_getrate(&freq));

		/* Clear IRQ status */
		outportb(WSS_STATUS, 0);

		__wss_outreg(WSSR_COUNT_LOW, 0);
		__wss_outreg(WSSR_COUNT_HIGH, 0);

		/* Prepare timeout counter */
		_farsetsel(_dos_ds);
		timer = _farnspeekl(0x46c);
		while (timer == _farnspeekl(0x46c));
		timer = _farnspeekl(0x46c);

		/* Reset all IRQ counters */
		irq_detect_clear();

		/* Tell codec to start transfer */
		__wss_regbit_set(WSSR_IFACE_CTRL, WSSM_PLAYBACK_ENABLE);

		/* Now wait 1/18 seconds */
		while (timer == _farnspeekl(0x46c));
		__wss_regbit_reset(WSSR_IFACE_CTRL, WSSM_PLAYBACK_ENABLE);
		dma_disable(wss.dma);

		/* Given frequency 5510Hz, a buffer size of 1 byte and a time interval
		   of 1/18.2 second, we should have received about 302 interrupts */
		for (i = 2; i <= 10; i++) {
			int count = abs(302 - irq_detect_get(i, &irqmask));
			if (count < delta)
				wss.irq = i, delta = count;
		}
		if (delta > 150)
			wss.irq = 0;

		irq_detect_end();
		if (!wss.irq)
			return FALSE;
	}

	return TRUE;
}

/*************************************************** High-level interface *****/

/* Detect whenever WSS is present and fill "wss" structure */
boolean wss_detect()
{
	char *env;

	/* Try to find the port and DMA from environment */
	env = getenv("WSS");

	while (env && *env) {
		/* Skip whitespace */
		while ((*env == ' ') || (*env == '\t'))
			env++;
		if (!*env)
			break;

		switch (*env++) {
		  case 'A':
		  case 'a':
			if (!wss.port)
				wss.port = strtol(env, &env, 16);
			break;
		  case 'I':
		  case 'i':
			if (!wss.irq)
				wss.irq = strtol(env, &env, 10);
			break;
		  case 'D':
		  case 'd':
			if (!wss.dma)
				wss.dma = strtol(env, &env, 10);
			break;
		  default:
			/* Skip other values */
			while (*env && (*env != ' ') && (*env != '\t'))
				env++;
			break;
		}
	}

	/* Try to fill the gaps in wss hardware parameters */
	__wss_detect();

	if (!wss.port || !wss.irq || !wss.dma)
		return FALSE;

	if (!__wss_ping())
		return FALSE;

	if (!__wss_reset())
		return FALSE;

	wss.ok = 1;
	return TRUE;
}

/* Reset WSS */
void wss_reset()
{
	wss_stop_dma();
	__wss_reset();
}

/* Open WSS for usage */
boolean wss_open()
{
	__dpmi_meminfo struct_info;

	if (!wss.ok)
		if (!wss_detect())
			return FALSE;

	if (wss.open)
		return FALSE;

	/* Now lock the wss structure in memory */
	struct_info.address = __djgpp_base_address + (unsigned long)&wss;
	struct_info.size = sizeof(wss);
	if (__dpmi_lock_linear_region(&struct_info))
		return FALSE;

	/* Hook the WSS IRQ */
	wss.irq_handle =
	  irq_hook(wss.irq, wss_irq, (long)wss_irq_end - (long)wss_irq);
	if (!wss.irq_handle) {
		__dpmi_unlock_linear_region(&struct_info);
		return FALSE;
	}

	/* Enable the interrupt */
	irq_enable(wss.irq_handle);
	if (wss.irq > 7)
		_irq_enable(2);

	wss.open++;

	return TRUE;
}

/* Finish working with WSS */
boolean wss_close()
{
	__dpmi_meminfo struct_info;
	if (!wss.open)
		return FALSE;

	wss.open--;

	/* Stop/free DMA buffer */
	wss_stop_dma();

	/* Unhook IRQ */
	irq_unhook(wss.irq_handle);
	wss.irq_handle = NULL;

	/* Unlock the wss structure */
	struct_info.address = __djgpp_base_address + (unsigned long)&wss;
	struct_info.size = sizeof(wss);
	__dpmi_unlock_linear_region(&struct_info);

	return TRUE;
}

/* Adjust frequency rate to nearest WSS available */
unsigned int wss_adjust_freq(unsigned int freq)
{
	__wss_getrate(&freq);
	return freq;
}

/* Enable/disable speaker output */
/* Start playing from DMA buffer in either 8/16 bit mono/stereo */
boolean wss_start_dma(unsigned char mode, unsigned int freq)
{
	int dmabuffsize;
	unsigned char format;

	/* Stop DMA transfer if it is enabled */
	wss_stop_dma();

	/* Sanity check: we support only 8-bit unsigned and 16-bit signed formats */
	if (((mode & WSSMODE_16BITS) && !(mode & WSSMODE_SIGNED))
		|| (!(mode & WSSMODE_16BITS) && (mode & WSSMODE_SIGNED)))
		return FALSE;

	/* Find the nearest frequency divisor (rate) */
	format = __wss_getrate(&freq);
	wss.mode = mode;

	/* Get a DMA buffer enough for a 1sec interval... 4K <= dmasize <= 32K */
	dmabuffsize = freq;
	if (mode & WSSMODE_STEREO)
		dmabuffsize *= 2;
	if (mode & WSSMODE_16BITS)
		dmabuffsize *= 2;
	dmabuffsize >>= 2;
	if (dmabuffsize < 4096)
		dmabuffsize = 4096;
	if (dmabuffsize > 32768)
		dmabuffsize = 32768;
	dmabuffsize = (dmabuffsize + 255) & 0xffffff00;

	wss.dma_buff = dma_allocate(wss.dma, dmabuffsize);
	if (!wss.dma_buff)
		return FALSE;

	/* Fill DMA buffer with silence */
	dmabuffsize = wss.dma_buff->size;
	if (mode & WSSMODE_SIGNED)
		memset(wss.dma_buff->linear, 0, dmabuffsize);
	else
		memset(wss.dma_buff->linear, 0x80, dmabuffsize);

	/* Check data size and build a WSSR_PLAY_FORMAT value accordingly */
	wss.samples = dmabuffsize;
	if (mode & WSSMODE_16BITS) {
		wss.samples >>= 1;
		format |= WSSM_16BITS;
	}

	if (mode & WSSMODE_STEREO) {
		wss.samples >>= 1;
		format |= WSSM_STEREO;
	}

	if (!__wss_setformat(format)) {
		wss_stop_dma();
		return FALSE;
	}

	/* Prime DMA for transfer */
	dma_start(wss.dma_buff, dmabuffsize, DMA_MODE_WRITE | DMA_MODE_AUTOINIT);

	/* Tell codec how many samples to transfer */
	wss.samples = (wss.samples >> 1) - 1;
	__wss_outreg(WSSR_COUNT_LOW, wss.samples & 0xff);
	__wss_outreg(WSSR_COUNT_HIGH, wss.samples >> 8);

	/* Tell codec to start transfer */
	__wss_regbit_set(WSSR_IFACE_CTRL, WSSM_PLAYBACK_ENABLE);

	return TRUE;
}

/* Stop playing from DMA buffer */
void wss_stop_dma()
{
	if (!wss.dma_buff)
		return;

	__wss_regbit_reset(WSSR_IFACE_CTRL, WSSM_PLAYBACK_ENABLE);
	dma_disable(wss.dma);
	dma_free(wss.dma_buff);
	wss.dma_buff = NULL;
}

/* Query current position/total size of the DMA buffer */
void wss_query_dma(unsigned int *dma_size, unsigned int *dma_pos)
{
	unsigned int dma_left;
	*dma_size = wss.dma_buff->size;
	/* It can happen we try to read DMA count when HI/LO bytes will be
	   inconsistent */
	for (;;) {
		unsigned int dma_left_test;
		dma_clear_ff(wss.dma);
		dma_left_test = dma_get_count(wss.dma);
		dma_left = dma_get_count(wss.dma);
		if ((dma_left >= dma_left_test) && (dma_left - dma_left_test < 10))
			break;
	}
	*dma_pos = *dma_size - dma_left;
}

void wss_output(boolean enable)
{
	if (enable)
		wss.curlevel = wss.level;
	else
		wss.curlevel = 0x3f;

	__wss_outreg(WSSR_MASTER_L, wss.curlevel);
	__wss_outreg(WSSR_MASTER_R, wss.curlevel);
}

void wss_level(int level)
{
	if (level < 0)
		level = 0;
	if (level > 63)
		level = 63;
	wss.curlevel = wss.level = level ^ 63;

	__wss_outreg(WSSR_MASTER_L, wss.curlevel);
	__wss_outreg(WSSR_MASTER_R, wss.curlevel);
}

#endif /* DRV_WSS */

/* ex:set ts=4: */
