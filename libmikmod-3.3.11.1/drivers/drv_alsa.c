/*	MikMod sound library
	(c) 1998, 1999, 2000 Miodrag Vallat and others - see file AUTHORS for
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

  Driver for Advanced Linux Sound Architecture (ALSA)

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_ALSA

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif

#ifdef MIKMOD_DYNAMIC
#include <dlfcn.h>
#endif
#include <stdlib.h>
#include <string.h>

#include <alsa/asoundlib.h>
#if defined(SND_LIB_VERSION) && (SND_LIB_VERSION >= 0x20000)
#undef DRV_ALSA
#endif

#if defined(SND_LIB_VERSION) && (SND_LIB_VERSION < 0x600)
#error ALSA Version too old. Please upgrade your Linux distribution.
#endif
#endif /* DRV_ALSA */

#ifdef DRV_ALSA

#ifdef MIKMOD_DYNAMIC
/* runtime link with libasound */
#ifndef HAVE_RTLD_GLOBAL
#define RTLD_GLOBAL (0)
#endif
static int (*alsa_pcm_subformat_mask_malloc)(snd_pcm_subformat_mask_t **);
static const char * (*alsa_strerror)(int);
static int (*alsa_pcm_resume)(snd_pcm_t *);
static int (*alsa_pcm_prepare)(snd_pcm_t *);
static int (*alsa_pcm_hw_params_any)(snd_pcm_t *, snd_pcm_hw_params_t *);
static int (*alsa_pcm_hw_params)(snd_pcm_t *, snd_pcm_hw_params_t *);
static int (*alsa_pcm_hw_params_current)(snd_pcm_t *, snd_pcm_hw_params_t *);
static int (*alsa_pcm_hw_params_set_access)(snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_access_t);
static int (*alsa_pcm_hw_params_set_format)(snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_format_t);
static int (*alsa_pcm_hw_params_set_rate_near)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *, int *);
static int (*alsa_pcm_hw_params_set_channels_near)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *);
static int (*alsa_pcm_hw_params_set_buffer_time_near)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *, int *);
static int (*alsa_pcm_hw_params_set_period_time_near)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *, int *);
static int (*alsa_pcm_hw_params_get_buffer_size)(const snd_pcm_hw_params_t *, snd_pcm_uframes_t *);
static int (*alsa_pcm_hw_params_get_period_size)(const snd_pcm_hw_params_t *, snd_pcm_uframes_t *, int *);
static int (*alsa_pcm_sw_params_sizeof)(void);
static int (*alsa_pcm_hw_params_sizeof)(void);
static int (*alsa_pcm_open)(snd_pcm_t**, const char *, int, int);
static int (*alsa_pcm_close)(snd_pcm_t*);
static int (*alsa_pcm_drain)(snd_pcm_t*);
static int (*alsa_pcm_drop)(snd_pcm_t*);
static int (*alsa_pcm_start)(snd_pcm_t *);
static snd_pcm_sframes_t (*alsa_pcm_writei)(snd_pcm_t*,const void*,snd_pcm_uframes_t);

static void* libasound = NULL;

#else
/* compile-time link with libasound */
#define alsa_pcm_subformat_mask_malloc		snd_pcm_subformat_mask_malloc
#define alsa_strerror				snd_strerror
#define alsa_pcm_hw_params_any			snd_pcm_hw_params_any
#define alsa_pcm_hw_params			snd_pcm_hw_params
#define alsa_pcm_hw_params_current		snd_pcm_hw_params_current
#define alsa_pcm_hw_params_set_access		snd_pcm_hw_params_set_access
#define alsa_pcm_hw_params_set_format		snd_pcm_hw_params_set_format
#define alsa_pcm_hw_params_set_rate_near	snd_pcm_hw_params_set_rate_near
#define alsa_pcm_hw_params_set_channels_near	snd_pcm_hw_params_set_channels_near
#define alsa_pcm_hw_params_set_buffer_time_near	snd_pcm_hw_params_set_buffer_time_near
#define alsa_pcm_hw_params_set_period_time_near	snd_pcm_hw_params_set_period_time_near
#define alsa_pcm_hw_params_get_buffer_size	snd_pcm_hw_params_get_buffer_size
#define alsa_pcm_hw_params_get_period_size	snd_pcm_hw_params_get_period_size
#define alsa_pcm_resume				snd_pcm_resume
#define alsa_pcm_prepare			snd_pcm_prepare
#define alsa_pcm_close				snd_pcm_close
#define alsa_pcm_drain				snd_pcm_drain
#define alsa_pcm_drop				snd_pcm_drop
#define alsa_pcm_start				snd_pcm_start
#define alsa_pcm_open				snd_pcm_open
#define alsa_pcm_writei				snd_pcm_writei
#endif /* MIKMOD_DYNAMIC */

