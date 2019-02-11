/**
 ** cl5426.c ---- Cirrus 5426 (or higher) driver
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
#include "arith.h"
#include "ioport.h"
#include "memmode.h"

static void setbank(int bk)
{
	outport_w(0x3ce,((bk << 12) | 9));
}

static GrVideoModeExt gr4ext = {
    GR_frameSVGA4,                      /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xa000,0),                    /* frame buffer address */
    { 6, 6, 6 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    _GrViDrvSetEGAVGAmode,              /* mode set */
    _GrViDrvVESAsetVirtualSize,         /* virtual size set */
    _GrViDrvVESAvirtualScroll,          /* virtual scroll */
    setbank,                            /* bank set function */
    NULL,                               /* double bank set function */
    _GrViDrvLoadColorVGA4               /* color loader */
};

static GrVideoModeExt gr8ext = {
    GR_frameSVGA8,                      /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xa000,0),                    /* frame buffer address */
    { 6, 6, 6 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    GR_VMODEF_FAST_SVGA8,               /* mode flag bits */
    _GrViDrvSetEGAVGAmode,              /* mode set */
    _GrViDrvVESAsetVirtualSize,         /* virtual size set */
    _GrViDrvVESAvirtualScroll,          /* virtual scroll */
    setbank,                            /* bank set function */
    NULL,                               /* double bank set function */
    _GrViDrvLoadColorVGA8               /* color loader */
};

static GrVideoModeExt gr15ext = {
    GR_frameSVGA16,                     /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xa000,0),                    /* frame buffer address */
    { 5,  5,  5 },                      /* color precisions */
    { 10, 5,  0 },                      /* color component bit positions */
    0,                                  /* mode flag bits */
    _GrViDrvSetEGAVGAmode,              /* mode set */
    _GrViDrvVESAsetVirtualSize,         /* virtual size set */
    _GrViDrvVESAvirtualScroll,          /* virtual scroll */
    setbank,                            /* bank set function */
    NULL,                               /* double bank set function */
    NULL                                /* color loader */
};

static GrVideoModeExt gr16ext = {
    GR_frameSVGA16,                     /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xa000,0),                    /* frame buffer address */
    { 5,  6,  5 },                      /* color precisions */
    { 11, 5,  0 },                      /* color component bit positions */
    0,                                  /* mode flag bits */
    _GrViDrvSetEGAVGAmode,              /* mode set */
    _GrViDrvVESAsetVirtualSize,         /* virtual size set */
    _GrViDrvVESAvirtualScroll,          /* virtual scroll */
    setbank,                            /* bank set function */
    NULL,                               /* double bank set function */
    NULL                                /* color loader */
};

static GrVideoModeExt gr24ext = {
    GR_frameSVGA24,                     /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xa000,0),                    /* frame buffer address */
    { 8,  8,  8 },                      /* color precisions */
    { 16, 8,  0 },                      /* color component bit positions */
    0,                                  /* mode flag bits */
    _GrViDrvSetEGAVGAmode,              /* mode set */
    _GrViDrvVESAsetVirtualSize,         /* virtual size set */
    _GrViDrvVESAvirtualScroll,          /* virtual scroll */
    setbank,                            /* bank set function */
    NULL,                               /* double bank set function */
    NULL                                /* color loader */
};

static GrVideoMode modes[] = {
    /* pres.  bpp wdt   hgt   BIOS   scan  priv. &ext                             */
    {  TRUE,  4,  132,  25,   0x14,  264,  0,    &_GrViDrvEGAVGAtextModeExt       },
    {  TRUE,  4,  132,  28,   0x14,  264,  0,    &_GrViDrvEGAVGAcustomTextModeExt },
    {  TRUE,  4,  132,  43,   0x54,  264,  0,    &_GrViDrvEGAVGAtextModeExt       },
    {  TRUE,  4,  132,  50,   0x14,  264,  0,    &_GrViDrvEGAVGAcustomTextModeExt },
    {  TRUE,  4,  800,  600,  0x102, 100,  0,    &gr4ext                          },
    {  TRUE,  4,  1024, 768,  0x104, 128,  0,    &gr4ext                          },
    {  TRUE,  4,  1280, 1024, 0x6c,  160,  0,    &gr4ext       /* ????? */        },
    {  TRUE,  8,  640,  480,  0x101, 640,  0,    &gr8ext                          },
    {  TRUE,  8,  800,  600,  0x103, 800,  0,    &gr8ext                          },
    {  TRUE,  8,  1024, 768,  0x105, 1024, 0,    &gr8ext                          },
    {  TRUE,  15, 640,  480,  0x110, 1280, 0,    &gr15ext                         },
    {  TRUE,  15, 800,  600,  0x113, 1600, 0,    &gr15ext                         },
    {  TRUE,  16, 640,  480,  0x111, 1280, 0,    &gr16ext                         },
    {  TRUE,  16, 800,  600,  0x114, 1600, 0,    &gr16ext                         },
    {  TRUE,  24, 640,  480,  0x112, 2048, 0,    &gr24ext                         }
};

GrVideoDriver _GrVideoDriverCL5426 = {
    "cl5426",                           /* name */
    GR_VGA,                             /* adapter type */
    &_GrVideoDriverSTDVGA,              /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    NULL,                               /* detection routine */
    _GrViDrvInitEGAVGA,                 /* initialization routine */
    _GrViDrvResetEGAVGA,                /* reset routine */
    _gr_selectmode,                     /* standard mode select routine */
    0                                   /* no additional capabilities */
};

