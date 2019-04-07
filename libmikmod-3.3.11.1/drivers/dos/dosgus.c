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

  Driver for GUS cards under DOS
  Written by Andrew Zabolotny <bit@eltech.ru>

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DRV_ULTRA

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/nearptr.h>
#include <go32.h>
#include <string.h>

#include "dosgus.h"
#include "mikmod.h" /* for MikMod_malloc() & co */

/********************************************* Private variables/routines *****/

/* The Gravis Ultrasound state/info */
__gus_state gus;

/* Try to avoid holes in DRAM less than this size */
#define DRAM_HOLE_THRESHOLD	8192
/* If hole is larger than that, create a free block describing it */
#define DRAM_SPLIT_THRESHOLD	64
/* The size of DMA buffer used for RAM->DRAM transfers */
#define GF1_DMA_BUFFER_SIZE	8192

/* Debug macro: useful to change screen locations when some event occurs */
#ifdef MIKMOD_DEBUG
#  define DEBUG_PRINT(x) printf x;
#  define DEBUG_OFS(addr, attr)			\
   {						\
     unsigned short x;				\
     _dosmemgetw (0xb8780 + addr*2, 1, &x);	\
     if ((x >> 8) != attr) x = '0';		\
     x = ((x + 1) & 0xff) | (attr << 8);	\
     _dosmemputw (&x, 1, 0xb8780 + addr*2);	\
   }
#else
#  define DEBUG_PRINT(x)
#  define DEBUG_OFS(addr, attr)
#endif

static unsigned short __gus_volume_table[512] = {
	0x0000, 0x7000, 0x7ff0, 0x8800, 0x8ff0, 0x9400, 0x9800, 0x9c00,
	0x9ff0, 0xa200, 0xa400, 0xa600, 0xa800, 0xaa00, 0xac00, 0xae00,
	0xaff0, 0xb100, 0xb200, 0xb300, 0xb400, 0xb500, 0xb600, 0xb700,
	0xb800, 0xb900, 0xba00, 0xbb00, 0xbc00, 0xbd00, 0xbe00, 0xbf00,
	0xbff0, 0xc080, 0xc100, 0xc180, 0xc200, 0xc280, 0xc300, 0xc380,
	0xc400, 0xc480, 0xc500, 0xc580, 0xc600, 0xc680, 0xc700, 0xc780,
	0xc800, 0xc880, 0xc900, 0xc980, 0xca00, 0xca80, 0xcb00, 0xcb80,
	0xcc00, 0xcc80, 0xcd00, 0xcd80, 0xce00, 0xce80, 0xcf00, 0xcf80,
	0xcff0, 0xd040, 0xd080, 0xd0c0, 0xd100, 0xd140, 0xd180, 0xd1c0,
	0xd200, 0xd240, 0xd280, 0xd2c0, 0xd300, 0xd340, 0xd380, 0xd3c0,
	0xd400, 0xd440, 0xd480, 0xd4c0, 0xd500, 0xd540, 0xd580, 0xd5c0,
	0xd600, 0xd640, 0xd680, 0xd6c0, 0xd700, 0xd740, 0xd780, 0xd7c0,
	0xd800, 0xd840, 0xd880, 0xd8c0, 0xd900, 0xd940, 0xd980, 0xd9c0,
	0xda00, 0xda40, 0xda80, 0xdac0, 0xdb00, 0xdb40, 0xdb80, 0xdbc0,
	0xdc00, 0xdc40, 0xdc80, 0xdcc0, 0xdd00, 0xdd40, 0xdd80, 0xddc0,
	0xde00, 0xde40, 0xde80, 0xdec0, 0xdf00, 0xdf40, 0xdf80, 0xdfc0,
	0xdff0, 0xe020, 0xe040, 0xe060, 0xe080, 0xe0a0, 0xe0c0, 0xe0e0,
	0xe100, 0xe120, 0xe140, 0xe160, 0xe180, 0xe1a0, 0xe1c0, 0xe1e0,
	0xe200, 0xe220, 0xe240, 0xe260, 0xe280, 0xe2a0, 0xe2c0, 0xe2e0,
	0xe300, 0xe320, 0xe340, 0xe360, 0xe380, 0xe3a0, 0xe3c0, 0xe3e0,
	0xe400, 0xe420, 0xe440, 0xe460, 0xe480, 0xe4a0, 0xe4c0, 0xe4e0,
	0xe500, 0xe520, 0xe540, 0xe560, 0xe580, 0xe5a0, 0xe5c0, 0xe5e0,
	0xe600, 0xe620, 0xe640, 0xe660, 0xe680, 0xe6a0, 0xe6c0, 0xe6e0,
	0xe700, 0xe720, 0xe740, 0xe760, 0xe780, 0xe7a0, 0xe7c0, 0xe7e0,
	0xe800, 0xe820, 0xe840, 0xe860, 0xe880, 0xe8a0, 0xe8c0, 0xe8e0,
	0xe900, 0xe920, 0xe940, 0xe960, 0xe980, 0xe9a0, 0xe9c0, 0xe9e0,
	0xea00, 0xea20, 0xea40, 0xea60, 0xea80, 0xeaa0, 0xeac0, 0xeae0,
	0xeb00, 0xeb20, 0xeb40, 0xeb60, 0xeb80, 0xeba0, 0xebc0, 0xebe0,
	0xec00, 0xec20, 0xec40, 0xec60, 0xec80, 0xeca0, 0xecc0, 0xece0,
	0xed00, 0xed20, 0xed40, 0xed60, 0xed80, 0xeda0, 0xedc0, 0xede0,
	0xee00, 0xee20, 0xee40, 0xee60, 0xee80, 0xeea0, 0xeec0, 0xeee0,
	0xef00, 0xef20, 0xef40, 0xef60, 0xef80, 0xefa0, 0xefc0, 0xefe0,
	0xeff0, 0xf010, 0xf020, 0xf030, 0xf040, 0xf050, 0xf060, 0xf070,
	0xf080, 0xf090, 0xf0a0, 0xf0b0, 0xf0c0, 0xf0d0, 0xf0e0, 0xf0f0,
	0xf100, 0xf110, 0xf120, 0xf130, 0xf140, 0xf150, 0xf160, 0xf170,
	0xf180, 0xf190, 0xf1a0, 0xf1b0, 0xf1c0, 0xf1d0, 0xf1e0, 0xf1f0,
	0xf200, 0xf210, 0xf220, 0xf230, 0xf240, 0xf250, 0xf260, 0xf270,
	0xf280, 0xf290, 0xf2a0, 0xf2b0, 0xf2c0, 0xf2d0, 0xf2e0, 0xf2f0,
	0xf300, 0xf310, 0xf320, 0xf330, 0xf340, 0xf350, 0xf360, 0xf370,
	0xf380, 0xf390, 0xf3a0, 0xf3b0, 0xf3c0, 0xf3d0, 0xf3e0, 0xf3f0,
	0xf400, 0xf410, 0xf420, 0xf430, 0xf440, 0xf450, 0xf460, 0xf470,
	0xf480, 0xf490, 0xf4a0, 0xf4b0, 0xf4c0, 0xf4d0, 0xf4e0, 0xf4f0,
	0xf500, 0xf510, 0xf520, 0xf530, 0xf540, 0xf550, 0xf560, 0xf570,
	0xf580, 0xf590, 0xf5a0, 0xf5b0, 0xf5c0, 0xf5d0, 0xf5e0, 0xf5f0,
	0xf600, 0xf610, 0xf620, 0xf630, 0xf640, 0xf650, 0xf660, 0xf670,
	0xf680, 0xf690, 0xf6a0, 0xf6b0, 0xf6c0, 0xf6d0, 0xf6e0, 0xf6f0,
	0xf700, 0xf710, 0xf720, 0xf730, 0xf740, 0xf750, 0xf760, 0xf770,
	0xf780, 0xf790, 0xf7a0, 0xf7b0, 0xf7c0, 0xf7d0, 0xf7e0, 0xf7f0,
	0xf800, 0xf810, 0xf820, 0xf830, 0xf840, 0xf850, 0xf860, 0xf870,
	0xf880, 0xf890, 0xf8a0, 0xf8b0, 0xf8c0, 0xf8d0, 0xf8e0, 0xf8f0,
	0xf900, 0xf910, 0xf920, 0xf930, 0xf940, 0xf950, 0xf960, 0xf970,
	0xf980, 0xf990, 0xf9a0, 0xf9b0, 0xf9c0, 0xf9d0, 0xf9e0, 0xf9f0,
	0xfa00, 0xfa10, 0xfa20, 0xfa30, 0xfa40, 0xfa50, 0xfa60, 0xfa70,
	0xfa80, 0xfa90, 0xfaa0, 0xfab0, 0xfac0, 0xfad0, 0xfae0, 0xfaf0,
	0xfb00, 0xfb10, 0xfb20, 0xfb30, 0xfb40, 0xfb50, 0xfb60, 0xfb70,
	0xfb80, 0xfb90, 0xfba0, 0xfbb0, 0xfbc0, 0xfbd0, 0xfbe0, 0xfbf0,
	0xfc00, 0xfc10, 0xfc20, 0xfc30, 0xfc40, 0xfc50, 0xfc60, 0xfc70,
	0xfc80, 0xfc90, 0xfca0, 0xfcb0, 0xfcc0, 0xfcd0, 0xfce0, 0xfcf0,
	0xfd00, 0xfd10, 0xfd20, 0xfd30, 0xfd40, 0xfd50, 0xfd60, 0xfd70,
	0xfd80, 0xfd90, 0xfda0, 0xfdb0, 0xfdc0, 0xfdd0, 0xfde0, 0xfdf0,
	0xfe00, 0xfe10, 0xfe20, 0xfe30, 0xfe40, 0xfe50, 0xfe60, 0xfe70,
	0xfe80, 0xfe90, 0xfea0, 0xfeb0, 0xfec0, 0xfed0, 0xfee0, 0xfef0,
	0xff00, 0xff10, 0xff20, 0xff30, 0xff40, 0xff50, 0xff60, 0xff70,
	0xff80, 0xff90, 0xffa0, 0xffb0, 0xffc0, 0xffd0, 0xffe0, 0xfff0
};

