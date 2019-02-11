/**
 ** egavga1.c ---- the mono EGA/VGA frame driver
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
#include "vgaregs.h"
#include "ioport.h"

/* frame offset address calculation */
#define FOFS(x,y,lo) umuladd32((y),(lo),((x) >> 3))

void GrSetEGAVGAmonoDrawnPlane(int plane)
{
    GRX_ENTER();
    plane &= 3;
    outport_w(VGA_SEQUENCER_PORT,((0x100 << plane) | VGA_WRT_PLANE_ENB_REG));
    outport_w(VGA_GR_CTRL_PORT,((plane << 8) | VGA_RD_PLANE_SEL_REG));
    GRX_LEAVE();
}

void GrSetEGAVGAmonoShownPlane(int plane)
{
	void (*DACload)(int c,int r,int g,int b);
	int i;
	GRX_ENTER();
	DACload = DRVINFO->actmode.extinfo->loadcolor;
	plane &= 3;
	if(DACload) for(i = 0; i < 16; i++) {
	    int v = (i & (1 << plane)) ? 255 : 0;
	    (*DACload)(i,v,v,v);
	}
	GRX_LEAVE();
}

static size_t LineBytes = 0;
static char far *LineBuff = NULL;

static int alloc_blit_buffer(void) {
    GRX_ENTER();
    LineBuff = _GrTempBufferAlloc(LineBytes);
    GRX_RETURN(LineBuff != NULL);
}

static int init(GrVideoMode *mp)
{
    GRX_ENTER();

    /* set write mode 0 */
    outport_w(VGA_GR_CTRL_PORT,((0 << 8) | VGA_MODE_REG));
    /* don't care register to 0 */
    outport_w(VGA_GR_CTRL_PORT,((0 << 8) | VGA_COLOR_DONTC_REG));
    /* disable all planes for set/reset */
    outport_w(VGA_GR_CTRL_PORT,((0 << 8) | VGA_SET_RESET_ENB_REG));
    GrSetEGAVGAmonoShownPlane(0);
    GrSetEGAVGAmonoDrawnPlane(0);

    /* set up LineBuff max. line length for blits */
    LineBytes = sizeof(char) * (((mp->width+7) >> 3)+2);

    GRX_RETURN(TRUE);
}

static INLINE
GrColor readpixel(GrFrame *c,int x,int y)
{
    char far *ptr;
    GRX_ENTER();
    ptr = &SCRN->gc_baseaddr[0][FOFS(x,y,SCRN->gc_lineoffset)];
    setup_far_selector(SCRN->gc_selector);
    GRX_RETURN((GrColor)((peek_b_f(ptr) >> (7 - (x & 7)) ) & 1));
}

static INLINE
void drawpixel(int x,int y,GrColor color)
{
	char far *ptr;
	unsigned cval;
	GRX_ENTER();
	ptr = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
	cval= ((unsigned int)color & 1) << (7 - (x &= 7));
	setup_far_selector(CURC->gc_selector);
	switch(C_OPER(color)) {
	    case C_XOR: poke_b_f_xor(ptr,cval);  break;
	    case C_OR:  poke_b_f_or(ptr,cval);   break;
	    case C_AND: poke_b_f_and(ptr,~cval); break;
	    default:    poke_b_f(ptr,((peek_b_f(ptr) & (~0x80 >> x)) | cval));
	}
	GRX_LEAVE();
}

#define maskoper(d,op,s,msk,SF,DF) do {                       \
    unsigned char _c_ = peek_b##DF(d);                        \
    poke_b##DF((d), (_c_ & ~(msk)) | ((_c_ op (s)) & (msk))); \
  } while (0)
#define maskset(d,c,msk,DF) \
    poke_b##DF((d),(peek_b##DF(d) & ~(msk)) | ((c) & (msk)))

