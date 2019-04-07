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

  libGUS-alike definitions for DOS

==============================================================================*/

#ifndef __DOSGUS_H__
#define __DOSGUS_H__

#include <pc.h>
#include "dosdma.h"
#include "dosirq.h"
#include "libgus.h"

/* Private header file for a libGUS-alike library for DOS */

#define JOYSTICK_TIMER			(gus.port+0x201)	/* 201 */
#define JOYSTICK_DATA			(gus.port+0x201)	/* 201 */

#define GF1_MIDI_CTRL			(gus.port+0x100)	/* 3X0 */
#define GF1_MIDI_DATA			(gus.port+0x101)	/* 3X1 */

#define GF1_VOICESEL			(gus.port+0x102)	/* 3X2 */
#define GF1_REGSEL			(gus.port+0x103)	/* 3X3 */
#define GF1_DATA			(gus.port+0x104)	/* 3X4 */
#define GF1_DATA_LOW			(gus.port+0x104)	/* 3X4 */
#define GF1_DATA_HIGH			(gus.port+0x105)	/* 3X5 */
#define GF1_IRQ_STATUS			(gus.port+0x006)	/* 2X6 */
#define GF1_DRAM			(gus.port+0x107)	/* 3X7 */

#define GF1_MIX_CTRL			(gus.port+0x000)	/* 2X0 */
#define GF1_TIMER_CTRL			(gus.port+0x008)	/* 2X8 */
#define GF1_TIMER_DATA			(gus.port+0x009)	/* 2X9 */
#define GF1_IRQ_CTRL			(gus.port+0x00B)	/* 2XB */
#define GF1_REG_CTRL			(gus.port+0x00F)	/* 2XF */

#define GF1_REVISION			(gus.port+0x506)	/* 7X6 */

/* The GF1 hardware clock rate */
#define CLOCK_RATE			9878400L

/* GF1 voice-independent registers */
#define	GF1R_DMA_CONTROL		0x41
#define	GF1R_DMA_ADDRESS		0x42
#define	GF1R_DRAM_LOW			0x43
#define	GF1R_DRAM_HIGH			0x44

#define	GF1R_TIMER_CONTROL		0x45
#define	GF1R_TIMER1			0x46
#define	GF1R_TIMER2			0x47

#define	GF1R_SAMPLE_RATE		0x48
#define	GF1R_SAMPLE_CONTROL		0x49

#define	GF1R_JOYSTICK			0x4B
#define	GF1R_RESET			0x4C

/* GF1 voice-specific registers */
#define	GF1R_VOICE_CONTROL		0x00
#define	GF1R_FREQUENCY			0x01
#define	GF1R_START_HIGH			0x02
#define	GF1R_START_LOW			0x03
#define	GF1R_END_HIGH			0x04
#define	GF1R_END_LOW			0x05
#define	GF1R_VOLUME_RATE		0x06
#define	GF1R_VOLUME_START		0x07
#define	GF1R_VOLUME_END			0x08
#define	GF1R_VOLUME			0x09
#define	GF1R_ACC_HIGH			0x0a
#define	GF1R_ACC_LOW			0x0b
#define	GF1R_BALANCE			0x0c
#define	GF1R_VOLUME_CONTROL		0x0d
#define	GF1R_VOICES			0x0e
#define	GF1R_IRQ_SOURCE			0x0f

/* Add this to above registers for reading */
#define GF1R_READ_MASK			0x80

/* MIDI */
#define	GF1M_MIDI_RESET			0x03
#define	GF1M_MIDI_ENABLE_XMIT		0x20
#define	GF1M_MIDI_ENABLE_RCV		0x80

#define	GF1M_MIDI_RCV_FULL		0x01
#define	GF1M_MIDI_XMIT_EMPTY		0x02
#define	GF1M_MIDI_FRAME_ERR		0x10
#define	GF1M_MIDI_OVERRUN		0x20
#define	GF1M_MIDI_IRQ_PEND		0x80