/* Wait a bit for GUS before doing something
 * Mark function as volatile: don't allow it to be inlined.
 * It *should* be slow, no need to make it work faster :-)
 */
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
 void __gus_delay()
{
	inportb(GF1_MIX_CTRL);
	inportb(GF1_MIX_CTRL);
	inportb(GF1_MIX_CTRL);
	inportb(GF1_MIX_CTRL);
	inportb(GF1_MIX_CTRL);
	inportb(GF1_MIX_CTRL);
	inportb(GF1_MIX_CTRL);
	inportb(GF1_MIX_CTRL);
}

static void __gus_stop_controller(unsigned char gf1reg)
{
	register unsigned char value = __gus_inregb(gf1reg);
	__gus_outregb(gf1reg, (value | GF1VC_STOPPED | GF1VC_STOP) &
                      ~(GF1VC_IRQ_PENDING | GF1VC_IRQ));
}

/* Returns 1 if volume is already at given position */
static boolean __gus_volume_ramp_to(unsigned short volume,
                                    unsigned char rate,
                                    unsigned char vol_ctrl)
{
	int svol = __gus_inregw(GF1R_VOLUME) & 0xfff0;
	int evol = volume;

	/* First of all, disable volume ramp */
	__gus_stop_controller(GF1R_VOLUME_CONTROL);

	/* If voice is stopped, set the volume to zero and return */
	if (__gus_inregb(GF1R_VOICE_CONTROL) & GF1VC_STOPPED) {
		__gus_outregw(GF1R_VOLUME, 0);
		return 1;
	}

	/* Avoid clicks when volume ramp goes too high or too low */
	if (svol < 0x0400)
		svol = 0x0400;
	if (svol > 0xfc00)
		svol = 0xfc00;
	if (evol < 0x0400)
		evol = 0x0400;
	if (evol > 0xfc00)
		evol = 0xfc00;

	/* Adjust start/end positions */
	if (svol > evol) {
		unsigned short tmp = evol;
		evol = svol;
		svol = tmp;
		vol_ctrl |= GF1VL_BACKWARD;
	}

	/* If we already are (near) the target volume, quit */
	if (evol - svol < 0x1000) {
		__gus_outregw(GF1R_VOLUME, volume);
		return 1;
	}

	__gus_outregb(GF1R_VOLUME_START, svol >> 8);
	__gus_outregb(GF1R_VOLUME_END, evol >> 8);
	__gus_outregb(GF1R_VOLUME_RATE, rate);
	__gus_outregb_slow(GF1R_VOLUME_CONTROL, vol_ctrl);
	return 0;
}

static inline void __gus_stop_voice()
{
	__gus_stop_controller(GF1R_VOICE_CONTROL);
	__gus_outregb_slow(GF1R_VOICE_CONTROL, GF1VC_STOPPED | GF1VC_STOP);
}

/* The GUS IRQ handler */
static void gf1_irq()
{
	unsigned char irq_source;	/* The contents of GF1_IRQ_STATUS register */
	boolean timer_cb = 0;		/* Call timer callback function */

	DEBUG_OFS(0, 0xCE)
	gus.eow_ignore = 0;
	while ((irq_source = inportb(GF1_IRQ_STATUS))) {
		DEBUG_OFS(1, 0xCE)

		  if (irq_source & GF1M_IRQ_DMA_COMPLETE) {
			DEBUG_OFS(4, 0x9F)
			  /* reset the IRQ pending bit */
			  __gus_inregb(GF1R_DMA_CONTROL);
			gus.dma_active = 0;

			if (gus.dma_callback)
				gus.dma_callback();
		}

		if (irq_source & (GF1M_IRQ_WAVETABLE | GF1M_IRQ_ENVELOPE)) {
			unsigned char vcirq;
			unsigned int done_mask = 0;

			/* IRQ bits are inverse (i.e. 0 = IRQ pending) */
			while ((vcirq = __gus_inregb(GF1R_IRQ_SOURCE) ^
                               (GF1IRQ_WAVE | GF1IRQ_VOLUME)) &
                               (GF1IRQ_WAVE | GF1IRQ_VOLUME)) {
				unsigned long voice = (vcirq & 0x1f);
				unsigned char voice_ctl, volume_ctl;
				unsigned int voice_mask = (1 << voice);

				/* Don't handle more than one IRQ from same voice */
				if (done_mask & voice_mask)
					continue;

				done_mask |= voice_mask;

				/* Read voice/volume selection registers */
				__gus_select_voice(voice);
				voice_ctl = __gus_inregb(GF1R_VOICE_CONTROL);
				volume_ctl = __gus_inregb(GF1R_VOLUME_CONTROL);

				if ((vcirq & GF1IRQ_WAVE) && (gus.wt_callback)
					&& !(gus.eow_ignore & voice_mask)) {
					DEBUG_OFS(5, 0xAF)
					gus.wt_callback(voice, voice_ctl, volume_ctl);
				}

				if ((vcirq & GF1IRQ_VOLUME) && (gus.vl_callback)) {
					DEBUG_OFS(6, 0xAF)
					gus.vl_callback(voice, voice_ctl, volume_ctl);
				}
			}
		}

		/* Reset timers that sent this IRQ */
		if (irq_source & (GF1M_IRQ_TIMER1 | GF1M_IRQ_TIMER2)) {
			unsigned char timer_ctl = gus.timer_ctl_reg;

			if (irq_source & GF1M_IRQ_TIMER1)
				timer_ctl &= ~GF1M_TIMER1;

			if (irq_source & GF1M_IRQ_TIMER2)
				timer_ctl &= ~GF1M_TIMER2;

			__gus_outregb_slow(GF1R_TIMER_CONTROL, timer_ctl);
			__gus_outregb_slow(GF1R_TIMER_CONTROL, gus.timer_ctl_reg);
		}

		if (irq_source & GF1M_IRQ_TIMER1)
			if (--gus.t1_countdown == 0) {
				gus.t1_countdown = gus.t1_multiple;
				gus.t1_ticks++;

				DEBUG_OFS(2, 0xCF)

				if (gus.t1_callback) {
					timer_cb = 1;
					gus.t1_callback();
				}
			}

		if (irq_source & GF1M_IRQ_TIMER2)
			if (--gus.t2_countdown == 0) {
				gus.t2_countdown = gus.t2_multiple;
				gus.t2_ticks++;

				DEBUG_OFS(3, 0xCF)

				if (gus.t2_callback)
					gus.t2_callback();
			}
#if 0
		/* The following are not used and implemented yet */
		if (irq_source & (GF1M_IRQ_MIDI_TX | GF1M_IRQ_MIDI_RX)) {
		}
#endif
	}

	irq_ack(gus.gf1_irq);

	if (timer_cb && gus.timer_callback)
		gus.timer_callback();
}

static void gf1_irq_end()
{
}

static boolean __gus_detect()
{
	/* A relatively relaxed autodetection;
	   We don't count on DRAM: GUS PnP could not have it
	   (although its anyway bad for us)
	 */
	__gus_select_voice(0);
	__gus_stop_voice();
	__gus_outregw(GF1R_FREQUENCY, 0x1234);
	__gus_outregw(GF1R_VOLUME, 0x5670);
	return ((__gus_inregw(GF1R_FREQUENCY) & 0xfffe) == 0x1234)
	  && ((__gus_inregw(GF1R_VOLUME) & 0xfff0) == 0x5670);
}

