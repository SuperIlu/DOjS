/**
 ** svga24.c ---- the 16M color Super VGA frame driver
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
#include "access24.h"

#if BYTE_ORDER!=HARDWARE_BYTE_ORDER
#error Mismatching byte order between ram and video ram !
#endif

/* helper ... */
#define MULT3(x)     ( (x)+(x)+(x) )
#ifdef __TURBOC__
#define REMAIN3(x)   ( ((unsigned int)(x)) % 3 )
#else
#define REMAIN3(x)   ( (x) % 3 )
#endif

/* frame offset address calculation */
#define FOFS(x,y,lo) umuladd32((y),(lo),MULT3(x))


static INLINE
GrColor readpixel(GrFrame *c,int x,int y)
{
	GrColor cval;
	GR_int32u offs;
	int  bank;
	char far *p;
	GRX_ENTER();
	cval = (GrColor)0;
	offs = FOFS(x,y,SCRN->gc_lineoffset);
	bank = BANKNUM(offs);
	p    = &SCRN->gc_baseaddr[0][BANKPOS(offs)];
	setup_far_selector(SCRN->gc_selector);
	CHKBANK(bank);
	switch (BANKLFT(offs)) {
	  case 1 : cval = peek_b_f(p);
		   ++bank;
		   SETBANK(bank);
		   p = SCRN->gc_baseaddr[0];
		   cval |= ((GR_int32u)peek_w_f(p)) << 8;
		   break;
	  case 2 : cval = peek_w_f(p);
		   ++bank;
		   SETBANK(bank);
		   p = SCRN->gc_baseaddr[0];
		   WR24BYTE(cval,2,peek_b_f(p));
		   break;
#ifdef PEEK_24_F_READS_ONE_MORE
	  case 3 : cval = peek_w_f(p);
		   WR24BYTE(cval,2,peek_b_f(p+2));
		   break;
#endif
	  default: cval = peek_24_f(p);
		   break;
	}
	GRX_RETURN(cval);
}

static INLINE
void drawpixel(int x,int y,GrColor color)
{
	GR_int32u offs;
	int bank;
	char far *p;
	GRX_ENTER();
	offs = FOFS(x,y,CURC->gc_lineoffset);
	p    = &CURC->gc_baseaddr[0][BANKPOS(offs)];
	setup_far_selector(CURC->gc_selector);
	bank = BANKNUM(offs);
	CHKBANK(bank);
	switch(C_OPER(color)) {
	  case C_XOR:
	      switch (BANKLFT(offs)) {
		case 1:
		  poke_b_f_xor(p,(GR_int8u)color);
		  SETBANK(++bank);
		  p = SCRN->gc_baseaddr[0];
		  poke_b_f_xor(p,RD24BYTE(color,1));
		  poke_b_f_xor(p+1,RD24BYTE(color,2));
		  break;
		case 2:
		  poke_w_f_xor(p,(GR_int16u)color);
		  SETBANK(++bank);
		  p = SCRN->gc_baseaddr[0];
		  poke_b_f_xor(p,RD24BYTE(color,2));
		  break;
		default:
		  poke_24_f_xor(p,color);
		  break;
	      }
	      break;
	  case C_OR:
	      switch (BANKLFT(offs)) {
		case 1:
		  poke_b_f_or(p,(GR_int8u)color);
		  SETBANK(++bank);
		  p = SCRN->gc_baseaddr[0];
		  poke_b_f_or(p,RD24BYTE(color,1));
		  poke_b_f_or(p+1,RD24BYTE(color,2));
		  break;
		case 2:
		  poke_w_f_or(p,(GR_int16u)color);
		  SETBANK(++bank);
		  p = SCRN->gc_baseaddr[0];
		  poke_b_f_or(p,RD24BYTE(color,2));
		  break;
		default:
		  poke_24_f_or(p,color);
		  break;
	      }
	      break;
	  case C_AND:
	      switch (BANKLFT(offs)) {
		case 1:
		  poke_b_f_and(p,(GR_int8u)color);
		  SETBANK(++bank);
		  p = SCRN->gc_baseaddr[0];
		  poke_b_f_and(p,RD24BYTE(color,1));
		  poke_b_f_and(p+1,RD24BYTE(color,2));
		  break;
		case 2:
		  poke_w_f_and(p,(GR_int16u)color);
		  SETBANK(++bank);
		  p = SCRN->gc_baseaddr[0];
		  poke_b_f_and(p,RD24BYTE(color,2));
		  break;
		default:
		  poke_24_f_and(p,color);
		  break;
	      }
	      break;
	  default:
	      switch (BANKLFT(offs)) {
		case 1:
		  poke_b_f(p,(GR_int8u)color);
		  SETBANK(++bank);
		  p = SCRN->gc_baseaddr[0];
		  poke_b_f(p,RD24BYTE(color,1));
		  poke_b_f(p+1,RD24BYTE(color,2));
		  break;
		case 2:
		  poke_w_f(p,(GR_int16u)color);
		  SETBANK(++bank);
		  p = SCRN->gc_baseaddr[0];
		  poke_b_f(p,RD24BYTE(color,2));
		  break;
		default:
		  poke_24_f(p,color);
		  break;
	      }
	      break;
	}
	GRX_LEAVE();
}


