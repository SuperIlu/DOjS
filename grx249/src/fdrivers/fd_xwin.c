/**
 ** fd_xwin.c - the X Window color frame driver
 **
 ** Author:     Ulrich Leodolter
 ** E-mail:     ulrich@lab1.psy.univie.ac.at
 ** Date:       Thu Sep 28 10:31:08 1995
 ** RCSId:      $Id: fd_xwin.c 1.1 1995/11/19 17:42:55 ulrich Exp $
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
 ** 070505 M.Alvarez, Using a Pixmap for BackingStore
 ** 080801 M.Alvarez, Sanitized pixel cache code
 ** 080801 M.Alvarez, New faster and specific for X11 putscanline function
 **
 **/

#include <string.h>
#include "libgrx.h"
#include "libxwin.h"
#include "arith.h"
#include "memfill.h"
#include "grdriver.h"

INLINE
void _XGrCopyBStore(int x, int y, int width, int lenght)
{
  if (USE_PIXMAP_FOR_BS) {
    XCopyArea (_XGrDisplay,
               _XGrBStore,
               _XGrWindow,
               DefaultGC (_XGrDisplay, _XGrScreen),
               x,
               y,
               width,
               lenght,
               x,
               y);
  }
}

static INLINE
void _XGrSetForeColor (GrColor color)
{
  GRX_ENTER();
  color &= GrCVALUEMASK;
  if (color != _XGrForeColor)
    {
      DBGPRINTF(DBG_DRIVER,("New foreground color: %x\n",(unsigned)color));
      _XGrForeColor = color;
      XSetForeground (_XGrDisplay, _XGrGC, _XGrForeColor);
    }
  GRX_LEAVE();
}

static INLINE
void _XGrSetBackColor (GrColor color)
{
  GRX_ENTER();
  color &= GrCVALUEMASK;
  if (color != _XGrBackColor)
    {
      DBGPRINTF(DBG_DRIVER,("New background color: %x\n",(unsigned)color));
      _XGrBackColor = color;
      XSetBackground (_XGrDisplay, _XGrGC, _XGrBackColor);
    }
  GRX_LEAVE();
}

static INLINE
void _XGrSetColorOper (GrColor color)
{
  static int _GXtable[4] = {
    GXcopy,     /* C_WRITE */
    GXxor,      /* C_XOR */
    GXor,       /* C_OR */
    GXand       /* C_AND */
  };
  unsigned int coper;

  GRX_ENTER();
  coper = C_OPER(color) & 0x03;
  if (coper != _XGrColorOper)
    {
      _XGrColorOper = coper;
      XSetFunction (_XGrDisplay, _XGrGC, _GXtable[_XGrColorOper]);
    }
  GRX_LEAVE();
}

/*
 * PixelCache uses complete rows now
 * Designed for loops like:
 *
 * for (y = 0; y < height; y++)
 *   for (x = 0; x < width; x++)
 *     *dest++ = GrPixel (x, y);
 */

#define PIXEL_CACHE_HEIGHT 4

static XImage * _XGrPixelCacheImage     = NULL;
static Drawable _XGrPixelCacheDrawable  = None;
static int      _XGrPixelCacheWidth = 0;
static int      _XGrPixelCacheHeight = 0;
static int      _XGrPixelCacheY1 = 0;
static int      _XGrPixelCacheY2 = 0;

/* y1 <= y2 required ! */
#define AREA_OVERLAP_PIXEL_CACHE(y1,y2) \
	(   (y2) >= _XGrPixelCacheY1          \
	 && (y1) <=  _XGrPixelCacheY2 )

#define PIXEL_CACHE_INVALIDATE() do {              \
	    _XGrPixelCacheDrawable = None;         \
	    if (_XGrPixelCacheImage) {             \
	      XDestroyImage (_XGrPixelCacheImage); \
	      _XGrPixelCacheImage = NULL;          \
	    }                                      \
	} while (0)

static INLINE
void _XGrCheckPixelCache(int y1, int y2)
{
  if (_XGrPixelCacheDrawable == None) return;
  if (AREA_OVERLAP_PIXEL_CACHE(y1,y2))
    PIXEL_CACHE_INVALIDATE();
}

