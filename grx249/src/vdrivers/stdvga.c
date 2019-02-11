/**
 ** stdvga.c ---- the standard VGA driver
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
 **/

#include <string.h>

#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"
#include "int86.h"
#include "memfill.h"
#include "mempeek.h"
#include "ioport.h"

/* so that GCC in its infinite wisdom does not optimize things away.. */
static volatile unsigned char junk;

static int DACshift = 2;

void _GrViDrvSetDACshift(int shift) {
  GRX_ENTER();
  DACshift = shift;
  GRX_LEAVE();
}

void _GrViDrvLoadColorVGA8(int c,int r,int g,int b) {
  GRX_ENTER();
	int_disable();
	outport_b(0x3c8,c);
	junk += inport_b(0x80);
	outport_b(0x3c9,((unsigned char)r >> DACshift));
	junk += inport_b(0x80);
	outport_b(0x3c9,((unsigned char)g >> DACshift));
	junk += inport_b(0x80);
	outport_b(0x3c9,((unsigned char)b >> DACshift));
	int_enable();
  GRX_LEAVE();
}

void _GrViDrvLoadColorVGA4(int c,int r,int g,int b)
{
	Int86Regs rg;
  GRX_ENTER();
	sttzero(&rg);
	IREG_AX(rg) = 0x1000;
	IREG_BX(rg) = (c & 0x0f) | ((c & 0x0f) << 8);
	int10(&rg);
	_GrViDrvLoadColorVGA8(c,r,g,b);
  GRX_LEAVE();
}

static GrVideoModeExt gr1ext = {
    GR_frameEGAVGA1,                    /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xa000,0),                    /* frame buffer address */
    { 1, 1, 1 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    _GrViDrvSetEGAVGAmode,              /* mode set */
    _GrViDrvVESAsetVirtualSize,         /* virtual size set */
    _GrViDrvVESAvirtualScroll,          /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    _GrViDrvLoadColorVGA4               /* color loader */
};

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
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    _GrViDrvLoadColorVGA4               /* color loader */
};

static GrVideoModeExt gr8ext = {
    GR_frameSVGA8,                      /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xa000,0),                    /* frame buffer address */
    { 6, 6, 6 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    _GrViDrvSetEGAVGAmode,              /* mode set */
    _GrViDrvVESAsetVirtualSize,         /* virtual size set */
    _GrViDrvVESAvirtualScroll,          /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    _GrViDrvLoadColorVGA8               /* color loader */
};

/* ==== tweaked planar 256 color graphics modes (MODE X) ==== */

static struct xtweakdef {
    unsigned char miscreg;
    unsigned char crtc_regs[24];
} xtweaks[] = {
    {                                   /* 320x240 */
	0xe3,
	{
	    0x5f, 0x4f, 0x50, 0x82,
	    0x54, 0x80, 0x0d, 0x3e,
	    0x00, 0x41, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0xea, 0xac, 0xdf, 0x28,
	    0x00, 0xe7, 0x06, 0xe3
	}
    },
    {                                   /* 320x400 */
	0,                              /* don't need to set it */
	{
	    0x5f, 0x4f, 0x50, 0x82,
	    0x54, 0x80, 0xbf, 0x1f,
	    0x00, 0x40, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x9c, 0x8e, 0x8f, 0x28,
	    0x00, 0x96, 0xb9, 0xe3
	}
    },
    {                                   /* 360x480 */
	0xe7,
	{
	    0x6b, 0x59, 0x5a, 0x8e,
	    0x5e, 0x8a, 0x0d, 0x3e,
	    0x00, 0x40, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x31,
	    0xea, 0xac, 0xdf, 0x2d,
	    0x00, 0xe7, 0x06, 0xe3
	}
    }
};

