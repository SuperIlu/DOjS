/**
 ** scancnvx.c ---- scan fill a convex polygon
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
#include "shape/polyedge.h"

typedef struct {
    unsigned int dir;		/* which direction to go for next point */
    unsigned int index;		/* index of the current point */
    polyedge e;
} edge;

#define next_edge(ed,n,pt) {					\
    ed.index   = (ed.index + ed.dir) % (unsigned int)n;		\
    ed.e.x     = ed.e.xlast;					\
    ed.e.y     = ed.e.ylast;					\
    ed.e.xlast = pt[ed.index][0];				\
    ed.e.ylast = pt[ed.index][1];				\
}

void _GrScanConvexPoly(int n,int pt[][2],GrFiller *f,GrFillArg c)
{
	edge L,R;
	int  xmin,xmax;
	int  ymin,ymax;
	int  ypos,i;
	if((n > 1) &&
	   (pt[0][0] == pt[n - 1][0]) &&
	   (pt[0][1] == pt[n - 1][1])) {
	    n--;
	}
	if(n < 1) {
	    return;
	}
	xmin = xmax = pt[0][0];
	ymin = ymax = pt[0][1];
	ypos = 0;
	for(i = 1; i < n; i++) {
	    int *ppt = pt[i];
	    if(ymin > ppt[1]) ymin = ppt[1],ypos = i;
	    if(ymax < ppt[1]) ymax = ppt[1];
	    if(xmin > ppt[0]) xmin = ppt[0];
	    if(xmax < ppt[0]) xmax = ppt[0];
	}
	clip_ordbox(CURC,xmin,ymin,xmax,ymax);
	mouse_block(CURC,xmin,ymin,xmax,ymax);
	L.dir	  = 1;
	R.dir	  = n - 1;
	L.index	  = R.index   = ypos;
	L.e.xlast = R.e.xlast = pt[ypos][0];
	L.e.ylast = R.e.ylast = pt[ypos][1];
	for( ; ; ) {
	    next_edge(L,n,pt);
	    if(L.e.ylast >= ymin) {
		clip_line_ymin(CURC,L.e.x,L.e.y,L.e.xlast,L.e.ylast);
		setup_edge(&L.e);
		break;
	    }
	}
	for( ; ; ) {
	    next_edge(R,n,pt);
	    if(R.e.ylast >= ymin) {
		clip_line_ymin(CURC,R.e.x,R.e.y,R.e.xlast,R.e.ylast);
		setup_edge(&R.e);
		break;
	    }
	}
	for(ypos = ymin; ypos <= ymax; ypos++) {
	    xmin = L.e.x;
	    xmax = L.e.x;
	    if(ypos == L.e.ylast) {
		xmin = imin(xmin,L.e.xlast);
		xmax = imax(xmax,L.e.xlast);
		if(ypos < ymax) for( ; ; ) {
		    next_edge(L,n,pt);
		    if(L.e.ylast > ypos) {
			setup_edge(&L.e);
			break;
		    }
		    xmin = imin(xmin,L.e.xlast);
		    xmax = imax(xmax,L.e.xlast);
		}
	    }
	    if(ypos != ymax) {
		if(L.e.xmajor) {
		    xstep_edge(&L.e);
		    xmin = imin(xmin,(L.e.x - L.e.xstep));
		    xmax = imax(xmax,(L.e.x - L.e.xstep));
		}
		else {
		    ystep_edge(&L.e);
		}
	    }
	    xmin = imin(xmin,R.e.x);
	    xmax = imax(xmax,R.e.x);
	    if(ypos == R.e.ylast) {
		xmin = imin(xmin,R.e.xlast);
		xmax = imax(xmax,R.e.xlast);
		if(ypos < ymax) for( ; ; ) {
		    next_edge(R,n,pt);
		    if(R.e.ylast > ypos) {
			setup_edge(&R.e);
			break;
		    }
		    xmin = imin(xmin,R.e.xlast);
		    xmax = imax(xmax,R.e.xlast);
		}
	    }
	    if(ypos != ymax) {
		if(R.e.xmajor) {
		    xstep_edge(&R.e);
		    xmin = imin(xmin,(R.e.x - R.e.xstep));
		    xmax = imax(xmax,(R.e.x - R.e.xstep));
		}
		else {
		    ystep_edge(&R.e);
		}
	    }
	    clip_ordxrange_(CURC,xmin,xmax,continue,CLIP_EMPTY_MACRO_ARG);
	    (*f->scan)(
		(xmin + CURC->gc_xoffset),
		(ypos + CURC->gc_yoffset),
		(xmax - xmin + 1),
		c
	    );
	}
	mouse_unblock();
}