static
GrColor readpixel(GrFrame *c, int x, int y)
{
  GrColor col;
  GRX_ENTER();
  if (_XGrPixelCacheDrawable != (Drawable) c->gf_baseaddr[0]
      || _XGrPixelCacheImage == NULL
      || !AREA_OVERLAP_PIXEL_CACHE(y,y)) {

    if (_XGrPixelCacheImage) {
      XDestroyImage (_XGrPixelCacheImage);
      _XGrPixelCacheImage = NULL;
    }
    _XGrPixelCacheDrawable = (Drawable) c->gf_baseaddr[0];

    _XGrPixelCacheWidth = GrScreenX();
    _XGrPixelCacheY1 = y;
    _XGrPixelCacheY2 = y + PIXEL_CACHE_HEIGHT - 1;
    if (_XGrPixelCacheY2 >= GrScreenY())
      _XGrPixelCacheY2 = GrScreenY() - 1;
    _XGrPixelCacheHeight = _XGrPixelCacheY2 - _XGrPixelCacheY1 + 1;

    _XGrPixelCacheImage = XGetImage (_XGrDisplay,
				     _XGrPixelCacheDrawable,
				     0,
				     _XGrPixelCacheY1,
				     _XGrPixelCacheWidth,
				     _XGrPixelCacheHeight,
				     AllPlanes,
				     ZPixmap);
    if (! _XGrPixelCacheImage) {
	  col = GrNOCOLOR;
	  goto done;
	}
      }
  col = XGetPixel (_XGrPixelCacheImage, x, y - _XGrPixelCacheY1);
done:
  GRX_RETURN( col );
}

static INLINE
void drawpixel(int x,int y,GrColor c)
{
  GRX_ENTER();
  _XGrSetForeColor (c);
  _XGrSetColorOper (c);
  XDrawPoint (_XGrDisplay,
	      (Drawable) CURC->gc_baseaddr[0],
	      _XGrGC,
	      x,
	      y);
  _XGrCopyBStore(x, y, 1, 1);
  _XGrCheckPixelCache(y, y);

  GRX_LEAVE();
}

static
void drawhline(int x,int y,int w,GrColor c)
{
  int x2;

  GRX_ENTER();
  _XGrSetForeColor (c);
  _XGrSetColorOper (c);
  x2 = x + w - 1;
  XDrawLine (_XGrDisplay,
	     (Drawable) CURC->gc_baseaddr[0],
	     _XGrGC,
	     x,  y,
	     x2, y);
  _XGrCopyBStore(x, y, w, 1);
  _XGrCheckPixelCache(y, y);

  GRX_LEAVE();
}

static
void drawvline(int x,int y,int h,GrColor c)
{
  int y2;

  GRX_ENTER();
  _XGrSetForeColor (c);
  _XGrSetColorOper (c);
  y2 = y + h - 1;
  XDrawLine (_XGrDisplay,
	     (Drawable) CURC->gc_baseaddr[0],
	     _XGrGC,
	     x, y,
	     x, y2);
  _XGrCopyBStore(x, y, 1, h);
  _XGrCheckPixelCache(y, y2);

  GRX_LEAVE();
}

static
void drawblock(int x,int y,int w,int h,GrColor c)
{
  GRX_ENTER();
  _XGrSetForeColor (c);
  _XGrSetColorOper (c);
  XFillRectangle (_XGrDisplay,
		  (Drawable) CURC->gc_baseaddr[0],
		  _XGrGC,
		  x,
		  y,
		  w,
		  h);
  _XGrCopyBStore(x, y, w, h);
  _XGrCheckPixelCache(y, y+h-1);

  GRX_LEAVE();
}


static
void drawline(int x,int y,int dx,int dy,GrColor c)
{
  GRX_ENTER();
  _XGrSetForeColor (c);
  _XGrSetColorOper (c);
  dx += x;
  dy += y;
  XDrawLine (_XGrDisplay,
	     (Drawable) CURC->gc_baseaddr[0],
	     _XGrGC,
	     x, y,
	     dx, dy);
      isort(x,dx);
      isort(y,dy);
  _XGrCopyBStore(x, y, dx-x+1, dy-y+1);
  _XGrCheckPixelCache(y, dy);

  GRX_LEAVE();
}

