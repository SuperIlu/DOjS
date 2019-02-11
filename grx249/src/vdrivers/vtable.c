/**
 ** vtable.c ---- a table of available video drivers
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

#include "libgrx.h"
#include "grdriver.h"

GrVideoDriver *_GrVideoDriverTable[] = {
#if defined(__MSDOS__)
    &_GrVideoDriverHERC,
    &_GrVideoDriverSTDEGA,
    &_GrVideoDriverSTDVGA,
    &_GrVideoDriverVESA,
    &_GrVideoDriverATI28800,
    &_GrVideoDriverET4000,
    &_GrVideoDriverCL5426,
    &_GrVideoDriverMACH64,
    &_GrVideoDriverS3,
#endif
#if defined(__XWIN__) && !defined(__SDL__)
#if defined(XF86DGA_DRIVER)
    &_GrVideoDriverXF86DGA,
#endif
    &_GrVideoDriverXWIN,
#endif
#if defined(__linux__) && !defined(__XWIN__) && !defined(__SDL__)
#ifdef SVGALIB_DRIVER
    &_GrVideoDriverSVGALIB,
#endif
#ifdef FRAMEBUFFER_DRIVER
    &_GrVideoDriverLINUXFB,
#endif
#endif
#if defined(__WIN32__) && !defined(__SDL__)
    &_GrVideoDriverWIN32,
#endif
#if defined(__SDL__)
    &_GrVideoDriverSDL,
#endif
    &_GrDriverMEM,
    NULL
};