#if defined(MIKMOD_DEBUG)
# define dbgprint			fprintf
#elif defined (__GNUC__) && !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
# define dbgprint(f, fmt, args...)	do {} while (0)
#else
# define dbgprint(f, ...)		do {} while (0)
#endif
static BOOL enabled = 0;
static snd_pcm_t *pcm_h = NULL;
static SBYTE *audiobuffer = NULL;
static snd_pcm_sframes_t period_size;
static int bytes_written = 0, bytes_played = 0;
static int global_frame_size;

#ifdef MIKMOD_DYNAMIC
static int ALSA_Link(void)
{
	if (libasound) return 0;

	/* load libasound.so */
	libasound = dlopen("libasound.so.2",RTLD_LAZY|RTLD_GLOBAL);
	if (!libasound) libasound = dlopen("libasound.so",RTLD_LAZY|RTLD_GLOBAL);
	if (!libasound) return 1;

	if (!(alsa_pcm_subformat_mask_malloc = (int (*)(snd_pcm_subformat_mask_t **))
						 dlsym(libasound,"snd_pcm_subformat_mask_malloc"))) return 1;
	if (!(alsa_strerror = (const char* (*)(int))
						 dlsym(libasound,"snd_strerror"))) return 1;
	if (!(alsa_pcm_prepare = (int (*)(snd_pcm_t *))
						 dlsym(libasound,"snd_pcm_prepare"))) return 1;
	if (!(alsa_pcm_sw_params_sizeof = (int (*)(void))
						 dlsym(libasound,"snd_pcm_sw_params_sizeof"))) return 1;
	if (!(alsa_pcm_hw_params_sizeof = (int (*)(void))
						 dlsym(libasound,"snd_pcm_hw_params_sizeof"))) return 1;
	if (!(alsa_pcm_resume = (int (*)(snd_pcm_t *))
						 dlsym(libasound,"snd_pcm_resume"))) return 1;
	if (!(alsa_pcm_hw_params_any = (int (*)(snd_pcm_t *, snd_pcm_hw_params_t *))
						 dlsym(libasound,"snd_pcm_hw_params_any"))) return 1;
	if (!(alsa_pcm_hw_params = (int (*)(snd_pcm_t *, snd_pcm_hw_params_t *))
						 dlsym(libasound,"snd_pcm_hw_params"))) return 1;
	if (!(alsa_pcm_hw_params_current = (int (*)(snd_pcm_t *, snd_pcm_hw_params_t *))
						 dlsym(libasound,"snd_pcm_hw_params_current"))) return 1;
	if (!(alsa_pcm_hw_params_set_access = (int (*)(snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_access_t))
						 dlsym(libasound,"snd_pcm_hw_params_set_access"))) return 1;
	if (!(alsa_pcm_hw_params_set_format = (int (*)(snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_format_t))
						 dlsym(libasound,"snd_pcm_hw_params_set_format"))) return 1;
	if (!(alsa_pcm_hw_params_set_rate_near = (int (*)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *, int *))
						 dlsym(libasound,"snd_pcm_hw_params_set_rate_near"))) return 1;
	if (!(alsa_pcm_hw_params_set_channels_near = (int (*)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *))
						 dlsym(libasound,"snd_pcm_hw_params_set_channels_near"))) return 1;
	if (!(alsa_pcm_hw_params_set_buffer_time_near = (int (*)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *, int *))
						 dlsym(libasound,"snd_pcm_hw_params_set_buffer_time_near"))) return 1;
	if (!(alsa_pcm_hw_params_set_period_time_near = (int (*)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *, int *))
						 dlsym(libasound,"snd_pcm_hw_params_set_period_time_near"))) return 1;
	if (!(alsa_pcm_hw_params_get_buffer_size = (int (*)(const snd_pcm_hw_params_t *, snd_pcm_uframes_t *))
						 dlsym(libasound,"snd_pcm_hw_params_get_buffer_size"))) return 1;
	if (!(alsa_pcm_hw_params_get_period_size = (int (*)(const snd_pcm_hw_params_t *, snd_pcm_uframes_t *, int *))
						 dlsym(libasound,"snd_pcm_hw_params_get_period_size"))) return 1;
	if (!(alsa_pcm_open = (int (*)(snd_pcm_t**, const char *, int, int))
						 dlsym(libasound,"snd_pcm_open"))) return 1;
	if (!(alsa_pcm_close = (int (*)(snd_pcm_t*))
						 dlsym(libasound,"snd_pcm_close"))) return 1;
	if (!(alsa_pcm_drain = (int (*)(snd_pcm_t*))
						 dlsym(libasound,"snd_pcm_drain"))) return 1;
	if (!(alsa_pcm_drop = (int (*)(snd_pcm_t*))
						 dlsym(libasound,"snd_pcm_drop"))) return 1;
	if (!(alsa_pcm_start = (int (*)(snd_pcm_t *))
						 dlsym(libasound,"snd_pcm_start"))) return 1;
	if (!(alsa_pcm_writei = (snd_pcm_sframes_t (*)(snd_pcm_t*,const void*,snd_pcm_uframes_t))
						 dlsym(libasound,"snd_pcm_writei"))) return 1;

	return 0;
}

