/**
 ** grdriver.h ---- utilities for frame buffer and video card drivers
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

#ifndef __GRDRIVER_H_INCLUDED__
#define __GRDRIVER_H_INCLUDED__

#ifndef __GRX20_H_INCLUDED__
#include "grx20.h"
#endif

/*
** The following definitions shouldn't be used in portable
** or binary distributed programs for compatibility with
** future versions of GRX
*/
#ifdef USE_GRX_INTERNAL_DEFINITIONS

/*
 * Video mode flag bits (in the 'GrVideoModeExt' structure)
 */
#define GR_VMODEF_LINEAR        1       /* uses linear memory mapping */
#define GR_VMODEF_ACCEL         2       /* it is an accelerated mode */
#define GR_VMODEF_FAST_SVGA8    4       /* use faster mixed linear-planar access */
#define GR_VMODEF_MEMORY        8       /* virtual screen, memory only mode */

extern GrFrameDriver
/*
 * Standard frame drivers in GRX
 */
_GrFrameDriverHERC1,                    /* Hercules mono */
_GrFrameDriverEGAVGA1,                  /* EGA VGA mono */
_GrFrameDriverEGA4,                     /* EGA 16 color */
_GrFrameDriverSVGA4,                    /* (Super) VGA 16 color */
_GrFrameDriverSVGA8,                    /* (Super) VGA 256 color */
_GrFrameDriverVGA8X,                    /* VGA 256 color mode X */
_GrFrameDriverSVGA16,                   /* Super VGA 32768/65536 color */
_GrFrameDriverSVGA24,                   /* Super VGA 16M color */
_GrFrameDriverSVGA32L,                  /* Super VGA 16M color padded #1 */
_GrFrameDriverSVGA32H,                  /* Super VGA 16M color padded #2 */
					/* Linear Framebuffer Modes : */
_GrFrameDriverSVGA8_LFB,                /* (Super) VGA 256 color */
_GrFrameDriverSVGA16_LFB,               /* Super VGA 32768/65536 color */
_GrFrameDriverSVGA24_LFB,               /* Super VGA 16M color */
_GrFrameDriverSVGA32L_LFB,              /* Super VGA 16M color padded #1 */
_GrFrameDriverSVGA32H_LFB,              /* Super VGA 16M color padded #2 */

_GrFrameDriverXWIN8,                    /* X 8 bpp */
_GrFrameDriverXWIN16,                   /* X 16 bpp */
_GrFrameDriverXWIN24,                   /* X 24 bpp */
_GrFrameDriverXWIN32L,                  /* X 32 bpp padded #1 */
_GrFrameDriverXWIN32H,                  /* X 32 bpp padded #2 */
	
_GrFrameDriverWIN32_4,                  /* WIN32 4 bpp */
_GrFrameDriverWIN32_8,                  /* WIN32 8 bpp */
_GrFrameDriverWIN32_24,                 /* WIN32 24 bpp */

_GrFrameDriverSDL8,                     /* SDL 8 bpp */
_GrFrameDriverSDL16,                    /* SDL 16 bpp */
_GrFrameDriverSDL24,                    /* SDL 24 bpp */
_GrFrameDriverSDL32L,                   /* SDL 32 bpp padded #2 */
_GrFrameDriverSDL32H,                   /* SDL 32 bpp padded #2 */

_GrFrameDriverRAM1,                     /* mono */
_GrFrameDriverRAM4,                     /* 16 color planar */
_GrFrameDriverRAM8,                     /* 256 color */
_GrFrameDriverRAM16,                    /* 32768/65536 color */
_GrFrameDriverRAM24,                    /* 16M color */
_GrFrameDriverRAM32L,                   /* 16M color padded #1 */
_GrFrameDriverRAM32H,                   /* 16M color padded #2 */
_GrFrameDriverRAM3x8,                   /* 16M color planar (image mode) */
/*
 * This is a NULL-terminated table of frame driver descriptor pointers. Users
 * can provide their own table with only the desired (or additional) drivers.
 * Otherwise the table from the GRX library is linked, which includes ALL
 * currently available drivers (i.e. the ones above).
 */
*_GrFrameDriverTable[];


extern GrVideoDriver
/*
 * Standard video drivers in GRX
 */
_GrVideoDriverHERC,                     /* Hercules driver */
_GrVideoDriverSTDEGA,                   /* standard EGA driver */
_GrVideoDriverSTDVGA,                   /* standard VGA driver */
_GrVideoDriverVESA,                     /* generic VESA Super VGA driver */
_GrVideoDriverATI28800,                 /* ATI 28800 chipset driver */
_GrVideoDriverET4000,                   /* Tseng ET4000 driver */
_GrVideoDriverCL5426,                   /* Cirrus 5426 driver */
_GrVideoDriverMACH64,                   /* ATI MACH64 driver */
_GrVideoDriverS3,                       /* S3 driver */

_GrVideoDriverXWIN,                     /* X11 interface */
_GrVideoDriverXF86DGA,                  /* XFree86 DGA interface */

