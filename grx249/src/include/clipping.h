/**
 ** clipping.h ---- macros to clip pixels lines and boxes
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

#ifndef __CLIPPING_H_INCLUDED__
#define __CLIPPING_H_INCLUDED__

#ifndef __ARITH_H_INCLUDED__
#include "arith.h"
#endif

/*
 * clip pixels and pixel ranges to the clip box
 */
#define clip_xdot_(c,x,when_out) {                                      \
    if((x > c->gc_xcliphi) || (x < c->gc_xcliplo)) { when_out; }        \
}

#define clip_ydot_(c,y,when_out) {                                      \
    if((y > c->gc_ycliphi) || (y < c->gc_ycliplo)) { when_out; }        \
}

#define clip_dot_(c,x,y,when_out) {                                     \
    clip_xdot_(c,x,when_out);                                           \
    clip_ydot_(c,y,when_out);                                           \
}

#define clip_ordxrange_(c,x1,x2,when_out,when_clip) {                   \
    if(x1 > c->gc_xcliphi) { when_out; }                                \
    if(x2 < c->gc_xcliplo) { when_out; }                                \
    if(x1 < c->gc_xcliplo) { x1 = c->gc_xcliplo; when_clip; }           \
    if(x2 > c->gc_xcliphi) { x2 = c->gc_xcliphi; when_clip; }           \
}

#define clip_ordyrange_(c,y1,y2,when_out,when_clip) {                   \
    if(y1 > c->gc_ycliphi) { when_out; }                                \
    if(y2 < c->gc_ycliplo) { when_out; }                                \
    if(y1 < c->gc_ycliplo) { y1 = c->gc_ycliplo; when_clip; }           \
    if(y2 > c->gc_ycliphi) { y2 = c->gc_ycliphi; when_clip; }           \
}

#define clip_xrange_(c,x1,x2,when_out,when_clip) {                      \
    isort(x1,x2);                                                       \
    clip_ordxrange_(c,x1,x2,when_out,when_clip);                        \
}

#define clip_yrange_(c,y1,y2,when_out,when_clip) {                      \
    isort(y1,y2);                                                       \
    clip_ordyrange_(c,y1,y2,when_out,when_clip);                        \
}

#define clip_ordbox_(c,x1,y1,x2,y2,when_out,when_clip) {                \
    clip_ordxrange_(c,x1,x2,when_out,when_clip);                        \
    clip_ordyrange_(c,y1,y2,when_out,when_clip);                        \
}

#define clip_box_(c,x1,y1,x2,y2,when_out,when_clip) {                   \
    clip_xrange_(c,x1,x2,when_out,when_clip);                           \
    clip_yrange_(c,y1,y2,when_out,when_clip);                           \
}

/*
 * clip pixels and pixel ranges to the full context
 */
#define cxclip_xdot_(c,x,when_out) {                                    \
    if((unsigned int)x > (unsigned int)c->gc_xmax) { when_out; }        \
}

#define cxclip_ydot_(c,y,when_out) {                                    \
    if((unsigned int)y > (unsigned int)c->gc_ymax) { when_out; }        \
}

#define cxclip_dot_(c,x,y,when_out) {                                   \
    cxclip_xdot_(c,x,when_out);                                         \
    cxclip_ydot_(c,y,when_out);                                         \
}

#define cxclip_ordxrange_(c,x1,x2,when_out,when_clip) {                 \
    if(x1 > c->gc_xmax) { when_out; }                                   \
    if(x2 < 0)          { when_out; }                                   \
    if(x1 < 0)          { x1 = 0;          when_clip; }                 \
    if(x2 > c->gc_xmax) { x2 = c->gc_xmax; when_clip; }                 \
}

#define cxclip_ordyrange_(c,y1,y2,when_out,when_clip) {                 \
    if(y1 > c->gc_ymax) { when_out; }                                   \
    if(y2 < 0)          { when_out; }                                   \
    if(y1 < 0)          { y1 = 0;          when_clip; }                 \
    if(y2 > c->gc_ymax) { y2 = c->gc_ymax; when_clip; }                 \
}

#define cxclip_xrange_(c,x1,x2,when_out,when_clip) {                    \
    isort(x1,x2);                                                       \
    cxclip_ordxrange_(c,x1,x2,when_out,when_clip);                      \
}