static void ALSA_Unlink(void)
{
	alsa_pcm_subformat_mask_malloc = NULL;
	alsa_strerror = NULL;
	alsa_pcm_resume = NULL;
	alsa_pcm_prepare = NULL;
	alsa_pcm_hw_params_any = NULL;
	alsa_pcm_hw_params = NULL;
	alsa_pcm_hw_params_current = NULL;
	alsa_pcm_hw_params_set_access = NULL;
	alsa_pcm_hw_params_set_format = NULL;
	alsa_pcm_hw_params_set_rate_near = NULL;
	alsa_pcm_hw_params_set_channels_near = NULL;
	alsa_pcm_hw_params_set_buffer_time_near = NULL;
	alsa_pcm_hw_params_set_period_time_near = NULL;
	alsa_pcm_hw_params_get_buffer_size = NULL;
	alsa_pcm_hw_params_get_period_size = NULL;
	alsa_pcm_close = NULL;
	alsa_pcm_drain = NULL;
	alsa_pcm_drop = NULL;
	alsa_pcm_start = NULL;
	alsa_pcm_open = NULL;
	alsa_pcm_writei = NULL;

	if (libasound) {
		dlclose(libasound);
		libasound = NULL;
	}
}

/* This is done to override the identifiers expanded
 * in the macros provided by the ALSA includes which are
 * not available.
 * */
#define snd_strerror			alsa_strerror
#define snd_pcm_sw_params_sizeof	alsa_pcm_sw_params_sizeof
#define snd_pcm_hw_params_sizeof	alsa_pcm_hw_params_sizeof
#endif /* MIKMOD_DYNAMIC */

static void ALSA_CommandLine(const CHAR *cmdline)
{
		/* no options */
}

