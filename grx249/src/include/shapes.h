/**
 ** shapes.h ---- declarations and global data for generating complex shapes
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
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

#ifndef __SHAPES_H_INCLUDED__
#define __SHAPES_H_INCLUDED__

#define USE_FDR_DRAWPATTERN 1

typedef union _GR_fillArg {
    GrColor color;
    struct _GR_bitmap *bmp;
    struct _GR_pixmap *pxp;
    GrPattern *p;
} GrFillArg;

typedef void (*PixelFillFunc)(int x,int y,GrFillArg fval);
typedef void (*LineFillFunc)(int x,int y,int dx,int dy,GrFillArg fval);
typedef void (*ScanFillFunc)(int x,int y,int w,GrFillArg fval);

typedef struct _GR_filler {
    PixelFillFunc pixel;
    LineFillFunc  line;
    ScanFillFunc  scan;
} GrFiller;

extern GrFiller _GrSolidFiller;
extern GrFiller _GrPatternFiller;
/*
extern GrFiller _GrBitmapFiller;
extern GrFiller _GrPixmapFiller;
*/

void _GrDrawPolygon(int n,int pt[][2],GrFiller *f,GrFillArg c,int doClose);
void _GrDrawCustomPolygon(int n,int pt[][2],const GrLineOption *lp,GrFiller *f,GrFillArg c,int doClose,int circle);
void _GrScanConvexPoly(int n,int pt[][2],GrFiller *f,GrFillArg c);
void _GrScanPolygon(int n,int pt[][2],GrFiller *f,GrFillArg c);
void _GrScanEllipse(int xc,int yc,int xa,int ya,GrFiller *f,GrFillArg c,int filled);

/* --- */
#define _GrDrawPatternedPixel ((PixelFillFunc)_GrPatternFilledPlot)
#define _GrDrawPatternedLine ((LineFillFunc)_GrPatternFilledLine)
void _GrFillPatternedScanLine(int x,int y,int w,GrFillArg arg);

/* --- */
void _GrFloodFill(int x,int y,GrColor border,GrFiller *f,GrFillArg fa);

/* -- */
void _GrFillPattern(int x,int y,int width,GrPattern *p);
void _GrFillPatternExt(int x,int y,int sx, int sy,int width,GrPattern *p);
void _GrPatternFilledLine(int x1,int y1,int dx,int dy,GrPattern *p);
void _GrPatternFilledPlot(int x,int y,GrPattern *p);

void _GrFillBitmapPattern(int x,int y,int w,int h,
			  char far *bmp,int pitch,int start,
			  GrPattern* p,GrColor bg);
void _GrFillBitmapPatternExt(int x,int y,int w,int h, int sx, int sy,
			     char far *bmp,int pitch,int start,
			     GrPattern* p,GrColor bg);

#endif
