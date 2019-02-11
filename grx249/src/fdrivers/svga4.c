/**
 ** svga4.c ---- the 16 color (Super) VGA frame driver
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
#include "mempeek.h"
#include "memcopy.h"
#include "memfill.h"
#include "vgaregs.h"
#include "ioport.h"
#include "highlow.h"

/* frame offset address calculation */
#define FOFS(x,y,lo) umuladd32((y),(int)(lo),((x)>>3))

static GrColor lastcolor;
static int  modereg;
static int  writeops[] = {
    (VGA_FUNC_SET << 8) + VGA_ROT_FN_SEL_REG,      /* C_SET */
    (VGA_FUNC_XOR << 8) + VGA_ROT_FN_SEL_REG,      /* C_XOR */
    (VGA_FUNC_OR  << 8) + VGA_ROT_FN_SEL_REG,      /* C_OR  */
    (VGA_FUNC_AND << 8) + VGA_ROT_FN_SEL_REG       /* C_AND */
};

static size_t LineBytes = 0;
static GR_int8u far *LineBuff[4] = {NULL, NULL, NULL, NULL};

static INLINE
void reginit(void) {
    GRX_ENTER();
    /* set write mode 3 and enable color compare */
    outport_w(VGA_GR_CTRL_PORT,(((8 + 3) << 8) | modereg));
    /* don't care register to 0 */
    outport_w(VGA_GR_CTRL_PORT,((0 << 8) | VGA_COLOR_DONTC_REG));
    /* enable all 4 planes for writing */
    outport_w(VGA_SEQUENCER_PORT,((0x0f << 8) | VGA_WRT_PLANE_ENB_REG));
    /* enable all 4 planes for set/reset */
    outport_w(VGA_GR_CTRL_PORT,((0x0f << 8) | VGA_SET_RESET_ENB_REG));
    lastcolor = (-1L);
    GRX_LEAVE();
}

static int alloc_blit_buffer(void) {
    size_t bytes;
    GR_int8u far *base;

    GRX_ENTER();

    bytes = LineBytes<<2;
    base = (GR_int8u far *)_GrTempBufferAlloc(bytes);
    LineBuff[0] = base;
    LineBuff[1] = base+LineBytes;
    LineBuff[2] = base+LineBytes*2;
    LineBuff[3] = base+LineBytes*3;
    GRX_RETURN(base != NULL);
}

static int init(GrVideoMode *mp)
{
    GRX_ENTER();

    /* save original mode register */
    outport_b(VGA_GR_CTRL_PORT,VGA_MODE_REG);
    modereg = ((inport_b(VGA_GR_CTRL_DATA) & 0xfc) << 8) | VGA_MODE_REG;

    /* set up default register values */
    reginit();

    /* set up LineBuff max. line length for blits */
    LineBytes = sizeof(char) * (((mp->width+7) >> 3)+2);

    GRX_RETURN(TRUE);
}


static INLINE
GrColor readpixel(GrFrame *c,int x,int y)
{
	GR_int32u offs;
	char far *ptr;
	unsigned int mask, pixval;
	GRX_ENTER();
	offs = FOFS(x,y,SCRN->gc_lineoffset);
	ptr  = &SCRN->gc_baseaddr[0][BANKPOS(offs)];
	mask = 0x80 >> (x &= 7);
	CHKBANK(BANKNUM(offs));
	setup_far_selector(SCRN->gc_selector);
	outport_w(VGA_GR_CTRL_PORT,modereg);
	outport_w(VGA_GR_CTRL_PORT,((3 << 8) | VGA_RD_PLANE_SEL_REG));
	pixval = peek_b_f(ptr) & mask;
	outport_b(VGA_GR_CTRL_DATA,2);
	pixval = (peek_b_f(ptr) & mask) | (pixval << 1);
	outport_b(VGA_GR_CTRL_DATA,1);
	pixval = (peek_b_f(ptr) & mask) | (pixval << 1);
	outport_b(VGA_GR_CTRL_DATA,0);
	pixval = (peek_b_f(ptr) & mask) | (pixval << 1);
	outport_w(VGA_GR_CTRL_PORT,(((8 + 3) << 8) | modereg));
	lastcolor = (-1L);
	GRX_RETURN((GrColor)(pixval >> (7 - x)));
}