static BOOL ALSA_IsThere(void)
{
	snd_pcm_subformat_mask_t *ptr = NULL;
	BOOL retval;

#ifdef MIKMOD_DYNAMIC
	if (ALSA_Link()) return 0;
#endif
	retval = (alsa_pcm_subformat_mask_malloc(&ptr) == 0) && (ptr != NULL);
	free(ptr);
#ifdef MIKMOD_DYNAMIC
	ALSA_Unlink();
#endif
	return retval;
}

static int ALSA_Init_internal(void)
{
	snd_pcm_format_t pformat;
	unsigned int btime = 250000;	/* 250ms */
	unsigned int ptime = 50000;	/* 50ms */
	snd_pcm_uframes_t psize;
	snd_pcm_uframes_t bsize;
	unsigned int rate, channels;
	snd_pcm_hw_params_t * hwparams;
	int err;

	/* setup playback format structure */
	pformat = (md_mode&DMODE_FLOAT)? SND_PCM_FORMAT_FLOAT :
			(md_mode&DMODE_16BITS)? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_U8;
	channels = (md_mode&DMODE_STEREO)?2:1;
	rate = md_mixfreq;

#define MIKMOD_ALSA_DEVICE "default"
	if ((err = alsa_pcm_open(&pcm_h, MIKMOD_ALSA_DEVICE, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) < 0) {
		_mm_errno = MMERR_OPENING_AUDIO;
		goto END;
	}

	snd_pcm_hw_params_alloca(&hwparams);
	err = alsa_pcm_hw_params_any(pcm_h, hwparams);
	if (err < 0) {
		_mm_errno = MMERR_ALSA_NOCONFIG;
		goto END;
	}

	err = alsa_pcm_hw_params_set_access(pcm_h, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (!err) err = alsa_pcm_hw_params_set_format(pcm_h, hwparams, pformat);
	if (!err) err = alsa_pcm_hw_params_set_rate_near(pcm_h, hwparams, &rate, NULL);
	if (!err) err = alsa_pcm_hw_params_set_channels_near(pcm_h, hwparams, &channels);
	if (!err) err = alsa_pcm_hw_params_set_buffer_time_near(pcm_h, hwparams, &btime, NULL);
	if (!err) err = alsa_pcm_hw_params_set_period_time_near(pcm_h, hwparams, &ptime, NULL);
	if (!err) err = alsa_pcm_hw_params(pcm_h, hwparams);
	if (err < 0) {
		_mm_errno = MMERR_ALSA_SETPARAMS;
		goto END;
	}

	if (rate != md_mixfreq) {
		_mm_errno = MMERR_ALSA_SETRATE;
		goto END;
	}
	if (!(md_mode&DMODE_STEREO) && channels != 1) {
		_mm_errno = MMERR_ALSA_SETCHANNELS;
		goto END;
	}
	if ((md_mode&DMODE_STEREO) && channels != 2) {
		_mm_errno = MMERR_ALSA_SETCHANNELS;
		goto END;
	}

	err = alsa_pcm_hw_params_current(pcm_h, hwparams);
	if (!err) err = alsa_pcm_hw_params_get_buffer_size(hwparams, &bsize);
	if (!err) err = alsa_pcm_hw_params_get_period_size(hwparams, &psize, NULL);
	if (err < 0) {
		_mm_errno = MMERR_ALSA_BUFFERSIZE;
		goto END;
	}

	period_size = psize;
	global_frame_size = channels *
				((md_mode&DMODE_FLOAT)? 4 : (md_mode&DMODE_16BITS)? 2 : 1);

	if (!(audiobuffer=(SBYTE*)MikMod_malloc(period_size * global_frame_size))) {
		_mm_errno = MMERR_OUT_OF_MEMORY;
		goto END;
	}

	/* sound device is ready to work */
	if (!VC_Init()) {
		enabled = 1;
		return 0;
	}
END:
	alsa_pcm_close(pcm_h);
	pcm_h = NULL;
	return 1;
}

static int ALSA_Init(void)
{
#ifdef HAVE_SSE2
/* TODO : Detect SSE2, then set  md_mode |= DMODE_SIMDMIXER;*/
#endif
#ifdef MIKMOD_DYNAMIC
	if (ALSA_Link()) {
		_mm_errno=MMERR_DYNAMIC_LINKING;
		return 1;
	}
#endif
	return ALSA_Init_internal();
}

static void ALSA_Exit_internal(void)
{
	enabled = 0;
	VC_Exit();
	if (pcm_h) {
		alsa_pcm_drain(pcm_h);
		alsa_pcm_close(pcm_h);
		pcm_h = NULL;
	}
	MikMod_free(audiobuffer);
	audiobuffer = NULL;
}

static void ALSA_Exit(void)
{
	ALSA_Exit_internal();
#ifdef MIKMOD_DYNAMIC
	ALSA_Unlink();
#endif
}

/* Underrun and suspend recovery - from alsa-lib:test/pcm.c
 */
static int xrun_recovery(snd_pcm_t *handle, int err)
{
	if (err == -EPIPE) {	/* under-run */
		err = alsa_pcm_prepare(handle);
		if (err < 0)
			dbgprint(stderr, "Can't recover from underrun, prepare failed: %s\n", snd_strerror(err));
		return 0;
	}
	else if (err == -ESTRPIPE) {
		while ((err = alsa_pcm_resume(handle)) == -EAGAIN)
			sleep(1);	/* wait until the suspend flag is released */
		if (err < 0) {
			err = alsa_pcm_prepare(handle);
			if (err < 0)
				dbgprint(stderr, "Can't recover from suspend, prepare failed: %s\n", snd_strerror(err));
		}
		return 0;
	}
	return err;
}

static void ALSA_Update(void)
{
	int err;

	if (!enabled) return;

	if (bytes_written == 0 || bytes_played == bytes_written) {
		bytes_written = VC_WriteBytes(audiobuffer,period_size * global_frame_size);
		bytes_played = 0;
	}

	while (bytes_played < bytes_written)
	{
		err = alsa_pcm_writei(pcm_h, &audiobuffer[bytes_played], (bytes_written - bytes_played) / global_frame_size);
		if (err == -EAGAIN)
			continue;
		if (err < 0) {
			if ((err = xrun_recovery(pcm_h, err)) < 0) {
				_mm_errno = MMERR_ALSA_PCM_RECOVER;
				enabled = 0;
				dbgprint(stderr, "Write error: %s\n", alsa_strerror(err));
			}
			break;
		}
		bytes_played += err * global_frame_size;
	}
}

static int ALSA_PlayStart(void)
{
	int err;

	if (pcm_h == NULL) return 1;
	err = alsa_pcm_prepare(pcm_h);
	if (err == 0)
	    err = alsa_pcm_start(pcm_h);
	if (err < 0) {
		enabled = 0;
		_mm_errno = MMERR_ALSA_PCM_START;
		return 1;
	}

	return VC_PlayStart();
}

static void ALSA_PlayStop(void)
{
	VC_PlayStop();
	if (pcm_h) alsa_pcm_drop(pcm_h);
}

static int ALSA_Reset(void)
{
	ALSA_Exit_internal();
	return ALSA_Init_internal();
}

MIKMODAPI MDRIVER drv_alsa = {
	NULL,
	"ALSA",
	"Advanced Linux Sound Architecture (ALSA) driver v1.11",
	0,255,
	"alsa",
	NULL,
	ALSA_CommandLine,
	ALSA_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	ALSA_Init,
	ALSA_Exit,
	ALSA_Reset,
	VC_SetNumVoices,
	ALSA_PlayStart,
	ALSA_PlayStop,
	ALSA_Update,
	NULL,
	VC_VoiceSetVolume,
	VC_VoiceGetVolume,
	VC_VoiceSetFrequency,
	VC_VoiceGetFrequency,
	VC_VoiceSetPanning,
	VC_VoiceGetPanning,
	VC_VoicePlay,
	VC_VoiceStop,
	VC_VoiceStopped,
	VC_VoiceGetPosition,
	VC_VoiceRealVolume
};

#else

MISSING(drv_alsa);

#endif

/* ex:set ts=8: */