/* Joystick */
#define	GF1M_JOY_POSITION		0x0f
#define	GF1M_JOY_BUTTONS		0xf0

/* GF1_IRQ_STATUS (port 2X6) */
#define	GF1M_IRQ_MIDI_TX		0x01	/* pending MIDI xmit IRQ */
#define	GF1M_IRQ_MIDI_RX		0x02	/* pending MIDI recv IRQ */
#define	GF1M_IRQ_TIMER1			0x04	/* general purpose timer */
#define	GF1M_IRQ_TIMER2			0x08	/* general purpose timer */
#define	GF1M_IRQ_WAVETABLE		0x20	/* pending wavetable IRQ */
#define	GF1M_IRQ_ENVELOPE		0x40	/* pending volume envelope IRQ */
#define	GF1M_IRQ_DMA_COMPLETE		0x80	/* pending dma transfer complete IRQ */

/* GF1_MIX_CTRL (port 2X0) */
#define	GF1M_MIXER_NO_LINE_IN		0x01	/* 0: enable */
#define	GF1M_MIXER_NO_OUTPUT		0x02	/* 0: enable */
#define	GF1M_MIXER_MIC_IN		0x04	/* 1: enable */
#define	GF1M_MIXER_GF1_IRQ		0x08	/* 1: enable */
#define GF1M_GF1_COMBINED_IRQ		0x10	/* 1: IRQ1 == IRQ2 */
#define	GF1M_MIDI_LOOPBACK		0x20	/* 1: enable loop back */
#define	GF1M_CONTROL_SELECT		0x40	/* 0: DMA latches; 1: IRQ latches */

/* Timer data register (2X9) */
#define GF1M_START_TIMER1		0x01
#define GF1M_START_TIMER2		0x02
#define GF1M_MASK_TIMER1		0x20
#define GF1M_MASK_TIMER2		0x40
#define GF1M_TIMER_CLRIRQ		0x80

/* IRQ/DMA control register (2XB) */
#define GF1M_IRQ_EQUAL			0x40
#define GF1M_DMA_EQUAL			0x40

/* (0x41) DMA control register bits */
#define	GF1M_DMAR_ENABLE		0x01	/* 1: go */
#define	GF1M_DMAR_READ			0x02	/* 1: read (->RAM), 0: write (->DRAM) */
#define	GF1M_DMAR_CHAN16		0x04	/* 1: 16 bit, 0: 8 bit DMA channel */
#define	GF1M_DMAR_RATE			0x18	/* 00: fast, 11: slow */
#define	GF1M_DMAR_IRQ_ENABLE		0x20	/* 1: enable */
#define	GF1M_DMAR_IRQ_PENDING		0x40	/* R: DMA irq pending */
#define	GF1M_DMAR_DATA16		0x40	/* W: 0: 8 bits; 1: 16 bits per sample */
#define	GF1M_DMAR_TOGGLE_SIGN		0x80	/* W: 1: invert high bit */

/* DMA transfer rate divisors */
#define	GF1M_DMAR_RATE0			0x00	/* Fastest DMA xfer (~650khz) */
#define	GF1M_DMAR_RATE1			0x08	/* fastest / 2 */
#define	GF1M_DMAR_RATE2			0x10	/* fastest / 4 */
#define	GF1M_DMAR_RATE3			0x18	/* Slowest DMA xfer (fastest / 8) */

/* (0x45) Timer Control */
#define GF1M_TIMER1			0x04	/* Enable timer 1 IRQ */
#define GF1M_TIMER2			0x08	/* Enable timer 2 IRQ */

/* (0x49) Sampling (ADC) control register */
#define	GF1M_DMAW_ENABLE		0x01	/* 1: Start sampling */
#define	GF1M_DMAW_MODE			0x02	/* 0: mono, 1: stereo */
#define	GF1M_DMAW_CHAN16		0x04	/* 0: 8 bit, 1: 16 bit */
#define	GF1M_DMAW_IRQ_ENABLE		0x20	/* 1: enable IRQ */
#define	GF1M_DMAW_IRQ_PENDING		0x40	/* 1: irq pending */
#define	GF1M_DMAW_TOGGLE_SIGN		0x80	/* 1: invert sign bit */

