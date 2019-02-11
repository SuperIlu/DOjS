/**
 ** speedtst.c ---- check all available frame drivers speed
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
 **
 ** This is a test/demo file of the GRX graphics library.
 ** You can use GRX test/demo files as you want.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** 070512 M.Alvarez, new version more accurate, but still had problems
 **                   in X11, because functions returns before the paint
 **                   is done.
 **/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#ifdef __WATCOMC__
/*#include <wcdefs.h>*/
#include <conio.h>
#else
#include <values.h>
#endif
#include <math.h>
#include "rand.h"

#include "grx20.h"

#if GRX_VERSION_API-0 <= 0x0220
#define GrColor unsigned long
#define BLIT_FAIL(gp) ((gp)->fm!=GR_frameVGA8X)
#else
#define BLIT_FAIL(gp)  0
#endif

#define MEASURE_RAM_MODES 1

#define READPIX_loops      (384*1)
#define READPIX_X11_loops  (4*1)
#define DRAWPIX_loops      (256*1)
#define DRAWLIN_loops      (12*1)
#define DRAWHLIN_loops     (16*1)
#define DRAWVLIN_loops     (12*1)
#define DRAWBLK_loops      (1*1)
#define BLIT_loops         (1*1)

typedef struct {
    double rate, count;
} perfm;

typedef struct {
    GrFrameMode fm;
    int    w,h,bpp;
    int    flags;
    perfm  readpix;
    perfm  drawpix;
    perfm  drawlin;
    perfm  drawhlin;
    perfm  drawvlin;
    perfm  drawblk;
    perfm  blitv2v;
    perfm  blitv2r;
    perfm  blitr2v;
} gvmode;
#define FLG_measured 0x0001
#define FLG_tagged   0x0002
#define FLG_rammode  0x0004
#define MEASURED(g) (((g)->flags&FLG_measured)!=0)
#define TAGGED(g)   (((g)->flags&FLG_tagged)!=0)
#define RAMMODE(g)  (((g)->flags&FLG_rammode)!=0)
#define SET_MEASURED(g)  (g)->flags |= FLG_measured
#define SET_TAGGED(g)    (g)->flags |= FLG_tagged
#define SET_RAMMODE(g)   (g)->flags |= FLG_rammode
#define TOGGLE_TAGGED(g) (g)->flags ^= FLG_tagged

int  nmodes = 0;
#define MAX_MODES 256
gvmode *grmodes = NULL;
#if MEASURE_RAM_MODES
gvmode *rammodes = NULL;
#endif

/* No of Points [(x,y) pairs]. Must be multiple of 2*3=6 */
#define PAIRS 4200

#define UL(x)  ((unsigned long)(x))
#define DBL(x)  ((double)(x))
#define INT(x) ((int)(x))

#ifndef min
#define min(a,b) ((a)<(b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a)>(b) ? (a) : (b))
#endif

typedef struct XYpairs {
  int x[PAIRS];
  int y[PAIRS];
  int w, h;
  struct XYpairs *nxt;
} XY_PAIRS;

XY_PAIRS *xyp = NULL;
int *xb = NULL, *yb = NULL; /* need sorted pairs for block operations */
int measured_any = 0;

XY_PAIRS *checkpairs(int w, int h) {
  XY_PAIRS *res = xyp;
  int i;

  if (xb == NULL) {
    xb = malloc(sizeof(int) * PAIRS);
    yb = malloc(sizeof(int) * PAIRS);
  }

  while (res != NULL) {
    if (res->w == w && res->h == h)
      return res;
    res = res->nxt;
  }

  SRND(12345);

  res = malloc(sizeof(XY_PAIRS));
  assert(res != NULL);
  res->w = w;
  res->h = h;
  res->nxt = xyp;
  xyp = res;
  for (i=0; i < PAIRS; ++i) {
    int x = RND() % w;
    int y = RND() % h;
    if (x < 0) x = 0; else
    if (x >=w) x = w-1;
    if (y < 0) y = 0; else
    if (y >=h) y = h-1;
    res->x[i] = x;
    res->y[i] = y;
  }
  return res;
}

double SQR(int a, int b) {
  double r = DBL(a-b);
  return r*r;
}

double ABS(int a, int b) {
  double r = DBL(a-b);
  return fabs(r);
}