static INLINE
void drawpixel(int x,int y,GrColor color)
{
	GR_int32u offs;
	char far *ptr;
	GRX_ENTER();
	offs = umul32(y,CURC->gc_lineoffset) + (x >> 3);
	ptr  = &CURC->gc_baseaddr[0][BANKPOS(offs)];
	CHKBANK(BANKNUM(offs));
	setup_far_selector(CURC->gc_selector);
	if(lastcolor != color) {
	    outport_w(VGA_GR_CTRL_PORT,writeops[C_OPER(color) & 3]);
	    outport_w(VGA_GR_CTRL_PORT,((((int)color & 0x0f) << 8) | VGA_SET_RESET_REG));
	    lastcolor = color;
	}
	poke_b_f_and(ptr,(0x80U >> (x & 7)));
	GRX_LEAVE();
}

static void drawhline(int x,int y,int w,GrColor color)
{
	GR_int32u offs;
	GR_int8u lmask, rmask;
	unsigned int w1, w2;
	int oper;
	GRX_ENTER();
	oper  = C_OPER(color);
	offs  = FOFS(x,y,CURC->gc_lineoffset);
	w2    = BANKLFT(offs);
	lmask = 0xff >> (x & 7);
	rmask = 0xff << ((-(w += x)) & 7);
	w  = ((w + 7) >> 3) - (x >> 3);
	w2 = w - (w1 = umin(w,w2));
	if(w == 1) lmask &= rmask;
	setup_far_selector(CURC->gc_selector);
	if(lastcolor != color) {
	    outport_w(VGA_GR_CTRL_PORT,writeops[oper & 3]);
	    outport_w(VGA_GR_CTRL_PORT,((((int)color & 0x0f) << 8) | VGA_SET_RESET_REG));
	    lastcolor = color;
	}
	do {
	    GR_int8u far *pp = (GR_int8u far *)&CURC->gc_baseaddr[0][BANKPOS(offs)];
	    CHKBANK(BANKNUM(offs));
	    offs += w1;
	    if( ((GR_int8u)(~lmask)) ) {
		poke_b_f_and(pp,lmask);
		lmask = 0xff;
		if(--w1 == 0) goto nextbank;
		pp++;
	    }
	    if( ((GR_int8u)(~rmask)) && (w2 == 0)) {
		w1--;
		poke_b_f_and(&pp[w1],rmask);
		if(w1 == 0) break;
	    }
	    if(oper == C_WRITE) repfill_b_f(   pp,(-1),w1);
	    else                rowfill_b_f_or(pp,0xff,w1);
	  nextbank:
	    w1 = w2;
	    w2 = 0;
	} while(w1 != 0);
	GRX_LEAVE();
}

static void drawvline(int x,int y,int h,GrColor color)
{
	unsigned int lwdt, mask;
	GR_int32u offs;
	GRX_ENTER();
	if(lastcolor != color) {
	    outport_w(VGA_GR_CTRL_PORT,writeops[C_OPER(color) & 3]);
	    outport_w(VGA_GR_CTRL_PORT,((((int)color & 0x0f) << 8) | VGA_SET_RESET_REG));
	    lastcolor = color;
	}
	lwdt = CURC->gc_lineoffset;
	offs = FOFS(x,y,lwdt);
	mask = 0x80 >> (x & 7);
	setup_far_selector(CURC->gc_selector);
	do {
	    char far *pp = &CURC->gc_baseaddr[0][BANKPOS(offs)];
	    unsigned h1 = BANKLFT(offs) / lwdt;
	    h -= (h1 = umin(h,umax(h1,1)));
	    CHKBANK(BANKNUM(offs));
	    offs += (h1 * lwdt);
	    colfill_b_f_and(pp,lwdt,mask,h1);
	} while(h != 0);
	GRX_LEAVE();
}