_GrVideoDriverSVGALIB,                  /* Linux SVGALIB interface */
_GrVideoDriverLINUXFB,                  /* Linux framebuffer */

_GrVideoDriverWIN32,                    /* WIN32 interface */

_GrVideoDriverSDL,                      /* SDL intrercace */

_GrDriverMEM,                           /* memory screen driver */

/*
 * This is a NULL-terminated table of video driver descriptor pointers. Users
 * can provide their own table with only the desired (or additional) drivers.
 * Otherwise the table from the GRX library is linked, which includes ALL
 * currently available drivers (i.e. the ones above).
 */
*_GrVideoDriverTable[];

/*
 * frame driver function types used inside GRX
 */
typedef GrColor  (*_GR_readPix)(GrFrame *,int,int);
typedef void     (*_GR_drawPix)(int,int,GrColor);
typedef void     (*_GR_blitFunc)(GrFrame *dst,int dx,int dy,
				 GrFrame *src,int  x,int  y,
				 int w,int h,GrColor op);
typedef GrColor far *(*_GR_getIndexedScanline)(GrFrame *c,int x,int y,
					       int w, int *indx);
typedef void     (*_GR_putScanline)(int x,int y,int w,
				    const GrColor far *scl,GrColor op);

/*
 * Frame driver utility functions
 */
void _GrFrDrvGenericBitBlt(GrFrame *dst,int dx,int dy,GrFrame *src,int x,int y,int w,int h,GrColor op);
void _GrFrDrvPackedBitBltR2R(GrFrame *dst,int dx,int dy,GrFrame *src,int x,int y,int w,int h,GrColor op);
void _GrFrDrvPackedBitBltR2V(GrFrame *dst,int dx,int dy,GrFrame *src,int x,int y,int w,int h,GrColor op);
void _GrFrDrvPackedBitBltV2R(GrFrame *dst,int dx,int dy,GrFrame *src,int x,int y,int w,int h,GrColor op);
void _GrFrDrvPackedBitBltV2V(GrFrame *dst,int dx,int dy,GrFrame *src,int x,int y,int w,int h,GrColor op);
void _GrFrDrvPackedBitBltR2V_LFB(GrFrame *dst,int dx,int dy,GrFrame *src,int x,int y,int w,int h,GrColor op);
void _GrFrDrvPackedBitBltV2R_LFB(GrFrame *dst,int dx,int dy,GrFrame *src,int x,int y,int w,int h,GrColor op);
void _GrFrDrvPackedBitBltV2V_LFB(GrFrame *dst,int dx,int dy,GrFrame *src,int x,int y,int w,int h,GrColor op);

void _GrFrDrvGenericPutScanline(int x,int y,int w,const GrColor far *scl, GrColor op );
GrColor *_GrFrDrvGenericGetIndexedScanline(GrFrame *c,
					   int x,int y,int w,
					   int *indx         );

void _GrFrDrvGenericStretchBlt(GrFrame *dst,int dx,int dy,int dw,int dh,
			       GrFrame *src,int sx,int sy,int sw,int sh,
			       GrColor op);

/*
 * Video driver utility functions
 */
int  _GrViDrvDetectEGAVGA(void);
int  _GrViDrvDetectVGA(void);
int  _GrViDrvDetectEGA(void);
int  _GrViDrvInitEGAVGA(char *options);
void _GrViDrvResetEGAVGA(void);
int  _GrViDrvGetCurrentEGAVGAmode(void);
int  _GrViDrvSetEGAVGAmode(GrVideoMode *mp,int noclear);
int  _GrViDrvSetEGAVGAcustomTextMode(GrVideoMode *mp,int noclear);
void _GrViDrvLoadColorEGA4(int c,int r,int g,int b);
void _GrViDrvLoadColorVGA4(int c,int r,int g,int b);
void _GrViDrvLoadColorVGA8(int c,int r,int g,int b);
int  _GrViDrvVESAsetVirtualSize(GrVideoMode *md,int w,int h,GrVideoMode *result);
int  _GrViDrvVESAvirtualScroll(GrVideoMode *md,int x,int y,int result[2]);

/* for 8 bit color modes:
**   normal DAC has 6bit for each rgb color component, VESA 2.0 allows
**   6bit or 8bit DAC color values.
**   _GrViDrvLoadColorVGA8() requires 8bit values that will be shifted right:
**
**      DAC width   6bit    8bit
**          shift   2       0
*/
void _GrViDrvSetDACshift(int shift);

/*
 * Commonly used video driver data structures
 */
extern GrVideoModeExt _GrViDrvEGAVGAtextModeExt;
extern GrVideoModeExt _GrViDrvEGAVGAcustomTextModeExt;

GrVideoMode * _gr_selectmode(GrVideoDriver *drv,int w,int h,int bpp,
			     int txt,unsigned int *ep);

#endif /* USE_GRX_INTERNAL_DEFINITIONS */

#endif /* whole file */

