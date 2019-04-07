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

  $Id: drv_dc.c,v 1.3 2011/08/05 20:30:00 raph Exp $

  Driver that outputs a stream to the AICA SPU of a Dreamcast.

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_DC

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <kos.h>
#include <dc/sound/stream.h>

/* Global variables */
static SBYTE sound_buffer[SND_STREAM_BUFFER_MAX]; /* Limit by SPU of Dreamcast */
static snd_stream_hnd_t shnd = -1;

static volatile int playing = 0;

static void DC_CommandLine(const CHAR *cmdline)
{
}

static BOOL DC_IsThere(void)
{
	return 1;/* Always valid */
}

/* Stop SPU */
static void DC_PlayStop(void)
{
	if (shnd!=-1) snd_stream_stop(shnd);
	playing = 0;
	VC_PlayStop();
}

/* Init SPU */
static int DC_PlayStart(void)
{
	snd_stream_start(shnd, md_mixfreq, 1);
	playing = 1;
	if (VC_PlayStart())
		return 1;
	return 0;
}

/* Stop the buffer, and free resources */
static void DC_Exit(void)
{
	if (shnd != -1) snd_stream_stop(shnd);
	playing = 0;
	/* Free the SPU stream */
	if (shnd != -1) {
		snd_stream_destroy(shnd);
		shnd = -1;
	}
	snd_stream_shutdown();
	sq_clr(sound_buffer, 65536);
	VC_Exit();
}

/* Called by Mikmod, means that we should get data from the buffer. */
static void DC_Update(void)
{
	snd_stream_poll(shnd); /* Request more data using the DC callback */
}

/* Function called by the Dreamcast SPU (polling).
 * Retrieve the buffer information into "sound_buffer" variable. */
static void *sound_callback(snd_stream_hnd_t hnd, int len, int *actual)
{
	if (playing) {
		*actual = VC_WriteBytes(sound_buffer, len);
	}
	else {
		memset(sound_buffer, 0, len);
		*actual = len;
	}

	return (uint16 *)(sound_buffer);
}

static int DC_Reset(void)
{
	if (shnd != -1) snd_stream_reinit(shnd, sound_callback);
	return 0;
}

/* Init the SPU and the buffers */
/* MikMod variables: _mm_errno, md_mode */
static int DC_Init(void)
{
	if (VC_Init()) {
		return 1;
	}

	/* spu_wave_stream: beginning */
	if (snd_stream_init() < 0) {
		_mm_errno = MMERR_OPENING_AUDIO;
		return 1;
	}

	/* Prepare buffer with a callback */
	shnd = snd_stream_alloc(sound_callback, SND_STREAM_BUFFER_MAX);
	if (shnd == SND_STREAM_INVALID) {
		_mm_errno = MMERR_OPENING_AUDIO;
		return 1;
	}

	/* Mikmod mode */
	/* md_mixfreq by default is 44100 (depends of app). */
	md_mode |= DMODE_STEREO;

	return 0;
}

MIKMODAPI MDRIVER drv_dc = {
	NULL,
	"DC",
	"Dreamcast AICA SPU driver v1.0",
	0,255,
	"dc",
	NULL,
	DC_CommandLine,
	DC_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	DC_Init,
	DC_Exit,
	DC_Reset,
	VC_SetNumVoices,
	DC_PlayStart,
	DC_PlayStop,
	DC_Update,
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

MISSING(drv_dc);

#endif

/* ex:set ts=4: */
