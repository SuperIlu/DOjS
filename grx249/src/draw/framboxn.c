/**
 ** framboxn.c ---- framed (possibly shaded) box primitive (no clip)
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

void GrFramedBoxNC(int x1,int y1,int x2,int y2,int wdt,const GrFBoxColors *c)
{
	isort(x1,x2);
	isort(y1,y2);
	if(wdt < 0) wdt = 0;
	if(c->fbx_intcolor != GrNOCOLOR) {
	    GrFilledBoxNC(x1,y1,x2,y2,c->fbx_intcolor);
	}
	while(--wdt >= 0) {
	    x1--; x2++;
	    y1--; y2++;
	    GrHLineNC(x1,x2,y1,c->fbx_topcolor);
	    GrVLineNC(x1,(y1 + 1),(y2 - 1),c->fbx_leftcolor);
	    GrVLineNC(x2,(y1 + 1),(y2 - 1),c->fbx_rightcolor);
	    GrHLineNC(x1,x2,y2,c->fbx_bottomcolor);
	}
}

