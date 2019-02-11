/**
 ** polyedge.h ---- polygon edge structures for scan routines
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

typedef struct {
    int x,xlast;		/* current and final X coordinates */
    int y,ylast;		/* current and final Y coordinates */
    int dx,dy;			/* line diagonals */
    int xmajor;			/* flag for X major lines */
    int xstep;			/* direction of X scan */
    int error;			/* Bresenham error term */
} polyedge;

#define setup_edge(ep) {						\
    (ep)->dy = (ep)->ylast - (ep)->y;					\
    (ep)->dx = (ep)->xlast - (ep)->x;					\
    if((ep)->dx < 0) {							\
	(ep)->xstep = (-1);						\
	(ep)->dx    = (-(ep)->dx);					\
    }									\
    else {								\
	(ep)->xstep  = 1;						\
    }									\
    if((ep)->dx > (ep)->dy) {						\
	(ep)->xmajor = TRUE;						\
	(ep)->error  = (ep)->dx >> 1;					\
    }									\
    else {								\
	(ep)->xmajor = FALSE;						\
	(ep)->error  = ((ep)->dy - ((1 - (ep)->xstep) >> 1)) >> 1;	\
    }									\
}

#define xstep_edge(ep) for( ; ; ) {					\
    (ep)->x += (ep)->xstep;						\
    if(((ep)->error -= (ep)->dy) < 0) {					\
	(ep)->error += (ep)->dx;					\
	break;								\
    }									\
}

#define ystep_edge(ep) {						\
    if(((ep)->error -= (ep)->dx) < 0) {					\
	(ep)->x	    += (ep)->xstep;					\
	(ep)->error += (ep)->dy;					\
    }									\
}