static void drawline(int x,int y,int dx,int dy,GrColor color)
{
	int cnt,err, yoff, mask;
	GR_int32u offs;
	GRX_ENTER();
	if(dx < 0) {
	    x += dx; dx = (-dx);
	    y += dy; dy = (-dy);
	}
	yoff = CURC->gc_lineoffset;
	offs = FOFS(x,y,yoff);
	if(dy < 0) {
	    yoff = (-yoff);
	    dy   = (-dy);
	}
	mask = 0x80 >> (x & 7);
	setup_far_selector(CURC->gc_selector);
	if(lastcolor != color) {
	    outport_w(VGA_GR_CTRL_PORT,writeops[C_OPER(color) & 3]);
	    outport_w(VGA_GR_CTRL_PORT,((((int)color & 0x0f) << 8) | VGA_SET_RESET_REG));
	    lastcolor = color;
	}
	if(dx > dy) {
	    char far *pp = NULL;
	    int newoffs = TRUE;
	    err = (cnt = dx) >> 1;
	    do {
		if (newoffs) {
		  CHKBANK(BANKNUM(offs));
		  pp = &CURC->gc_baseaddr[0][BANKPOS(offs)];
		  newoffs = FALSE;
		}
		poke_b_f_and(pp,mask);
		if((err -= dy) < 0) {
		  err += dx;
		  offs += yoff;
		  newoffs = TRUE;
		}
		if((mask >>= 1) == 0) {
		  mask = 0x80;
		  offs++;
		  newoffs = TRUE;
		}
	    } while(--cnt >= 0);
	}
	else {
	    err = (cnt = dy) >> 1;
	    do {
		CHKBANK(BANKNUM(offs));
		poke_b_f_and(&CURC->gc_baseaddr[0][BANKPOS(offs)],mask);
		if((err -= dx) < 0) {
		    err += dy;
		    if((mask >>= 1) == 0) {
		      mask = 0x80;
		      offs++;
		    }
		}
		offs += yoff;
	    } while(--cnt >= 0);
	}
	GRX_LEAVE();
}


static
#include "fdrivers/generic/block.c"


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
**   for plane = 0 .. 3
**     select plane
**     read data from scanline into LineBuff[plane]
**   if bit alignment between source and destination differs
**     shift LineBuff for destination bit alignment
**   for plane = 0 .. 3
**     select plane
**     write LineBuff[plane] to video memory
**
** ram -> video algorithm:
**   for plane = 0 .. 3
**     calculate 'start of line' pointer (slp)
**     holding the first blitting byte in RAM
**   if bit alignment between RAM and screen differs
**     shift&copy from slp to LineBuff with destination
**     bit alignment. Reload slp with LineBuff
**   for plane = 0 .. 3
**     select plane
**     write slp[plane] to video memory
**
** video -> ram algorithm
**   for plane = 0 .. 3
**     select plane
**     read data from scanline into LineBuff[plane]
**   if bit alignment between screen and RAM differs
**     shift LineBuff for RAM bit alignment
**   for plane = 0 .. 3
**     copy from LineBuff[plane] to RAM
**
** These algorithms save a lot of VGA register port settings
** compared with the old pixel by pixel method. There's one
** minor problem: When writing the scanline intermediate
** pixel colors may occur since not all color bits will
** be updated simultanously. Heavy blitting to video RAM
** may force a little 'snow' effect on screen.
*/

