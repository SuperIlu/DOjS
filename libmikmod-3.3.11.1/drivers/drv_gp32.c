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

  Driver for output on gp32 platform
  Originally from mikplay32 source by aj0 - http://www.cs.vu.nl/~cvwalta/
  http://www.cs.vu.nl/~cvwalta/downloads/mikplay_src.rar
  http://web.archive.org/web/20031018223157/http://www.cs.vu.nl/~cvwalta/?option=articles/gp32

  Altered from mikplay source by aj0 for Mr.Mirkos SDK replacement
  and updated to mikmod 3.2.0 --- PEA (www.gp32.co.nz), March 2005
  http://www.pea.co.nz/gp32/downloads.php
  http://web.archive.org/web/20050830081635/http://www.pea.co.nz/gp32/downloads.php
  http://dl.openhandhelds.org/cgi-bin/gp32.cgi?0,0,0,0,46,632

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DRV_GP32

#ifndef GP32
#define GP32 1
#endif

#include <stddef.h>
#define size_t size_t

#include <gp32.h>
#include "mikmod_internals.h"

#define GP32_buffersize 2048
#define GP32_ringsize (GP32_buffersize*2)

static SBYTE *GP32_buffer = NULL;

static BOOL GP32_IsThere(void) {
	return 1;
}

static int GP32_Init(void) {
	md_mode = DMODE_STEREO | DMODE_16BITS | DMODE_SOFT_MUSIC | DMODE_SOFT_SNDFX;
	md_mixfreq = 44100;
	gp_initSound(44100, 16, GP32_ringsize); /* 44k sound, 16bpp, 2x4k buffers */
	GP32_buffer = (SBYTE *) MikMod_malloc(GP32_buffersize); /* Half of the 8k ringbuffer */
	if (!GP32_buffer) return 1;
	gp_clearRingbuffer();
	return VC_Init();
}

static void GP32_Stop(void) {
	gp_clearRingbuffer();
}

static void GP32_Restart(void) {
	/* No restart yet */
}

static void GP32_Exit(void) {
	gp_clearRingbuffer();
	MikMod_free(GP32_buffer);
	GP32_buffer=NULL;
	VC_Exit();
}

static void GP32_Update(void) {
	unsigned long bytesread;
#ifdef GP32_DEBUG
/*	gp_debug( 1, "Read bytes" );*/
#endif
	bytesread = VC_WriteBytes(GP32_buffer, GP32_buffersize);
#ifdef GP32_DEBUG
/*	gp_debug(1, "Read %d bytes", bytesread);*/
#endif
	if (!bytesread) return; /* exit if a whole buffer hasn't been read */

	gp_addRingsegment((unsigned short*)GP32_buffer);
}

static void GP32_PlayStop(void) {
	gp_clearRingbuffer();
	VC_PlayStop();
}

MIKMODAPI MDRIVER drv_gp32={
	NULL,
	"GP32 SDK Audio v0.2",
	"GP32 SDK Audio driver v0.2",
	0,255,
	"gp32",
	NULL,
	NULL,
	GP32_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	GP32_Init,
	GP32_Exit,
	NULL,
	VC_SetNumVoices,
	VC_PlayStart,
	GP32_PlayStop,
	GP32_Update,
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

#include "mikmod_internals.h"
MISSING(drv_gp32);

#endif

/* ex:set ts=4: */
