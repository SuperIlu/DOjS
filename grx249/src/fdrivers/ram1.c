/**
 ** ram1.c ---- the mono system RAM frame driver
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
#include "allocate.h"
#include "arith.h"
#include "mempeek.h"
#include "memcopy.h"
#include "memfill.h"

#include "fdrivers/rblit_14.h"

/* frame offset address calculation */
#define FOFS(x,y,lo)  umuladd32((y),(lo),((x)>>3))

static INLINE
GrColor readpixel(GrFrame *c,int x,int y)
{
	GR_int8u far *ptr;
	GRX_ENTER();
	ptr = (GR_int8u far *)&c->gf_baseaddr[0][FOFS(x,y,c->gf_lineoffset)];
	GRX_RETURN((GrColor)( (*ptr >> (7 - (x & 7)) ) & 1));
}

static INLINE
void drawpixel(int x,int y,GrColor color)
{
	GR_int8u far *ptr;
	GR_int8u cval;

	GRX_ENTER();
	ptr = (GR_int8u far *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
	cval = (color & 1) << (7 - (x &= 7));
	switch(C_OPER(color)) {
	    case C_XOR: *ptr ^=  cval; break;
	    case C_OR:  *ptr |=  cval; break;
	    case C_AND: *ptr &= ~cval; break;
	    default:    *ptr  = (*ptr & (~0x80 >> x)) | cval; break;
	}
	GRX_LEAVE();
}

#define maskoper(d,op,s,msk,SF,DF) do {                       \
    unsigned char _c_ = peek_b##DF(d);                        \
    poke_b##DF((d), (_c_ & ~(msk)) | ((_c_ op (s)) & (msk))); \
  } while (0)
#define maskset(d,c,msk,DF) \
    poke_b##DF((d),(peek_b##DF(d) & ~(msk)) | ((c) & (msk)))

static void drawhline(int x,int y,int w,GrColor color) {
  int oper;

  GRX_ENTER();
  oper = C_OPER(color);
  color &= 1;
  if (!( !color && (oper==C_OR||oper==C_XOR)) && !(color && oper==C_AND) ) {
    GR_int8u lm = 0xff >> (x & 7);
    GR_int8u rm = 0xff << ((-(w + x)) & 7);
    GR_int8u far *p = (GR_int8u far *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    GR_repl cv = 0;
    if (color) cv = ~cv;
    w = ((x+w+7) >> 3) - (x >> 3);
    if (w==1) lm &= rm;
    if ( ((GR_int8u)(~lm)) ) {
	switch(oper) {
	  case C_XOR: maskoper(p,^,(GR_int8u)cv,lm,_set,_n); break;
	  case C_OR:  maskoper(p,|,(GR_int8u)cv,lm,_set,_n); break;
	  case C_AND: maskoper(p,&,(GR_int8u)cv,lm,_set,_n); break;
	  default:    maskset(p,(GR_int8u)cv,lm,_n);         break;
	}
	if (!(--w)) goto done;
	++p;
    }
    if ( ((GR_int8u)(~rm)) ) --w;
    if (w) {
	switch(oper) {
	  case C_XOR: repfill_b_xor(p,cv,w); break;
	  case C_OR:  repfill_b_or(p,cv,w);  break;
	  case C_AND: repfill_b_and(p,cv,w); break;
	  default:    repfill_b(p,cv,w);     break;
	}
    }
    if ( ((GR_int8u)(~rm)) ) {
	switch(oper) {
	  case C_XOR: maskoper(p,^,(GR_int8u)cv,rm,_set,_n); break;
	  case C_OR:  maskoper(p,|,(GR_int8u)cv,rm,_set,_n); break;
	  case C_AND: maskoper(p,&,(GR_int8u)cv,rm,_set,_n); break;
	  default:    maskset(p,(GR_int8u)cv,rm,_n);         break;
	}
    }
  }
done:
  GRX_LEAVE();
}

static void drawvline(int x,int y,int h,GrColor color)
{
	unsigned int lwdt, mask, oper;
	char far *p;
	GRX_ENTER();
	oper = C_OPER(color);
	color &= 1;
	lwdt = CURC->gc_lineoffset;
	mask = 0x80 >> (x & 7);
	switch (oper) {
	  case C_XOR:
	      /* no need to xor anything with 0 */
	      if (color) {
		p = &CURC->gc_baseaddr[0][FOFS(x,y,lwdt)];
		colfill_b_xor(p,lwdt,mask,h);
	      }
	      break;
	  case C_OR:
	      /* no need to or anything with 0 */
	      if (color) {
	    do_OR:
		p = &CURC->gc_baseaddr[0][FOFS(x,y,lwdt)];
		colfill_b_or(p,lwdt,mask,h);
	      }
	      break;
	  case C_AND:
	      /* no need to and anything with 1 */
	      if (!color) {
	    do_AND:
		mask = ~mask;
		p = &CURC->gc_baseaddr[0][FOFS(x,y,lwdt)];
		colfill_b_and(p,lwdt,mask,h);
	      }
	      break;
	  default:
	      if (color) goto do_OR;
	      goto do_AND;
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
    _GR_rblit_14(dst,dx,dy,src,x,y,w,h,op,1,bitblt);
    GRX_LEAVE();
}


/* -------------------------------------------------------------------- */

static
#include "fdrivers/generic/getiscl.c"

/* -------------------------------------------------------------------- */

GrFrameDriver _GrFrameDriverRAM1 = {
    GR_frameRAM1,               /* frame mode */
    GR_frameUndef,              /* compatible RAM frame mode */
    FALSE,                      /* onscreen */
    4,                          /* scan line width alignment */
    1,                          /* number of planes */
    1,                          /* bits per pixel */
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
    getindexedscanline,
    _GrFrDrvGenericPutScanline
};

