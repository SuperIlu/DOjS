/**
 ** herc.c ---- Hercules video driver
 **
 ** Author:     Ulrich Leodolter
 ** E-mail:     ulrich@lab1.psy.univie.ac.at
 ** Date:       Tue Nov 14 15:55:56 1995
 ** Comment:    Driver code partly from X11R6/XFree86 (see below)
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

/* $XFree86: xc/programs/Xserver/hw/xfree86/mono/drivers/hercules/hercules.c,v 3.2 1994/09/23 10:18:52 dawes Exp $ */
/*
 * MONO: Driver family for interlaced and banked monochrome video adaptors
 * Pascal Haible 8/93, 3/94, 4/94 haible@IZFM.Uni-Stuttgart.DE
 *
 * mono/drivers/hercules
 *
 * original:
 * hga2/drivers/hga6845.c
 *
 * Author:  Davor Matic, dmatic@athena.mit.edu
 *
 * heavily edited for R6 by
 * Pascal Haible 4/94 haible@IZFM.Uni-Stuttgart.DE
 *
 */

#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"
#include "int86.h"
#include "ioport.h"
#include "memfill.h"

/*
 * Define the generic HGA I/O Ports
 */
#define HGA_INDEX       0x3B4
#define HGA_DATA        0x3B5
#define HGA_MODE        0x3B8
#define HGA_STATUS      0x3BA
#define HGA_CONFIG      0x3BF

/*
 * Since the conf and mode registers are write only, we need to keep
 * a local copy of the state here.  The initial state is assumed to be:
 * conf: enable setting of graphics mode, and disable page one
 *       (allows coexistence with a color graphics board)
 * mode: text, deactivate screen, enable text blink, and page zero at 0xB0000
 */
static unsigned char static_config = 0x01;
static unsigned char static_mode = 0x20;

/*
 * Since the table of 6845 registers is write only, we need to keep
 * a local copy of the state here.  The initial state is assumed to
 * be 80x25 text mode.
 */
static unsigned char
  static_tbl[] = {0x61, 0x50, 0x52, 0x0F, 0x19, 0x06, 0x19, 0x19,
		  0x02, 0x0D, 0x0B, 0x0C, 0x00, 0x00, 0x00, 0x28};

static unsigned char init_config = 0x03;
static unsigned char init_mode = 0x0A;
static unsigned char       /* 720x348 graphics mode parameters */
  init_tbl[] = {0x35, 0x2D, 0x2E, 0x07, 0x5B, 0x02, 0x57, 0x57,
		  0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A};
/*
 * Video ram size in Kbytes
 */
static int videoRam = 64;

static int setmode(GrVideoMode *mp,int noclear)
{
  int i;

  if (noclear == FALSE) {
    long far *p;
    long far *q;

    p = (long far *)LINP_PTR(mp->extinfo->frame);
    q = p + (videoRam * 1024) / sizeof(*p);
    setup_far_selector (LINP_SEL(mp->extinfo->frame));
    for (; p < q; p++) poke_l_f(p,0L);
  }
  outport_b (HGA_CONFIG, static_config = init_config);
  outport_b (HGA_MODE, static_mode = init_mode);
  for (i = 0; i < 16; i++) {
    outport_b (HGA_INDEX, i);
    outport_b (HGA_DATA, static_tbl[i] = init_tbl[i]);
  }
  return(TRUE);
}

static void setbank(int bk)
{
}

static void setrwbanks(int rb,int wb)
{
}

static void loadcolor (int c,int r,int g,int b)
{
}

static GrVideoModeExt grherc1ext = {
    GR_frameHERC1,                      /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xb000,0),                    /* frame buffer address */
    { 1, 1, 1 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    setmode,                            /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    setbank,                            /* bank set function */
    setrwbanks,                         /* double bank set function */
    loadcolor,                          /* color loader */
};

static GrVideoMode modes[] = {
    /* pres.  bpp wdt   hgt   BIOS   scan  priv. &ext */
    {  TRUE,  1,  80,   25,   0x07,  160,  0,    &_GrViDrvEGAVGAtextModeExt },
    {  TRUE,  1,  720,  348,  0x00,   92,  0,    &grherc1ext }
};

static int detect (void)
{
#define DSP_VSYNC_MASK  0x80
#define DSP_ID_MASK  0x70
  unsigned char dsp, dsp_old;
  int i, cnt;

  /*
   * Checks if there is a HGA 6845 based bard in the system.
   * The following loop tries to see if the Hercules display
   * status port register is counting vertical syncs (50Hz).
   */
  cnt = 0;
  dsp_old = inport_b(HGA_STATUS) & DSP_VSYNC_MASK;
  for (i = 0; i < 0x100; i++) {
    int j;
    for(j = 0; j < 0x100; j++) {
       dsp = inport_b(HGA_STATUS) & DSP_VSYNC_MASK;
       if (dsp != dsp_old)
	  cnt++;
       dsp_old = dsp;
    }
  }

  /* If there are active sync changes, we found a Hercules board. */
  if (cnt) {
    videoRam = 64;

    /* The Plus and InColor versions have an ID code as well. */
    dsp = inport_b(HGA_STATUS) & DSP_ID_MASK;
    switch(dsp) {
    case 0x10:        /* Plus */
      videoRam = 64;
      break;
    case 0x50:        /* InColor */
      videoRam = 256;
      break;
    }
  } else { /* there is no hga card */
    return(FALSE);
  }
  return(TRUE);
}

GrVideoDriver _GrVideoDriverHERC = {
    "herc",                             /* name */
    GR_HERC,                            /* adapter type */
    NULL,                               /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    detect,                             /* detection routine */
    NULL,                               /* initialization routine */
    _GrViDrvResetEGAVGA,                /* reset routine */
    _gr_selectmode,                     /* standard mode select routine */
    0                                   /* no additional capabilities */
};