/* (0x4C) GF1 reset register */
#define	GF1M_MASTER_RESET		0x01	/* 0: hold in reset */
#define	GF1M_OUTPUT_ENABLE		0x02	/* 1: enable output */
#define	GF1M_MASTER_IRQ			0x04	/* 1: master IRQ enable */

/* (0x0,0x80) Voice control register - GF1R_VOICE_CONTROL */
#define	GF1VC_STOPPED			0x01	/* 1: voice has stopped */
#define	GF1VC_STOP			0x02	/* 1: stop voice */
#define	GF1VC_DATA16			0x04	/* 0: 8 bit, 1: 16 bit */
#define	GF1VC_LOOP_ENABLE		0x08	/* 1: enable */
#define	GF1VC_BI_LOOP			0x10	/* 1: bi directional looping */
#define	GF1VC_IRQ			0x20	/* 1: enable voice's wave irq */
#define	GF1VC_BACKWARD			0x40	/* 0: increasing, 1: decreasing */
#define	GF1VC_IRQ_PENDING		0x80	/* 1: wavetable irq pending */

/* (0x01,0x81) Frequency control */
/* Bit 0	- Unused */
/* Bits	1-9	- Fractional portion */
/* Bits	10-15	- Integer portion */

/* (0x02,0x82) Accumulator start address - GF1R_START_HIGH */
/* Bits	0-11	- HIGH 12 bits of address */
/* Bits	12-15	- Unused */

/* (0x03,0x83) Accumulator start address - GF1R_START_LOW */
/* Bits	0-4	- Unused */
/* Bits	5-8	- Fractional portion */
/* Bits	9-15	- Low 7 bits of integer portion */

/* (0x04,0x84) Accumulator end address - GF1R_END_HIGH */
/* Bits	0-11	- HIGH 12 bits of address */
/* Bits	12-15	- Unused */

/* (0x05,0x85) Accumulator end address - GF1R_END_LOW */
/* Bits	0-4	- Unused */
/* Bits	5-8	- Fractional portion */
/* Bits	9-15	- Low 7 bits of integer portion */

/* (0x06,0x86) Volume Envelope control register - GF1R_VOLUME_RATE */
#define	GF1VL_RATE_MANTISSA		0x3f
#define	GF1VL_RATE_RANGE		0xC0

/* (0x07,0x87) Volume envelope start - GF1R_VOLUME_START */
#define	GF1VL_START_MANT		0x0F
#define	GF1VL_START_EXP			0xF0

/* (0x08,0x88) Volume envelope end - GF1R_VOLUME_END */
#define	GF1VL_END_MANT			0x0F
#define	GF1VL_END_EXP			0xF0

/* (0x09,0x89) Current volume register - GF1R_VOLUME */
/* Bits	0-3	- Unused */
/* Bits	4-11	- Mantissa of current volume */
/* Bits	10-15	- Exponent of current volume */

/* (0x0A,0x8A) Accumulator value (high) */
/* Bits	0-12	- HIGH 12 bits of current position (a19-a7) */

/* (0x0B,0x8B) Accumulator value (low) */
/* Bits	0-8	- Fractional portion */
/* Bits	9-15	- Integer portion of low adress (a6-a0) */

/* (0x0C,0x8C) Pan (balance) position */
/* Bits	0-3	- Balance position 0=full left, 0x0f=full right */

