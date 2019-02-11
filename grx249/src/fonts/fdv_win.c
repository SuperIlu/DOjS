/**
 ** fdv_win.c -- driver for Windows resource font file format
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
 **/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libgrx.h"
#include "grfontdv.h"
#include "arith.h"
#include "allocate.h"
#include "fonts/fdv_win.h"

#ifndef SEEK_SET
#define SEEK_SET        0
#endif

static FILE *fontfp = NULL;
static int offset = 0;
static GrResourceFileHeaderWIN rhdr;
static GrFontFileHeaderWIN fhdr;
static GrCharHeaderWIN far *ctable = NULL;

#if BYTE_ORDER==BIG_ENDIAN
#include "ordswap.h"
static void swap_resource(void)
{
	GRX_ENTER();
	_GR_swap16u(&rhdr.type_id);
	_GR_swap16u(&rhdr.name_id);
	_GR_swap16u(&rhdr.flags);
	_GR_swap32u(&rhdr.size);
	GRX_LEAVE();
}

static void swap_header(void)
{
	GRX_ENTER();
	_GR_swap16u(&fhdr.version);
	_GR_swap32u(&fhdr.size);
	_GR_swap16u(&fhdr.type);
	_GR_swap16u(&fhdr.points);
	_GR_swap16u(&fhdr.vert_res);
	_GR_swap16u(&fhdr.horiz_res);
	_GR_swap16u(&fhdr.ascent);
	_GR_swap16u(&fhdr.internal_leading);
	_GR_swap16u(&fhdr.external_leading);
	_GR_swap16u(&fhdr.weight);
	_GR_swap16u(&fhdr.pix_width);
	_GR_swap16u(&fhdr.pix_height);
	_GR_swap16u(&fhdr.avg_width);
	_GR_swap16u(&fhdr.max_width);
	_GR_swap16u(&fhdr.width_bytes);
	_GR_swap32u(&fhdr.device);
	_GR_swap32u(&fhdr.face);
	_GR_swap32u(&fhdr.bits_pointer);
	_GR_swap32u(&fhdr.bits_offset);
	GRX_LEAVE();
}

static void swap_ctable(void)
{
	int i;
	GRX_ENTER();
	for(i = 0; i < fhdr.last_char - fhdr.first_char + 2; i++) {
	    _GR_swap16u(&ctable[i].width);
	    _GR_swap16u(&ctable[i].offset);
	}
	GRX_LEAVE();
}
#endif

static void cleanup(void)
{
	GRX_ENTER();
	if(fontfp != NULL) fclose(fontfp);
	if(ctable != NULL) farfree(ctable);
	fontfp = NULL;
	ctable = NULL;
	offset = 0;
	GRX_LEAVE();
}

static int openfile(char *fname)
{
	int res;
	unsigned size;
	GRX_ENTER();
	res = FALSE;
	cleanup();
	/* open and test the file */
	fontfp = fopen(fname, "rb");
	if(fontfp == NULL) {
	    DBGPRINTF(DBG_FONT, ("fopen(\"%s\") failed\n", fname));
	    goto done;
	}
	if(fread(&rhdr, 1, sizeof rhdr, fontfp) != sizeof rhdr) {
	    DBGPRINTF(DBG_FONT, ("read resource failed\n", fname));
	    goto done;
	}
	if(rhdr.type_ff == 0xFF) {
#if BYTE_ORDER==BIG_ENDIAN
	    DBGPRINTF(DBG_FONT, ("swapping resource byte order\n"));
	    swap_resource();
#endif
	    if(rhdr.type_id != 0x0008 || rhdr.name_ff != 0xFF) {
		DBGPRINTF(DBG_FONT, ("invalid or unsupported resource header\n"));
		goto done;
	    }
	    offset = sizeof rhdr;
	}
	else if(fseek(fontfp, 0, SEEK_SET < 0)) {
	    DBGPRINTF(DBG_FONT, ("rewind failed"));
	    goto done;
	}
	if(fread(&fhdr, 1, sizeof fhdr, fontfp) != sizeof fhdr) {
	    DBGPRINTF(DBG_FONT, ("read header failed\n"));
	    goto done;
	}
#if BYTE_ORDER==BIG_ENDIAN
	DBGPRINTF(DBG_FONT, ("swapping header byte order\n"));
	swap_header();
#endif
	if(fhdr.version != 0x0200 || fhdr.type != 0) {
	    DBGPRINTF(DBG_FONT, ("unrecognized font header\n"));
	    goto done;
	}
	/* allocate and read the ctable */
	size = (fhdr.last_char - fhdr.first_char + 2) * sizeof *ctable;
	if((ctable = farmalloc(size)) == NULL) {
	    DBGPRINTF(DBG_FONT, ("allocate ctable failed\n"));
	    goto done;
	}
	if(fread(ctable, 1, size, fontfp) != size) {
	    DBGPRINTF(DBG_FONT, ("read ctable failed\n"));
	    goto done;
	}
#if BYTE_ORDER==BIG_ENDIAN
	DBGPRINTF(DBG_FONT, ("swapping ctable byte order\n"));
	swap_ctable();
#endif
	res = TRUE;
done:	if(!res) cleanup();
	GRX_RETURN(res);
}

