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

  Sound Blaster I/O routines, common for SB8, SBPro and SB16
  Written by Andrew Zabolotny <bit@eltech.ru>

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DRV_SB

#include <stdlib.h>
#include <dpmi.h>
#include <go32.h>
#include <dos.h>
#include <sys/nearptr.h>
#include <sys/farptr.h>
#include <string.h>

#include "dossb.h"

/********************************************* Private variables/routines *****/

__sb_state sb;

/* Wait for SoundBlaster for some time */
#if !defined(__GNUC__) || (__GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ == 0)
# define _func_noinline volatile /* match original code */
# define _func_noclone
#else
/* avoid warnings from newer gcc:
 * "function definition has qualified void return type" and
 * function return types not compatible due to 'volatile' */
# define _func_noinline __attribute__((__noinline__))
# if (__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 5)
#  define _func_noclone
# else
#  define _func_noclone __attribute__((__noclone__))
# endif
#endif
_func_noinline
_func_noclone
 void __sb_wait()
{
	inportb(SB_DSP_RESET);
	inportb(SB_DSP_RESET);
	inportb(SB_DSP_RESET);
	inportb(SB_DSP_RESET);
	inportb(SB_DSP_RESET);
	inportb(SB_DSP_RESET);
}

static void sb_irq()
{
	/* Make sure its not a spurious IRQ */
	if (!irq_check(sb.irq_handle))
		return;

	sb.irqcount++;

	/* Acknowledge DMA transfer is complete */
	if (sb.mode & SBMODE_16BITS)
		__sb_dsp_ack_dma16();
	else
		__sb_dsp_ack_dma8();

	/* SoundBlaster 1.x cannot do autoinit ... */
	if (sb.dspver < SBVER_20)
		__sb_dspreg_outwlh(SBDSP_DMA_PCM8, (sb.dma_buff->size >> 1) - 1);

	/* Send EOI */
	irq_ack(sb.irq_handle);

	enable();
	if (sb.timer_callback)
		sb.timer_callback();
}

static void sb_irq_end()
{
}

static boolean __sb_reset()
{
	/* Disable the output */
	sb_output(FALSE);

	/* Clear pending ints if any */
	__sb_dsp_ack_dma8();
	__sb_dsp_ack_dma16();

	/* Reset the DSP */
	outportb(SB_DSP_RESET, SBM_DSP_RESET);
	__sb_wait();
	__sb_wait();
	outportb(SB_DSP_RESET, 0);

	/* Now wait for AA coming from datain port */
	if (__sb_dsp_in() != 0xaa)
		return FALSE;

	/* Finally, get the DSP version */
	if ((sb.dspver = __sb_dsp_version()) == 0xffff)
		return FALSE;
	/* Check again */
	if (sb.dspver != __sb_dsp_version())
		return FALSE;

	return TRUE;
}

/***************************************************** SB detection stuff *****/

static int __sb_irq_irqdetect(int irqno)
{
	__sb_dsp_ack_dma8();
	return 1;
}

static void __sb_irq_dmadetect()
{
	/* Make sure its not a spurious IRQ */
	if (!irq_check(sb.irq_handle))
		return;

	sb.irqcount++;

	/* Acknowledge DMA transfer is complete */
	if (sb.mode & SBMODE_16BITS)
		__sb_dsp_ack_dma16();
	else
		__sb_dsp_ack_dma8();

	/* Send EOI */
	irq_ack(sb.irq_handle);
}

