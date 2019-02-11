/**
 ** stdega.c ---- the standard EGA driver
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
#include "int86.h"
#include "memfill.h"

void _GrViDrvLoadColorEGA4(int c,int r,int g,int b)
{
	Int86Regs rg;
	sttzero(&rg);
	IREG_AX(rg)  = 0x1000;
	IREG_BX(rg)  = c & 0x0f;
	IREG_BX(rg) |= ((r & 0x40) << 7) | ((r & 0x80) << 3);
	IREG_BX(rg) |= ((g & 0x40) << 6) | ((g & 0x80) << 2);
	IREG_BX(rg) |= ((b & 0x40) << 5) | ((b & 0x80) << 1);
	int10(&rg);
}

static GrVideoModeExt gr1ext = {
    GR_frameEGAVGA1,                    /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xa000,0),                    /* frame buffer address */
    { 1, 1, 1 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    _GrViDrvSetEGAVGAmode,              /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    _GrViDrvLoadColorEGA4               /* color loader */
};

static GrVideoModeExt gr4ext = {
    GR_frameEGA4,                       /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xa000,0),                    /* frame buffer address */
    { 2, 2, 2 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    _GrViDrvSetEGAVGAmode,              /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    _GrViDrvLoadColorEGA4               /* color loader */
};

static GrVideoMode modes[] = {
    /* pres.  bpp wdt   hgt   BIOS   scan  priv. &ext                             */
    {  TRUE,  1,  80,   25,   0x07,  160,  0,    &_GrViDrvEGAVGAtextModeExt       },
    {  TRUE,  1,  80,   43,   0x07,  160,  0,    &_GrViDrvEGAVGAcustomTextModeExt },
    {  TRUE,  4,  40,   25,   0x01,  80,   0,    &_GrViDrvEGAVGAtextModeExt       },
    {  TRUE,  4,  80,   25,   0x03,  160,  0,    &_GrViDrvEGAVGAtextModeExt       },
    {  TRUE,  4,  80,   43,   0x03,  160,  0,    &_GrViDrvEGAVGAcustomTextModeExt },
    {  TRUE,  1,  320,  200,  0x0d,  40,   0,    &gr1ext                          },
    {  TRUE,  1,  640,  200,  0x0e,  80,   0,    &gr1ext                          },
    {  TRUE,  1,  640,  350,  0x10,  80,   0,    &gr1ext                          },
    {  TRUE,  4,  320,  200,  0x0d,  40,   0,    &gr4ext                          },
    {  TRUE,  4,  640,  200,  0x0e,  80,   0,    &gr4ext                          },
    {  TRUE,  4,  640,  350,  0x10,  80,   0,    &gr4ext                          }
};

GrVideoDriver _GrVideoDriverSTDEGA = {
    "stdega",                           /* name */
    GR_EGA,                             /* adapter type */
    NULL,                               /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    _GrViDrvDetectEGA,                  /* detection routine */
    _GrViDrvInitEGAVGA,                 /* initialization routine */
    _GrViDrvResetEGAVGA,                /* reset routine */
    _gr_selectmode,                     /* standard mode select routine */
    0                                   /* no additional capabilities */
};

