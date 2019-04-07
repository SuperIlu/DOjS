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

  Driver for the Enlightened sound daemon (EsounD)

==============================================================================*/

/*

	You should set the hostname of the machine running esd in the environment
	variable 'ESPEAKER'. If this variable is not set, localhost is used.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_ESD

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef MIKMOD_DYNAMIC
#include <dlfcn.h>
#endif
#include <errno.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include <esd.h>

#ifdef MIKMOD_DYNAMIC
/* runtime link with libesd */
/* note that since we only need the network part of EsounD, we don't need to
   load libasound.so or libaudiofile.so as well */
static int(*esd_closestream)(int);
static int(*esd_playstream)(esd_format_t,int,const char*,const char*);
static void* libesd=NULL;
#ifndef HAVE_RTLD_GLOBAL
#define RTLD_GLOBAL (0)
#endif
#else /* MIKMOD_DYNAMIC */
/* compile-time link with libesd */
#define esd_closestream esd_close
#define esd_playstream esd_play_stream
#endif

#ifdef HAVE_SETENV
#define SETENV setenv("ESD_NO_SPAWN","1",0)
#else
static char ESDENV[] = "ESD_NO_SPAWN=1";
#define SETENV		putenv(ESDENV)
#endif

static	int sndfd=-1;
static	esd_format_t format;
static	CHAR *espeaker=NULL;

#ifdef MIKMOD_DYNAMIC
/* straight from audio.c in esd sources */
esd_format_t esd_audio_format=ESD_BITS16|ESD_STEREO;
int esd_audio_rate=ESD_DEFAULT_RATE;

static int ESD_Link(void)
{
	if (libesd) return 0;

	/* load libesd.so */
	libesd=dlopen("libesd.so.0",RTLD_LAZY|RTLD_GLOBAL);
	if (!libesd) libesd=dlopen("libesd.so",RTLD_LAZY|RTLD_GLOBAL);
	if (!libesd) return 1;

	/* resolve function references */
	if (!(esd_closestream = (int (*)(int)) dlsym(libesd,"esd_close"))) return 1;
	if (!(esd_playstream = (int (*)(esd_format_t,int,const char*,const char*))
					 dlsym(libesd,"esd_play_stream"))) return 1;

	return 0;
}

static void ESD_Unlink(void)
{
	esd_closestream=NULL;
	esd_playstream=NULL;

	if (libesd) {
		dlclose(libesd);
		libesd=NULL;
	}
}
#endif

/* A simple chunk fifo buffer, as we cannot query to ask how much free space
   there is with esd, we keep a fifo filled and try to write the entire fifo
   non blocking, stopping when we get -EAGAIN */

#define AUDIO_BUF_SIZE (ESD_BUF_SIZE * 4)

static SBYTE audiobuffer[AUDIO_BUF_SIZE];
static int audiobuffer_start, audiobuffer_end, audiobuffer_empty;

static void audiobuffer_init(void)
{
	audiobuffer_empty = 1;
	audiobuffer_start = audiobuffer_end = 0;
}

/* Get a free chunk of the fifo, the caller is expected to use all of it, so
   all of it gets marked filled */
static SBYTE *audiobuffer_get_free_chunk(int *size)
{
	int end = audiobuffer_end;

	if (audiobuffer_empty)
	{
		audiobuffer_empty = 0;
		*size = AUDIO_BUF_SIZE;
	}
	else if (audiobuffer_end > audiobuffer_start)
	{
		*size = AUDIO_BUF_SIZE - audiobuffer_end;
		audiobuffer_end = 0;
	}
	else
	{
		*size = audiobuffer_start - audiobuffer_end;
		audiobuffer_end = audiobuffer_start;
	}

	return audiobuffer + end;
}

/* Get a filled chunk from the fifo, the caller must call audiobuffer_mark_free
   to tell the fifo how much of the chunk it has consumed */