static int setmodex(GrVideoMode *mp,int noclear)
{
  int res = FALSE;
  GRX_ENTER();
	if(((unsigned int)mp->privdata < itemsof(xtweaks)) &&
	   (_GrViDrvSetEGAVGAmode(mp,noclear) != FALSE)) {
	    struct xtweakdef *tp = &xtweaks[mp->privdata];
	    unsigned int i;
	    /* turn off chain4 */
	    outport_w(0x3c4,0x604);
	    if(!noclear) {
		char far *ptr = LINP_PTR(mp->extinfo->frame);
		int size = 0x8000U;
		/* enable all planes */
		outport_w(0x3c4,((0x0f << 8) | 2));
		setup_far_selector(LINP_SEL(mp->extinfo->frame));
		rowfill_w_f(ptr,0,size);
	    }
	    /* wait for vertical retrace */
	    while((inport_b(0x3da) & 8) != 0);
	    while((inport_b(0x3da) & 8) == 0);
	    int_disable();
	    if(tp->miscreg) {
		outport_w(0x3c4,0x100);
		outport_b(0x3c2,tp->miscreg);
		outport_w(0x3c4,0x300);
	    }
	    outport_w(0x3d4,(((tp->crtc_regs[17] & 0x7f) << 8) | 17));
	    for(i = 0; i < itemsof(tp->crtc_regs); i++) {
		outport_w(0x3d4,((tp->crtc_regs[i] << 8) | i));
	    }
	    int_enable();
	    res = TRUE;
	}
  GRX_LEAVE();
  return(res);
}

static GrVideoModeExt gr8xext = {
    GR_frameVGA8X,                      /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xa000,0),                    /* frame buffer address */
    { 6, 6, 6 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    setmodex,                           /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    _GrViDrvLoadColorVGA8               /* color loader */
};

/* ==== tweaked text modes: 90x30, 90x34, 94x30, 94x34 ==== */

static unsigned short ttweaks[][16] = {
    {   /* 90x30 */
	0x0c11,0x6d00,0x5901,0x5a02,0x9003,0x6004,0x8b05,0x0b06,
	0x3e07,0x4f09,0xea10,0x8c11,0xdf12,0x2d13,0xe715,0x0416
    },
    {   /* 90x34 */
	0x0c11,0x6d00,0x5901,0x5a02,0x9003,0x6004,0x8b05,0x0b06,
	0x3e07,0x4d09,0xea10,0x8c11,0xdf12,0x2d13,0xe715,0x0416
    },
    {   /* 94x30 */
	0x0c11,0x6c00,0x5d01,0x5e02,0x8f03,0x6204,0x8e05,0x0b06,
	0x3e07,0x4f09,0xea10,0x8c11,0xdf12,0x2f13,0xe715,0x0416
    },
    {   /* 94x34 */
	0x0c11,0x6c00,0x5d01,0x5e02,0x8f03,0x6204,0x8e05,0x0b06,
	0x3e07,0x4d09,0xea10,0x8c11,0xdf12,0x2f13,0xe715,0x0416
    }
};

static int set_tweaked_text(GrVideoMode *mp,int noclear)
{
  int res = FALSE;
  GRX_ENTER();
	if(((unsigned int)mp->privdata < itemsof(ttweaks)) &&
	   (_GrViDrvSetEGAVGAmode(mp,noclear) != FALSE)) {
	    unsigned int i;
	    /* load 8x14 font for 34 line modes */
	    if(mp->height == 34) {
		Int86Regs r;
		sttzero(&r);
		IREG_AX(r) = 0x1111;
		int10(&r);
	    }
	    /* wait for vertical retrace */
	    while((inport_b(0x3da) & 8) != 0);
	    while((inport_b(0x3da) & 8) == 0);
	    int_disable();
	    outport_b(0x3c2,0xe7);
	    /* set sequencer clocking mode */
	    outport_w(0x3c4,0x101);
	    /* reset data flip-flop to address mode */
	    junk = inport_w(0x3da);
	    outport_b(0x3c0,0x30);
	    /* set mode control register */
	    outport_b(0x3c0,0x04);
	    outport_b(0x3c0,0x33);
	    /* set horizontal pixel pan */
	    outport_b(0x3c0,0x00);
	    for(i = 0; i < itemsof(ttweaks[0]); i++) {
		outport_w(0x3d4,ttweaks[mp->privdata][i]);
	    }
	    setup_far_selector(LINP_SEL(0));
	    /* update BIOS data area */
	    poke_w_f(0x44a,mp->width);
	    poke_b_f(0x484,(mp->height - 1));
	    poke_w_f(0x44c,(mp->width * mp->height * 2));
	    int_enable();
	    res = TRUE;
	}
  GRX_LEAVE();
  return res;
}