static void get_one(long offs, char far *ptr, int w, GR_int8u far *sl) {
  int bnk;
  int w1, w2;
  GRX_ENTER();
  bnk = BANKNUM(offs);
  w2 = BANKLFT(offs);
  w2 = w - (w1 = umin(w,w2));
  CHKBANK(bnk);
  while (w1-- > 0) {
    *(sl++) = peek_b_f(ptr);
    ++ptr;
  }
  if (w2) {
    SETBANK(bnk+1);
    ptr = SCRN->gc_baseaddr[0];
    while (w2-- > 0) {
      *(sl++) = peek_b_f(ptr);
      ++ptr;
    }
  }
  GRX_LEAVE();
}


#define poke_b_f_rw(p,v) do {           \
  register GR_int8u __v = (v);          \
  (void) (volatile int) peek_b_f(p);    \
  poke_b_f((p),__v);                    \
} while (0)

static void put_one(int op, long offs, GR_int8u far *ptr, int w,
		    GR_int8u lm, GR_int8u rm, GR_int8u far *sl  ) {
  int w1, w2;
  int bnk;
  GRX_ENTER();
  bnk = BANKNUM(offs);
  CHKBANK(bnk);
  if (w==1) lm &= rm;
  if ( ((GR_int8u)(~lm)) ) {
    outport_w(VGA_GR_CTRL_PORT,((lm << 8) | VGA_BIT_MASK_REG));
    poke_b_f_rw(ptr,*sl);
    if (--w == 0) goto done;
    ++ptr;
    ++sl;
    ++offs;
  }
  if ( ((GR_int8u)(~rm)) ) --w;
  if (w) {
    w2 = BANKLFT(offs);
    w2 = w - (w1 = umin(w,w2));
    outport_w(VGA_GR_CTRL_PORT,((0xff << 8) | VGA_BIT_MASK_REG));
    if (op) while (w1-- > 0) { poke_b_f_rw(ptr,*sl);
			       ++sl; ++ptr;         }
    else    while (w1-- > 0) { poke_b_f(ptr, *sl);
			       ++sl; ++ptr;         }
    if (w2) {
      SETBANK(bnk+1);
      ptr = (GR_int8u far *)SCRN->gc_baseaddr[0];
      if (op) while (w2-- > 0) { poke_b_f_rw(ptr,*sl);
				 ++sl; ++ptr;         }
      else    while (w2-- > 0) { poke_b_f(ptr, *sl);
				 ++sl; ++ptr;         }
    }
  }
  if ( ((GR_int8u)(~rm)) ) {
    offs += w;
    CHKBANK(BANKNUM(offs));
    outport_w(VGA_GR_CTRL_PORT,((rm << 8) | VGA_BIT_MASK_REG));
    poke_b_f_rw(ptr,*sl);
  }
done:
  GRX_LEAVE();
}

static void get_scanline(long offs, int w) {
    char far *ptr;
    int plane;
    GRX_ENTER();
    ptr = &SCRN->gc_baseaddr[0][BANKPOS(offs)];
    outport_w(VGA_GR_CTRL_PORT,modereg);
    for (plane = 0; plane < 4; ++plane) {
      outport_w(VGA_GR_CTRL_PORT,((plane << 8) | VGA_RD_PLANE_SEL_REG));
      get_one(offs, ptr, w, LineBuff[plane]);
    }
    outport_w(VGA_GR_CTRL_PORT,(((8 + 3) << 8) | modereg));
    lastcolor = (-1L);
    GRX_LEAVE();
}

extern void _GR_shift_scanline(GR_int8u far **dst,
			       GR_int8u far **src,
			       int ws, int shift, int planes );
#define shift_scanline(dst,src,w,sh) \
    _GR_shift_scanline((dst),(src),(w),(sh),4)

