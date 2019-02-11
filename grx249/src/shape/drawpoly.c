/**
 ** drawpoly.c ---- draw the outline of a polygon
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

#include "libgrx.h"
#include "shapes.h"
#include "clipping.h"
#include "arith.h"

void _GrDrawPolygon(int n,int pt[][2],GrFiller *f,GrFillArg c,int doClose)
{
	int i,px,py,x1,y1,x2,y2;
	if(n <= 0) return;
	if(n == 1) doClose = TRUE;
	x1 = x2 = pt[0][0];
	y1 = y2 = pt[0][1];
	for(i = 1; i < n; i++) {
	    int *ppt = pt[i];
	    if(x1 > ppt[0]) x1 = ppt[0];
	    if(x2 < ppt[0]) x2 = ppt[0];
	    if(y1 > ppt[1]) y1 = ppt[1];
	    if(y2 < ppt[1]) y2 = ppt[1];
	}
	clip_ordbox(CURC,x1,y1,x2,y2);
	mouse_block(CURC,x1,y1,x2,y2);
	px = pt[n - 1][0];
	py = pt[n - 1][1];
	for(i = 0; i < n; i++) {
	    x1 = px;
	    y1 = py;
	    x2 = px = pt[i][0];
	    y2 = py = pt[i][1];
	    if(i | doClose) {
		if(y1 > y2) {
		    iswap(x1,x2);
		    iswap(y1,y2);
		}
		clip_line_(CURC,x1,y1,x2,y2,continue,CLIP_EMPTY_MACRO_ARG);
		(*f->line)(
		    (x1 + CURC->gc_xoffset),
		    (y1 + CURC->gc_yoffset),
		    (x2 - x1),
		    (y2 - y1),
		    c
		);
	    }
	}
	mouse_unblock();
}