static
void drawbitmap(int x,int y,int w,int h,char far *bmp,int pitch,int start,GrColor fg,GrColor bg)
{
  XImage ximage;

  GRX_ENTER();
  bmp += (unsigned int)start >> 3;
  start &= 7;

  ximage.width          = w;
  ximage.height         = h;
  ximage.xoffset        = start;
  ximage.format         = XYBitmap;
  ximage.data           = bmp;
  ximage.byte_order     = LSBFirst;
  ximage.bitmap_unit    = 8;
  ximage.bitmap_bit_order = MSBFirst;
  ximage.bitmap_pad     = 8;
  ximage.depth          = 1;
  ximage.bytes_per_line = pitch;
  ximage.bits_per_pixel = 1;
  ximage.red_mask       = 0L;
  ximage.green_mask     = 0L;
  ximage.blue_mask      = 0L;
  ximage.obdata         = NULL;
  sttzero(&ximage.f);

# ifndef PRE_R6_ICCCM
  if ( XInitImage (&ximage) != 0)
# endif
  {
    if ((C_OPER(fg) == C_OPER(bg)) && (fg != GrNOCOLOR)) {
      _XGrSetForeColor (fg);
      _XGrSetBackColor (bg);
      _XGrSetColorOper (fg);
      DBGPRINTF(DBG_DRIVER,("Calling XPutImage (1) ...\n"));
      XPutImage (_XGrDisplay,
		 (Drawable) CURC->gc_baseaddr[0],
		 _XGrGC,
		 &ximage,
		 0,
		 0,
		 x,
		 y,
		 w,
		 h);
      DBGPRINTF(DBG_DRIVER,("Calling XPutImage (1) done\n"));
    }
    else {
      if (fg != GrNOCOLOR) {
	XSetForeground (_XGrDisplay, _XGrBitmapGC, 1);
	XSetBackground (_XGrDisplay, _XGrBitmapGC, 0);
	DBGPRINTF(DBG_DRIVER,("Calling XPutImage (2) ...\n"));
	XPutImage (_XGrDisplay,
		   _XGrBitmap,
		   _XGrBitmapGC,
		   &ximage,
		   0,
		   0,
		   0,
		   0,
		   w,
		   h);
	DBGPRINTF(DBG_DRIVER,("Calling XPutImage (2) done\n"));
	XSetStipple (_XGrDisplay, _XGrGC, _XGrBitmap);
	XSetTSOrigin (_XGrDisplay, _XGrGC, x, y);
	XSetFillStyle (_XGrDisplay, _XGrGC, FillStippled);
	_XGrSetForeColor (fg);
	_XGrSetColorOper (fg);
	XFillRectangle (_XGrDisplay,
			(Drawable) CURC->gc_baseaddr[0],
			_XGrGC,
			x,
			y,
			w,
			h);
      }
      if (bg != GrNOCOLOR) {
	XSetForeground (_XGrDisplay, _XGrBitmapGC, 0);
	XSetBackground (_XGrDisplay, _XGrBitmapGC, 1);
	DBGPRINTF(DBG_DRIVER,("Calling XPutImage (3) ...\n"));
	XPutImage (_XGrDisplay,
		   _XGrBitmap,
		   _XGrBitmapGC,
		   &ximage,
		   0,
		   0,
		   0,
		   0,
		   w,
		   h);
	DBGPRINTF(DBG_DRIVER,("Calling XPutImage (3) done\n"));
	XSetStipple (_XGrDisplay, _XGrGC, _XGrBitmap);
	XSetTSOrigin (_XGrDisplay, _XGrGC, x, y);
	XSetFillStyle (_XGrDisplay, _XGrGC, FillStippled);
	_XGrSetForeColor (bg);
	_XGrSetColorOper (bg);
	XFillRectangle (_XGrDisplay,
			(Drawable) CURC->gc_baseaddr[0],
			_XGrGC,
			x,
			y,
			w,
			h);
      }
      XSetFillStyle (_XGrDisplay, _XGrGC, FillSolid);
    }

    _XGrCopyBStore(x, y, w, h);
    _XGrCheckPixelCache(y, y+h-1);
  }
  GRX_LEAVE();
}

