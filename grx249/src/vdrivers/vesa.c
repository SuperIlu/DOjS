/**
 ** vesa.c ---- the GRX 2.0 VESA BIOS interface
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
 ** Hartmut Schirmer (hsc@techfak.uni-kiel.de)
 ** Andrzej Lawa [FidoNet: Andrzej Lawa 2:480/19.77]
 **/

#include <string.h>

#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"
#include "int86.h"
#include "ioport.h"
#include "memcopy.h"
#include "memfill.h"
#include "vesa.h"

#if defined(__GNUC__) || (defined(__WATCOMC__) && defined(__386__))
#define  NUM_MODES    100               /* max # of supported modes */
#define  NUM_EXTS     25                /* max # of mode extensions */
#else
#define  NUM_MODES    40                /* max # of supported modes */
#define  NUM_EXTS     12                /* max # of mode extensions */
#endif

static GrVideoMode    modes[NUM_MODES];
static GrVideoModeExt exts[NUM_EXTS];

static unsigned char _GrVidDrvVESAflags = 0;
#define HICOLOR32K   0x01
#define PROTBANKING  0x02
#define USE_REALMODE 0x04
#define NOT_LINEAR   0x08
#define NO_8BIT_DAC  0x10

static char fast256 = 0;
static char varDAC  = FALSE;
static unsigned long VRAMsize = 0;
static int VESAversion;
static int nexts;

/* VESA driver (and others) need this information for paging */
int  _GrVidDrvVESAbanksft = (-1);
int  _GrVidDrvVESArdbank;
int  _GrVidDrvVESAwrbank;

/* these variables hold the correct paging functions:
**
** Turbo-C : normal bios access or direct function call
** GNU-C   : normal real mode bios access or protected mode banking
*/
static void (*_SETRWBANKS)(int rb, int wb);
static void (*_SETBANK)(int bk);


/* get the real mode stuff ... */
#include "vesa_rm.c"

#if   (defined(__WATCOMC__) && defined (__386__)) \
   || (defined(DJGPP) && defined(DJGPP_MINOR))
#define HAVE_VBE2
/* get the VBE2 protected mode stuff ... */
#include "vesa_pm.c"

#  define MC(a,b) a##b
#  define MODE(md)   ((ep->flags & GR_VMODEF_LINEAR) ? MC(md,_LFB) : md)

#else /* no protected mode support */
#  define _SETUP          _GrViDrvSetEGAVGAmode
#  define reset           _GrViDrvResetEGAVGA
#  define MODE(md)        (md)
#endif


static int detect(void)
{
	if(_GrViDrvDetectVGA()) {
	    VESAvgaInfoBlock blk;
	    if(_GrViDrvVESAgetVGAinfo(&blk)) return(TRUE);
	}
	return(FALSE);
}


/* special setup: check for 8bit DAC in palette modes */
static int setup48(GrVideoMode *mp,int noclear) {
  int res = _SETUP(mp, noclear);
  DBGPRINTF(DBG_DRIVER,("setup48 called\n"));
  if (res) {
    mp->extinfo->cprec[0] =
    mp->extinfo->cprec[1] =
    mp->extinfo->cprec[2] = 6;
    _GrViDrvSetDACshift(8-6);
    if (varDAC) {
      /* have variable DAC, try setting 8bit per color component */
      Int86Regs r;
      sttzero(&r);
      IREG_AX(r) = VESA_FUNC + VESA_PAL_CNTRL;
      IREG_BX(r) = 0x0800; /* BL = 0 -> set DAC width, BH=8 -> req. width */
      DBGPRINTF(DBG_DRIVER,("Variable DAC\n"));
#ifdef __WATCOMC__
      int10x(&r);
#else
      int10(&r);
#endif
      if(IREG_AX(r) == VESA_SUCCESS) {
	DBGPRINTF(DBG_DRIVER,("Variable DAC initialised\n"));
	_GrViDrvSetDACshift(8-IREG_BH(r));
	mp->extinfo->cprec[0] =
	mp->extinfo->cprec[1] =
	mp->extinfo->cprec[2] = IREG_BH(r);
      }
    }
  }
  DBGPRINTF(DBG_DRIVER,("setup48 complete\n"));
  return res;
}


