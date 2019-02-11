/**
 ** svga32l.c ---- the 16M color padded Super VGA frame driver (low)
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

#define PIX2COL(col) ((col)&((GrColor)0xFFFFFFUL))
#define COL2PIX(col) ((col)&((GrColor)0xFFFFFFUL))

#include "fdrivers/banked32.h"

GrFrameDriver _GrFrameDriverSVGA32L = {
    GR_frameSVGA32L,            /* frame mode */
#ifdef GRX_USE_RAM3x8
    GR_frameRAM3x8,             /* compatible RAM frame mode */
#else
    GR_frameRAM32L,             /* compatible RAM frame mode */
#endif
    TRUE,                       /* onscreen */
    4,                          /* line width alignment */
    1,                          /* number of planes */
    32,                         /* bits per pixel */
    32*16*1024L*1024L,          /* max plane size the code can handle */
    NULL,
    readpixel,
    drawpixel,
    drawline,
    drawhline,
    drawvline,
    drawblock,
    drawbitmap,
    drawpattern,
    bitblt,
#ifdef GRX_USE_RAM3x8
    _GrFrDrvGenericBitBlt,
    _GrFrDrvGenericBitBlt,
#else
    bltv2r,
    bltr2v,
#endif
    _GrFrDrvGenericGetIndexedScanline,
    _GrFrDrvGenericPutScanline
};
