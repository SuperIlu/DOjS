/*	MikMod sound library
	(c) 2004, Raphael Assenat
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

  Driver for output to a file called MUSIC.AIFF [or .AIF on Windows].

==============================================================================*/

/*

	Written by Axel "awe" Wefers <awe@fruitz-of-dojo.de>
	Raphael Assenat: 19 Feb 2004: Command line options documented in the MDRIVER structure,
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_AIFF

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <math.h>	/* required for IEEE extended conversion */

#define AIFF_BUFFERSIZE			32768
#if defined(_WIN32) || defined(__DJGPP__)
#define AIFF_FILENAME			"music.aif"
#else
#define AIFF_FILENAME			"music.aiff"
#endif /* WIN32 */

#define AIFF_FLOAT_TO_UNSIGNED(f)	((ULONG)(((SLONG)(f - 2147483648.0)) + 2147483647L + 1))

static	MWRITER	*gAiffOut = NULL;
static	FILE	*gAiffFile = NULL;
static	SBYTE	*gAiffAudioBuffer = NULL;
static	CHAR	*gAiffFileName = NULL;
static	ULONG	gAiffDumpSize = 0;

#ifdef SUNOS
extern int fclose(FILE *);
#endif
#ifdef __VBCC__
#define unlink remove
#endif
#ifdef _WIN32
#define unlink _unlink
#endif

static void	AIFF_ConvertToIeeeExtended (double theValue, char *theBytes);
static void	AIFF_PutHeader (void);
static void	AIFF_CommandLine (const CHAR *theCmdLine);
static BOOL	AIFF_IsThere (void);
static int	AIFF_Init (void);
static void	AIFF_Exit (void);
static void	AIFF_Update (void);


static void AIFF_ConvertToIeeeExtended (double theValue, char *theBytes)
{
    int mySign, myExponent;
    double myFMant, myFsMant;
    ULONG myHiMant, myLoMant;

    if (theValue < 0)
    {
        mySign = 0x8000;
        theValue *= -1;
    } else
    {
        mySign = 0;
    }

    if (theValue < 1e-18)
    {
        myExponent = 0;
        myHiMant = 0;
        myLoMant = 0;
    }
    else
    {
        myFMant = frexp (theValue, &myExponent);
        if ((myExponent > 16384) || !(myFMant < 1))
        {
            myExponent = mySign | 0x7FFF;
            myHiMant = 0;
            myLoMant = 0;
        }
        else
        {
            myExponent += 16382;
            if (myExponent < 0)
            {
                myFMant = ldexp (myFMant, myExponent);
                myExponent = 0;
            }
            myExponent |= mySign;
            myFMant = ldexp (myFMant, 32);
            myFsMant = floor (myFMant);
            myHiMant = AIFF_FLOAT_TO_UNSIGNED (myFsMant);
            myFMant = ldexp (myFMant - myFsMant, 32);
            myFsMant = floor (myFMant);
            myLoMant = AIFF_FLOAT_TO_UNSIGNED (myFsMant);
        }
    }

    theBytes[0] = myExponent >> 8;
    theBytes[1] = myExponent;
    theBytes[2] = myHiMant >> 24;
    theBytes[3] = myHiMant >> 16;
    theBytes[4] = myHiMant >> 8;
    theBytes[5] = myHiMant;
    theBytes[6] = myLoMant >> 24;
    theBytes[7] = myLoMant >> 16;
    theBytes[8] = myLoMant >> 8;
    theBytes[9] = myLoMant;
}

static void AIFF_PutHeader(void)
{
    ULONG	myFrames;
    UBYTE	myIEEE[10];

    myFrames = gAiffDumpSize / (((md_mode&DMODE_STEREO) ? 2 : 1) * ((md_mode & DMODE_16BITS) ? 2 : 1));
    AIFF_ConvertToIeeeExtended ((double) md_mixfreq, (char *)myIEEE);

    _mm_fseek (gAiffOut, 0, SEEK_SET);
    _mm_write_string  ("FORM", gAiffOut);				/* chunk 'FORM' */
    _mm_write_M_ULONG (gAiffDumpSize + 36, gAiffOut);			/* length of the file */
    _mm_write_string  ("AIFFCOMM", gAiffOut);				/* chunk 'AIFF', 'COMM' */
    _mm_write_M_ULONG (18, gAiffOut);					/* length of this AIFF block */
    _mm_write_M_UWORD ((md_mode & DMODE_STEREO) ? 2 : 1, gAiffOut);	/* channels */
    _mm_write_M_ULONG (myFrames, gAiffOut);				/* frames = freq * secs */
    _mm_write_M_UWORD ((md_mode & DMODE_16BITS) ? 16 : 8, gAiffOut);	/* bits per sample */
    _mm_write_UBYTES  (myIEEE, 10, gAiffOut);				/* frequency [IEEE extended] */
    _mm_write_string  ("SSND", gAiffOut);				/* data chunk 'SSND' */
    _mm_write_M_ULONG (gAiffDumpSize, gAiffOut);			/* data length */
    _mm_write_M_ULONG (0, gAiffOut);					/* data offset, always zero */
    _mm_write_M_ULONG (0, gAiffOut);					/* data blocksize, always zero */
}