/* (0x0D,0x8D) Volume control register - GF1R_VOLUME_CONTROL */
#define	GF1VL_STOPPED			0x01	/* volume has stopped */
#define	GF1VL_STOP			0x02	/* stop volume */
#define	GF1VL_ROLLOVER			0x04	/* Roll PAST end & gen IRQ */
#define	GF1VL_LOOP_ENABLE		0x08	/* 1: enable */
#define	GF1VL_BI_LOOP			0x10	/* 1: bi directional looping */
#define	GF1VL_IRQ			0x20	/* 1: enable voice's volume irq */
#define	GF1VL_BACKWARD			0x40	/* 0: increasing, 1: decreasing */
#define	GF1VL_IRQ_PENDING		0x80	/* 1: wavetable irq pending */

/* (0x0E,0x8E) Number of active voices */
/* Bits	0-5	- Number of active voices - 1 */

/* (0x0F,0x8F) Sources of IRQs */
/* Bits	0-4	- interrupting voice number */
/* Bit 5	- Always a 1 */
#define	GF1IRQ_VOLUME			0x40	/* individual voice irq bit */
#define	GF1IRQ_WAVE			0x80	/* individual waveform irq bit */

/* Commands are pooled and executed ON TIMER (1st timer) interrupt.
 * Currently there is a limit on the number of commands that you can
 * issue between gus_do_flush (...); this should not be an issue however
 * because each voice has a limited (little) set of parameters that
 * you can change (freq, vol, pan... what else?)
 *
 * The pool is a pseudo-CPU code that gets executed once per timer interrupt.
 */

/* Below are definitions for commands placed in GUS command pool */
#define PCMD_NOP			0x00	/* Traditionally ... */
#define PCMD_VOICE			0x01	/* +B: select voice */
#define PCMD_START			0x02	/* +L: start voice */
#define PCMD_STOP			0x03	/*     stop voice */
#define PCMD_FREQ			0x04	/* +W: set frequence */
#define PCMD_VOLUME			0x05	/* +W: set volume */
#define PCMD_VOLUME_PREPARE		0x06	/* +W: prepare to set volume on (soon to follow) kick */
#define PCMD_PAN			0x07	/* +B: set panning */
#define PCMD_OFFSET			0x08	/* +L: set DRAM offset */
#define PCMD_STOP_LOOP			0x09	/*     stop looping */

#define GUS_VOLCHANGE_RAMP		0x20	/* Volume change ramp speed */

/* Definition for the boolean type */
typedef unsigned char boolean;
/* Prototype for functions that do block transfers to GUS DRAM:
   flags can contain any of the following bits:
   GUS_WAVE_16BIT    - sample is 16-bit
   GUS_WAVE_UNSIGNED - do not invert sign bit while downloading
 */
typedef void (*__gus_transfer_func) (unsigned long address,
                                     unsigned char *source,
                                     unsigned long size, int flags);
typedef void (*__gus_callback) ();
typedef void (*__gus_callback_3) (unsigned int, unsigned int, unsigned int);

/* Structure used to keep track of all on-board GUS memory */
typedef struct __struct_gus_mcb {
	struct __struct_gus_mcb *next;		/* Next MCB in chain */
	struct __struct_gus_mcb *prev;		/* Previous MCB in chain */
	unsigned int addr;			/* GUS DRAM address */
	unsigned int size;			/* Memory block size */
	int free;				/* 1: block is free */
} __gus_mcb;