static SBYTE *audiobuffer_get_filled_chunk(int *size)
{
	if (audiobuffer_empty)
	{
		*size = 0;
	}
	else if (audiobuffer_end > audiobuffer_start)
	{
		*size = audiobuffer_end - audiobuffer_start;
	}
	else
	{
		*size = AUDIO_BUF_SIZE - audiobuffer_start;
	}

	return audiobuffer + audiobuffer_start;
}

/* Tell the fifo to mark size bytes as free starting from the current head of
   the fifo */
static void audiobuffer_mark_free(int size)
{
	audiobuffer_start = (audiobuffer_start + size) % AUDIO_BUF_SIZE;
	if (audiobuffer_start == audiobuffer_end)
	{
		audiobuffer_empty = 1;
		audiobuffer_start = audiobuffer_end = 0;
	}
}

/* I hope to have this function integrated into libesd someday...*/
static ssize_t esd_writebuf(int fd, const void *buffer, size_t count)
{
	ssize_t res;

	res = write(fd, buffer, count);
	if (res < 0 && errno == EAGAIN)
		return 0;

	return res;
}

static void ESD_CommandLine(const CHAR *cmdline)
{
	CHAR *ptr=MD_GetAtom("machine",cmdline,0);

	if (ptr) {
		MikMod_free(espeaker);
		espeaker=ptr;
	}
}

static BOOL ESD_IsThere(void)
{
	BOOL retval;
	int fd;

#ifdef MIKMOD_DYNAMIC
	if (ESD_Link()) return 0;
#endif
	/* Try to esablish a connection with default esd settings, but we don't
	   want esdlib to spawn esd if esd is not running ! */
	if (SETENV)
		retval=0;
	else {
		if ((fd=esd_playstream(ESD_BITS16|ESD_STEREO|ESD_STREAM|ESD_PLAY,
		                       ESD_DEFAULT_RATE,espeaker,"libmikmod"))<0)
			retval=0;
		else {
			esd_closestream(fd);
			retval=1;
		}
	}
#ifdef MIKMOD_DYNAMIC
	ESD_Unlink();
#endif
	return retval;
}

static int ESD_Init_internal(void)
{
	format=(md_mode&DMODE_16BITS?ESD_BITS16:ESD_BITS8)|
	       (md_mode&DMODE_STEREO?ESD_STEREO:ESD_MONO)|ESD_STREAM|ESD_PLAY;

	if (md_mixfreq > ESD_DEFAULT_RATE)
		md_mixfreq = ESD_DEFAULT_RATE;

	/* make sure we can open an esd stream with our parameters */
	if (!(SETENV)) {
		if ((sndfd=esd_playstream(format,md_mixfreq,espeaker,"libmikmod"))<0) {
			_mm_errno=MMERR_OPENING_AUDIO;
			return 1;
		}
		fcntl(sndfd, F_SETFL, fcntl(sndfd, F_GETFL) | O_NONBLOCK);
	} else {
		_mm_errno=MMERR_OUT_OF_MEMORY;
		return 1;
	}

	/* Initialize the audiobuffer */
	audiobuffer_init();

	return VC_Init();
}

static int ESD_Init(void)
{
#ifdef MIKMOD_DYNAMIC
	if (ESD_Link()) {
		_mm_errno=MMERR_DYNAMIC_LINKING;
		return 1;
	}
#endif
	return ESD_Init_internal();
}

static void ESD_Exit_internal(void)
{
	VC_Exit();
	if (sndfd>=0) {
		esd_closestream(sndfd);
		sndfd=-1;
		signal(SIGPIPE,SIG_DFL);
	}
}

static void ESD_Exit(void)
{
	ESD_Exit_internal();
#ifdef MIKMOD_DYNAMIC
	ESD_Unlink();
#endif
}

typedef ULONG (*VC_WriteBytesFunc)(SBYTE*, ULONG);

