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

  Driver for output to a file called MUSIC.WAV

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_WAV

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>

#ifdef SUNOS
extern int fclose(FILE *);
#endif
#ifdef __VBCC__
#define unlink remove
#endif
#ifdef _WIN32
#define unlink _unlink
#endif

#define BUFFERSIZE 32768
#define FILENAME "music.wav"

static	MWRITER *wavout=NULL;
static	FILE *wavfile=NULL;
static	SBYTE *audiobuffer=NULL;
static	ULONG dumpsize;
static	CHAR *filename=NULL;

static void putheader(void)
{
	ULONG rflen = 36;
	if (md_mode&DMODE_FLOAT)
		rflen += 2 + 12; /* FmtExt + "fact" chunk sizes */
	rflen += dumpsize;

	_mm_fseek(wavout,0,SEEK_SET);
	_mm_write_string("RIFF",wavout);
	_mm_write_I_ULONG(rflen,wavout);
	_mm_write_string("WAVEfmt ",wavout);
	_mm_write_I_ULONG((md_mode&DMODE_FLOAT)?18:16,wavout);	/* length of this RIFF block crap */

	_mm_write_I_UWORD((md_mode&DMODE_FLOAT)? 3:1,wavout);	/* WAVE_FORMAT_PCM :1, WAVE_FORMAT_IEEE_FLOAT :3 */
	_mm_write_I_UWORD((md_mode&DMODE_STEREO)?2:1,wavout);
	_mm_write_I_ULONG(md_mixfreq,wavout);
	_mm_write_I_ULONG(md_mixfreq *	((md_mode&DMODE_STEREO)?2:1) *
					((md_mode&DMODE_FLOAT)? 4:
					 (md_mode&DMODE_16BITS)?2:1), wavout);
	/* block alignment (8/16 bit) */
	_mm_write_I_UWORD(((md_mode&DMODE_FLOAT)?4:(md_mode&DMODE_16BITS)?2:1)*
	                  ((md_mode&DMODE_STEREO)?2:1),wavout);
	_mm_write_I_UWORD((md_mode&DMODE_FLOAT)?32:(md_mode&DMODE_16BITS)?16:8,wavout);

	if (md_mode&DMODE_FLOAT) {
		_mm_write_I_UWORD(0,wavout);						/* 0 byte of FmtExt */
		_mm_write_string("fact",wavout);
		_mm_write_I_ULONG(4,wavout);
		_mm_write_I_ULONG(dumpsize / ((md_mode&DMODE_STEREO)?2:1) /		/* # of samples written */
					     ((md_mode&DMODE_FLOAT)? 4:
					      (md_mode&DMODE_16BITS)?2:1), wavout);
	}

	_mm_write_string("data",wavout);
	_mm_write_I_ULONG(dumpsize,wavout);
}

static void WAV_CommandLine(const CHAR *cmdline)
{
	CHAR *ptr=MD_GetAtom("file",cmdline,0);

	if(ptr) {
		MikMod_free(filename);
		filename=ptr;
	}
}

static BOOL WAV_IsThere(void)
{
	return 1;
}

static int WAV_Init(void)
{
#if (MIKMOD_UNIX)
	if (!MD_Access(filename?filename:FILENAME)) {
		_mm_errno=MMERR_OPENING_FILE;
		return 1;
	}
#endif

	if(!(wavfile=fopen(filename?filename:FILENAME,"wb"))) {
		_mm_errno=MMERR_OPENING_FILE;
		return 1;
	}
	if(!(wavout=_mm_new_file_writer (wavfile))) {
		fclose(wavfile);unlink(filename?filename:FILENAME);
		wavfile=NULL;
		return 1;
	}
	if(!(audiobuffer=(SBYTE*)MikMod_malloc(BUFFERSIZE))) {
		_mm_delete_file_writer(wavout);
		fclose(wavfile);unlink(filename?filename:FILENAME);
		wavfile=NULL;wavout=NULL;
		return 1;
	}

	md_mode|=DMODE_SOFT_MUSIC|DMODE_SOFT_SNDFX;

	if (VC_Init()) {
		_mm_delete_file_writer(wavout);
		fclose(wavfile);unlink(filename?filename:FILENAME);
		wavfile=NULL;wavout=NULL;
		return 1;
	}
	dumpsize=0;
	putheader();

	return 0;
}

static void WAV_Exit(void)
{
	VC_Exit();

	/* write in the actual sizes now */
	if(wavout) {
		putheader();
		_mm_delete_file_writer(wavout);
		fclose(wavfile);
		wavfile=NULL;wavout=NULL;
	}
	MikMod_free(audiobuffer);
	audiobuffer=NULL;
}

static void WAV_Update(void)
{
	ULONG done;

	done=VC_WriteBytes(audiobuffer,BUFFERSIZE);

	if (md_mode & DMODE_FLOAT) {
	/* O.S. - assuming same endian model for integer vs fp values	*/
		_mm_write_I_ULONGS((ULONG *) audiobuffer,done>>2,wavout);
	}
	else if (md_mode & DMODE_16BITS) {
	/* <AWE> Fix for 16bit samples on big endian systems: Just swap
	 * bytes via "_mm_write_I_UWORDS ()" if we have 16 bit output.	*/
		_mm_write_I_UWORDS((UWORD *) audiobuffer,done>>1,wavout);
	}
	else {
		_mm_write_UBYTES(audiobuffer,done,wavout);
	}
	dumpsize+=done;
}

MIKMODAPI MDRIVER drv_wav={
	NULL,
	"Disk writer (wav)",
	"Wav disk writer (music.wav) v1.3",
	0,255,
	"wav",
	"file:t:music.wav:Output file name\n",
	WAV_CommandLine,
	WAV_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	WAV_Init,
	WAV_Exit,
	NULL,
	VC_SetNumVoices,
	VC_PlayStart,
	VC_PlayStop,
	WAV_Update,
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

MISSING(drv_wav);

#endif

/* ex:set ts=4: */