/* Structure defining overall GUS state/information */
typedef struct __gus_state_s {
	unsigned int port;			/* Base I/O port (0x220, 0x240, ...) */
	unsigned int irq[2];			/* GF1 IRQ and MIDI IRQ */
	unsigned int dma[2];			/* Play / record DMA */
	unsigned int ram;			/* Memory size (K), i.e. 256, 1024 etc */
	unsigned int version;			/* GUS version (see GUS_CARD_VERSION_XXX in libgus.h */
	unsigned int freq;			/* Current mixing frequency */
	unsigned int voices;			/* Active voices (14-32) */
	unsigned int dynmask;			/* Dynamically allocated voices mask */
	unsigned int timer_base;		/* The relative timer speed in percents (def: 100) */
	volatile unsigned int t1_ticks;		/* Incremented per each timer1 tick */
	volatile unsigned int t2_ticks;		/* Incremented per each timer2 tick */
	volatile unsigned int t1_countdown;	/* t1_callback is called when this reaches zero */
	volatile unsigned int t2_countdown;	/* t2_callback is called when this reaches zero */
	unsigned int t1_multiple;		/* Timer1 handler is called once per such many ticks */
	unsigned int t2_multiple;		/* Timer2 handler is called once per such many ticks */
	struct irq_handle *gf1_irq;		/* The interrupt handler for GF1 events */
	dma_buffer *dma_buff;			/* Pre-allocated DMA buffer */
	__gus_callback dma_callback;		/* Routine called at end of DMA transfers */
	__gus_callback t1_callback;		/* Routine called on Timer1 events */
	__gus_callback t2_callback;		/* Routine called on Timer1 events */
	__gus_callback timer_callback;		/* Called once per TEMPO ticks */
	__gus_callback_3 wt_callback;		/* Routine called on WaveTable events */
	__gus_callback_3 vl_callback;		/* Routine called on Volume ramp events */
	__gus_mcb *mcb;				/* Chained list of memory control blocks */
	__gus_transfer_func transfer;		/* Best working function for DRAM transfer */
	gus_instrument_t *instr;		/* The list of registered instruments */
	unsigned short mixer;			/* Current mixer register state */
	unsigned char dma_rate;			/* One of GF1M_DMAR_RATEX constants defined above */
	unsigned char timer_ctl;		/* Timer control register value (2x8/2x9) */
	unsigned char timer_ctl_reg;		/* Timer control register value (GF1/0x45) */
	boolean ok;				/* Is the information below okay? */
	boolean open;				/* 1 if between gus_open() and gus_close() */
	boolean ics;				/* Is it equipped with an ICS mixer? */
	boolean ics_flipped;			/* rev 5 (3.7) has flipped R/L mixer */
	boolean codec;				/* Is it equipped with a GUS MAX codec? */
	boolean interwave;			/* GUS InterWave card */
	volatile boolean dma_active;		/* DMA is transferring data */
	volatile boolean cmd_pool_ready;	/* Flush cmd_pool during timer interrupt */
	unsigned char cmd_voice;		/* Pool selection index cache */
	unsigned int cmd_pool_top;		/* Command pool top */
	unsigned char *cmd_pool;		/* Async commands pool */
	/* The following data is for private use only by interrupt routines! */
	gus_wave_t *cur_wave[32];		/* Currently played waves */
	boolean voice_kick[32];			/* Kick wave on next volume ramp IRQ */
	unsigned int kick_offs[32];		/* Sample start position on kick */
	unsigned short cur_vol[32];		/* Current voice volumes */
	unsigned int cur_voice;			/* Current voice */
	unsigned int eow_ignore;		/* Temp ignore end-of-wave IRQ for these voices */
} __gus_state;

extern __gus_state gus;
extern void __gus_delay();

static unsigned long __gus_convert_addr16(unsigned long address)
{
	return ((address & 0x0003ffff) >> 1) | (address & ~0x0003ffff);
}

/* The XXX_slow routines cannot be used outside IRQ handler! */
static inline void __gus_outregb_slow(unsigned char reg, unsigned char value)
{
	outportb(GF1_REGSEL, reg);
	outportb(GF1_DATA_HIGH, value);
	__gus_delay();
	outportb(GF1_DATA_HIGH, value);
}

static inline void __gus_outregw_slow(unsigned char reg, unsigned short value)
{
	outportb(GF1_REGSEL, reg);
	outportw(GF1_DATA, value);
	__gus_delay();
	outportw(GF1_DATA, value);
}

static inline void __gus_outregb(unsigned char reg, unsigned char value)
{
	outportb(GF1_REGSEL, reg);
	outportb(GF1_DATA_HIGH, value);
}

