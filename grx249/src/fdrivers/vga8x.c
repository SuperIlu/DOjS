/**
 ** vga8x.c ---- the 256 color VGA mode X frame driver
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
#include "allocate.h"
#include "arith.h"
#include "docolor.h"
#include "mempeek.h"
#include "memcopy.h"
#include "memfill.h"
#include "ioport.h"
#include "vgaregs.h"
#include "highlow.h"

/* frame offset address calculation */
#define FOFS(x,y,lo) umuladd32((y),(lo),((x)>>2))

#define _SetVGAWritePlanes(planes) \
    outport_w(VGA_SEQUENCER_PORT,highlow((planes),VGA_WRT_PLANE_ENB_REG))

#define _SetVGAReadPlane(plane) \
    outport_w(VGA_GR_CTRL_PORT,highlow((plane),VGA_RD_PLANE_SEL_REG))

#define _SetVGAWritePlane(plane) _SetVGAWritePlanes(1<<(plane))

#define _SetVGAWriteAllPlanes()  _SetVGAWritePlanes(0x0f)

static GR_int8u _GrPXLmaskTable[] = { 0, 0x0e, 0x0c, 0x08 };
static GR_int8u _GrPXRmaskTable[] = { 0, 0x01, 0x03, 0x07 };


#define _SetNoPlane_
#define POKEX(P,C,OP,SRP) do {                                \
	   switch(OP) {                                       \
	      case C_XOR: SRP; poke_b_f_xor((P),(C)); break;  \
	      case C_OR:  SRP; poke_b_f_or( (P),(C)); break;  \
	      case C_AND: SRP; poke_b_f_and((P),(C)); break;  \
	      default:         poke_b_f(    (P),(C)); break;  \
	   }                                                  \
       } while (0)
#define POKEFAST(P,C,OP) POKEX((P),(C),(OP),_SetNoPlane_)


static INLINE
GrColor readpixel(GrFrame *c,int x,int y)
{
	GRX_ENTER();
	_SetVGAReadPlane(x&3);
	setup_far_selector(SCRN->gc_selector);
	GRX_RETURN((GR_int8u)
	  peek_b_f(&SCRN->gc_baseaddr[0][FOFS(x,y,SCRN->gc_lineoffset)])
	);
}


static INLINE
void drawpixel(int x,int y,GrColor color)
{
	char far *ptr;
	GRX_ENTER();
	ptr = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
	x &= 3;
	_SetVGAWritePlane(x);
	setup_far_selector(CURC->gc_selector);
	POKEX(ptr,color,C_OPER(color),_SetVGAReadPlane(x));
	GRX_LEAVE();
}


static void drawhline(int x,int y,int w,GrColor color) {
  int opr;

  GRX_ENTER();
  opr = C_OPER(color);
  if (w > 0 && DOCOLOR8(color,opr)) {
    GR_repl cval = freplicate_b(color);
    char far *p = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    setup_far_selector(CURC->gc_selector);
    if ((opr == C_WRITE) && (w >= 10)) {  /* 10 = 3(left) + 1*4 + 3(right) */
	int rmask = (x + w) & 3;
	int lmask =  x      & 3;
	if(lmask) {
	  _SetVGAWritePlanes(_GrPXLmaskTable[lmask]);
	  poke_b_f(p,(GR_int8u)cval);
	  ++p;
	  w -= (4 - lmask);
	}
	w >>= 2;
	if(rmask) {
	  _SetVGAWritePlanes(_GrPXRmaskTable[rmask]);
	  poke_b_f(p+w,(GR_int8u)cval);
	}
	_SetVGAWriteAllPlanes();
	repfill_b_f(p,cval,w);
    } else {
      int i, plane = x & 3;
      for (i=0; i < 4; ++i) {
	GR_int8u far *pp = (GR_int8u far *)p;
	int ww = (w+3) >> 2;
	if ( !ww ) break;
	_SetVGAWritePlane(plane);
	switch (opr) {
	  case C_XOR: _SetVGAReadPlane(plane);
		      repfill_b_f_xor(pp,cval,ww);
		      break;
	  case C_OR:  _SetVGAReadPlane(plane);
		      repfill_b_f_or(pp,cval,ww);
		      break;
	  case C_AND: _SetVGAReadPlane(plane);
		      repfill_b_f_and(pp,cval,ww);
		      break;
	  default   : repfill_b_f(pp,cval,ww);
		      break;
	}
	--w;
	if (++plane == 4) { plane = 0; ++p; }
      }
    }
  }
  GRX_LEAVE();
}


