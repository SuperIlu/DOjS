/**
 ** fonttest.c ---- test text drawing
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

#include <string.h>
#include "test.h"

int  cx;
int  cy;
GrColor c1;
GrColor c2;
GrColor c3;
GrColor c4;

char test_text[] = {
    "QUICK BROWN FOX JUMPS OVER THE LAZY DOG, "
    "quick brown fox jumps over the lazy dog !@#$%^&*()1234567890"
};

void displayfont(GrFont *font,char *text,int len)
{
	GrTextOption opt;
	int ww,hh;
	int bx,by;
	int bw,bh;

	memset(&opt,0,sizeof(opt));
	opt.txo_font   = font;
	opt.txo_xalign = GR_ALIGN_LEFT;
	opt.txo_yalign = GR_ALIGN_TOP;
	GrFilledBox(0,0,GrSizeX(),GrSizeY(),GrBlack());
	opt.txo_direct    = GR_TEXT_RIGHT;
	opt.txo_fgcolor.v = GrBlack();
	opt.txo_bgcolor.v = c1;
	ww = GrStringWidth(text,len,&opt);
	hh = GrStringHeight(text,len,&opt);
	bw = ww+2*hh;
	bh = ww;
	bx = cx - bw/2;
	by = cy - bh/2;
	GrDrawString(text,len,bx+hh,by,&opt);
	opt.txo_direct    = GR_TEXT_DOWN;
	opt.txo_bgcolor.v = c2;
	GrDrawString(text,len,bx+bw-hh,by,&opt);
	opt.txo_direct    = GR_TEXT_LEFT;
	opt.txo_bgcolor.v = c3;
	GrDrawString(text,len,bx+bw-ww-hh,by+bh-hh,&opt);
	opt.txo_direct    = GR_TEXT_UP;
	opt.txo_bgcolor.v = c4;
	GrDrawString(text,len,bx,by+bh-ww,&opt);
	GrKeyRead();
	GrClearClipBox(GrBlack());
	opt.txo_direct    = GR_TEXT_RIGHT;
	opt.txo_fgcolor.v = c1;
	opt.txo_bgcolor.v = GrBlack();
	bx = GrSizeX() / 16;
	by = GrSizeY() / 16;
	bx = (bx + 7) & ~7;
	while(by < GrSizeY()) {
	    GrDrawString(test_text,strlen(test_text),bx,by,&opt);
	    opt.txo_fgcolor.v ^= GR_UNDERLINE_TEXT;
	    by += hh;
	}
	GrKeyRead();
}

TESTFUNC(fonttest)
{
	GrFont *f;
	int i;
	char buff[100];
	cx = GrSizeX() / 2;
	cy = GrSizeY() / 2;
	c1 = GrAllocColor(100,200,100);
	c2 = GrAllocColor(150,150,100);
	c3 = GrAllocColor(100,100,200);
	c4 = GrAllocColor(100,180,180);
	GrBox(GrSizeX()/16 - 2,
	    GrSizeY()/16 - 2,
	    GrSizeX() - GrSizeX()/16 + 1,
	    GrSizeY() - GrSizeY()/16 + 1,
	    GrAllocColor(250,100,100)
	);
	GrSetClipBox(GrSizeX()/16,
	    GrSizeY()/16,
	    GrSizeX() - GrSizeX()/16 - 1,
	    GrSizeY() - GrSizeY()/16 - 1
	);
	strcpy(buff,"Default GRX font");
	displayfont(&GrDefaultFont,buff,strlen(buff));
	strcpy(buff,"Default font scaled to 6x10");
	displayfont(
	    GrBuildConvertedFont(
		&GrDefaultFont,
		(GR_FONTCVT_SKIPCHARS | GR_FONTCVT_RESIZE),
		6,
		10,
		' ',
		'z'
	    ),
	    buff,
	    strlen(buff)
	);
	strcpy(buff,"Default font scaled to 12x24");
	displayfont(
	    GrBuildConvertedFont(
		&GrDefaultFont,
		(GR_FONTCVT_SKIPCHARS | GR_FONTCVT_RESIZE),
		12,
		24,
		' ',
		'z'
	    ),
	    buff,
	    strlen(buff)
	);
	strcpy(buff,"Default font scaled to 18x36");
	displayfont(
	    GrBuildConvertedFont(
		&GrDefaultFont,
		(GR_FONTCVT_SKIPCHARS | GR_FONTCVT_RESIZE),
		18,
		36,
		' ',
		'z'
	    ),
	    buff,
	    strlen(buff)
	);
	strcpy(buff,"Default font scaled to 10x20 proportional");
	displayfont(
	    GrBuildConvertedFont(
		&GrDefaultFont,
		(GR_FONTCVT_SKIPCHARS | GR_FONTCVT_RESIZE | GR_FONTCVT_PROPORTION),
		10,
		20,
		' ',
		'z'
	    ),
	    buff,
	    strlen(buff)
	);
	strcpy(buff,"Default font scaled to 10x20 bold");
	displayfont(
	    GrBuildConvertedFont(
		&GrDefaultFont,
		(GR_FONTCVT_SKIPCHARS | GR_FONTCVT_RESIZE | GR_FONTCVT_BOLDIFY),
		10,
		20,
		' ',
		'z'
	    ),
	    buff,
	    strlen(buff)
	);
	strcpy(buff,"Default font scaled to 10x20 italic");
	displayfont(
	    GrBuildConvertedFont(
		&GrDefaultFont,
		(GR_FONTCVT_SKIPCHARS | GR_FONTCVT_RESIZE | GR_FONTCVT_ITALICIZE),
		10,
		20,
		' ',
		'z'
	    ),
	    buff,
	    strlen(buff)
	);
	for(i = 0; i < Argc; i++) {
	    f = GrLoadFont(Argv[i]);
	    if(f) {
		sprintf(buff,"This is font %s",Argv[i]);
		displayfont(f,buff,strlen(buff));
	    }
	}
}

