/**
 ** patfline.c
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
 **
 **  Copyright (C) 1992, Csaba Biegl
 **    820 Stirrup Dr, Nashville, TN, 37221
 **    csaba@vuse.vanderbilt.edu
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
#include "shapes.h"

void _GrPatternFilledLine(int x1,int y1,int dx,int dy,GrPattern *p)
{
	union { GrFrame *c; unsigned char *b; } src;
	int sy,ymajor;
	int pw,ph,px,py;
	int ispixmap;
	GrColor fgc = 0, bgc = 0;
	int points,error;

	if (dx < 0) {
		x1 += dx; dx = -dx;
		y1 += dy; dy = -dy;
	}
	if(dy==0) {
/*int check_if_offsets_correct;*/
	    _GrFillPattern(x1,y1,dx+1,p);
	    return;
	}
	if(dy >= 0) {
	    sy = 1;
	}
	else {
	    dy = (-dy);
	    sy = (-1);
	}
	if((ispixmap = p->gp_ispixmap) != FALSE) {
	    pw = p->gp_pxp_width;
	    ph = p->gp_pxp_height;
	    px = x1 % pw;
	    py = y1 % ph;
	    src.c = &p->gp_pxp_source;
	}
	else {
	    pw = 8;
	    ph = p->gp_bmp_height;
	    px = x1 & 7;
	    py = y1 % ph;
	    src.b = (unsigned char *) p->gp_bmp_data;
	    fgc = p->gp_bmp_fgcolor;
	    bgc = p->gp_bmp_bgcolor;
	}
	if(dy > dx) {
	    points = dy + 1;
	    error  = dy >> 1;
	    ymajor = TRUE;
	}
	else {
	    points = dx + 1;
	    error  = dx >> 1;
	    ymajor = FALSE;
	}
	while(--points >= 0) {
	    (*CURC->gc_driver->drawpixel)(
                x1,y1,
		ispixmap ?
		    (*src.c->gf_driver->readpixel)(src.c,px,py) :
		    (src.b[py] & (0x80U >> px)) ? fgc : bgc
	    );
	    if(ymajor) {
		if((error -= dx) < 0) error += dy,x1++,px++;
		y1 += sy,py += sy;
	    }
	    else {
		if((error -= dy) < 0) error += dx,y1 += sy,py += sy;
		x1++,px++;
	    }
	    if((unsigned)py >= (unsigned)ph) {
		if(py < 0) py += ph;
		else       py -= ph;
	    }
	    if(px >= pw) px = 0;
	}
}

void GrPatternFilledLine(int x1,int y1,int x2,int y2,GrPattern *p)
{
	clip_line(CURC,x1,y1,x2,y2);
	mouse_block(CURC,x1,y1,x2,y2);
	_GrPatternFilledLine(x1+CURC->gc_xoffset,y1+CURC->gc_xoffset,x2-x1,y2-y1,p);
	mouse_unblock();
}
