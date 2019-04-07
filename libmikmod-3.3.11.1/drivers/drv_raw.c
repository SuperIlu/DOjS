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

  Driver for output to a file called MUSIC.RAW

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DRV_RAW

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>

#include "mikmod_internals.h"

#ifdef __VBCC__
#define unlink remove
#endif
#ifdef _WIN32
#define unlink _unlink
#endif

#define BUFFERSIZE 32768
#define FILENAME "music.raw"

static	FILE *rawout=NULL;
static	SBYTE *audiobuffer=NULL;
static	CHAR *filename=NULL;

static void RAW_CommandLine(const CHAR *cmdline)
{
	CHAR *ptr=MD_GetAtom("file",cmdline,0);

	if(ptr) {
		MikMod_free(filename);
		filename=ptr;
	}
}

static BOOL RAW_IsThere(void)
{
	return 1;
}

static int RAW_Init(void)
{
#if (MIKMOD_UNIX)
	if(!MD_Access(filename?filename:FILENAME)) {
		_mm_errno=MMERR_OPENING_FILE;
		return 1;
	}
#endif

	rawout=fopen(filename?filename:FILENAME,"wb");
	if(!rawout) {
		_mm_errno=MMERR_OPENING_FILE;
		return 1;
	}
	md_mode|=DMODE_SOFT_MUSIC|DMODE_SOFT_SNDFX;

	if (!(audiobuffer=(SBYTE*)MikMod_malloc(BUFFERSIZE))) {
		fclose(rawout);
		unlink(filename?filename:FILENAME);
		rawout=NULL;
		return 1;
	}

	if ((VC_Init())) {
		fclose(rawout);
		unlink(filename?filename:FILENAME);
		rawout=NULL;
		return 1;
	}
	return 0;
}

static void RAW_Exit(void)
{
	VC_Exit();
	if (rawout) {
		fclose(rawout);
		rawout=NULL;
	}
	MikMod_free(audiobuffer);
	audiobuffer = NULL;
}

static void RAW_Update(void)
{
	fwrite(audiobuffer,VC_WriteBytes(audiobuffer,BUFFERSIZE),1,rawout);
}

static int RAW_Reset(void)
{
	fclose(rawout);
	rawout=fopen(filename?filename:FILENAME,"wb");
	if(!rawout) {
		_mm_errno=MMERR_OPENING_FILE;
		return 1;
	}

	return 0;
}

MIKMODAPI MDRIVER drv_raw={
	NULL,
	"Disk writer (raw data)",
	"Raw disk writer (music.raw) v1.1",
	0,255,
	"raw",
	"file:t:music.raw:Output file name\n",
	RAW_CommandLine,
	RAW_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	RAW_Init,
	RAW_Exit,
	RAW_Reset,
	VC_SetNumVoices,
	VC_PlayStart,
	VC_PlayStop,
	RAW_Update,
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
MISSING(drv_raw);

#endif

/* ex:set ts=4: */