char *FrameDriverName(GrFrameMode m) {

#if GRX_VERSION_API-0 >= 0x0229				     
  unsigned sys = GrGetLibrarySystem();
#else
  unsigned sys = (unsigned) GRX_VERSION;
#endif

  int x11 = ( (sys == GRX_VERSION_GENERIC_X11) ||
              (sys == GRX_VERSION_GCC_386_X11) ||
              (sys == GRX_VERSION_GCC_X86_64_X11) );
  int w32 = ( (sys == GRX_VERSION_GCC_386_WIN32) ||
              (sys == GRX_VERSION_MSC_386_WIN32) ||
	      (sys == GRX_VERSION_GCC_386_CYG32) );
  int sdl = strcmp( GrCurrentVideoDriver()->name , "sdl") == 0;

  switch(m) {
    case GR_frameUndef: return "Undef";
    case GR_frameText : return "Text";
    case GR_frameHERC1: return "HERC1";
    case GR_frameEGAVGA1: return x11 ? "XWIN1" : w32 ? "WIN32_1" : "EGAVGA1";
    case GR_frameEGA4: return "EGA4";
    case GR_frameSVGA4: return x11 ? "XWIN4" : w32 ? "WIN32_4" : "SVGA4";
    case GR_frameSVGA8: return sdl ? "SDL8" : x11 ? "XWIN8" : w32 ? "WIN32_8" : "SVGA8";
    case GR_frameVGA8X: return "VGA8X";
    case GR_frameSVGA16: return sdl ? "SDL16" : x11 ? "XWIN16" : w32 ? "WIN32_16" : "SVGA16";
    case GR_frameSVGA24: return sdl ? "SDL24" : x11 ? "XWIN24" : w32 ? "WIN32_24" : "SVGA24";
    case GR_frameSVGA32L: return sdl ? "SDL32L" : x11 ? "XWIN32L" : w32 ? "WIN32_32L" : "SVGA32L";
    case GR_frameSVGA32H: return sdl ? "SDL32H" : x11 ? "XWIN32H" : w32 ? "WIN32_32H" : "SVGA32H";
    case GR_frameSVGA8_LFB: return "LFB8";
    case GR_frameSVGA16_LFB: return "LFB16";
    case GR_frameSVGA24_LFB: return "LFB24";
    case GR_frameSVGA32L_LFB: return "LFB32L";
    case GR_frameSVGA32H_LFB: return "LFB32H";
    case GR_frameRAM1: return "RAM1";
    case GR_frameRAM4: return "RAM4";
    case GR_frameRAM8: return "RAM8";
    case GR_frameRAM16: return "RAM16";
    case GR_frameRAM24: return "RAM24";
    case GR_frameRAM32L: return "RAM32L";
    case GR_frameRAM32H: return "RAM32H";
    case GR_frameRAM3x8: return "RAM3x8";
    default: return "UNKNOWN";
  }
  return "UNKNOWN";
}

void Message(int disp, char *txt, gvmode *gp) {
  char msg[200];
  sprintf(msg, "%s: %d x %d x %dbpp",
		FrameDriverName(gp->fm), gp->w, gp->h, gp->bpp);
#if GRX_VERSION_API-0 >= 0x0229				     
  unsigned sys = GrGetLibrarySystem();
#else
  unsigned sys = (unsigned) GRX_VERSION;
#endif
  if ( (sys == GRX_VERSION_GENERIC_X11) ||
       (sys == GRX_VERSION_GCC_386_X11) ||
       (sys == GRX_VERSION_GCC_X86_64_X11) )
    fprintf(stderr,"%s\t%s\n", msg, txt);
  if (disp) {
    GrTextOption to;
    GrContext save;
    GrSaveContext(&save);
    GrSetContext(NULL);
    to.txo_font = &GrFont_PC6x8;
    to.txo_fgcolor.v = GrWhite();
    to.txo_bgcolor.v = GrBlack();
    to.txo_chrtype = GR_BYTE_TEXT;
    to.txo_direct  = GR_TEXT_RIGHT;
    to.txo_xalign  = GR_ALIGN_LEFT;
    to.txo_yalign  = GR_ALIGN_TOP;
    GrDrawString(msg,strlen(msg),0,0,&to);
    GrDrawString(txt,strlen(txt),0,10,&to);
    GrSetContext(&save);
  }
}

void printresultheader(FILE *f) {
  fprintf(f, "Driver               readp drawp line   hline vline  block  v2v    v2r    r2v\n");
}

