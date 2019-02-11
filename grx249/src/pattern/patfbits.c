/**
 ** patfbits.c                            
 **
 ** Author unknow                                         
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

#include "libgrx.h"
#include "clipping.h"
#include "shapes.h"

void  _GrFillBitmapPatternExt(int x,int y,int w,int h, int sx, int sy,
				char far *bmp,int pitch,int start,
				GrPattern* p,GrColor bg)
{
   GR_int8u far *bits, *dptr;
   GR_int8u mask;
   int xx, width, oldx;
   GRX_ENTER();
   bits = (GR_int8u far *)bmp + ((unsigned int)start >> 3);
   if ( bg == GrNOCOLOR )
   {
     w += x;
     h += y;
     do {
       mask = 0x80;
       dptr = bits;
       oldx = xx = x;
       width = 0;
       do {
	 if ( *dptr & mask ) {
	   if ( width == 0 ) oldx = xx;
	   width++;
	 }
	 else if ( width ) {
	   _GrFillPatternExt(oldx, y, sx, sy, width, p);
	   width = 0;
	 }
	 if((mask >>= 1) == 0) { mask = 0x80; ++dptr; }
       } while ( ++xx < w );
       if ( width ) _GrFillPatternExt(oldx, y, sx, sy, width, p);
       bits += pitch;
     } while ( ++y < h );
   }
   else {
     int  widthbg;
     w += x; h += y;
     do {
       mask = 0x80;
       dptr = bits;
       oldx = xx = x;
       widthbg = width = 0;
       do {
	 if ( *dptr & mask ) {
	   if ( widthbg )
	   {
	     (*FDRV->drawhline)(oldx, y, widthbg, bg);
	     widthbg = 0;
	     oldx = xx;
	   }
	   width++;
	 }
	 else
	 {
	   if ( width )
	   {
	     _GrFillPatternExt(oldx, y, sx, sy, width, p);
	     width = 0;
	     oldx = xx;
	   }
	   widthbg++;
	 }
	 if((mask >>= 1) == 0) { mask = 0x80; ++dptr; }
       } while ( ++xx < w );
       if ( width   ) _GrFillPatternExt(oldx, y, sx, sy, width, p); else
       if ( widthbg ) (*FDRV->drawhline)(oldx, y, widthbg, bg);
       bits += pitch;
     } while ( ++y < h );
   }
   GRX_LEAVE();
}

void  _GrFillBitmapPattern(int x,int y,int w,int h,char far *bmp,int pitch,int start,GrPattern* p,GrColor bg)
{
   GRX_ENTER();
   _GrFillBitmapPatternExt(x, y, w, h, 0, 0, bmp, pitch, start, p, bg);
   GRX_LEAVE();
}

