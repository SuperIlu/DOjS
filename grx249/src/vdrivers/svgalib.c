/**
 ** svgalib.c ---- Linux driver, i.e. an interface to SVGALIB
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

#include <string.h>
#include <vga.h>
#include <unistd.h>

#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"
#include "memcopy.h"
#include "memfill.h"

#define  NUM_MODES    80                /* max # of supported modes */
#define  NUM_EXTS     15                /* max # of mode extensions */

static int initted  = (-1);
static int initmode = 0;
static int isEGA;

static int detect(void)
{
	if(initted < 0) {
#if 0
	    /* make sure VGA will map to 64K boundary ... */
	    long endmem = (long)(sbrk(0));
	    if((endmem & 0xffffL) != 0) {
		brk((void *)((endmem + 0xffffL) & ~0xffffL));
	    }
#endif
	    if(vga_init() >= 0) {
		initted  = 1;
		isEGA    = (vga_getcurrentchipset() == EGA);
		initmode = vga_getcurrentmode();
	    }
	    else initted = 0;
	}
	return((initted > 0) ? TRUE : FALSE);
}

static void reset(void)
{
	if(initted > 0 && vga_getcurrentmode() != initmode)
	    vga_setmode(initmode);
}

static void setrwbanks(int rb,int wb)
{
	vga_setreadpage(rb);
	vga_setwritepage(wb);
}

static void loadcolor(int c,int r,int g,int b)
{
	vga_setpalette(c,(r >> 2),(g >> 2),(b >> 2));
}

static int setmode(GrVideoMode *mp,int noclear)
{
	vga_setmode(mp->mode);
	if (mp->extinfo->flags & GR_VMODEF_LINEAR) {
	  if (vga_setlinearaddressing() == -1)
	    return(FALSE);
	}
	mp->extinfo->frame = (char *)vga_getgraphmem();
	return(TRUE);
}

static int settext(GrVideoMode *mp,int noclear)
{
	vga_setmode(mp->mode);
	return(TRUE);
}