void printresultline(FILE *f, gvmode * gp) {
  fprintf(f, "%-9s %4dx%4d ", FrameDriverName(gp->fm), gp->w, gp->h);
  fprintf(f, "%6.2f", gp->readpix.rate  / (1024.0 * 1024.0));
  fprintf(f, "%6.2f", gp->drawpix.rate  / (1024.0 * 1024.0));
  fprintf(f, "%6.2f", gp->drawlin.rate  / (1024.0 * 1024.0));
  fprintf(f, "%7.2f", gp->drawhlin.rate / (1024.0 * 1024.0));
  fprintf(f, "%6.2f", gp->drawvlin.rate / (1024.0 * 1024.0));
  fprintf(f, "%7.2f", gp->drawblk.rate  / (1024.0 * 1024.0));
  fprintf(f, "%7.2f", gp->blitv2v.rate  / (1024.0 * 1024.0));
  fprintf(f, "%7.2f", gp->blitv2r.rate  / (1024.0 * 1024.0));
  fprintf(f, "%7.2f", gp->blitr2v.rate  / (1024.0 * 1024.0));
  fprintf(f, "\n");
}

void readpixeltest(gvmode *gp, XY_PAIRS *pairs,int loops) {
  int i, j;
  long t1,t2;
  double seconds;
  int *x = pairs->x;
  int *y = pairs->y;

  if (!MEASURED(gp)) {
    gp->readpix.rate  = 0.0;
    gp->readpix.count = DBL(PAIRS) * DBL(loops);
  }

  t1 = GrMsecTime();
  for (i=loops; i > 0; --i) {
    for (j=PAIRS-1; j >= 0; j--)
       GrPixelNC(x[j],y[j]);
  }
  t2 = GrMsecTime();
  seconds = (double)(t2 - t1) / 1000.0;
  if (seconds > 0)
    gp->readpix.rate = gp->readpix.count / seconds;
}

void drawpixeltest(gvmode *gp, XY_PAIRS *pairs) {
  int i, j;
  GrColor c1 = GrWhite();
  GrColor c2 = GrWhite() | GrXOR;
  GrColor c3 = GrWhite() | GrOR;
  GrColor c4 = GrBlack() | GrAND;
  long t1,t2;
  double seconds;
  int *x = pairs->x;
  int *y = pairs->y;

  if (!MEASURED(gp)) {
    gp->drawpix.rate  = 0.0;
    gp->drawpix.count = DBL(PAIRS) * DBL(DRAWPIX_loops) * 4.0;
  }

  t1 = GrMsecTime();
  for (i=0; i < DRAWPIX_loops; ++i) {
    for (j=PAIRS-1; j >= 0; j--) GrPlotNC(x[j],y[j],c1);
    for (j=PAIRS-1; j >= 0; j--) GrPlotNC(x[j],y[j],c2);
    for (j=PAIRS-1; j >= 0; j--) GrPlotNC(x[j],y[j],c3);
    for (j=PAIRS-1; j >= 0; j--) GrPlotNC(x[j],y[j],c4);
  }
  t2 = GrMsecTime();
  seconds = (double)(t2 - t1) / 1000.0;
  if (seconds > 0)
    gp->drawpix.rate = gp->drawpix.count / seconds;
}

void drawlinetest(gvmode *gp, XY_PAIRS *pairs) {
  int i, j;
  int *x = pairs->x;
  int *y = pairs->y;
  GrColor c1 = GrWhite();
  GrColor c2 = GrWhite() | GrXOR;
  GrColor c3 = GrWhite() | GrOR;
  GrColor c4 = GrBlack() | GrAND;
  long t1,t2;
  double seconds;

  if (!MEASURED(gp)) {
    gp->drawlin.rate  = 0.0;
    gp->drawlin.count = 0.0;
    for (j=0; j < PAIRS; j+=2)
      gp->drawlin.count += sqrt(SQR(x[j],x[j+1])+SQR(y[j],y[j+1]));
    gp->drawlin.count *= 4.0 * DRAWLIN_loops;
  }

  t1 = GrMsecTime();
  for (i=0; i < DRAWLIN_loops; ++i) {
    for (j=PAIRS-2; j >= 0; j-=2)
	GrLineNC(x[j],y[j],x[j+1],y[j+1],c1);
    for (j=PAIRS-2; j >= 0; j-=2)
	GrLineNC(x[j],y[j],x[j+1],y[j+1],c2);
    for (j=PAIRS-2; j >= 0; j-=2)
	GrLineNC(x[j],y[j],x[j+1],y[j+1],c3);
    for (j=PAIRS-2; j >= 0; j-=2)
	GrLineNC(x[j],y[j],x[j+1],y[j+1],c4);
  }
  t2 = GrMsecTime();
  seconds = (double)(t2 - t1) / 1000.0;
  if (seconds > 0)
    gp->drawlin.rate = gp->drawlin.count / seconds;
}