static void __gus_reset(boolean reset_io_dma)
{
	static unsigned char irqctl[16] = { 0, 0, 1, 3, 0, 2, 0, 4, 0, 0, 0, 5, 6, 0, 0, 7 };
	static unsigned char dmactl[8] = { 0, 1, 0, 2, 0, 3, 4, 5 };
	unsigned char irqtmp, dmatmp;

	/* Disable interrupts while resetting to avoid spurious IRQs */
	int i, timer, old_ints = disable();

	/* Stop the timer so that GUS IRQ won't clobber registers */
	timer = (gus.timer_ctl_reg & GF1M_TIMER1);
	if (timer)
		gus_timer_stop();

	gus.dma_active = 0;

	__gus_outregb(GF1R_RESET, 0);
	for (i = 0; i < 10; i++)
		__gus_delay();
	__gus_outregb(GF1R_RESET, GF1M_MASTER_RESET);
	for (i = 0; i < 10; i++)
		__gus_delay();

	outportb(GF1_MIDI_CTRL, GF1M_MIDI_RESET);
	for (i = 0; i < 10; i++)
		__gus_delay();
	outportb(GF1_MIDI_CTRL, 0);

	/* Reset all IRQ sources */
	__gus_outregb(GF1R_DMA_CONTROL, 0);
	__gus_outregb(GF1R_TIMER_CONTROL, 0);
	__gus_outregb(GF1R_SAMPLE_CONTROL, 0);

	/* Reset all voices */
	gus_reset(gus.voices, gus.dynmask);

	/* Flush any pending IRQs */
	inportb(GF1_IRQ_STATUS);
	__gus_inregb(GF1R_DMA_CONTROL);
	__gus_inregb(GF1R_SAMPLE_CONTROL);
	__gus_inregb(GF1R_IRQ_SOURCE);

	if (reset_io_dma) {
		/* Now set up the GUS card to required IRQs and DMAs */
		if (gus.irq[0] == gus.irq[1])
			irqtmp = irqctl[gus.irq[0]] | GF1M_IRQ_EQUAL;
		else
			irqtmp = irqctl[gus.irq[0]] | (irqctl[gus.irq[1]] << 3);

		if (gus.dma[0] == gus.dma[1])
			dmatmp = dmactl[gus.dma[0]] | GF1M_DMA_EQUAL;
		else
			dmatmp = dmactl[gus.dma[0]] | (dmactl[gus.dma[1]] << 3);

		/* Reset IRQs if possible */
		gus.mixer =
		  GF1M_MIXER_NO_LINE_IN | GF1M_MIXER_NO_OUTPUT | GF1M_MIXER_GF1_IRQ;
		if (gus.version >= GUS_CARD_VERSION_CLASSIC1) {
			outportb(GF1_REG_CTRL, 0x05);
			outportb(GF1_MIX_CTRL, gus.mixer);
			outportb(GF1_IRQ_CTRL, 0x00);	/* Reset IRQs */
			outportb(GF1_REG_CTRL, 0x00);
		}

		/* Set up DMA channels: NEVER disable MIXER_GF1_IRQ in the future */
		outportb(GF1_MIX_CTRL, gus.mixer);
		outportb(GF1_IRQ_CTRL, dmatmp);

		/* Set up IRQ channels */
		outportb(GF1_MIX_CTRL, gus.mixer | GF1M_CONTROL_SELECT);
		outportb(GF1_IRQ_CTRL, irqtmp);
	}

	__gus_outregb(GF1R_RESET, GF1M_MASTER_RESET | GF1M_OUTPUT_ENABLE | GF1M_MASTER_IRQ);
	__gus_delay();

	/* Flush IRQs again */
	inportb(GF1_IRQ_STATUS);
	__gus_inregb(GF1R_DMA_CONTROL);
	__gus_inregb(GF1R_SAMPLE_CONTROL);
	__gus_inregb(GF1R_IRQ_SOURCE);

	_irq_ack(gus.irq[0]);
	_irq_ack(gus.irq[1]);

	if (timer)
		gus_timer_continue();

	if (old_ints)
		enable();

	/* Enable output */
	__gus_mixer_output(1);
}

/* Transfer a block of data from GUS DRAM to main RAM through port I/O */
static void __gus_transfer_io_in(unsigned long address, unsigned char *source,
                                 unsigned long size)
{
	while (size) {
		register unsigned int size64k;

		size64k = 0x10000 - (address & 0xffff);
		if (size64k > size)
			size64k = size;
		size -= size64k;

		__gus_outregb(GF1R_DRAM_HIGH, address >> 16);
		while (size64k--) {
			__gus_outregw(GF1R_DRAM_LOW, address++);
			*source++ = inportb(GF1_DRAM);
		}
	}
}

/* Transfer a block of data into GUS DRAM through port I/O */
static void __gus_transfer_io(unsigned long address, unsigned char *source,
                              unsigned long size, int flags)
{
	while (size) {
		register unsigned int size64k;

		size64k = 0x10000 - (address & 0xffff);
		if (size64k > size)
			size64k = size;
		size -= size64k;

		__gus_outregb(GF1R_DRAM_HIGH, address >> 16);
		if (flags & GUS_WAVE_INVERT)
			if (flags & GUS_WAVE_16BIT)
				while (size64k-- && size64k--) {
					__gus_outregw(GF1R_DRAM_LOW, address++);
					outportb(GF1_DRAM, *source++);
					__gus_outregw(GF1R_DRAM_LOW, address++);
					outportb(GF1_DRAM, (*source++) ^ 0x80);
			} else
				while (size64k--) {
					__gus_outregw(GF1R_DRAM_LOW, address++);
					outportb(GF1_DRAM, (*source++) ^ 0x80);
		} else
			while (size64k--) {
				__gus_outregw(GF1R_DRAM_LOW, address++);
				outportb(GF1_DRAM, *source++);
			}
	}
}

/* Wait for DMA transfer to finish between 8-9 1/18sec timer ticks */
static int __gus_wait_dma()
{
	unsigned long timer;
	_farsetsel(_dos_ds);
	timer = _farnspeekl(0x46c);
	while (gus.dma_active)
		if (_farnspeekl(0x46c) - timer > 8) {
			/* Force DMA abort since something went wrong */
			__gus_reset(0);
			return -1;
		}

	return 0;
}

/* Transfer a block of data into GUS DRAM through DMA controller */
static void __gus_transfer_dma(unsigned long address, unsigned char *source,
                               unsigned long size, int flags)
{
	unsigned char dma_control;
	unsigned long bytes_left;
	unsigned long cur_size;
	unsigned long dest_addr;

	if ((gus.dma[0] > 3) || (flags & GUS_WAVE_16BIT))
		size = (size + 1) & ~1;

	bytes_left = size;
	while (bytes_left) {
		__gus_wait_dma();

		cur_size = gus.dma_buff->size;
		if (cur_size > bytes_left)
			cur_size = bytes_left;
		bytes_left -= cur_size;
		dest_addr = address;

		if (gus.dma_buff->linear != source)
			memmove(gus.dma_buff->linear, source, cur_size);
		source += cur_size;
		address += cur_size;

		/* Disable GUS -> DMA tie */
		__gus_outregb(GF1R_DMA_CONTROL, 0);
		__gus_delay();

		/* Set up the DMA */
		dma_start(gus.dma_buff, cur_size, DMA_MODE_WRITE);
		gus.dma_active = 1;

		/* Reset the DMA IRQ pending bit if set */
		__gus_inregb(GF1R_DMA_CONTROL);

		/* The 16-bit DMA channels needs a slightly different approach */
		dma_control = GF1M_DMAR_ENABLE | GF1M_DMAR_IRQ_ENABLE | gus.dma_rate;
		if (gus.dma[0] > 3) {
			dest_addr = __gus_convert_addr16(dest_addr);
			dma_control |= GF1M_DMAR_CHAN16;
		}

		__gus_outregw(GF1R_DMA_ADDRESS, dest_addr >> 4);

		if (flags & GUS_WAVE_16BIT)
			dma_control |= GF1M_DMAR_DATA16;
		if (flags & GUS_WAVE_INVERT)
			dma_control |= GF1M_DMAR_TOGGLE_SIGN;

		/* Tell GUS to start transfer */
		__gus_outregb(GF1R_DMA_CONTROL, dma_control);
	}
}

