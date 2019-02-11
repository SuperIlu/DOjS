/**
 ** scanpoly.c ---- scan fill an arbitrary polygon
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
#include "allocate.h"
#include "clipping.h"
#include "arith.h"
#include "shape/polyedge.h"

typedef enum {
    inactive,                           /* not reached yet */
    active,                             /* currently contributes point */
    passed                              /* above current scan line */
} edgestat;

typedef struct {
    edgestat status;                    /* status of this edge */
    polyedge e;                         /* the edge data */
} edge;

typedef struct _scan {
    struct _scan *next;                 /* next segment/point in the list */
    int    x1,x2;                       /* endpoints of this filled segment */
} scan;

#define add_scanpoint(List,Scp,X1,X2) {                         \
    scan *prev = NULL;                                          \
    scan *work = List;                                          \
    while(work != NULL) {                                       \
	if(work->x1 > X1) break;                                \
	prev = work;                                            \
	work = work->next;                                      \
    }                                                           \
    Scp->x1   = X1;                                             \
    Scp->x2   = X2;                                             \
    Scp->next = work;                                           \
    if(prev) prev->next = Scp;                                  \
    else     List       = Scp;                                  \
}

#define add_scansegment(List,Scp,X1,X2) {                       \
    scan *prev   = NULL;                                        \
    scan *work   = List;                                        \
    int  overlap = FALSE;                                       \
    while(work != NULL) {                                       \
	if((work->x1 <= X2) && (X1 <= work->x2)) {              \
	    overlap = TRUE;                                     \
	    if(X1 < work->x1) work->x1 = X1;                    \
	    if(X2 > work->x2) {                                 \
		prev = work;                                    \
		while((work = work->next) != NULL) {            \
		    if(work->x1 > X2) break;                    \
		    if(work->x2 > X2) X2 = work->x2;            \
		}                                               \
		prev->x2   = X2;                                \
		prev->next = work;                              \
	    }                                                   \
	    break;                                              \
	}                                                       \
	if(work->x1 > X2) break;                                \
	prev = work;                                            \
	work = work->next;                                      \
    }                                                           \
    if(!overlap) {                                              \
	Scp->x1   = X1;                                         \
	Scp->x2   = X2;                                         \
	Scp->next = work;                                       \
	if(prev) prev->next = Scp;                              \
	else     List       = Scp;                              \
    }                                                           \
}

void _GrScanPolygon(int n,int pt[][2],GrFiller *f,GrFillArg c)
{
	edge *edges,*ep;
	scan *scans,*sp,*points,*segments;
	int  xmin,xmax,ymin,ymax;
	int  ypos,nedges;
	if((n > 1) &&
	   (pt[0][0] == pt[n-1][0]) &&
	   (pt[0][1] == pt[n-1][1])) {
	    n--;
	}
	if(n < 1) {
	    return;
	}
	setup_ALLOC();
	edges = (edge *)ALLOC(sizeof(edge) * (n + 2));
	scans = (scan *)ALLOC(sizeof(scan) * (n + 8));
	if(edges && scans) {
	    /*
	     * Build the edge table. Store only those edges which are in the
	     * valid Y region. Clip them in Y if necessary. Store them with
	     * the endpoints ordered by Y in the edge table.
	     */
	    int prevx = xmin = xmax = pt[0][0];
	    int prevy = ymin = ymax = pt[0][1];
	    nedges = 0;
	    ep     = edges;
	    while(--n >= 0) {
		if(pt[n][1] >= prevy) {
		    ep->e.x     = prevx;
		    ep->e.y     = prevy;
		    ep->e.xlast = prevx = pt[n][0];
		    ep->e.ylast = prevy = pt[n][1];
		}
		else {
		    ep->e.xlast = prevx;
		    ep->e.ylast = prevy;
		    ep->e.x     = prevx = pt[n][0];
		    ep->e.y     = prevy = pt[n][1];
		}
		if((ep->e.y > GrHighY()) || (ep->e.ylast < GrLowY())) continue;
		clip_line_ymin(CURC,ep->e.x,ep->e.y,ep->e.xlast,ep->e.ylast);
		if(ymin > ep->e.y)     ymin = ep->e.y;
		if(ymax < ep->e.ylast) ymax = ep->e.ylast;
		if(xmin > ep->e.x)     xmin = ep->e.x;
		if(xmax < ep->e.xlast) xmax = ep->e.xlast;
		setup_edge(&ep->e);
		ep->status = inactive;
		nedges++;
		ep++;
	    }
	    if((nedges > 0) && (xmin <= GrHighX()) && (xmax >= GrLowX())) {
		if(xmin < GrLowX())  xmin = GrLowX();
		if(ymin < GrLowY())  ymin = GrLowY();
		if(xmax > GrHighX()) xmax = GrHighX();
		if(ymax > GrHighY()) ymax = GrHighY();
		mouse_block(CURC,xmin,ymin,xmax,ymax);
		/*
		 * Scan for every row between ymin and ymax.
		 * Build a linked list of disjoint segments to fill. Rules:
		 *   (1) a horizontal edge in the row contributes a segment
		 *   (2) any other edge crossing the row contributes a point
		 *   (3) every segment between even and odd points is filled
		 */
		for(ypos = ymin; ypos <= ymax; ypos++) {
		    sp       = scans;
		    points   = NULL;
		    segments = NULL;
		    for(n = nedges,ep = edges; --n >= 0; ep++) {
			switch(ep->status) {
			  case inactive:
			    if(ep->e.y != ypos) break;
			    if(ep->e.dy == 0) {
				ep->status = passed;
				xmin = ep->e.x;
				xmax = ep->e.xlast;
				isort(xmin,xmax);
				add_scansegment(segments,sp,xmin,xmax);
				sp++;
				break;
			    }
			    ep->status = active;
			  case active:
			    xmin = xmax = ep->e.x;
			    if(ep->e.ylast == ypos) {
				ep->status = passed;
				xmax = ep->e.xlast;
				isort(xmin,xmax);
				add_scanpoint(points,sp,xmin,xmax);
				sp++;
			    }
			    else if(ep->e.xmajor) {
				xstep_edge(&ep->e);
				xmax = ep->e.x - ep->e.xstep;
				isort(xmin,xmax);
			    }
			    else {
				ystep_edge(&ep->e);
			    }
			    add_scanpoint(points,sp,xmin,xmax);
			    sp++;
			    break;
			  default:
			    break;
			}
		    }
		    while(points != NULL) {
			scan *nextpt = points->next;
			if(!nextpt) break;
			xmin   = points->x1;
			xmax   = nextpt->x2;
			points = nextpt->next;
			add_scansegment(segments,nextpt,xmin,xmax);
		    }
		    while(segments != NULL) {
			xmin     = segments->x1;
			xmax     = segments->x2;
			segments = segments->next;
			clip_ordxrange_(CURC,xmin,xmax,continue,CLIP_EMPTY_MACRO_ARG);
			(*f->scan)(
			    (xmin + CURC->gc_xoffset),
			    (ypos + CURC->gc_yoffset),
			    (xmax - xmin + 1),
			    c
			);
		    }
		}
		mouse_unblock();
	    }
	}
	if (edges) FREE(edges);
	if (scans) FREE(scans);
	reset_ALLOC();
}

