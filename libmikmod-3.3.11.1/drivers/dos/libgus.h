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

  Linux libGUS-alike library for DOS, used by drv_ultra.c under DOS.

==============================================================================*/

/*
	Current limitations:
	- Only a subset of libgus is supported
	- Only one GUS card is supported (due to the fact that ULTRASND environment
	  variable is used)
	- No Interwawe support (if IW works the old way, it's ok).
*/

#ifndef __LIBGUS_H__
#define __LIBGUS_H__

#include <stddef.h>

#define __LITTLE_ENDIAN

typedef struct _gus_info_t gus_info_t;
typedef struct _gus_instrument_t gus_instrument_t;
typedef struct _gus_wave_t gus_wave_t;
typedef struct _gus_layer_t gus_layer_t;

#define GUS_CARD_VERSION_CLASSIC	0x0024	/* revision 2.4 */
#define GUS_CARD_VERSION_CLASSIC1	0x0034	/* revision 3.4? */
#define GUS_CARD_VERSION_CLASSIC_ICS	0x0037	/* revision 3.7 (ICS mixer) */
#define GUS_CARD_VERSION_EXTREME	0x0050	/* GUS Extreme */
#define GUS_CARD_VERSION_ACE		0x0090	/* GUS ACE */
#define GUS_CARD_VERSION_MAX		0x00a0	/* GUS MAX - revision 10 */
#define GUS_CARD_VERSION_MAX1		0x00a1	/* GUS MAX - revision 11 */
#define GUS_CARD_VERSION_PNP		0x0100	/* GUS Plug & Play */

#define GUS_STRU_INFO_F_DB16		0x00000001	/* 16-bit daughter board present */
#define GUS_STRU_INFO_F_PCM		0x00000004	/* GF1 PCM during SYNTH enabled */
#define GUS_STRU_INFO_F_ENHANCED	0x00000008	/* InterWave - enhanced mode */
#define GUS_STRU_INFO_F_DAEMON		0x00000010	/* instrument daemon is present */

struct _gus_info_t {
	unsigned char id[8];		/* id of this card (warning! maybe unterminated!!!) */

	unsigned int flags;		/* some info flags - see to GUS_STRU_INFO_F_XXXX */
	unsigned int version;		/* see to GUS_CARD_VERSION_XXXX constants */

	unsigned short port;
	unsigned short irq;
	unsigned short dma1;		/* DMA1 - GF1 download & codec record */
	unsigned short dma2;		/* DMA2 - GF1 record & codec playback */

	unsigned int mixing_freq;	/* mixing frequency in Hz */

	unsigned int memory_size;	/* in bytes */
	unsigned int memory_free;	/* in bytes */
	unsigned int memory_block_8;	/* largest free 8-bit block in memory */
	unsigned int memory_block_16;	/* largest free 16-bit block in memory */
};

/* struct gus_instrument_t - mode */

#define GUS_INSTR_SIMPLE	0x00	/* simple format - for MOD players */
#define GUS_INSTR_PATCH		0x01	/* old GF1 patch format */
#define GUS_INSTR_COUNT		2

#define GUS_INSTR_F_NORMAL	0x0000	/* normal mode */
#define GUS_INSTR_F_NOT_FOUND	0x0001	/* instrument can't be loaded */
#define GUS_INSTR_F_ALIAS	0x0002	/* alias */
#define GUS_INSTR_F_NOT_LOADED	0x00ff	/* instrument not loaded (not found) */

#define GUS_INSTR_E_NONE	0x0000	/* exclusion mode - none */
#define GUS_INSTR_E_SINGLE	0x0001	/* exclude single - single note from this instrument */
#define GUS_INSTR_E_MULTIPLE	0x0002	/* exclude multiple - stop only same note from this instrument */

#define GUS_INSTR_L_NONE	0x0000	/* not layered */
#define GUS_INSTR_L_ON		0x0001	/* layered */
#define GUS_INSTR_L_VELOCITY	0x0002	/* layered by velocity */
#define GUS_INSTR_L_FREQUENCY	0x0003	/* layered by frequency */

struct _gus_instrument_t {
	union {
		unsigned int instrument;/* instrument number */
	} number;

	char *name;			/* name of this instrument or NULL */

	unsigned int mode:8,		/* see to GUS_INSTR_XXXX */
	    flags:8,			/* see to GUS_INSTR_F_XXXX */
	    exclusion:4,		/* see to GUS_INSTR_E_XXXX */
	    layer:4;			/* see to GUS_INSTR_L_XXXX */
	unsigned short exclusion_group;	/* 0 - none, 1-65535 */

	struct {
		unsigned char effect1:4,/* use global effect if available */
		    effect2:4;		/* use global effect if available */
		unsigned char effect1_depth;/* 0-127 */
		unsigned char effect2_depth;/* 0-127 */
	} patch;

	union {
		gus_layer_t *layer;	/* first layer */
		unsigned int alias;	/* pointer to instrument */
	} info;
	gus_instrument_t *next;		/* next instrument */
};

