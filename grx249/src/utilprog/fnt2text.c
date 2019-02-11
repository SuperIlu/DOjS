/**
 ** font2txt.c ---- make an ASCII dump of a font
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
 **
 ** This is a test/demo file of the GRX graphics library.
 ** You can use GRX test/demo files as you want.
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

#include <stdio.h>
#include "grx20.h"

void dumpf(GrFont *f)
{
	int  chr,wdt,hgt,xpos,ypos;
	char far *bmp;
	hgt = f->h.height;
	for(chr = f->h.minchar; chr < (f->h.minchar + f->h.numchars); chr++) {
	    wdt = GrFontCharWidth(f,chr);
	    bmp = GrFontCharBitmap(f,chr);
	    printf("char '%c', code = 0x%04x\n",chr,chr);
	    for(ypos = 0; ypos < hgt; ypos++) {
		for(xpos = 0; xpos < wdt; xpos++) {
		    putchar((bmp[xpos >> 3] & (0x80 >> (xpos & 7))) ? '#' : '.');
		}
		putchar('\n');
		bmp += ((wdt + 7) >> 3);
	    }
	    putchar('\n');
	}
}

int main()
{
	dumpf(GrLoadFont("pc8x16"));
	dumpf(GrBuildConvertedFont(
	    &GrDefaultFont,
	    (GR_FONTCVT_SKIPCHARS | GR_FONTCVT_RESIZE | GR_FONTCVT_PROPORTION),
	    10,
	    20,
	    ' ',
	    'z'
	));
	return(0);
}