static boolean __sb_detect()
{
	/* First find the port number */
	if (!sb.port) {
		int i;
		for (i = 5; i >= 0; i--) {
			sb.port = 0x210 + i * 0x10;
			if (__sb_reset())
				break;
		}
		if (i < 0) {
			sb.port = 0;
			return FALSE;
		}
	}

	/* Now detect the IRQ and DMA numbers */
	if (!sb.irq) {
		unsigned int irqmask, sbirqmask, sbirqcount;
		unsigned long timer;

		/* IRQ can be one of 2,3,5,7,10 */
		irq_detect_start(0x04ac, __sb_irq_irqdetect);

		/* Prepare timeout counter */
		_farsetsel(_dos_ds);
		timer = _farnspeekl(0x46c);

		sbirqmask = 0;
		sbirqcount = 10;		/* Emit 10 SB irqs */

		/* Tell SoundBlaster to emit IRQ for 8-bit transfers */
		__sb_dsp_out(SBDSP_GEN_IRQ8);
		__sb_wait();
		for (;;) {
			irq_detect_get(0, &irqmask);
			if (irqmask) {
				sbirqmask |= irqmask;
				if (!--sbirqcount)
					break;
				__sb_dsp_out(SBDSP_GEN_IRQ8);
			}
			if (_farnspeekl(0x46c) - timer >= 9)	/* Wait ~1/2 secs */
				break;
		}
		if (sbirqmask)
			for (sb.irq = 15; sb.irq > 0; sb.irq--)
				if (irq_detect_get(sb.irq, &irqmask) == 10)
					break;

		irq_detect_end();
		if (!sb.irq)
			return FALSE;
	}

	/* Detect the 8-bit and 16-bit DMAs */
	if (!sb.dma8 || ((sb.dspver >= SBVER_16) && !sb.dma16)) {
		static int __dma8[] = { 0, 1, 3 };
		static int __dma16[] = { 5, 6, 7 };
		int *dma;

		sb_output(FALSE);
		/* Temporary hook SB IRQ */
		sb.irq_handle = irq_hook(sb.irq, __sb_irq_dmadetect, 200);
		irq_enable(sb.irq_handle);
		if (sb.irq > 7)
			_irq_enable(2);

		/* Start a short DMA transfer and check if IRQ happened */
		for (;;) {
			int i;
			unsigned int timer, oldcount;

			if (!sb.dma8)
				dma = &sb.dma8;
			else if ((sb.dspver >= SBVER_16) && !sb.dma16)
				dma = &sb.dma16;
			else
				break;

			for (i = 0; i < 3; i++) {
				boolean success = 1;

				*dma = (dma == &sb.dma8) ? __dma8[i] : __dma16[i];
				oldcount = sb.irqcount;

				dma_disable(*dma);
				dma_set_mode(*dma, DMA_MODE_WRITE);
				dma_clear_ff(*dma);
				dma_set_count(*dma, 2);
				dma_enable(*dma);

				__sb_dspreg_out(SBDSP_SET_TIMING, 206);	/* 20KHz */
				if (dma == &sb.dma8) {
					sb.mode = 0;
					__sb_dspreg_outwlh(SBDSP_DMA_PCM8, 1);
				} else {
					sb.mode = SBMODE_16BITS;
					__sb_dspreg_out(SBDSP_DMA_GENERIC16, 0);
					__sb_dsp_out(0);
					__sb_dsp_out(1);
				}

				_farsetsel(_dos_ds);
				timer = _farnspeekl(0x46c);

				while (oldcount == sb.irqcount)
					if (_farnspeekl(0x46c) - timer >= 2) {
						success = 0;
						break;
					}
				dma_disable(*dma);
				if (success)
					break;
				*dma = 0;
			}
			if (!*dma)
				break;
		}

		irq_unhook(sb.irq_handle);
		sb.irq_handle = NULL;
		if (!sb.dma8 || ((sb.dspver >= SBVER_16) && !sb.dma16))
			return FALSE;
	}
	return TRUE;
}

/*************************************************** High-level interface *****/

