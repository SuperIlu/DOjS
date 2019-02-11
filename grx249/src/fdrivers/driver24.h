/**
 ** driver24.h ---- the 24bpp color padded SVGA linear frame buffer driver
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
 ** Contributions by: (See "doc/credits.doc" for details)
 ** Hartmut Schirmer (hsc@techfak.uni-kiel.de)
 ** Andrzej Lawa [FidoNet: Andrzej Lawa 2:480/19.77]
 **
 **/

#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"
#include "mempeek.h"
#include "memfill.h"
#include "access24.h"

/* frame offset address calculation */
#define MULT3(x)     ( (x)+(x)+(x) )
#define FOFS(x,y,lo) umuladd32((y),(lo),MULT3(x))

/* #define FAR_ACCESS for video access routines */

#ifdef FAR_ACCESS
# define peek24(p)       peek_24_f((p))
# define poke24_xor      poke_24_f_xor
# define poke24_or       poke_24_f_or
# define poke24_and      poke_24_f_and
# define poke24_set      poke_24_f
# define repfill8_xor    repfill_b_f_xor
# define repfill8_or     repfill_b_f_or
# define repfill8_and    repfill_b_f_and
# define repfill8_set    repfill_b_f
# define repfill24_xor   repfill_24_f_xor
# define repfill24_or    repfill_24_f_or
# define repfill24_and   repfill_24_f_and
# define repfill24_set   repfill_24_f_set
# define SETFARSEL(sel)  setup_far_selector(sel)
#else /* defined FAR_ACCESS */
# define peek24(p)       peek_24((p))
# define poke24_xor      poke_24_xor
# define poke24_or       poke_24_or
# define poke24_and      poke_24_and
# define poke24_set      poke_24
# define repfill8_xor    repfill_b_xor
# define repfill8_or     repfill_b_or
# define repfill8_and    repfill_b_and
# define repfill8_set    repfill_b
# define repfill24_xor   repfill_24_xor
# define repfill24_or    repfill_24_or
# define repfill24_and   repfill_24_and
# define repfill24_set   repfill_24_set
# define SETFARSEL(sel)
#endif


static INLINE
GrColor readpixel(GrFrame *c,int x,int y)
{
	GrColor col;
	char far *p;
	GRX_ENTER();
#ifdef FAR_ACCESS
	p = &SCRN->gc_baseaddr[0][FOFS(x,y,SCRN->gc_lineoffset)];
	setup_far_selector(SCRN->gc_selector);
#else
	p = &c->gf_baseaddr[0][FOFS(x,y,c->gf_lineoffset)];
#endif
	col = peek24(p);
	GRX_RETURN(col);
}


static INLINE
void drawpixel(int x,int y,GrColor color)
{
	char far *p;
	GRX_ENTER();
	p = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
	SETFARSEL(CURC->gc_selector);
	switch(C_OPER(color)) {
	    case C_XOR: poke24_xor(p,color);  break;
	    case C_OR:  poke24_or( p,color);  break;
	    case C_AND: poke24_and(p,color);  break;
	    default:    poke24_set(p,color);  break;
	}
	GRX_LEAVE();
}


static void drawhline(int x,int y,int w,GrColor color)
{
	char far *p;
	GRX_ENTER();
	p  = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];

	w = MULT3(w);
	SETFARSEL(CURC->gc_selector);
#       ifndef GRX_HAVE_FAST_REPFILL24
	{
	  GR_int8u c0;
	  c0 = RD24BYTE(color,0);
	  if (c0 == RD24BYTE(color,1) && c0 == RD24BYTE(color,2) ) {
	     GR_repl cval = freplicate_b(c0);
	     switch(C_OPER(color)) {
	       case C_XOR: repfill8_xor(p,cval,w); break;
	       case C_OR:  repfill8_or( p,cval,w); break;
	       case C_AND: repfill8_and(p,cval,w); break;
	       default:    repfill8_set(p,cval,w); break;
	     }
	     goto done;
	   }
	}
#       endif
	switch (C_OPER(color)) {
	  case C_XOR: repfill24_xor(p,color,w); break;
	  case C_OR:  repfill24_or( p,color,w); break;
	  case C_AND: repfill24_and(p,color,w); break;
	  default:    repfill24_set(p,color,w); break;
	}
#ifndef GRX_HAVE_FAST_REPFILL24
  done:
#endif
	GRX_LEAVE();
}



static
#include "fdrivers/generic/vline.c"

static
#include "fdrivers/generic/line.c"

static
#include "fdrivers/generic/block.c"

static
#include "fdrivers/generic/bitmap.c"

static
#include "fdrivers/generic/pattern.c"

static void bitblt(GrFrame *dst,int dx,int dy,GrFrame *src,int sx,int sy,int w,int h,GrColor op)
{
	GRX_ENTER();
	if(GrColorMode(op) == GrIMAGE) _GrFrDrvGenericBitBlt(
	    dst,dx,dy,
	    src,sx,sy,
	    w,h,
	    op
	);
	else
#ifdef FAR_ACCESS
	  _GrFrDrvPackedBitBltV2V_LFB(
#else
	  _GrFrDrvPackedBitBltR2R(
#endif
	    dst,MULT3(dx),dy,
	    src,MULT3(sx),sy,
	    MULT3(w),h,
	    op
	);
	GRX_LEAVE();
}

#ifdef FAR_ACCESS

static void bltv2r(GrFrame *dst,int dx,int dy,
		   GrFrame *src,int sx,int sy,
		   int w,int h,GrColor op)
{
	GRX_ENTER();
	if(GrColorMode(op) == GrIMAGE) _GrFrDrvGenericBitBlt(
	    dst,dx,dy,
	    src,sx,sy,
	    w,h,
	    op
	);
	else _GrFrDrvPackedBitBltV2R_LFB(
	    dst,MULT3(dx),dy,
	    src,MULT3(sx),sy,
	    MULT3(w),h,
	    op
	);
	GRX_LEAVE();
}

static void bltr2v(GrFrame *dst,int dx,int dy,
		   GrFrame *src,int sx,int sy,
		   int w,int h,GrColor op)
{
	GRX_ENTER();
	if(GrColorMode(op) == GrIMAGE) _GrFrDrvGenericBitBlt(
	    dst,dx,dy,
	    src,sx,sy,
	    w,h,
	    op
	);
	else _GrFrDrvPackedBitBltR2V_LFB(
	    dst,MULT3(dx),dy,
	    src,MULT3(sx),sy,
	    MULT3(w),h,
	    op
	);
	GRX_LEAVE();
}
#endif /* FAR_ACCESS */
