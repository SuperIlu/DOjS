/*	MikMod sound library
	(c) 2004, Raphael Assenat and others - see file AUTHORS for
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

  ASYLUM Music Format v1.0 (.amf) loader
  adapted from load_mod.c by Raphael Assenat <raph@raphnet.net>,
  with the help of the AMF2MOD utility sourcecode,
  written to convert crusader's amf files into 8
  channels mod file in 1995 by Mr. P / Powersource
  mrp@fish.share.net, ac054@sfn.saskatoon.sk.ca


==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string.h>

#include "mikmod_internals.h"

/*========== Module structure */

typedef struct MSAMPINFO {
	CHAR samplename[24];
	UBYTE finetune;
	UBYTE volume;
	ULONG length;
	ULONG reppos;
	ULONG replen;
} MSAMPINFO;

typedef struct MODULEHEADER {
	CHAR songname[21];
	UBYTE num_patterns;	/* number of patterns used */
	UBYTE num_orders;
	UBYTE positions[256];	/* which pattern to play at pos */
	MSAMPINFO samples[64];	/* all sampleinfo */
} MODULEHEADER;

typedef struct MODTYPE {
	CHAR id[5];
	UBYTE channels;
	CHAR *name;
} MODTYPE;

typedef struct MODNOTE {
	UBYTE a, b, c, d;
} MODNOTE;

/* This table is taken from AMF2MOD.C
 * written in 1995 by Mr. P / Powersource
 * mrp@fish.share.net, ac054@sfn.saskatoon.sk.ca */
static const UWORD periodtable[] = {
	6848,6464,6096,5760,5424,5120,4832,4560,4304,
	4064,3840,3628,3424,3232,3048,2880,2712,2560,
	2416,2280,2152,2032,1920,1814,1712,1616,1524,
	1440,1356,1280,1208,1140,1076,1016, 960, 907,
	856, 808, 762, 720, 678, 640, 604, 570, 538,
	508, 480, 453, 428, 404, 381, 360, 339, 320,
	302, 285, 269, 254, 240, 226, 214, 202, 190,
	180, 170, 160, 151, 143, 135, 127, 120, 113,
	107, 101,  95,  90,  85,  80,  75,  71,  67,
	63,  60,  56,  53,  50,  47,  45,  42,  40,
	37,  35,  33,  31,  30,  28};

/*========== Loader variables */

static CHAR asylum[] = "Asylum 1.0";

static MODULEHEADER *mh = NULL;
static MODNOTE *patbuf = NULL;
static int modtype = 0;

/*========== Loader code */

static BOOL ASY_CheckType(UBYTE *id, UBYTE *numchn, CHAR **descr)
{
	if (!memcmp(id, "ASYLUM Music Format V1.0", 24))
	{
		*descr = asylum;
		*numchn = 8;
		modtype = 1;
		return 1;
	}

	return 0;
}

static BOOL ASY_Test(void)
{
	UBYTE namestring[24], numchn;
	CHAR *descr;

	/* Read the magic string */
	_mm_fseek(modreader, 0, SEEK_SET);
	if (!_mm_read_UBYTES(namestring, 24, modreader))
		return 0;

	/* Test if the string is what we expect */
	if (ASY_CheckType(namestring, &numchn, &descr))
		return 1;

	return 0;
}

static BOOL ASY_Init(void)
{
	if (!(mh = (MODULEHEADER *)MikMod_malloc(sizeof(MODULEHEADER))))
		return 0;
	return 1;
}

static void ASY_Cleanup(void)
{
	MikMod_free(mh);
	MikMod_free(patbuf);
	mh = NULL;
	patbuf = NULL;
}

static void ConvertNote(MODNOTE *n)
{
	UBYTE instrument, effect, effdat, note;
	UWORD period;
	UBYTE lastnote = 0;

	instrument = n->b&0x1f;
	effect = n->c;
	effdat = n->d;

	/* convert amf note to mod period */
	if (n->a) {
		period = periodtable[n->a];
	} else {
		period = 0;
	}

	/* Convert the period to a note number */
	note = 0;
	if (period)
	{
		for (note = 0; note < 7 * OCTAVE; note++)
			if (period >= npertab[note])
				break;
		if (note == 7 * OCTAVE)
			note = 0;
		else
			note++;
	}

	if (instrument) {
		/* if instrument does not exist, note cut */
		if ((instrument > 31) || (!mh->samples[instrument - 1].length)) {
			UniPTEffect(0xc, 0);
			if (effect == 0xc)
				effect = effdat = 0;
		} else {
			/* Protracker handling */
			if (!modtype) {
				/* if we had a note, then change instrument...*/
				if (note)
					UniInstrument(instrument - 1);
				/* ...otherwise, only adjust volume... */
				else {
					/* ...unless an effect was specified,
					 * which forces a new note to be
					 * played */
					if (effect || effdat) {
						UniInstrument(instrument - 1);
						note = lastnote;
					} else
						UniPTEffect(0xc,
							mh->samples[instrument -
							1].volume & 0x7f);
				}
			} else {
				/* Fasttracker handling */
				UniInstrument(instrument - 1);
				if (!note)
					note = lastnote;
			}
		}
	}
	if (note) {
		UniNote(note + 2 * OCTAVE - 1);
		lastnote = note;
	}

	/* Convert pattern jump from Dec to Hex */
	if (effect == 0xd)
		effdat = (((effdat & 0xf0) >> 4) * 10) + (effdat & 0xf);

	/* Volume slide, up has priority */
	if ((effect == 0xa) && (effdat & 0xf) && (effdat & 0xf0))
		effdat &= 0xf0;

	UniPTEffect(effect, effdat);
}