static void __gus_detect_version()
{
	unsigned char tmp;

	switch (gus.version = inportb(GF1_REVISION)) {
	  case 5:
		gus.version = GUS_CARD_VERSION_CLASSIC_ICS;
		gus.ics = 1;
		gus.ics_flipped = 1;
		break;
	  case 6:
	  case 7:
	  case 8:
	  case 9:
		gus.version = GUS_CARD_VERSION_CLASSIC_ICS;
		gus.ics = 1;
		break;
	  case 10:
		gus.version = GUS_CARD_VERSION_MAX;
		gus.codec = 1;
		break;
	  case 11:
		gus.version = GUS_CARD_VERSION_MAX1;
		gus.codec = 1;
		break;
	  case 0x30:
		gus.version = GUS_CARD_VERSION_ACE;
		break;
	  case 0x50:
		gus.version = GUS_CARD_VERSION_EXTREME;
		break;
	  case 0xff:
		/* Pre-3.7 board */
		outportb(GF1_REG_CTRL, 0x20);
		tmp = inportb(GF1_REG_CTRL);
		if ((tmp != 0xff) && (tmp & 0x06))
			gus.version = GUS_CARD_VERSION_CLASSIC1;
		else
			gus.version = GUS_CARD_VERSION_CLASSIC;
		break;
	  default:
		/* Hmm... unknown revision. Assume a safe Classic model */
#ifdef MIKMOD_DEBUG
		fprintf(stderr, "libgus: Unknown board revision (%02x)\n",
				gus.version);
#endif
		gus.version = GUS_CARD_VERSION_CLASSIC;
		break;
	}
}

static void __gus_detect_transfer()
{
	unsigned char *outbuff, *inbuff;
	unsigned int i, j, seed = 0x13243546;
	__gus_transfer_func func;

#define TRANSFER_SIZE	0x4000

	outbuff = (unsigned char *) MikMod_malloc(TRANSFER_SIZE);
	inbuff = (unsigned char *) MikMod_malloc(TRANSFER_SIZE);

	/* Suppose we have an malfunctioning GUS */
	gus.transfer = NULL;

	for (i = (gus.dma_buff ? 0 : 4); i <= 4; i++) {
		switch (i) {
		  case 0:
			gus.dma_rate = GF1M_DMAR_RATE0;
			func = __gus_transfer_dma;
			break;
		  case 1:
			gus.dma_rate = GF1M_DMAR_RATE1;
			func = __gus_transfer_dma;
			break;
		  case 2:
			gus.dma_rate = GF1M_DMAR_RATE2;
			func = __gus_transfer_dma;
			break;
		  case 3:
			gus.dma_rate = GF1M_DMAR_RATE3;
			func = __gus_transfer_dma;
			break;
		  case 4:
			func = __gus_transfer_io;
			break;
		}

		/* Fill data array each time with pseudo-random values */
		for (j = 0; j < TRANSFER_SIZE; j++)
			outbuff[j] = seed, seed =
			  ((seed + 358979323) ^ (seed >> 16)) * 314159265;

		/* Transfer the random array to GUS */
		/* Poke a security fence around dest block */
		__gus_poke(0x100 - 1, 0xAA);
		__gus_poke(0x100 - 2, 0x55);
		__gus_poke(0x100 + TRANSFER_SIZE + 0, 0xAA);
		__gus_poke(0x100 + TRANSFER_SIZE + 1, 0x55);

		func(0x100, outbuff, TRANSFER_SIZE, 0);

		if (__gus_wait_dma() == 0) {
			/* Check if the security fence was not damaged */
			if ((__gus_peek(0x100 - 1) != 0xAA)
				|| (__gus_peek(0x100 - 2) != 0x55)
				|| (__gus_peek(0x100 + TRANSFER_SIZE + 0) != 0xAA)
				|| (__gus_peek(0x100 + TRANSFER_SIZE + 1) != 0x55))
				continue;

			/* Now check if GUS DRAM really data that we expects to be transferred */
			__gus_transfer_io_in(0x100, inbuff, TRANSFER_SIZE);
			if (memcmp(outbuff, inbuff, TRANSFER_SIZE) == 0) {
				gus.transfer = func;
				break;
			}
		}
	}

#undef TRANSFER_SIZE

	MikMod_free(inbuff);
	MikMod_free(outbuff);
}

static void __gus_detect_memory()
{
	unsigned int size;
	for (size = 0; size < 1024; size += 256) {
		__gus_poke(size * 1024, 0xaa);
		if (__gus_peek(size * 1024) != 0xaa)
			break;
		__gus_poke(size * 1024, 0x55);
		if (__gus_peek(size * 1024) != 0x55)
			break;
	}
	gus.ram = size;
}

static void __gus_init()
{
	char *gusenv = getenv("ULTRASND");

	memset((void *)&gus, 0, sizeof(gus));
	gus.cmd_voice = -1;

	if (!gusenv)
		return;

	sscanf(gusenv, "%x,%d,%d,%d,%d", &gus.port, &gus.dma[0], &gus.dma[1],
		   &gus.irq[0], &gus.irq[1]);

	/* A relaxed sanity check */
	if ((gus.port < 0x100) || (gus.port > 0x1000)
		|| (gus.irq[0] < 2) || (gus.irq[0] > 15)
		|| (gus.irq[1] < 2) || (gus.irq[1] > 15)
		|| (gus.dma[0] < 0) || (gus.dma[0] > 7)
		|| (gus.dma[1] < 0) || (gus.dma[1] > 7))
		return;

	gus.voices = 32;
	gus.timer_ctl = GF1M_MASK_TIMER1 | GF1M_MASK_TIMER2;

	/* Detect if the card is really there */
	if (__gus_detect() == 0)
		return;

	/* Detect the version of Gravis Ultrasound */
	__gus_detect_version();

	/* Reset the card */
	__gus_reset(1);

	/* Detect the amount of on-board memory */
	__gus_detect_memory();

	gus.ok = 1;
}

static void __gus_kick(gus_wave_t * wave, unsigned int wave_offset)
{
	unsigned char vc;

	vc = GF1VC_IRQ;
	if (wave->format & GUS_WAVE_16BIT)
		vc |= GF1VC_DATA16;
	if (wave->format & GUS_WAVE_BACKWARD)
		vc |= GF1VC_BACKWARD;
	if (wave->format & GUS_WAVE_LOOP) {
		vc |= GF1VC_LOOP_ENABLE;
		if (wave->format & GUS_WAVE_BIDIR)
			vc |= GF1VC_BI_LOOP;
	}
	__gus_set_loop_start(vc, (wave->begin.memory << 4) + wave->loop_start);
	if (wave->format & GUS_WAVE_LOOP)
		__gus_set_loop_end(vc, (wave->begin.memory << 4) + wave->loop_end);
	else
		__gus_set_loop_end(vc, (wave->begin.memory + wave->size) << 4);
	__gus_set_current(vc, (wave->begin.memory << 4) + wave_offset + 100);
	__gus_outregb_slow(GF1R_VOICE_CONTROL, vc);
}

