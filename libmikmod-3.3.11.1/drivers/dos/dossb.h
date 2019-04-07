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

  SoundBlaster and compatible soundcards definitions

==============================================================================*/

#ifndef __DOSSB_H__
#define __DOSSB_H__

#include "dosdma.h"
#include "dosirq.h"

#define SB_FM_LEFT_STATUS		(sb.port + 0x00)	/* (r) Left FM status */
#define SB_FM_LEFT_REGSEL		(sb.port + 0x00)	/* (w) Left FM register select */
#define SB_FM_LEFT_DATA			(sb.port + 0x01)	/* (w) Left FM data */
#define SB_FM_RIGHT_STATUS		(sb.port + 0x02)	/* (r) Right FM status */
#define SB_FM_RIGHT_REGSEL		(sb.port + 0x02)	/* (w) Right FM register select */
#define SB_FM_RIGHT_DATA		(sb.port + 0x03)	/* (w) Right FM data */
#define SB_MIXER_REGSEL			(sb.port + 0x04)	/* (w) Mixer register select */
#define SB_MIXER_DATA			(sb.port + 0x05)	/* (rw)Mixer data */
#define SB_DSP_RESET			(sb.port + 0x06)	/* (w) DSP reset */
#define SB_FM_STATUS			(sb.port + 0x08)	/* (r) FM status */
#define SB_FM_REGSEL			(sb.port + 0x08)	/* (w) FM register select */
#define SB_FM_DATA			(sb.port + 0x09)	/* (w) FM data */
#define SB_DSP_DATA_IN			(sb.port + 0x0a)	/* (r) DSP data input */
#define SB_DSP_DATA_OUT			(sb.port + 0x0c)	/* (w) DSP data output */
#define SB_DSP_DATA_OUT_STATUS		(sb.port + 0x0c)	/* (r) DSP data output status */
#define SB_DSP_TIMER_IRQ		(sb.port + 0x0d)	/* (r) clear timer IRQ? */
#define SB_DSP_DATA_IN_STATUS		(sb.port + 0x0e)	/* (r) DSP data input status */
#define SB_DSP_DMA8_IRQ			(sb.port + 0x0e)	/* (r) Acknowledge 8-bit DMA transfer */
#define SB_DSP_DMA16_IRQ		(sb.port + 0x0f)	/* (r) Acknowledge 16-bit DMA transfer */

