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

  Driver for Gravis Ultrasound cards using libGUS.
  A subset of libGUS is provided for DOS/DJGPP and OS/2

==============================================================================*/

/*

	Written by Andy Lo A Foe <andy@alsa-project.org>

	Updated to work with later versions of both the ultrasound driver and
	libmikmod by C. Ray C. <crayc@pyro.net>

	Major fixes by Andrew Zabolotny <bit@eltech.ru>
	+ Ported to OS/2 and DOS.
	+ Eight-bit samples are not converted to 16-bit anymore.
	+ Samples are no longer kept in normal memory.
	+ Removed sample 'unclick' logic... libGUS does unclick internally.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_ULTRA

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif

#ifdef MIKMOD_DYNAMIC
#include <dlfcn.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <libgus.h>

#if !defined(GUS_INSTR_SIMPLE) || !defined(GUS_WAVE_BIDIR)
#error libgus version is too old
#endif
/* just in case */
#ifndef LIBGUS_VERSION_MAJOR
#define LIBGUS_VERSION_MAJOR 0x0003
#endif

/* DOS/DJGPP and OS/2 libGUS'es have gus_get_voice_status() */
#if defined(__EMX__) || defined(__DJGPP__)
#define HAVE_VOICE_STATUS
#else
#include <time.h>
#endif


#ifdef MIKMOD_DYNAMIC
/* runtime link with libgus */
static int (*_libgus_cards) (void);

#if LIBGUS_VERSION_MAJOR < 0x0004
static int (*_libgus_close) (int);
static int (*_libgus_do_flush) (void);
static void (*_libgus_do_tempo) (unsigned int);
static void (*_libgus_do_voice_frequency) (unsigned char, unsigned int);
static void (*_libgus_do_voice_pan) (unsigned char, unsigned short);
static void (*_libgus_do_voice_start) (unsigned char, unsigned int,
					unsigned int, unsigned short,
					unsigned short);
static void (*_libgus_do_voice_start_position) (unsigned char, unsigned int,
						unsigned int, unsigned short,
						unsigned short, unsigned int);
static void (*_libgus_do_voice_stop) (unsigned char, unsigned char);
static void (*_libgus_do_voice_volume) (unsigned char, unsigned short);
static void (*_libgus_do_wait) (unsigned int);
static int (*_libgus_get_handle) (void);
static int (*_libgus_info) (gus_info_t *, int);
static int (*_libgus_memory_alloc) (gus_instrument_t *);
static int (*_libgus_memory_free) (gus_instrument_t *);
static int (*_libgus_memory_free_size) (void);
static int (*_libgus_memory_pack) (void);
static int (*_libgus_open) (int, size_t, int);
static int (*_libgus_queue_flush) (void);
static int (*_libgus_queue_read_set_size) (int);
static int (*_libgus_queue_write_set_size) (int);
static int (*_libgus_reset) (int, unsigned int);
static int (*_libgus_select) (int);
static int (*_libgus_timer_start) (void);
static int (*_libgus_timer_stop) (void);
static int (*_libgus_timer_tempo) (int);
#else
static int (*_libgus_close) (void*);
static int (*_libgus_do_flush) (void*);
static void (*_libgus_do_tempo) (void*, unsigned int);
static void (*_libgus_do_voice_frequency) (void*, unsigned char, unsigned int);
static void (*_libgus_do_voice_pan) (void*, unsigned char,unsigned short);
static void (*_libgus_do_voice_start) (void*, unsigned char, unsigned int,
					unsigned int, unsigned short,
					unsigned short);
static void (*_libgus_do_voice_start_position) (void*, unsigned char, unsigned int,
						unsigned int,unsigned short,
						unsigned short, unsigned int);
static void (*_libgus_do_voice_stop) (void*, unsigned char, unsigned char);
static void (*_libgus_do_voice_volume) (void*, unsigned char, unsigned short);
static void (*_libgus_do_wait) (void*, unsigned int);
static int (*_libgus_get_file_descriptor) (void*);
static int (*_libgus_info) (void*, gus_info_t*, int);
static int (*_libgus_memory_alloc) (void*, gus_instrument_t*);
static int (*_libgus_memory_free) (void*, gus_instrument_t*);
static int (*_libgus_memory_free_size) (void*);
static int (*_libgus_memory_pack) (void*);
static int (*_libgus_open) (void**, int, int, size_t, int);
static int (*_libgus_queue_flush) (void*);
static int (*_libgus_queue_read_set_size) (void*, int);
static int (*_libgus_queue_write_set_size) (void*, int);
static int (*_libgus_reset) (void*, int, unsigned int);
static int (*_libgus_timer_start)(void*);
static int (*_libgus_timer_stop) (void*);
static int (*_libgus_timer_tempo) (void*, int);
#endif
#ifndef HAVE_RTLD_GLOBAL
#define RTLD_GLOBAL (0)
#endif
static void *libgus = NULL;

