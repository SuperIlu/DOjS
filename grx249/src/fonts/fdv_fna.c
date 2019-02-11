/**
 ** fdv_fna.c -- driver for ascii font file format
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

#ifndef SEEK_SET
#define SEEK_SET        0
#endif

static FILE   *fontfp = NULL;

static struct {
    char buffer[131];
    long offset;
    int  index;
    int  minchar;
    int  maxchar;
    int  width;
    int  height;
    int  isfixed;
} fhdr;

static int readline(void)
{
	int res;
	char *s;
	GRX_ENTER();
	res = FALSE;
	do {
	    if(fgets(fhdr.buffer, sizeof fhdr.buffer, fontfp) == NULL) {
		DBGPRINTF(DBG_FONT, ("read line failed at index %d\n", fhdr.index));
		goto done;
	    }
	    s = fhdr.buffer + strlen(fhdr.buffer);
	    while(--s >= fhdr.buffer && (*s == '\n' || *s == '\r'));
	    *++s = '\0';
	    if(strlen(fhdr.buffer) > 127) {
		DBGPRINTF(DBG_FONT, ("line too long \"%s\"", fhdr.buffer));
		goto done;
	    }
	    while(--s >= fhdr.buffer && isspace(*s));
	    *++s = '\0';
	} while(s == fhdr.buffer || *fhdr.buffer == ';');
	res = TRUE;
done:	GRX_RETURN(res);
}

static int readindex(int chr, int y)
{
	int res;
	int index;
	GRX_ENTER();
	res = FALSE;
	index = (chr - fhdr.minchar) * fhdr.height + y;
	if(fhdr.index > index) {
	    DBGPRINTF(DBG_FONT, ("current index %d > requested %d\n", fhdr.index, index));
	    if(fseek(fontfp, fhdr.offset, SEEK_SET) < 0) goto done;
	    fhdr.index = -1;
	}
	while(fhdr.index < index) {
	    if(!readline()) goto done;
	    fhdr.index++;
	}
	res = TRUE;
done:	GRX_RETURN(res);
}

static void cleanup(void)
{
	GRX_ENTER();
	if(fontfp != NULL) fclose(fontfp);
	fontfp = NULL;
	fhdr.index = -1;
	GRX_LEAVE();
}

static int openfile(char *fname)
{
	int res;
	GRX_ENTER();
	res = FALSE;
	cleanup();
	fontfp = fopen(fname, "rb");
	if(fontfp == NULL) {
	    DBGPRINTF(DBG_FONT, ("fopen(\"%s\") failed\n", fname));
	    goto done;
	}
	res = TRUE;
done:	if(!res) cleanup();
	GRX_RETURN(res);
}

static int header(GrFontHeader *hdr)
{
	int res;
	char *s;
	int index;
	int i, n;
	static char *names[] = {
	    "name",
	    "family",
	    "isfixed",
	    "width",
	    "height",
	    "minchar",
	    "maxchar",
	    "baseline",
	    "undwidth",
	    "avgwidth",
	    "minwidth",
	    "maxwidth",
	    "note",
	    NULL
	};
	int attrib;
	GRX_ENTER();
	res = FALSE;
	if(fontfp == NULL) goto done;
	attrib = 0;
	while(readline() && isalpha(*fhdr.buffer)) {
	    fhdr.offset = ftell(fontfp);
	    if(fhdr.offset == -1) {
		DBGPRINTF(DBG_FONT, ("ftell failed after \"%s\"\n", fhdr.buffer));
		goto done;
	    }
	    if(!strcmp(fhdr.buffer, "note")) continue;
	    s = fhdr.buffer;
	    while(isalpha(*++s));
	    if(!isspace(*s)) {
		DBGPRINTF(DBG_FONT, ("invalid header line \"%s\"\n", fhdr.buffer));
		goto done;
	    }
	    *s = '\0';
	    while(isspace(*++s));
	    for(index = 0; names[index] != NULL; index++)
		if(!strcmp(fhdr.buffer, names[index])) break;
	    if(names[index] == NULL) {
		DBGPRINTF(DBG_FONT, ("unknown attribute \"%s\"\n", fhdr.buffer));
		goto done;
	    }
	    if(index == 9) index = 3;
	    if(attrib & (1 << index)) {
		DBGPRINTF(DBG_FONT, ("duplicate attribute \"%s\"\n", fhdr.buffer));
		goto done;
	    }
	    if(index >= 2 && index <= 11) {
		if(sscanf(s, "%d%n", &i, &n) != 1 || n != strlen(s)) {
		    DBGPRINTF(DBG_FONT, ("invalid number \"%s\"\n", s));
		    goto done;
		}
		if(i < 0) {
		    DBGPRINTF(DBG_FONT, ("negative number %d\n", i));
		    goto done;
		}
	    }
	    switch(index) {
		case 0 : strcpy(hdr->name, s); break;
		case 1 : strcpy(hdr->family, s); break;
		case 2 :
		    fhdr.isfixed = i;
		    hdr->proportional = !fhdr.isfixed;
		    break;
		case 3 : hdr->width = fhdr.width = i; break;
		case 4 : hdr->height = fhdr.height = i; break;
		case 5 : hdr->minchar = fhdr.minchar = i; break;
		case 6 : fhdr.maxchar = i; break;
		case 7 : hdr->baseline = i; break;
		case 8 : hdr->ulheight = i; break;
		case 10 :
		    if(i == 0) {
			DBGPRINTF(DBG_FONT, ("invalid width %d\n", i));
		        goto done;
		    }
		    break;
		case 11 :
		    if(i > 127) {
			DBGPRINTF(DBG_FONT, ("invalid width %d\n", i));
		        goto done;
		    }
		    break;
		case 12 : continue;
		default :
		    DBGPRINTF(DBG_FONT, ("unsupported attribute \"%s\"\n", fhdr.buffer));
		    goto done;
	    }
	    attrib |= 1 << index;
	}
	if((attrib & 0xFF) != 0xFF) {
	    DBGPRINTF(DBG_FONT, ("insufficient attributes 0x%x\n", attrib));
	    goto done;
	}
	hdr->numchars = fhdr.maxchar - fhdr.minchar + 1;
	if(hdr->numchars <= 0) {
	    DBGPRINTF(DBG_FONT, ("minchar %d > maxchar %d\n", fhdr.minchar, fhdr.maxchar));
	    goto done;
	}
	fhdr.index++;
	hdr->scalable = FALSE;
	hdr->preloaded = FALSE;
	hdr->modified = GR_FONTCVT_NONE;
	if((attrib & 0x0100) == 0) hdr->ulheight = imax(1, hdr->height / 15);
	hdr->ulpos = hdr->height - hdr->ulheight;
	res = TRUE;
done:	GRX_RETURN(res);
}

static int charwdt(int chr)
{
	int res;
	GRX_ENTER();
	DBGPRINTF(DBG_FONT, ("charwdt(%d)\n", chr));
	res = -1;
	if(fontfp != NULL && chr >= fhdr.minchar && chr <= fhdr.maxchar) {
	    if(fhdr.isfixed) res = fhdr.width;
	    else if(readindex(chr, 0)) res = strlen(fhdr.buffer);
	}
	GRX_RETURN(res);
}

static int bitmap(int chr, int w, int h, char *buffer)
{
	int res;
	int y, x;
	int bytes;
	GRX_ENTER();
	DBGPRINTF(DBG_FONT, ("bitmap(%d, %d, %d)\n", chr, w, h));
	res = FALSE;
	if(w != charwdt(chr) || h != fhdr.height) goto done;
	bytes = (w - 1) / 8 + 1;
	memset(buffer, '\0', bytes * h);
	for(y = 0; y < h; y++) {
	    if(!readindex(chr, y)) goto done;
	    if(strlen(fhdr.buffer) != w) {
		DBGPRINTF(DBG_FONT, ("strlen(\"%s\") != %d\n", fhdr.buffer, w));
		goto done;
	    }
	    for(x = 0; x < w; x++) {
		if(fhdr.buffer[x] == '#') buffer[x >> 3] |= 1 << (7 - (x & 7));
		else if(fhdr.buffer[x] != '.') {
		    DBGPRINTF(DBG_FONT, ("invalid character data \'%c\'\n", fhdr.buffer[x]));
		    goto done;
		}
	    }
	    buffer += bytes;
	}
	res = TRUE;
done:	GRX_RETURN(res);
}

GrFontDriver _GrFontDriverFNA = {
    "FNA",                              /* driver name (doc only) */
    ".fna",                             /* font file extension */
    FALSE,                              /* scalable */
    openfile,                           /* file open and check routine */
    header,                             /* font header reader routine */
    charwdt,                            /* character width reader routine */
    bitmap,                             /* character bitmap reader routine */
    cleanup                             /* cleanup routine */
};
