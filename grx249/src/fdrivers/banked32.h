/**
 ** banked32.h ---- the 16M color padded SVGA banked frame buffer driver
 **                 standard routines for 32H and 32L support
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
 ** 
 **/

#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"
#include "mempeek.h"
#include "memfill.h"

/* frame offset address calculation */
#define FOFS(x,y,lo) umuladd32((y),(lo),((x)<<2))

static INLINE
GrColor readpixel(GrFrame *c,int x,int y)
{
	GR_int32u offs;
	char far *pp;
	GRX_ENTER();
	offs = FOFS(x,y,SCRN->gc_lineoffset);
	CHKBANK(BANKNUM(offs));
	pp = &SCRN->gc_baseaddr[0][BANKPOS(offs)];
	setup_far_selector(SCRN->gc_selector);
	GRX_RETURN(PIX2COL(peek_l_f(pp)));
}

static INLINE
void drawpixel(int x,int y,GrColor color)
{
	GR_int32u offs;
	char far *ptr;
	int op;
	GRX_ENTER();
	offs = FOFS(x,y,SCRN->gc_lineoffset);
	CHKBANK(BANKNUM(offs));
	ptr = &SCRN->gc_baseaddr[0][BANKPOS(offs)];
	op = C_OPER(color);
	color = COL2PIX(color);
	setup_far_selector(CURC->gc_selector);
	switch(op) {
	    case C_XOR: poke_l_f_xor(ptr,color); break;
	    case C_OR:  poke_l_f_or( ptr,color); break;
	    case C_AND: poke_l_f_and(ptr,color); break;
	    default:    poke_l_f(    ptr,color); break;
	}
	GRX_LEAVE();
}

#ifdef freplicate_l
static void drawhline(int x,int y,int w,GrColor color)
{
	GR_int32u offs;
	GR_repl cval;
	int oper;
	unsigned int w1, w2;
	GRX_ENTER();
	offs = FOFS(x,y,SCRN->gc_lineoffset);
	w2 = BANKLFT(offs) >> 2;
	w2 = w - (w1 = umin(w,w2));
	oper = C_OPER(color);
	color = COL2PIX(color);
	cval = freplicate_l(color);
	setup_far_selector(CURC->gc_selector);
	do {
	    char far *pp = &CURC->gc_baseaddr[0][BANKPOS(offs)];
	    CHKBANK(BANKNUM(offs));
	    offs += (w1 << 2);
	    switch(oper) {
		case C_XOR: repfill_l_f_xor(pp,cval,w1); break;
		case C_OR:  repfill_l_f_or( pp,cval,w1); break;
		case C_AND: repfill_l_f_and(pp,cval,w1); break;
		default:    repfill_l_f(    pp,cval,w1); break;
	    }
	    w1 = w2;
	    w2 = 0;
	} while(w1 != 0);
	GRX_LEAVE();
}
#else
static
#include "fdrivers/generic/hline.c"
#endif

static
#include "fdrivers/generic/vline.c"

static
#include "fdrivers/generic/block.c"

static
#include "fdrivers/generic/line.c"

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
	else _GrFrDrvPackedBitBltV2V(
	    dst,(dx << 2),dy,
	    src,(sx << 2),sy,
	    (w << 2),h,
	    op
	);
	GRX_LEAVE();
}

#ifndef GRX_USE_RAM3x8

static void bltv2r(GrFrame *dst,int dx,int dy,GrFrame *src,int sx,int sy,int w,int h,GrColor op)
{
	GRX_ENTER();
	if(GrColorMode(op) == GrIMAGE) _GrFrDrvGenericBitBlt(
	    dst,dx,dy,
	    src,sx,sy,
	    w,h,
	    op
	);
	else _GrFrDrvPackedBitBltV2R(
	    dst,(dx << 2),dy,
	    src,(sx << 2),sy,
	    (w << 2),h,
	    op
	);
	GRX_LEAVE();
}

static void bltr2v(GrFrame *dst,int dx,int dy,GrFrame *src,int sx,int sy,int w,int h,GrColor op)
{
	GRX_ENTER();
	if(GrColorMode(op) == GrIMAGE) _GrFrDrvGenericBitBlt(
	    dst,dx,dy,
	    src,sx,sy,
	    w,h,
	    op
	);
	else _GrFrDrvPackedBitBltR2V(
	    dst,(dx << 2),dy,
	    src,(sx << 2),sy,
	    (w << 2),h,
	    op
	);
	GRX_LEAVE();
}

#endif /* !GRX_USE_RAM3x8 */