struct _gus_layer_t {
	unsigned char mode;		/* see to GUS_INSTR_XXXX constants */

	gus_wave_t *wave;
	gus_layer_t *next;
};

/* bits for format variable in gus_wave_t */

#define GUS_WAVE_16BIT          0x0001	/* 16-bit wave */
#define GUS_WAVE_UNSIGNED       0x0002	/* unsigned wave */
#define GUS_WAVE_INVERT         0x0002	/* same as unsigned wave */
#define GUS_WAVE_BACKWARD       0x0004	/* forward mode */
#define GUS_WAVE_LOOP           0x0008	/* loop mode */
#define GUS_WAVE_BIDIR          0x0010	/* bidirectional mode */
#define GUS_WAVE_ULAW           0x0020	/* uLaw compressed wave */
#define GUS_WAVE_RAM            0x0040	/* wave is _preloaded_ in RAM (it is used for ROM simulation) */
#define GUS_WAVE_ROM            0x0080	/* wave is in ROM */
#define GUS_WAVE_DELTA          0x0100

#define GUS_WAVE_PATCH_ENVELOPE 0x01	/* envelopes on */
#define GUS_WAVE_PATCH_SUSTAIN  0x02	/* sustain mode */

struct _gus_wave_t {
	unsigned char mode;		/* see to GUS_INSTR_XXXX constants */
	unsigned char format;		/* see to GUS_WAVE_XXXX constants */
	unsigned int size;		/* size of waveform in bytes */
	unsigned int start;		/* start offset in bytes * 16 (lowest 4 bits - fraction) */
	unsigned int loop_start;	/* bits loop start offset in bytes * 16 (lowest 4 bits - fraction) */
	unsigned int loop_end;		/* loop start offset in bytes * 16 (lowest 4 bits - fraction) */
	unsigned short loop_repeat;	/* loop repeat - 0 = forever */
	struct {
		unsigned int memory;	/* begin of waveform in GUS's memory */
		unsigned char *ptr;	/* pointer to waveform in system memory */
	} begin;

	struct {
		unsigned char flags;
		unsigned int sample_rate;
		unsigned int low_frequency;/* low frequency range for this waveform */
		unsigned int high_frequency;/* high frequency range for this waveform */
		unsigned int root_frequency;/* root frequency for this waveform */
		signed short tune;
		unsigned char balance;
		unsigned char envelope_rate[6];
		unsigned char envelope_offset[6];
		unsigned char tremolo_sweep;
		unsigned char tremolo_rate;
		unsigned char tremolo_depth;
		unsigned char vibrato_sweep;
		unsigned char vibrato_rate;
		unsigned char vibrato_depth;
		unsigned short scale_frequency;
		unsigned short scale_factor;/* 0-2048 or 0-2 */
	} patch;

	gus_wave_t *next;
};

/* defines for gus_memory_reset () */
#define GUS_DOWNLOAD_MODE_NORMAL 0x0000
#define GUS_DOWNLOAD_MODE_TEST	 0x0001

/*
    A subset of libgus functions (used by MikMod Ultrasound driver)
*/
int gus_cards(void);
  /*
   * return value:      number of GUS cards installed in system or
   *                    zero if driver isn't installed
   */
int gus_close(int card);
  /*
   * close file (gus synthesizer) previously opened with gusOpen function
   * return value:      zero if success
   */
int gus_do_flush(void);
  /*
   * return value:      zero if command queue was successfully flushed
   *                    in non block mode - number of written bytes
   */
void gus_do_tempo(unsigned int tempo);
  /*
   * set new tempo
   */
void gus_do_voice_frequency(unsigned char voice, unsigned int freq);
  /*
   * set voice frequency in Hz
   */
void gus_do_voice_pan(unsigned char voice, unsigned short pan);
  /*
   * set voice pan (0-16384) (full left - full right)
   */
void gus_do_voice_start(unsigned char voice, unsigned int program,
                        unsigned int freq, unsigned short volume,
                        unsigned short pan);
  /*
   * start voice
   *            voice    : voice #
   *            program  : program # or ~0 = current
   *            freq     : frequency in Hz
   *            volume   : volume level (0-16384) or ~0 = current
   *            pan      : pan level (0-16384) or ~0 = current
   */
void gus_do_voice_start_position(unsigned char voice, unsigned int program,
                                 unsigned int freq, unsigned short volume,
                                 unsigned short pan, unsigned int position);
  /*
   * start voice
   *            voice    : voice #
   *            program  : program # or ~0 = current
   *            freq     : frequency in Hz
   *            volume   : volume level (0-16384) or ~0 = current
   *            pan      : pan level (0-16384) or ~0 = current
   *            position : offset to wave in bytes * 16 (lowest 4 bits - fraction)
   */
