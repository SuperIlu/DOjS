/**
 ** driver8.h ---- the 256 color padded SVGA linear frame buffer driver
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
#include "allocate.h"
#include "arith.h"
#include "docolor.h"
#include "mempeek.h"
#include "memfill.h"

/* frame offset address calculation */
#define FOFS(x,y,lo) umuladd32((y),(lo),(x))

/* #define FAR_ACCESS for video access routines */

#ifdef FAR_ACCESS
# define peek8          peek_b_f
# define poke8_xor      poke_b_f_xor
# define poke8_or       poke_b_f_or
# define poke8_and      poke_b_f_and
# define poke8          poke_b_f
# define colfill8_xor   colfill_b_f_xor
# define colfill8_or    colfill_b_f_or
# define colfill8_and   colfill_b_f_and
# define colfill8       colfill_b_f
# define repfill8_xor   repfill_b_f_xor
# define repfill8_or    repfill_b_f_or
# define repfill8_and   repfill_b_f_and
# define repfill8       repfill_b_f
# define SETFARSEL(sel) setup_far_selector(sel)
# if defined(__GNUC__) && defined(__i386__)
#   define ASM_386_SEL  I386_GCC_FAR_SELECTOR
# endif /* GCC i386 */
#else /* defined FAR_ACCESS */
# define peek8          peek_b
# define poke8_xor      poke_b_xor
# define poke8_or       poke_b_or
# define poke8_and      poke_b_and
# define poke8          poke_b
# define colfill8_xor   colfill_b_xor
# define colfill8_or    colfill_b_or
# define colfill8_and   colfill_b_and
# define colfill8       colfill_b
# define repfill8_xor   repfill_b_xor
# define repfill8_or    repfill_b_or
# define repfill8_and   repfill_b_and
# define repfill8       repfill_b
# define SETFARSEL(sel)
#endif

#ifndef ASM_386_SEL
# define ASM_386_SEL
#endif


static INLINE
GrColor readpixel(GrFrame *c,int x,int y)
{
    GR_int8u far *pp;
    GRX_ENTER();
#ifdef FAR_ACCESS
    pp = (GR_int8u far *)&SCRN->gc_baseaddr[0][FOFS(x,y,SCRN->gc_lineoffset)];
    SETFARSEL(SCRN->gc_selector);
    GRX_RETURN((GR_int8u)peek8(pp));
#else
/* problem with LFB_BY_NEAR_POINTER here? Does c always point to screen? */
    pp = (GR_int8u far *)&c->gf_baseaddr[0][FOFS(x,y,c->gf_lineoffset)];
    GRX_RETURN(*pp);
#endif
}

/* -------------------------------------------------------------------- */

static INLINE
void drawpixel(int x,int y,GrColor color)
{
    char far *pp;
    GRX_ENTER();
    pp = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    SETFARSEL(CURC->gc_selector);
    switch(C_OPER(color)) {
	case C_XOR: poke8_xor(pp,(GR_int8u)color); break;
	case C_OR:  poke8_or( pp,(GR_int8u)color); break;
	case C_AND: poke8_and(pp,(GR_int8u)color); break;
	default:    poke8(    pp,(GR_int8u)color); break;
    }
    GRX_LEAVE();
}

/* -------------------------------------------------------------------- */

static void drawvline(int x,int y,int h,GrColor color)
{
    int copr;
    GRX_ENTER();
    copr = C_OPER(color);
    if(DOCOLOR8(color,copr)) {
	unsigned lwdt = CURC->gc_lineoffset;
	char far *pp = &CURC->gc_baseaddr[0][FOFS(x,y,lwdt)];
	SETFARSEL(CURC->gc_selector);
	switch(copr) {
	    case C_XOR: colfill8_xor(pp,lwdt,(GR_int8u)color,h); break;
	    case C_OR:  colfill8_or( pp,lwdt,(GR_int8u)color,h); break;
	    case C_AND: colfill8_and(pp,lwdt,(GR_int8u)color,h); break;
	    default:    colfill8(    pp,lwdt,(GR_int8u)color,h); break;
	}
    }
    GRX_LEAVE();
}

/* -------------------------------------------------------------------- */

static void drawhline(int x,int y,int w,GrColor color)
{
    int copr;
    GRX_ENTER();
    copr = C_OPER(color);
    if(DOCOLOR8(color,copr)) {
	GR_repl cval = freplicate_b(color);
	char far *pp = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
	SETFARSEL(CURC->gc_selector);
	switch(copr) {
	    case C_XOR: repfill8_xor(pp,cval,w); break;
	    case C_OR:  repfill8_or( pp,cval,w); break;
	    case C_AND: repfill8_and(pp,cval,w); break;
	    default:    repfill8(    pp,cval,w); break;
	}
    }
    GRX_LEAVE();
}