#else
/* compile-time link with libgus */
#define _libgus_cards				gus_cards
#define _libgus_close				gus_close
#define _libgus_do_flush			gus_do_flush
#define _libgus_do_tempo			gus_do_tempo
#define _libgus_do_voice_frequency		gus_do_voice_frequency
#define _libgus_do_voice_pan			gus_do_voice_pan
#define _libgus_do_voice_start			gus_do_voice_start
#define _libgus_do_voice_start_position		gus_do_voice_start_position
#define _libgus_do_voice_stop			gus_do_voice_stop
#define _libgus_do_voice_volume			gus_do_voice_volume
#define _libgus_do_wait				gus_do_wait
#if LIBGUS_VERSION_MAJOR < 0x0004
#define _libgus_get_handle			gus_get_handle
#else
#define _libgus_get_file_descriptor		gus_get_file_descriptor
#endif
#define _libgus_info				gus_info
#define _libgus_memory_alloc			gus_memory_alloc
#define _libgus_memory_free			gus_memory_free
#define _libgus_memory_free_size		gus_memory_free_size
#define _libgus_memory_pack			gus_memory_pack
#define _libgus_open				gus_open
#define _libgus_queue_flush			gus_queue_flush
#define _libgus_queue_read_set_size		gus_queue_read_set_size
#define _libgus_queue_write_set_size		gus_queue_write_set_size
#define _libgus_reset				gus_reset
#if LIBGUS_VERSION_MAJOR < 0x0004
#define _libgus_select				gus_select
#endif
#define _libgus_timer_start			gus_timer_start
#define _libgus_timer_stop			gus_timer_stop
#define _libgus_timer_tempo			gus_timer_tempo
#endif

#define libgus_cards				_libgus_cards	/* same between v3 and v4 */
#define libgus_open				_libgus_open	/* different between v3 and v4: must use #ifdef */
#define libgus_close				_libgus_close	/* different between v3 and v4: must use #ifdef */
/* the following can be handled easily by macros: v4 only adds them the handle as the first param */
#if LIBGUS_VERSION_MAJOR < 0x0004
#define libgus_get_handle			_libgus_get_handle /* only in v3 */
#define libgus_do_flush				_libgus_do_flush
#define libgus_do_tempo				_libgus_do_tempo
#define libgus_do_voice_frequency		_libgus_do_voice_frequency
#define libgus_do_voice_pan			_libgus_do_voice_pan
#define libgus_do_voice_start			_libgus_do_voice_start
#define libgus_do_voice_start_position		_libgus_do_voice_start_position
#define libgus_do_voice_stop			_libgus_do_voice_stop
#define libgus_do_voice_volume			_libgus_do_voice_volume
#define libgus_do_wait				_libgus_do_wait
#define libgus_info				_libgus_info
#define libgus_memory_alloc			_libgus_memory_alloc
#define libgus_memory_free			_libgus_memory_free
#define libgus_memory_free_size			_libgus_memory_free_size
#define libgus_memory_pack			_libgus_memory_pack
#define libgus_queue_flush			_libgus_queue_flush
#define libgus_queue_read_set_size		_libgus_queue_read_set_size
#define libgus_queue_write_set_size		_libgus_queue_write_set_size
#define libgus_reset				_libgus_reset
#define libgus_select				_libgus_select
#define libgus_timer_start			_libgus_timer_start
#define libgus_timer_stop			_libgus_timer_stop
#define libgus_timer_tempo			_libgus_timer_tempo
#else
#define libgus_get_file_descriptor		_libgus_get_file_descriptor /* only in v4 */
#define libgus_do_flush()			_libgus_do_flush(ultra_h)
#define libgus_do_tempo(t)			_libgus_do_tempo(ultra_h,t)
#define libgus_do_voice_frequency(a,b)		_libgus_do_voice_frequency(ultra_h,a,b)
#define libgus_do_voice_pan(a,b)		_libgus_do_voice_pan(ultra_h,a,b)
#define libgus_do_voice_start(a,b,c,d,e)	_libgus_do_voice_start(ultra_h,a,b,c,d,e)
#define libgus_do_voice_start_position(a,b,c,d,e,f) _libgus_do_voice_start_position(ultra_h,a,b,c,d,e,f)
#define libgus_do_voice_stop(a,b)		_libgus_do_voice_stop(ultra_h,a,b)
#define libgus_do_voice_volume(a,b)		_libgus_do_voice_volume(ultra_h,a,b)
#define libgus_do_wait(a)			_libgus_do_wait(ultra_h,a)
#define libgus_info(a,b)			_libgus_info(ultra_h,a,b)
#define libgus_memory_alloc(a)			_libgus_memory_alloc(ultra_h,a)
#define libgus_memory_free(a)			_libgus_memory_free(ultra_h,a)
#define libgus_memory_free_size()		_libgus_memory_free_size(ultra_h)
#define libgus_memory_pack()			_libgus_memory_pack(ultra_h)
#define libgus_queue_flush()			_libgus_queue_flush(ultra_h)
#define libgus_queue_read_set_size(a)		_libgus_queue_read_set_size(ultra_h,a)
#define libgus_queue_write_set_size(a)		_libgus_queue_write_set_size(ultra_h,a)
#define libgus_reset(a,b)			_libgus_reset(ultra_h,a,b)
#define libgus_timer_start()			_libgus_timer_start(ultra_h)
#define libgus_timer_stop()			_libgus_timer_stop(ultra_h)
#define libgus_timer_tempo(a)			_libgus_timer_tempo(ultra_h,a)
#endif