/* Note: drawpattern is not tested because it's not used in this GRX version */
static
void drawpattern(int x,int y,int w,char patt,GrColor fg,GrColor bg)
{
  XImage ximage;

  GRX_ENTER();
  ximage.width          = 8;
  ximage.height         = 1;
  ximage.xoffset        = 0;
  ximage.format         = XYBitmap;
  ximage.data           = &patt;
  ximage.byte_order     = LSBFirst;
  ximage.bitmap_unit    = 8;
  ximage.bitmap_bit_order = MSBFirst;
  ximage.bitmap_pad     = 8;
  ximage.depth          = 1;
  ximage.bytes_per_line = 1;
  ximage.bits_per_pixel = 1;
  ximage.red_mask       = 0L;
  ximage.green_mask     = 0L;
  ximage.blue_mask      = 0L;
  ximage.obdata         = NULL;
  sttzero(&ximage.f);

# ifndef PRE_R6_ICCCM
  if (XInitImage (&ximage) != 0)
# endif
  {
    if ((C_OPER(fg) == C_OPER(bg)) && (fg != GrNOCOLOR)) {
      XSetForeground (_XGrDisplay, _XGrPatternGC, 1);
      XSetBackground (_XGrDisplay, _XGrPatternGC, 0);
      XPutImage (_XGrDisplay,
		 _XGrPattern,
		 _XGrPatternGC,
		 &ximage,
		 0,
		 0,
		 0,
		 0,
		 8,
		 1);
      XSetStipple (_XGrDisplay, _XGrGC, _XGrPattern);
      XSetTSOrigin (_XGrDisplay, _XGrGC, x, y);
      XSetFillStyle (_XGrDisplay, _XGrGC, FillOpaqueStippled);
      _XGrSetForeColor (fg);
      _XGrSetBackColor (bg);
      _XGrSetColorOper (fg);
      XFillRectangle (_XGrDisplay,
		      (Drawable) CURC->gc_baseaddr[0],
		      _XGrGC,
		      x,
		      y,
		      w,
		      1);
    }
    else {
      if (fg != GrNOCOLOR) {
	XSetForeground (_XGrDisplay, _XGrPatternGC, 1);
	XSetBackground (_XGrDisplay, _XGrPatternGC, 0);
	XPutImage (_XGrDisplay,
		   _XGrPattern,
		   _XGrPatternGC,
		   &ximage,
		   0,
		   0,
		   0,
		   0,
		   8,
		   1);
	XSetStipple (_XGrDisplay, _XGrGC, _XGrPattern);
	XSetTSOrigin (_XGrDisplay, _XGrGC, x, y);
	XSetFillStyle (_XGrDisplay, _XGrGC, FillStippled);
	_XGrSetForeColor (fg);
	_XGrSetColorOper (fg);
	XFillRectangle (_XGrDisplay,
			(Drawable) CURC->gc_baseaddr[0],
			_XGrGC,
			x,
			y,
			w,
			1);
      }
      if (bg != GrNOCOLOR) {
	XSetForeground (_XGrDisplay, _XGrPatternGC, 0);
	XSetBackground (_XGrDisplay, _XGrPatternGC, 1);
	XPutImage (_XGrDisplay,
		   _XGrPattern,
		   _XGrPatternGC,
		   &ximage,
		   0,
		   0,
		   0,
		   0,
		   8,
		   1);
	XSetStipple (_XGrDisplay, _XGrGC, _XGrPattern);
	XSetTSOrigin (_XGrDisplay, _XGrGC, x, y);
	XSetFillStyle (_XGrDisplay, _XGrGC, FillStippled);
	_XGrSetForeColor (bg);
	_XGrSetColorOper (bg);
	XFillRectangle (_XGrDisplay,
			(Drawable) CURC->gc_baseaddr[0],
			_XGrGC,
			x,
			y,
			w,
			1);
      }
    }
    XSetFillStyle (_XGrDisplay, _XGrGC, FillSolid);

    _XGrCopyBStore(x, y, w, 1);
    _XGrCheckPixelCache(y, y);
  }
  GRX_LEAVE();
}

static
void bitblt(GrFrame *dst,int dx,int dy,GrFrame *src,int x,int y,int w,int h,GrColor op)
{
  GRX_ENTER();
  _XGrSetColorOper (op);
  XCopyArea (_XGrDisplay,
	     (Drawable) src->gf_baseaddr[0],
	     (Drawable) dst->gf_baseaddr[0],
	     _XGrGC,
	     x,
	     y,
	     w,
	     h,
	     dx,
	     dy);

  _XGrCopyBStore(dx, dy, w, h);
  _XGrCheckPixelCache(dy, dy+h-1);

  GRX_LEAVE();
}

