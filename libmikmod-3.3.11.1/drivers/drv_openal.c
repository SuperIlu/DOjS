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

  $Id: drv_openal.c 35 2009-10-07 08:18:59Z kervala $

  Driver for OpenAL API

==============================================================================*/

/*

	Written by Cedric OCHS <kervala@gmail.com>
	DMODE_FLOAT support/misc fixes by: O.Sezer <sezero@users.sourceforge.net>

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_OPENAL

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#if defined(__APPLE__) && defined(__MACH__)
/* Mac OS X framework -- no alext.h */
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <OpenAL/MacOSX_OALExtensions.h>
#else
/* FIXME: Creative OpenAL 1.1 SDK has no alext.h and its headers
 * are not under AL/ directory.  Ugh..  However with OpenAL-soft
 * (http://kcat.strangesoft.net/openal.html), which is available
 * for windows too, all the headers are under the AL/ directory
 * and alext.h is present as well. */
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#endif

#ifndef AL_FORMAT_MONO_FLOAT32
#define AL_FORMAT_MONO_FLOAT32		0x10010
#endif
#ifndef AL_FORMAT_STEREO_FLOAT32
#define AL_FORMAT_STEREO_FLOAT32	0x10011
#endif

static ALCcontext *context;
static ALCdevice *device;

static ALuint* buffers = NULL;
static ALuint source = 0;
static ALenum format;

static ALsizei buffer_queue = 4;
static ULONG buffer_size = 32768;

static SBYTE *audiobuffer = NULL;

static BOOL OPENAL_check_error(int err)
{
	if (alGetError() == AL_NO_ERROR) return FALSE;

	if (err) _mm_errno = err;

	return TRUE;
}

static BOOL OPENAL_stream(ALuint buffer)
{
	if (!audiobuffer) return FALSE;

	/* write bytes to audio buffer */
	VC_WriteBytes(audiobuffer, buffer_size);

	/* copy audio buffer to OpenAL */
	alBufferData(buffer, format, audiobuffer, buffer_size, md_mixfreq);
	if (OPENAL_check_error(MMERR_OPENAL_BUFFERDATA)) return FALSE;

	return TRUE;
}

static BOOL set_format(void)
{
	/* need AL_EXT_float32 for DMODE_FLOAT */
	if (md_mode & DMODE_FLOAT)
	{
		if (!alIsExtensionPresent("AL_EXT_FLOAT32")) {
			_mm_errno = MMERR_NO_FLOAT32;
			return FALSE;
		}
		if (md_mode & DMODE_STEREO) format = AL_FORMAT_STEREO_FLOAT32;
		else format = AL_FORMAT_MONO_FLOAT32;
	}
	else /* standart S16 or U8 */
	if (md_mode & DMODE_STEREO)
	{
		if (md_mode & DMODE_16BITS) format = AL_FORMAT_STEREO16;
		else format = AL_FORMAT_STEREO8;
	}
	else
	{
		if (md_mode & DMODE_16BITS) format = AL_FORMAT_MONO16;
		else format = AL_FORMAT_MONO8;
	}
	return TRUE;
}

static void OPENAL_CommandLine(const CHAR *cmdline)
{
	CHAR *ptr;

	if((ptr=MD_GetAtom("buffersize",cmdline,0)) != NULL)
	{
		buffer_size = atoi(ptr);
		MikMod_free(ptr);
	}
	else
	{
		buffer_size = 32768;
	}

	if (buffer_size < 4096) buffer_size = 4096;

	if((ptr=MD_GetAtom("bufferqueue",cmdline,0)) != NULL)
	{
		buffer_queue = atoi(ptr);
		MikMod_free(ptr);
	}
	else
	{
		buffer_queue = 4;
	}

	if (buffer_queue < 2) buffer_queue = 2;
}

static BOOL OPENAL_IsPresent(void)
{
	if(!device)
	{
		device = alcOpenDevice(NULL);
		if (!device)
			return FALSE;
		alcCloseDevice(device);
		device = NULL;
	}
	return TRUE;
}