static inline void __gus_outregw(unsigned char reg, unsigned short value)
{
	outportb(GF1_REGSEL, reg);
	outportw(GF1_DATA, value);
}

static inline unsigned char __gus_inregb(unsigned char reg)
{
	if (reg < 0x10)
		reg |= GF1R_READ_MASK;
	outportb(GF1_REGSEL, reg);
	return inportb(GF1_DATA_HIGH);
}

static inline unsigned short __gus_inregw(unsigned char reg)
{
	if (reg < 0x10)
		reg |= GF1R_READ_MASK;
	outportb(GF1_REGSEL, reg);
	return inportw(GF1_DATA);
}

static inline void __gus_set_dram_address(unsigned int address)
{
	__gus_outregb(GF1R_DRAM_HIGH, address >> 16);
	__gus_outregw(GF1R_DRAM_LOW, address);
}

static inline unsigned char __gus_peek(unsigned int address)
{
	__gus_set_dram_address(address);
	return inportb(GF1_DRAM);
}

static inline void __gus_poke(unsigned int address, unsigned char value)
{
	__gus_set_dram_address(address);
	outportb(GF1_DRAM, value);
}

static inline void __gus_select_voice(unsigned char voice)
{
	outportb(GF1_VOICESEL, voice);
}

static inline void __gus_set_current(unsigned char mode,
                                     unsigned long address)
{
	if (mode & GF1VC_DATA16)
		address = __gus_convert_addr16(address);
	__gus_outregw_slow(GF1R_ACC_HIGH, address >> 11);
	__gus_outregw_slow(GF1R_ACC_LOW, address << 5);
}

static inline void __gus_set_loop_start(unsigned char mode,
										unsigned long address)
{
	if (mode & GF1VC_DATA16)
		address = __gus_convert_addr16(address);
	__gus_outregw_slow(GF1R_START_HIGH, address >> 11);
	__gus_outregw_slow(GF1R_START_LOW, address << 5);
}

static inline void __gus_set_loop_end(unsigned char mode,
                                      unsigned long address)
{
	address--;
	if (mode & GF1VC_DATA16)
		address = __gus_convert_addr16(address);
	__gus_outregw_slow(GF1R_END_HIGH, address >> 11);
	__gus_outregw_slow(GF1R_END_LOW, address << 5);
}

static inline void __gus_mixer_output(boolean state)
{
	if (state)
		gus.mixer &= ~GF1M_MIXER_NO_OUTPUT;
	else
		gus.mixer |= GF1M_MIXER_NO_OUTPUT;
	outportb(GF1_MIX_CTRL, gus.mixer);
	/* Dummy read to avoid touching DMA latches */
	__gus_inregb(GF1R_BALANCE);
}

/* Inline routines for working with command pools */

/* WARNING: no bounds checking due to performance reasons */
#define __POOL_VALUE(type,value)								\
  *((unsigned type *)&gus.cmd_pool [gus.cmd_pool_top]) = value;	\
  gus.cmd_pool_top += sizeof (type);

static inline void __pool_command(unsigned char command)
{
	__POOL_VALUE(char, command);
}

static inline void __pool_command_b(unsigned char command, unsigned char arg)
{
	__POOL_VALUE(char, command);
	__POOL_VALUE(char, arg);
}

static inline void __pool_command_w(unsigned char command, unsigned short arg)
{
	__POOL_VALUE(char, command);
	__POOL_VALUE(short, arg);
}

static inline void __pool_command_l(unsigned char command, unsigned long arg)
{
	__POOL_VALUE(char, command);
	__POOL_VALUE(long, arg);
}

static inline void __pool_select_voice(unsigned char voice)
{
	if (gus.cmd_voice != voice)
		__pool_command_b(PCMD_VOICE, gus.cmd_voice = voice);
}

#undef __POOL_VALUE

#ifdef DEBUG
/* Debug dump of GUS DRAM heap */
extern void __gus_mem_dump();
#endif

#endif /* __DOSGUS_H__ */

/* ex:set ts=4: */
