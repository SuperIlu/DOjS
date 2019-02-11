/**
 ** et400.cC ---- Tseng ET4000 video driver
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
 ** Contributions by: (See "doc/credits.doc" for details)
 ** Christian Domp (alma.student.uni-kl.de)
 **
 **/

#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"
#include "int86.h"
#include "ioport.h"
#include "memfill.h"

static void setbank(int bk)
{
	outport_b(0x3cd,((bk & 15) | ((bk & 15) << 4)));
}

static void setrwbanks(int rb,int wb)
{
	outport_b(0x3cd,((wb & 15) | ((rb & 15) << 4)));
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

static int setmode15(GrVideoMode *mp,int noclear)
{
	if(_GrViDrvSetEGAVGAmode(mp,noclear)) {
	    Int86Regs r;
	    sttzero(&r);
	    IREG_AX(r) = 0x10f0;
	    IREG_BX(r) = mp->mode;
	    int10(&r);
	    if(IREG_AX(r) == 0x10) return(TRUE);
	}
	return(FALSE);
}

static GrVideoModeExt gr15ext = {
    GR_frameSVGA16,                     /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xa000,0),                    /* frame buffer address */
    { 5,  5,  5 },                      /* color precisions */
    { 10, 5,  0 },                      /* color component bit positions */
    0,                                  /* mode flag bits */
    setmode15,                          /* mode set */
    _GrViDrvVESAsetVirtualSize,         /* virtual size set */
    _GrViDrvVESAvirtualScroll,          /* virtual scroll */
    setbank,                            /* bank set function */
    setrwbanks,                         /* double bank set function */
    NULL                                /* color loader */
};

static int setmode16(GrVideoMode *mp,int noclear)
{
    if(setmode15(mp,noclear)) {         /* set 15-bit mode */
	Int86Regs r;
	sttzero(&r);
	IREG_AX(r) = 0x10f2;
	IREG_BX(r) = 2;
	int10(&r);                      /* switch to 16-bit mode */
	if(IREG_AX(r) == 0x10) return(TRUE);
    }
    return(FALSE);
}

static GrVideoModeExt gr16ext = {
    GR_frameSVGA16,                     /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xa000,0),                    /* frame buffer address */
    { 5,  6,  5 },                      /* color precisions */
    { 11, 5,  0 },                      /* color component bit positions */
    0,                                  /* mode flag bits */
    setmode16,                          /* mode set */
    _GrViDrvVESAsetVirtualSize,         /* virtual size set */
    _GrViDrvVESAvirtualScroll,          /* virtual scroll */
    setbank,                            /* bank set function */
    setrwbanks,                         /* double bank set function */
    NULL                                /* color loader */
};

static int setmode24(GrVideoMode *mp,int noclear)
{
    if(_GrViDrvSetEGAVGAmode(mp,noclear)) {
	Int86Regs r;
	sttzero(&r);
	IREG_AX(r) = 0x10f0;
	IREG_BX(r) = (mp->mode<<8)|0xff;
	int10(&r);
	if(IREG_AX(r) == 0x10) return(TRUE);
    }
    return(FALSE);
}

static GrVideoModeExt gr24ext = {
    GR_frameSVGA24,                     /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xa000,0),                    /* frame buffer address */
    { 8,  8,  8 },                      /* color precisions */
    { 16, 8,  0 },                      /* color component bit positions */
    0,                                  /* mode flag bits */
    setmode24,                          /* mode set */
    _GrViDrvVESAsetVirtualSize,         /* virtual size set */
    _GrViDrvVESAvirtualScroll,          /* virtual scroll */
    setbank,                            /* bank set function */
    setrwbanks,                         /* double bank set function */
    NULL                                /* color loader */
};

