/*	MikMod sound library
	(c) 2004, Raphael Assenat
	(c) 1998, 1999, 2000, 2001 Miodrag Vallat and others - see file AUTHORS
	for complete list.

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

  WAV sample loader

==============================================================================*/

/*
   FIXME: Stereo .WAV files are not yet supported as samples.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string.h>

#include "mikmod_internals.h"

#ifdef SUNOS
extern int fprintf(FILE *, const char *, ...);
#endif

static void extract_channel(const char *src, char *dst, int num_chan, int num_samples, int samp_size, int channel);


typedef struct WAV {
	CHAR  rID[4];
	ULONG rLen;
	CHAR  wID[4];
	CHAR  fID[4];
	ULONG fLen;
	UWORD wFormatTag;
	UWORD nChannels;
	ULONG nSamplesPerSec;
	ULONG nAvgBytesPerSec;
	UWORD nBlockAlign;
	UWORD nFormatSpecific;
} WAV;

static BOOL isWaveFile(MREADER* reader)
{
	WAV wh;

	_mm_fseek(reader, SEEK_SET, 0);

	/* read wav header */
	_mm_read_string(wh.rID,4,reader);
	wh.rLen = _mm_read_I_ULONG(reader);
	_mm_read_string(wh.wID,4,reader);

	/* check for correct header */
	if(_mm_eof(reader)|| memcmp(wh.rID,"RIFF",4) || memcmp(wh.wID,"WAVE",4)) {
		return 0;
	}
	return 1;
}

static SAMPLE* Sample_LoadGeneric_internal_wav(MREADER* reader)
{
	SAMPLE *si=NULL;
	WAV wh;
	BOOL have_fmt=0;

	_mm_fseek(reader, SEEK_SET, 0);

	/* read wav header */
	_mm_read_string(wh.rID,4,reader);
	wh.rLen = _mm_read_I_ULONG(reader);
	_mm_read_string(wh.wID,4,reader);

	/* check for correct header */
	if(_mm_eof(reader)|| memcmp(wh.rID,"RIFF",4) || memcmp(wh.wID,"WAVE",4)) {
		_mm_errno = MMERR_UNKNOWN_WAVE_TYPE;
		return NULL;
	}

	/* scan all RIFF blocks until we find the sample data */
	for(;;) {
		CHAR dID[4];
		ULONG len,start;

		_mm_read_string(dID,4,reader);
		len = _mm_read_I_ULONG(reader);
		/* truncated file ? */
		if (_mm_eof(reader)) {
			_mm_errno=MMERR_UNKNOWN_WAVE_TYPE;
			return NULL;
		}
		start = _mm_ftell(reader);

		/* sample format block
		   should be present only once and before a data block */
		if(!memcmp(dID,"fmt ",4)) {
			wh.wFormatTag      = _mm_read_I_UWORD(reader);
			wh.nChannels       = _mm_read_I_UWORD(reader);
			wh.nSamplesPerSec  = _mm_read_I_ULONG(reader);
			wh.nAvgBytesPerSec = _mm_read_I_ULONG(reader);
			wh.nBlockAlign     = _mm_read_I_UWORD(reader);
			wh.nFormatSpecific = _mm_read_I_UWORD(reader);

#ifdef MIKMOD_DEBUG
			fprintf(stderr,"\rwavloader : wFormatTag=%04x blockalign=%04x nFormatSpc=%04x\n",
				wh.wFormatTag,wh.nBlockAlign,wh.nFormatSpecific);
#endif

			if((have_fmt)||(wh.nChannels>1)) {
				_mm_errno=MMERR_UNKNOWN_WAVE_TYPE;
				return NULL;
			}
			have_fmt=1;
		} else
		/* sample data block
		   should be present only once and after a format block */
		  if(!memcmp(dID,"data",4)) {
			if(!have_fmt) {
				_mm_errno=MMERR_UNKNOWN_WAVE_TYPE;
				return NULL;
			}
			if(!(si=(SAMPLE*)MikMod_malloc(sizeof(SAMPLE)))) return NULL;
			si->speed  = wh.nSamplesPerSec/wh.nChannels;
			si->volume = 64;
			si->length = len;
			if(wh.nBlockAlign == 2) {
				si->flags    = SF_16BITS | SF_SIGNED;
				si->length >>= 1;
			}
			si->inflags = si->flags;
			SL_RegisterSample(si,MD_SNDFX,reader);
			SL_LoadSamples();

			/* skip any other remaining blocks - so in case of repeated sample
			   fragments, we'll return the first anyway instead of an error */
			break;
		}
		/* onto next block */
		_mm_fseek(reader,start+len,SEEK_SET);
		if (_mm_eof(reader))
			break;
	}

	return si;
}