/* DSP commands */
#define SBDSP_ASP_STATUS		0x03	/* ASP Status (SB16ASP) */
#define SBDSP_STATUS_OLD		0x04	/* DSP Status (Obsolete) (SB2.0-Pro2) */
#define SBDSP_DIRECT_DAC		0x10	/* Direct DAC, 8-bit (SB) */
#define SBDSP_DMA_PCM8			0x14	/* DMA DAC, 8-bit (SB) */
#define SBDSP_DMA_ADPCM2		0x16	/* DMA DAC, 2-bit ADPCM (SB) */
#define SBDSP_DMA_ADPCM2R		0x17	/* DMA DAC, 2-bit ADPCM Reference (SB) */
#define SBDSP_DMA_PCM8_AUTO		0x1C	/* Auto-Initialize DMA DAC, 8-bit (SB2.0) */
#define SBDSP_DMA_ADPCM2R_AUTO		0x1F	/* Auto-Initialize DMA DAC, 2-bit ADPCM Reference (SB2.0) */
#define SBDSP_DIRECT_ADC		0x20	/* Direct ADC, 8-bit (SB) */
#define SBDSP_DMA_ADC8			0x24	/* DMA ADC, 8-bit (SB) */
#define SBDSP_DIRECT_ADC8_BURST		0x28	/* Direct ADC, 8-bit (Burst) (SB-Pro2) */
#define SBDSP_DMA_ADC8_AUTO		0x2C	/* Auto-Initialize DMA ADC, 8-bit (SB2.0) */
#define SBDSP_MIDI_READ_POLL		0x30	/* MIDI Read Poll (SB) */
#define SBDSP_MIDI_READ_IRQ		0x31	/* MIDI Read Interrupt (SB) */
#define SBDSP_MIDI_READ_TIME		0x32	/* MIDI Read Timestamp Poll (SB???) */
#define SBDSP_MIDI_READ_TIME_IRQ	0x33	/* MIDI Read Timestamp Interrupt (SB???) */
#define SBDSP_MIDI_RW_POLL		0x34	/* MIDI Read Poll + Write Poll (UART) (SB2.0) */
#define SBDSP_MIDI_RW_IRQ		0x35	/* MIDI Read Interrupt + Write Poll (UART) (SB2.0???) */
#define SBDSP_MIDI_RW_TIME_IRQ		0x37	/* MIDI Read Timestamp Interrupt + Write Poll (UART) (SB2.0???) */
#define SBDSP_MIDI_WRITE_POLL		0x38	/* MIDI Write Poll (SB) */
#define SBDSP_SET_TIMING		0x40	/* Set Time Constant (SB) */
#define SBDSP_SET_RATE			0x41	/* Set Sample Rate, Hz (SB16) */
#define SBDSP_DMA_CONT8_AUTO		0x45	/* Continue Auto-Initialize DMA, 8-bit (SB16) */
#define SBDSP_DMA_CONT16_AUTO		0x47	/* Continue Auto-Initialize DMA, 16-bit (SB16) */
#define SBDSP_SET_DMA_BLOCK		0x48	/* Set DMA Block Size (SB2.0) */
#define SBDSP_DMA_ADPCM4		0x74	/* DMA DAC, 4-bit ADPCM (SB) */
#define SBDSP_DMA_ADPCM4_REF		0x75	/* DMA DAC, 4-bit ADPCM Reference (SB) */
#define SBDSP_DMA_ADPCM26		0x76	/* DMA DAC, 2.6-bit ADPCM (SB) */
#define SBDSP_DMA_ADPCM26_REF		0x77	/* DMA DAC, 2.6-bit ADPCM Reference (SB) */
#define SBDSP_DMA_ADPCM4R_AUTO		0x7D	/* Auto-Initialize DMA DAC, 4-bit ADPCM Reference (SB2.0) */
#define SBDSP_DMA_ADPCM26R_AUTO		0x7F	/* Auto-Initialize DMA DAC, 2.6-bit ADPCM Reference (SB2.0) */
#define SBDSP_DISABLE_DAC		0x80	/* Silence DAC (SB) */
#define SBDSP_HS_DMA_DAC8_AUTO		0x90	/* Auto-Initialize DMA DAC, 8-bit (High Speed) (SB2.0-Pro2) */
#define SBDSP_HS_DMA_ADC8_AUTO		0x98	/* Auto-Initialize DMA ADC, 8-bit (High Speed) (SB2.0-Pro2) */
#define SBDSP_STEREO_ADC_DIS		0xA0	/* Disable Stereo Input Mode (SBPro Only) */
#define SBDSP_STEREO_ADC_ENA		0xA8	/* Enable Stereo Input Mode (SBPro Only) */
#define SBDSP_DMA_GENERIC16		0xB0	/* Generic DAC/ADC DMA (16-bit) (SB16) */
#define SBDSP_DMA_GENERIC8		0xC0	/* Generic DAC/ADC DMA (8-bit) (SB16) */
#define SBDSP_DMA_HALT8			0xD0	/* Halt DMA Operation, 8-bit (SB) */
#define SBDSP_SPEAKER_ENA		0xD1	/* Enable Speaker (SB) */
#define SBDSP_SPEAKER_DIS		0xD3	/* Disable Speaker (SB) */
#define SBDSP_DMA_CONT8			0xD4	/* Continue DMA Operation, 8-bit (SB) */
#define SBDSP_DMA_HALT16		0xD5	/* Halt DMA Operation, 16-bit (SB16) */
#define SBDSP_DMA_CONT16		0xD6	/* Continue DMA Operation, 16-bit (SB16) */
#define SBDSP_SPEAKER_STATUS		0xD8	/* Speaker Status (SB) */
#define SBDSP_DMA_EXIT16_AUTO		0xD9	/* Exit Auto-Initialize DMA Operation, 16-bit (SB16) */
#define SBDSP_DMA_EXIT8_AUTO		0xDA	/* Exit Auto-Initialize DMA Operation, 8-bit (SB2.0) */
#define SBDSP_IDENTIFY			0xE0	/* DSP Identification (SB2.0) */
#define SBDSP_VERSION			0xE1	/* DSP Version (SB) */
#define SBDSP_COPYRIGHT			0xE3	/* DSP Copyright (SBPro2???) */
#define SBDSP_WRITE_TEST		0xE4	/* Write Test Register (SB2.0) */
#define SBDSP_READ_TEST			0xE8	/* Read Test Register (SB2.0) */
#define SBDSP_SINE_GEN			0xF0	/* Sine Generator (SB) */
#define SBDSP_AUX_STATUS_PRO		0xF1	/* DSP Auxiliary Status (Obsolete) (SB-Pro2) */
#define SBDSP_GEN_IRQ8			0xF2	/* IRQ Request, 8-bit (SB) */
#define SBDSP_GEN_IRQ16			0xF3	/* IRQ Request, 16-bit (SB16) */
#define SBDSP_STATUS			0xFB	/* DSP Status (SB16) */
#define SBDSP_AUX_STATUS_16		0xFC	/* DSP Auxiliary Status (SB16) */
#define SBDSP_CMD_STATUS		0xFD	/* DSP Command Status (SB16) */

