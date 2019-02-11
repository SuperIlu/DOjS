/**
 ** driver16.h ---- the 32k/64k color padded SVGA linear frame buffer driver
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

/* frame offset address calculation */
#define FOFS(x,y,lo) umuladd32((y),(lo),((x) << 1))

/* #define FAR_ACCESS for video access routines */

#ifdef FAR_ACCESS
# define peek16          peek_w_f
# define poke16_xor      poke_w_f_xor
# define poke16_or       poke_w_f_or
# define poke16_and      poke_w_f_and
# define poke16          poke_w_f
# define colfill16_xor   colfill_w_f_xor
# define colfill16_or    colfill_w_f_or
# define colfill16_and   colfill_w_f_and
# define colfill16       colfill_w_f
# define repfill16_xor   repfill_w_f_xor
# define repfill16_or    repfill_w_f_or
# define repfill16_and   repfill_w_f_and
# define repfill16       repfill_w_f
# define SETFARSEL(sel)  setup_far_selector(sel)
# if defined(__GNUC__) && defined(__i386__)
#   define ASM_386_SEL   I386_GCC_FAR_SELECTOR
# endif /* GCC i386 */
#else /* defined FAR_ACCESS */
# define peek16          peek_w
# define poke16_xor      poke_w_xor
# define poke16_or       poke_w_or
# define poke16_and      poke_w_and
# define poke16          poke_w
# define colfill16_xor   colfill_w_xor
# define colfill16_or    colfill_w_or
# define colfill16_and   colfill_w_and
# define colfill16       colfill_w
# define repfill16_xor   repfill_w_xor
# define repfill16_or    repfill_w_or
# define repfill16_and   repfill_w_and
# define repfill16       repfill_w
# define SETFARSEL(sel)
#endif

#ifndef ASM_386_SEL
# define ASM_386_SEL
#endif

static INLINE
GrColor readpixel(GrFrame *c,int x,int y)
{
    GR_int16u far *pp;
    GRX_ENTER();
#ifdef FAR_ACCESS
    pp = (GR_int16u far *)&SCRN->gc_baseaddr[0][FOFS(x,y,SCRN->gc_lineoffset)];
    setup_far_selector(SCRN->gc_selector);
#else
/* problem with LFB_BY_NEAR_POINTER here? Does c always point to screen? */
    pp = (GR_int16u far *)&c->gf_baseaddr[0][FOFS(x,y,c->gf_lineoffset)];
#endif
#if defined(MISALIGNED_16bit_OK) && !defined(FAR_ACCESS)
    GRX_RETURN(*pp);
#else
    GRX_RETURN((GR_int16u)peek16(pp));
#endif
}

static INLINE
void drawpixel(int x,int y,GrColor color)
{
    char far *ptr;
    GRX_ENTER();
    ptr = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    SETFARSEL(CURC->gc_selector);
    switch(C_OPER(color)) {
	case C_XOR: poke16_xor(ptr,(GR_int16u)color); break;
	case C_OR:  poke16_or( ptr,(GR_int16u)color); break;
	case C_AND: poke16_and(ptr,(GR_int16u)color); break;
	default:    poke16(    ptr,(GR_int16u)color); break;
    }
    GRX_LEAVE();
}

static void drawhline(int x,int y,int w,GrColor color)
{
    char far *pp;
    GR_repl cval;
    GRX_ENTER();
    pp = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    cval = freplicate_w(color);
    SETFARSEL(CURC->gc_selector);
    switch(C_OPER(color)) {
	case C_XOR: repfill16_xor(pp,cval,w); break;
	case C_OR:  repfill16_or( pp,cval,w); break;
	case C_AND: repfill16_and(pp,cval,w); break;
	default:    repfill16(    pp,cval,w); break;
    }
    GRX_LEAVE();
}

static void drawvline(int x,int y,int h,GrColor color)
{
    unsigned lwdt;
    char far *pp;
    GRX_ENTER();
    lwdt = CURC->gc_lineoffset;
    pp   = &CURC->gc_baseaddr[0][FOFS(x,y,lwdt)];
    SETFARSEL(CURC->gc_selector);
    switch(C_OPER(color)) {
	case C_XOR: colfill16_xor(pp,lwdt,(GR_int16u)color,h); break;
	case C_OR:  colfill16_or( pp,lwdt,(GR_int16u)color,h); break;
	case C_AND: colfill16_and(pp,lwdt,(GR_int16u)color,h); break;
	default:    colfill16(    pp,lwdt,(GR_int16u)color,h); break;
    }
    GRX_LEAVE();
}

static void drawblock(int x,int y,int w,int h,GrColor color)
{
    int  skip;
    char far *ptr;
    GR_repl cval;

    GRX_ENTER();
    skip = CURC->gc_lineoffset;
    ptr  = &CURC->gc_baseaddr[0][FOFS(x,y,skip)];
    skip -= w<<1;
    cval = freplicate_w(color);
    SETFARSEL(CURC->gc_selector);
    switch (C_OPER(color)) {
      case C_XOR: while (h-- != 0) {
		    int ww = w;
		    repfill16_xor(ptr,cval,ww);
		    ptr += skip;
		  }
		  break;
      case C_OR:  while (h-- != 0) {
		    int ww = w;
		    repfill16_or(ptr,cval,ww);
		    ptr += skip;
		  }
		  break;
      case C_AND: while (h-- != 0) {
		    int ww = w;
		    repfill16_and(ptr,cval,ww);
		    ptr += skip;
		  }
		  break;
      default:    while (h-- != 0) {
		    int ww = w;
		    repfill16(ptr,cval,ww);
		    ptr += skip;
		  }
		  break;
    }
    GRX_LEAVE();
}

