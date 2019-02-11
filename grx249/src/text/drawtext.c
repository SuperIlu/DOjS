/**
 ** drawtext.c ---- draw a character string with the default font
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
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

#include <string.h>
#include "libgrx.h"

void GrTextXY(int x,int y,char *text,GrColor fg,GrColor bg)
{
	GrTextOption opt;
	opt.txo_font      = &GrDefaultFont;
	opt.txo_fgcolor.v = fg;
	opt.txo_bgcolor.v = bg;
	opt.txo_chrtype   = GR_BYTE_TEXT;
	opt.txo_direct    = GR_TEXT_RIGHT;
	opt.txo_xalign    = GR_ALIGN_LEFT;
	opt.txo_yalign    = GR_ALIGN_TOP;
	GrDrawString(text,(int)strlen(text),x,y,&opt);
}