/* -------------------------------------------------------------------- */

static void drawblock(int x,int y,int w,int h,GrColor color)
{
    int copr;
    GRX_ENTER();
    copr = C_OPER(color);
    if(DOCOLOR8(color,copr)) {
	GR_repl cval = freplicate_b(color);
	unsigned int skip = CURC->gc_lineoffset;
	char far *pp = &CURC->gc_baseaddr[0][FOFS(x,y,skip)];
	skip -= w;
	SETFARSEL(CURC->gc_selector);
	switch(copr) {
	    case C_XOR: do { int ww = w;
			     repfill8_xor(pp,cval,ww);
			     pp += skip;
			} while(--h != 0);
			break;
	    case C_OR:  do { int ww = w;
			     repfill8_or(pp,cval,ww);
			     pp += skip;
			} while(--h != 0);
			break;
	    case C_AND: do { int ww = w;
			     repfill8_and(pp,cval,ww);
			     pp += skip;
			} while(--h != 0);
			break;
	    default:    do { int ww = w;
			     repfill8(pp,cval,ww);
			     pp += skip;
			} while(--h != 0);
			break;
	}
    }
    GRX_LEAVE();
}

/* -------------------------------------------------------------------- */

#if defined(__GNUC__) && defined(__i386__)
static void drawline(int x,int y,int dx,int dy,GrColor color)
{
    struct {
	int errsub;
	int erradd;
	int offset1;
	int offset2;
    } lndata;
    int  copr,xstep,npts,error;
    char far *ptr;

    GRX_ENTER();
    copr = C_OPER(color);
    if(!DOCOLOR8(color,copr)) goto done;

#   ifdef __GNUC__
#   ifdef __i386__
#   define ASM_LINE1(OPC) asm volatile(""                     \
	"   .align 2,0x90                             \n"     \
	"0: "#OPC"b %6,"ASM_386_SEL"(%0)              \n"     \
	"   subl %7,%2                                \n"     \
	"   jb   1f                                   \n"     \
	"   decl %3                                   \n"     \
	"   decl %1                                   \n"     \
	"   jne  0b                                   \n"     \
	"   jmp  2f                                   \n"     \
	"   .align 2,0x90                             \n"     \
	"1: addl 4  + %7,%2                           \n"     \
	"   addl 12 + %7,%3                           \n"     \
	"   decl %1                                   \n"     \
	"   jne  0b                                   \n"     \
	"2: "                                                 \
	 :  "=r" (ptr),         "=r" (npts), "=r" (error)     \
	 :  "0"  ((long)(ptr)), "1"  (npts), "2"  (error),    \
	    "q"  ((char)(color)),      "o"  (lndata)          \
	)
#   define ASM_LINE2(OPC) asm volatile(""                     \
	"   .align 2,0x90                             \n"     \
	"0: "#OPC"b %6,"ASM_386_SEL"(%0)              \n"     \
	"   subl %7,%2                                \n"     \
	"   jb   1f                                   \n"     \
	"   addl 8 + %7,%3                            \n"     \
	"   decl %1                                   \n"     \
	"   jne  0b                                   \n"     \
	"   jmp  2f                                   \n"     \
	"   .align 2,0x90                             \n"     \
	"1: addl 4  + %7,%2                           \n"     \
	"   addl 12 + %7,%3                           \n"     \
	"   decl %1                                   \n"     \
	"   jne  0b                                   \n"     \
	"2: "                                                 \
	 :  "=r" (ptr),         "=r" (npts), "=r" (error)     \
	 :  "0"  ((long)(ptr)), "1"  (npts), "2"  (error),    \
	    "q"  ((char)(color)),      "o"  (lndata)          \
	)