#define GUS_SAMPLES			256	/* Max. GUS samples loadable */
#define GUS_CHANNELS			32	/* Max. GUS channels available */
#define SIZE_OF_SEQBUF		(8 * 1024)	/* Size of the sequence buffer */
#define ULTRA_PAN_MIDDLE	(16383 >> 1)	/* Middle balance position */

#define CH_FREQ	1
#define CH_VOL	2
#define CH_PAN	4

/*	This structure holds the current state of a GUS voice channel. */
typedef struct GUS_VOICE {
	UBYTE kick;
	UBYTE active;
	UWORD flags;
	SWORD handle;
	ULONG start;
	ULONG size;
	ULONG reppos;
	ULONG repend;
	ULONG frq;
	int vol;
	int decvol;
	int pan;

	int changes;
#ifndef HAVE_VOICE_STATUS
	time_t started;
#endif
} GUS_VOICE;

/* Global declarations follow */

static SAMPLE *samples[GUS_SAMPLES];	/* sample handles */
static GUS_VOICE voices[GUS_CHANNELS];	/* channel status */

static int ultra_dev = 0;	/* GUS index, if more than one card */
#if LIBGUS_VERSION_MAJOR < 0x0004
static int ultra_card = -1;	/* returned by gus_open(ultra_dev,,) - must be same as ultra_dev */
#else
static void* ultra_h = NULL;	/* GUS handle */
#endif
static int ultra_fd = -1;	/* GUS file descriptor */


