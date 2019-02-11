/**
 ** ftable.c ---- a table of available frame drivers
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

GrFrameDriver *_GrFrameDriverTable[] = {
/* first the drivers for video based context */
#if defined(__DJGPP__) || defined(__TURBOC__) || defined (__WATCOMC__)
    &_GrFrameDriverHERC1,
#endif
#if defined(__MSDOS__) || (defined(__linux__) && !defined(__XWIN__))
#ifdef  __i386__
    &_GrFrameDriverEGAVGA1,
    &_GrFrameDriverEGA4,
    &_GrFrameDriverSVGA4,
    &_GrFrameDriverVGA8X,
#endif   
    &_GrFrameDriverSVGA8,
    &_GrFrameDriverSVGA16,
    &_GrFrameDriverSVGA24,
    &_GrFrameDriverSVGA32L,
    &_GrFrameDriverSVGA32H,
#endif
#if defined(__DJGPP__) || defined(XF86DGA_FRAMEBUFFER) \
  || ( defined(LFB_BY_NEAR_POINTER) && !defined(__WIN32__) ) \
  || ( defined(__WATCOMC__) && defined ( __386__ ) )
    &_GrFrameDriverSVGA8_LFB,
    &_GrFrameDriverSVGA16_LFB,
    &_GrFrameDriverSVGA24_LFB,
    &_GrFrameDriverSVGA32L_LFB,
    &_GrFrameDriverSVGA32H_LFB,
#endif
#if defined(__XWIN__) && !defined(__SDL__)
    &_GrFrameDriverXWIN8,
    &_GrFrameDriverXWIN16,
    &_GrFrameDriverXWIN24,
    &_GrFrameDriverXWIN32L,
    &_GrFrameDriverXWIN32H,
#endif
#if defined(__WIN32__) && !defined(__SDL__)
    &_GrFrameDriverWIN32_8,
    &_GrFrameDriverWIN32_24,
#endif
#if defined(__SDL__)
    &_GrFrameDriverSDL8,
    &_GrFrameDriverSDL16,
    &_GrFrameDriverSDL24,
    &_GrFrameDriverSDL32L,
    &_GrFrameDriverSDL32H,
#endif
/* now the drivers for RAM based context */
    &_GrFrameDriverRAM1,
    &_GrFrameDriverRAM4,
    &_GrFrameDriverRAM8,
    &_GrFrameDriverRAM16,
#ifdef GRX_USE_RAM3x8
    &_GrFrameDriverRAM3x8,
#else
    &_GrFrameDriverRAM24,
    &_GrFrameDriverRAM32L,
    &_GrFrameDriverRAM32H,
#endif
    NULL
};

