/**
 ** DRAWCURS.C ---- display, erase and move graphics cursors
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

#include "mouse/input.h"
#include "libgrx.h"

#define XORMASK(c,x,y)  &(c)->work.gc_frame,(x),(y)
#define ANDMASK(c,x,y)  &(c)->work.gc_frame,((x) + ((c)->xwork >> 1)),(y)
#define SAVECXT(c,x,y)  &(c)->work.gc_frame,(x),((y) + (c)->ysize)
#define WORKCXT(c,x,y)  &(c)->work.gc_frame,(x),((y) + (c)->ysize + (c)->ywork)

void GrDisplayCursor(GrCursor *C)
{
	int xpos,ypos;
	int xwrk,ywrk;
	int xsiz,ysiz;
	if(!C || !COMPATIBLE(C) || C->displayed) return;
	C->displayed = TRUE;
	xpos = C->xcord - C->xoffs;
	ypos = C->ycord - C->yoffs;
	xsiz = C->xwork;
	ysiz = C->ywork;
	xwrk = xpos & ~7;
	ywrk = ypos & ~7;
	if(xwrk < 0) xwrk = 0;
	if(ywrk < 0) ywrk = 0;
	if(xwrk > (GrVirtualX() - xsiz)) xwrk = GrVirtualX() - xsiz;
	if(ywrk > (GrVirtualY() - ysiz)) ywrk = GrVirtualY() - ysiz;
	C->xwpos = xwrk;
	C->ywpos = ywrk;
	mouse_block(SCRN,xwrk,ywrk,(xwrk + xsiz - 1),(ywrk + ysiz - 1));
	(*SDRV->bltv2r)(
	    SAVECXT(C,0,0),
	    &SCRN->gc_frame,xwrk,ywrk,
	    xsiz,ysiz,
	    GrWRITE
	);
	(*C->work.gc_driver->bitblt)(
	    WORKCXT(C,0,0),
	    SAVECXT(C,0,0),
	    xsiz,ysiz,
	    GrWRITE
	);
	xpos -= xwrk;
	ypos -= ywrk;
	xsiz  = C->xsize;
	ysiz  = C->ysize;
	xwrk  = ywrk = 0;
	if(xpos < 0) { xwrk -= xpos; xsiz += xpos; xpos = 0; }
	if(ypos < 0) { ywrk -= ypos; ysiz += ypos; ypos = 0; }
	if(xsiz > (C->xwork - xpos)) xsiz = C->xwork - xpos;
	if(ysiz > (C->ywork - ypos)) ysiz = C->ywork - ypos;
	if((xsiz <= 0) || (ysiz <= 0)) return;
	(*C->work.gc_driver->bitblt)(
	    WORKCXT(C,xpos,ypos),
	    ANDMASK(C,xwrk,ywrk),
	    xsiz,ysiz,
	    GrAND
	);
	(*C->work.gc_driver->bitblt)(
	    WORKCXT(C,xpos,ypos),
	    XORMASK(C,xwrk,ywrk),
	    xsiz,ysiz,
	    GrXOR
	);
	(*SDRV->bltr2v)(
	    &SCRN->gc_frame,C->xwpos,C->ywpos,
	    WORKCXT(C,0,0),
	    C->xwork,C->ywork,
	    GrWRITE
	);
	mouse_unblock();
}

void GrEraseCursor(GrCursor *C)
{
	if(C && COMPATIBLE(C) && C->displayed) {
	    mouse_block(SCRN,
		C->xwpos,                 C->ywpos,
	       (C->xwpos + C->xwork - 1),(C->ywpos + C->ywork - 1)
	    );
	    (*SDRV->bltr2v)(
		&SCRN->gc_frame,C->xwpos,C->ywpos,
		SAVECXT(C,0,0),
		C->xwork,C->ywork,
		GrWRITE
	    );
	    C->displayed = FALSE;
	    mouse_unblock();
	}
}

void GrMoveCursor(GrCursor *C,int x,int y)
{
	int xpos,ypos;
	int xsiz,ysiz;
	if(!C || ((C->xcord == x) && (C->ycord == y))) return;
	C->xcord = x;
	C->ycord = y;
	if(!C->displayed || !COMPATIBLE(C)) return;
	xpos = x - C->xoffs - C->xwpos;
	ypos = y - C->yoffs - C->ywpos;
	xsiz = C->xsize;
	ysiz = C->ysize;
	if((xpos >= 0) && ((xpos + xsiz) <= C->xwork) &&
	   (ypos >= 0) && ((ypos + ysiz) <= C->ywork)) {
	    mouse_block(SCRN,
		C->xwpos,                 C->ywpos,
	       (C->xwpos + C->xwork - 1),(C->ywpos + C->ywork - 1)
	    );
	    (*C->work.gc_driver->bitblt)(
		WORKCXT(C,0,0),
		SAVECXT(C,0,0),
		C->xwork,C->ywork,
		GrWRITE
	    );
	    (*C->work.gc_driver->bitblt)(
		WORKCXT(C,xpos,ypos),
		ANDMASK(C,0,0),
		xsiz,ysiz,
		GrAND
	    );
	    (*C->work.gc_driver->bitblt)(
		WORKCXT(C,xpos,ypos),
		XORMASK(C,0,0),
		xsiz,ysiz,
		GrXOR
	    );
	    (*SDRV->bltr2v)(
		&SCRN->gc_frame,C->xwpos,C->ywpos,
		WORKCXT(C,0,0),
		C->xwork,C->ywork,
		GrWRITE
	    );
	    mouse_unblock();
	    return;
	}
	GrEraseCursor(C);
	GrDisplayCursor(C);
}

