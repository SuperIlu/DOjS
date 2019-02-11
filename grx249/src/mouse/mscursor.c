/**
 ** mscursor.c ---- the mouse cursor related stuff
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

#include <stdarg.h>

#include "mouse/input.h"
#include "libgrx.h"
#include "clipping.h"
#include "mempeek.h"

#define  MSCURSOR       (MOUINFO->cursor)
#define  CURSORMODE     (MOUINFO->cursmode)
#define  SPECIALMODE    (MOUINFO->cursmode != GR_M_CUR_NORMAL)
#define  BLOCKED        1
#define  ERASED         2

static void draw_special(void)
{
	int xpos  = MSCURSOR->xcord;
	int ypos  = MSCURSOR->ycord;
	int check = MOUINFO->docheck;
	GrContext csave;
	MOUINFO->docheck = FALSE;
	GrSaveContext(&csave);
	GrSetContext(SCRN);
	switch(CURSORMODE) {
	  case GR_M_CUR_RUBBER:
	    GrBox(xpos,ypos,MOUINFO->x1,MOUINFO->y1,MOUINFO->curscolor);
	    break;
	  case GR_M_CUR_LINE:
	    GrLine(xpos,ypos,MOUINFO->x1,MOUINFO->y1,MOUINFO->curscolor);
	    break;
	  case GR_M_CUR_BOX:
	    GrBox(
		(xpos + MOUINFO->x1),(ypos + MOUINFO->y1),
		(xpos + MOUINFO->x2),(ypos + MOUINFO->y2),
		MOUINFO->curscolor
	    );
	    break;
	}
	GrSetContext(&csave);
	MOUINFO->docheck = check;
}

static void move_mouse(void)
{
	if((MOUINFO->xpos != MSCURSOR->xcord) ||
	   (MOUINFO->ypos != MSCURSOR->ycord)) {
	    int check  = MOUINFO->docheck;
	    int dospec = SPECIALMODE && MSCURSOR->displayed;
	    MOUINFO->docheck = FALSE;
	    if(dospec) draw_special();
	    GrMoveCursor(MSCURSOR,MOUINFO->xpos,MOUINFO->ypos);
	    if(dospec) draw_special();
	    MOUINFO->docheck = check;
	}
}

static void draw_mouse(void)
{
	int check = MOUINFO->docheck;
	MOUINFO->docheck = FALSE;
	GrDisplayCursor(MSCURSOR);
	if(SPECIALMODE) draw_special();
	MOUINFO->docheck = check;
}

static void erase_mouse(void)
{
	int check = MOUINFO->docheck;
	MOUINFO->docheck = FALSE;
	if(SPECIALMODE) draw_special();
	GrEraseCursor(MSCURSOR);
	MOUINFO->docheck = check;
}

void GrMouseDisplayCursor(void)
{
	if(MOUINFO->msstatus  != 2)     GrMouseInit();
	if(MOUINFO->msstatus  != 2)     return;
	if(MOUINFO->cursor    == NULL)  return;
	if(MOUINFO->displayed != FALSE) return;
	move_mouse();
	draw_mouse();
	MOUINFO->displayed = TRUE;
	MOUINFO->docheck   = TRUE;
	MOUINFO->blockflag = 0;
}

void GrMouseEraseCursor(void)
{
	if(MOUINFO->msstatus  != 2)     return;
	if(MOUINFO->cursor    == NULL)  return;
	if(MOUINFO->displayed == FALSE) return;
	if(MOUINFO->blockflag != 0)     return;
	MOUINFO->displayed = FALSE;
	MOUINFO->docheck   = FALSE;
	erase_mouse();
}

void GrMouseSetCursor(GrCursor *C)
{
	if(!MOUINFO->displayed && C && (C != MSCURSOR) && COMPATIBLE(C)) {
	    GrCursor *oldcursor  = MSCURSOR;
	    if(C->displayed)       GrEraseCursor(C);
	    MOUINFO->cursor      = C;
	    if(MOUINFO->owncursor) GrDestroyCursor(oldcursor);
	    MOUINFO->owncursor   = FALSE;
	}
}

void GrMouseSetColors(GrColor fg,GrColor bg)
{
	static char ptr12x16bits[] = {
	    0,1,0,0,0,0,0,0,0,0,0,0,
	    1,2,1,0,0,0,0,0,0,0,0,0,
	    1,2,2,1,0,0,0,0,0,0,0,0,
	    1,2,2,2,1,0,0,0,0,0,0,0,
	    1,2,2,2,2,1,0,0,0,0,0,0,
	    1,2,2,2,2,2,1,0,0,0,0,0,
	    1,2,2,2,2,2,2,1,0,0,0,0,
	    1,2,2,2,2,2,2,2,1,0,0,0,
	    1,2,2,2,2,2,2,2,2,1,0,0,
	    1,2,2,2,2,2,2,2,2,2,1,0,
	    1,2,2,2,2,2,2,2,2,2,2,1,
	    1,2,2,2,2,1,1,1,1,1,1,0,
	    1,2,2,2,1,0,0,0,0,0,0,0,
	    1,2,2,1,0,0,0,0,0,0,0,0,
	    1,2,1,0,0,0,0,0,0,0,0,0,
	    0,1,0,0,0,0,0,0,0,0,0,0,
	};
	GrCursor *newc;
	GrColor cols[3];
	if(MOUINFO->displayed) return;
	cols[0] = 2;
	cols[1] = bg;
	cols[2] = fg;
	newc = GrBuildCursor(ptr12x16bits,12,12,16,1,1,cols);
	if(!newc) return;
	GrMouseSetCursor(newc);
	MOUINFO->owncursor = TRUE;
}

void GrMouseSetCursorMode(int mode,...)
{
	va_list ap;
	if(MOUINFO->displayed) return;
	va_start(ap,mode);
	switch(mode) {
	  case GR_M_CUR_BOX:
	    MOUINFO->x2        = va_arg(ap,int);
	    MOUINFO->y2        = va_arg(ap,int);
	  case GR_M_CUR_RUBBER:
	  case GR_M_CUR_LINE:
	    MOUINFO->cursmode  = mode;
	    MOUINFO->x1        = va_arg(ap,int);
	    MOUINFO->y1        = va_arg(ap,int);
	    MOUINFO->curscolor = GrXorModeColor(va_arg(ap,GrColor));
	    break;
	  default:
	    MOUINFO->cursmode  = GR_M_CUR_NORMAL;
	    break;
	}
	va_end(ap);
}

static int block(GrContext *c,int x1,int y1,int x2,int y2)
{
	int mx1,my1,mx2,my2,oldblock = MOUINFO->blockflag;
	if(!c) c = CURC;
	if(!MOUINFO->displayed) return(0);
	if(!MOUINFO->docheck)   return(0);
	if(!c->gc_onscreen)     return(0);
	if(oldblock & ERASED)   return(0);
	MOUINFO->blockflag = BLOCKED;
	isort(x1,x2); x1 += c->gc_xoffset; x2 += c->gc_xoffset;
	isort(y1,y2); y1 += c->gc_yoffset; y2 += c->gc_yoffset;
	mx1 = MSCURSOR->xwpos;
	my1 = MSCURSOR->ywpos;
	mx2 = MSCURSOR->xwork + mx1 - 1;
	my2 = MSCURSOR->ywork + my1 - 1;
	if(SPECIALMODE) {
	    int cx1,cy1,cx2,cy2;
	    switch(CURSORMODE) {
	      case GR_M_CUR_RUBBER:
	      case GR_M_CUR_LINE:
		cx1 = MSCURSOR->xcord;
		cy1 = MSCURSOR->ycord;
		cx2 = MOUINFO->x1;
		cy2 = MOUINFO->y1;
		break;
	      case GR_M_CUR_BOX:
		cx1 = MSCURSOR->xcord + MOUINFO->x1;
		cy1 = MSCURSOR->ycord + MOUINFO->y1;
		cx2 = MSCURSOR->xcord + MOUINFO->x2;
		cy2 = MSCURSOR->ycord + MOUINFO->y2;
		break;
	      default:
		return(0);
	    }
	    isort(cx1,cx2); mx1 = imin(cx1,mx1); mx2 = imax(mx2,cx2);
	    isort(cy1,cy2); my1 = imin(cy1,my1); my2 = imax(my2,cy2);
	}
	x1 = imax(x1,mx1); y1 = imax(y1,my1);
	x2 = imin(x2,mx2); y2 = imin(y2,my2);
	if((x1 <= x2) && (y1 <= y2)) {
	    MOUINFO->blockflag = oldblock | ERASED;
	    MOUINFO->docheck   = FALSE;
	    erase_mouse();
	    return(ERASED);
	}
	return((oldblock & BLOCKED) ? 0 : BLOCKED);
}

static void unblock(int flags)
{
	if(!MOUINFO->displayed) return;
	if(flags & MOUINFO->blockflag & ERASED) {
	    if(!(MOUINFO->blockflag & BLOCKED) || (flags & BLOCKED)) {
		_GrUpdateInputs();
		move_mouse();
		MOUINFO->blockflag &= ~BLOCKED;
	    }
	    draw_mouse();
	    MOUINFO->blockflag &= ~ERASED;
	    MOUINFO->docheck    = TRUE;
	    return;
	}
	if(flags & MOUINFO->blockflag & BLOCKED) {
	    if(!(MOUINFO->blockflag & ERASED)) {
		int updflag;
		test_unblock(updflag);
		if(updflag) {
		    _GrUpdateInputs();
		    move_mouse();
		}
	    }
	    MOUINFO->blockflag &= ~BLOCKED;
	}
}

void GrMouseUpdateCursor(void)
{
	if(MOUINFO->displayed && !MOUINFO->blockflag) {
	    _GrUpdateInputs();
	    move_mouse();
	}
}

void _GrInitMouseCursor(void)
{
	if(MSCURSOR && !COMPATIBLE(MSCURSOR)) {
	    if(MOUINFO->owncursor) {
		GrCursor *obsolete = MSCURSOR;
		MOUINFO->cursor    = NULL;
		MOUINFO->owncursor = FALSE;
		GrDestroyCursor(obsolete);
	    }
	    MOUINFO->cursor = NULL;
	}
	if(MSCURSOR == NULL) {
	    GrMouseSetColors(GrAllocColor(255,0,0),GrBlack());
	}
	if(MSCURSOR && MSCURSOR->displayed) {
	    GrEraseCursor(MSCURSOR);
	}
	MOUINFO->cursmode  = GR_M_CUR_NORMAL;
	MOUINFO->displayed = FALSE;
	MOUINFO->blockflag = 0;
	MOUINFO->docheck   = FALSE;
	MOUINFO->block     = block;
	MOUINFO->unblock   = unblock;
}