static
void bltv2r(GrFrame *dst,int dx,int dy,GrFrame *src,int sx,int sy,int w,int h,GrColor op)
{
  GRX_ENTER();
  if(GrColorMode(op) == GrIMAGE)
    _GrFrDrvGenericBitBlt(dst,dx,dy,
			  src,sx,sy,
			  w,h,
			  op
			  );
  else {
    XImage *ximage;

    ximage = XGetImage (_XGrDisplay,
			(Drawable) src->gf_baseaddr[0],
			sx,
			sy,
			w,
			h,
			AllPlanes,
			ZPixmap);
    if (ximage) {
      int bytes_per_pixel = _XGrBitsPerPixel >> 3;
      GrFrame tmp = *dst;

      tmp.gf_baseaddr[0] =
	tmp.gf_baseaddr[1] =
	  tmp.gf_baseaddr[2] =
	    tmp.gf_baseaddr[3] = ximage->data;
      tmp.gf_lineoffset = ximage->bytes_per_line;

      _GrFrDrvPackedBitBltR2R(dst, (dx * bytes_per_pixel), dy,
			      &tmp, 0, 0,
			      (w * bytes_per_pixel), h,
			      op
			      );
      XDestroyImage (ximage);
    }
  }
  GRX_LEAVE();
}

static void bltr2v(GrFrame *dst,int dx,int dy,GrFrame *src,int sx,int sy,int w,int h,GrColor op)
{
  GRX_ENTER();
  if(GrColorMode(op) == GrIMAGE)
    _GrFrDrvGenericBitBlt(dst,dx,dy,
			  src,sx,sy,
			  w,h,
			  op
			  );
  else {
    XImage ximage;
    Visual *visual = DefaultVisual(_XGrDisplay,_XGrScreen);

    ximage.width        = sx + w;
    ximage.height       = sy + h;
    ximage.xoffset      = 0;
    ximage.format       = ZPixmap;
    ximage.data         = src->gf_baseaddr[0];
    ximage.byte_order   = LSBFirst;
    ximage.bitmap_unit  = BitmapUnit(_XGrDisplay);
    ximage.bitmap_bit_order = BitmapBitOrder(_XGrDisplay);
    ximage.bitmap_pad   = BitmapPad(_XGrDisplay);
    ximage.depth        = _XGrDepth;
    ximage.bytes_per_line = src->gf_lineoffset;
    ximage.bits_per_pixel = _XGrBitsPerPixel;
    ximage.red_mask     = visual->red_mask;
    ximage.green_mask   = visual->green_mask;
    ximage.blue_mask    = visual->blue_mask;
    ximage.obdata       = NULL;
    sttzero(&ximage.f);

# ifndef PRE_R6_ICCCM
      if (XInitImage (&ximage) != 0)
# endif
    {
      _XGrSetColorOper (op);
      XPutImage (_XGrDisplay,
		 (Drawable) dst->gf_baseaddr[0],
		 _XGrGC,
		 &ximage,
		 sx,
		 sy,
		 dx,
		 dy,
		 w,
		 h);
      _XGrCopyBStore(dx, dy, w, h);
      _XGrCheckPixelCache(dy, dy+h-1);
    }
  }
  GRX_LEAVE();
}

static
void putscanline(int x, int y, int w, const GrColor *scl, GrColor op)
{
  GrColor skipc;
  int ind;
  GRX_ENTER();
  skipc = op ^ GrIMAGE;
  _XGrSetColorOper(op);

  for (ind = 0; ind < w; ind++) {
    if (scl[ind] != skipc) {
      _XGrSetForeColor(scl[ind]);
      XDrawPoint (_XGrDisplay,
                  (Drawable) CURC->gc_baseaddr[0],
                  _XGrGC, x+ind, y);
    }
  }

  _XGrCopyBStore(x, y, w, 1);
  _XGrCheckPixelCache(y, y);

  GRX_LEAVE();
}

#define ROUNDCOLORCOMP(x,n) (                                   \
    ((uint)(x) >= CLRINFO->mask[n]) ?                           \
	CLRINFO->mask[n] :                                      \
	(((x) + CLRINFO->round[n]) & CLRINFO->mask[n])          \
)