static void put_scanline(GR_int8u **src, long offs, int ws, int wd,
			 GR_int8u lm, GR_int8u rm, int oper) {
  int plane;
  GR_int8u far *ptr;

  GRX_ENTER();
  ptr = (GR_int8u far *)&CURC->gc_baseaddr[0][BANKPOS(offs)];

  /* dump to screen */
  outport_w(VGA_GR_CTRL_PORT,modereg);
  outport_w(VGA_GR_CTRL_PORT,((0 << 8) | VGA_SET_RESET_ENB_REG));
  outport_w(VGA_GR_CTRL_PORT,writeops[(unsigned)oper & 3]);
  for (plane = 0; plane < 4; ++plane) {
    outport_w(VGA_SEQUENCER_PORT,((1<<(plane+8)) | VGA_WRT_PLANE_ENB_REG));
    put_one(oper != C_WRITE, offs, ptr, wd, lm, rm, src[plane]);
  }
  outport_w(VGA_GR_CTRL_PORT,((0xff << 8) | VGA_BIT_MASK_REG));
/*  reginit(); */
  GRX_LEAVE();
}

static
#include "fdrivers/generic/bitblt.c"

static void bltv2v(GrFrame *dst,int dx,int dy,
		   GrFrame *src,int x,int y,int w,int h,
		   GrColor op)
{
    GRX_ENTER();
    if(GrColorMode(op) != GrIMAGE && alloc_blit_buffer()) {
	int shift = ((int)(x&7)) - ((int)(dx&7));
	long SO, DO;
	int      oper= C_OPER(op);
	GR_int8u lm = 0xff >> (dx & 7);
	GR_int8u rm = 0xff << ((-(w + dx)) & 7);
	int      ws = ((x+w+7) >> 3) - (x >> 3);
	int      wd = ((dx+w+7) >> 3) - (dx >> 3);
	int      hh = h;
	setup_far_selector(SCRN->gc_selector);
	if (dy < y) {
	  SO = FOFS(x,y,SCRN->gc_lineoffset);
	  DO = FOFS(dx,dy,SCRN->gc_lineoffset);
	  while (hh-- > 0) {
	    get_scanline(SO, ws);
	    if (shift)
	      shift_scanline(LineBuff,LineBuff,ws,shift);
	    put_scanline(LineBuff, DO, ws, wd, lm, rm, oper);
	    SO += SCRN->gc_lineoffset;
	    DO += SCRN->gc_lineoffset;
	  }
	} else {
	  y += h-1; dy += h-1;
	  SO = FOFS(x,y,SCRN->gc_lineoffset);
	  DO = FOFS(dx,dy,SCRN->gc_lineoffset);
	  while (hh-- > 0) {
	    get_scanline(SO, ws);
	    if (shift)
	      shift_scanline(LineBuff,LineBuff,ws,shift);
	    put_scanline(LineBuff, DO, ws, wd, lm, rm, oper);
	    SO -= SCRN->gc_lineoffset;
	    DO -= SCRN->gc_lineoffset;
	  }
	}
	reginit();
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
	GR_int32u SO, DO;
	GR_int8u lm = 0xff >> (dx & 7);
	GR_int8u rm = 0xff << ((-(w + dx)) & 7);
	int ws = ((x+w+7) >> 3) - (x >> 3);
	int wd = ((dx+w+7) >> 3) - (dx >> 3);
	int hh = h;
	setup_far_selector(SCRN->gc_selector);
	SO = FOFS(x,y,src->gf_lineoffset);
	DO = FOFS(dx,dy,SCRN->gc_lineoffset);
	while (hh-- > 0) {
	    /* load pointer to RAM */
	    int i;
	    GR_int8u far *slp[4];
	    for (i=0; i < 4; ++i)
	      slp[i] = (GR_int8u far *)&src->gf_baseaddr[i][SO];
	    if (shift) {
	      shift_scanline(LineBuff,slp,ws,shift);
	      for (i=0; i < 4; ++i)
		slp[i] = LineBuff[i];
	    }
	    put_scanline(slp, DO, ws, wd, lm, rm, oper);
	    SO += src->gf_lineoffset;
	    DO += SCRN->gc_lineoffset;
	}
	reginit();
    } else
	_GrFrDrvGenericBitBlt(dst,dx,dy,src,x,y,w,h,op);
    GRX_LEAVE();
}

