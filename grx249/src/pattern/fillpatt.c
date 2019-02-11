/**
 ** fillpatt.c ---- draw a pattern filled horizontal line
 **
 ** Copyright (C) 1997, Michael Goffioul
 ** [e-mail : goffioul@emic.ucl.ac.be]
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
 ** Contributions by: (See "doc/credits.doc" for details)
 ** Hartmut Schirmer (hsc@techfak.uni-kiel.de)
 **
 **/

#include "libgrx.h"
#include "arith.h"
#include "shapes.h"

void _GrFillPatternExt(int x, int y, int sx, int sy, int width, GrPattern *p)
{
    GRX_ENTER();
    if (p->gp_ispixmap) {
	void (*bltfun)(GrFrame*,int,int,GrFrame*,int,int,int,int,GrColor);
	int pattwdt = p->gp_pxp_width;
        int xdest = x;
        int ydest = y;
	int ypatt = (y-sy) % p->gp_pxp_height;
	int xpatt = (x-sx) % pattwdt;
	int cpysize = pattwdt - xpatt;
	GrColor optype = p->gp_pxp_oper;

	if (CURC->gc_onscreen) bltfun = CURC->gc_driver->bltr2v;
	else                   bltfun = CURC->gc_driver->bitblt;
	while (width > 0) {
		if (cpysize > width) cpysize = width;
		(*bltfun)(
			&CURC->gc_frame,xdest,ydest,
			&p->gp_pxp_source,xpatt,ypatt,cpysize,1,
			optype
		);
		width -= cpysize;
		xpatt = 0;
		xdest += cpysize;
		cpysize = pattwdt;
	}
    }
    else {

	char bits = p->gp_bmp_data[y % p->gp_bmp_height];
	if (bits == 0)
	  (*CURC->gc_driver->drawhline)(x,y,width,p->gp_bmp_bgcolor);
	else if ((GR_int8u)bits == 0xff)
	  (*CURC->gc_driver->drawhline)(x,y,width,p->gp_bmp_fgcolor);
	else {
	  GrColor fg = p->gp_bmp_fgcolor;
	  GrColor bg = p->gp_bmp_bgcolor;
	  int xoffs = x & 7;
#         if USE_FDR_DRAWPATTERN-0
	      GR_int8u pp = replicate_b2w(bits) >> (8-xoffs);
	      (*CURC->gc_driver->drawpattern)(x,y,width,pp,fg,bg);
#         else
	      unsigned char mask = 0x80;
	      mask >>= xoffs;
	      width += x;
	      do {
		  (*CURC->gc_driver->drawpixel)(x,y,(bits & mask) ? fg : bg);
		  if((mask >>= 1) == 0) mask = 0x80;
	      } while(++x != width);
#         endif
	}
    }
    GRX_LEAVE();
}

void _GrFillPattern(int x, int y, int width, GrPattern *p)
{
  GRX_ENTER();
  _GrFillPatternExt(x,y,0,0,width,p);
  GRX_LEAVE();
}

void _GrFillPatternedScanLine(int x,int y,int w,GrFillArg arg)
{
  GRX_ENTER();
  _GrFillPatternExt(x,y,0,0,w,arg.p);
  GRX_LEAVE();
}
