/**
 ** driver32.h ---- the 16M color padded SVGA linear frame buffer driver
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
 ** Andrzej Lawa [FidoNet: Andrzej Lawa 2:480/19.77]
 **
 **/

#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"
#include "mempeek.h"
#include "memfill.h"

/* frame offset address calculation */
#define FOFS(x,y,lo) umuladd32((y),(lo),((x)<<2))

/* #define FAR_ACCESS for video access routines */

#ifdef FAR_ACCESS
# define peek32          peek_l_f
# define poke32_xor      poke_l_f_xor
# define poke32_or       poke_l_f_or
# define poke32_and      poke_l_f_and
# define poke32          poke_l_f
# ifdef colfill_l_f
#  define colfill32_xor  colfill_l_f_xor
#  define colfill32_or   colfill_l_f_or
#  define colfill32_and  colfill_l_f_and
#  define colfill32      colfill_l_f
# endif /* defined colfill_l */
# ifdef repfill_l_f
#  define repfill32_xor  repfill_l_f_xor
#  define repfill32_or   repfill_l_f_or
#  define repfill32_and  repfill_l_f_and
#  define repfill32      repfill_l_f
# endif /* defined repfill_l */
# define SETFARSEL(sel)  setup_far_selector(sel)
# if defined(__GNUC__) && defined(__i386__)
#   define ASM_386_SEL  I386_GCC_FAR_SELECTOR
# endif /* GCC i386 */
#else /* defined FAR_ACCESS */
# define peek32          peek_l
# define poke32_xor      poke_l_xor
# define poke32_or       poke_l_or
# define poke32_and      poke_l_and
# define poke32          poke_l
# ifdef colfill_l
#  define colfill32_xor  colfill_l_xor
#  define colfill32_or   colfill_l_or
#  define colfill32_and  colfill_l_and
#  define colfill32      colfill_l
# endif /* defined colfill_l */
# ifdef repfill_l
#  define repfill32_xor  repfill_l_xor
#  define repfill32_or   repfill_l_or
#  define repfill32_and  repfill_l_and
#  define repfill32      repfill_l
# endif /* defined repfill_l */
# define SETFARSEL(sel)
#endif

#ifndef ASM_386_SEL
# define ASM_386_SEL
#endif

static INLINE
GrColor readpixel(GrFrame *c,int x,int y)
{
	char far *pp;
	GRX_ENTER();
#ifdef FAR_ACCESS
	pp = &SCRN->gc_baseaddr[0][FOFS(x,y,SCRN->gc_lineoffset)];
	SETFARSEL(SCRN->gc_selector);
#else
/* problem with LFB_BY_NEAR_POINTER here? Does c always point to screen? */
	pp = &c->gf_baseaddr[0][FOFS(x,y,c->gf_lineoffset)];
#endif
	GRX_RETURN(PIX2COL(peek32(pp)));
}

static INLINE
void drawpixel(int x,int y,GrColor color)
{
	char far *ptr;
	int op;
	GRX_ENTER();
	ptr  = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
	op   = C_OPER(color);
	color= COL2PIX(color);
	SETFARSEL(CURC->gc_selector);
	switch(op) {
	    case C_XOR: poke32_xor(ptr,color); break;
	    case C_OR:  poke32_or( ptr,color); break;
	    case C_AND: poke32_and(ptr,color); break;
	    default:    poke32(    ptr,color); break;
	}
	GRX_LEAVE();
}

#ifdef colfill32
static void drawvline(int x,int y,int h,GrColor color)
{
	unsigned lwdt;
	char far *pp;
	int op;

	GRX_ENTER();
	lwdt = CURC->gc_lineoffset;
	pp   = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
	op   = C_OPER(color);
	color= COL2PIX(color);
	SETFARSEL(CURC->gc_selector);
	switch(op) {
	    case C_XOR: colfill32_xor(pp,lwdt,color,h); break;
	    case C_OR:  colfill32_or( pp,lwdt,color,h); break;
	    case C_AND: colfill32_and(pp,lwdt,color,h); break;
	    default:    colfill32(    pp,lwdt,color,h); break;
	}
	GRX_LEAVE();
}
#else
static
#include "fdrivers/generic/vline.c"
#endif

#ifdef repfill32
static void drawhline(int x,int y,int w,GrColor color)
{
	int op;
	char far *pp;
	GR_repl cval;

	GRX_ENTER();
	pp   = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
	op   = C_OPER(color);
	color= COL2PIX(color);
	cval = freplicate_l(color);
	SETFARSEL(CURC->gc_selector);
	switch(op) {
	    case C_XOR: repfill32_xor(pp,cval,w); break;
	    case C_OR:  repfill32_or( pp,cval,w); break;
	    case C_AND: repfill32_and(pp,cval,w); break;
	    default:    repfill32(    pp,cval,w); break;
	}
	GRX_LEAVE();
}

