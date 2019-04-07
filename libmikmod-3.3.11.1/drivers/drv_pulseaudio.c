/* MikMod sound library (c) 2003-2014 Raphael Assenat and others -
 * see AUTHORS file for a complete list.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/* PulseAudio driver for libmikmod, quick'n'dirty, using pa_simple api.
 * Written by O. Sezer <sezero@users.sourceforge.net>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_PULSEAUDIO

#include <string.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#define PA_NUMSAMPLES 256	/* a fair default for md_mixfreq <= 11025 Hz */

static BOOL enabled = 0;
static pa_simple *pasp;
static char *server = NULL;	/* Server name, or NULL for default */
static char *sink = NULL;	/* Sink (resp. source) name, or NULL for default */
static SBYTE *pabuf;
static ULONG pabufsize;

static void PULSEAUDIO_CommandLine(const CHAR *cmdline)
{
	MikMod_free(server);
	MikMod_free(sink);

	server = MD_GetAtom("server", cmdline, 0);
	if (server && !*server) {
		MikMod_free(server);
		server = NULL;
	}

	sink = MD_GetAtom("sink", cmdline, 0);
	if (sink && !*sink) {
		MikMod_free(sink);
		sink = NULL;
	}
}

static BOOL PULSEAUDIO_IsPresent(void)
{
	if (!pasp) {
		pa_sample_spec paspec;
		paspec.format = PA_SAMPLE_S16NE;
		paspec.rate = 22050;
		paspec.channels = 2;
		pasp = pa_simple_new (server, "libMikMod client", PA_STREAM_PLAYBACK,
					sink, "_mm_output_test", &paspec, NULL, NULL, NULL);
		if (!pasp) return 0;
		pa_simple_free(pasp);
		pasp = NULL;
	}
	return 1;
}

static int PULSEAUDIO_Init(void)
{
	pa_sample_spec paspec;
	int err;

	paspec.format = (md_mode & DMODE_FLOAT)? PA_SAMPLE_FLOAT32NE :
			(md_mode & DMODE_16BITS)? PA_SAMPLE_S16NE : PA_SAMPLE_U8;
	paspec.rate = md_mixfreq;
	paspec.channels = (md_mode & DMODE_STEREO) ? 2 : 1;

	pasp = pa_simple_new (server, "libMikMod client", PA_STREAM_PLAYBACK,
				sink, "libMikMod music", &paspec, NULL, NULL, &err);
	if (!pasp) {
#ifdef MIKMOD_DEBUG
		fprintf(stderr, "PulseAudio error: %s", pa_strerror(err));
#endif
		_mm_errno = MMERR_OPENING_AUDIO;
		return 1;
	}

	pabufsize = 	(md_mixfreq <= 11025) ? (PA_NUMSAMPLES    ) :
			(md_mixfreq <= 22050) ? (PA_NUMSAMPLES * 2) :
			(md_mixfreq <= 44100) ? (PA_NUMSAMPLES * 4) :
						(PA_NUMSAMPLES * 8);
	if (md_mode & DMODE_FLOAT) pabufsize *= 4;
	else if (md_mode & DMODE_16BITS) pabufsize *= 2;
	if (md_mode & DMODE_STEREO) pabufsize *= 2;

	if (!(pabuf = (SBYTE *) MikMod_malloc(pabufsize))) {
		pa_simple_free(pasp);
		pasp = NULL;
		_mm_errno = MMERR_OUT_OF_MEMORY;
		return 1;
	}

	enabled = 1;
	md_mode |= DMODE_SOFT_MUSIC;

	return VC_Init();
}

static void PULSEAUDIO_Exit(void)
{
	enabled = 0;
/*	pa_simple_drain(pasp, NULL);*/
	pa_simple_flush(pasp, NULL);
	pa_simple_free(pasp);
	pasp = NULL;
	MikMod_free(pabuf);
	pabuf = NULL;
	VC_Exit();
}

static int PULSEAUDIO_Reset(void)
{
	PULSEAUDIO_Exit();
	return PULSEAUDIO_Init();
}

static void PULSEAUDIO_Update(void)
{
	ULONG len;
	int err;

	len = VC_WriteBytes(pabuf, pabufsize);
	if (enabled) {
		if (pa_simple_write(pasp, pabuf, len, &err) < 0) {
			enabled = 0;
#ifdef MIKMOD_DEBUG
			fprintf(stderr, "PulseAudio write: %s", pa_strerror(err));
#endif
		}
	}
}

static void PULSEAUDIO_PlayStop(void)
{
	VC_PlayStop();
	if (pasp) pa_simple_flush(pasp, NULL);
}

static int PULSEAUDIO_PlayStart(void)
{
	if (!pasp) return 1;
	return VC_PlayStart();
}

MIKMODAPI struct MDRIVER drv_pulseaudio =
{
	NULL,
	"PulseAudio",
	"PulseAudio driver v0.1",
	0, 255,
	"pulseaudio",
	"server:t::PulseAudio server name\n"
	"sink:t::Sink (resp. source) name\n",
	PULSEAUDIO_CommandLine,
	PULSEAUDIO_IsPresent,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	PULSEAUDIO_Init,
	PULSEAUDIO_Exit,
	PULSEAUDIO_Reset,
	VC_SetNumVoices,
	PULSEAUDIO_PlayStart,
	PULSEAUDIO_PlayStop,
	PULSEAUDIO_Update,
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

MISSING(drv_pulseaudio);

#endif

/* ex:set ts=8: */
