/**
 ** bitbltnc.c ---- the pixel transfer routine (no clip)
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
#include "clipping.h"

void GrBitBltNC(GrContext *dst,int dx,int dy,
		GrContext *src,int x1,int y1,int x2,int y2,GrColor oper)
{
	void (*bltfun)(GrFrame*,int,int,GrFrame*,int,int,int,int,GrColor);
	if(dst == NULL) dst = CURC;
	if(src == NULL) src = CURC;
	isort(x1,x2);
	isort(y1,y2);
	if(src->gc_driver == dst->gc_driver)
	    bltfun = src->gc_driver->bitblt;
	else if(src->gc_driver->mode == dst->gc_driver->rmode)
	    bltfun = dst->gc_driver->bltr2v;
	else if(src->gc_driver->rmode == dst->gc_driver->mode)
	    bltfun = src->gc_driver->bltv2r;
	else return;
	(*bltfun)(
	    &dst->gc_frame,(dx + dst->gc_xoffset),(dy + dst->gc_yoffset),
	    &src->gc_frame,(x1 + src->gc_xoffset),(y1 + src->gc_yoffset),
	    (x2 - x1 + 1),
	    (y2 - y1 + 1),
	    oper
	);
}