static void drawvline(int x,int y,int h,GrColor color)
{
	char far *ptr;
	unsigned skip;
	GR_int8u cv;
	GRX_ENTER();
	skip = CURC->gc_lineoffset;
	ptr = &CURC->gc_baseaddr[0][FOFS(x,y,skip)];
	cv = (GR_int8u)color;
	x &= 3;
	_SetVGAWritePlane(x);
	setup_far_selector(CURC->gc_selector);
	switch(C_OPER(color)) {
	  case C_XOR: _SetVGAReadPlane(x);
		      colfill_b_f_xor(ptr,skip,cv,h);
		      break;
	  case C_OR:  _SetVGAReadPlane(x);
		      colfill_b_f_or(ptr,skip,cv,h);
		      break;
	  case C_AND: _SetVGAReadPlane(x);
		      colfill_b_f_and(ptr,skip,cv,h);
		      break;
	  default:    colfill_b_f(ptr,skip,cv,h);
		      break;
	}
	GRX_LEAVE();
}


static
#include "fdrivers/generic/block.c"

/* ------------------------------------------------------------------------ */

#if 1

static INLINE
void xmajor(GR_int8u far *ptr, int len, int yskip,
	    GR_int32u ErrorAcc, GR_int32u ErrorAdj,
	    int op, int color)
{
    if (len) {
	while (--len) {
	    POKEFAST(ptr,color,op);
	    ptrinc(ptr,1);
	    ErrorAcc += ErrorAdj;

	    if (ErrorAcc & ~0xFFFFL) {
		ErrorAcc &= 0xFFFFL;
		ptrinc(ptr,yskip);
	    }
	}
	POKEFAST(ptr,color,op);
    }
}

static INLINE
void middle(GR_int8u far *ptr, int len, int yskip,
	    GR_int32u ErrorAcc, GR_int32u ErrorAdj,
	    int op, int color)
{
    if (len) {
	 while (--len) {
	    POKEFAST(ptr,color,op);
	    ErrorAcc += ErrorAdj;
	    ptrinc(ptr, (yskip * (int)(ErrorAcc >> 16)) + 1);
	    ErrorAcc &= 0xFFFFL;
	}
	POKEFAST(ptr,color,op);
    }
}


static INLINE
void ymajor(GR_int8u *ptr, int len, int yskip,
	    GR_int32u ErrorAcc, GR_int32u ErrorAdj,
	    int op, int color)
{

    if (len) {
	int i;
	GR_int32u TinyAdj = (ErrorAdj >> 2);
	ErrorAdj -= TinyAdj;

	while (--len) {
	    ErrorAcc += TinyAdj;
	    i = (ErrorAcc >> 16);
	    ErrorAcc &= 0xFFFFL;

	    while (i--) {
		POKEFAST(ptr,color,op);
		ptrinc(ptr,yskip);
	    }

	    ErrorAcc += ErrorAdj;
	    ptrinc(ptr, (yskip * (int)(ErrorAcc >> 16)) + 1);
	    ErrorAcc &= 0xFFFFL;
	}
	ErrorAcc += TinyAdj;
	i = (ErrorAcc >> 16);
	while (i--) {
	    POKEFAST(ptr,color,op);
	    ptrinc(ptr,yskip);
	}
    }
}