static GrVideoModeExt twtext = {
    GR_frameText,                       /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xb800,0),                    /* frame buffer address */
    { 0, 0, 0 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    set_tweaked_text,                   /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    NULL                                /* color loader */
};

static GrVideoMode modes[] = {
    /* pres.  bpp wdt   hgt   BIOS   scan  priv. &ext                             */
    {  TRUE,  1,  80,   25,   0x07,  160,  0,    &_GrViDrvEGAVGAtextModeExt       },
    {  TRUE,  1,  80,   28,   0x07,  160,  0,    &_GrViDrvEGAVGAcustomTextModeExt },
    {  TRUE,  1,  80,   50,   0x07,  160,  0,    &_GrViDrvEGAVGAcustomTextModeExt },
    {  TRUE,  4,  40,   25,   0x01,  80,   0,    &_GrViDrvEGAVGAtextModeExt       },
    {  TRUE,  4,  80,   25,   0x03,  160,  0,    &_GrViDrvEGAVGAtextModeExt       },
    {  TRUE,  4,  80,   28,   0x03,  160,  0,    &_GrViDrvEGAVGAcustomTextModeExt },
    {  TRUE,  4,  80,   50,   0x03,  160,  0,    &_GrViDrvEGAVGAcustomTextModeExt },
    {  TRUE,  4,  90,   30,   0x03,  180,  0,    &twtext                          },
    {  TRUE,  4,  90,   34,   0x03,  180,  1,    &twtext                          },
    {  TRUE,  4,  94,   30,   0x03,  188,  2,    &twtext                          },
    {  TRUE,  4,  94,   34,   0x03,  188,  3,    &twtext                          },
    {  TRUE,  1,  320,  200,  0x0d,  40,   0,    &gr1ext                          },
    {  TRUE,  1,  640,  200,  0x0e,  80,   0,    &gr1ext                          },
    {  TRUE,  1,  640,  350,  0x10,  80,   0,    &gr1ext                          },
    {  TRUE,  1,  640,  480,  0x12,  80,   0,    &gr1ext                          },
    {  FALSE, 1,  800,  600,  0x6a,  100,  0,    &gr1ext                          },
    {  TRUE,  4,  320,  200,  0x0d,  40,   0,    &gr4ext                          },
    {  TRUE,  4,  640,  200,  0x0e,  80,   0,    &gr4ext                          },
    {  TRUE,  4,  640,  350,  0x10,  80,   0,    &gr4ext                          },
    {  TRUE,  4,  640,  480,  0x12,  80,   0,    &gr4ext                          },
    {  FALSE, 4,  800,  600,  0x6a,  100,  0,    &gr4ext                          },
    {  TRUE,  8,  320,  200,  0x13,  320,  0,    &gr8ext                          },
    {  TRUE,  8,  320,  240,  0x13,  80,   0,    &gr8xext                         },
    {  TRUE,  8,  320,  400,  0x13,  80,   1,    &gr8xext                         },
    {  TRUE,  8,  360,  480,  0x13,  90,   2,    &gr8xext                         }
};

static int init(char *options)
{
  int res = FALSE;
  GRX_ENTER();
	if(_GrViDrvInitEGAVGA(options)) {
	    if(options && (strncmp(options,"svga",4) == 0)) {
		int svgamode;
		GrVideoMode *mp;
		if(sscanf(options,"svga=%x",&svgamode) != 1) svgamode = 0;
		for(mp = modes; mp < &modes[itemsof(modes)]; mp++) {
		    if(mp->width == 800) {
			mp->present = TRUE;
			mp->mode    = ((svgamode > 0x13) && (svgamode <= 0x110)) ? svgamode : 0x6a;
		    }
		}
	    }
	    res = TRUE;
	}
  GRX_LEAVE();
  return res;
}

GrVideoDriver _GrVideoDriverSTDVGA = {
    "stdvga",                           /* name */
    GR_VGA,                             /* adapter type */
    NULL,                               /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    _GrViDrvDetectVGA,                  /* detection routine */
    init,                               /* initialization routine */
    _GrViDrvResetEGAVGA,                /* reset routine */
    _gr_selectmode,                     /* standard mode select routine */
    0                                   /* no additional capabilities */
};

