/**
 ** genblit.c ---- generic, VERY SLOW bitblt routine
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu].
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
#include "grdriver.h"
#include "memcopy.h"

void _GrFrDrvGenericBitBlt(GrFrame *dst,int dx,int dy,GrFrame *src,int x,int y,int w,int h,GrColor op)
{
	GrFrame csave;
	GrColor (*readpix)(GrFrame *,int,int);
	void (*drawpix)(int,int,GrColor);
	GrColor skipc;
	int  step;
	GRX_ENTER();
	readpix = src->gf_driver->readpixel;
	drawpix = dst->gf_driver->drawpixel;
	skipc = op ^ GrIMAGE;
	step  = 1;
	op &= GrCMODEMASK;
	sttcopy(&csave,&CURC->gc_frame);
	sttcopy(&CURC->gc_frame,dst);
	if((dy > y) || ((dy == y) && (dx > x))) {
	    x += (w - 1); dx += (w - 1);
	    y += (h - 1); dy += (h - 1);
	    step = (-1);
	}
	do {
	    int dxx = dx,xx = x,ww = w;
	    do {
		GrColor c = (*readpix)(src,xx,y);
		if(c != skipc) (*drawpix)(dxx,dy,(c | op));
		dxx += step; xx += step;
	    } while(--ww > 0);
	    dy += step; y += step;
	} while(--h > 0);
	sttcopy(&CURC->gc_frame,&csave);
	GRX_LEAVE();
}