void drawhlinetest(gvmode *gp, XY_PAIRS *pairs) {
  int  i, j;
  int *x = pairs->x;
  int *y = pairs->y;
  GrColor c1 = GrWhite();
  GrColor c2 = GrWhite() | GrXOR;
  GrColor c3 = GrWhite() | GrOR;
  GrColor c4 = GrBlack() | GrAND;
  long t1,t2;
  double seconds;

  if (!MEASURED(gp)) {
    gp->drawhlin.rate = 0.0;
    gp->drawhlin.count = 0.0;
    for (j=0; j < PAIRS; j+=2)
      gp->drawhlin.count += ABS(x[j],x[j+1]);
    gp->drawhlin.count *= 4.0 * DRAWHLIN_loops;
  }

  t1 = GrMsecTime();
  for (i=0; i < DRAWHLIN_loops; ++i) {
    for (j=PAIRS-2; j >= 0; j-=2)
      GrHLineNC(x[j],x[j+1],y[j],c1);
    for (j=PAIRS-2; j >= 0; j-=2)
      GrHLineNC(x[j],x[j+1],y[j],c2);
    for (j=PAIRS-2; j >= 0; j-=2)
      GrHLineNC(x[j],x[j+1],y[j],c3);
    for (j=PAIRS-2; j >= 0; j-=2)
      GrHLineNC(x[j],x[j+1],y[j],c4);
  }
  t2 = GrMsecTime();
  seconds = (double)(t2 - t1) / 1000.0;
  if (seconds > 0)
    gp->drawhlin.rate = gp->drawhlin.count / seconds;
}

void drawvlinetest(gvmode *gp, XY_PAIRS *pairs) {
  int i, j;
  int *x = pairs->x;
  int *y = pairs->y;
  GrColor c1 = GrWhite();
  GrColor c2 = GrWhite() | GrXOR;
  GrColor c3 = GrWhite() | GrOR;
  GrColor c4 = GrBlack() | GrAND;
  long t1,t2;
  double seconds;

  if (!MEASURED(gp)) {
    gp->drawvlin.rate = 0.0;
    gp->drawvlin.count = 0.0;
    for (j=0; j < PAIRS; j+=2)
      gp->drawvlin.count += ABS(y[j],y[j+1]);
    gp->drawvlin.count *= 4.0 * DRAWVLIN_loops;
  }

  t1 = GrMsecTime();
  for (i=0; i < DRAWVLIN_loops; ++i) {
    for (j=PAIRS-2; j >= 0; j-=2)
       GrVLineNC(x[j],y[j],y[j+1],c1);
    for (j=PAIRS-2; j >= 0; j-=2)
       GrVLineNC(x[j],y[j],y[j+1],c2);
    for (j=PAIRS-2; j >= 0; j-=2)
       GrVLineNC(x[j],y[j],y[j+1],c3);
    for (j=PAIRS-2; j >= 0; j-=2)
       GrVLineNC(x[j],y[j],y[j+1],c4);
  }
  t2 = GrMsecTime();
  seconds = (double)(t2 - t1) / 1000.0;
  if (seconds > 0)
    gp->drawvlin.rate = gp->drawvlin.count / seconds;
}

