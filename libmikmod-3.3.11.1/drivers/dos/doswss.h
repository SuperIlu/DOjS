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

  $Id$

  Windows Sound System and compatible soundcards definitions

==============================================================================*/

#ifndef __DOSWSS_H__
#define __DOSWSS_H__

#include "dosdma.h"
#include "dosirq.h"

#define WSS_ADDR			(wss.port + 0x04)
#define WSS_DATA			(wss.port + 0x05)
#define WSS_STATUS			(wss.port + 0x06)
#define WSS_PIO				(wss.port + 0x07)

/* WSS_ADDR: Bits 0-4 select an internal register to read/write */
#define WSSR_INPUT_L			0x00	/* Left input control register */
#define WSSR_INPUT_R			0x01	/* RIght input control register */
#define WSSR_AUX1_L			0x02	/* Left Aux #1 input control */
#define WSSR_AUX1_R			0x03	/* Right Aux #1 input control */
#define WSSR_CD_L			0x04	/* Left Aux #2 input control */
#define WSSR_CD_R			0x05	/* Right Aux #2 input control */
#define WSSR_MASTER_L			0x06	/* Left output control */
#define WSSR_MASTER_R			0x07	/* Right output control */
#define WSSR_PLAY_FORMAT		0x08	/* Clock and data format */
#define WSSR_IFACE_CTRL			0x09	/* Interface control */
#define WSSR_PIN_CTRL			0x0a	/* Pin control */
#define WSSR_TEST_INIT			0x0b	/* Test and initialization */
#define WSSR_MISC_INFO			0x0c	/* Miscellaneaous information */
#define WSSR_LOOPBACK			0x0d	/* Digital Mix */
#define WSSR_COUNT_HIGH			0x0e	/* Playback Upper Base Count */
#define WSSR_COUNT_LOW			0x0f	/* Playback Lower Base Count */
#define WSSR_ALT_FEATURE_1		0x10	/* alternate #1 feature enable */
#define WSSR_ALT_FEATURE_2		0x11	/* alternate #2 feature enable */
#define WSSR_LINE_IN_L			0x12	/* left line input control */
#define WSSR_LINE_IN_R			0x13	/* right line input control */
#define WSSR_TIMER_LOW			0x14	/* timer low byte */
#define WSSR_TIMER_HIGH			0x15	/* timer high byte */
#define WSSR_IRQ_STATUS			0x18	/* irq status register */
#define WSSR_MONO_IO_CTRL		0x1a	/* mono input/output control */
#define WSSR_REC_FORMAT			0x1c	/* record format */
#define WSSR_REC_COUNT_HIGH		0x1e	/* record upper count */
#define WSSR_REC_COUNT_LOW		0x1f	/* record lower count */

/* WSS_ADDR bits 7-5 definition */
#define WSSM_INIT			0x80	/* Codec is initializing */
#define WSSM_MCE			0x40	/* Mode change enable */
#define WSSM_TRD			0x20	/* Transfer Request Disable */
/* bits 4-0 are indirect register address (0-15) */

/* WSS_STATUS bit masks */
#define WSSM_CUL			0x80	/* Capture data upper/lower byte */
#define WSSM_CLR			0x40	/* Capture left/right sample */
#define WSSM_CRDY			0x20	/* Capture data read */
#define WSSM_SOUR			0x10	/* Playback over/under run error */
#define WSSM_PUL			0x08	/* Playback upper/lower byte */
#define WSSM_PLR			0x04	/* Playback left/right sample */
#define WSSM_PRDY			0x02	/* Playback data register read */
#define WSSM_INT			0x01	/* interrupt status */

/* Definitions for output level registers */
#define WSSM_MUTE			0x80	/* Mute this output source */
/* bits 5-0 are left output attenuation select (0-63) */
/* bits 5-0 are right output attenuation select (0-63) */

/* Definitions for clock and data format register (WSSR_PLAY_FORMAT) */
#define WSSM_STEREO			0x10	/* stero mode */
#define WSSM_ULAW_8			0x20	/* 8-bit U-law companded */
#define WSSM_16BITS			0x40	/* 16 bit twos complement data - little endian */
#define WSSM_ALAW_8			0x60	/* 8-bit A-law companded */
#define WSSM_16BITS_BE			0xc0	/* 16-bit twos complement data - big endian */
#define WSSM_ADPCM_16			0xa0	/* 16-bit ADPCM */
/* Bits 3-1 define frequency divisor */
#define WSSM_XTAL1			0x00	/* 24.576 crystal */
#define WSSM_XTAL2			0x01	/* 16.9344 crystal */

