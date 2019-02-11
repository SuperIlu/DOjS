/**
 ** drwcpoly.c ---- draw the outline of a custom (wide and/or dashed) polygon
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
#include "memcopy.h"

/*
 * update the end point of line #1 and the starting point of line #2
 * so that they intersect
 */
static void intersect(int l1s[2],int l1e[2],int l2s[2],int l2e[2])
{
#   define x11 l1s[0]
#   define y11 l1s[1]
#   define x12 l1e[0]
#   define y12 l1e[1]
#   define x21 l2s[0]
#   define y21 l2s[1]
#   define x22 l2e[0]
#   define y22 l2e[1]
    if(x12 != x21 || y12 != y21) {
	int  dx1 = x12 - x11;
	int  dy1 = y12 - y11;
	int  dx2 = x22 - x21;
	int  dy2 = y22 - y21;
	long det = imul32(dx2,dy1) - imul32(dx1,dy2);
	if( det != 0 ) {
	    /* Compute t for the parametric equation of line #2 */
	    /* then: x = x21 + dx2 * t2, and y = y21 + dy2 * t2 */
	    /* but do this with integer arithmetic */
	    /* and do rounding instead of truncation */
	    long det_t2  = imul32(dx1,(y21 - y11)) - imul32(dy1,(x21 - x11));
	    long ldet = labs(det);
	    /* make sure we're still near old start point of line #2 */
	    if( labs(det_t2) < (((ldet<<1) + ldet)>>1) ) {
		int xdif2 = (int)(((long)(dx2 << 1) * det_t2) / det);
		int ydif2 = (int)(((long)(dy2 << 1) * det_t2) / det);
		if(xdif2 > 0) xdif2++;
		if(ydif2 > 0) ydif2++;
		xdif2 = x21 + (xdif2 >> 1);
		ydif2 = y21 + (ydif2 >> 1);
		/* don't create triangles */
		if ( xdif2 != x11 && xdif2 != x22 &&
		     ydif2 != y11 && ydif2 != y22    ) {
		  l1e[0] = l2s[0] = xdif2;
		  l1e[1] = l2s[1] = ydif2;
		  return;
		}
	    }
	}
	{   /*
	    ** no good intersection point till now
	    ** Check mean point and its eight neighbours for
	    ** best compromise intersection point
	    **
	    **  y-y11   y12-y11           y-y21   y22-y21
	    **  ----- - ------- = 0  and  ----- - ------- = 0
	    **  x-x11   x12-x11           x-x21   x22-x21
	    **
	    ** should hold for intersection point (x,y)
	    ** Measuring the errors for both lines:
	    **
	    ** e1 = (x12-x11)(y-y11)-(x-x11)(y12-y11) = dx1(y-y11)-(x-x11)dy1
	    ** e2 = (x22-x21)(y-y21)-(x-x21)(y22-y21) = dx2(y-y21)-(x-x21)dy2
	    **
	    ** search minimal err = |e1| + |e2|
	    */
	    static int xr[9] = { 0, -1, 0, 1,  0, -1, 1,  1, -1};
	    static int yr[9] = { 0,  0, 1, 0, -1,  1, 1, -1, -1};
	    int xc = (x12+x21) >> 1;
	    int yc = (y12+y21) >> 1;
	    int xb = 0, yb = 0;
	    int i;
	    long minerr=0, err;
	    for (i = 0; i < 9; ++i) {
		int x = xc+xr[i];
		int y = yc+yr[i];
		long e1 = imul32(dx1,(y-y11)) - imul32(dy1,(x-x11));
		long e2 = imul32(dx2,(y-y21)) - imul32(dy2,(x-x21));
		err = labs(e1) + labs(e2);
		if ( i==0 || err < minerr) {
		  minerr = err;
		  xb = xr[i]; yb = yr[i];
		  if (minerr == 0) break;
		}
	    }
	    l1e[0] = l2s[0] = xc + xb;
	    l1e[1] = l2s[1] = yc + yb;
	}
    }
#   undef x11
#   undef y11
#   undef x12
#   undef y12
#   undef x21
#   undef y21
#   undef x22
#   undef y22
}

/*
 * generate the four corner points of a wide line segment
 *
 * p1 end : rect[0]  rect[1]
 * p2 end : rect[2]  rect[3]
 *
 */