void drawblocktest(gvmode *gp, XY_PAIRS *pairs) {
  int i, j;
  GrColor c1 = GrWhite();
  GrColor c2 = GrWhite() | GrXOR;
  GrColor c3 = GrWhite() | GrOR;
  GrColor c4 = GrBlack() | GrAND;
  long t1,t2;
  double seconds;

  if (xb == NULL || yb == NULL) return;

  for (j=0; j < PAIRS; j+=2) {
    xb[j]   = min(pairs->x[j],pairs->x[j+1]);
    xb[j+1] = max(pairs->x[j],pairs->x[j+1]);
    yb[j]   = min(pairs->y[j],pairs->y[j+1]);
    yb[j+1] = max(pairs->y[j],pairs->y[j+1]);
  }

  if (!MEASURED(gp)) {
    gp->drawblk.rate = 0.0;
    gp->drawblk.count = 0.0;
    for (j=0; j < PAIRS; j+=2)
      gp->drawblk.count += ABS(xb[j],xb[j+1]) * ABS(yb[j],yb[j+1]);
    gp->drawblk.count *= 4.0 * DRAWBLK_loops;
  }

  t1 = GrMsecTime();
  for (i=0; i < DRAWBLK_loops; ++i) {
    for (j=PAIRS-2; j >= 0; j-=2)
      GrFilledBoxNC(xb[j],yb[j],xb[j+1],yb[j+1],c1);
    for (j=PAIRS-2; j >= 0; j-=2)
      GrFilledBoxNC(xb[j],yb[j],xb[j+1],yb[j+1],c2);
    for (j=PAIRS-2; j >= 0; j-=2)
      GrFilledBoxNC(xb[j],yb[j],xb[j+1],yb[j+1],c3);
    for (j=PAIRS-2; j >= 0; j-=2)
      GrFilledBoxNC(xb[j],yb[j],xb[j+1],yb[j+1],c4);
  }
  t2 = GrMsecTime();
  seconds = (double)(t2 - t1) / 1000.0;
  if (seconds > 0)
    gp->drawblk.rate = gp->drawblk.count / seconds;
}

void xor_draw_blocks(GrContext *c) {
  GrContext save;
  int i;

  GrSaveContext(&save);
  GrSetContext(c);
  GrClearContext(GrBlack());
  for (i=28; i > 1; --i)
    GrFilledBox(GrMaxX()/i,GrMaxY()/i,
		(i-1)*GrMaxX()/i,(i-1)*GrMaxY()/i,GrWhite()|GrXOR);
  GrSetContext(&save);
}

void blit_measure(gvmode *gp, perfm *p,
		  int *xb, int *yb,
		  GrContext *dst,GrContext *src) {
  int i, j;
  long t1,t2;
  double seconds;
  GrContext save;

  GrSaveContext(&save);
  if (dst != src) {
    GrSetContext(dst);
    GrClearContext(GrBlack());
  }
  xor_draw_blocks(src);
  GrSetContext(&save);

  if (dst != NULL) {
    char *s = src != NULL ? "ram" : "video";
    char *d = dst != NULL ? "ram" : "video";
    char txt[50];
    sprintf(txt, "blit test: %s -> %s", s, d);
    Message(1,txt, gp);
  }

  t1 = GrMsecTime();
  for (i=0; i < BLIT_loops; ++i) {
    for (j=PAIRS-3; j >= 0; j-=3)
      GrBitBlt(dst,xb[j+2],yb[j+2],src,xb[j+1],yb[j+1],xb[j],yb[j],GrWRITE);
    for (j=PAIRS-3; j >= 0; j-=3)
      GrBitBlt(dst,xb[j+2],yb[j+2],src,xb[j+1],yb[j+1],xb[j],yb[j],GrXOR);
    for (j=PAIRS-3; j >= 0; j-=3)
      GrBitBlt(dst,xb[j+2],yb[j+2],src,xb[j+1],yb[j+1],xb[j],yb[j],GrOR);
    for (j=PAIRS-3; j >= 0; j-=3)
      GrBitBlt(dst,xb[j+2],yb[j+2],src,xb[j+1],yb[j+1],xb[j],yb[j],GrAND);
  }
  t2 = GrMsecTime();
  seconds = (double)(t2 - t1) / 1000.0;
  if (seconds > 0)
    p->rate = p->count / seconds;
}