/* Timer 1 callback function (updates voices) */
static void __gus_timer_update()
{
	gus_wave_t *wave;
	unsigned long wave_offset;
	unsigned char *src, *top;
	unsigned int vmask = (1 << gus.cur_voice);

	if (!gus.cmd_pool_ready)
		return;

	__gus_select_voice(gus.cur_voice);
	wave_offset = 0;
	src = gus.cmd_pool;
	top = gus.cmd_pool + gus.cmd_pool_top;

#define GET_B	*src
#define GET_W	*((unsigned short *)src)
#define GET_L	*((unsigned long *)src)

	while (src < top) {
		__gus_delay();
		switch (GET_B++) {
		  case PCMD_VOICE:
			__gus_select_voice(gus.cur_voice = GET_B++);
			vmask = (1 << gus.cur_voice);
			break;
		  case PCMD_FREQ:
		/*	__gus_outregw(GF1R_FREQUENCY, GET_W++);*/
			__gus_outregw(GF1R_FREQUENCY, *(unsigned short *)src);
			src += 2;
			break;
		  case PCMD_PAN:
			__gus_outregb(GF1R_BALANCE, GET_B++);
			break;
		  case PCMD_VOLUME:
			__gus_volume_ramp_to(gus.cur_vol[gus.cur_voice] =
							/*	 GET_W++, GUS_VOLCHANGE_RAMP, GF1VL_IRQ);*/
						  *(unsigned short *)src, GUS_VOLCHANGE_RAMP, GF1VL_IRQ);
								src += 2;
			break;
		  case PCMD_VOLUME_PREPARE:
		/*	gus.cur_vol[gus.cur_voice] = GET_W++;*/
			gus.cur_vol[gus.cur_voice] = *(unsigned short *)src;
			src += 2;
			break;
		  case PCMD_OFFSET:
		/*	wave_offset = GET_L++;*/
			wave_offset = *(unsigned long *)src;
			src += 4;
			break;
		  case PCMD_START:
		/*	wave = (gus_wave_t *) GET_L++;*/
			wave = (gus_wave_t *) *(unsigned long *)src;
			src += 4;
			gus.cur_wave[gus.cur_voice] = wave;
			gus.kick_offs[gus.cur_voice] = wave_offset;
			if (__gus_volume_ramp_to(0, GUS_VOLCHANGE_RAMP, GF1VL_IRQ)) {
				__gus_kick(wave, wave_offset);
				__gus_volume_ramp_to(gus.cur_vol[gus.cur_voice],
									 GUS_VOLCHANGE_RAMP, GF1VL_IRQ);
			} else
				gus.voice_kick[gus.cur_voice] = 1;
			wave_offset = 0;
			gus.eow_ignore |= vmask;
			break;
		  case PCMD_STOP:
			/* If volume is close to nothing, abort immediately instead of
			   ramping */
			gus.cur_vol[gus.cur_voice] = 0;
			gus.cur_wave[gus.cur_voice] = NULL;
			if (__gus_volume_ramp_to(0, GUS_VOLCHANGE_RAMP, GF1VL_IRQ))
				__gus_stop_voice();
			break;
		  case PCMD_STOP_LOOP:
			__gus_outregb_slow(GF1R_VOICE_CONTROL,
							   (__gus_inregb(GF1R_VOICE_CONTROL) | GF1VC_IRQ)
							   & ~GF1VC_LOOP_ENABLE);
			__gus_outregb_slow(GF1R_VOLUME_CONTROL,
							   __gus_inregb(GF1R_VOLUME_CONTROL) &
							   ~GF1VL_ROLLOVER);
			break;
		  default:
			/* Alarm! Break out immediately */
			src = top;
			break;
		}
	}

#undef GET_B
#undef GET_W
#undef GET_L

	gus.cmd_pool_ready = 0;
	gus.cmd_pool_top = 0;
}

static void __gus_wavetable_update(unsigned int voice, unsigned int voice_ctl,
								   unsigned int volume_ctl)
{
	gus_wave_t *wave = gus.cur_wave[voice];

	if (!wave || !(wave->format & GUS_WAVE_LOOP)) {
		__gus_stop_voice();
		gus.cur_wave[voice] = NULL;
		gus.cur_vol[voice] = 0;
		if (__gus_volume_ramp_to(0, GUS_VOLCHANGE_RAMP, GF1VL_IRQ))
			__gus_stop_voice();
	}
}

static void __gus_volume_update(unsigned int voice, unsigned int voice_ctl,
								unsigned int volume_ctl)
{
	__gus_volume_ramp_to(gus.cur_vol[voice], GUS_VOLCHANGE_RAMP, GF1VL_IRQ);
	if (!gus.cur_wave[voice])
		__gus_stop_voice();
	else if (gus.voice_kick[voice])
		__gus_kick(gus.cur_wave[voice], gus.kick_offs[voice]);
	gus.voice_kick[voice] = 0;
}

/***************************************************** GUS memory manager *****/

/* Mark all GUS memory as available */
static void __gus_mem_clear()
{
	__gus_mcb *cur = gus.mcb;

	while (cur) {
		__gus_mcb *next = cur->next;
		if (cur != gus.mcb)
			MikMod_free(cur);
		cur = next;
	}

	if (!gus.mcb)
		gus.mcb = (__gus_mcb *) MikMod_malloc(sizeof(__gus_mcb));

	gus.mcb->next = gus.mcb->prev = NULL;
	gus.mcb->addr = 0;
	gus.mcb->size = gus.ram * 1024;
	gus.mcb->free = 1;
}

/* Return amount of free memory */
static unsigned int __gus_mem_get_free()
{
	__gus_mcb *cur = gus.mcb;
	unsigned int size = 0;

	if (!gus.open)
		return gus.ram * 1024;

	while (cur) {
		if (cur->free)
			size += cur->size;
		cur = cur->next;
	}

	return size;
}

/* Return largest size for a 8-bit sample */
static unsigned int __gus_mem_get_free_8()
{
	__gus_mcb *cur = gus.mcb;
	unsigned int size = 0;

	if (!gus.open)
		return 0;

	while (cur) {
		if (cur->free && (cur->size > size))
			size = cur->size;
		cur = cur->next;
	}

	return size;
}

/* Return largest size for a 16-bit sample */
static unsigned int __gus_mem_get_free_16()
{
	__gus_mcb *cur = gus.mcb;
	unsigned int size = 0;

	if (!gus.open)
		return 0;

	while (cur) {
		if (cur->free) {
			unsigned int size16 = cur->size;
			unsigned int tmp;
			/* 16-bit samples cannot cross 256K boundaries */
			tmp = 0x40000 - (cur->addr & 0x3ffff);
			if (size16 > tmp)
				size16 = tmp;
			/* 16-bit samples should be aligned on a 32-byte boundary */
			size16 -= (32 - cur->addr) & 0x1f;

			if (size16 > size)
				size = size16;

			/* Now try vice versa: skip a portion of aligned memory */
			size16 =
			  (cur->addr + cur->size) - ((cur->addr + 0x3ffff) & ~0x3ffff);
			if ((size16 < 0x7fffffff) && (size16 > size))
				size = size16;
		}
		cur = cur->next;
	}

	return size;
}

/* Allocate a segment of GUS DRAM for a sample with given bits per sample.
 * The algorithm tries to find the smallest free block that fits requested
 * size; but if found free block is larger by some (large) delta than
 * requested block size, the largest possible block is preffered.
 */
static unsigned int __gus_mem_alloc(unsigned int size, int bits16)
{
	__gus_mcb *cur = gus.mcb;
	__gus_mcb *best_max = NULL, *best_min = NULL;
	unsigned int best_max_delta = 0, best_min_delta = 0xffffffff;
	unsigned int best_max_prefix = 0, best_min_prefix = 0;
	unsigned int memaddr, memsize;

	if (!gus.open || !size || (bits16 && size > 0x40000))
		return -1;

	/* Round block size up to nearest acceptable DMA bound */
	if (bits16)
		size = (size + 0x1f) & ~0x1f;
	else
		size = (size + 0x0f) & ~0x0f;

	while (cur) {
		if (cur->free) {
			unsigned char fits = 0;

			memsize = cur->size;
			memaddr = cur->addr;

			if (bits16) {
				/* 16-bit samples cannot cross 256K boundaries */
				unsigned int tmp = 256 * 1024 - (memaddr & 0x3ffff);
				if (memsize > tmp)
					memsize = tmp;
				/* 16-bit samples should be aligned on a 32-byte boundary */
				memsize -= (32 - memaddr) & 0x1f;
				memaddr = (memaddr + 0x1f) & ~0x1f;
			}

			/* If block fits, analyze it */
			if (size <= memsize)
				fits = 1;
			/* Look if we still can complete the request by creating a free
			   block */
			else if (size <= cur->size) {
				/* Align start address to next 256k boundary */
				unsigned int endaddr = cur->addr + cur->size;
				memaddr = (cur->addr + 0x3ffff) & ~0x3ffff;
				/* Can we split current block by inserting a free block at the
				   beginning? */
				if ((memaddr < endaddr) && (memaddr + size <= endaddr))
					fits = 1;
			}

			if (fits) {
				unsigned int size_delta = cur->size - size;
				unsigned int size_prefix = memaddr - cur->addr;
				if (size_delta < best_min_delta)
					best_min = cur, best_min_delta =
					  size_delta, best_min_prefix = size_prefix;
				if (size_delta > best_max_delta)
					best_max = cur, best_max_delta =
					  size_delta, best_max_prefix = size_prefix;
			}
		}

		cur = cur->next;
	}

	if (!best_min)
		return -1;

	/* If minimal block that fits is too large, use largest block that fits */
	/* But if using the maximal block is going to create a small hole, forget
	   it */
	if ((best_max_prefix == 0)
		|| (best_max_prefix >= DRAM_HOLE_THRESHOLD)
		|| (best_min_prefix != 0))
		if (
			((best_min_delta < DRAM_HOLE_THRESHOLD) &&
			 (best_max_delta >= DRAM_HOLE_THRESHOLD)) ||
			((best_min_prefix > 0) && (best_min_prefix < DRAM_HOLE_THRESHOLD)
			 && ((best_max_prefix == 0) ||
				 (best_max_prefix > best_min_prefix))) ||
			((best_min_prefix != 0) && (best_max_prefix == 0))) {
			best_min = best_max;
			best_min_delta = best_max_delta;
			best_min_prefix = best_max_prefix;
		}

	/* Compute the DRAM address to return */
	memaddr = best_min->addr + best_min_prefix;
	if (bits16)
		memaddr = (memaddr + 0x1f) & ~0x1f;
	else
		memaddr = (memaddr + 0x0f) & ~0x0f;

	/* If we have a considerable hole at the beginning of sample,
	   create a free node describing the hole */
	if (memaddr - best_min->addr >= DRAM_SPLIT_THRESHOLD) {
		__gus_mcb *newmcb = (__gus_mcb *) MikMod_malloc(sizeof(__gus_mcb));
		newmcb->prev = best_min->prev;
		newmcb->next = best_min;
		newmcb->addr = best_min->addr;
		newmcb->size = memaddr - best_min->addr;
		newmcb->free = 1;
		best_min->addr = memaddr;
		best_min->size -= newmcb->size;
		best_min->prev = newmcb;
		if (newmcb->prev)
			newmcb->prev->next = newmcb;
	}

	/* Compute the size of hole at the end of block */
	memsize = (best_min->addr + best_min->size) - (memaddr + size);

	/* Split the block if the block is larger than requested amount */
	if (memsize > DRAM_SPLIT_THRESHOLD) {
		/* The next node cannot be free since free blocks are always glued
		   together */
		__gus_mcb *newmcb = (__gus_mcb *) MikMod_malloc(sizeof(__gus_mcb));
		best_min->size -= memsize;
		newmcb->prev = best_min;
		newmcb->next = best_min->next;
		newmcb->addr = best_min->addr + best_min->size;
		newmcb->size = memsize;
		newmcb->free = 1;
		if (best_min->next)
			best_min->next->prev = newmcb;
		best_min->next = newmcb;
	}
	best_min->free = 0;

	return memaddr;
}