static void drawhline(int x,int y,int w,GrColor color)
{
    int oper;
    GRX_ENTER();
    oper  = C_OPER(color);
    color = color & 1 ? ~0L : 0L;
    if (   !(!color && (oper==C_OR||oper==C_XOR))
	&& !( color && oper==C_AND)                ) {
      GR_int8u far *pp = (GR_int8u far *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
      GR_int8u lm = 0xff >> (x & 7);
      GR_int8u rm = 0xff << ((-(w += x)) & 7);
      w  = ((w + 7) >> 3) - (x >> 3);
      if(w == 1) lm &= rm;
      setup_far_selector(CURC->gc_selector);
      if( lm ) {
	switch(oper) {
	  case C_XOR: maskoper(pp,^,(GR_int8u)color,lm,_f,_f); break;
	  case C_OR:  maskoper(pp,|,(GR_int8u)color,lm,_f,_f); break;
	  case C_AND: maskoper(pp,&,(GR_int8u)color,lm,_f,_f); break;
	  default:    maskset(pp,(GR_int8u)color,lm,_f);       break;
		      break;
	}
	if (!(--w)) goto done;
	++pp;
      }
      if ( rm ) --w;
      if (w) {
	switch(oper) {
	  case C_XOR: repfill_b_f_xor(pp,color,w); break;
	  case C_OR:  repfill_b_f_or( pp,color,w); break;
	  case C_AND: repfill_b_f_and(pp,color,w); break;
	  default:    repfill_b_f(    pp,color,w); break;
	}
      }
      if ( rm ) {
	switch(oper) {
	  case C_XOR: maskoper(pp,^,(GR_int8u)color,rm,_f,_f); break;
	  case C_OR:  maskoper(pp,|,(GR_int8u)color,rm,_f,_f); break;
	  case C_AND: maskoper(pp,&,(GR_int8u)color,rm,_f,_f); break;
	  default:    maskset(pp,(GR_int8u)color,rm,_f);       break;
	}
      }
    }
done:
    GRX_LEAVE();
}


static void drawvline(int x,int y,int h,GrColor color)
{
	char far *p;
	unsigned int cval;
	unsigned int lwdt;
	GRX_ENTER();
	lwdt = CURC->gc_lineoffset;
	cval = ((unsigned int)color & 1) << (7 - (x & 7));
	setup_far_selector(CURC->gc_selector);
	switch (C_OPER(color)) {
	  case C_XOR:
	      /* no need to xor anything with 0 */
	      if (cval) {
		p = &CURC->gc_baseaddr[0][FOFS(x,y,lwdt)];
		colfill_b_f_xor(p,lwdt,cval,h);
	      }
	      break;
	  case C_OR:
	      /* no need to or anything with 0 */
	      if (cval) {
	    do_OR:
		p = &CURC->gc_baseaddr[0][FOFS(x,y,lwdt)];
		colfill_b_f_or(p,lwdt,cval,h);
	      }
	      break;
	  case C_AND:
	      /* no need to and anything with 1 */
	      if (!cval) {
	    do_AND:
		cval = ~(1 << (7 - (x & 7)));
		p = &CURC->gc_baseaddr[0][FOFS(x,y,lwdt)];
		colfill_b_f_and(p,lwdt,cval,h);
	      }
	      break;
	  default:
	      if (cval) goto do_OR;
	      goto do_AND;
	      break;
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

/* some routines for fast blit operations
**
** All algorithms use a preallocated buffer for temporary
** storage and manipulation of pixel data: LineBuff[4]
**
** video -> video algorithm:
**   read data from scanline into LineBuff[plane]
**   if bit alignment between source and destination differs
**     shift LineBuff for destination bit alignment
**   write LineBuff[plane] to video memory
**
** ram -> video algorithm:
**   calculate 'start of line' pointer (slp) holding the first
**            blitting byte in RAM
**   if bit alignment between RAM and screen differs
**     shift&copy from slp to LineBuff with destination
**     bit alignment. Reload slp with LineBuff
**   write slp to video memory
**
** video -> ram algorithm
**   read data from scanline into LineBuff[plane]
**   if bit alignment between screen and RAM differs
**     shift LineBuff for RAM bit alignment
**   copy from LineBuff to RAM
**
** These algorithms save a lot of VGA register port settings
** compared with the old pixel by pixel method.
*/

static void put_scanline(char far *dptr,char far *sptr,int w,
			 GR_int8u lm, GR_int8u rm, int op    ) {
  GRX_ENTER();
  if (w==1) lm &= rm;
  if ( ((GR_int8u)(~lm)) ) {
    switch (op) {
      case C_XOR: maskoper(dptr,^,*sptr,lm,_set,_f); break;
      case C_OR : maskoper(dptr,|,*sptr,lm,_set,_f); break;
      case C_AND: maskoper(dptr,&,*sptr,lm,_set,_f); break;
      default   : maskset(dptr,*sptr,lm,_f);         break;
    }
    if (--w == 0) goto done;
    ++dptr;
    ++sptr;
  }
  if ( ((GR_int8u)(~rm)) ) --w;
  if (w) switch (op) {
      case C_XOR: fwdcopy_f_xor(dptr,dptr,sptr,w); break;
      case C_OR : fwdcopy_f_or( dptr,dptr,sptr,w); break;
      case C_AND: fwdcopy_f_and(dptr,dptr,sptr,w); break;
      default   : fwdcopy_f_set(dptr,dptr,sptr,w); break;
  }
  if ( ((GR_int8u)(~rm)) )
    switch (op) {
      case C_XOR: maskoper(dptr,^,*sptr,rm,_set,_f); break;
      case C_OR : maskoper(dptr,|,*sptr,rm,_set,_f); break;
      case C_AND: maskoper(dptr,&,*sptr,rm,_set,_f); break;
      default   : maskset(dptr,*sptr,rm,_f);         break;
    }
done:
  GRX_LEAVE();
}

static void get_scanline(char far *dptr, char far *sptr, int w) {
    GRX_ENTER();
    fwdcopy_set_f(sptr,dptr,sptr,w);
    GRX_LEAVE();
}

extern void _GR_shift_scanline(GR_int8u far **dst,
			       GR_int8u far **src,
			       int ws, int shift, int planes );
#define shift_scanline(dst,src,w,sh) \
    _GR_shift_scanline((GR_int8u **)&(dst),(GR_int8u **)&(src),(w),(sh),1)


static
#include "fdrivers/generic/bitblt.c"

static void bltv2v(GrFrame *dst,int dx,int dy,
		   GrFrame *src,int x,int y,int w,int h,
		   GrColor op)
{
    GRX_ENTER();
    if(GrColorMode(op) != GrIMAGE && alloc_blit_buffer()) {
	int shift = ((int)(x&7)) - ((int)(dx&7));
	char far *dptr, *sptr;
	int      skip, lo = SCRN->gc_lineoffset;
	int      oper= C_OPER(op);
	GR_int8u lm = 0xff >> (dx & 7);
	GR_int8u rm = 0xff << ((-(w + dx)) & 7);
	int      ws = ((x+w+7) >> 3) - (x >> 3);
	int      wd = ((dx+w+7) >> 3) - (dx >> 3);
	setup_far_selector(SCRN->gc_selector);
	if (dy < y) {
	  skip = lo;
	} else {
	  y += h-1; dy += h-1;
	  skip = -lo;
	}
	sptr = &SCRN->gc_baseaddr[0][FOFS(x,y,lo)];
	dptr = &SCRN->gc_baseaddr[0][FOFS(dx,dy,lo)];
	while (h-- > 0) {
	  get_scanline(LineBuff,sptr,ws);
	  if (shift)
	    shift_scanline(LineBuff,LineBuff,ws,shift);
	  put_scanline(dptr,LineBuff, wd, lm, rm, oper);
	  dptr += skip;
	  sptr += skip;
	}
    } else
	bitblt(dst,dx,dy,src,x,y,w,h,op);
    GRX_LEAVE();
}

static void bltr2v(GrFrame *dst,int dx,int dy,
		   GrFrame *src,int x,int y,int w,int h,
		   GrColor op)
{
    GRX_ENTER();
    if(GrColorMode(op) != GrIMAGE && alloc_blit_buffer()) {
	int oper  = C_OPER(op);
	int shift = ((int)(x&7)) - ((int)(dx&7));
	char far *dptr, *sptr;
	int sskip,dskip;
	GR_int8u lm = 0xff >> (dx & 7);
	GR_int8u rm = 0xff << ((-(w + dx)) & 7);
	int ws = ((x+w+7) >> 3) - (x >> 3);
	int wd = ((dx+w+7) >> 3) - (dx >> 3);
	setup_far_selector(SCRN->gc_selector);
	dskip = SCRN->gc_lineoffset;
	dptr = &SCRN->gc_baseaddr[0][FOFS(dx,dy,dskip)];
	sskip = src->gf_lineoffset;
	sptr = &src->gf_baseaddr[0][FOFS(x,y,sskip)];
	if (shift)
	  while (h-- > 0) {
	      shift_scanline(LineBuff,sptr,ws,shift);
	      put_scanline(dptr,LineBuff,wd,lm,rm,oper);
	      dptr += dskip;
	      sptr += sskip;
	  }
	else
	  while (h-- > 0) {
	      put_scanline(dptr,sptr,wd,lm,rm,oper);
	      dptr += dskip;
	      sptr += sskip;
	  }
    } else
	_GrFrDrvGenericBitBlt(dst,dx,dy,src,x,y,w,h,op);
    GRX_LEAVE();
}


static void bltv2r(GrFrame *dst,int dx,int dy,
		   GrFrame *src,int x,int y,int w,int h,
		   GrColor op)
{
    GRX_ENTER();
    while(GrColorMode(op) != GrIMAGE)
      if(alloc_blit_buffer()) {
	int oper  = C_OPER(op);
	int shift = ((int)(x&7)) - ((int)(dx&7));
	char far *dp, *dptr, *sp, *sptr;
	int sskip,dskip;
	GR_int8u lm = 0xff >> (dx & 7);
	GR_int8u rm = 0xff << ((-(w + dx)) & 7);
	int ws = ((x+w+7) >> 3) - (x >> 3);
	int wd = ((dx+w+7) >> 3) - (dx >> 3);
	if (wd==1) break;
	setup_far_selector(SCRN->gc_selector);
	sskip = SCRN->gc_lineoffset;
	sp    = &SCRN->gc_baseaddr[0][FOFS(x,y,sskip)];
	dskip = dst->gf_lineoffset;
	dp    = &dst->gf_baseaddr[0][FOFS(x,y,dskip)];
	while (h-- > 0) {
	  int ww = wd;
	  get_scanline(LineBuff,sp,ws);
	  if (shift)
	    shift_scanline(LineBuff,LineBuff,ws,shift);
	  sptr = LineBuff;
	  dptr = dp;
	  if ( ((GR_int8u)(~lm)) ) {
	    switch (op) {
	      case C_XOR: maskoper(dptr,^,*sptr,lm,_set,_n); break;
	      case C_OR : maskoper(dptr,|,*sptr,lm,_set,_n); break;
	      case C_AND: maskoper(dptr,&,*sptr,lm,_set,_n); break;
	      default   : maskset(dptr,*sptr,lm,_n);         break;
	    }
	    ++dptr;
	    ++sptr;
	    if (!(--ww)) goto next;
	  }
	  if ( ((GR_int8u)(~rm)) ) --ww;
	  if (ww) switch(oper) {
	      case C_XOR: fwdcopy_xor(dptr,dptr,sptr,ww); break;
	      case C_OR:  fwdcopy_or( dptr,dptr,sptr,ww); break;
	      case C_AND: fwdcopy_and(dptr,dptr,sptr,ww); break;
	      default:    fwdcopy_set(dptr,dptr,sptr,ww); break;
	  }
	  if ( ((GR_int8u)(~rm)) ) {
	    switch (op) {
	      case C_XOR: maskoper(dptr,^,*sptr,rm,_set,_n); break;
	      case C_OR : maskoper(dptr,|,*sptr,rm,_set,_n); break;
	      case C_AND: maskoper(dptr,&,*sptr,rm,_set,_n); break;
	      default   : maskset(dptr,*sptr,rm,_n);         break;
	    }
	  }
	next:
	  sp += sskip;
	  dp += dskip;
	}
	goto done;
      }
    _GrFrDrvGenericBitBlt(dst,dx,dy,src,x,y,w,h,op);
done:
    GRX_LEAVE();
}


GrFrameDriver _GrFrameDriverEGAVGA1 = {
    GR_frameEGAVGA1,            /* frame mode */
    GR_frameRAM1,               /* compatible RAM frame mode */
    TRUE,                       /* onscreen */
    4,                          /* line width alignment */
    1,                          /* number of planes */
    1,                          /* bits per pixel */
    64*1024L,                   /* max plane size the code can handle */
    init,
    readpixel,
    drawpixel,
    drawline,
    drawhline,
    drawvline,
    drawblock,
    drawbitmap,
    drawpattern,
    bltv2v,
    bltv2r,
    bltr2v,
    _GrFrDrvGenericGetIndexedScanline,
    _GrFrDrvGenericPutScanline
};