/* Detect whenever SoundBlaster is present and fill "sb" structure */
boolean sb_detect()
{
	char *env;

	/* Try to find the port and DMA from environment */
	env = getenv("BLASTER");

	while (env && *env) {
		/* Skip whitespace */
		while ((*env == ' ') || (*env == '\t'))
			env++;
		if (!*env)
			break;

		switch (*env++) {
		  case 'A':
		  case 'a':
			if (!sb.port)
				sb.port = strtol(env, &env, 16);
			break;
		  case 'E':
		  case 'e':
			if (!sb.aweport)
				sb.aweport = strtol(env, &env, 16);
			break;
		  case 'I':
		  case 'i':
			if (!sb.irq)
				sb.irq = strtol(env, &env, 10);
			break;
		  case 'D':
		  case 'd':
			if (!sb.dma8)
				sb.dma8 = strtol(env, &env, 10);
			break;
		  case 'H':
		  case 'h':
			if (!sb.dma16)
				sb.dma16 = strtol(env, &env, 10);
			break;
		  default:
			/* Skip other values (H == MIDI, T == model, any other?) */
			while (*env && (*env != ' ') && (*env != '\t'))
				env++;
			break;
		}
	}

	/* Try to detect missing sound card parameters */
	__sb_detect();

	if (!sb.port || !sb.irq || !sb.dma8)
		return FALSE;

	if (!__sb_reset())
		return FALSE;

	if ((sb.dspver >= SBVER_16) && !sb.dma16)
		return FALSE;

	if (sb.dspver >= SBVER_PRO)
		sb.caps |= SBMODE_STEREO;
	if (sb.dspver >= SBVER_16 && sb.dma16)
		sb.caps |= SBMODE_16BITS;
	if (sb.dspver < SBVER_20)
		sb.maxfreq_mono = 22222;
	else
		sb.maxfreq_mono = 45454;
	if (sb.dspver <= SBVER_16)
		sb.maxfreq_stereo = 22727;
	else
		sb.maxfreq_stereo = 45454;

	sb.ok = 1;
	return TRUE;
}

/* Reset SoundBlaster */
void sb_reset()
{
	sb_stop_dma();
	__sb_reset();
}

/* Start working with SoundBlaster */
boolean sb_open()
{
	__dpmi_meminfo struct_info;

	if (!sb.ok)
		if (!sb_detect())
			return FALSE;

	if (sb.open)
		return FALSE;

	/* Now lock the sb structure in memory */
	struct_info.address = __djgpp_base_address + (unsigned long)&sb;
	struct_info.size = sizeof(sb);
	if (__dpmi_lock_linear_region(&struct_info))
		return FALSE;

	/* Hook the SB IRQ */
	sb.irq_handle = irq_hook(sb.irq, sb_irq, (long)sb_irq_end - (long)sb_irq);
	if (!sb.irq_handle) {
		__dpmi_unlock_linear_region(&struct_info);
		return FALSE;
	}

	/* Enable the interrupt */
	irq_enable(sb.irq_handle);
	if (sb.irq > 7)
		_irq_enable(2);

	sb.open++;

	return TRUE;
}

/* Finish working with SoundBlaster */
boolean sb_close()
{
	__dpmi_meminfo struct_info;
	if (!sb.open)
		return FALSE;

	sb.open--;

	/* Stop/free DMA buffer */
	sb_stop_dma();

	/* Unhook IRQ */
	irq_unhook(sb.irq_handle);
	sb.irq_handle = NULL;

	/* Unlock the sb structure */
	struct_info.address = __djgpp_base_address + (unsigned long)&sb;
	struct_info.size = sizeof(sb);
	__dpmi_unlock_linear_region(&struct_info);

	return TRUE;
}

/* Enable/disable stereo DSP mode */
/* Enable/disable speaker output */
void sb_output(boolean enable)
{
	__sb_dsp_out(enable ? SBDSP_SPEAKER_ENA : SBDSP_SPEAKER_DIS);
}