#define cxclip_yrange_(c,y1,y2,when_out,when_clip) {                    \
    isort(y1,y2);                                                       \
    cxclip_ordyrange_(c,y1,y2,when_out,when_clip);                      \
}

#define cxclip_ordbox_(c,x1,y1,x2,y2,when_out,when_clip) {              \
    cxclip_ordxrange_(c,x1,x2,when_out,when_clip);                      \
    cxclip_ordyrange_(c,y1,y2,when_out,when_clip);                      \
}

#define cxclip_box_(c,x1,y1,x2,y2,when_out,when_clip) {                 \
    cxclip_xrange_(c,x1,x2,when_out,when_clip);                         \
    cxclip_yrange_(c,y1,y2,when_out,when_clip);                         \
}

/*
 * clip lines to the clip box
 */
#define clip_hline_(c,x1,x2,y,when_out,when_clip) {                     \
    clip_ydot_(c,y,when_out);                                           \
    clip_xrange_(c,x1,x2,when_out,when_clip);                           \
}

#define clip_vline_(c,x,y1,y2,when_out,when_clip) {                     \
    clip_xdot_(c,x,when_out);                                           \
    clip_yrange_(c,y1,y2,when_out,when_clip);                           \
}

#define clip_line_xmin_(c,x1,y1,x2,y2,when_clip) {                      \
    if(x1 < c->gc_xcliplo) {                                            \
	y1 += irscale((y2 - y1),(c->gc_xcliplo - x1),(x2 - x1));        \
	x1  = c->gc_xcliplo;                                            \
	when_clip;                                                      \
    }                                                                   \
}

#define clip_line_xmax_(c,x1,y1,x2,y2,when_clip) {                      \
    if(x2 > c->gc_xcliphi) {                                            \
	y2 -= irscale((y2 - y1),(x2 - c->gc_xcliphi),(x2 - x1));        \
	x2  = c->gc_xcliphi;                                            \
	when_clip;                                                      \
    }                                                                   \
}

#define clip_line_ymin_(c,x1,y1,x2,y2,when_clip) {                      \
    if(y1 < c->gc_ycliplo) {                                            \
	x1 += irscale((x2 - x1),(c->gc_ycliplo - y1),(y2 - y1));        \
	y1  = c->gc_ycliplo;                                            \
	when_clip;                                                      \
    }                                                                   \
}

#define clip_line_ymax_(c,x1,y1,x2,y2,when_clip) {                      \
    if(y2 > c->gc_ycliphi) {                                            \
	x2 -= irscale((x2 - x1),(y2 - c->gc_ycliphi),(y2 - y1));        \
	y2  = c->gc_ycliphi;                                            \
	when_clip;                                                      \
    }                                                                   \
}

#define clip_line_(c,x1,y1,x2,y2,when_out,when_clip) {                  \
    if(x1 < x2) {                                                       \
	if(x2 < c->gc_xcliplo) { when_out; }                            \
	if(x1 > c->gc_xcliphi) { when_out; }                            \
	clip_line_xmin_(c,x1,y1,x2,y2,when_clip);                       \
	clip_line_xmax_(c,x1,y1,x2,y2,when_clip);                       \
    }                                                                   \
    else {                                                              \
	if(x1 < c->gc_xcliplo) { when_out; }                            \
	if(x2 > c->gc_xcliphi) { when_out; }                            \
	clip_line_xmin_(c,x2,y2,x1,y1,when_clip);                       \
	clip_line_xmax_(c,x2,y2,x1,y1,when_clip);                       \
    }                                                                   \
    if(y1 < y2) {                                                       \
	if(y2 < c->gc_ycliplo) { when_out; }                            \
	if(y1 > c->gc_ycliphi) { when_out; }                            \
	clip_line_ymin_(c,x1,y1,x2,y2,when_clip);                       \
	clip_line_ymax_(c,x1,y1,x2,y2,when_clip);                       \
    }                                                                   \
    else {                                                              \
	if(y1 < c->gc_ycliplo) { when_out; }                            \
	if(y2 > c->gc_ycliphi) { when_out; }                            \
	clip_line_ymin_(c,x2,y2,x1,y1,when_clip);                       \
	clip_line_ymax_(c,x2,y2,x1,y1,when_clip);                       \
    }                                                                   \
}