void blittest(gvmode *gp, XY_PAIRS *pairs, int ram) {
  int j;

  if (xb == NULL || yb == NULL) return;

  for (j=0; j < PAIRS; j+=3) {
    int wh;
    xb[j]   = max(pairs->x[j],pairs->x[j+1]);
    xb[j+1] = min(pairs->x[j],pairs->x[j+1]);
    xb[j+2] = pairs->x[j+2];
    wh      = xb[j]-xb[j+1];
    if (xb[j+2]+wh >= gp->w) xb[j+2] = gp->w - wh - 1;
    yb[j]   = max(pairs->y[j],pairs->y[j+1]);
    yb[j+1] = min(pairs->y[j],pairs->y[j+1]);
    yb[j+2] = pairs->y[j+2];
    wh      = yb[j]-yb[j+1];
    if (yb[j+2]+wh >= gp->h) yb[j+2] = gp->h - wh - 1;
  }

  if (!MEASURED(gp)) {
    double count = 0.0;
    for (j=0; j < PAIRS; j+=3)
      count += ABS(xb[j],xb[j+1]) * ABS(yb[j],yb[j+1]);
    gp->blitv2v.count =
    gp->blitr2v.count =
    gp->blitv2r.count = count * 4.0 * BLIT_loops;
    gp->blitv2v.rate  =
    gp->blitr2v.rate  =
    gp->blitv2r.rate  = 0.0;
  }

#if BLIT_loops-0
  blit_measure(gp, &gp->blitv2v, xb, yb,
	       (GrContext *)(RAMMODE(gp) ? GrCurrentContext() : NULL),
	       (GrContext *)(RAMMODE(gp) ? GrCurrentContext() : NULL));
  if (!BLIT_FAIL(gp) && !ram) {
    GrContext rc;
    GrContext *rcp = GrCreateContext(gp->w,gp->h,NULL,&rc);
    if (rcp) {
      blit_measure(gp, &gp->blitv2r, xb, yb, rcp, NULL);
      blit_measure(gp, &gp->blitr2v, xb, yb, NULL, rcp);
      GrDestroyContext(rcp);
    }
  }
#endif
}

void measure_one(gvmode *gp, int ram) {
  XY_PAIRS *pairs;

  if (MEASURED(gp)) return;
  pairs = checkpairs(gp->w, gp->h);
  GrFilledBox( 0, 0, gp->w-1, gp->h-1, GrBlack());
  Message(RAMMODE(gp),"read pixel test", gp);
  { int rd_loops = READPIX_loops;
#if GRX_VERSION_API-0 >= 0x0229				     
  unsigned sys = GrGetLibrarySystem();
#else
  unsigned sys = (unsigned) GRX_VERSION;
#endif
  if ( (sys == GRX_VERSION_GENERIC_X11) ||
       (sys == GRX_VERSION_GCC_386_X11) ||
       (sys == GRX_VERSION_GCC_X86_64_X11) )
      if (!RAMMODE(gp)) rd_loops = READPIX_X11_loops;
    readpixeltest(gp,pairs,rd_loops);
  }
  GrFilledBox( 0, 0, gp->w-1, gp->h-1, GrBlack());
  Message(RAMMODE(gp),"draw pixel test", gp);
  drawpixeltest(gp,pairs);
  GrFilledBox( 0, 0, gp->w-1, gp->h-1, GrBlack());
  Message(RAMMODE(gp),"draw line test ", gp);
  drawlinetest(gp,pairs);
  GrFilledBox( 0, 0, gp->w-1, gp->h-1, GrBlack());
  Message(RAMMODE(gp),"draw hline test", gp);
  drawhlinetest(gp,pairs);
  GrFilledBox( 0, 0, gp->w-1, gp->h-1, GrBlack());
  Message(RAMMODE(gp),"draw vline test", gp);
  drawvlinetest(gp,pairs);
  GrFilledBox( 0, 0, gp->w-1, gp->h-1, GrBlack());
  Message(RAMMODE(gp),"draw block test", gp);
  drawblocktest(gp,pairs);
  GrFilledBox( 0, 0, gp->w-1, gp->h-1, GrBlack());
  blittest(gp, pairs, ram);
  GrFilledBox( 0, 0, gp->w-1, gp->h-1, GrBlack());
  SET_MEASURED(gp);
  measured_any = 1;
}

#if MEASURE_RAM_MODES
int identical_measured(gvmode *tm) {
  int i;
  for (i=0; i < nmodes; ++i) {
    if (tm      != &rammodes[i]    &&
	tm->fm  == rammodes[i].fm  &&
	tm->w   == rammodes[i].w   &&
	tm->h   == rammodes[i].h   &&
	tm->bpp == rammodes[i].bpp &&
	MEASURED(&rammodes[i])        ) return (1);
  }
  return 0;
}
#endif

static int first = 0;

