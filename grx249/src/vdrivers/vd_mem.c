/**
 ** vd_mem.c ---- driver for creating image in memory for later exporting
 **
 ** Author:  Andris Pavenis
 ** [e-mail: pavenis@acad.latnet.lv]
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

#include <stdio.h>

#include "libgrx.h"
#include "grdriver.h"
#include "allocate.h"
#include "arith.h"
#include "int86.h"
#include "memfill.h"


static  char far * MemBuf = NULL;
static  unsigned long MemBufSze = 0;

static void FreeMemBuf(void) {
  if (MemBuf) farfree(MemBuf);
  MemBuf = NULL;
  MemBufSze = 0;
}

static int AllocMemBuf(unsigned long sze) {
  int clear = 1;
  if (!MemBuf) {
    MemBuf = farcalloc(1,(size_t)sze);
    if (!MemBuf) return 0;
    MemBufSze = sze;
    clear = 0;
  }
  if (MemBufSze < sze) {
    MemBuf = farrealloc(MemBuf,(size_t)sze);
    if (!MemBuf) return 0;
    MemBufSze = sze;
  }
  if (clear) memzero(MemBuf,sze);
  return 1;
}

static int mem_setmode (GrVideoMode *mp,int noclear);


static GrVideoModeExt gr1ext = {
    GR_frameRAM1,                       /* frame driver */
    NULL,                               /* frame driver override */
    0,                                  /* frame buffer address */
    { 1, 1, 1 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    GR_VMODEF_MEMORY,                   /* mode flag bits */
    mem_setmode,                        /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    NULL                                /* color loader */
};

static GrVideoModeExt gr4ext = {
    GR_frameRAM4,                       /* frame driver */
    NULL,                               /* frame driver override */
    NULL,                               /* frame buffer address */
    { 8, 8, 8 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    GR_VMODEF_MEMORY,                   /* mode flag bits */
    mem_setmode,                        /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    NULL                                /* color loader */
};

static GrVideoModeExt gr8ext = {
    GR_frameRAM8,                       /* frame driver */
    NULL,                               /* frame driver override */
    NULL,                               /* frame buffer address */
    { 8, 8, 8 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    GR_VMODEF_MEMORY,                   /* mode flag bits */
    mem_setmode,                        /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    NULL                                /* color loader */
};

static GrVideoModeExt gr24ext = {
#ifdef GRX_USE_RAM3x8
    GR_frameRAM3x8,                     /* frame driver */
#else
    GR_frameRAM24,                      /* frame driver */
#endif
    NULL,                               /* frame driver override */
    NULL,                               /* frame buffer address */
    { 8, 8, 8 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    GR_VMODEF_MEMORY,                   /* mode flag bits */
    mem_setmode,                        /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    NULL                                /* color loader */
};


static int dummymode (GrVideoMode * mp , int noclear )
{
    FreeMemBuf();
    return TRUE;
}


GrVideoModeExt   dummyExt = {
    GR_frameText,                       /* frame driver */
    NULL,                               /* frame driver override */
    MK_FP(0xb800,0),                    /* frame buffer address */
    { 0, 0, 0 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    dummymode,                          /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    NULL                                /* color loader */
};




static GrVideoMode modes[] = {
    /* pres.  bpp wdt   hgt   BIOS   scan  priv. &ext                             */
    {  TRUE,  1,  640,  480,  0x00,   80,    0,  &gr1ext                          },
    {  TRUE,  4,  640,  480,  0x00,  320,    0,  &gr4ext                          },
    {  TRUE,  8,  640,  480,  0x00,  640,    0,  &gr8ext                          },
    {  TRUE, 24,  640,  480,  0x00, 1920,    0,  &gr24ext                         },
    {  TRUE,  1,   80,   25,  0x00,  160,    0,  &dummyExt                        }
};



static int mem_setmode (GrVideoMode *mp,int noclear)
{
     return MemBuf ? TRUE : FALSE;
}



static GrVideoMode * mem_selectmode ( GrVideoDriver * drv, int w, int h,
				      int bpp, int txt, unsigned int * ep )
{
    int  index;
    unsigned long  size;
    int  LineOffset;

    if (txt) return _gr_selectmode (drv,w,h,bpp,txt,ep);
/* why ???
    if (w<320) w=320;
    if (h<240) h=240;
*/
    if (w < 1 || h < 1) return NULL;

    switch (bpp)
      {
	 case 1:   index = 0;
		   LineOffset = (w + 7) >> 3;
		   size = h;
		   break;
	 case 4:   index = 1;
		   LineOffset = (w + 7) >> 3;
		   size = 4*h;
		   break;
	 case 8:   index = 2;
		   LineOffset = w;
		   size = h;
		   break;
	 case 24:  index = 3;
#ifdef GRX_USE_RAM3x8
                   LineOffset = w;
		   size = 3*h;
#else
		   LineOffset = 3*w;
		   size = h;
#endif
		   break;
	 default:  return NULL;
      }

    LineOffset = (LineOffset+7) & (~7); /* align rows to 64bit boundary */
    size *= LineOffset;

    if (((size_t)size) != size) return NULL;

			       /* why ???       */
    modes[index].width       = /* w<320 ? 320 : */ w;
    modes[index].height      = /* h<200 ? 200 : */ h;
    modes[index].bpp         = bpp;
    modes[index].lineoffset  = LineOffset;

    if ( AllocMemBuf(size) ) {
	modes[index].extinfo->frame = MemBuf;
        return _gr_selectmode (drv,w,h,bpp,txt,ep);
    }
    return FALSE;
}


/*
static int detect (void)
{
	return TRUE;
}
*/

static void mem_reset (void)
{
    if(DRVINFO->moderestore) {
      FreeMemBuf();
    }
}


GrVideoDriver _GrDriverMEM = {
    "memory",                           /* name */
    GR_MEM,                             /* adapter type */
    NULL,                               /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    NULL, /* detect, */                 /* detection routine */
    NULL,                               /* initialization routine */
    mem_reset,                          /* reset routine */
    mem_selectmode,                     /* special mode select routine */
    GR_DRIVERF_USER_RESOLUTION          /* arbitrary resolution possible */
};