/*
 * clipping with default actions: outside => return, clipped => nothing
 */
/* some systems have problems with emtpy macro args ... */
#if defined(__GNUC__) || defined(__TURBOC__)
#define CLIP_EMPTY_MACRO_ARG
#endif

#ifndef CLIP_EMPTY_MACRO_ARG
#define CLIP_EMPTY_MACRO_ARG  do ; while(0)
#endif

#define clip_xdot(c,x)                  clip_xdot_(c,x,return)
#define clip_ydot(c,x)                  clip_ydot_(c,y,return)
#define clip_dot(c,x,y)                 clip_dot_(c,x,y,return)
#define clip_ordxrange(c,x1,x2)         clip_ordxrange_(c,x1,x2,return,CLIP_EMPTY_MACRO_ARG)
#define clip_ordyrange(c,y1,y2)         clip_ordyrange_(c,y1,y2,return,CLIP_EMPTY_MACRO_ARG)
#define clip_xrange(c,x1,x2)            clip_xrange_(c,x1,x2,return,CLIP_EMPTY_MACRO_ARG)
#define clip_yrange(c,y1,y2)            clip_yrange_(c,y1,y2,return,CLIP_EMPTY_MACRO_ARG)
#define clip_ordbox(c,x1,y1,x2,y2)      clip_ordbox_(c,x1,y1,x2,y2,return,CLIP_EMPTY_MACRO_ARG)
#define clip_box(c,x1,y1,x2,y2)         clip_box_(c,x1,y1,x2,y2,return,CLIP_EMPTY_MACRO_ARG)

#define cxclip_xdot(c,x)                cxclip_xdot_(c,x,return)
#define cxclip_ydot(c,x)                cxclip_ydot_(c,y,return)
#define cxclip_dot(c,x,y)               cxclip_dot_(c,x,y,return)
#define cxclip_ordxrange(c,x1,x2)       cxclip_ordxrange_(c,x1,x2,return,CLIP_EMPTY_MACRO_ARG)
#define cxclip_ordyrange(c,y1,y2)       cxclip_ordyrange_(c,y1,y2,return,CLIP_EMPTY_MACRO_ARG)
#define cxclip_xrange(c,x1,x2)          cxclip_xrange_(c,x1,x2,return,CLIP_EMPTY_MACRO_ARG)
#define cxclip_yrange(c,y1,y2)          cxclip_yrange_(c,y1,y2,return,CLIP_EMPTY_MACRO_ARG)
#define cxclip_ordbox(c,x1,y1,x2,y2)    cxclip_ordbox_(c,x1,y1,x2,y2,return,CLIP_EMPTY_MACRO_ARG)
#define cxclip_box(c,x1,y1,x2,y2)       cxclip_box_(c,x1,y1,x2,y2,return,CLIP_EMPTY_MACRO_ARG)

#define clip_hline(c,x1,x2,y)           clip_hline_(c,x1,x2,y,return,CLIP_EMPTY_MACRO_ARG)
#define clip_vline(c,x,y1,y2)           clip_vline_(c,x,y1,y2,return,CLIP_EMPTY_MACRO_ARG)
#define clip_line_xmin(c,x1,y1,x2,y2)   clip_line_xmin_(c,x1,y1,x2,y2,CLIP_EMPTY_MACRO_ARG)
#define clip_line_xmax(c,x1,y1,x2,y2)   clip_line_xmax_(c,x1,y1,x2,y2,CLIP_EMPTY_MACRO_ARG)
#define clip_line_ymin(c,x1,y1,x2,y2)   clip_line_ymin_(c,x1,y1,x2,y2,CLIP_EMPTY_MACRO_ARG)
#define clip_line_ymax(c,x1,y1,x2,y2)   clip_line_ymax_(c,x1,y1,x2,y2,CLIP_EMPTY_MACRO_ARG)
#define clip_line(c,x1,y1,x2,y2)        clip_line_(c,x1,y1,x2,y2,return,CLIP_EMPTY_MACRO_ARG)

#endif /* whole file */