#if defined(__GNUC__) && defined(__i386__)
static void drawline(int x,int y,int dx,int dy,GrColor color)
{
    struct {
	int errsub;         /* subtract from error term */
	int erradd;         /* add to error term when carry */
	int offset1;        /* add to pointer if no carry on error term */
	int offset2;        /* add to pointer if carry / banking dir */
    } lndata;
    int  npts,error,xstep;
    char far *ptr;

    GRX_ENTER();

#   ifdef __GNUC__
#   ifdef __i386__
#   define ASM_LINE1(OPC) asm volatile(""              \
	"   .align 2,0x90                      \n"     \
	"0: "#OPC"w %6,"ASM_386_SEL"(%0)       \n"     \
	"   subl %7,%2                         \n"     \
	"   jb   1f                            \n"     \
	"   leal -2(%3),%3                     \n"     \
	"   decl %1                            \n"     \
	"   jne  0b                            \n"     \
	"   jmp  2f                            \n"     \
	"   .align 2,0x90                      \n"     \
	"1: addl 4  + %7,%2                    \n"     \
	"   addl 12 + %7,%3                    \n"     \
	"   decl %1                            \n"     \
	"   jne  0b                            \n"     \
	"2: "                                          \
	 : "=r" (ptr), "=r" (npts), "=r" (error)       \
	 : "0" ((long)(ptr)), "1" (npts), "2" (error), \
	   "r" ((short)(color)), "o" (lndata)          \
	)
#   define ASM_LINE2(OPC) asm volatile(""              \
	"   .align 2,0x90                      \n"     \
	"0: "#OPC"w %6,"ASM_386_SEL"(%0)       \n"     \
	"   subl %7,%2                         \n"     \
	"   jb   1f                            \n"     \
	"   addl 8 + %7,%3                     \n"     \
	"   decl %1                            \n"     \
	"   jne  0b                            \n"     \
	"   jmp  2f                            \n"     \
	"   .align 2,0x90                      \n"     \
	"1: addl 4  + %7,%2                    \n"     \
	"   addl 12 + %7,%3                    \n"     \
	"   decl %1                            \n"     \
	"   jne  0b                            \n"     \
	"2: "                                          \
	 : "=r" (ptr), "=r" (npts), "=r" (error)       \
	 : "0" ((long)(ptr)), "1" (npts), "2" (error), \
	   "r" ((short)(color)), "o" (lndata)          \
	)
#   endif
#   endif

    if(dy < 0) {
	y -= (dy = (-dy));
	x -= (dx = (-dx));
    }
    if(dx < 0) {
	xstep = (-2);
	dx    = (-dx);
    } else
	xstep = 2;

    ptr = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    SETFARSEL(CURC->gc_selector);
    if(dx > dy) {
	npts  = dx +  1;
	error = dx >> 1;
	lndata.errsub  = dy;
	lndata.erradd  = dx;
	lndata.offset1 = xstep;
	lndata.offset2 = CURC->gc_lineoffset + xstep;
	if(xstep < 0) {
	    lndata.offset1 = 1;
	    switch(C_OPER(color)) {
		case C_XOR: ASM_LINE1(xor); break;
		case C_OR:  ASM_LINE1(or);  break;
		case C_AND: ASM_LINE1(and); break;
		default:    ASM_LINE1(mov); break;
	    }
	    goto done;
	}
    }
    else {
	npts  = dy +  1;
	error = dy >> 1;
	lndata.errsub  = dx;
	lndata.erradd  = dy;
	lndata.offset1 = CURC->gc_lineoffset;
	lndata.offset2 = CURC->gc_lineoffset + xstep;
    }
    switch(C_OPER(color)) {
	case C_XOR: ASM_LINE2(xor); break;
	case C_OR:  ASM_LINE2(or);  break;
	case C_AND: ASM_LINE2(and); break;
	default:    ASM_LINE2(mov); break;
    }
  done:
    GRX_LEAVE();
}
#else
static
#include "fdrivers/generic/line.c"
#endif

static
#include "fdrivers/generic/bitmap.c"

static
#include "fdrivers/generic/pattern.c"

static void bitblt(GrFrame *dst,int dx,int dy,
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
	else
#ifdef FAR_ACCESS
	  _GrFrDrvPackedBitBltV2V_LFB(
#else
	  _GrFrDrvPackedBitBltR2R(
#endif
	    dst,(dx << 1),dy,
	    src,(sx << 1),sy,
	    (w << 1),h,
	    op
	);
	GRX_LEAVE();
}

#ifdef FAR_ACCESS
static void bltv2r(GrFrame *dst,int dx,int dy,GrFrame *src,int sx,int sy,int w,int h,GrColor op)
{
	GRX_ENTER();
	if(GrColorMode(op) == GrIMAGE) _GrFrDrvGenericBitBlt(
	    dst,dx,dy,
	    src,sx,sy,
	    w,h,
	    op
	);
	else _GrFrDrvPackedBitBltV2R_LFB(
	    dst,(dx << 1),dy,
	    src,(sx << 1),sy,
	    (w << 1),h,
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
	else _GrFrDrvPackedBitBltR2V_LFB(
	    dst,(dx << 1),dy,
	    src,(sx << 1),sy,
	    (w << 1),h,
	    op
	);
	GRX_LEAVE();
}
#endif

