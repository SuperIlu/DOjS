/**
 ** mach64.c ---- ATI MACH64 driver
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
#include "highlow.h"
#include "ioport.h"
#include "memmode.h"

static void setbank(int bk)
{
	register unsigned b1;
	if(inport_b(0x6aec) & 4) {
	    bk <<= 1;
	    b1 = bk+1;
	    outport_b(0x56ec,bk);
	    outport_b(0x56ee,b1);
	    outport_b(0x5aec,bk);
	    outport_b(0x5aee,b1);
	} else {
	    b1 = (GR_int16u)(bk&7);
	    b1 = ( ((b1<<4)+b1) << 9) + 0xb2;
	    outport_w(0x1ce,b1);
	}
}

static void setrwbanks(int rb,int wb)
{
	if(inport_b(0x6aec) & 4) {
	    wb <<= 1;
	    outport_b(0x56ec,(wb + 0));
	    outport_b(0x56ee,(wb + 1));
	    rb <<= 1;
	    outport_b(0x5aec,(rb + 0));
	    outport_b(0x5aee,(rb + 1));
	} else {
	    register unsigned b = ((((rb&7) << 4) + (wb&7)) << 9) + 0xb2;
	    outport_w(0x1ce,b);
	}
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
    setrwbanks,                         /* double bank set function */
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
    setrwbanks,                         /* double bank set function */
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
    setrwbanks,                         /* double bank set function */
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
    setrwbanks,                         /* double bank set function */
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
    setrwbanks,                         /* double bank set function */
    NULL                                /* color loader */
};

static GrVideoMode modes[] = {
    /* pres.  bpp wdt   hgt   BIOS   scan  priv. &ext                             */
    {  TRUE,  4,  100,  25,   0x21,  200,  0,    &_GrViDrvEGAVGAtextModeExt       },
    {  TRUE,  4,  100,  28,   0x21,  200,  0,    &_GrViDrvEGAVGAcustomTextModeExt },
    {  TRUE,  4,  100,  30,   0x22,  200,  0,    &_GrViDrvEGAVGAtextModeExt       },
    {  TRUE,  4,  100,  34,   0x22,  200,  0,    &_GrViDrvEGAVGAcustomTextModeExt },
    {  TRUE,  4,  100,  50,   0x21,  200,  0,    &_GrViDrvEGAVGAcustomTextModeExt },
    {  TRUE,  4,  132,  25,   0x23,  264,  0,    &_GrViDrvEGAVGAtextModeExt       },
    {  TRUE,  4,  132,  28,   0x23,  264,  0,    &_GrViDrvEGAVGAcustomTextModeExt },
    {  TRUE,  4,  132,  44,   0x33,  264,  0,    &_GrViDrvEGAVGAtextModeExt       },
    {  TRUE,  4,  132,  50,   0x23,  264,  0,    &_GrViDrvEGAVGAcustomTextModeExt },
    {  TRUE,  4,  800,  600,  0x102, 100,  0,    &gr4ext                          },
    {  TRUE,  4,  1024, 768,  0x104, 128,  0,    &gr4ext                          },
    {  TRUE,  8,  640,  480,  0x101, 640,  0,    &gr8ext                          },
    {  TRUE,  8,  800,  600,  0x103, 800,  0,    &gr8ext                          },
    {  TRUE,  8,  1024, 768,  0x105, 1024, 0,    &gr8ext                          },
    {  TRUE,  8,  1280, 1024, 0x107, 1280, 0,    &gr8ext                          },
    {  TRUE,  15, 640,  480,  0x110, 1280, 0,    &gr15ext                         },
    {  TRUE,  15, 800,  600,  0x113, 1600, 0,    &gr15ext                         },
    {  TRUE,  15, 1024, 768,  0x116, 2048, 0,    &gr15ext                         },
    {  TRUE,  16, 640,  480,  0x111, 1280, 0,    &gr16ext                         },
    {  TRUE,  16, 800,  600,  0x114, 1600, 0,    &gr16ext                         },
    {  TRUE,  16, 1024, 768,  0x117, 2048, 0,    &gr16ext                         },
    {  TRUE,  24, 640,  480,  0x112, 1920, 0,    &gr24ext                         },
    {  TRUE,  24, 800,  600,  0x115, 2400, 0,    &gr24ext                         }
};

GrVideoDriver _GrVideoDriverMACH64 = {
    "mach64",                           /* name */
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