#ifdef MIKMOD_DYNAMIC
static int Ultra_Link(void)
{
	if (libgus)
		return 0;

	/* load libgus.so */
#if LIBGUS_VERSION_MAJOR < 0x0004
	libgus = dlopen("libgus.so.3", RTLD_LAZY | RTLD_GLOBAL);
#else
	libgus = dlopen("libgus.so.4", RTLD_LAZY | RTLD_GLOBAL);
#endif
	if (!libgus) /* then this won't succeed either, but whatever.. */
		libgus = dlopen("libgus.so", RTLD_LAZY | RTLD_GLOBAL);
	if (!libgus)
		return 1;

	/* resolve function references */
#define IMPORT_SYMBOL(x,ret,params) \
	if (!(_lib##x = (ret (*)params) dlsym(libgus, #x))) return 1

	IMPORT_SYMBOL(gus_cards, int, (void));
#if LIBGUS_VERSION_MAJOR < 0x0004
	IMPORT_SYMBOL(gus_close, int, (int));
	IMPORT_SYMBOL(gus_do_flush, int, (void));
	IMPORT_SYMBOL(gus_do_tempo, void, (unsigned int));
	IMPORT_SYMBOL(gus_do_voice_frequency, void, (unsigned char, unsigned int));
	IMPORT_SYMBOL(gus_do_voice_pan, void, (unsigned char, unsigned short));
	IMPORT_SYMBOL(gus_do_voice_start, void, (unsigned char, unsigned int, unsigned int, unsigned short, unsigned short));
	IMPORT_SYMBOL(gus_do_voice_start_position, void, (unsigned char, unsigned int, unsigned int, unsigned short, unsigned short, unsigned int));
	IMPORT_SYMBOL(gus_do_voice_stop, void, (unsigned char, unsigned char));
	IMPORT_SYMBOL(gus_do_voice_volume, void, (unsigned char, unsigned short));
	IMPORT_SYMBOL(gus_do_wait, void, (unsigned int));
	IMPORT_SYMBOL(gus_get_handle, int, (void));
	IMPORT_SYMBOL(gus_info, int, (gus_info_t *, int));
	IMPORT_SYMBOL(gus_memory_alloc, int, (gus_instrument_t *));
	IMPORT_SYMBOL(gus_memory_free, int, (gus_instrument_t *));
	IMPORT_SYMBOL(gus_memory_free_size, int, (void));
	IMPORT_SYMBOL(gus_memory_pack, int, (void));
	IMPORT_SYMBOL(gus_open, int, (int, size_t, int));
	IMPORT_SYMBOL(gus_queue_flush, int, (void));
	IMPORT_SYMBOL(gus_queue_read_set_size, int, (int));
	IMPORT_SYMBOL(gus_queue_write_set_size, int, (int));
	IMPORT_SYMBOL(gus_reset, int, (int, unsigned int));
	IMPORT_SYMBOL(gus_select, int, (int));
	IMPORT_SYMBOL(gus_timer_start, int, (void));
	IMPORT_SYMBOL(gus_timer_stop, int, (void));
	IMPORT_SYMBOL(gus_timer_tempo, int, (int));
#else
	IMPORT_SYMBOL(gus_close, int, (void*));
	IMPORT_SYMBOL(gus_do_flush, int, (void*));
	IMPORT_SYMBOL(gus_do_tempo, void, (void*, unsigned int));
	IMPORT_SYMBOL(gus_do_voice_frequency, void, (void*, unsigned char, unsigned int));
	IMPORT_SYMBOL(gus_do_voice_pan, void, (void*, unsigned char, unsigned short));
	IMPORT_SYMBOL(gus_do_voice_start, void, (void*, unsigned char, unsigned int, unsigned int, unsigned short, unsigned short));
	IMPORT_SYMBOL(gus_do_voice_start_position, void, (void*, unsigned char, unsigned int, unsigned int, unsigned short, unsigned short, unsigned int));
	IMPORT_SYMBOL(gus_do_voice_stop, void, (void*, unsigned char, unsigned char));
	IMPORT_SYMBOL(gus_do_voice_volume, void, (void*, unsigned char, unsigned short));
	IMPORT_SYMBOL(gus_do_wait, void, (void*, unsigned int));
	IMPORT_SYMBOL(gus_get_file_descriptor, int, (void*));
	IMPORT_SYMBOL(gus_info, int, (void*, gus_info_t *, int));
	IMPORT_SYMBOL(gus_memory_alloc, int, (void*, gus_instrument_t *));
	IMPORT_SYMBOL(gus_memory_free, int, (void*, gus_instrument_t *));
	IMPORT_SYMBOL(gus_memory_free_size, int, (void*));
	IMPORT_SYMBOL(gus_memory_pack, int, (void*));
	IMPORT_SYMBOL(gus_open, int, (void**, int, int, size_t, int));
	IMPORT_SYMBOL(gus_queue_flush, int, (void*));
	IMPORT_SYMBOL(gus_queue_read_set_size, int, (void*, int));
	IMPORT_SYMBOL(gus_queue_write_set_size, int, (void*, int));
	IMPORT_SYMBOL(gus_reset, int, (void*, int, unsigned int));
	IMPORT_SYMBOL(gus_timer_start, int, (void*));
	IMPORT_SYMBOL(gus_timer_stop, int, (void*));
	IMPORT_SYMBOL(gus_timer_tempo, int, (void*, int));
#endif
#undef IMPORT_SYMBOL

	return 0;
}

static void Ultra_Unlink(void)
{
	_libgus_cards = NULL;
	_libgus_close = NULL;
	_libgus_do_flush = NULL;
	_libgus_do_tempo = NULL;
	_libgus_do_voice_frequency = NULL;
	_libgus_do_voice_pan = NULL;
	_libgus_do_voice_start = NULL;
	_libgus_do_voice_start_position = NULL;
	_libgus_do_voice_stop = NULL;
	_libgus_do_voice_volume = NULL;
	_libgus_do_wait = NULL;
#if LIBGUS_VERSION_MAJOR < 0x0004
	_libgus_get_handle = NULL;
#else
	_libgus_get_file_descriptor = NULL;
#endif
	_libgus_info = NULL;
	_libgus_memory_alloc = NULL;
	_libgus_memory_free = NULL;
	_libgus_memory_free_size = NULL;
	_libgus_memory_pack = NULL;
	_libgus_open = NULL;
	_libgus_queue_flush = NULL;
	_libgus_queue_read_set_size = NULL;
	_libgus_queue_write_set_size = NULL;
	_libgus_reset = NULL;
#if LIBGUS_VERSION_MAJOR < 0x0004
	_libgus_select = NULL;
#endif
	_libgus_timer_start = NULL;
	_libgus_timer_stop = NULL;
	_libgus_timer_tempo = NULL;

	if (libgus) {
		dlclose(libgus);
		libgus = NULL;
	}
}
#endif

static void Ultra_CommandLine(const CHAR *cmdline)
{
	CHAR *ptr = MD_GetAtom("card", cmdline, 0);

	if (ptr) {
		int buf = atoi(ptr);

		if (buf >= 0 && buf <= 8)
			ultra_dev = buf;
		MikMod_free(ptr);
	}
#ifdef __DJGPP__
	ptr = MD_GetAtom("dma", cmdline, 0);
	if (ptr) {
		gus_dma_usage (atoi(ptr));
		MikMod_free(ptr);
	}
#endif
}

/* Checks for the presence of GUS cards */
static BOOL Ultra_IsThere(void)
{
	BOOL retval;

#ifdef MIKMOD_DYNAMIC
	if (Ultra_Link())
		return 0;
#endif
	retval = libgus_cards()? 1 : 0;
#ifdef MIKMOD_DYNAMIC
	Ultra_Unlink();
#endif
	return retval;
}

/* Load a new sample directly into GUS DRAM and return a handle */
static SWORD Ultra_SampleLoad(struct SAMPLOAD *sload, int type)
{
	int handle;
	SAMPLE *s = sload->sample;
	gus_instrument_t instrument;
	gus_layer_t layer;
	gus_wave_t wave;
	unsigned char *buffer;
	unsigned int length, loopstart, loopend;

	if (s->length > MAX_SAMPLE_SIZE) {
		_mm_errno = MMERR_NOT_A_STREAM;/* better error? */
		return -1;
	}

	/* Find empty slot to put sample in */
	for (handle = 0; handle < GUS_SAMPLES; handle++)
		if (!samples[handle])
			break;

	if (handle == GUS_SAMPLES) {
		_mm_errno = MMERR_OUT_OF_HANDLES;
		return -1;
	}

	/* Fill an gus_instrument_t structure and feed it to libgus. We can
	   download 8 and 16 bit, both signed and unsigned samples into GUS, so
	   don't bother much about formats here. */

	/* convert position/length data from samples to bytes */
	length = s->length;
	loopstart = s->loopstart;
	loopend = s->loopend ? s->loopend : length;
	/* sanity checks */
	if (loopend > length)
		loopend = length;
	if (loopstart > loopend)
		loopstart = loopend;
	if (s->flags & SF_16BITS) {
		length <<= 1;
		loopstart <<= 1;
		loopend <<= 1;
	}

	/* Load sample into normal memory */
	if (!(buffer = (unsigned char *) MikMod_malloc(length))) {
		_mm_errno = MMERR_SAMPLE_TOO_BIG;
		return -1;
	}

	if (SL_Load(buffer, sload, s->length)) {
		MikMod_free(buffer);
		return -1;
	}

	samples[handle] = s;

	memset(&wave, 0, sizeof(wave));
	memset(&layer, 0, sizeof(layer));
	memset(&instrument, 0, sizeof(instrument));

	wave.format =
		((s->flags & SF_SIGNED) ? 0 : GUS_WAVE_INVERT) |
		((s->flags & SF_16BITS) ? GUS_WAVE_16BIT : 0) |
		((s->flags & SF_DELTA ) ? GUS_WAVE_DELTA : 0) |
		((s->flags & SF_LOOP  ) ? GUS_WAVE_LOOP  : 0) |
		((s->flags & SF_BIDI  ) ? GUS_WAVE_BIDIR : 0);
	wave.begin.ptr = buffer;
	wave.loop_start = loopstart << 4;
	wave.loop_end = loopend << 4;
	wave.size = length;

	layer.wave = &wave;

	instrument.mode = layer.mode = wave.mode = GUS_INSTR_SIMPLE;
	instrument.number.instrument = handle;
	instrument.info.layer = &layer;

	/* Download the sample to GUS RAM */
	if (libgus_memory_alloc(&instrument)) {
		MikMod_free(buffer);
		_mm_errno = MMERR_SAMPLE_TOO_BIG;
		return -1;
	}

	MikMod_free(buffer);
	return handle;
}

/* Discards a sample from the GUS memory and mark handle as free */
static void Ultra_SampleUnload(SWORD handle)
{
	gus_instrument_t instrument;

	if (handle >= GUS_SAMPLES || handle < 0 || !samples[handle])
		return;

	memset(&instrument, 0, sizeof(instrument));
	instrument.mode = GUS_INSTR_SIMPLE;
	instrument.number.instrument = handle;
	libgus_memory_free(&instrument);
	samples[handle] = NULL;
}

/* Reports available sample space */
static ULONG Ultra_SampleSpace(int type)
{
	libgus_memory_pack();
	return (libgus_memory_free_size());
}

/* Reports the size of a sample */
static ULONG Ultra_SampleLength(int type, SAMPLE *s)
{
	if (!s)
		return 0;

	if (s->flags & SF_16BITS)
		return ((s->length << 1) + 31) & ~31;
	else
		return ( s->length       + 15) & ~15;
}

/* Initializes the driver */
static int Ultra_Init_internal(void)
{
	gus_info_t info;

#if LIBGUS_VERSION_MAJOR < 0x0004
	if ((ultra_card = libgus_open(ultra_dev, SIZE_OF_SEQBUF, 0)) < 0) {
		_mm_errno = (errno == ENOMEM)? MMERR_OUT_OF_MEMORY : MMERR_INVALID_DEVICE;
		return 1;
	}
	libgus_select(ultra_card);
	ultra_fd = libgus_get_handle();
#else
	if (libgus_open(&ultra_h, ultra_dev, 0, SIZE_OF_SEQBUF, GUS_OPEN_FLAG_NONE) < 0) {
		_mm_errno = (errno == ENOMEM)? MMERR_OUT_OF_MEMORY : MMERR_INVALID_DEVICE;
		return 1;
	}
	ultra_fd = libgus_get_file_descriptor(ultra_h);
#endif

	/* We support only 16-bit stereo with 44K mixing frequency. On UltraSound
	   Classic mixing frequency depends on number of channels, on Interwave it
	   is always 44KHz. */
	md_mode |= DMODE_16BITS | DMODE_STEREO;
	md_mixfreq = info.mixing_freq;

	libgus_info(&info, 0);
#ifdef MIKMOD_DEBUG
	switch (info.version) {
	  case 0x24:
		fputs("GUS 2.4", stderr);
		break;
	  case 0x35:
		fputs("GUS 3.7 (flipped)", stderr);
		break;
	  case 0x37:
		fputs("GUS 3.7", stderr);
		break;
	  case 0x90:
		fputs("GUS ACE", stderr);
		break;
	  case 0xa0:
		fputs("GUS MAX 10", stderr);
		break;
	  case 0xa1:
		fputs("GUS MAX 11", stderr);
		break;
	  case 0x100:
		fputs("Interwave/GUS PnP", stderr);
		break;
	  default:
		fprintf(stderr, "Unknown GUS type %x", info.version);
		break;
	}
	fprintf(stderr, " with %dKb RAM on board\n", info.memory_size >> 10);
#endif

	/* Zero the voice states and sample handles */
	memset (&voices, 0, sizeof (voices));
	memset (&samples, 0, sizeof (samples));

	return 0;
}

static int Ultra_Init(void)
{
#ifdef MIKMOD_DYNAMIC
	if (Ultra_Link()) {
		_mm_errno = MMERR_DYNAMIC_LINKING;
		return 1;
	}
#endif
	return Ultra_Init_internal();
}

/* Closes the driver */
static void Ultra_Exit_internal(void)
{
#if LIBGUS_VERSION_MAJOR < 0x0004
	if (ultra_card >= 0) {
		ultra_card = -1;
		libgus_close(ultra_dev);
	}
#else
	if (ultra_h) {
		libgus_close(ultra_h);
		ultra_h = NULL;
	}
#endif
	ultra_fd = -1;
}

static void Ultra_Exit(void)
{
	Ultra_Exit_internal();
#ifdef MIKMOD_DYNAMIC
	Ultra_Unlink();
#endif
}

/* Poor man's reset function */
static int Ultra_Reset(void)
{
	Ultra_Exit_internal();
	return Ultra_Init_internal();
}

static int Ultra_SetNumVoices(void)
{
	return 0;
}

/* Module player tick function */
static void UltraPlayer(void)
{
	int channel, panning;
	struct GUS_VOICE *voice;
	static BOOL ultra_pause = 1;	/* paused status */

	md_player();

	if (ultra_pause != Player_Paused())
		if ((ultra_pause = Player_Paused()))
			for (channel = 0, voice = voices; channel < md_numchn;
			     channel++, voice++) {
				libgus_do_voice_volume (channel, 0);
				voices->changes |= (CH_VOL | CH_FREQ | CH_PAN);
			}

	if (ultra_pause)
		return;

	for (channel = 0, voice = voices; channel < md_numchn; channel++, voice++) {
		panning = (voice->pan == PAN_SURROUND) ?
			       ULTRA_PAN_MIDDLE : (voice->pan << 6);

		if (voice->kick) {
			voice->kick = 0;
			voice->decvol = voice->vol;
			if (voice->start > 0)
				libgus_do_voice_start_position(channel, voice->handle,
				      voice->frq, voice->vol << 6, panning, voice->start << 4);
			else
				libgus_do_voice_start(channel, voice->handle, voice->frq,
			                                 voice->vol << 6, voice->pan << 6);
		} else {
			if (voice->changes & CH_FREQ)
				libgus_do_voice_frequency(channel, voice->frq);
			if (voice->changes & CH_VOL)
				libgus_do_voice_volume(channel, voice->vol << 6);
			if (voice->changes & CH_PAN)
				libgus_do_voice_pan(channel, panning);
			if (voice->decvol)
				voice->decvol -= 4;
		}
		voice->changes = 0;
	}
}

/* Play sound */
#if defined(__DJGPP__) || defined(__EMX__)
static void Ultra_Callback(void)
{
	UltraPlayer();
	libgus_do_flush();
}

static void Ultra_Update(void)
{
	static UWORD ultra_bpm = 0;		/* current GUS tempo */

	/* All real work is done during GF1 timer 1 interrupt */
	if (ultra_bpm != md_bpm) {
		libgus_do_tempo((md_bpm * 50) / 125);
		ultra_bpm = md_bpm;
	}
}

#else
static void Ultra_Update(void)
{
	fd_set write_fds;
	int need_write;
	static UWORD ultra_bpm = 0;	/* current GUS tempo */

	if (ultra_bpm != md_bpm) {
		libgus_do_tempo((md_bpm * 50) / 125);
		ultra_bpm = md_bpm;
	}

	UltraPlayer();

	do {
		need_write = libgus_do_flush();

		FD_ZERO(&write_fds);
		do {
			FD_SET(ultra_fd, &write_fds);

			select(ultra_fd + 1, NULL, &write_fds, NULL, NULL);
		} while (!FD_ISSET(ultra_fd, &write_fds));
	} while(need_write > 0);

	/* Wait so that all voice commands gets executed */
	libgus_do_wait (1);
}

#endif

/* Start playback */
static int Ultra_PlayStart(void)
{
	int t;
	gus_info_t info;

	for (t = 0; t < md_numchn; t++) {
		voices[t].flags = 0;
		voices[t].handle = 0;
		voices[t].size = 0;
		voices[t].start = 0;
		voices[t].reppos = 0;
		voices[t].repend = 0;
		voices[t].changes = 0;
		voices[t].kick = 0;
		voices[t].frq = 10000;
		voices[t].vol = 0;
		voices[t].pan = ULTRA_PAN_MIDDLE;
		voices[t].active = 0;
	}

#if LIBGUS_VERSION_MAJOR < 0x0004
	libgus_select(ultra_card);
#endif
	if (libgus_reset(md_numchn, 0) < 0) {
		_mm_errno = MMERR_GUS_RESET;
		return 1;
	}

	/* Query mixing frequency */
	libgus_info(&info, 0);
	md_mixfreq = info.mixing_freq;

	libgus_queue_write_set_size(1024);
	libgus_queue_read_set_size(128);

	if (libgus_timer_start() < 0) {
		_mm_errno = MMERR_GUS_TIMER;
		return 1;
	}

#if defined(__DJGPP__) || defined(__EMX__)
	gus_timer_callback(Ultra_Callback);
#endif

	libgus_timer_tempo(50);

	for (t = 0; t < md_numchn; t++) {
		libgus_do_voice_pan(t, ULTRA_PAN_MIDDLE);
		libgus_do_voice_volume(t, 0 << 8);
	}

	libgus_do_flush();

	return 0;
}

/* Stop playback */
static void Ultra_PlayStop(void)
{
	int voice;

	libgus_queue_flush();
	libgus_timer_stop();
	libgus_queue_write_set_size(0);
	libgus_queue_read_set_size(0);
	for(voice = 0; voice < md_numchn; voice++)
		libgus_do_voice_stop(voice, 0);

#if defined(__DJGPP__) || defined(__EMX__)
	gus_timer_callback(NULL);
#endif

	libgus_do_flush();
}

/* Set the volume for the given voice */
static void Ultra_VoiceSetVolume(UBYTE voice, UWORD vol)
{
	if (voice < md_numchn)
		if (vol != voices[voice].vol) {
			voices[voice].decvol =
			voices[voice].vol = vol;
			voices[voice].changes |= CH_VOL;
		}
}

/* Returns the volume of the given voice */
static UWORD Ultra_VoiceGetVolume(UBYTE voice)
{
	return (voice < md_numchn) ? voices[voice].vol : 0;
}

/* Set the pitch for the given voice */
static void Ultra_VoiceSetFrequency(UBYTE voice, ULONG frq)
{
	if (voice < md_numchn)
		if (frq != voices[voice].frq) {
			voices[voice].frq = frq;
			voices[voice].changes |= CH_FREQ;
		}
}

/* Returns the frequency of the given voice */
static ULONG Ultra_VoiceGetFrequency(UBYTE voice)
{
	return (voice < md_numchn) ? voices[voice].frq : 0;
}

/* Set the panning position for the given voice */
static void Ultra_VoiceSetPanning(UBYTE voice, ULONG pan)
{
	if (voice < md_numchn)
		if (pan != voices[voice].pan) {
			voices[voice].pan = pan;
			voices[voice].changes |= CH_PAN;
		}
}

/* Returns the panning of the given voice */
static ULONG Ultra_VoiceGetPanning(UBYTE voice)
{
	return (voice < md_numchn) ? voices[voice].pan : 0;
}

/* Start a new sample on a voice */
static void Ultra_VoicePlay(UBYTE voice, SWORD handle, ULONG start,
							ULONG size, ULONG reppos, ULONG repend,
							UWORD flags)
{
	if ((voice >= md_numchn) || (start >= size))
		return;

	if (flags & SF_LOOP)
		if (repend > size)
			repend = size;

	voices[voice].flags = flags;
	voices[voice].handle = handle;
	voices[voice].start = start;
	voices[voice].size = size;
	voices[voice].reppos = reppos;
	voices[voice].repend = repend;
	voices[voice].kick = 1;
	voices[voice].active = 1;
#ifndef HAVE_VOICE_STATUS
	voices[voice].started = time(NULL);
#endif
}

/* Stops a voice */
static void Ultra_VoiceStop(UBYTE voice)
{
	if (voice < md_numchn)
		voices[voice].active = 0;
}

/* Returns whether a voice is stopped */
static BOOL Ultra_VoiceStopped(UBYTE voice)
{
	if (voice >= md_numchn)
		return 1;

#ifdef HAVE_VOICE_STATUS
	if (voices[voice].active)
		return gus_get_voice_status(voice) ? 0 : 1;
	else
		return 1;
#else
	if (voices[voice].active) {
		/* is sample looping ? */
		if (voices[voice].flags & (SF_LOOP | SF_BIDI))
			return 0;
		else
		  if (time(NULL) - voices[voice].started >=
			  ((voices[voice].size - voices[voice].start + md_mixfreq -1)
			  / md_mixfreq)) {
			voices[voice].active = 0;
			return 1;
		}
		return 0;
	} else
		return 1;
#endif
}

/* Returns current voice position */
static SLONG Ultra_VoiceGetPosition(UBYTE voice)
{
	/* NOTE This information can not be determined. */
	return -1;
}

/* Returns voice real volume */
static ULONG Ultra_VoiceRealVolume(UBYTE voice)
{
	int retval = 0;
	if (!Ultra_VoiceStopped (voice)) {
		/* NOTE This information can not be accurately determined. */
		retval = (voices [voice].decvol) << 8;
		if (retval > 0xffff) retval = 0xffff;
	}
	return retval;
}

MDRIVER drv_ultra = {
	NULL,
	"Gravis Ultrasound native mode",
	"Gravis Ultrasound native mode driver v1.2",
	0, (GUS_CHANNELS)-1,
	"ultra",
#ifdef __DJGPP__
        "dma:b:1:Use DMA for transferring samples into GUS DRAM\n"
#endif
	"card:r:0,8,0:Soundcard number\n",

	Ultra_CommandLine,
	Ultra_IsThere,
	Ultra_SampleLoad,
	Ultra_SampleUnload,
	Ultra_SampleSpace,
	Ultra_SampleLength,
	Ultra_Init,
	Ultra_Exit,
	Ultra_Reset,
	Ultra_SetNumVoices,
	Ultra_PlayStart,
	Ultra_PlayStop,
	Ultra_Update,
	NULL,
	Ultra_VoiceSetVolume,
	Ultra_VoiceGetVolume,
	Ultra_VoiceSetFrequency,
	Ultra_VoiceGetFrequency,
	Ultra_VoiceSetPanning,
	Ultra_VoiceGetPanning,
	Ultra_VoicePlay,
	Ultra_VoiceStop,
	Ultra_VoiceStopped,
	Ultra_VoiceGetPosition,
	Ultra_VoiceRealVolume
};
#else

MISSING(drv_ultra);

#endif /* DRV_ULTRA */

/* ex:set ts=8: */
