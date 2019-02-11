/**
 ** ram4.c ---- the 16 color RAM frame driver
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
#include "arith.h"
#include "memcopy.h"
#include "memfill.h"

#include "fdrivers/rblit_14.h"

/* frame offset address calculation */
#define FOFS(x,y,lo) ( umul32((y),(int)(lo)) + ((x)>>3) )


static INLINE
GrColor readpixel(GrFrame *c,int x,int y)
{
	GR_int32u offs;
	unsigned mask;
	GrColor pix;
	GRX_ENTER();
	offs = FOFS(x,y,c->gf_lineoffset);
	mask = 0x80 >> (x &= 7);
	pix = (GrColor)(
	    (((c->gf_baseaddr[0][offs] & mask)     ) |
	     ((c->gf_baseaddr[1][offs] & mask) << 1) |
	     ((c->gf_baseaddr[2][offs] & mask) << 2) |
	     ((c->gf_baseaddr[3][offs] & mask) << 3)
	    ) >> (7 - x)
	);
	GRX_RETURN(pix);
}

static INLINE
void drawpixel(int x,int y,GrColor color)
{
	GR_int32u offs;
	unsigned int mask;
	unsigned int cval;
	int  op;
	GRX_ENTER();
	offs = FOFS(x,y,CURC->gc_lineoffset);
	mask = 0x80 >> (x &= 7);
	cval = (unsigned int)color << (7 - x);
        switch(op = C_OPER(color) & 3) {
	  case C_WRITE:
	  case C_AND:
	    CURC->gc_baseaddr[0][offs] &= ((cval     ) | ~mask);
	    CURC->gc_baseaddr[1][offs] &= ((cval >> 1) | ~mask);
	    CURC->gc_baseaddr[2][offs] &= ((cval >> 2) | ~mask);
	    CURC->gc_baseaddr[3][offs] &= ((cval >> 3) | ~mask);
	    if(op != C_WRITE) break;
	  case C_OR:
	    CURC->gc_baseaddr[0][offs] |= ((cval     ) & mask);
	    CURC->gc_baseaddr[1][offs] |= ((cval >> 1) & mask);
	    CURC->gc_baseaddr[2][offs] |= ((cval >> 2) & mask);
	    CURC->gc_baseaddr[3][offs] |= ((cval >> 3) & mask);
	    break;
	  default:
	    CURC->gc_baseaddr[0][offs] ^= ((cval     ) & mask);
	    CURC->gc_baseaddr[1][offs] ^= ((cval >> 1) & mask);
	    CURC->gc_baseaddr[2][offs] ^= ((cval >> 2) & mask);
	    CURC->gc_baseaddr[3][offs] ^= ((cval >> 3) & mask);
	    break;
	}
	GRX_LEAVE();
}

#define maskoper(d,op,s,msk) do {                        \
  unsigned char _c_ = peek_b(d);                         \
  poke_b((d),(_c_ & ~(msk)) | ((_c_ op (s)) & (msk)) );  \
} while (0)
#define maskset(d,c,msk) \
    poke_b((d),(peek_b(d) & ~(msk)) | ((c) & (msk)))

static void drawhline(int x,int y,int w,GrColor color) {
  int oper, cval, wd, pl;
  GR_int32u DO;
  GR_int8u  lm, rm;
  GRX_ENTER();
  oper = C_OPER(color);
  cval = (int)color;
  DO   = FOFS(x,y,CURC->gc_lineoffset);
  lm   = 0xff >> (x & 7);
  rm   = 0xff << ((-(w + x)) & 7);
  wd   = ((x+w+7) >> 3) - (x >> 3);
  if (wd == 1) lm &= rm;
  for (pl = 0; pl < 4; ++pl) {
    GR_int32u cv = cval & 1 ? ~0L : 0L;
    cval >>= 1;
    if (!( !cv && (oper==C_OR||oper==C_XOR)) && !(cv && oper==C_AND) ) {
      GR_int8u far *dptr = (GR_int8u far *)&CURC->gc_baseaddr[pl][DO];
      int ww = wd;
      if ( ((GR_int8u)(~lm)) ) {
	  switch(oper) {
	    case C_XOR: maskoper(dptr,^,(GR_int8u)cv,lm); break;
	    case C_OR:  maskoper(dptr,|,(GR_int8u)cv,lm); break;
	    case C_AND: maskoper(dptr,&,(GR_int8u)cv,lm); break;
	    default:    maskset(dptr,(GR_int8u)cv,lm);    break;
	  }
	  if (!(--ww)) continue;
	  ++dptr;
      }
      if ( ((GR_int8u)(~rm)) ) --ww;
      if (ww) {
	  switch(oper) {
	    case C_XOR: repfill_b_xor(dptr,cv,ww); break;
	    case C_OR:  repfill_b_or( dptr,cv,ww); break;
	    case C_AND: repfill_b_and(dptr,cv,ww); break;
	    default:    repfill_b(    dptr,cv,ww); break;
	  }
      }
      if ( ((GR_int8u)(~rm)) ) {
	  switch(oper) {
	    case C_XOR: maskoper(dptr,^,(GR_int8u)cv,rm); break;
	    case C_OR:  maskoper(dptr,|,(GR_int8u)cv,rm); break;
	    case C_AND: maskoper(dptr,&,(GR_int8u)cv,rm); break;
	    default:    maskset(dptr,(GR_int8u)cv,rm);    break;
	  }
      }
    }
  }
  GRX_LEAVE();
}
#undef maskoper

