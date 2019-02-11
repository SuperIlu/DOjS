/**
 ** ram3x8.c ---- the 16M color planar (image) system RAM frame driver
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu].
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
#include "access24.h"
#include "arith.h"
#include "docolor.h"
#include "mempeek.h"
#include "memcopy.h"
#include "memfill.h"

/* frame offset address calculation */
#define FOFS(x,y,lo) umuladd32((y),(lo),(x))


/* -------------------------------------------------------------------- */

static INLINE
GrColor readpixel(GrFrame *c,int x,int y)
{
	GR_int32u offs;
	GrColor pix;
	GRX_ENTER();
	offs = FOFS(x,y,c->gf_lineoffset);
	pix = 0;
	WR24BYTE(pix,0,peek_b(&c->gf_baseaddr[0][offs]));
	WR24BYTE(pix,1,peek_b(&c->gf_baseaddr[1][offs]));
	WR24BYTE(pix,2,peek_b(&c->gf_baseaddr[2][offs]));
	GRX_RETURN(pix);
}

/* -------------------------------------------------------------------- */

static INLINE
void drawpixel(int x,int y,GrColor color)
{
	GR_int32u offs;
	char far *p0,*p1,*p2;
	GRX_ENTER();
	offs = FOFS(x,y,CURC->gc_lineoffset);
	p0 = &CURC->gc_baseaddr[0][offs];
	p1 = &CURC->gc_baseaddr[1][offs];
	p2 = &CURC->gc_baseaddr[2][offs];
	switch(C_OPER(color)) {
	  case C_XOR: poke_b_xor(p0,RD24BYTE(color,0));
		      poke_b_xor(p1,RD24BYTE(color,1));
		      poke_b_xor(p2,RD24BYTE(color,2));
		      break;
	  case C_OR:  poke_b_or(p0,RD24BYTE(color,0));
		      poke_b_or(p1,RD24BYTE(color,1));
		      poke_b_or(p2,RD24BYTE(color,2));
		      break;
	  case C_AND: poke_b_and(p0,RD24BYTE(color,0));
		      poke_b_and(p1,RD24BYTE(color,1));
		      poke_b_and(p2,RD24BYTE(color,2));
		      break;
	  default:    poke_b(p0,RD24BYTE(color,0));
		      poke_b(p1,RD24BYTE(color,1));
		      poke_b(p2,RD24BYTE(color,2));
		      break;
	}
	GRX_LEAVE();
}

/* -------------------------------------------------------------------- */

static void drawhline(int x,int y,int w,GrColor color) {
    int copr, pl;
    GR_int32u offs;
    GRX_ENTER();
    copr = C_OPER(color);
    offs = FOFS(x,y,CURC->gc_lineoffset);
    for (pl=0; pl < 3; ++pl) {
      if(DOCOLOR8(color,copr)) {
	GR_repl cval = freplicate_b(color);
	char far *pp = &CURC->gc_baseaddr[pl][offs];
	int ww = w;
	switch(copr) {
	    case C_XOR: repfill_b_xor(pp,cval,ww); break;
	    case C_OR:  repfill_b_or( pp,cval,ww); break;
	    case C_AND: repfill_b_and(pp,cval,ww); break;
	    default:    repfill_b(    pp,cval,ww); break;
	}
      }
      color >>= 8;
    }
    GRX_LEAVE();
}


/* -------------------------------------------------------------------- */

static void drawvline(int x,int y,int h,GrColor color)
{
    int copr, pl;
    GR_int32u offs;
    unsigned lwdt;
    GRX_ENTER();
    copr = C_OPER(color);
    lwdt = CURC->gc_lineoffset;
    offs = FOFS(x,y,lwdt);
    for (pl=0; pl < 3; ++pl) {
      if(DOCOLOR8(color,copr)) {
	char far *pp = &CURC->gc_baseaddr[pl][offs];
	int hh = h;
	switch(copr) {
	    case C_XOR: colfill_b_xor(pp,lwdt,(GR_int8u)color,hh); break;
	    case C_OR:  colfill_b_or( pp,lwdt,(GR_int8u)color,hh); break;
	    case C_AND: colfill_b_and(pp,lwdt,(GR_int8u)color,hh); break;
	    default:    colfill_b(    pp,lwdt,(GR_int8u)color,hh); break;
	}
      }
      color >>= 8;
    }
    GRX_LEAVE();
}

/* -------------------------------------------------------------------- */

static
#include "fdrivers/generic/block.c"

static
#include "fdrivers/generic/line.c"

static
#include "fdrivers/generic/bitmap.c"

static
#include "fdrivers/generic/pattern.c"

/* -------------------------------------------------------------------- */

static
#include "fdrivers/generic/bitblt.c"

/* in pblitr2r.c if GRX_USE_RAM3x8 defined */
extern void _GrFrDrvPackedBitBltR2Rpl(GrFrame *dst,int dx,int dy,
				      GrFrame *src,int x,int y,
				      int w,int h,GrColor op,int plane);

static void bitblit(GrFrame *dst,int dx,int dy,
		    GrFrame *src,int sx,int sy,
		    int w,int h,GrColor op)
{
    int pl;
    GRX_ENTER();
    if(GrColorMode(op) == GrIMAGE)
	bitblt(dst,dx,dy,src,sx,sy,w,h,op);
    else
	for (pl=0; pl < 3; ++pl)
	    _GrFrDrvPackedBitBltR2Rpl(dst,dx,dy,src,sx,sy,w,h,op,pl);
    GRX_LEAVE();
}

/* -------------------------------------------------------------------- */

GrFrameDriver _GrFrameDriverRAM3x8 = {
    GR_frameRAM3x8,             /* frame mode */
    GR_frameUndef,              /* compatible RAM frame mode */
    FALSE,                      /* onscreen */
    4,                          /* scan line width alignment */
    3,                          /* number of planes */
    24,                         /* bits per pixel */
#ifdef __TURBOC__
    65520L,                     /* max plane size the code can handle */
#else
    8*16*1024L*1024L,           /* max plane size the code can handle */
#endif
    NULL,
    readpixel,
    drawpixel,
    drawline,
    drawhline,
    drawvline,
    drawblock,
    drawbitmap,
    drawpattern,
    bitblit,
    NULL,
    NULL,
    _GrFrDrvGenericGetIndexedScanline,
    _GrFrDrvGenericPutScanline
};