static char *families[] = { "Unknown", "Roman", "Swiss", "Modern", "Script", "Decorative" };

static int header(GrFontHeader *hdr)
{
	int res;
	int c;
	char *s;
	GRX_ENTER();
	res = FALSE;
	if(fontfp != NULL) {
	    if((c = fhdr.pitch_and_family >> 4) <= 5) strcpy(hdr->family, families[c]);
	    else sprintf(hdr->family, "0x%x", fhdr.pitch_and_family);
	    if(fhdr.face) {
		s = hdr->name;
		if(fseek(fontfp, offset + fhdr.face, SEEK_SET) < 0) goto done;
		do {
		    if((c = fgetc(fontfp)) == EOF) goto done;
		    *(s++) = c;
		} while(c && s - hdr->name < 99);
		if(c) *s = '\0';
	    }
	    else sprintf(hdr->name, "%s-%d", hdr->family, fhdr.pix_height);
	    hdr->proportional = fhdr.pix_width == 0;
	    hdr->scalable = FALSE;
	    hdr->preloaded = FALSE;
	    hdr->modified = GR_FONTCVT_NONE;
	    hdr->width = hdr->proportional ? fhdr.avg_width : fhdr.pix_width;
	    hdr->height = fhdr.pix_height;
	    hdr->baseline = fhdr.ascent;
	    hdr->ulheight = imax(1, hdr->height / 15);
	    hdr->ulpos = hdr->height - hdr->ulheight;
	    hdr->minchar = fhdr.first_char;
	    hdr->numchars = fhdr.last_char - fhdr.first_char + 1;
	    res = TRUE;
	}
done:	GRX_RETURN(res);
}

static int charwdt(int chr)
{
	int res;
	GRX_ENTER();
	res = -1;
	if(fontfp != NULL && chr >= fhdr.first_char && chr <= fhdr.last_char) res = ctable[chr - fhdr.first_char].width;
	GRX_RETURN(res);
}

static int bitmap(int chr, int w, int h, char *buffer)
{
	int res;
	int i, y;
	int bytes;
	GRX_ENTER();
	res = FALSE;
	if(w != charwdt(chr) || h != fhdr.pix_height) goto done;
	bytes = (w - 1) / 8 + 1;
	if(fseek(fontfp, offset + ctable[chr - fhdr.first_char].offset, SEEK_SET) < 0) goto done;
	for(i = 0; i < bytes; i++)
	    for(y = 0; y < h; y++)
		if(fread(buffer + bytes * y + i, 1, 1, fontfp) != 1) goto done;
	res = TRUE;
done:	GRX_RETURN(res);
}

GrFontDriver _GrFontDriverWIN = {
    "WIN",                              /* driver name (doc only) */
    ".res",                             /* font file extension */
    FALSE,                              /* scalable */
    openfile,                           /* file open and check routine */
    header,                             /* font header reader routine */
    charwdt,                            /* character width reader routine */
    bitmap,                             /* character bitmap reader routine */
    cleanup                             /* cleanup routine */
};