static void drawvline(int x,int y,int h,GrColor color)
{
	unsigned int lwdt, mask, oper, hh;
	char far *p;
	GR_int32u offs;
	GRX_ENTER();
	oper = C_OPER(color);
	lwdt = CURC->gc_lineoffset;
	offs = FOFS(x,y,lwdt);
	mask = 0x80 >> (x & 7);
	switch (oper) {
	  case C_XOR:
	      /* no need to xor anything with 0 */
	      if (color&1) {
		p = &CURC->gc_baseaddr[0][offs];
		hh = h;
		colfill_b_xor(p,lwdt,mask,hh);
	      }
	      if (color&2) {
		p = &CURC->gc_baseaddr[1][offs];
		hh = h;
		colfill_b_xor(p,lwdt,mask,hh);
	      }
	      if (color&4) {
		p = &CURC->gc_baseaddr[2][offs];
		hh = h;
		colfill_b_xor(p,lwdt,mask,hh);
	      }
	      if (color&8) {
		p = &CURC->gc_baseaddr[3][offs];
		colfill_b_xor(p,lwdt,mask,h);
	      }
	      break;
	  case C_OR:
	      /* no need to or anything with 0 */
	      if (color&1) {
		p = &CURC->gc_baseaddr[0][offs];
		hh = h;
		colfill_b_or(p,lwdt,mask,hh);
	      }
	      if (color&2) {
		p = &CURC->gc_baseaddr[1][offs];
		hh = h;
		colfill_b_or(p,lwdt,mask,hh);
	      }
	      if (color&4) {
		p = &CURC->gc_baseaddr[2][offs];
		hh = h;
		colfill_b_or(p,lwdt,mask,hh);
	      }
	      if (color&8) {
		p = &CURC->gc_baseaddr[3][offs];
		colfill_b_or(p,lwdt,mask,h);
	      }
	      break;
	  case C_AND:
	      /* no need to and anything with 1 */
	      mask = ~mask; /* set up reset mask */
	      if (!(color&1)) {
		p = &CURC->gc_baseaddr[0][offs];
		hh = h;
		colfill_b_and(p,lwdt,mask,hh);
	      }
	      if (!(color&2)) {
		p = &CURC->gc_baseaddr[1][offs];
		hh = h;
		colfill_b_and(p,lwdt,mask,hh);
	      }
	      if (!(color&4)) {
		p = &CURC->gc_baseaddr[2][offs];
		hh = h;
		colfill_b_and(p,lwdt,mask,hh);
	      }
	      if (!(color&8)) {
		p = &CURC->gc_baseaddr[3][offs];
		colfill_b_and(p,lwdt,mask,h);
	      }
	      break;
	  default: {
	      p = &CURC->gc_baseaddr[0][offs];
	      hh = h;
	      if (color&1) colfill_b_or(p,lwdt,mask,hh);
	      else         colfill_b_and(p,lwdt,~mask,hh);
	      p = &CURC->gc_baseaddr[1][offs];
	      hh = h;
	      if (color&2) colfill_b_or(p,lwdt,mask,hh);
	      else         colfill_b_and(p,lwdt,~mask,hh);
	      p = &CURC->gc_baseaddr[2][offs];
	      hh = h;
	      if (color&4) colfill_b_or(p,lwdt,mask,hh);
	      else         colfill_b_and(p,lwdt,~mask,hh);
	      p = &CURC->gc_baseaddr[3][offs];
	      if (color&8) colfill_b_or(p,lwdt,mask,h);
	      else         colfill_b_and(p,lwdt,~mask,h);
	      break;
	    }
	}
	GRX_LEAVE();
}

static
#include "fdrivers/generic/block.c"

static
#include "fdrivers/generic/line.c"

static
#include "fdrivers/generic/bitmap.c"

static
#include "fdrivers/generic/pattern.c"

static
#include "fdrivers/generic/bitblt.c"

static void bltr2r(GrFrame *dst,int dx,int dy,
		   GrFrame *src,int x,int y,int w,int h,
		   GrColor op)
{
    GRX_ENTER();
    _GR_rblit_14(dst,dx,dy,src,x,y,w,h,op,4,bitblt);
    GRX_LEAVE();
}

GrFrameDriver _GrFrameDriverRAM4 = {
    GR_frameRAM4,               /* frame mode */
    GR_frameUndef,              /* compatible RAM frame mode */
    FALSE,                      /* onscreen */
    4,                          /* scan line width alignment */
    4,                          /* number of planes */
    4,                          /* bits per pixel */
#ifdef __TURBOC__
    65520L,                     /* max plane size the code can handle */
#else
    16*1024L*1024L,             /* max plane size the code can handle */
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
    bltr2r,
    NULL,
    NULL,
    _GrFrDrvGenericGetIndexedScanline,
    _GrFrDrvGenericPutScanline
};
