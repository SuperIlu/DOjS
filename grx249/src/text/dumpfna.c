/**
 ** dumpfna.c ---- write an ascii font file from a font in memory
 **
 ** Copyright (C) 2002 Dimitar Zhekov
 ** E-Mail: jimmy@is-vn.bg
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

#include "libgrx.h"

void GrDumpFnaFont(const GrFont *f, char *fileName)
{
	int chr;
	int x, y, width, bytes;
	char *buffer;
	FILE *fp = fopen(fileName, "w");
	if(!fp) return;
	/* write header */
	fprintf(fp, "name %s\n", f->h.name);
	fprintf(fp, "family %s\n", f->h.family);
	fprintf(fp, "isfixed %d\n", !f->h.proportional);
	if(f->h.proportional) {
	    fprintf(fp, "minwidth %d\n", f->minwidth);
	    fprintf(fp, "maxwidth %d\n", f->maxwidth);
	    fprintf(fp, "avg");
	}
	fprintf(fp, "width %d\n", f->h.width);
	fprintf(fp, "height %d\n", f->h.height);
	fprintf(fp, "minchar %d\n", f->h.minchar);
	fprintf(fp, "maxchar %d\n", f->h.minchar + f->h.numchars - 1);
	fprintf(fp, "baseline %d\n", f->h.baseline);
	fprintf(fp, "undwidth %d\n", f->h.ulheight);
	/* write characters */
	for(chr = f->h.minchar; chr < f->h.minchar + f->h.numchars; chr++) {
	    width = GrFontCharWidth(f, chr);
	    bytes = (width - 1) / 8 + 1;
	    buffer = GrFontCharBitmap(f, chr);
	    /* write character header */
	    fprintf(fp, "\n; character %d", chr);
	    if(isgraph(chr)) fprintf(fp, " (%c)", chr);
	    fprintf(fp, " width = %d\n", width);
	    /* write character data */
	    for(y = 0; y < f->h.height; y++) {
		for(x = 0; x < width; x++)
		    putc(buffer[x >> 3] & (1 << (7 - (x & 7))) ? '#' : '.', fp);
		putc('\n', fp);
		buffer += bytes;
	    }
	}
	fclose(fp);
}
