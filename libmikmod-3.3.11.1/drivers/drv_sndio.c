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

/* drv_sndio for OpenBSD:
 * Copyright (c) 2009 Jacob Meuser <jakemsr@sdf.lonestar.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_SNDIO

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sndio.h>

#define DEFAULT_FRAGSIZE 12

static struct sio_hdl *hdl;
static struct sio_par par;
static int fragsize = 1 << DEFAULT_FRAGSIZE;
static SBYTE *audiobuffer = NULL;

static void Sndio_CommandLine(const CHAR *cmdline)
{
	CHAR *ptr = MD_GetAtom("buffer", cmdline, 0);

	if (!ptr)
		fragsize = 1 << DEFAULT_FRAGSIZE;
	else {
		int n = atoi(ptr);
		if (n < 7 || n > 17)
			n = DEFAULT_FRAGSIZE;
		fragsize = 1 << n;
		MikMod_free(ptr);
	}
}

static BOOL Sndio_IsThere(void)
{
	if (!hdl) {
		struct sio_hdl *h = sio_open(NULL, SIO_PLAY, 0);
		if (!h) return 0;
		sio_close(h);
	}
	return 1;
}

static int Sndio_Init(void)
{
	hdl = sio_open(NULL, SIO_PLAY, 0);
	if (hdl == NULL) {
		_mm_errno = MMERR_OPENING_AUDIO;
		return 1;
	}

	if (!(audiobuffer = (SBYTE *)MikMod_malloc(fragsize))) {
		_mm_errno = MMERR_OUT_OF_MEMORY;
		return 1;
	}

	sio_initpar(&par);
	par.bits = (md_mode & DMODE_16BITS) ? 16 : 8;
	par.pchan = (md_mode & DMODE_STEREO) ? 2 : 1;
	par.rate = md_mixfreq;
	par.le = SIO_LE_NATIVE;
	par.sig = (par.bits == 8) ? 0 : 1;
	par.appbufsz = 4 * fragsize / SIO_BPS(par.bits) / par.pchan;

	if (!sio_setpar(hdl, &par) || !sio_getpar(hdl, &par)) {
		_mm_errno = MMERR_SNDIO_SETPARAMS;
		return 1;
	}

	/* Align to what the card gave us */
	if (par.rate <= 65535)
		md_mixfreq = par.rate;
	else	goto _bad_parms;
	if (par.bits == 8)
		md_mode &= ~(DMODE_16BITS);
	else if (par.bits == 16)
		md_mode |= DMODE_16BITS;
	else	goto _bad_parms;
	if (par.pchan == 1)
		md_mode &= ~(DMODE_STEREO);
	else if (par.pchan == 2)
		md_mode |= DMODE_STEREO;
	else	goto _bad_parms;

	return VC_Init();

_bad_parms:
	_mm_errno = MMERR_SNDIO_BADPARAMS;
	return 1;
}

static void Sndio_Exit(void)
{
	if (hdl) {
		sio_close(hdl);
		hdl = NULL;
	}
	VC_Exit();
	MikMod_free(audiobuffer);
	audiobuffer = NULL;
}

static void Sndio_Update(void)
{
	ULONG done = VC_WriteBytes(audiobuffer, fragsize);
	sio_write(hdl, audiobuffer, done);
}

static void Sndio_Pause(void)
{
	ULONG done = VC_SilenceBytes(audiobuffer, fragsize);
	sio_write(hdl, audiobuffer, done);
}

static int Sndio_PlayStart(void)
{
	if (!sio_start(hdl))
		return 1;
	return VC_PlayStart();
}

static void Sndio_PlayStop(void)
{
	VC_PlayStop();
	sio_stop(hdl);
}

MIKMODAPI MDRIVER drv_sndio = {
	NULL,
	"OpenBSD sndio",
	"OpenBSD sndio driver v1.01",
	0, 255,
	"sndio",
	"buffer:r:7,17,12:Audio buffer log2 size\n",
	Sndio_CommandLine,
	Sndio_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	Sndio_Init,
	Sndio_Exit,
	NULL,
	VC_SetNumVoices,
	Sndio_PlayStart,
	Sndio_PlayStop,
	Sndio_Update,
	Sndio_Pause,
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

MISSING(drv_sndio);

#endif

/* ex:set ts=8: */