static void genrect(int p1[2],int p2[2],int w,int rect[4][2])
{
	int dx  = p2[0] - p1[0];
	int dy  = p2[1] - p1[1];
	int wx,wy,wx1,wy1,wx2,wy2;

	if(dx == 0) {
	    wx = w;
	    wy = 0;
	}
	else if (dy == 0) {
	   wx = 0;
	   wy = w;
	}
	else {
	    unsigned long minerr,error = ~0UL,w2 = imul32(w,w);
	    int mindelta = umin(iabs(dx),iabs(dy));
	    int maxdelta = umax(iabs(dx),iabs(dy));
	    wx1 = w/2;
	    if (wx1 <= 0) wx1 = 1;
	    if (wx1+wx1 < w) ++wx1;
	    wy1 = 0;
	    do {
		wx  = wx1++;
		wy  = wy1;
		wy1 = urscale(wx1,mindelta,maxdelta);
		minerr = error;
		error  = imul32(wx1,wx1) + imul32(wy1,wy1) - w2;
		error  = labs(error);
	    } while(error <= minerr);
	    if(iabs(dx) > iabs(dy)) iswap(wx,wy);
	}
	if(dx <  0) wy = (-wy);
	if(dy >= 0) wx = (-wx);
	wx1 = -(wx >> 1);
	wy1 = -(wy >> 1);
	wx2 = wx + wx1;
	wy2 = wy + wy1;
	if((wx1 + wx2) < 0) wx1++,wx2++;
	if((wy1 + wy2) < 0) wy1++,wy2++;
	rect[0][0] = p1[0] + wx1;
	rect[0][1] = p1[1] + wy1;
	rect[1][0] = p1[0] + wx2;
	rect[1][1] = p1[1] + wy2;
	rect[2][0] = p2[0] + wx2;
	rect[2][1] = p2[1] + wy2;
	rect[3][0] = p2[0] + wx1;
	rect[3][1] = p2[1] + wy1;
}

/*
 * working version of the line pattern and fill argument structures
 */
typedef struct {
    int       w;                /* line width */
    int       psegs;            /* number of pattern segments */
    int       plength;          /* total length of pattern in pixels */
    int       ppos;             /* current pattern position (modulo plength) */
    int       on;               /* is the pattern currently on ? */
    unsigned char *patt;        /* the pattern bits */
    GrFiller *f;                /* the filler functions */
    GrFillArg c;                /* the filler argument */
} linepatt;

static void solidsegment1(
    int p1[2],  int p2[2],
    int prev[2],int next[2],
    linepatt *p
){
	int x1 = p1[0], y1 = p1[1];
	int x2 = p2[0], y2 = p2[1];
	(*p->f->line)(
	    (x1 + CURC->gc_xoffset),
	    (y1 + CURC->gc_yoffset),
	    (x2 - x1),
	    (y2 - y1),
	    p->c
	);
}

static void solidsegmentw(
    int p1[2],  int p2[2],
    int prev[2],int next[2],
    linepatt *p
){
	int rect[4][2], prect[4][2], nrect[4][2];
	genrect(p1,p2,p->w,rect);
	if(prev) genrect(prev,p1,p->w,prect);
	if(next) genrect(p2,next,p->w,nrect);
	if(prev && next) {
	    int points[2];
	    points[0] = rect[1][0]; points[1] = rect[1][1];
	    intersect(prect[1],prect[2],rect[1],rect[2]);
	    intersect(points,rect[2],nrect[1],nrect[2]);
	    points[0] = rect[0][0]; points[1] = rect[0][1];
	    intersect(prect[0],prect[3],rect[0],rect[3]);
	    intersect(points,rect[3],nrect[0],nrect[3]);
	} else
	if(prev) {
	    intersect(prect[1],prect[2],rect[1],rect[2]);
	    intersect(prect[0],prect[3],rect[0],rect[3]);
	} else
	if(next) {
	    intersect(rect[1],rect[2],nrect[1],nrect[2]);
	    intersect(rect[0],rect[3],nrect[0],nrect[3]);
	}
	_GrScanConvexPoly(4,rect,p->f,p->c);
}

static void dashedsegment(
    int p1[2],  int p2[2],
    int prev[2],int next[2],
    linepatt *p,
    void (*doseg)(int[2],int[2],int[2],int[2],linepatt*)
){
	int on,pos,len,seg;
	int x,y,dx,dy;
	int error,erradd,errsub,count;
	int xinc1,xinc2,yinc1,yinc2;
	int start[2],end[2], se_init;

	/* find the current starting segment for the pattern */
	pos = (p->ppos %= p->plength);
	for(on = 1,seg = 0; ; ) {
	    len = p->patt[seg];
	    if(pos < len) { len -= pos; break; }
	    if(++seg >= p->psegs) seg = 0;
	    on  ^= 1;
	    pos -= len;
	}
	/* Can't have a zero length element here */

	/* set up line drawing */
	x = p1[0]; dx = p2[0] - x;
	y = p1[1]; dy = p2[1] - y;
	if(dx >= 0) { xinc2 =  1; }
	else        { xinc2 = -1; dx = -dx; }
	if(dy >= 0) { yinc2 =  1; }
	else        { yinc2 = -1; dy = -dy; }
	if(dx >= dy) {
	    count  = dx +  1;
	    error  = dx >> 1;
	    errsub = dy;
	    erradd = dx;
	    xinc1  = xinc2;
	    yinc1  = 0;
	}
	else {
	    count  = dy +  1;
	    error  = dy >> 1;
	    errsub = dx;
	    erradd = dy;
	    yinc1  = yinc2;
	    xinc1  = 0;
	}
	se_init = 0;
	if(on) {
	    start[0] = x;
	    start[1] = y;
	    se_init = 1;
	}
	else {
	    prev = NULL;
	}
	/* go */
	while(--count >= 0) {
	    if(on) {
		end[0] = x;
		end[1] = y;
		se_init |= 2;
	    }
	    if((error -= errsub) < 0) {
		error += erradd;
		x += xinc2;
		y += yinc2;
	    }
	    else {
		x += xinc1;
		y += yinc1;
	    }
	    if(--len <= 0) {
		/* end of current segment */
		int old_state = on;
		do {
		  if(++seg >= p->psegs) seg = 0;
		  len = p->patt[seg];
		  on ^= 1;
		} while (len == 0);
		if ( !old_state &&  on && count > 0) {
		    start[0] = x;
		    start[1] = y;
		    se_init = 1;
		} else
		if (  old_state && !on) {
		    (*doseg)(start,end,prev,NULL,p);
		    prev = NULL;
		    se_init = 0;
		}
		/* else: zero length element(s), continue current segment */
	    }
	}
	if(on && se_init==3) {
	    (*doseg)(start,end,prev,next,p);
	}
	p->on = on;
}