#   endif
#   endif

    if(dy < 0) {
	y -= (dy = (-dy));
	x -= (dx = (-dx));
    }
    if(dx < 0) {
	xstep = (-1);
	dx    = (-dx);
    } else
	xstep = 1;

    ptr  = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    SETFARSEL(CURC->gc_selector);
    if(dx > dy) {
	npts  = dx +  1;
	error = dx >> 1;
	lndata.errsub  = dy;
	lndata.erradd  = dx;
	lndata.offset1 = xstep;
	lndata.offset2 = CURC->gc_lineoffset + xstep;
	if(xstep < 0) {
	  again_nonlinear:
	    lndata.offset1 = 1;
	    switch(copr) {
		case C_XOR: ASM_LINE1(xor); break;
		case C_OR:  ASM_LINE1(or);  break;
		case C_AND: ASM_LINE1(and); break;
		default:    ASM_LINE1(mov); break;
	    }
	    if(npts == 0) goto done;
	    goto again_nonlinear;
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
  again_linear:
    switch(copr) {
	case C_XOR: ASM_LINE2(xor); break;
	case C_OR:  ASM_LINE2(or);  break;
	case C_AND: ASM_LINE2(and); break;
	default:    ASM_LINE2(mov); break;
    }
    if(npts == 0) goto done;
    goto again_linear;
  done:
    GRX_LEAVE();
}
#else
static
#include "fdrivers/generic/line.c"
#endif

/* -------------------------------------------------------------------- */

static void drawbitmap(int x,int y,int w,int h,
		       char far *bmp,int pitch,int start,GrColor fg,GrColor bg)
{
	int fgop, bgop;
	int dofg, dobg;
	GRX_ENTER();
	fgop = C_OPER(fg);
	bgop = C_OPER(bg);
	dofg = DOCOLOR8(fg,fgop);
	dobg = DOCOLOR8(bg,bgop);
	if(dofg | dobg) {
	    long offs = FOFS(x,y,CURC->gc_lineoffset);
	    int  once = dofg && dobg && (fgop == bgop);
	    int  skip = CURC->gc_lineoffset;
	    bmp   += start >> 3;
	    start &= 7;
	    SETFARSEL(CURC->gc_selector);
	    do {
		GR_int8u  far *bp = (GR_int8u far *)bmp;
		GR_int8u  bits    = *bp;
		GR_int8u  mask    = 0x80 >> start;
		GR_int32u w1      = w;
		char far *pp = &CURC->gc_baseaddr[0][offs];
#               define DOBOTH(POKEOP) do {                              \
		    POKEOP(pp,((bits&mask)?(GR_int8u)fg:(GR_int8u)bg)); \
		    if((mask >>= 1) == 0) bits = *++bp,mask = 0x80;     \
		    pp++;                                               \
		} while(--w1 != 0)
#               define DOFGC(POKEOP) do {                               \
		    if((mask & bits) != 0) POKEOP(pp,(GR_int8u)fg);     \
		    if((mask >>= 1)  == 0) bits = *++bp,mask = 0x80;    \
		    pp++;                                               \
		} while(--w1 != 0)
#               define DOBGC(POKEOP) do {                               \
		    if((mask & bits) == 0) POKEOP(pp,(GR_int8u)bg);     \
		    if((mask >>= 1)  == 0) bits = *++bp,mask = 0x80;    \
		    pp++;                                               \
		} while(--w1 != 0)
		if(once) switch(fgop) {
		    case C_XOR: DOBOTH(poke8_xor); break;
		    case C_OR:  DOBOTH(poke8_or);  break;
		    case C_AND: DOBOTH(poke8_and); break;
		    default:    DOBOTH(poke8);     break;
		}
		else {
		    char far *ppsave = pp;
		    GR_int32u w1save = w1;
		    if(dofg) switch(fgop) {
			case C_XOR: DOFGC(poke8_xor); break;
			case C_OR:  DOFGC(poke8_or);  break;
			case C_AND: DOFGC(poke8_and); break;
			default:    DOFGC(poke8);     break;
		    }
		    if(dobg) {
			pp   = ppsave;
			w1   = w1save;
			bp   = (GR_int8u far *)bmp;
			bits = *bp;
			mask = 0x80 >> start;
			switch(bgop) {
			    case C_XOR: DOBGC(poke8_xor); break;
			    case C_OR:  DOBGC(poke8_or);  break;
			    case C_AND: DOBGC(poke8_and); break;
			    default:    DOBGC(poke8);     break;
			}
		    }
		}
		offs += skip;
		bmp  += pitch;
	    } while(--h != 0);
	}
	GRX_LEAVE();
}

/* -------------------------------------------------------------------- */

static
#include "fdrivers/generic/pattern.c"

/* -------------------------------------------------------------------- */

#ifdef FAR_ACCESS
#define bitblit _GrFrDrvPackedBitBltV2V_LFB
#define bltr2v  _GrFrDrvPackedBitBltR2V_LFB
#define bltv2r  _GrFrDrvPackedBitBltV2R_LFB
#else
#define bitblit _GrFrDrvPackedBitBltR2R
#endif

/* -------------------------------------------------------------------- */

static
#include "fdrivers/generic/getiscl.c"

/* -------------------------------------------------------------------- */

static
#include "fdrivers/generic/putscl.c"

/* -------------------------------------------------------------------- */