static void drawhline(int x,int y,int w,GrColor color)
{
      char far *p;
      int op, bank;
      unsigned int  w1, w2;
      GR_int32u offs;
      GR_int8u c2;

      GRX_ENTER();
      op   =  C_OPER(color);
      offs = FOFS(x,y,CURC->gc_lineoffset);
      p    = &CURC->gc_baseaddr[0][BANKPOS(offs)];
      setup_far_selector(CURC->gc_selector);

      c2 = RD24BYTE(color,2);
      bank = BANKNUM(offs);
      CHKBANK(bank);
      w = MULT3(w);
#     ifndef GRX_HAVE_FAST_REPFILL24
	if (c2 == RD24BYTE(color,0) && c2 == RD24BYTE(color,1) ) {
	   GR_repl cval = freplicate_b(c2);
	   w1 = BANKLFT(offs);
	   w2 = w - (w1 = umin(w,w1));
	   for (;;) {
	     switch(op) {
		 case C_XOR: repfill_b_f_xor(p,cval,w1); break;
		 case C_OR:  repfill_b_f_or( p,cval,w1); break;
		 case C_AND: repfill_b_f_and(p,cval,w1); break;
		 default:    repfill_b_f(    p,cval,w1); break;
	     }
	     if (!w2) break;
	     w1 = w2;
	     w2 = 0;
	     ++bank;
	     SETBANK(bank);
	     p = CURC->gc_baseaddr[0];
	   }
	   goto done;
	}
#     endif
      for (;;) {
	w1 = BANKLFT(offs);
	w1 = umin(w,w1);
	w -= w1;
	if (w1 <= 3) {
	  /* make sure we don't run in problems on first pixel */
	  w2 = w1;
	  switch (w1) {
	    case 1 : switch (op) {
		       case C_XOR: poke_b_f_xor(p,color); break;
		       case C_OR:  poke_b_f_or( p,color); break;
		       case C_AND: poke_b_f_and(p,color); break;
		       default:    poke_b_f_set(p,color); break;
		     }
		     break;
	    case 2 : switch (op) {
		       case C_XOR: poke_w_f_xor(p,color); break;
		       case C_OR:  poke_w_f_or( p,color); break;
		       case C_AND: poke_w_f_and(p,color); break;
		       default:    poke_w_f_set(p,color); break;
		     }
		     break;
	    case 3 : switch (op) {
		       case C_XOR: poke_24_f_xor(p,color); break;
		       case C_OR:  poke_24_f_or( p,color); break;
		       case C_AND: poke_24_f_and(p,color); break;
		       default:    poke_24_f_set(p,color); break;
		     }
		     goto next_bank;
	  }
	  goto complete;
	}
	if (w) w2 = REMAIN3(w1);
	else   w2 = 0;
	switch (op) {
	  case C_XOR: repfill_24_f_xor(p,color,w1); break;
	  case C_OR:  repfill_24_f_or( p,color,w1); break;
	  case C_AND: repfill_24_f_and(p,color,w1); break;
	  default:    repfill_24_f_set(p,color,w1); break;
	}
	if (w2) {
	  /* complete pixel on next bank */
  complete:
	  bank++;
	  SETBANK(bank);
	  p = CURC->gc_baseaddr[0];
	  switch (w2) {
	    case 1 : offs = 2;
		     switch (op) {
		       case C_XOR: poke_w_f_xor(p,color>>8); break;
		       case C_OR:  poke_w_f_or( p,color>>8); break;
		       case C_AND: poke_w_f_and(p,color>>8); break;
		       default:    poke_w_f_set(p,color>>8); break;
		     }
		     if ( !(w-=2) ) goto done;
		     p += 2;
		     break;
	    case 2 : offs = 1;
		     switch (op) {
		       case C_XOR: poke_b_f_xor(p,c2); break;
		       case C_OR:  poke_b_f_or( p,c2); break;
		       case C_AND: poke_b_f_and(p,c2); break;
		       default:    poke_b_f_set(p,c2); break;
		     }
		     if ( !(--w) ) goto done;
		     ++p;
		     break;
	  }
	  continue;
	}
  next_bank:
	if (!w) goto done;
	bank++;
	SETBANK(bank);
	p = CURC->gc_baseaddr[0];
	offs = 0;
      }
  done:
      GRX_LEAVE();
}

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
	    dst,MULT3(dx),dy,
	    src,MULT3(sx),sy,
	    MULT3(w),h,
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
	    dst,MULT3(dx),dy,
	    src,MULT3(sx),sy,
	    MULT3(w),h,
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
	    dst,MULT3(dx),dy,
	    src,MULT3(sx),sy,
	    MULT3(w),h,
	    op
	);
	GRX_LEAVE();
}

#endif /* !GRX_USE_RAM3x8 */

GrFrameDriver _GrFrameDriverSVGA24 = {
    GR_frameSVGA24,             /* frame mode */
#ifdef GRX_USE_RAM3x8
    GR_frameRAM3x8,             /* compatible RAM frame mode */
#else
    GR_frameRAM24,              /* compatible RAM frame mode */
#endif
    TRUE,                       /* onscreen */
    4,                          /* line width alignment */
    1,                          /* number of planes */
    24,                         /* bits per pixel */
    24*16*1024L*1024L,          /* max plane size the code can handle */
    NULL,
    readpixel,
    drawpixel,
    drawline,
    drawhline,
    drawvline,
    drawblock,
    drawbitmap,
    drawpattern,
    bitblt,
#ifdef GRX_USE_RAM3x8
    _GrFrDrvGenericBitBlt,
    _GrFrDrvGenericBitBlt,
#else
    bltv2r,
    bltr2v,
#endif
    _GrFrDrvGenericGetIndexedScanline,
    _GrFrDrvGenericPutScanline
};