void speedcheck(gvmode *gp, int print, int wait) {
  char m[41];
  gvmode *rp = NULL;

  if (first) {
    printf(
      "speedtest may take some time to process.\n"
      "Now press <CR> to continue..."
    );
    fflush(stdout);
    fgets(m,40,stdin);
  }

  GrSetMode(
      GR_width_height_bpp_graphics,
      gp->w, gp->h, gp->bpp
  );

  if (first) {
    /* xor_draw_blocks(NULL);
       getch(); */
    first = 0;
  }

  if ( GrScreenFrameMode() != gp->fm) {
    GrFrameMode act = GrScreenFrameMode();
    GrSetMode(GR_default_text);
    printf("Setup failed : %s != %s\n",
    FrameDriverName(act),
    FrameDriverName(gp->fm));
    fgets(m,40,stdin);
    return;
  }

  if (!MEASURED(gp))
    measure_one(gp, 0);

#if MEASURE_RAM_MODES
  rp = &rammodes[(unsigned)(gp-grmodes)];
  rp->fm = GrCoreFrameMode();
  if (!MEASURED(rp) && !identical_measured(rp)) {
    GrContext rc;
    if (GrCreateFrameContext(rp->fm,gp->w,gp->h,NULL,&rc)) {
      GrSetContext(&rc);
      measure_one(rp, 1);
      GrDestroyContext(&rc);
      GrSetContext(NULL);
    }
  }
#endif

  GrSetMode(GR_default_text);
  if (print) {
    printf("Results: \n");
    printresultheader(stdout);
    printresultline(stdout, gp);
    if (rp)
      printresultline(stdout, rp);
  }
  if (wait) 
    fgets(m,40,stdin);
}

int collectmodes(const GrVideoDriver *drv)
{
	gvmode *gp = grmodes;
	GrFrameMode fm;
	const GrVideoMode *mp;
	for(fm =GR_firstGraphicsFrameMode;
	      fm <= GR_lastGraphicsFrameMode; fm++) {
	    for(mp = GrFirstVideoMode(fm); mp; mp = GrNextVideoMode(mp)) {
		gp->fm    = fm;
		gp->w     = mp->width;
		gp->h     = mp->height;
		gp->bpp   = mp->bpp;
		gp->flags = 0;
		gp++;
		if (gp-grmodes >= MAX_MODES) return MAX_MODES;
	    }
	}
	return(int)(gp-grmodes);
}

int vmcmp(const void *m1,const void *m2)
{
	gvmode *md1 = (gvmode *)m1;
	gvmode *md2 = (gvmode *)m2;
	if(md1->bpp != md2->bpp) return(md1->bpp - md2->bpp);
	if(md1->w   != md2->w  ) return(md1->w   - md2->w  );
	if(md1->h   != md2->h  ) return(md1->h   - md2->h  );
	return(0);
}

#define LINES   20
#define COLUMNS 80

void ModeText(int i, int shrt,char *mdtxt) {
	char *flg;

	if (MEASURED(&grmodes[i])) flg = " #"; else
	if (TAGGED(&grmodes[i]))   flg = " *"; else
				   flg = ") ";
	switch (shrt) {
	  case 2 : sprintf(mdtxt,"%2d%s %dx%d ", i+1, flg, grmodes[i].w, grmodes[i].h);
		   break;
	  case 1 : sprintf(mdtxt,"%2d%s %4dx%-4d ", i+1, flg, grmodes[i].w, grmodes[i].h);
		   break;
	  default: sprintf(mdtxt,"  %2d%s  %4dx%-4d ", i+1, flg, grmodes[i].w, grmodes[i].h);
		   break;
	}
	mdtxt += strlen(mdtxt);

	if (grmodes[i].bpp > 20)
	  sprintf(mdtxt, "%ldM", 1L << (grmodes[i].bpp-20));
	else  if (grmodes[i].bpp > 10)
	  sprintf(mdtxt, "%ldk", 1L << (grmodes[i].bpp-10));
	else
	  sprintf(mdtxt, "%ld", 1L << grmodes[i].bpp);
	switch (shrt) {
	  case 2 : break;
	  case 1 : strcat(mdtxt, " col"); break;
	  default: strcat(mdtxt, " colors"); break;
	}
}

int ColsCheck(int cols, int ml, int sep) {
  int len;

  len = ml * cols + (cols-1) * sep + 1;
  return len <= COLUMNS;
}