/* Mixer commands */
#define SBMIX_RESET			0x00	/* Reset                        Write       SBPro */
#define SBMIX_STATUS			0x01	/* Status                       Read        SBPro */
#define SBMIX_MASTER_LEVEL1		0x02	/* Master Volume                Read/Write  SBPro Only */
#define SBMIX_DAC_LEVEL			0x04	/* DAC Level                    Read/Write  SBPro */
#define SBMIX_FM_OUTPUT			0x06	/* FM Output Control            Read/Write  SBPro Only */
#define SBMIX_MIC_LEVEL			0x0A	/* Microphone Level             Read/Write  SBPro */
#define SBMIX_INPUT_SELECT		0x0C	/* Input/Filter Select          Read/Write  SBPro Only */
#define SBMIX_OUTPUT_SELECT		0x0E	/* Output/Stereo Select         Read/Write  SBPro Only */
#define SBMIX_FM_LEVEL			0x22	/* Master Volume                Read/Write  SBPro */
#define SBMIX_MASTER_LEVEL		0x26	/* FM Level                     Read/Write  SBPro */
#define SBMIX_CD_LEVEL			0x28	/* CD Audio Level               Read/Write  SBPro */
#define SBMIX_LINEIN_LEVEL		0x2E	/* Line In Level                Read/Write  SBPro */
#define SBMIX_MASTER_LEVEL_L		0x30	/* Master Volume Left           Read/Write  SB16 */
#define SBMIX_MASTER_LEVEL_R		0x31	/* Master Volume Right          Read/Write  SB16 */
#define SBMIX_DAC_LEVEL_L		0x32	/* DAC Level Left               Read/Write  SB16 */
#define SBMIX_DAC_LEVEL_R		0x33	/* DAC Level Right              Read/Write  SB16 */
#define SBMIX_FM_LEVEL_L		0x34	/* FM Level Left                Read/Write  SB16 */
#define SBMIX_FM_LEVEL_R		0x35	/* FM Level Right               Read/Write  SB16 */
#define SBMIX_CD_LEVEL_L		0x36	/* CD Audio Level Left          Read/Write  SB16 */
#define SBMIX_CD_LEVEL_R		0x37	/* CD Audio Level Right         Read/Write  SB16 */
#define SBMIX_LINEIN_LEVEL_L		0x38	/* Line In Level Left           Read/Write  SB16 */
#define SBMIX_LINEIN_LEVEL_R		0x39	/* Line In Level Right          Read/Write  SB16 */
#define SBMIX_MIC_LEVEL_16		0x3A	/* Microphone Level             Read/Write  SB16 */
#define SBMIX_PCSPK_LEVEL		0x3B	/* PC Speaker Level             Read/Write  SB16 */
#define SBMIX_OUTPUT_CONTROL		0x3C	/* Output Control               Read/Write  SB16 */
#define SBMIX_INPUT_CONTROL_L		0x3D	/* Input Control Left           Read/Write  SB16 */
#define SBMIX_INPUT_CONTROL_R		0x3E	/* Input Control Right          Read/Write  SB16 */
#define SBMIX_INPUT_GAIN_L		0x3F	/* Input Gain Control Left      Read/Write  SB16 */
#define SBMIX_INPUT_GAIN_R		0x40	/* Input Gain Control Right     Read/Write  SB16 */
#define SBMIX_OUTPUT_GAIN_L		0x41	/* Output Gain Control Left     Read/Write  SB16 */
#define SBMIX_OUTPUT_GAIN_R		0x42	/* Output Gain Control Right    Read/Write  SB16 */
#define SBMIX_AGC_CONTROL		0x43	/* Automatic Gain Control (AGC) Read/Write  SB16 */
#define SBMIX_TREBLE_L			0x44	/* Treble Left                  Read/Write  SB16 */
#define SBMIX_TREBLE_R			0x45	/* Treble Right                 Read/Write  SB16 */
#define SBMIX_BASS_L			0x46	/* Bass Left                    Read/Write  SB16 */
#define SBMIX_BASS_R			0x47	/* Bass Right                   Read/Write  SB16 */
#define SBMIX_IRQ_SELECT		0x80	/* IRQ Select                   Read/Write  SB16 */
#define SBMIX_DMA_SELECT		0x81	/* DMA Select                   Read/Write  SB16 */
#define SBMIX_IRQ_STATUS		0x82	/* IRQ Status                   Read        SB16 */