static void drawline(int x,int y,int dx,int dy,GrColor color)
{
    int i, yskip, oper, plane;
    int len[4];
    GR_int8u far *ptr;

    /* Mode X 4-way folded Bresenham line function - by David Boeren */

    GRX_ENTER();
    /* Make sure the line runs left to right */
    if (dx < 0) {
	x -= (dx=-dx);
	y -= (dy=-dy);
    }

    yskip = CURC->gc_lineoffset;
    if (dy < 0) {
	dy = -dy;  /* Make dy positive */
	yskip = -yskip;
    }

    if (dx == 0) {
	/* Vertical Line (and one pixel lines) */
	if (yskip > 0) {
	    drawvline(x,y,dy+1,color);
	    goto done;
	}
	drawvline(x,y-dy,dy+1,color);
	goto done;
    }

    if (dy == 0) {
	/* Horizontal Line */
	drawhline(x,y,dx+1,color);
	goto done;
    }

    oper = C_OPER(color);
    ptr = (GR_int8u far *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];

    /* Length of sub-line in each plane */
    plane = (x & 3);
    i = dx + plane;
    len[0] = ((i--) >> 2);
    len[1] = ((i--) >> 2);
    len[2] = ((i--) >> 2);
    len[3] = (i >> 2) + 1;

    for (i=plane; i < 3; i++) len[i]++;

    if ((dx >> 2) >= dy) {
	/* X-Major line (0.00 < slope <= 0.25) */
	GR_int32u ErrorAcc = 0x8000;
	GR_int32u ErrorAdj = ((((GR_int32u)dy << 18) / (GR_int32u)dx));
	GR_int32u TinyAdj  = (ErrorAdj >> 2);
	while (i--) {
	    if (oper != C_WRITE) _SetVGAReadPlane(plane);
	    _SetVGAWritePlane(plane);
	    xmajor(ptr, len[plane], yskip, ErrorAcc, ErrorAdj, oper, color);
	    if (++plane == 4) {
		plane = 0;
		ptrinc(ptr,1);
	    }
	    ErrorAcc += TinyAdj;
	    if (ErrorAcc & ~0xFFFFL) {
		ErrorAcc &= 0xFFFFL;
		ptrinc(ptr,yskip);
	    }
	}
	if (oper != C_WRITE) _SetVGAReadPlane(plane);
	_SetVGAWritePlane(plane);
	xmajor(ptr, len[plane], yskip, ErrorAcc, ErrorAdj, oper, color);
    } else if (dx >= dy) {
	/* Middle line (0.25 < slope <= 1.00) */
	GR_int32u ErrorAcc = 0x8000;
	GR_int32u ErrorAdj = ((((GR_int32u)dy << 18) / (GR_int32u)dx));
	GR_int32u TinyAdj  = (ErrorAdj >> 2);
	while (i--) {
	    if (oper != C_WRITE) _SetVGAReadPlane(plane);
	    _SetVGAWritePlane(plane);
	    middle(ptr, len[plane], yskip, ErrorAcc, ErrorAdj, oper, color);
	    if (++plane == 4) {
		plane = 0;
		ptrinc(ptr,1);
	    }
	    ErrorAcc += TinyAdj;
	    if (ErrorAcc & ~0xFFFFL) {
		ptrinc(ptr,yskip);
		ErrorAcc &= 0xFFFFL;
	    }
	}
	if (oper != C_WRITE) _SetVGAReadPlane(plane);
	_SetVGAWritePlane(plane);
	middle(ptr, len[plane], yskip, ErrorAcc, ErrorAdj, oper, color);
    } else {
	/* Y-Major line (slope > 1) */
	GR_int32u ErrorAcc = 0x8000;
	GR_int32u ErrorAdj = ((((GR_int32u)(dy+1) << 18) / (GR_int32u)(dx+1)));
	GR_int32u TinyAdj  = (ErrorAdj >> 2);
	while (i--) {
	    if (oper != C_WRITE) _SetVGAReadPlane(plane);
	    _SetVGAWritePlane(plane);
	    ymajor(ptr, len[plane], yskip, ErrorAcc, ErrorAdj, oper, color);
	    if (++plane == 4) {
		plane = 0;
		ptrinc(ptr,1);
	    }
	    ErrorAcc += TinyAdj;
	    ptrinc(ptr,(yskip * (ErrorAcc >> 16)));
	    ErrorAcc &= 0xFFFFL;
	}
	if (oper != C_WRITE) _SetVGAReadPlane(plane);
	_SetVGAWritePlane(plane);
	ymajor(ptr, len[plane], yskip, ErrorAcc, ErrorAdj, oper, color);
    }
  done:
    GRX_LEAVE();
}
#else
static
#include "fdrivers/generic/line.c"
#endif

/* ------------------------------------------------------------------------ */

static
#include "fdrivers/generic/bitmap.c"

static
#include "fdrivers/generic/pattern.c"

/* ---------------------------------------------------- video -> video blit */
static
#include "fdrivers/generic/bitblt.c"

static char far *LineBuff = NULL;

