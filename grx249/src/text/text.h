/**
 ** text.h ---- some shared definitions
 **
 ** Copyright (c) 1998 Hartmut Schirmer
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
 */

typedef void (*TextDrawBitmapFunc)(int x,int y,int w,int h,int ox, int oy,
				   char far *bmp,int pitch,int start,
				   GrColor fg,GrColor bg,GrPattern *p);

void _GrDrawString(const void *text,int length,int x,int y,
		   const GrTextOption *opt, GrPattern *p, TextDrawBitmapFunc dbm);
