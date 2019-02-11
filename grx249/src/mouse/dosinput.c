/**
 ** dosinput.c ---- polled mode mouse and keyboard interface for DOS
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

#include <stdlib.h>

#if defined(__TURBOC__) || defined(__WATCOMC__) /* GS - Watcom C++ 11.0 */
#include <conio.h>
#endif

#ifdef   __DJGPP__
#include <pc.h>
#endif

#include "libgrx.h"
#include "grxkeys.h"
#include "allocate.h"
#include "arith.h"
#include "int86.h"
#include "memcopy.h"
#include "memfill.h"
#include "mouse/input.h"

static int  kbd_enabled = TRUE;
static int  mou_enabled = TRUE;
static int  mou_buttons = 0;
static long evt_lasttime;

static void uninit(void)
{
	if(MOUINFO->msstatus > 1) MOUINFO->msstatus = 1;
}

int GrMouseDetect(void)
{
	Int86Regs r;
	if(MOUINFO->msstatus == 0) {
	    MOUINFO->msstatus = (-1);           /* assume missing */
	    sttzero(&r);
	    int33(&r);
	    if(IREG_AX(r) != 0) {
		atexit(uninit);
		MOUINFO->msstatus = 1;          /* present, but not initted */
	    }
	}
	return((MOUINFO->msstatus > 0) ? TRUE : FALSE);
}

void GrMouseInitN(int queue_size)
{
	uninit();
	queue_size = umax(4,umin(256,queue_size));
	init_queue(queue_size);
	if(GrMouseDetect()) {
	    GrMouseSetSpeed(1,1);
	    GrMouseSetAccel(100,1);
	    GrMouseSetLimits(0,0,SCRN->gc_xmax,SCRN->gc_ymax);
	    GrMouseWarp((SCRN->gc_xmax >> 1),(SCRN->gc_ymax >> 1));
	    _GrInitMouseCursor();
	    MOUINFO->msstatus = 2;
	    mou_buttons = 0;
	}
	GrMouseEventEnable(TRUE,TRUE);
	real_time(evt_lasttime);
	MOUINFO->uninit = uninit;
}

void GrMouseSetSpeed(int spmult,int spdiv)
{
	MOUINFO->spmult = umin(16,umax(1,spmult));
	MOUINFO->spdiv  = umin(16,umax(1,spdiv));
}

void GrMouseSetAccel(int thresh,int accel)
{
	MOUINFO->thresh = umin(64,umax(1,thresh));
	MOUINFO->accel  = umin(16,umax(1,accel));
}

void GrMouseSetLimits(int x1,int y1,int x2,int y2)
{
	isort(x1,x2);
	isort(y1,y2);
	MOUINFO->xmin = imax(0,imin(x1,SCRN->gc_xmax));
	MOUINFO->ymin = imax(0,imin(y1,SCRN->gc_ymax));
	MOUINFO->xmax = imax(0,imin(x2,SCRN->gc_xmax));
	MOUINFO->ymax = imax(0,imin(y2,SCRN->gc_ymax));
}

void GrMouseWarp(int x,int y)
{
	MOUINFO->xpos = imax(MOUINFO->xmin,imin(MOUINFO->xmax,x));
	MOUINFO->ypos = imax(MOUINFO->ymin,imin(MOUINFO->ymax,y));
	GrMouseUpdateCursor();
}

void GrMouseEventEnable(int enable_kb,int enable_ms)
{
	kbd_enabled = enable_kb;
	mou_enabled = enable_ms;
}

void _GrUpdateInputs(void)
{
	for( ; ; ) {
	    Int86Regs r;
	    GrMouseEvent ev;
	    int gotevt = FALSE;
	    if(mou_enabled && (MOUINFO->msstatus == 2)) {
		int mick,btn;
		sttzero(&r);
		IREG_AX(r) = 11;                /* read mickey counters */
		int33(&r);
		if((mick = (short)IREG_CX(r)) != 0) {
		    update_coord(x,mick);
		}
		if((mick = (short)IREG_DX(r)) != 0) {
		    update_coord(y,mick);
		}
		IREG_AX(r) = 3;                 /* read button state */
		int33(&r);
		btn = IREG_BX(r);
		if(btn != mou_buttons) {
		    fill_mouse_ev(
			ev,
			mou_buttons,btn,
			GR_M_LEFT,
			GR_M_MIDDLE,
			GR_M_RIGHT,
			GR_M_P4,
			GR_M_P5,
			GrKeyStat()
		    );
		    real_dtime(ev.dtime,evt_lasttime);
		    enqueue_event(ev);
		    MOUINFO->moved = FALSE;
		    mou_buttons = btn;
		    gotevt = TRUE;
		}
	    }
	    if(kbd_enabled && GrKeyPressed()) {
		fill_keybd_ev(ev,GrKeyRead(),GrKeyStat());
		real_dtime(ev.dtime,evt_lasttime);
		enqueue_event(ev);
		MOUINFO->moved = FALSE;
		gotevt = TRUE;
	    }
	    if(!gotevt) break;
	}
}

void GrMouseGetEventT(int flags,GrMouseEvent *ev,long tout)
{
	int  msdraw;
	long prevtime;
	if(MOUINFO->msstatus == 0) GrMouseInit();
	msdraw = !MOUINFO->displayed && !(flags & GR_M_NOPAINT);
	if(msdraw) GrMouseDisplayCursor();
	real_time(prevtime);
	for( ; ; ) {
	    _GrUpdateInputs();
	    GrMouseUpdateCursor();
	    while(MOUINFO->qlength > 0) {
		dequeue_event((*ev));
		if(ev->flags & flags) {
		    if(msdraw) GrMouseEraseCursor();
		    return;
		}
	    }
	    if((flags & GR_M_POLL) ||
	       (tout == 0L) ||
	       (MOUINFO->moved && (flags & GR_M_MOTION))) {
		fill_mouse_ev(
		    (*ev),
		    mou_buttons,mou_buttons,
		    GR_M_LEFT,
		    GR_M_MIDDLE,
		    GR_M_RIGHT,
		    GR_M_P4,
		    GR_M_P5,
		    GrKeyStat()
		);
		if ( ev->flags ) /* something happend */
		  real_dtime(ev->dtime,evt_lasttime);
		else
		  ev->dtime = -1; /* special time if nothing happend */
		MOUINFO->moved = FALSE;
		if(msdraw) GrMouseEraseCursor();
		return;
	    }
	    if(tout > 0L) {
		long dtime;
		real_dtime(dtime,prevtime);
		if((tout -= dtime) < 0L) tout = 0L;
	    }
	}
}