static void ESD_Update_internal(VC_WriteBytesFunc WriteBytes)
{
	static time_t losttime;

	if (sndfd>=0) {
		SBYTE *chunk;
		int size;
		ssize_t written;

		/* Fill fifo */
		chunk = audiobuffer_get_free_chunk(&size);
		while (size)
		{
			WriteBytes(chunk, size);
			chunk = audiobuffer_get_free_chunk(&size);
		}

		/* And write untill fifo empty, or we would block */
		chunk = audiobuffer_get_filled_chunk(&size);
		while (size)
		{
			written = esd_writebuf(sndfd, chunk, size);
			if (written < 0)
			{
				/* if we lost our connection with esd, clean up
				   and work as the nosound driver until we can
				   reconnect */
				esd_closestream(sndfd);
				sndfd = -1;
				signal(SIGPIPE, SIG_DFL);
				losttime = time(NULL);
				break;
			}

			/* Stop if the buffer is full */
			if (written == 0)
				break;

			audiobuffer_mark_free(written);

			chunk = audiobuffer_get_filled_chunk(&size);
		}
	} else {
		/* an alarm would be better, but then the library user could not use
		   alarm(2) himself... */
		if (time(NULL)-losttime>=5) {
			losttime=time(NULL);
			/* Attempt to reconnect every 5 seconds */
			if (!(SETENV))
				if ((sndfd=esd_playstream(format,md_mixfreq,espeaker,"libmikmod"))>=0) {
					/* reconnected, clear fifo (contains old sound) and recurse
					   to play sound */
					audiobuffer_init();
					fcntl(sndfd, F_SETFL, fcntl(sndfd, F_GETFL) | O_NONBLOCK);
					ESD_Update_internal(WriteBytes);
				}
		}
	}
}

static void ESD_Update(void)
{
	ESD_Update_internal(VC_WriteBytes);
}

static void ESD_Pause(void)
{
	ESD_Update_internal(VC_SilenceBytes);
}

static int ESD_PlayStart(void)
{
	if (sndfd<0)
		if (!(SETENV)) {
			if ((sndfd=esd_playstream(format,md_mixfreq,espeaker,"libmikmod"))<0) {
				_mm_errno=MMERR_OPENING_AUDIO;
				return 1;
			}
			fcntl(sndfd, F_SETFL, fcntl(sndfd, F_GETFL) | O_NONBLOCK);
		}
	/* since the default behaviour of SIGPIPE on most Unices is to kill the
	   program, we'll prefer handle EPIPE ourselves should the esd die - recent
	   esdlib use a do-nothing handler, which prevents us from receiving EPIPE,
	   so we override this */
	signal(SIGPIPE,SIG_IGN);

	/* silence buffers */
	VC_SilenceBytes(audiobuffer,ESD_BUF_SIZE);
	esd_writebuf(sndfd,audiobuffer,ESD_BUF_SIZE);
	audiobuffer_init();

	return VC_PlayStart();
}

static void ESD_PlayStop(void)
{
	if (sndfd>=0) {
		/* silence buffers */
		VC_SilenceBytes(audiobuffer,ESD_BUF_SIZE);
		esd_writebuf(sndfd,audiobuffer,ESD_BUF_SIZE);
		audiobuffer_init();

		signal(SIGPIPE,SIG_DFL);
	}

	VC_PlayStop();
}

static int ESD_Reset(void)
{
	ESD_Exit_internal();
	return ESD_Init_internal();
}

MIKMODAPI MDRIVER drv_esd={
	NULL,
	"Enlightened sound daemon",
	/* use the same version number as the EsounD release it works best with */
	"Enlightened sound daemon (EsounD) driver v0.2.38",
	0,255,
	"esd",
	"machine:t::Audio server machine (hostname:port)\n",
	ESD_CommandLine,
	ESD_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	ESD_Init,
	ESD_Exit,
	ESD_Reset,
	VC_SetNumVoices,
	ESD_PlayStart,
	ESD_PlayStop,
	ESD_Update,
	ESD_Pause,
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

MISSING(drv_esd);

#endif

/* ex:set ts=4: */