static void __gus_mem_free(unsigned int addr)
{
	__gus_mcb *cur = gus.mcb;
	while (cur) {
		if (!cur->free && (cur->addr <= addr) &&
			(cur->addr + cur->size > addr)) {
			cur->free = 1;

			/* If next block is free as well, link them together */
			if (cur->next && cur->next->free) {
				__gus_mcb *next = cur->next;
				cur->size += next->size;
				cur->next = next->next;
				if (next->next)
					next->next->prev = cur;
				MikMod_free(next);
			}

			/* If previous block is free, link current block with it */
			if (cur->prev && cur->prev->free) {
				cur->prev->size += cur->size;
				cur->prev->next = cur->next;
				if (cur->next)
					cur->next->prev = cur->prev;
				MikMod_free(cur);
			}
			return;
		}
		cur = cur->next;
	}
}

static void __gus_mem_pack()
{
}

#ifdef MIKMOD_DEBUG

/* Debug dump of GUS DRAM heap */
void __gus_mem_dump()
{
	__gus_mcb *cur = gus.mcb;
	fprintf(stderr, "/-- Offset --+-- Prev --+-- Size --+-- Free --\\\n");
	while (cur) {
		fprintf(stderr, "|  %08X  | %08X |  %6d  |   %s    |\n",
				cur->addr, cur->prev ? cur->prev->addr : -1, cur->size,
				cur->free ? "yes" : " no");
		cur = cur->next;
	}
	fprintf(stderr, "\\------------+----------+----------+----------/\n");
}

#endif

/************************************************** Middle-level routines *****/

static int __gus_instrument_free(gus_instrument_t * instrument)
{
	gus_instrument_t **cur_instr;
	gus_layer_t *cur_layer;
	gus_wave_t *cur_wave, *wave_head;

	/* Remove the instrument from the list of registered instruments */
	cur_instr = (gus_instrument_t **) & gus.instr;
	while (*cur_instr) {
		if (*cur_instr == instrument) {
			*cur_instr = instrument->next;
			goto instr_loaded;
		}
		cur_instr = &(*cur_instr)->next;
	}
	return -1;

instr_loaded:
	wave_head = NULL;
	for (cur_layer = instrument->info.layer; cur_layer;
		 cur_layer = cur_layer->next)
		/* Free all waves */
		for (cur_wave = cur_layer->wave; cur_wave; cur_wave = cur_wave->next) {
			if (!wave_head)
				wave_head = cur_wave;
			if (cur_wave->begin.memory != (unsigned int)-1)
				__gus_mem_free(cur_wave->begin.memory);
		}
	if (wave_head)
		MikMod_free(wave_head);

	MikMod_free(instrument->info.layer);
	if (instrument->name)
		MikMod_free(instrument->name);
	MikMod_free(instrument);
	return 0;
}

static gus_instrument_t *__gus_instrument_get(int program)
{
	gus_instrument_t *cur_instr = (gus_instrument_t *) gus.instr;
	while (cur_instr) {
		if (cur_instr->number.instrument == program)
			return cur_instr;
		cur_instr = cur_instr->next;
	}
	return NULL;
}

static gus_instrument_t *__gus_instrument_copy(gus_instrument_t * instrument)
{
	gus_instrument_t **cur_instr, *instr;
	gus_layer_t *cur_layer, *dest_layer;
	gus_wave_t *cur_wave, *dest_wave;
	unsigned int waves, layers;

	if (!instrument || !instrument->info.layer || !gus.open)
		return NULL;

	if (__gus_instrument_get(instrument->number.instrument))
		return NULL;

	instr = (gus_instrument_t *) MikMod_malloc(sizeof(gus_instrument_t));
	*instr = *instrument;

	if (instrument->name)
		instr->name = MikMod_strdup(instrument->name);

	/* Make a copy of all layers at once */
	for (layers = 0, cur_layer = instrument->info.layer; cur_layer; layers++)
		cur_layer = cur_layer->next;

	if (!(dest_layer = instr->info.layer = (gus_layer_t *) MikMod_malloc(sizeof(gus_layer_t) * layers))) {
		if (instr->name)
			MikMod_free(instr->name);
		MikMod_free(instr);
		return NULL;
	}
	for (waves = 0, cur_layer = instrument->info.layer; cur_layer;
		 cur_layer = cur_layer->next) {
		*dest_layer = *cur_layer;
		dest_layer->wave = NULL;
		/* Count the total number of waves */
		for (cur_wave = cur_layer->wave; cur_wave; cur_wave = cur_wave->next)
			waves++;
		if (cur_layer->next)
			dest_layer->next = dest_layer + 1;
		else
			dest_layer->next = NULL;
		dest_layer++;
	}

	/* Allocate memory for waves */
	if (!(dest_wave = (gus_wave_t *) MikMod_malloc(sizeof(gus_wave_t) * waves))) {
		MikMod_free(instr->info.layer);
		if (instr->name)
			MikMod_free(instr->name);
		MikMod_free(instr);
		return NULL;
	}
	for (cur_layer = instrument->info.layer, dest_layer = instr->info.layer;
	     cur_layer; cur_layer = cur_layer->next, dest_layer = dest_layer->next)
		/* Copy all waves */
		for (cur_wave = cur_layer->wave; cur_wave; cur_wave = cur_wave->next) {
			if (!dest_layer->wave)
				dest_layer->wave = dest_wave;

			*dest_wave = *cur_wave;
			/* Mark DRAM address as unallocated */
			dest_wave->begin.memory = -1;

			if (cur_wave->next)
				dest_wave->next = (dest_wave + 1);
			else
				dest_wave->next = NULL;
			dest_wave++;
		}

	/* Insert the instrument into list of registered instruments */
	cur_instr = (gus_instrument_t **) & gus.instr;
	while (*cur_instr)
		cur_instr = &(*cur_instr)->next;
	*cur_instr = instr;

	return instr;
}

static void __gus_instruments_clear()
{
	gus_instrument_t *next_instr, *cur_instr = (gus_instrument_t *) gus.instr;
	while (cur_instr) {
		next_instr = cur_instr->next;
		__gus_instrument_free(cur_instr);
		cur_instr = next_instr;
	}
}

/******************************************************* libGUS interface *****/

/* return value: number of GUS cards installed in system */
int gus_cards()
{
	if (!gus.ok)
		__gus_init();
	return gus.ok ? 1 : 0;
}

int gus_info(gus_info_t * info, int reread)
{
	if (!gus.ok)
		__gus_init();
	if (!gus.ok)
		return -1;

	strcpy((char *)info->id, "gus0");
	info->flags = (gus.ram ? GUS_STRU_INFO_F_PCM : 0);
	info->version = gus.version;
	info->port = gus.port;
	info->irq = gus.irq[0];
	info->dma1 = gus.dma[0];
	info->dma2 = gus.dma[1];

	info->mixing_freq = gus.freq;

	info->memory_size = gus.ram * 1024;
	info->memory_free = __gus_mem_get_free();
	info->memory_block_8 = __gus_mem_get_free_8();
	info->memory_block_16 = __gus_mem_get_free_16();
	return 0;
}