static void drawblock(int x,int y,int w,int h,GrColor color)
{
	unsigned skip;
	int op;
	char far *pp;
	GR_repl cval;

	GRX_ENTER();
	skip  = CURC->gc_lineoffset;
	pp    = &CURC->gc_baseaddr[0][FOFS(x,y,skip)];
	skip -= w<<2;
	op    = C_OPER(color);
	color = COL2PIX(color);
	cval  = freplicate_l(color);
	SETFARSEL(CURC->gc_selector);
	switch(op) {
	    case C_XOR: do { int ww = w;
			     repfill32_xor(pp,cval,ww);
			     pp += skip;
			} while(--h != 0);
			break;
	    case C_OR:  do { int ww = w;
			     repfill32_or(pp,cval,ww);
			     pp += skip;
			} while(--h != 0);
			break;
	    case C_AND: do { int ww = w;
			     repfill32_and(pp,cval,ww);
			     pp += skip;
			} while(--h != 0);
			break;
	    default:    do { int ww = w;
			     repfill32(pp,cval,ww);
			     pp += skip;
			} while(--h != 0);
			break;
	}
	GRX_LEAVE();
}
#else
static
#include "fdrivers/generic/hline.c"

static
#include "fdrivers/generic/block.c"
#endif


#if defined(__GNUC__) && defined(__i386__)
static void drawline(int x,int y,int dx,int dy,GrColor color)
{
	struct {
	    int errsub;         /* subtract from error term */
	    int erradd;         /* add to error term when carry */
	    int offset1;        /* add to pointer if no carry on error term */
	    int offset2;        /* add to pointer if carry / banking dir */
	} lndata;
	int  op,npts,error,xstep;
	char far *ptr;

	GRX_ENTER();
	op = C_OPER(color);
	color = COL2PIX(color);

#       ifdef __GNUC__
#       ifdef __i386__
#       define ASM_LINE1(OPC) asm volatile(""              \
	"   .align 2,0x90                             \n"  \
	"0: "#OPC"l %6,"ASM_386_SEL"(%0)              \n"  \
	"   subl %7,%2                                \n"  \
	"   jb   1f                                   \n"  \
	"   leal -4(%3),%3                            \n"  \
	"   decl %1                                   \n"  \
	"   jne  0b                                   \n"  \
	"   jmp  2f                                   \n"  \
	"   .align 2,0x90                             \n"  \
	"1: addl 4  + %7,%2                           \n"  \
	"   addl 12 + %7,%3                           \n"  \
	"   decl %1                                   \n"  \
	"   jne  0b                                   \n"  \
	"2: "                                              \
	 :  "=r" (ptr),         "=r" (npts), "=r" (error)  \
	 :  "0"  ((long)(ptr)), "1"  (npts), "2"  (error), \
	    "r"  ((long)(color)),      "o"  (lndata)       \
	)
#       define ASM_LINE2(OPC) asm volatile(""              \
	"   .align 2,0x90                             \n"  \
	"0: "#OPC"l %6,"ASM_386_SEL"(%0)              \n"  \
	"   subl %7,%2                                \n"  \
	"   jb   1f                                   \n"  \
	"   addl 8 + %7,%3                            \n"  \
	"   decl %1                                   \n"  \
	"   jne  0b                                   \n"  \
	"   jmp  2f                                   \n"  \
	"   .align 2,0x90                             \n"  \
	"1: addl 4  + %7,%2                           \n"  \
	"   addl 12 + %7,%3                           \n"  \
	"   decl %1                                   \n"  \
	"   jne  0b                                   \n"  \
	"2: "                                              \
	 :  "=r" (ptr),         "=r" (npts), "=r" (error)  \
	 :  "0"  ((long)(ptr)), "1"  (npts), "2"  (error), \
	    "r"  ((long)(color)),      "o"  (lndata)       \
	)
#       endif
#       endif

	if(dy < 0) {
	    y -= (dy = (-dy));
	    x -= (dx = (-dx));
	}
	if(dx < 0) {
	    xstep = (-4);
	    dx    = (-dx);
	} else
	    xstep = 4;

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
		switch(op) {
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
	switch(op) {
	    case C_XOR: ASM_LINE2(xor); break;
	    case C_OR:  ASM_LINE2(or);  break;
	    case C_AND: ASM_LINE2(and); break;
	    default:    ASM_LINE2(mov); break;
	}
done:   GRX_LEAVE();
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
	    dst,(dx << 2),dy,
	    src,(sx << 2),sy,
	    (w << 2),h,
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
	    dst,(dx << 2),dy,
	    src,(sx << 2),sy,
	    (w << 2),h,
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
	    dst,(dx << 2),dy,
	    src,(sx << 2),sy,
	    (w << 2),h,
	    op
	);
	GRX_LEAVE();
}
#endif /* FAR_ACCESS */