/* SB_DSP_DATA_OUT_STATUS and SB_DSP_DATA_IN_STATUS bits */
#define SBM_DSP_READY			0x80

/* SB_DSP_RESET / SBMIX_RESET */
#define SBM_DSP_RESET			0x01

/* SBMIX_OUTPUT_SELECT */
#define SBM_MIX_STEREO			0x02
#define SBM_MIX_FILTER			0x20

/* SBDSP_DMA_GENERIC16/SBDSP_DMA_GENERIC8 */
#define SBM_GENDAC_FIFO			0x02
#define SBM_GENDAC_AUTOINIT		0x04
#define SBM_GENDAC_ADC			0x08
/* Second (mode) byte */
#define SBM_GENDAC_SIGNED		0x10
#define SBM_GENDAC_STEREO		0x20

/* DSP version masks */
#define SBVER_10			0x0100	/* Original SoundBlaster */
#define SBVER_15			0x0105	/* SoundBlaster 1.5 */
#define SBVER_20			0x0200	/* SoundBlaster 2.0 */
#define SBVER_PRO			0x0300	/* SoundBlaster Pro */
#define SBVER_PRO2			0x0301	/* SoundBlaster Pro 2 */
#define SBVER_16			0x0400	/* SoundBlaster 16 */
#define SBVER_AWE32			0x040c	/* SoundBlaster AWE32 */

typedef unsigned char boolean;

#ifndef FALSE
#define FALSE				0
#define TRUE				1
#endif

/* Play mode bits */
#define SBMODE_16BITS			0x0001
#define SBMODE_STEREO			0x0002
#define SBMODE_SIGNED			0x0004

/* Mask for capabilities that never change */
#define SBMODE_MASK			(SBMODE_16BITS | SBMODE_STEREO)

/* You can fill some members of this struct (i.e. port,irq,dma) before
 * calling sb_detect() or sb_open()... this will ignore environment settings.
 */
typedef struct __sb_state_s {
	boolean ok;			/* Are structure contents valid? */
	int port, aweport;		/* sb/awe32 base port */
	int irq;			/* SoundBlaster IRQ */
	int dma8, dma16;		/* 8-bit and 16-bit DMAs */
	int maxfreq_mono;		/* Maximum discretization frequency / mono mode */
	int maxfreq_stereo;		/* Maximum discretization frequency / stereo mode */
	unsigned short dspver;		/* DSP version number */
	struct irq_handle *irq_handle;	/* The interrupt handler */
	dma_buffer *dma_buff;		/* Pre-allocated DMA buffer */
	unsigned char caps;		/* SoundBlaster capabilities (SBMODE_XXX) */
	unsigned char mode;		/* Current SB mode (SBMODE_XXX) */
	boolean open;			/* Whenever the card has been opened */
	volatile int irqcount;		/* Incremented on each IRQ... for diagnostics */
	void (*timer_callback) ();	/* Called TWICE per buffer play */
} __sb_state;