static int OPENAL_Init(void)
{
	/* open device and create context */
	device = alcOpenDevice(NULL);
	if(!device)
	{
		_mm_errno = MMERR_OPENING_AUDIO;
		return 1;
	}
	context = alcCreateContext(device, NULL);
	if (!context)
	{
		_mm_errno = MMERR_OPENAL_CREATECTX;
		return 1;
	}
	if (!alcMakeContextCurrent(context))
	{
		_mm_errno = MMERR_OPENAL_CTXCURRENT;
		return 1;
	}

	if (!set_format())
		return 1;

	buffers = (ALuint*)MikMod_malloc(buffer_queue * sizeof(ALuint));
	audiobuffer = (SBYTE*)MikMod_malloc(buffer_size * sizeof(SBYTE));
	if (!buffers || !audiobuffer)
	{
		_mm_errno = MMERR_OUT_OF_MEMORY;
		return 1;
	}

	alGetError();	/* clear last error */

	/* create buffers */
	alGenBuffers(buffer_queue, buffers);
	if (OPENAL_check_error(MMERR_OPENAL_GENBUFFERS)) return 1;

	/* create 1 source */
	alGenSources(1, &source);
	if (OPENAL_check_error(MMERR_OPENAL_GENSOURCES)) return 1;

	/* set source parameters */
	alSource3f(source, AL_POSITION, 0.0, 0.0, 0.0);
	alSource3f(source, AL_VELOCITY, 0.0, 0.0, 0.0);
	alSource3f(source, AL_DIRECTION, 0.0, 0.0, 0.0);
	alSourcef(source, AL_ROLLOFF_FACTOR, 0.0);
	alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
	if (OPENAL_check_error(MMERR_OPENAL_SOURCE)) return 1;

	return VC_Init();
}

static void OPENAL_Exit(void)
{
	int queued;
	ALuint buffer;

	if (!device || !context)
		goto cleanup;

	/* get queued buffers count */
	alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);

	/* stop playing source */
	alSourceStop(source);

	while (queued--)
		/* removing buffer from queue */
		alSourceUnqueueBuffers(source, 1, &buffer);

	alDeleteSources(1, &source);

	alDeleteBuffers(buffer_queue, buffers);

cleanup:
	VC_Exit();

	if (context)
	{
		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		context = NULL;
	}

	if (device)
	{
		alcCloseDevice(device);
		device = NULL;
	}

	MikMod_free(audiobuffer);
	audiobuffer = NULL;
	MikMod_free(buffers);
	buffers = NULL;
}

static void OPENAL_Update(void)
{
	int processed;
	ALuint buffer;
	ALenum state;

	/* get processed buffers count */
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
	if (OPENAL_check_error(MMERR_OPENAL_GETSOURCE)) return;

	while(processed--)
	{
		/* remove a buffer from queue */
		alSourceUnqueueBuffers(source, 1, &buffer);
		if (OPENAL_check_error(MMERR_OPENAL_UNQUEUEBUFFERS)) return;

		/* fill temporary buffer and copy it into OpenAL buffer */
		if (!OPENAL_stream(buffer)) return;

		/* put buffer in queue */
		alSourceQueueBuffers(source, 1, &buffer);
		if (OPENAL_check_error(MMERR_OPENAL_QUEUEBUFFERS)) return;
	}

	/* get source state */
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	if (OPENAL_check_error(MMERR_OPENAL_GETSOURCE)) return;

	/* if already playing, leave */
	if (state == AL_PLAYING) return;

	/* if initial state */
	if (state == AL_INITIAL)
	{
		ALsizei i;

		/* fill buffers */
		for(i = 0; i < buffer_queue; ++i)
			if (!OPENAL_stream(buffers[i])) return;

		/* put buffers in queue */
		alSourceQueueBuffers(source, buffer_queue, buffers);
		if (OPENAL_check_error(MMERR_OPENAL_QUEUEBUFFERS)) return;
	}

	/* play the source */
	alSourcePlay(source);
	OPENAL_check_error(MMERR_OPENAL_SOURCEPLAY);
}

static void OPENAL_Pause(void)
{
	/* pause the source */
	alSourcePause(source);
}

static int OPENAL_Reset(void)
{
	int queued;
	ALuint buffer;

	alSourceStop(source);
	if (OPENAL_check_error(MMERR_OPENAL_SOURCESTOP)) return 1;

	alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
	if (OPENAL_check_error(MMERR_OPENAL_GETSOURCE)) return 1;

	while (queued--)
	{
		alSourceUnqueueBuffers(source, 1, &buffer);
		if (OPENAL_check_error(MMERR_OPENAL_UNQUEUEBUFFERS)) return 1;
	}

	if (!set_format()) return 1;

	return 0;
}

static void OPENAL_PlayStop(void)
{
	alSourceStop(source);
	VC_PlayStop();
}

MIKMODAPI MDRIVER drv_openal =
{
	NULL,
	"OpenAL",
	"OpenAL driver v0.4",
	0,255,
	"openal",
	"buffersize:r:32768,16777216,32768:Buffer size\n"
		"bufferqueue:r:2,16,4:Buffer queue size\n",
	OPENAL_CommandLine,
	OPENAL_IsPresent,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	OPENAL_Init,
	OPENAL_Exit,
	OPENAL_Reset,
	VC_SetNumVoices,
	VC_PlayStart,
	OPENAL_PlayStop,
	OPENAL_Update,
	OPENAL_Pause,
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

MISSING(drv_openal);

#endif

/* ex:set ts=4: */
