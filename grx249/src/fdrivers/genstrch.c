/**
 ** genstrch.c ---- generic (and slow) stretching bitblt routine
 **
 ** Copyright (c) 1998 Hartmut Schirmer
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
#include "grdriver.h"
#include "allocate.h"
#include "arith.h"
#include "mempeek.h"
#include "memcopy.h"

/* ----------------------------- generic Bresenham line code for stretching */

typedef struct {
    int x,  y;
    int dx, dy;
    int cnt;
    int err;
} _GR_lineData;

static INLINE int XLineInit(_GR_lineData *ld, int x,int y,int dx,int dy)
{
	GRX_ENTER();
	ld->x = x;
	ld->y = y;
	ld->dx = dx;
	ld->dy = dy;
	ld->cnt = dx;
	if (dx <= dy) ld->err = dx >> 1;
	else          ld->err = dx-dy;
	GRX_RETURN(ld->cnt > 0);
}

#define XLineStep(ldp) do {                \
	(ldp)->err -= (ldp)->dy;           \
	while((ldp)->err < 0) {            \
	    (ldp)->err += (ldp)->dx;       \
	    (ldp)->y   += 1;               \
	}                                  \
	(ldp)->x++;                        \
	--((ldp)->cnt);                    \
} while(0)

#define XLineCheckDone(ldp) ((ldp)->cnt <= 0)

/* ---------------------------------------------------- x- and y-dir stretch */
static void stretch(GrFrame *dst,int dx,int dy,int dw, int dh,
		    GrFrame *src,int sx,int sy,int sw, int sh,
		    GrColor op)
{
    int maxi;
    GRX_ENTER();
    setup_ALLOC();
    do {
	GrFrame csave;
	GrColor far *pixels = NULL;
	_GR_lineData lne;
	_GR_getIndexedScanline getscl = src->gf_driver->getindexedscanline;
	_GR_putScanline        putscl = dst->gf_driver->putscanline;
	int rd_y = -1;
	int *xsrc = ALLOC(sizeof(int) * dw);
	if (!xsrc) break;
	/* set up xsrc[0..dw-1] = (line 0,sx to dw-1,sx+sw-1).y */
	if(!XLineInit(&lne,0,sx,dw,sw)) {
	  FREE(xsrc);
	  break;
	}
	DBGPRINTF(DBG_DRIVER,("dw=%d, sw=%d\n",dw,sw));
	maxi = sx+sw-1;
	do {
	  /* we need to check for upper bound here         */
	  /* in rare cases the last element could overflow */
	  xsrc[lne.x] = min(lne.y,maxi);
	  DBGPRINTF(DBG_DRIVER,("xsrc[%d] = %d\n",lne.x,xsrc[lne.x]));
	  XLineStep(&lne);
	} while (!XLineCheckDone(&lne));

	if(!XLineInit(&lne,dy,sy,dh,sh)) {
	  FREE(xsrc);
	  break;
	}
	maxi = sy+sh-1;
	sttcopy(&csave,&CURC->gc_frame);
	sttcopy(&CURC->gc_frame,dst);
	do {
	  int y = min(lne.y,maxi);
	  if (!pixels || y != rd_y)
	    pixels = getscl(src,sx,(rd_y=y),dw,xsrc);
	  if (pixels)
	    putscl(dx,lne.x,dw,pixels,op);
	  XLineStep(&lne);
	} while (!XLineCheckDone(&lne));
	sttcopy(&CURC->gc_frame,&csave);
	FREE(xsrc);
    } while (0);
    reset_ALLOC();
    GRX_LEAVE();
}

/* -----------------------------------------------------general stretch blit */

void _GrFrDrvGenericStretchBlt(GrFrame *dst,int dx,int dy,int dw,int dh,
			       GrFrame *src,int sx,int sy,int sw,int sh,
			       GrColor op)
{
  GRX_ENTER();
  if (sw > 0 && dw > 0 && sh > 0 && dh > 0) {
    if (sw == dw) {
      _GR_blitFunc blit;
      if (dst->gf_onscreen) {
	if(src->gf_onscreen) blit = dst->gf_driver->bitblt;
			else blit = dst->gf_driver->bltr2v;
      } else {
	if(src->gf_onscreen) blit = dst->gf_driver->bltv2r;
			else blit = dst->gf_driver->bitblt;
      }

      if (sh == dh) {
	  /* no stretching required */
	  (*blit)(dst,dx,dy,src,sx,sy,sw,sh,op);
      } else {
	  /* can blit line by line */
	  _GR_lineData yline;
	  if (XLineInit(&yline,dy,sy,dh,sh)) {
	    do {
	      (*blit)(dst,dx,yline.x,src,sx,yline.y,sw,1,op);
	      XLineStep(&yline);
	    } while (!XLineCheckDone(&yline));
	  }
      }
    } else
	stretch(dst,dx,dy,dw,dh,
		src,sx,sy,sw,sh,
		op);
  }
  GRX_LEAVE();
}