static GrVideoMode modes[] = {
    /* pres.  bpp wdt   hgt   BIOS   scan  priv. &ext                           */
    {  TRUE,  4,  80,   60,   0x26,  160,  0,    &_GrViDrvEGAVGAtextModeExt     },
    {  TRUE,  4,  100,  40,   0x2a,  200,  0,    &_GrViDrvEGAVGAtextModeExt     },
    {  TRUE,  4,  132,  25,   0x23,  264,  0,    &_GrViDrvEGAVGAtextModeExt     },
    {  TRUE,  4,  132,  28,   0x24,  264,  0,    &_GrViDrvEGAVGAtextModeExt     },
    {  TRUE,  4,  132,  44,   0x22,  264,  0,    &_GrViDrvEGAVGAtextModeExt     },
    {  TRUE,  4,  132,  50,   0x61,  264,  0,    &_GrViDrvEGAVGAtextModeExt     },
    {  TRUE,  4,  132,  60,   0x21,  264,  0,    &_GrViDrvEGAVGAtextModeExt     },
    {  TRUE,  4,  800,  600,  0x29,  100,  0,    &gr4ext                        },
    {  TRUE,  4,  1024, 768,  0x37,  128,  0,    &gr4ext                        },
    {  TRUE,  4,  1280, 1024, 0x3d,  160,  0,    &gr4ext                        },
    {  TRUE,  8,  640,  350,  0x2d,  640,  0,    &gr8ext                        },
    {  TRUE,  8,  640,  400,  0x2f,  640,  0,    &gr8ext                        },
    {  TRUE,  8,  640,  480,  0x2e,  640,  0,    &gr8ext                        },
    {  TRUE,  8,  800,  600,  0x30,  800,  0,    &gr8ext                        },
    {  TRUE,  8,  1024, 768,  0x38,  1024, 0,    &gr8ext                        },
    {  FALSE, 15, 320,  200,  0x13,  640,  0,    &gr15ext                       },
    {  FALSE, 15, 640,  350,  0x2d,  1280, 0,    &gr15ext                       },
    {  FALSE, 15, 640,  400,  0x2f,  1280, 0,    &gr15ext                       },
    {  FALSE, 15, 640,  480,  0x2e,  1280, 0,    &gr15ext                       },
    {  FALSE, 15, 800,  600,  0x30,  1600, 0,    &gr15ext                       },
    {  FALSE, 16, 320,  200,  0x13,  640,  0,    &gr16ext                       },
    {  FALSE, 16, 640,  350,  0x2d,  1280, 0,    &gr16ext                       },
    {  FALSE, 16, 640,  400,  0x2f,  1280, 0,    &gr16ext                       },
    {  FALSE, 16, 640,  480,  0x2e,  1280, 0,    &gr16ext                       },
    {  FALSE, 16, 800,  600,  0x30,  1600, 0,    &gr16ext                       },
    {  FALSE, 24, 640,  350,  0x2d,  1920, 0,    &gr24ext                       },
    {  FALSE, 24, 640,  400,  0x2f,  1920, 0,    &gr24ext                       },
    {  FALSE, 24, 640,  480,  0x2e,  1920, 0,    &gr24ext                       }
};

static int init(char *options)
{
	if(_GrViDrvInitEGAVGA(options)) {
	    Int86Regs r;
	    sttzero(&r);
	    IREG_AX(r) = 0x10f1;
	    int10(&r);
	    if((IREG_AX(r) == 0x10) && (IREG_BX(r) >= 1)) {
		GrVideoMode *mp = &modes[itemsof(modes)];
		while(--mp >= modes) {
		    switch(mp->bpp) {
		      case 24:
			if(IREG_BX(r) < 3) break;
		      case 16:
			if(IREG_BX(r) < 2) break;
		      case 15:
			mp->present = TRUE;
		    }
		}
	    }
	    return(TRUE);
	}
	return(FALSE);
}

GrVideoDriver _GrVideoDriverET4000 = {
    "et4000",                           /* name */
    GR_VGA,                             /* adapter type */
    &_GrVideoDriverSTDVGA,              /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    NULL,                               /* detection routine */
    init,                               /* initialization routine */
    _GrViDrvResetEGAVGA,                /* reset routine */
    _gr_selectmode,                     /* standard mode select routine */
    0                                   /* no additional capabilities */
};