static UBYTE *ConvertTrack(MODNOTE *n)
{
	int t;

	UniReset();
	for (t = 0; t < 64; t++) {
		ConvertNote(n);
		UniNewline();
		n += of.numchn;
	}
	return UniDup();
}

/* Loads all patterns of a modfile and converts them into the 3 byte format. */
static BOOL ML_LoadPatterns(void)
{
	int t, s, tracks = 0;

	if (!AllocPatterns()) {
		return 0;
	}
	if (!AllocTracks()) {
		return 0;
	}

	/* Allocate temporary buffer for loading and converting the patterns */
	if (!(patbuf = (MODNOTE *)MikMod_calloc(64U * of.numchn, sizeof(MODNOTE))))
		return 0;


	/* patterns start here */
	_mm_fseek(modreader, 0xA66, SEEK_SET);
	for (t = 0; t < of.numpat; t++) {
		/* Load the pattern into the temp buffer and convert it */
		for (s = 0; s < (64U * of.numchn); s++) {
			patbuf[s].a = _mm_read_UBYTE(modreader);
			patbuf[s].b = _mm_read_UBYTE(modreader);
			patbuf[s].c = _mm_read_UBYTE(modreader);
			patbuf[s].d = _mm_read_UBYTE(modreader);
		}
		for (s = 0; s < of.numchn; s++) {
			if (!(of.tracks[tracks++] = ConvertTrack(patbuf + s))) {
				return 0;
			}
		}
	}
	return 1;
}

static BOOL ASY_Load(BOOL curious)
{
	int t;
	SAMPLE *q;
	MSAMPINFO *s;
	CHAR *descr=asylum;
	ULONG seekpos;

	/* no title in asylum amf files :( */
	mh->songname[0] = '\0';

	_mm_fseek(modreader, 0x23, SEEK_SET);
	mh->num_patterns = _mm_read_UBYTE(modreader);
	mh->num_orders = _mm_read_UBYTE(modreader);

	/* skip unknown byte */
	_mm_skip_BYTE(modreader);
	_mm_read_UBYTES(mh->positions, 256, modreader);

	/* read samples headers*/
	for (t = 0; t < 64; t++) {
		s = &mh->samples[t];

		_mm_fseek(modreader, 0x126 + (t*37), SEEK_SET);

		_mm_read_string(s->samplename, 22, modreader);
		s->samplename[21] = 0;	/* just in case */

		s->finetune = _mm_read_UBYTE(modreader);
		s->volume = _mm_read_UBYTE(modreader);
		_mm_skip_BYTE(modreader);/* skip unknown byte */
		s->length = _mm_read_I_ULONG(modreader);
		s->reppos = _mm_read_I_ULONG(modreader);
		s->replen = _mm_read_I_ULONG(modreader);
	}

	if (_mm_eof(modreader)) {
		_mm_errno = MMERR_LOADING_HEADER;
		return 0;
	}

	/* set module variables */
	of.initspeed = 6;
	of.inittempo = 125;
	of.numchn = 8;
	modtype = 0;
	of.songname = DupStr(mh->songname, 21, 1);
	of.numpos = mh->num_orders;
	of.reppos = 0;
	of.numpat = mh->num_patterns;
	of.numtrk = of.numpat * of.numchn;

	/* Copy positions (orders) */
	if (!AllocPositions(of.numpos))
		return 0;
	for (t = 0; t < of.numpos; t++) {
		of.positions[t] = mh->positions[t];
	}

	/* Finally, init the sampleinfo structures  */
	of.numins = 31;
	of.numsmp = 31;
	if (!AllocSamples())
		return 0;
	s = mh->samples;
	q = of.samples;
	seekpos = 2662+(2048*(of.numpat));
	for (t = 0; t < of.numins; t++) {
		/* convert the samplename */
		q->samplename = DupStr(s->samplename, 23, 1);

		/* init the sampleinfo variables */
		q->speed = finetune[s->finetune & 0xf];
		q->volume = s->volume & 0x7f;

		q->loopstart = (ULONG)s->reppos;
		q->loopend = (ULONG)q->loopstart + (s->replen);
		q->length = (ULONG)s->length;

		q->flags = SF_SIGNED;

		q->seekpos = seekpos;
		seekpos += q->length;

		if ((s->replen) > 2) {
			q->flags |= SF_LOOP;
		}

		/* fix replen if repend > length */
		if (q->loopend > q->length)
			q->loopend = q->length;

		s++;
		q++;
	}

	of.modtype = MikMod_strdup(descr);

	if (!ML_LoadPatterns())
		return 0;

	return 1;
}

static CHAR *ASY_LoadTitle(void)
{
	return MikMod_strdup("");
}

/*========== Loader information */

MLOADER load_asy = {
	NULL,
	"AMF",
	"AMF (ASYLUM Music Format V1.0)",
	ASY_Init,
	ASY_Test,
	ASY_Load,
	ASY_Cleanup,
	ASY_LoadTitle
};

/* ex:set ts=4: */