GrVideoModeExt _GrViDrvEGAVGAtextModeExt = {
    GR_frameText,                       /* frame driver */
    NULL,                               /* frame driver override */
    NULL,                               /* frame buffer address */
    { 6, 6, 6 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    settext,                            /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    NULL,                               /* color loader */
};

static GrVideoModeExt exts[NUM_EXTS];
static GrVideoMode   modes[NUM_MODES] = {
    /* pres.  bpp wdt   hgt   mode   scan  priv. &ext                             */
    {  TRUE,  4,  80,   25,   TEXT,  160,  0,    &_GrViDrvEGAVGAtextModeExt },
    {  0  }
};

static int build_video_mode(
    vga_modeinfo   *ip,
    GrVideoMode    *mp,
    GrVideoModeExt *ep
){
	mp->present    = TRUE;
	mp->width      = ip->width;
	mp->height     = ip->height;
	mp->lineoffset = ip->linewidth;
	mp->extinfo    = NULL;
	mp->privdata   = 0;
	ep->drv        = NULL;
	ep->frame      = NULL;          /* filled in after mode set */
	ep->flags      = 0;
	ep->setup      = setmode;
	ep->setvsize   = NULL;          /* tbd */
	ep->scroll     = NULL;          /* tbd */
	ep->setbank    = isEGA ? NULL : vga_setpage;
	ep->setrwbanks = (ip->flags & HAVE_RWPAGE) ? setrwbanks : NULL;
	ep->loadcolor  = NULL;
	switch(ip->colors) {
#ifdef INOUTP_FRAMEDRIVERS
	  case 2:
	    mp->bpp       = 1;
	    ep->mode      = GR_frameEGAVGA1;
	    ep->cprec[0]  =
	    ep->cprec[1]  =
	    ep->cprec[2]  = 1;
	    ep->cpos[0]   =
	    ep->cpos[1]   =
	    ep->cpos[2]   = 0;
	    break;
	  case 16:
	    mp->bpp       = 4;
	    ep->mode      = isEGA ? GR_frameEGA4 : GR_frameSVGA4;
	    ep->cprec[0]  =
	    ep->cprec[1]  =
	    ep->cprec[2]  = isEGA ? 2 : 6;
	    ep->cpos[0]   =
	    ep->cpos[1]   =
	    ep->cpos[2]   = 0;
	    ep->loadcolor = loadcolor;
	    break;
#endif
	  case 256:
	    mp->bpp       = 8;
	    if (ip->flags & IS_MODEX)
#ifdef INOUTP_FRAMEDRIVERS
		ep->mode = GR_frameVGA8X;
#else
		return(FALSE);
#endif
	    else
	    if (ip->flags & CAPABLE_LINEAR) {
		ep->mode  = GR_frameSVGA8_LFB;
		ep->flags|= GR_VMODEF_LINEAR;
	    } else
		ep->mode = GR_frameSVGA8;
	    ep->cprec[0]  =
	    ep->cprec[1]  =
	    ep->cprec[2]  = 6;
	    ep->cpos[0]   =
	    ep->cpos[1]   =
	    ep->cpos[2]   = 0;
	    ep->loadcolor = loadcolor;
	    break;
	  case 32*1024:
	    mp->bpp       = 15;
	    if (ip->flags & CAPABLE_LINEAR) {
		ep->mode  = GR_frameSVGA16_LFB;
		ep->flags|= GR_VMODEF_LINEAR;
	    } else
		ep->mode      = GR_frameSVGA16;
	    ep->cprec[0]  =
	    ep->cprec[1]  =
	    ep->cprec[2]  = 5;
	    ep->cpos[0]   = 10;
	    ep->cpos[1]   = 5;
	    ep->cpos[2]   = 0;
	    break;
	  case 64*1024:
	    mp->bpp       = 16;
	    if (ip->flags & CAPABLE_LINEAR) {
		ep->mode  = GR_frameSVGA16_LFB;
		ep->flags|= GR_VMODEF_LINEAR;
	    } else
		ep->mode      = GR_frameSVGA16;
	    ep->cprec[0]  = 5;
	    ep->cprec[1]  = 6;
	    ep->cprec[2]  = 5;
	    ep->cpos[0]   = 11;
	    ep->cpos[1]   = 5;
	    ep->cpos[2]   = 0;
	    break;
	  case 16*1024*1024:
	    mp->bpp       = 24;
	    if (ip->flags & CAPABLE_LINEAR) {
		ep->mode  = GR_frameSVGA24_LFB;
		ep->flags|= GR_VMODEF_LINEAR;
	    } else
		ep->mode  = GR_frameSVGA24;
	    ep->cprec[0]  =
	    ep->cprec[1]  =
	    ep->cprec[2]  = 8;
	    ep->cpos[0]   = 16;
	    ep->cpos[1]   = 8;
	    ep->cpos[2]   = 0;
	    if(ip->bytesperpixel == 3) break;
	    mp->bpp       = 32;
	    ep->mode      = (ip->flags & CAPABLE_LINEAR) ? GR_frameSVGA32L_LFB
							 : GR_frameSVGA32L;
	    if(!(ip->flags & RGB_MISORDERED)) break;
	    ep->cpos[0]   = 24;
	    ep->cpos[1]   = 16;
	    ep->cpos[2]   = 8;
	    ep->mode      = (ip->flags & CAPABLE_LINEAR) ? GR_frameSVGA32H_LFB
							 : GR_frameSVGA32H;
	    break;
	  default:
	    return(FALSE);
	}
	return(TRUE);
}

static void add_video_mode(
    GrVideoMode *mp,  GrVideoModeExt *ep,
    GrVideoMode **mpp,GrVideoModeExt **epp
){
	if(*mpp < &modes[NUM_MODES]) {
	    if(!mp->extinfo) {
		GrVideoModeExt *etp = &exts[0];
		while(etp < *epp) {
		    if(memcmp(etp,ep,sizeof(GrVideoModeExt)) == 0) {
			mp->extinfo = etp;
			break;
		    }
		    etp++;
		}
		if(!mp->extinfo) {
		    if(etp >= &exts[NUM_EXTS]) return;
		    sttcopy(etp,ep);
		    mp->extinfo = etp;
		    *epp = ++etp;
		}
	    }
	    sttcopy(*mpp,mp);
	    (*mpp)++;
	}
}

static int init(char *options)
{
	if(detect()) {
	    vga_modeinfo  *mdinfo;
	    GrVideoMode    mode,*modep = &modes[1];
	    GrVideoModeExt ext, *extp  = &exts[0];
	    int            mindex;
	    memzero(modep,(sizeof(modes) - sizeof(modes[0])));
	    for(mindex = G320x200x16; mindex <= GLASTMODE; mindex++) {
		if(!(vga_hasmode(mindex)))                 continue;
		if(!(mdinfo = vga_getmodeinfo(mindex)))    continue;
		if(!(build_video_mode(mdinfo,&mode,&ext))) continue;
		mode.mode = mindex;
		add_video_mode(&mode,&ext,&modep,&extp);
	    }
	    _GrVideoDriverSVGALIB.adapter = isEGA ? GR_EGA : GR_VGA;
	    return(TRUE);
	}
	return(FALSE);
}

GrVideoDriver _GrVideoDriverSVGALIB = {
    "svgalib",                          /* name */
    GR_VGA,                             /* adapter type */
    NULL,                               /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    detect,                             /* detection routine */
    init,                               /* initialization routine */
    reset,                              /* reset routine */
    _gr_selectmode,                     /* standard mode select routine */
    0                                   /* no additional capabilities */
};