int gus_open(int card, size_t queue_buffer_size, int non_block)
{
	__dpmi_meminfo struct_info, pool_info;

	if (!gus.ok)
		__gus_init();

	if (!gus.ok || gus.open || card != 0)
		return -1;

	/* Now lock the gus structure in memory */
	struct_info.address = __djgpp_base_address + (unsigned long)&gus;
	struct_info.size = sizeof(gus);
	if (__dpmi_lock_linear_region(&struct_info))
		return -1;

	/* And hook the GF1 interrupt */
	__irq_stack_count = 4;
	gus.gf1_irq =
	  irq_hook(gus.irq[0], gf1_irq, (long)gf1_irq_end - (long)gf1_irq);
	__irq_stack_count = 1;
	if (!gus.gf1_irq) {
		__dpmi_unlock_linear_region(&struct_info);
		return -1;
	}

	/* Enable the interrupt */
	irq_enable(gus.gf1_irq);
	if (gus.irq[0] > 7)
		_irq_enable(2);

	/* Allocate a DMA buffer: if we fail, we just use I/O so don't fail */
	if ((gus.transfer == NULL) || (gus.transfer == __gus_transfer_dma))
		gus.dma_buff = dma_allocate(gus.dma[0], GF1_DMA_BUFFER_SIZE);
	else
		gus.dma_buff = NULL;

	/* Detect the best available RAM -> DRAM transfer function */
	if (!gus.transfer) {
		__gus_detect_transfer();
		if (gus.transfer != __gus_transfer_dma || !gus.transfer)
			dma_free(gus.dma_buff), gus.dma_buff = NULL;

		/* If no transfer function worked, fail */
		if (!gus.transfer) {
			if (gus.dma_buff) {
				dma_free(gus.dma_buff);
				gus.dma_buff = NULL;
			}
			__dpmi_unlock_linear_region(&struct_info);
			irq_unhook(gus.gf1_irq);
			gus.gf1_irq = NULL;
			return -1;
		}
	}

	/* Allocate and lock command pool buffer */
	if (queue_buffer_size < 64)
		queue_buffer_size = 64;
	if (queue_buffer_size > 16384)
		queue_buffer_size = 16384;
	gus.cmd_pool = (unsigned char *) MikMod_malloc(queue_buffer_size);
	pool_info.address = __djgpp_base_address + (unsigned long)&gus.cmd_pool;
	pool_info.size = sizeof(queue_buffer_size);
	if (__dpmi_lock_linear_region(&pool_info)) {
		if (gus.dma_buff) {
			dma_free(gus.dma_buff);
			gus.dma_buff = NULL;
		}
		__dpmi_unlock_linear_region(&struct_info);
		irq_unhook(gus.gf1_irq);
		gus.gf1_irq = NULL;
		return -1;
	}

	gus.open++;

	__gus_mem_clear();
	gus.t1_callback = __gus_timer_update;
	gus.wt_callback = __gus_wavetable_update;
	gus.vl_callback = __gus_volume_update;
	gus_do_tempo(60);			/* Default is 60 Hz */

	return 0;
}

int gus_close(int card)
{
	__dpmi_meminfo struct_info;

	if (!gus.open || card != 0)
		return -1;

	/* First reset the card: disable any operation it can currently perform */
	__gus_reset(0);

	gus.open--;

	/* Stop the timer */
	gus_timer_stop();

	/* Free DMA buffer if used */
	if (gus.dma_buff) {
		dma_free(gus.dma_buff);
		gus.dma_buff = NULL;
	}

	/* And unhook the GF1 interrupt */
	irq_unhook(gus.gf1_irq);
	gus.gf1_irq = NULL;

	/* Unlock the gus structure */
	struct_info.address = __djgpp_base_address + (unsigned long)&gus;
	struct_info.size = sizeof(gus);
	__dpmi_unlock_linear_region(&struct_info);

	__gus_mem_clear();
	__gus_instruments_clear();

	return 0;
}

int gus_select(int card)
{
	if (!gus.open || (card != 0))
		return -1;

	return 0;
}

/* return value: same as gus_reset function
   note: this command doesn't change number of active voices and doesn't do
   hardware reset */
int gus_reset_engine_only()
{
	gus.timer_base = 100;
	return 0;
}

int gus_reset(int voices, unsigned int channel_voices)
{
	static unsigned short freq_table[32 - 14 + 1] = {
		44100, 41160, 38587, 36317, 34300, 32494, 30870, 29400, 28063, 26843,
		25725, 24696, 23746, 22866, 22050, 21289, 20580, 19916, 19293
	};
	int voice;
	int timer;

	/* No support for dynamically allocated voices for now */
	gus.dynmask = channel_voices;

	if (voices < 14)
		voices = 14;
	if (voices > 32)
		voices = 32;

	/* Stop the timer so that GUS IRQ won't clobber registers */
	timer = (gus.timer_ctl_reg & GF1M_TIMER1);
	if (timer)
		gus_timer_stop();

	/* Stop all voices */
	for (voice = 0; voice < 32; voice++) {
		__gus_select_voice(voice);
		__gus_stop_voice();
		gus.cur_wave[voice] = NULL;
		gus.cur_vol[voice] = 0;

		__gus_delay();

		/* Reset voice parameters to reasonable values */
		__gus_set_current(0, 0);
		__gus_set_loop_start(0, 0);
		__gus_set_loop_end(0, 0);
		__gus_outregw(GF1R_VOLUME, 0);
		__gus_outregb(GF1R_VOLUME_RATE, 0);
		__gus_outregb(GF1R_VOLUME_START, 0);
		__gus_outregb(GF1R_VOLUME_END, 0);
		__gus_outregb(GF1R_BALANCE, 0x7);
	}

	voice = (__gus_inregb(GF1R_VOICES) & 0x1f) + 1;

	if (voice != voices) {
		int reset = __gus_inregb(GF1R_RESET);
		__gus_outregb(GF1R_RESET, reset & ~GF1M_OUTPUT_ENABLE);
		__gus_delay();
		__gus_outregb(GF1R_VOICES, 0xc0 | (voices - 1));
		__gus_delay();
		__gus_outregb(GF1R_RESET, reset);
	}

	/* Compute the discretization frequence */
	gus.voices = voices;
	if (gus.interwave)
		gus.freq = 44100;
	else
		gus.freq = freq_table[voices - 14];

	gus_reset_engine_only();

	if (timer)
		gus_timer_continue();

	return gus.voices;
}

int gus_do_flush()
{
	DEBUG_PRINT(("gus_do_flush: top = %d\n", gus.cmd_pool_top))
	  gus.cmd_pool_ready = 1;
	return 0;
}

/* set new tempo */
void gus_do_tempo(unsigned int tempo)
{
	DEBUG_PRINT(("gus_do_tempo (%d)\n", tempo))
	  gus_timer_tempo(tempo);
	gus_timer_start();
}

/* set voice frequency in Hz */
void gus_do_voice_frequency(unsigned char voice, unsigned int freq)
{
	DEBUG_PRINT(("gus_do_voice_frequency (%d, %d)\n", voice, freq))
	  __pool_select_voice(voice);
	__pool_command_w(PCMD_FREQ,
					 (((freq << 9) + (gus.freq >> 1)) / gus.freq) << 1);
}

/* set voice pan (0-16384) (full left - full right) */
void gus_do_voice_pan(unsigned char voice, unsigned short pan)
{
	DEBUG_PRINT(("gus_do_voice_pan (%d, %d)\n", voice, pan))
	  pan >>= 10;
	if (pan > 15)
		pan = 15;
	__pool_select_voice(voice);
	__pool_command_b(PCMD_PAN, pan);
}

/* set voice volume level 0-16384 (linear) */
void gus_do_voice_volume(unsigned char voice, unsigned short vol)
{
	DEBUG_PRINT(("gus_do_voice_volume (%d, %d)\n", voice, vol))
	  if (vol > 0x3fff)
		vol = 0x3fff;
	__pool_select_voice(voice);
	__pool_command_w(PCMD_VOLUME, __gus_volume_table[vol >> 5]);
}

/* start voice
 *   voice    : voice #
 *   program  : program # or ~0 = current
 *   freq     : frequency in Hz
 *   volume   : volume level (0-16384) or ~0 = current
 *   pan      : pan level (0-16384) or ~0 = current
 */
void gus_do_voice_start(unsigned char voice, unsigned int program,
						unsigned int freq, unsigned short volume,
						unsigned short pan)
{
	gus_do_voice_start_position(voice, program, freq, volume, pan, 0);
}