static SAMPLE* Sample_LoadRawGeneric_internal(MREADER* reader, ULONG rate, ULONG channel, ULONG flags)
{
	SAMPLE *si;
	long len;
	int samp_size=1;

	if(!(si=(SAMPLE*)MikMod_malloc(sizeof(SAMPLE)))) return NULL;

	/* length */
	_mm_fseek(reader, 0, SEEK_END);
	len = _mm_ftell(reader);

	si->panning = PAN_CENTER;
	si->speed = rate/1;
	si->volume = 64;
	si->length = len;
	si->loopstart=0;
	si->loopend = len;
	si->susbegin = 0;
	si->susend = 0;
	si->inflags = si->flags = flags;
	if (si->flags & SF_16BITS) {
		si->length >>= 1;
		si->loopstart >>= 1;
		si->loopend >>= 1;
		samp_size = 2;
	}

	if (si->flags & SF_STEREO)
	{
		char *data, *channel_data;
		int num_samp = si->length/samp_size/2;
		MREADER *chn_reader;

		data = (char*)MikMod_malloc(si->length);
		if (!data) { MikMod_free(si); return NULL; }

		channel_data = (char*)MikMod_malloc(si->length/2);
		if (!channel_data) { MikMod_free(data); MikMod_free(si); return NULL; }

		/* load the raw samples completely, and fully extract the
		 * requested channel. Create a memory reader pointing to
		 * the channel data. */
		_mm_fseek(reader, 0, SEEK_SET);
		reader->Read(reader, data, si->length);

		extract_channel(data, channel_data, 2, num_samp, samp_size, channel);
		chn_reader = _mm_new_mem_reader(channel_data, num_samp * samp_size);
		if (!chn_reader) {
			MikMod_free(channel_data);
			MikMod_free(data);
			MikMod_free(si);
			return NULL;
		}

		/* half of the samples were in the other channel */
		si->loopstart=0;
		si->length=num_samp;
		si->loopend=num_samp;

		SL_RegisterSample(si, MD_SNDFX, chn_reader);
		SL_LoadSamples();

		_mm_delete_mem_reader(chn_reader);
		MikMod_free(channel_data);
		MikMod_free(data);

		return si;
	}

	_mm_fseek(reader, 0, SEEK_SET);
	SL_RegisterSample(si, MD_SNDFX, reader);
	SL_LoadSamples();

	return si;
}

static SAMPLE* Sample_LoadGeneric_internal(MREADER* reader, const char *options)
{
	if (isWaveFile(reader)) {
		return Sample_LoadGeneric_internal_wav(reader);
	}

	return NULL;
}

MIKMODAPI SAMPLE* Sample_LoadRawGeneric(MREADER* reader, ULONG rate, ULONG channel, ULONG flags)
{
	SAMPLE* result;

	MUTEX_LOCK(vars);
	result = Sample_LoadRawGeneric_internal(reader, rate, channel, flags);
	MUTEX_UNLOCK(vars);

	return result;
}

MIKMODAPI SAMPLE *Sample_LoadRawMem(const char *buf, int len, ULONG rate, ULONG channel, ULONG flags)
{
	SAMPLE *result=NULL;
	MREADER *reader;

	if (!buf || len <= 0) return NULL;
	if ((reader=_mm_new_mem_reader(buf, len)) != NULL) {
		result=Sample_LoadRawGeneric(reader, rate, channel, flags);
		_mm_delete_mem_reader(reader);
	}
	return result;
}

MIKMODAPI SAMPLE* Sample_LoadRawFP(FILE *fp, ULONG rate, ULONG channel, ULONG flags)
{
	SAMPLE* result=NULL;
	MREADER* reader;

	if(fp && (reader=_mm_new_file_reader(fp)) != NULL) {
		result=Sample_LoadRawGeneric(reader, rate, channel, flags);
		_mm_delete_file_reader(reader);
	}
	return result;
}

MIKMODAPI SAMPLE* Sample_LoadRaw(const CHAR* filename, ULONG rate, ULONG channel, ULONG flags)
{
	FILE *fp;
	SAMPLE *si=NULL;

	if(!(md_mode & DMODE_SOFT_SNDFX)) return NULL;
	if((fp=_mm_fopen(filename,"rb")) != NULL) {
		si = Sample_LoadRawFP(fp, rate, channel, flags);
		_mm_fclose(fp);
	}
	return si;
}

MIKMODAPI SAMPLE* Sample_LoadGeneric(MREADER* reader)
{
	SAMPLE* result;

	MUTEX_LOCK(vars);
	result=Sample_LoadGeneric_internal(reader, NULL);
	MUTEX_UNLOCK(vars);

	return result;
}

MIKMODAPI SAMPLE *Sample_LoadMem(const char *buf, int len)
{
	SAMPLE* result=NULL;
	MREADER* reader;

	if (!buf || len <= 0) return NULL;
	if ((reader=_mm_new_mem_reader(buf, len)) != NULL) {
		result=Sample_LoadGeneric(reader);
		_mm_delete_mem_reader(reader);
	}
	return result;
}

MIKMODAPI SAMPLE* Sample_LoadFP(FILE *fp)
{
	SAMPLE* result=NULL;
	MREADER* reader;

	if(fp && (reader=_mm_new_file_reader(fp)) != NULL) {
		result=Sample_LoadGeneric(reader);
		_mm_delete_file_reader(reader);
	}
	return result;
}

MIKMODAPI SAMPLE* Sample_Load(const CHAR* filename)
{
	FILE *fp;
	SAMPLE *si=NULL;

	if(!(md_mode & DMODE_SOFT_SNDFX)) return NULL;
	if((fp=_mm_fopen(filename,"rb")) != NULL) {
		si = Sample_LoadFP(fp);
		_mm_fclose(fp);
	}
	return si;
}

MIKMODAPI void Sample_Free(SAMPLE* si)
{
	if(si) {
		if (si->onfree) si->onfree(si->ctx);
		MD_SampleUnload(si->handle);
		MikMod_free(si);
	}
}

void Sample_Free_internal(SAMPLE *si)
{
	MUTEX_LOCK(vars);
	Sample_Free(si);
	MUTEX_UNLOCK(vars);
}

static void extract_channel(const char *src, char *dst, int num_chan, int num_samples, int samp_size, int channel)
{
	int i;
#ifdef MIKMOD_DEBUG
	fprintf(stderr,"Extract channel: %p %p, num_chan=%d, num_samples=%d, samp_size=%d, channel=%d\n",
		src,dst,num_chan,num_samples,samp_size,channel);
#endif
	src += channel * samp_size;

	while (num_samples--)
	{
		for (i=0; i<samp_size; i++) {
			dst[i] = src[i];
		}
		src += samp_size * num_chan;
		dst += samp_size;
	}
}

/* ex:set ts=8: */