#define mcopy(d,op,s,msk) do {                     \
  unsigned char _c_ = *(d);                        \
  *(d) = (_c_ & ~(msk)) | ((_c_ op *(s)) & (msk)); \
} while (0)

static void bltv2r(GrFrame *dst,int dx,int dy,
		   GrFrame *src,int x,int y,int w,int h,
		   GrColor op)
{
    GRX_ENTER();
    while(GrColorMode(op) != GrIMAGE)
      if(alloc_blit_buffer()) {
	int oper  = C_OPER(op);
	int shift = ((int)(x&7)) - ((int)(dx&7));
	GR_int32u SO, DO;
	GR_int8u lm = 0xff >> (dx & 7);
	GR_int8u rm = 0xff << ((-(w + dx)) & 7);
	int ws = ((x+w+7) >> 3) - (x >> 3);
	int wd = ((dx+w+7) >> 3) - (dx >> 3);
	if (wd==1) break;
	setup_far_selector(SCRN->gc_selector);
	SO = FOFS(x,y,SCRN->gc_lineoffset);
	DO = FOFS(dx,dy,dst->gf_lineoffset);
	while (h-- > 0) {
	  int pl;
	  get_scanline(SO, ws);
	  if (shift)
	    shift_scanline(LineBuff,LineBuff,ws,shift);
	  for (pl = 0; pl < 4; ++pl) {
	    GR_int8u far *sptr = LineBuff[pl];
	    GR_int8u far *dptr = (GR_int8u far *)&dst->gf_baseaddr[pl][DO];
	    int ww = wd;
	    if ( ((GR_int8u)(~lm)) ) {
	      switch(oper) {
		case C_XOR: mcopy(dptr,^,sptr,lm); break;
		case C_OR:  mcopy(dptr,|,sptr,lm); break;
		case C_AND: mcopy(dptr,&,sptr,lm); break;
		default:    *dptr = (*dptr & ~lm) | (*sptr & lm);
			    break;
	      }
	      if (!(--ww)) continue;
	      ++dptr; ++sptr;
	    }
	    if ( ((GR_int8u)(~rm)) ) --ww;
	    if (ww) switch(oper) {
		case C_XOR: fwdcopy_xor(dptr,dptr,sptr,ww); break;
		case C_OR:  fwdcopy_or( dptr,dptr,sptr,ww); break;
		case C_AND: fwdcopy_and(dptr,dptr,sptr,ww); break;
		default:    fwdcopy_set(dptr,dptr,sptr,ww); break;
	    }
	    if ( ((GR_int8u)(~rm)) ) {
	      switch(oper) {
		case C_XOR: mcopy(dptr,^,sptr,rm); break;
		case C_OR:  mcopy(dptr,|,sptr,rm); break;
		case C_AND: mcopy(dptr,&,sptr,rm); break;
		default:    *dptr = (*dptr & ~rm) | (*sptr & rm); break;
	      }
	    }
	  }
	  SO += SCRN->gc_lineoffset;
	  DO += dst->gf_lineoffset;
	}
	goto done;
      }
    _GrFrDrvGenericBitBlt(dst,dx,dy,src,x,y,w,h,op);
done:
    GRX_LEAVE();
}


GrFrameDriver _GrFrameDriverSVGA4 = {
    GR_frameSVGA4,              /* frame mode */
    GR_frameRAM4,               /* compatible RAM frame mode */
    TRUE,                       /* onscreen */
    4,                          /* scan line width alignment */
    4,                          /* number of planes */
    4,                          /* bits per pixel */
    16*1024L*1024L,             /* max plane size the code can handle */
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