/* Definitions for interface control register (WSSR_IFACE_CTRL) */
#define WSSM_CAPTURE_PIO		0x80	/* Capture PIO enable */
#define WSSM_PLAYBACK_PIO		0x40	/* Playback PIO enable */
#define WSSM_AUTOCALIB			0x08	/* auto calibrate */
#define WSSM_SINGLE_DMA			0x04	/* Use single DMA channel */
#define WSSM_PLAYBACK_ENABLE		0x01	/* playback enable */

/* Definitions for Pin control register (WSSR_PIN_CTRL) */
#define WSSM_IRQ_ENABLE			0x02	/* interrupt enable */
#define WSSM_XCTL1			0x40	/* external control #1 */
#define WSSM_XCTL0			0x80	/* external control #0 */

/* Definitions for WSSR_TEST_INIT register */
#define WSSM_CALIB_IN_PROGRESS 0x20	/* auto calibrate in progress */

/* Definitions for misc control register (WSR_MISC_INFO) */
#define WSSM_MODE2			0x40	/* MODE 2 */
#define WSSM_MODE3			0x6c	/* MODE 3 - enhanced mode */

/* Definitions for codec irq status (WSSR_IRQ_STATUS) */
#define WSSM_PLAYBACK_IRQ		0x10
#define WSSM_RECORD_IRQ			0x20
#define WSSM_TIMER_IRQ			0x40

typedef unsigned char boolean;

#ifndef FALSE
#define FALSE				0
#define TRUE				1
#endif

/* Play mode bits */
#define WSSMODE_16BITS			0x0001
#define WSSMODE_STEREO			0x0002
#define WSSMODE_SIGNED			0x0004

/* You can fill some members of this struct (i.e. port,irq,dma) before
 * calling wss_detect() or wss_open()... this will ignore environment settings.
 */
typedef struct __wss_state_s {
	boolean ok;			/* Set if this structure is properly filled */
	int port;			/* Base codec port */
	int irq;			/* codec IRQ */
	int dma;			/* codec DMA */
	struct irq_handle *irq_handle;	/* The interrupt handler */
	dma_buffer *dma_buff;		/* Pre-allocated DMA buffer */
	unsigned char mode;		/* Current WSS mode (WSSMODE_XXX) */
	boolean open;			/* Whenever the card has been opened */
	int samples;			/* Number of samples in DMA buffer */
	unsigned char level;		/* Output level (63..0): doesn't change when mute */
	unsigned char curlevel;		/* Current output level (63(min)..0(max)) */
	volatile int irqcount;		/* Incremented on each IRQ... for diagnostics */
	void (*timer_callback) ();	/* Called TWICE per buffer play */
} __wss_state;

extern __wss_state wss;

/* Wait until codec finishes initialization */
static inline boolean __wss_wait()
{
	int count;
	for (count = 10000; count >= 0; count--)
		if (!(inportb(WSS_ADDR) & WSSM_INIT))
			return TRUE;
	return FALSE;
}

static inline void __wss_outreg(unsigned char reg, unsigned char val)
{
	outportb(WSS_ADDR, reg);
	outportb(WSS_DATA, val);
}

static inline unsigned char __wss_inreg(unsigned char reg)
{
	outportb(WSS_ADDR, reg);
	return inportb(WSS_DATA);
}

/* Set some bits in a specific register */
static inline void __wss_regbit_set(unsigned char reg, unsigned char mask)
{
	outportb(WSS_ADDR, reg);
	outportb(WSS_DATA, inportb(WSS_DATA) | mask);
}

/* Reset some bits in a specific register */
static inline void __wss_regbit_reset(unsigned char reg, unsigned char mask)
{
	outportb(WSS_ADDR, reg);
	outportb(WSS_DATA, inportb(WSS_DATA) & ~mask);
}

/* Detect whenever WSS is present and fill "wss" structure */
extern boolean wss_detect();
/* Reset WSS */
extern void wss_reset();
/* Open WSS for usage */
extern boolean wss_open();
/* Finish working with WSS */
extern boolean wss_close();
/* Enable/disable speaker output */
extern void wss_output(boolean enable);
/* Adjust frequency rate to nearest WSS available */
extern unsigned int wss_adjust_freq(unsigned int freq);
/* Start playing from DMA buffer in either 8/16 bit mono/stereo */
extern boolean wss_start_dma(unsigned char mode, unsigned int freq);
/* Stop playing from DMA buffer */
extern void wss_stop_dma();
/* Query current position/total size of the DMA buffer */
extern void wss_query_dma(unsigned int *dma_size, unsigned int *dma_pos);
/* Set output level (0(min)-63(max)) */
extern void wss_level(int level);

#endif /* __DOSWSS_H__ */

/* ex:set ts=4: */