/* Start playing from DMA buffer */
boolean sb_start_dma(unsigned char mode, unsigned int freq)
{
	int dmachannel = (mode & SBMODE_16BITS) ? sb.dma16 : sb.dma8;
	int dmabuffsize;
	unsigned int tc = 0;		/* timing constant (<=sbpro only) */

	/* Stop DMA transfer if it is enabled */
	sb_stop_dma();

	/* Sanity check */
	if ((mode & SBMODE_MASK & sb.caps) != (mode & SBMODE_MASK))
		return FALSE;

	/* Check this SB can perform at requested frequency */
	if (((mode & SBMODE_STEREO) && (freq > sb.maxfreq_stereo))
		|| (!(mode & SBMODE_STEREO) && (freq > sb.maxfreq_mono)))
		return FALSE;

	/* Check the timing constant here to avoid failing later */
	if (sb.dspver < SBVER_16) {
		/* SBpro cannot do signed transfer */
		if (mode & SBMODE_SIGNED)
			return FALSE;

		/* Old SBs have a different way on setting DMA timing constant */
		tc = freq;
		if (mode & SBMODE_STEREO)
			tc *= 2;
		tc = 1000000 / tc;
		if (tc > 255)
			return FALSE;
	}

	sb.mode = mode;

	/* Get a DMA buffer enough for a 1/4sec interval... 4K <= dmasize <= 32K */
	dmabuffsize = freq;
	if (mode & SBMODE_STEREO)
		dmabuffsize *= 2;
	if (mode & SBMODE_16BITS)
		dmabuffsize *= 2;
	dmabuffsize >>= 2;
	if (dmabuffsize < 4096)
		dmabuffsize = 4096;
	if (dmabuffsize > 32768)
		dmabuffsize = 32768;
	dmabuffsize = (dmabuffsize + 255) & 0xffffff00;

	sb.dma_buff = dma_allocate(dmachannel, dmabuffsize);
	if (!sb.dma_buff)
		return FALSE;

	/* Fill DMA buffer with silence */
	dmabuffsize = sb.dma_buff->size;
	if (mode & SBMODE_SIGNED)
		memset(sb.dma_buff->linear, 0, dmabuffsize);
	else
		memset(sb.dma_buff->linear, 0x80, dmabuffsize);

	/* Prime DMA for transfer */
	dma_start(sb.dma_buff, dmabuffsize, DMA_MODE_WRITE | DMA_MODE_AUTOINIT);

	/* Tell SoundBlaster to start transfer */
	if (sb.dspver >= SBVER_16) {	/* SB16 */
		__sb_dspreg_outwhl(SBDSP_SET_RATE, freq);

		/* Start DMA->DAC transfer */
		__sb_dspreg_out(SBM_GENDAC_AUTOINIT | SBM_GENDAC_FIFO |
						((mode & SBMODE_16BITS) ? SBDSP_DMA_GENERIC16 :
						 SBDSP_DMA_GENERIC8),
						((mode & SBMODE_SIGNED) ? SBM_GENDAC_SIGNED : 0) |
						((mode & SBMODE_STEREO) ? SBM_GENDAC_STEREO : 0));

		/* Write the length of transfer */
		dmabuffsize = (dmabuffsize >> 2) - 1;
		__sb_dsp_out(dmabuffsize);
		__sb_dsp_out(dmabuffsize >> 8);
	} else {
		__sb_dspreg_out(SBDSP_SET_TIMING, 256 - tc);
		dmabuffsize = (dmabuffsize >> 1) - 1;
		if (sb.dspver >= SBVER_20) {	/* SB 2.0/Pro */
			/* Set stereo mode */
			__sb_stereo((mode & SBMODE_STEREO) ? TRUE : FALSE);
			__sb_dspreg_outwlh(SBDSP_SET_DMA_BLOCK, dmabuffsize);
			if (sb.dspver >= SBVER_PRO)
				__sb_dsp_out(SBDSP_HS_DMA_DAC8_AUTO);
			else
				__sb_dsp_out(SBDSP_DMA_PCM8_AUTO);
		} else {				/* Original SB */
			/* Start DMA->DAC transfer */
			__sb_dspreg_outwlh(SBDSP_DMA_PCM8, dmabuffsize);
		}
	}

	return TRUE;
}

/* Stop playing from DMA buffer */
void sb_stop_dma()
{
	if (!sb.dma_buff)
		return;

	if (sb.mode & SBMODE_16BITS)
		__sb_dsp_out(SBDSP_DMA_HALT16);
	else
		__sb_dsp_out(SBDSP_DMA_HALT8);

	dma_disable(sb.dma_buff->channel);
	dma_free(sb.dma_buff);
	sb.dma_buff = NULL;
}

/* Query current position/total size of the DMA buffer */
void sb_query_dma(unsigned int *dma_size, unsigned int *dma_pos)
{
	unsigned int dma_left;
	*dma_size = sb.dma_buff->size;
	/* It can happen we try to read DMA count when HI/LO bytes will be
	   inconsistent */
	for (;;) {
		unsigned int dma_left_test;
		dma_clear_ff(sb.dma_buff->channel);
		dma_left_test = dma_get_count(sb.dma_buff->channel);
		dma_left = dma_get_count(sb.dma_buff->channel);
		if ((dma_left >= dma_left_test) && (dma_left - dma_left_test < 10))
			break;
	}
	*dma_pos = *dma_size - dma_left;
}

#endif /* DRV_SB */

/* ex:set ts=4: */