static void pbltv2v(GrFrame *dst,int dx,int dy,
		    GrFrame *src,int sx,int sy,
		    int w,int h,GrColor op     )
{
    GR_int32u soffs, doffs;
    int skip, lo;
    char far *vp;
    GRX_ENTER();
    op = C_OPER(op);
    lo = SCRN->gc_lineoffset;
    if (dy <= sy) {
      /* forward */
      skip  = lo;
    } else {
      /* reverse */
      dy += h-1;
      sy += h-1;
      skip = -lo;
    }
    soffs = FOFS(sx,sy,lo);
    doffs = FOFS(dx,dy,lo);
    setup_far_selector(SCRN->gc_selector);
    while (h--) {
      char far *dptr, *sptr;
      int ww;
      int plc, pl;
      pl = sx & 3;
      ww = w;
      vp = &SCRN->gc_baseaddr[0][soffs];
      dptr = LineBuff;
      for (plc=0; plc < 4; ++plc) {
	char far *sptr = vp;
	int bytes = (ww+3)>>2;
	_SetVGAReadPlane(pl);
	fwdcopy_set_f(sptr,dptr,sptr,bytes);
	if (--ww <= 0) break;
	if (++pl == 4) { pl=0; ++vp; }
      }
      pl = dx & 3;
      ww = w;
      vp = &SCRN->gc_baseaddr[0][doffs];
      sptr = LineBuff;
      switch (op) {
	case C_XOR:
	  for (plc=0; plc < 4; ++plc) {
	    char far *dptr = vp;
	    int bytes = (ww+3)>>2;
	    _SetVGAReadPlane(pl);
	    _SetVGAWritePlane(pl);
	    fwdcopy_f_xor(dptr,dptr,sptr,bytes);
	    if (--ww <= 0) break;
	    if (++pl == 4) { pl=0; ++vp; }
	  }
	  break;
	case C_OR:
	  for (plc=0; plc < 4; ++plc) {
	    char far *dptr = vp;
	    int bytes = (ww+3)>>2;
	    _SetVGAReadPlane(pl);
	    _SetVGAWritePlane(pl);
	    fwdcopy_f_or(dptr,dptr,sptr,bytes);
	    if (--ww <= 0) break;
	    if (++pl == 4) { pl=0; ++vp; }
	  }
	  break;
	case C_AND:
	  for (plc=0; plc < 4; ++plc) {
	    char far *dptr = vp;
	    int bytes = (ww+3)>>2;
	    _SetVGAReadPlane(pl);
	    _SetVGAWritePlane(pl);
	    fwdcopy_f_and(dptr,dptr,sptr,bytes);
	    if (--ww <= 0) break;
	    if (++pl == 4) { pl=0; ++vp; }
	  }
	  break;
	default:
	  for (plc=0; plc < 4; ++plc) {
	    char far *dptr = vp;
	    int bytes = (ww+3)>>2;
	    _SetVGAWritePlane(pl);
	    fwdcopy_f_set(dptr,dptr,sptr,bytes);
	    if (--ww <= 0) break;
	    if (++pl == 4) { pl=0; ++vp; }
	  }
	  break;
      }
      doffs += skip;
      soffs += skip;
    }
    GRX_LEAVE();
}

static int alloc_blit_buffer(int width) {
    GRX_ENTER();
    LineBuff = _GrTempBufferAlloc(width);
    GRX_RETURN(LineBuff != NULL);
}

static void bltv2v(GrFrame *dst,int dx,int dy,
		   GrFrame *src,int sx,int sy,
		   int w,int h,GrColor op)
{
	GRX_ENTER();
	if(GrColorMode(op) != GrIMAGE && alloc_blit_buffer(w))
	    pbltv2v(dst,dx,dy,src,sx,sy,w,h,op);
	else
	    bitblt(dst,dx,dy,src,sx,sy,w,h,op);
	GRX_LEAVE();
}



/* ------------------------------------------------- video -> ram blit */

#define VID2MEM_OPR(M,OP) do {                   \
      int _ww_ = w;                              \
      for (plc = 0; plc < 4; ++plc) {            \
	char far *_p_ = vp;                      \
	char far *_dptr_ = &M[plc];              \
	int _w_ = (_ww_+3)>>2;                   \
	_SetVGAReadPlane(pl);                    \
	colcopy_b##OP##_f(_dptr_,4,_p_,1,_w_);   \
	if ((--_ww_) == 0) break;                \
	if (++pl == 4) { pl = 0; ++vp; }         \
      }                                          \
} while (0)