void PrintModes(void) {
	char mdtxt[100];
	unsigned int maxlen;
	int i, n, shrt, c, cols;

	cols = (nmodes+LINES-1) / LINES;
	do {
	  for (shrt = 0; shrt <= 2; ++shrt) {
	    maxlen = 0;
	    for (i = 0; i < nmodes; ++i) {
	      ModeText(i,shrt,mdtxt);
	      if (strlen(mdtxt) > maxlen) maxlen = strlen(mdtxt);
	    }
	    n = 2;
	    if (cols>1 || shrt<2) {
	      if (!ColsCheck(cols, maxlen, n)) continue;
	      while (ColsCheck(cols, maxlen, n+1) && n < 4) ++n;
	    }
	    c = 0;
	    for (i = 0; i < nmodes; ++i) {
	      if (++c == cols) c = 0;
	      ModeText(i,shrt,mdtxt);
	      printf("%*s%s", (c ? -((int)(maxlen+n)) : -((int)maxlen)), mdtxt, (c || (i+1==nmodes) ? "" : "\n") );
	    }
	    return;
	  }
	  --cols;
	} while (1);
}

int main(int argc, char **argv)
{
	int  i;

	grmodes = malloc(MAX_MODES*sizeof(gvmode));
	assert(grmodes!=NULL);
#if MEASURE_RAM_MODES
	rammodes = malloc(MAX_MODES*sizeof(gvmode));
	assert(rammodes!=NULL);
#endif

	GrSetDriver(NULL);
	if(GrCurrentVideoDriver() == NULL) {
	    printf("No graphics driver found\n");
	    exit(1);
	}

	nmodes = collectmodes(GrCurrentVideoDriver());
	if(nmodes == 0) {
	    printf("No graphics modes found\n");
	    exit(1);
	}
	qsort(grmodes,nmodes,sizeof(grmodes[0]),vmcmp);
#if MEASURE_RAM_MODES
	for (i=0; i < nmodes; ++i) {
	  rammodes[i].fm    = GR_frameUndef;      /* filled in later */
	  rammodes[i].w     = grmodes[i].w;
	  rammodes[i].h     = grmodes[i].h;
	  rammodes[i].bpp   = grmodes[i].bpp;
	  rammodes[i].flags = FLG_rammode;
	}
#endif

	if(argc >= 2 && (i = atoi(argv[1])) >= 1 && i <= nmodes) {
	    speedcheck(&grmodes[i - 1], 1, 0);
	    goto done;
	}

	first = 1;
	for( ; ; ) {
	    char mb[41], *m = mb;
	    int tflag = 0;
	    GrSetMode(GR_default_text);
	    printf(
		"Graphics driver: \"%s\"\t"
		"graphics defaults: %dx%d %ld colors\n",
		GrCurrentVideoDriver()->name,
		GrDriverInfo->defgw,
		GrDriverInfo->defgh,
		(long)GrDriverInfo->defgc
	    );
	    PrintModes();
	    printf("\nEnter #, 't#' toggels tag, 'm' measure tagged and 'q' to quit> ");
	    fflush(stdout);
	    if(!fgets(m,40,stdin)) break;
	    switch (*m) {
	      case 't':
	      case 'T': tflag = 1;
			++m;
			break;
	      case 'A':
	      case 'a': for (i=0; i < nmodes; ++i)
			  SET_TAGGED(&grmodes[i]);
			break;
	      case 'M':
	      case 'm': for (i=0; i < nmodes; ++i)
			  if (TAGGED(&grmodes[i])) {
			    speedcheck(&grmodes[i], 0, 0);
			    TOGGLE_TAGGED(&grmodes[i]);
			  }
			break;
	      case 'Q':
	      case 'q': goto done;
	    }
	    if ((sscanf(m,"%d",&i) != 1) || (i < 1) || (i > nmodes))
		continue;
	    i--;
	    if (tflag) TOGGLE_TAGGED(&grmodes[i]);
		  else speedcheck(&grmodes[i], 1, 1);
	}
done:
	if (measured_any) {
	    int i;
	    FILE *log = fopen("speedtst.log", "a");

	    if (!log) exit(1);

	    fprintf( log, "\nGraphics driver: \"%s\"\n\n",
					       GrCurrentVideoDriver()->name);
	    printf("Results: \n");
	    printresultheader(log);

	    for (i=0; i < nmodes; ++i)
	      if (MEASURED(&grmodes[i]))
		printresultline(log, &grmodes[i]);
#if MEASURE_RAM_MODES
	    for (i=0; i < nmodes; ++i)
	      if (MEASURED(&rammodes[i]))
		printresultline(log, &rammodes[i]);
#endif
	    fclose(log);
	}
	return(0);
}