static void dashedsegment1(
    int p1[2],  int p2[2],
    int prev[2],int next[2],
    linepatt *p
){
	dashedsegment(p1,p2,prev,next,p,solidsegment1);
}

static void dashedsegmentw(
    int p1[2],  int p2[2],
    int prev[2],int next[2],
    linepatt *p
){
	dashedsegment(p1,p2,prev,next,p,solidsegmentw);
}

void _GrDrawCustomPolygon(
     int n,int pt[][2],
     const GrLineOption *lp,
     GrFiller *f,GrFillArg c,
     int doClose,int circle
){
#       define x1 start[0]
#       define y1 start[1]
#       define x2 end[0]
#       define y2 end[1]
	int  i,start[2],end[2];
	void (*doseg)(int[2],int[2],int[2],int[2],linepatt*);
	linepatt  p;
	GrContext preclip;
	if(n < 2) return;
	/* set up working pattern */
	p.f       = f;
	p.c       = c;
	p.w       = imax((lp->lno_width - 1),0);
	p.ppos    = 0;
	p.patt    = lp->lno_dashpat;
	p.psegs   = p.patt ? imax(lp->lno_pattlen,0) : 0;
	p.plength = 0;
	for(i = 0; i < p.psegs; i++) {
/*          if(!p.patt[i]) { p.plength = 0; break; } */
	    p.plength += p.patt[i];
	}
	if(p.plength)
	    doseg = p.w ? dashedsegmentw : dashedsegment1;
	else {
	    if (p.psegs && p.patt[0]==0 ) return; /* nothing to do */
	    doseg = p.w ? solidsegmentw : solidsegment1;
	}
	/* preclip */
	x1 = x2 = pt[0][0];
	y1 = y2 = pt[0][1];
	for(i = 1; i < n; i++) {
	    int *ppt = pt[i];
	    if(x1 > ppt[0]) x1 = ppt[0];
	    if(x2 < ppt[0]) x2 = ppt[0];
	    if(y1 > ppt[1]) y1 = ppt[1];
	    if(y2 < ppt[1]) y2 = ppt[1];
	}
	sttcopy(&preclip,CURC);
	preclip.gc_xcliplo -= p.w; preclip.gc_ycliplo -= p.w;
	preclip.gc_xcliphi += p.w; preclip.gc_ycliphi += p.w;
	clip_ordbox((&preclip),x1,y1,x2,y2);
	mouse_block(CURC,x1,y1,x2,y2);
	/* do the polygon segments */
	if(doClose) {
	    int *p1 = pt[0], *p2 = pt[n - 1];
	    if((n > 1) && (p1[0] == p2[0]) && (p1[1] == p2[1])) n--;
	    if(n < 3) doClose = FALSE;
	}
	for(i = 0; i < n; i++) {
	    int clipped,xmajor,length;
	    int *p1,*p2,*prev,*next;
	    if(!(i + doClose)) continue;
	    p1 = pt[(i + n - 1) % n];
	    p2 = pt[i];
	    prev = ((i > 1) || doClose) ? pt[(i + n - 2) % n] : NULL;
	    next = ((i < (n - 1)) || doClose) ? pt[(i + 1) % n] : NULL;
	    x1 = p1[0];
	    y1 = p1[1];
	    x2 = p2[0];
	    y2 = p2[1];
	    clipped = 0;
	    xmajor  = iabs(x1 - x2);
	    length  = iabs(y1 - y2);
	    if(xmajor > length) { length = xmajor; xmajor = 1; }
	    else xmajor = 0;
	    clip_line_((&preclip),x1,y1,x2,y2,goto outside,clipped = p.plength);
	    if(clipped) {
		clipped = xmajor ? iabs(p1[0] - x1) : iabs(p1[1] - y1);
		p.ppos += clipped;
		length -= clipped;
	    }
	    (*doseg)(start,end,prev,next,&p);
	  outside:
	    p.ppos += length;
	}
	mouse_unblock();
#       undef x1
#       undef y1
#       undef x2
#       undef y2
}