/* start voice
 *   voice    : voice #
 *   program  : program # or ~0 = current
 *   freq     : frequency in Hz
 *   volume   : volume level (0-16384) or ~0 = current
 *   pan      : pan level (0-16384) or ~0 = current
 *   position : offset to wave in bytes * 16 (lowest 4 bits - fraction)
 */
void gus_do_voice_start_position(unsigned char voice, unsigned int program,
								 unsigned int freq, unsigned short volume,
								 unsigned short pan, unsigned int position)
{
	gus_instrument_t *instrument;
	gus_wave_t *wave;

	DEBUG_PRINT(
				("gus_do_voice_start_position (%d, %d, pos: %d)\n", voice,
				 program, position))

	  instrument = __gus_instrument_get(program);

	if (!instrument
		|| !instrument->info.layer
		|| !instrument->info.layer->wave
		|| instrument->flags == GUS_INSTR_F_NOT_FOUND
		|| instrument->flags == GUS_INSTR_F_NOT_LOADED) return;

	gus_do_voice_frequency(voice, freq);
	gus_do_voice_pan(voice, pan);

	/* We have to set volume different way, to avoid unneeded work in handler */
	if (volume > 0x3fff)
		volume = 0x3fff;
	__pool_command_w(PCMD_VOLUME_PREPARE, __gus_volume_table[volume >> 5]);

	switch (instrument->mode) {
	  case GUS_INSTR_SIMPLE:
		wave = instrument->info.layer->wave;
		if (position)
			__pool_command_l(PCMD_OFFSET, position);
		__pool_command_l(PCMD_START, (unsigned long)wave);
		break;
	}
}

/* stop voice
 *   mode = 0 : stop voice now
 *   mode = 1 : disable wave loop and finish it
 */
void gus_do_voice_stop(unsigned char voice, unsigned char mode)
{
	__pool_select_voice(voice);
	if (mode)
		__pool_command(PCMD_STOP_LOOP);
	else
		__pool_command(PCMD_STOP);
}

/* wait x ticks - this command is block separator
   all commands between blocks are interpreted in the begining of one tick */
void gus_do_wait(unsigned int ticks)
{
	DEBUG_PRINT(("gus_do_wait (%d)\n", ticks))

	  ticks += gus.t1_ticks;
	while ((int)(ticks - gus.t1_ticks) > 0);
}

int gus_get_voice_status(int voice)
{
	__gus_select_voice(voice);
	return __gus_inregb(GF1R_VOICE_CONTROL) & GF1VC_STOPPED ? 0 : 1;
}

/* return value: file handle (descriptor) for /dev/gus */
int gus_get_handle()
{
	/* Return stdout handle so that select() will "work" with it */
	return 0;
}

/* return value: zero if instrument was successfully allocated */
int gus_memory_alloc(gus_instrument_t * instrument)
{
	gus_instrument_t *instr = __gus_instrument_copy(instrument);
	gus_layer_t *cur_layer;
	gus_wave_t *cur_wave;

	DEBUG_PRINT(("gus_memory_alloc (%d)\n", instrument->number.instrument))

	  if (!instr)
		return -1;

	for (cur_layer = instr->info.layer; cur_layer;
		 cur_layer = cur_layer->next) for (cur_wave = cur_layer->wave;
										   cur_wave;
										   cur_wave = cur_wave->next) {
			if (cur_layer->mode == GUS_INSTR_SIMPLE) {
				cur_wave->begin.memory = __gus_mem_alloc(cur_wave->size,
														 cur_wave->format &
														 GUS_WAVE_16BIT);
				if (cur_wave->begin.memory == (unsigned int)-1) {
					__gus_instrument_free(instr);
					return -1;
				}
				gus.transfer(cur_wave->begin.memory, cur_wave->begin.ptr,
							 cur_wave->size, cur_wave->format);
			} else if (cur_layer->mode == GUS_INSTR_PATCH)
				/* not supported yet */ ;
		}

	return 0;
}

/* return value: zero if instrument was successfully removed */
int gus_memory_free(gus_instrument_t * instrument)
{
	gus_instrument_t *cur_instr = gus.instr;

	DEBUG_PRINT(("gus_memory_free (%d)\n", instrument->number.instrument))

	  for (; cur_instr; cur_instr = cur_instr->next)
		if (cur_instr->number.instrument == instrument->number.instrument)
			return __gus_instrument_free(cur_instr);

	return -1;
}

/* return value: unused gus memory in bytes */
int gus_memory_free_size()
{
	return __gus_mem_get_free();
}

/* return value: zero if success */
int gus_memory_pack()
{
	__gus_mem_pack();
	return 0;
}

/* return value: gus memory size in bytes */
int gus_memory_size()
{
	return gus.ram * 1024;
}

/* return value: current largest free block for 8-bit or 16-bit wave */
int gus_memory_free_block(int w_16bit)
{
	return w_16bit ? __gus_mem_get_free_16() : __gus_mem_get_free_8();
}

/* input value:	see to GUS_DOWNLOAD_MODE_XXXX constants (gus.h)
   return value: zero if samples & instruments was successfully removed	from
   GF1 memory manager */
int gus_memory_reset(int mode)
{
	__gus_mem_clear();
	__gus_instruments_clear();
	return 0;
}

/* return value: zero if command queue was successfully flushed */
int gus_queue_flush()
{
	return 0;
}

/* input value: echo buffer size in items (if 0 - erase echo buffer) */
int gus_queue_read_set_size(int items)
{
	return 0;
}

/* input value: write queue size in items (each item have 8 bytes) */
int gus_queue_write_set_size(int items)
{
	return 0;
}

/* return value: zero if successfull */
int gus_timer_start()
{
	gus.timer_ctl_reg |= GF1M_TIMER1;
	__gus_outregb_slow(GF1R_TIMER_CONTROL, gus.timer_ctl_reg);

	gus.timer_ctl = gus.timer_ctl & ~GF1M_MASK_TIMER1;
	outportb(GF1_TIMER_CTRL, 0x04);
	outportb(GF1_TIMER_DATA, gus.timer_ctl | GF1M_START_TIMER1);
	return 0;
}

/* return value: zero if timer was stoped */
int gus_timer_stop()
{
	gus.timer_ctl_reg &= ~GF1M_TIMER1;
	__gus_outregb_slow(GF1R_TIMER_CONTROL, gus.timer_ctl_reg);

	gus.timer_ctl = gus.timer_ctl | GF1M_MASK_TIMER1;
	outportb(GF1_TIMER_CTRL, 0x04);
	outportb(GF1_TIMER_DATA, gus.timer_ctl);
	return 0;
}

/* return value: zero if setup was success */
int gus_timer_tempo(int ticks)
{
	unsigned int counter;

	/* Limit ticks per second to 1..1000 range */
	if (ticks < 1)
		ticks = 1;
	if (ticks > 1000)
		ticks = 1000;

	/* GF1 timer1 period is 80 usecs, 12500 times per second */
	counter = 1250000 / (ticks * gus.timer_base);
	gus.t1_multiple = 1;
	while (counter > 255) {
		counter >>= 1;
		gus.t1_multiple <<= 1;
	}
	gus.t1_countdown = gus.t1_multiple;
	__gus_outregb(GF1R_TIMER1, 256 - counter);
	return 0;
}

/* return value: zero if timer will be continue */
int gus_timer_continue()
{
	return gus_timer_start();
}

/* return value: zero if setup was success (default timebase = 100) */
int gus_timer_base(int base)
{
	gus.timer_base = base;
	return 0;
}

void gus_timer_callback(void (*timer_callback) ())
{
	gus.timer_callback = timer_callback;
}

void gus_convert_delta(unsigned int type, unsigned char *dest,
					   unsigned char *src, size_t size)
{
	if (!(type & GUS_WAVE_DELTA))
		return;

	/* This doesn't depend much on wave signedness, since addition/subtraction
	   do not depend on operand signedness */
	if (type & GUS_WAVE_16BIT) {
		unsigned short delta = type & GUS_WAVE_UNSIGNED ? 0x8000 : 0;
		while (size--) {
			delta = *(unsigned short *)dest = *(unsigned short *)src + delta;
			src += sizeof(unsigned short);
			dest += sizeof(unsigned short);
		}
	} else {
		unsigned char delta = type & GUS_WAVE_UNSIGNED ? 0x80 : 0;
		while (size--) {
			delta = *(unsigned char *)dest = *(unsigned char *)src + delta;
			src++;
			dest++;
		}
	}
}

int gus_dma_usage (int use)
{
	if (gus.dma_buff)
		return -1;
	gus.transfer = __gus_transfer_io;
	return 0;
}

#endif /* DRV_ULTRA */

/* ex:set ts=4: */