extern __sb_state sb;

extern void __sb_wait();

static inline boolean __sb_dsp_ready_in()
{
	int count;
	for (count = 10000; count >= 0; count--)
		if (inportb(SB_DSP_DATA_IN_STATUS) & SBM_DSP_READY)
			return TRUE;
	return FALSE;
}

static inline boolean __sb_dsp_ready_out()
{
	int count;
	for (count = 10000; count >= 0; count--)
		if ((inportb(SB_DSP_DATA_OUT_STATUS) & SBM_DSP_READY) == 0)
			return TRUE;
	return FALSE;
}

static inline void __sb_dsp_out(unsigned char reg)
{
	__sb_dsp_ready_out();
	outportb(SB_DSP_DATA_OUT, reg);
}

static inline unsigned char __sb_dsp_in()
{
	__sb_dsp_ready_in();
	return inportb(SB_DSP_DATA_IN);
}

static inline void __sb_dspreg_out(unsigned char reg, unsigned char val)
{
	__sb_dsp_out(reg);
	__sb_dsp_out(val);
}

static inline void __sb_dspreg_outwlh(unsigned char reg, unsigned short val)
{
	__sb_dsp_out(reg);
	__sb_dsp_out(val);
	__sb_dsp_out(val >> 8);
}

static inline void __sb_dspreg_outwhl(unsigned char reg, unsigned short val)
{
	__sb_dsp_out(reg);
	__sb_dsp_out(val >> 8);
	__sb_dsp_out(val);
}

static inline unsigned char __sb_dspreg_in(unsigned char reg)
{
	__sb_dsp_out(reg);
	return __sb_dsp_in();
}

static inline void __sb_dsp_ack_dma8()
{
	inportb(SB_DSP_DMA8_IRQ);
}

static inline void __sb_dsp_ack_dma16()
{
	inportb(SB_DSP_DMA16_IRQ);
}

static inline unsigned short __sb_dsp_version()
{
	unsigned short ver;
	__sb_dsp_out(SBDSP_VERSION);
	__sb_dsp_ready_in();
	ver = ((unsigned short)__sb_dsp_in()) << 8;
	ver |= __sb_dsp_in();
	return ver;
}

static inline void __sb_mixer_out(unsigned char reg, unsigned char val)
{
	outportb(SB_MIXER_REGSEL, reg);
	outportb(SB_MIXER_DATA, val);
}

static inline unsigned char __sb_mixer_in(unsigned char reg)
{
	outportb(SB_MIXER_REGSEL, reg);
	return inportb(SB_MIXER_DATA);
}

/* Enable stereo transfers: sbpro mode only */
static inline void __sb_stereo(boolean stereo)
{
	unsigned char val = __sb_mixer_in(SBMIX_OUTPUT_SELECT);
	if (stereo)
		val |= SBM_MIX_STEREO;
	else
		val &= ~SBM_MIX_STEREO;
	__sb_mixer_out(SBMIX_OUTPUT_SELECT, val);
}

/* Detect whenever SoundBlaster is present and fill "sb" structure */
extern boolean sb_detect();
/* Reset SoundBlaster */
extern void sb_reset();
/* Start working with SoundBlaster */
extern boolean sb_open();
/* Finish working with SoundBlaster */
extern boolean sb_close();
/* Enable/disable speaker output */
extern void sb_output(boolean enable);
/* Start playing from DMA buffer in either 8/16 bit mono/stereo */
extern boolean sb_start_dma(unsigned char mode, unsigned int freq);
/* Stop playing from DMA buffer */
extern void sb_stop_dma();
/* Query current position/total size of the DMA buffer */
extern void sb_query_dma(unsigned int *dma_size, unsigned int *dma_pos);

#endif /* __DOSSB_H__ */

/* ex:set ts=4: */