static int init(GrVideoMode *mp)
{

  if (_XGrColorNumPixels == 1) {
    unsigned long i;

    for (i = 0; i < CLRINFO->ncolors; i++) {
      if (i >= _XGrColorPixels[0] && i <= _XGrColorPixels[1]) {
	CLRINFO->ctable[i].defined  = FALSE;
      }
      else {
	int r, g, b;
	XColor xcolor;

	xcolor.red   = 0;
	xcolor.green = 0;
	xcolor.blue  = 0;
	xcolor.pixel = i;
	XQueryColors (_XGrDisplay, _XGrColormap, &xcolor, 1);
	r = xcolor.red   >> 8;
	g = xcolor.green >> 8;
	b = xcolor.blue  >> 8;
	/*
	 * Preallocate Cell; Only a buggy program will free this entry.
	 */
	CLRINFO->ctable[i].r    = ROUNDCOLORCOMP(r,0);
	CLRINFO->ctable[i].g    = ROUNDCOLORCOMP(g,1);
	CLRINFO->ctable[i].b    = ROUNDCOLORCOMP(b,2);
	CLRINFO->ctable[i].defined      = TRUE;
	CLRINFO->ctable[i].writable     = FALSE;
	CLRINFO->ctable[i].nused        = 1;
	CLRINFO->nfree--;
      }
    }
  }
  PIXEL_CACHE_INVALIDATE();
  return(TRUE);
}


GrFrameDriver _GrFrameDriverXWIN8 = {
  GR_frameXWIN8,                /* frame mode */
  GR_frameRAM8,                 /* compatible RAM frame mode */
  TRUE,                         /* onscreen */
  4,                            /* line width alignment */
  1,                            /* number of planes */
  8,                            /* bits per pixel */
  8*16*1024L*1024L,             /* max plane size the code can handle */
  init,
  readpixel,
  drawpixel,
  drawline,
  drawhline,
  drawvline,
  drawblock,
  drawbitmap,
  drawpattern,
  bitblt,
  bltv2r,
  bltr2v,
  _GrFrDrvGenericGetIndexedScanline,
  putscanline
};

GrFrameDriver _GrFrameDriverXWIN16 = {
  GR_frameXWIN16,               /* frame mode */
  GR_frameRAM16,                /* compatible RAM frame mode */
  TRUE,                         /* onscreen */
  4,                            /* line width alignment */
  1,                            /* number of planes */
  16,                           /* bits per pixel */
  16*16*1024L*1024L,            /* max plane size the code can handle */
  init,
  readpixel,
  drawpixel,
  drawline,
  drawhline,
  drawvline,
  drawblock,
  drawbitmap,
  drawpattern,
  bitblt,
  bltv2r,
  bltr2v,
  _GrFrDrvGenericGetIndexedScanline,
  putscanline
};

GrFrameDriver _GrFrameDriverXWIN24 = {
  GR_frameXWIN24,               /* frame mode */
  GR_frameRAM24,                /* compatible RAM frame mode */
  TRUE,                         /* onscreen */
  4,                            /* line width alignment */
  1,                            /* number of planes */
  24,                           /* bits per pixel */
  24*16*1024L*1024L,            /* max plane size the code can handle */
  init,
  readpixel,
  drawpixel,
  drawline,
  drawhline,
  drawvline,
  drawblock,
  drawbitmap,
  drawpattern,
  bitblt,
  bltv2r,
  bltr2v,
  _GrFrDrvGenericGetIndexedScanline,
  putscanline
};

GrFrameDriver _GrFrameDriverXWIN32L = {
  GR_frameXWIN32L,              /* frame mode */
  GR_frameRAM32L,               /* compatible RAM frame mode */
  TRUE,                         /* onscreen */
  4,                            /* line width alignment */
  1,                            /* number of planes */
  32,                           /* bits per pixel */
  32*16*1024L*1024L,            /* max plane size the code can handle */
  init,
  readpixel,
  drawpixel,
  drawline,
  drawhline,
  drawvline,
  drawblock,
  drawbitmap,
  drawpattern,
  bitblt,
  bltv2r,
  bltr2v,
  _GrFrDrvGenericGetIndexedScanline,
  putscanline
};

GrFrameDriver _GrFrameDriverXWIN32H = {
  GR_frameXWIN32H,              /* frame mode */
  GR_frameRAM32H,                /* compatible RAM frame mode */
  TRUE,                         /* onscreen */
  4,                            /* line width alignment */
  1,                            /* number of planes */
  32,                           /* bits per pixel */
  32*16*1024L*1024L,            /* max plane size the code can handle */
  init,
  readpixel,
  drawpixel,
  drawline,
  drawhline,
  drawvline,
  drawblock,
  drawbitmap,
  drawpattern,
  bitblt,
  bltv2r,
  bltr2v,
  _GrFrDrvGenericGetIndexedScanline,
  putscanline
};