static void AIFF_CommandLine (const CHAR *theCmdLine)
{
    CHAR *myFileName = MD_GetAtom ("file", theCmdLine,0);

    if (myFileName != NULL)
    {
        MikMod_free (gAiffFileName);
        gAiffFileName = myFileName;
    }
}

static BOOL AIFF_IsThere (void)
{
    return 1;
}

static int AIFF_Init (void)
{
#if (MIKMOD_UNIX)
    if (!MD_Access (gAiffFileName ? gAiffFileName : AIFF_FILENAME))
    {
        _mm_errno=MMERR_OPENING_FILE;
        return 1;
    }
#endif

    if (!(gAiffFile = fopen (gAiffFileName ? gAiffFileName : AIFF_FILENAME, "wb")))
    {
        _mm_errno = MMERR_OPENING_FILE;
        return 1;
    }
    if (!(gAiffOut =_mm_new_file_writer (gAiffFile)))
    {
        fclose (gAiffFile);
        unlink(gAiffFileName ? gAiffFileName : AIFF_FILENAME);
        gAiffFile = NULL;
        return 1;
    }

    if (!(gAiffAudioBuffer = (SBYTE*) MikMod_malloc (AIFF_BUFFERSIZE)))
    {
        _mm_delete_file_writer (gAiffOut);
        fclose (gAiffFile);
        unlink (gAiffFileName ? gAiffFileName : AIFF_FILENAME);
        gAiffFile = NULL;
        gAiffOut = NULL;
        return 1;
    }

    md_mode|=DMODE_SOFT_MUSIC|DMODE_SOFT_SNDFX;
    md_mode&=~DMODE_FLOAT;

    if (VC_Init ())
    {
        _mm_delete_file_writer (gAiffOut);
        fclose (gAiffFile);
        unlink (gAiffFileName ? gAiffFileName : AIFF_FILENAME);
        gAiffFile = NULL;
        gAiffOut = NULL;
        return 1;
    }

    gAiffDumpSize = 0;
    AIFF_PutHeader ();

    return 0;
}

static void AIFF_Exit (void)
{
    VC_Exit ();

    /* write in the actual sizes now */
    if (gAiffOut)
    {
        AIFF_PutHeader ();
        _mm_delete_file_writer (gAiffOut);
        fclose (gAiffFile);
        gAiffFile = NULL;
        gAiffOut = NULL;
    }

    MikMod_free (gAiffAudioBuffer);
    gAiffAudioBuffer = NULL;
}

static void AIFF_Update (void)
{
    ULONG	myByteCount;

    myByteCount = VC_WriteBytes (gAiffAudioBuffer, AIFF_BUFFERSIZE);
    if (md_mode & DMODE_16BITS)
    {
        _mm_write_M_UWORDS ((UWORD *) gAiffAudioBuffer, myByteCount >> 1, gAiffOut);
    }
    else
    {
        ULONG	i;
        for (i = 0; i < myByteCount; i++) {
            gAiffAudioBuffer[i] -= 0x80; /* convert to signed PCM */
        }
        _mm_write_UBYTES (gAiffAudioBuffer, myByteCount, gAiffOut);
    }
    gAiffDumpSize += myByteCount;
}

MIKMODAPI MDRIVER drv_aiff = {
    NULL,
    "Disk writer (aiff)",
    "AIFF disk writer (music.aiff) v1.2",
    0,255,
    "aif",
    "file:t:music.aiff:Output file name\n",
    AIFF_CommandLine,
    AIFF_IsThere,
    VC_SampleLoad,
    VC_SampleUnload,
    VC_SampleSpace,
    VC_SampleLength,
    AIFF_Init,
    AIFF_Exit,
    NULL,
    VC_SetNumVoices,
    VC_PlayStart,
    VC_PlayStop,
    AIFF_Update,
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

MISSING(drv_aiff);

#endif
