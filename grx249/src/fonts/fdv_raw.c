/**
 ** fdv_raw.c -- driver for raw font file format
 **
 ** Copyright (C) 2003 Dimitar Zhekov
 ** [e-mail: jimmy@is-vn.bg]
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** Changes by Dimitar Zhekov (jimmy@is-vn.bg) Nov 20 2003
 **   - Added psf2 support, raw and psf1 are now treated as pseudo-psf2.
 **   - Better support for RAW files: up to 16x32, assuming scale 1:2.
 **
 **/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libgrx.h"
#include "grfontdv.h"
#include "arith.h"
#include "fonts/fdv_raw.h"

#ifndef SEEK_SET
#define SEEK_SET        0
#endif

#ifndef SEEK_END
#define SEEK_END        2
#endif

static FILE *fontfp = NULL;
static int   nextch = 0;
static char name[40], family[40];
static GrFontFileHeaderPSF fhdr;

#if BYTE_ORDER==BIG_ENDIAN
#include "ordswap.h"
static void swap_header(void) {
    GRX_ENTER();
    _GR_swap32u(&fhdr.version);
    _GR_swap32u(&fhdr.offset);
    _GR_swap32u(&fhdr.flags);
    _GR_swap32u(&fhdr.length);
    _GR_swap32u(&fhdr.charsize);
    _GR_swap32u(&fhdr.height);
    _GR_swap32u(&fhdr.width);
    GRX_LEAVE();
}
#endif

static void cleanup(void)
{
	GRX_ENTER();
	if(fontfp != NULL) fclose(fontfp);
	fontfp = NULL;
	nextch = 0;
	GRX_LEAVE();
}

static int openfile(char *fname)
{
	int res;
	long size;
	char *s;
	GRX_ENTER();
	res = FALSE;
	cleanup();
	/* open fname and read header */
	fontfp = fopen(fname, "rb");
	if(fontfp == NULL) {
	    DBGPRINTF(DBG_FONT, ("fopen(\"%s\") failed\n", fname));
	    goto done;
	}
	if(fread(&fhdr, 1, sizeof fhdr, fontfp) != sizeof fhdr) {
	    DBGPRINTF(DBG_FONT, ("read header failed\n"));
	    goto done;
	}
	if(fseek(fontfp, 0, SEEK_END) < 0) {
	    DBGPRINTF(DBG_FONT, ("seek to end of file failed\n"));
	    goto done;
	}
	size = ftell(fontfp);
	if(size < 0) {
	    DBGPRINTF(DBG_FONT, ("tell file position failed\n"));
	    goto done;
	}
	/* try to guess file type */
	if(fhdr.id[0] == PSF1_MAGIC0 && fhdr.id[1] == PSF1_MAGIC1) {
	    fhdr.offset = PSF1_HDRSIZE;
	    fhdr.width = 8;
	    fhdr.height = fhdr.charsize = fhdr.size;
	    fhdr.numchars = (fhdr.mode & PSF1_MODE512) == 0 ? 256 : 512;
	}
	else if(fhdr.id[0] == PSF2_MAGIC0 && fhdr.id[1] == PSF2_MAGIC1 && fhdr.mode == PSF2_MAGIC2 && fhdr.size == PSF2_MAGIC3)
	{
#if BYTE_ORDER==BIG_ENDIAN
	    DBGPRINTF(DBG_FONT, ("swapping header byte order\n"));
	    swap_header();
#endif
	    fhdr.charsize = ((fhdr.width + 7) / 8) * fhdr.height;
	    if(fhdr.numchars == 0) {
		DBGPRINTF(DBG_FONT, ("invalid numchars\n"));
		goto done;
	    }
	}
	else {
	    if(size > 16384 || size % (size <= 4096 ? 256 : 512)) {
		DBGPRINTF(DBG_FONT, ("invalid raw file size\n"));
		goto done;
	    }
	    fhdr.offset = 0;
	    fhdr.charsize = size / 256;
	    if(size <= 4096) {
		fhdr.width = 8;
		fhdr.height = fhdr.charsize;
	}
	else {
		fhdr.height = size / 512;
		fhdr.width = (fhdr.height + 1) / 2;
	    }
	    fhdr.numchars = 256;
	}
	if(fhdr.offset != 0) {
	    if(fhdr.charsize == 0) {
		DBGPRINTF(DBG_FONT, ("invalid psf charsize\n"));
		goto done;
	    }
	    if(size - fhdr.offset < fhdr.charsize * fhdr.numchars) {
		DBGPRINTF(DBG_FONT, ("invalid psf file size\n"));
		goto done;
	    }
	}
	/* get font name and family */
	s = strrchr(fname, '/');
#if defined(__MSDOS__) || defined(__WIN32__)
	if(s == NULL) {
	    s = strrchr(fname, '\\');
	    if(s == NULL) s = strrchr(fname, ':');
	}
	else if(strrchr(s, '\\') != NULL) s = strrchr(s, '\\');
#endif
	if(s == NULL || *++s == '\0') s = fname;
	strncpy(name, s, sizeof name - 1);
	name[sizeof name - 1] = '\0';
	if((s = strrchr(name, '.')) != NULL) *s = '\0';
	if(*name == '\0') sprintf(name, fhdr.offset != 0 ? "psf%d" : "raw%d", (int) fhdr.height);
	strcpy(family, name);
	for(s = family; isalpha(*s); s++);
	if(s > family) *s = '\0';
	/* finish and return */
	nextch = fhdr.numchars;
	res = TRUE;
done:   if(!res) cleanup();
	GRX_RETURN(res);
}

static int header(GrFontHeader *hdr)
{
	int res;
	GRX_ENTER();
	res = FALSE;
	if(fontfp != NULL) {
	    strcpy(hdr->name, name);
	    strcpy(hdr->family, family);
	    hdr->proportional = FALSE;
	    hdr->scalable = FALSE;
	    hdr->preloaded = FALSE;
	    hdr->modified = GR_FONTCVT_NONE;
	    hdr->width = fhdr.width;
	    hdr->height = fhdr.height;
	    hdr->baseline = (hdr->height * 4) / 5 + (hdr->height < 15);
	    hdr->ulheight = imax(1, hdr->height / 15);
	    hdr->ulpos = hdr->height - hdr->ulheight;
	    hdr->minchar = 0;
	    hdr->numchars = fhdr.numchars;
	    res = TRUE;
	}
	GRX_RETURN(res);
}

static int charwdt(int chr)
{
	int res;
	GRX_ENTER();
	res = -1;
	if(fontfp != NULL && chr >= 0 && chr < fhdr.numchars) res = fhdr.width;
	GRX_RETURN(res);
}

static int bitmap(int chr,int w,int h,char *buffer)
{
	int res;
	GRX_ENTER();
	res = FALSE;
	if(w != charwdt(chr) || h != fhdr.height) goto done;
	if(chr != nextch && fseek(fontfp, fhdr.offset + fhdr.charsize * chr, SEEK_SET) < 0) goto done;
	if(fread(buffer, 1, fhdr.charsize, fontfp) != fhdr.charsize) goto done;
	nextch = chr + 1;
	res = TRUE;
done:	GRX_RETURN(res);
}

GrFontDriver _GrFontDriverRAW = {
    "RAW",                              /* driver name (doc only) */
    ".psf",                             /* font file extension */
    FALSE,                              /* scalable */
    openfile,                           /* file open and check routine */
    header,                             /* font header reader routine */
    charwdt,                            /* character width reader routine */
    bitmap,                             /* character bitmap reader routine */
    cleanup                             /* cleanup routine */
};