void gus_do_voice_stop(unsigned char voice, unsigned char mode);
  /*
   * stop voice
   *            mode = 0 : stop voice now
   *            mode = 1 : disable wave loop and finish it
   */
void gus_do_voice_volume(unsigned char voice, unsigned short vol);
  /*
   * set voice volume level 0-16384 (linear)
   */
void gus_do_wait(unsigned int ticks);
  /*
   * wait x ticks - this command is block separator
   * all commands between blocks are interpreted in the begining of one tick
   */
int gus_get_voice_status(int voice);
  /*
   * THIS IS NOT A FUNCTION OF ORIGINAL libGUS!
   * Return voice status: -1 on error, 0 if voice stopped, 1 if playing
   */
int gus_get_handle(void);
  /*
   * return value:      file handle (descriptor) for /dev/gus
   */
int gus_info(gus_info_t * info, int reread);
  /*
   * return value:      filled info variable with actual values
   *                    (look at gus.h header file for more informations)
   * version field:     0x0024  - GUS revision 2.4
   *                    0x0035  - GUS revision 3.7 with flipped mixer channels
   *                    0x0037  - GUS revision 3.7
   *                    0x0090  - GUS ACE
   *                    0x00a0  - GUS MAX revision 10
   *                    0x00a1  - GUS MAX revision 11
   *                    0x0100  - InterWave (full version)
   * flags field:       see to GUS_STRU_INFO_F_???? constants (gus.h header file)
   * port field:        port number (for example 0x220)
   * irq field:         irq number (for example 11)
   * dma1 field:        dma1 number (for example 5)
   * dma2 field:        dma2 number (for example 6)
   * note:              dma1 and dma2 could be same in case of only one dma channel used
   */
int gus_memory_alloc(gus_instrument_t * instrument);
  /*
   * input value:       look at gus.h for more details about gus_instrument_t structure
   * return value:      zero if instrument was successfully allocated
   */
int gus_memory_free(gus_instrument_t * instrument);
  /*
   * input value:       look at gus.h for more details about gus_instrument_t structure
   * return value:      zero if instrument was successfully removed
   */
int gus_memory_size(void);
  /*
   * return value:  gus memory size in bytes
   */
int gus_memory_free_size(void);
  /*
   * return value:      unused gus memory in bytes
   * warning:           reset function must be called before
   */
int gus_memory_free_block(int w_16bit);
  /*
   * return value:  current largest free block for 8-bit or 16-bit wave
   */
int gus_memory_pack(void);
  /*
   * return value:      zero if success
   */
int gus_memory_reset(int mode);
  /*
   * input value:   see to GUS_DOWNLOAD_MODE_XXXX constants (gus.h)
   * return value:  zero if samples & instruments was successfully removed
   *            from GF1 memory manager
   */

int gus_open(int card, size_t queue_buffer_size, int non_block);
  /*
   * input values:      card number,
   *                    size of command queue buffer (512-1MB)
   *                    buffer is allocated dynamically,
   *                    non block mode
   * return value:      zero if success
   * note 1:            this function must be called as first
   *                    open file /dev/gus
   * note 2:            you can open more cards with one process
   */
int gus_queue_flush(void);
  /*
   * return value:      zero if command queue was successfully flushed
   */
int gus_queue_read_set_size(int items);
  /*
   * input value:       echo buffer size in items (if 0 - erase echo buffer)
   */
int gus_queue_write_set_size(int items);
  /*
   * input value:       write queue size in items (each item have 8 bytes)
   */
int gus_reset(int voices, unsigned int channel_voices);
  /*
   * input values:      active voices and channel voices (for dynamic allocation)
   * return value:      number of active voices if reset was successfull (GF1 chip active)
   */
int gus_reset_engine_only(void);
  /*
   * return value:  same as gus_reset function
   * note:      this command doesn't change number of active
   *            voices and doesn't do hardware reset
   */
int gus_select(int card);
  /*
   * select specified card
   * return value:      zero if success
   */
int gus_timer_start(void);
  /*
   * return value:      zero if successfull
   */
int gus_timer_stop(void);
  /*
   * return value:      zero if timer was stoped
   */
int gus_timer_continue(void);
  /*
   * return value:  zero if timer will be continue
   */
int gus_timer_tempo(int ticks);
  /*
   * return value:      zero if setup was success
   */
int gus_timer_base(int base);
  /*
   * return value:  zero if setup was success (default timebase = 100)
   */

void gus_convert_delta(unsigned int type, unsigned char *dest,
                       unsigned char *src, size_t size);
  /*
   * note: dest and src pointers can be equal
   */

void gus_timer_callback(void (*timer_callback) ());
  /*
   * Set a callback to be called once per tempo tick
   */

int gus_dma_usage (int use);
  /*
   * Tell GUS library to use/to not use DMA for sample transfer.
   * In some environments/on some hardware platforms you will need
   * to disable DMA in order to function properly. You should call
   * this function before opening the card.
   */

#endif /* __LIBGUS_H__ */

/* ex:set ts=4: */