static int build_video_mode( VESAmodeInfoBlock *ip,
			     GrVideoMode *mp, GrVideoModeExt *ep)
{
    int banksft = 0;
    int rdbank  = (-1);
    int wrbank  = (-1);
    mp->present    = TRUE;
    mp->width      = ip->XResolution;
    mp->height     = ip->YResolution;
    mp->bpp        = ip->BitsPerPixel;
    mp->lineoffset = ip->BytesPerScanLine;
    mp->extinfo    = NULL;
    mp->privdata   = 0;
    if(!(ip->ModeAttributes & MODE_ISGRAPHICS)) {
	mp->extinfo = &_GrViDrvEGAVGAtextModeExt;
	return(TRUE);
    }
    if(ip->WinSize != 64) return(FALSE);
    while((ip->WinGranularity << banksft) < 64) banksft++;
    if((ip->WinGranularity << banksft) != 64) return(FALSE);
    if(ip->WinAAttributes & WIN_SUPPORTED) {
	if(ip->WinAAttributes & WIN_WRITABLE) wrbank = 0;
	if(ip->WinAAttributes & WIN_READABLE) rdbank = 0;
    }
    if(ip->WinBAttributes & WIN_SUPPORTED) {
	if(ip->WinBAttributes & WIN_WRITABLE) wrbank = 1;
	if(ip->WinBAttributes & WIN_READABLE) rdbank = 1;
    }
    if(wrbank < 0) return(FALSE);
    if(rdbank >= 0) {
	if(rdbank == wrbank) rdbank = (-1);
	if(ip->WinASegment != ip->WinBSegment) rdbank = (-1);
    }
    if(_GrVidDrvVESAbanksft >= 0) {
	if(banksft != _GrVidDrvVESAbanksft) return(FALSE);
	if(wrbank  != _GrVidDrvVESAwrbank)  return(FALSE);
	if(rdbank  != _GrVidDrvVESArdbank)  return(FALSE);
    }
    _GrVidDrvVESAbanksft = banksft;
    _GrVidDrvVESAwrbank  = wrbank;
    _GrVidDrvVESArdbank  = rdbank;

    ep->mode       = GR_frameUndef;
    ep->drv        = NULL;
    ep->frame      = MK_FP((wrbank ? ip->WinBSegment : ip->WinASegment),0);
    ep->flags      = 0;
    ep->cprec[0]   = ep->cprec[1] = ep->cprec[2] = 6;
    ep->cpos[0]    = ep->cpos[1]  = ep->cpos[2]  = 0;

#ifdef __TURBOC__
    if (  _GrVidDrvVESAflags & PROTBANKING) {
	if(VESAbankfn && (VESAbankfn != ip->WinFuncPtr)) {
	    _GrVidDrvVESAflags &= ~PROTBANKING;
	    _SETRWBANKS = RM_setrwbanks;
	    _SETBANK    = RM_setbank;
	} else {
	    VESAbankfn = ip->WinFuncPtr;
	    _SETRWBANKS = RM_protsetrwbanks;
	    _SETBANK    = RM_protsetbank;
	}
    }
#endif
#ifdef HAVE_VBE2
    if(!(_GrVidDrvVESAflags&NOT_LINEAR) && VESAversion>=VESA_VERSION(2,0)) {
	/* check for linear frame buffer */
	if (ip->ModeAttributes&MODE_LIN_FRAME) {
	  DBGPRINTF(DBG_DRIVER,("Linear mode at 0x%08x\n", ip->LinearFrameBuffer));
	  map_linear(ip->LinearFrameBuffer,VRAMsize,
		     &LFB_Selector, &LFB_LinearAddress);
	  DBGPRINTF(DBG_DRIVER,("Linear mode mapped to selector 0x%08x, linear address 0x%08x\n", LFB_Selector, LFB_LinearAddress));
	  ep->LFB_Selector=LFB_Selector;
	  if (LFB_Selector >= 0 && LFB_LinearAddress) {
	    /* every thing went well: allow linear frame buffer access */
	    ep->flags |= GR_VMODEF_LINEAR;
	    ep->frame  = LFB_virtualAddr();
	  }
	}
    }
#endif
    ep->setup      = _SETUP;
    ep->setvsize   = _GrViDrvVESAsetVirtualSize;
    ep->scroll     = _GrViDrvVESAvirtualScroll;
    ep->setbank    = _SETBANK;
    ep->setrwbanks = (rdbank >= 0) ? _SETRWBANKS : NULL;
    ep->loadcolor  = NULL;
    switch(ip->BitsPerPixel) {
      case 4:
	if(ip->MemoryModel != MODEL_4PLANE) return(FALSE);
	if(ip->NumberOfPlanes != 4) return(FALSE);
	ep->mode      = GR_frameSVGA4;
	ep->loadcolor = _GrViDrvLoadColorVGA4;
	ep->flags     = 0; /* no LFB with 4bit modes */
	ep->setup     = setup48;
	break;
      case 8:
	if(ip->MemoryModel != MODEL_PACKED) return(FALSE);
	if(ip->NumberOfPlanes != 1) return(FALSE);
	ep->mode      = MODE(GR_frameSVGA8);
	ep->loadcolor = _GrViDrvLoadColorVGA8;
	ep->flags    |= fast256;
	ep->setup     = setup48;
	break;
      case 15:
      case 16:
      case 24:
      case 32:
	if((ip->MemoryModel != MODEL_PACKED) &&
	   (ip->MemoryModel != MODEL_DIRECT)) return(FALSE);
	if(ip->NumberOfPlanes != 1) return(FALSE);
	mp->bpp = ip->BitsPerPixel;
	switch (ip->BitsPerPixel) {
	  case 32: if(VESAversion < VESA_VERSION(1,2))
		     return(FALSE);
		   if (ip->ReservedMaskSize !=  8)
		      return(FALSE);
		   switch (ip->ReservedMaskPos) {
		     case 24: ep->mode = MODE(GR_frameSVGA32L);
			      break;
		     case  0: ep->mode = MODE(GR_frameSVGA32H);
			      break;
		     default: return(FALSE);
		   }
		   mp->bpp = 24;
		   break;
	  case 24: ep->mode = MODE(GR_frameSVGA24);
		   break;
	  case 16: if (ip->ReservedMaskSize == 1) mp->bpp = 15;
		   goto Default;
	  default:
	  Default: ep->mode = MODE(GR_frameSVGA16);
		   break;
	}
	if(VESAversion < VESA_VERSION(1,2)) {
	    if(ip->BitsPerPixel == 24 || ip->BitsPerPixel == 32) {
		ep->cprec[0] = 8; ep->cpos[0] = 16;
		ep->cprec[1] = 8; ep->cpos[1] = 8;
		ep->cprec[2] = 8; ep->cpos[2] = 0;
		break;
	    }
	    if((ip->BitsPerPixel==15) || (_GrVidDrvVESAflags&HICOLOR32K)) {
		mp->bpp      = 15;
		ep->cprec[0] = 5; ep->cpos[0] = 10;
		ep->cprec[1] = 5; ep->cpos[1] = 5;
		ep->cprec[2] = 5; ep->cpos[2] = 0;
		break;
	    }
	    mp->bpp      = 16;
	    ep->cprec[0] = 5; ep->cpos[0] = 11;
	    ep->cprec[1] = 6; ep->cpos[1] = 5;
	    ep->cprec[2] = 5; ep->cpos[2] = 0;
	    break;
	}
	ep->cprec[0] = ip->RedMaskSize;   ep->cpos[0] = ip->RedMaskPos;
	ep->cprec[1] = ip->GreenMaskSize; ep->cpos[1] = ip->GreenMaskPos;
	ep->cprec[2] = ip->BlueMaskSize;  ep->cpos[2] = ip->BlueMaskPos;
	break;
      default:
	return(FALSE);
    }
    DBGPRINTF(DBG_DRIVER,("build_video_mode complete\n" ));
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

static int get_tweaked_text_mode(GrVideoMode *mp,GrVideoMode *etable)
{
    if(etable < &modes[NUM_MODES]) {
	GrVideoMode *m1,*m2;
	for(m1 = modes; m1 < etable; m1++) {
	    if((m1->present) &&
	       (m1->extinfo) &&
	       (m1->extinfo->mode == GR_frameText) &&
	       (m1->height == 25) &&
	       (m1->width > 80)) {
		VESAmodeInfoBlock mdinfo;
		if((_GrViDrvVESAgetModeInfo(m1->mode,&mdinfo)) &&
		   (mdinfo.ModeAttributes & MODE_EXTINFO) &&
		   (mdinfo.YCharSize == 16)) {
		    int h,exists = FALSE;
		    for(h = 28; h <= 50; h += (50 - 28)) {
			for(m2 = modes; m2 < etable; m2++) {
			    if((m2->present) &&
			       (m2->extinfo) &&
			       (m2->extinfo->mode == GR_frameText) &&
			       (m2->height == h) &&
			       (m2->width == m1->width) &&
			       (m2->bpp == m1->bpp)) {
				exists = TRUE;
				break;
			    }
			}
			if(!exists) {
			    sttcopy(mp,m1);
			    mp->height  = h;
			    mp->extinfo = &_GrViDrvEGAVGAcustomTextModeExt;
			    return(TRUE);
			}
		    }
		}
	    }
	}
    }
    return(FALSE);
}

/* This code will fail if the controller is not VGA compatible.
** One should get the VESA info first and check the Capability
** flags for VGA base controller. If not VGA based all functions
** must use BIOS calls (eg. DAC programming), no tweaked modes,
** don't inherit EGA/VGA modes
** Don't know if (and how) this should be done (hsc 970710)  */
static int init(char *options)
{
    DBGPRINTF(DBG_DRIVER,("options: \"%s\"\n",options));
    if(_GrViDrvInitEGAVGA(options)) {
	VESAvgaInfoBlock blk;
	memzero(modes,sizeof(modes));
	memzero(exts,sizeof(exts));
	nexts = 0;
	if(_GrViDrvVESAgetVGAinfo(&blk)) {
	    VESAmodeInfoBlock mdinfo;
	    GrVideoMode       mode,*modep = &modes[0];
	    GrVideoModeExt    ext, *extp  = &exts[0];
	    short far        *mp;
	    VRAMsize     = blk.MemorySize;
	    VRAMsize   <<= 16;
	    _GrVidDrvVESAflags = 0;
	    _GrVidDrvVESAbanksft  = (-1);
	    VESAversion  = blk.VESAversion;
	    DBGPRINTF(DBG_DRIVER,("VESAversion = %d.%d\n",
		    VESA_VERSION_MAJOR(VESAversion),
		    VESA_VERSION_MINOR(VESAversion) ));
	    DBGPRINTF(DBG_DRIVER,("VRAMsize = %lu bytes\n", VRAMsize));
	    if(options) while(*options != '\0') {
		switch(*options) {
		  case '5':
		    _GrVidDrvVESAflags |= HICOLOR32K;
		    DBGPRINTF(DBG_DRIVER,("option '5': setting HICOLOR32K\n"));
		    break;
		  case 'p':
		  case 'P':
		    DBGPRINTF(DBG_DRIVER,("option 'p': setting PROTBANKING\n"));
		    _GrVidDrvVESAflags |= PROTBANKING;
		    break;
		  case 'f':
		  case 'F':
		    fast256 = GR_VMODEF_FAST_SVGA8;
		    break;
		  case 'r':
		  case 'R':
		    _GrVidDrvVESAflags |= USE_REALMODE;
		    break;
		  case 'b':
		  case 'B':
		    _GrVidDrvVESAflags |= NOT_LINEAR;
		    break;
		}
		options++;
	    }
	    /* set up default banking function */
	    _SETRWBANKS = RM_setrwbanks;
	    _SETBANK    = RM_setbank;
#ifdef HAVE_VBE2
	    /* Check out and set up VBE 2+ portected mode banking */
	    if ( !(_GrVidDrvVESAflags & USE_REALMODE) )
	      VBE2ProtMode();
#endif
	    /* Check for variable width DAC */
	    varDAC =  (_GrVidDrvVESAflags & NO_8BIT_DAC) == 0
		   && (blk.Capabilities & CAP_DAC_WIDTH) != 0;
	    for(mp = blk.VideoModePtr; *mp != (-1); mp++) {
		mode.mode = *mp;
		if(!_GrViDrvVESAgetModeInfo(*mp,&mdinfo))   continue;
		if(!(mdinfo.ModeAttributes & MODE_EXTINFO)) continue;
		if(!build_video_mode(&mdinfo,&mode,&ext))   continue;
		add_video_mode(&mode,&ext,&modep,&extp);
	    }
	    while(get_tweaked_text_mode(&mode,modep)) {
		add_video_mode(&mode,&ext,&modep,&extp);
	    }
	    nexts = (int)(extp-exts);
	}
	return(TRUE);
    }
    return(FALSE);
}

GrVideoDriver _GrVideoDriverVESA = {
    "VESA",                             /* name */
    GR_VGA,                             /* adapter type */
    &_GrVideoDriverSTDVGA,              /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    detect,                             /* detection routine */
    init,                               /* initialization routine */
    reset,                              /* reset routine */
    _gr_selectmode,                     /* standard mode select routine */
    0                                   /* no additional capabilities */
};