static void pbltv2r(GrFrame *dst,int dx,int dy,
		    GrFrame *src,int sx,int sy,
		    int w,int h,GrColor op     )
{
    GR_int32u soffs;
    int sskip, dskip;
    char far *rp, *vp;
    GRX_ENTER();
    op = C_OPER(op);
    sskip = SCRN->gc_lineoffset;
    dskip = dst->gf_lineoffset;
    soffs = FOFS(sx,sy,sskip);
    rp = &dst->gf_baseaddr[0][umuladd32(dy,dskip,dx)];
    setup_far_selector(SCRN->gc_selector);
    while (h--) {
      int plc, pl;
      pl = sx & 3;
      vp = &SCRN->gc_baseaddr[0][soffs];
      switch (op) {
	case C_XOR: VID2MEM_OPR(rp,_xor); break;
	case C_OR:  VID2MEM_OPR(rp,_or);  break;
	case C_AND: VID2MEM_OPR(rp,_and); break;
	default:    VID2MEM_OPR(rp,_set); break;
      }
      rp += dskip;
      soffs += sskip;
    }
    GRX_LEAVE();
}

static void bltv2r(GrFrame *dst,int dx,int dy,
		   GrFrame *src,int sx,int sy,
		   int w,int h,GrColor op)
{
	GRX_ENTER();
	if(GrColorMode(op) != GrIMAGE)
	    pbltv2r(dst,dx,dy,src,sx,sy,w,h,op);
	else
	    _GrFrDrvGenericBitBlt(dst,dx,dy,src,sx,sy,w,h,op);
	GRX_LEAVE();
}

/* ------------------------------------------------- ram -> video blit */

#define MEM2VID_OPR(M,OP) do {                   \
      int _ww_ = w;                              \
      for (plc = 0; plc < 4; ++plc) {            \
	char far *_p_ = vp;                      \
	char far *_sptr_ = &M[plc];              \
	int _w_ = (_ww_+3)>>2;                   \
	_SetVGAWritePlane(pl);                   \
	_SetVGAReadPlane(pl);                    \
	colcopy_b_f##OP(_p_,1,_sptr_,4,_w_);     \
	if ((--_ww_) == 0) break;                \
	if (++pl == 4) { pl = 0; ++vp; }         \
      }                                          \
} while (0)

#define MEM2VID_SET(M) do {                      \
      int _ww_ = w;                              \
      for (plc = 0; plc < 4; ++plc) {            \
	char far *_p_ = vp;                      \
	char far *_sptr_ = &M[plc];              \
	int _w_ = (_ww_+3)>>2;                   \
	_SetVGAWritePlane(pl);                   \
	colcopy_b_f_set(_p_,1,_sptr_,4,_w_);     \
	if ((--_ww_) == 0) break;                \
	if (++pl == 4) { pl = 0; ++vp; }         \
      }                                          \
} while (0)

static void pbltr2v(GrFrame *dst,int dx,int dy,
		    GrFrame *src,int sx,int sy,
		    int w,int h,GrColor op     )
{
    GR_int32u doffs;
    int sskip, dskip;
    char far *rp, *vp;
    GRX_ENTER();
    op = C_OPER(op);
    dskip = SCRN->gc_lineoffset;
    sskip = src->gf_lineoffset;
    doffs = FOFS(dx,dy,dskip);
    rp = &src->gf_baseaddr[0][umuladd32(sy,sskip,sx)];
    setup_far_selector(SCRN->gc_selector);
    while (h--) {
      int plc, pl;
      pl = dx & 3;
      vp = &SCRN->gc_baseaddr[0][doffs];
      switch (op) {
	case C_XOR: MEM2VID_OPR(rp,_xor); break;
	case C_OR:  MEM2VID_OPR(rp,_or);  break;
	case C_AND: MEM2VID_OPR(rp,_and); break;
	default:    MEM2VID_SET(rp);      break;
      }
      doffs += dskip;
      rp += sskip;
    }
    GRX_LEAVE();
}

static void bltr2v(GrFrame *dst,int dx,int dy,
		   GrFrame *src,int sx,int sy,
		   int w,int h,GrColor op)
{
	GRX_ENTER();
	if(GrColorMode(op) == GrIMAGE)
	    _GrFrDrvGenericBitBlt(dst,dx,dy,src,sx,sy,w,h,op);
	else
	    pbltr2v(dst,dx,dy,src,sx,sy,w,h,op);
	GRX_LEAVE();
}



GrFrameDriver _GrFrameDriverVGA8X = {
    GR_frameVGA8X,              /* frame mode */
    GR_frameRAM8,               /* compatible RAM frame mode */
    TRUE,                       /* onscreen */
    2,                          /* scan line width alignment */
    4,                          /* number of planes */
    8,                          /* bits per pixel */
    64*1024L,                   /* max plane size the code can handle */
    NULL,
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

