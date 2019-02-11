/**
 ** vd_xfdga.c -- DGA2 driver (interface to X11 DGA2)
 **
 ** Copyright (C) 2003 Dimitar Zhekov
 ** [e-mail: jimmy@is-vn.bg]
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
 ** Changes by Dimitar Zhekov (jimmy@is-vn.bg) Aug 27 2003
 **   - use Xlib for rendering unless defined XF86DGA_FRAMEBUFFER
 **
 **/

#include "libgrx.h"
#include "libxwin.h"
#include "grdriver.h"
#include "arith.h"
#include "memcopy.h"
#include "memfill.h"

int _XGrWindowedMode = 1;

static int setmode(GrVideoMode *mp,int noclear)
{
	int res;
	XDGADevice *dev;
	GRX_ENTER();
	res = FALSE;
	DBGPRINTF(DBG_DRIVER, ("attempting to set DGA mode %d\n", mp->mode));
#ifndef XF86DGA_FRAMEBUFFER
	if(_XGrPatternGC != None) XFreeGC(_XGrDisplay, _XGrPatternGC);
	if(_XGrPattern != None) XFreePixmap(_XGrDisplay, _XGrPattern);
	if(_XGrBitmapGC != None) XFreeGC(_XGrDisplay, _XGrBitmapGC);
	if(_XGrBitmap != None) XFreePixmap(_XGrDisplay, _XGrBitmap);
	if(_XGrGC != None) XFreeGC(_XGrDisplay, _XGrGC);
	_XGrGC = _XGrBitmapGC = _XGrPatternGC = None;
	_XGrBitmap = _XGrPattern = None;
#endif
	XUngrabPointer(_XGrDisplay, CurrentTime);
	XUngrabKeyboard(_XGrDisplay, CurrentTime);
	dev = XDGASetMode(_XGrDisplay, _XGrScreen, mp->mode);
	if(mp->mode != 0) {
	    if(dev == NULL) {
		DBGPRINTF(DBG_DRIVER, ("can't set DGA mode\n"));
		goto done;
	    }
#ifdef XF86DGA_FRAMEBUFFER
	    mp->extinfo->frame = dev->data;
#else
	    mp->extinfo->frame = (char *) dev->pixmap;
#endif
	    XDGASync(_XGrDisplay, _XGrScreen);
	    XDGASetViewport(_XGrDisplay, _XGrScreen, 0, 0, XDGAFlipRetrace);
	    XGrabKeyboard(_XGrDisplay, _XGrWindow, True, GrabModeAsync,
			  GrabModeAsync, CurrentTime);
	    XGrabPointer(_XGrDisplay, _XGrWindow, True, StructureNotifyMask |
			 ButtonPressMask | ButtonReleaseMask |
			 ButtonMotionMask | PointerMotionMask, GrabModeAsync,
			 GrabModeAsync, None, None, CurrentTime);
	    if(mp->bpp == 8) {
		if(_XGrColormap == None) {
		    /* one colormap for all modes, XFreeColormap() problems */
		    _XGrColormap = XDGACreateColormap(_XGrDisplay, _XGrScreen,
						      dev, AllocAll);
		}
		if(_XGrColormap != None) {
		    XDGAInstallColormap(_XGrDisplay, _XGrScreen, _XGrColormap);
		    _GR_lastFreeColor = 255;
		}
	    }
#ifdef XF86DGA_FRAMEBUFFER
	    if(!noclear) memzero(dev->data, mp->lineoffset * mp->height);
#else
	    _XGrDepth = _XGrBitsPerPixel = mp->bpp;
	    if(_XGrDepth == 32) _XGrDepth = 24;
	    _XGrForeColor = GrNOCOLOR;		/* force XSetForeground */
	    _XGrBackColor = GrNOCOLOR;		/* force XSetBackground */
	    _XGrColorOper = C_WRITE;
	    _XGrGC = XCreateGC(_XGrDisplay, dev->pixmap, 0L, NULL);
	    _XGrBitmap = XCreatePixmap(_XGrDisplay, dev->pixmap, 128, 128, 1);
	    _XGrBitmapGC = XCreateGC(_XGrDisplay, _XGrBitmap, 0L, NULL);
	    _XGrPattern = XCreatePixmap(_XGrDisplay, dev->pixmap, 8, 1, 1);
	    _XGrPatternGC = XCreateGC(_XGrDisplay, _XGrPattern, 0L, NULL);
	    if(!noclear) {
		XSetForeground(_XGrDisplay, _XGrGC, 0);
		XSetFunction(_XGrDisplay, _XGrGC, GXcopy);
		XFillRectangle(_XGrDisplay, dev->pixmap, _XGrGC, 0, 0,
			       mp->width, mp->height);
	    }
#endif
	}
	res = TRUE;
done:	if(dev != NULL) XFree(dev);
	GRX_RETURN(res);
}

static void reset(void)
{
	XDGADevice *dev;
	GRX_ENTER();
	if(_XGrDisplay != NULL) {
	    XUngrabPointer(_XGrDisplay, CurrentTime);
	    XUngrabKeyboard(_XGrDisplay, CurrentTime);
#ifdef XF86DGA_FRAMEBUFFER
	    XDGACloseFramebuffer(_XGrDisplay, _XGrScreen);
#endif
	    dev = XDGASetMode(_XGrDisplay, _XGrScreen, 0);
	    if(dev != NULL) XFree(dev);
	    XCloseDisplay(_XGrDisplay);
	}
	_XGrDisplay = NULL;
	_XGrColormap = None;
	_GR_firstFreeColor =  0;
	_GR_lastFreeColor  = -1;
	GRX_LEAVE();
}

static int detect(void)
{
	int res;
	int event, error;
	int major, minor;
	GRX_ENTER();
	if(_XGrDisplay == NULL) {
	    res = FALSE;
#ifdef XF86DGA_FRAMEBUFFER
	    if(geteuid()) {
		DBGPRINTF(DBG_DRIVER, ("root priviledges required\n"));
		goto done;
	    }
#endif
	    if((_XGrDisplay = XOpenDisplay("")) == NULL) {
		DBGPRINTF(DBG_DRIVER, ("can't connect to X server\n"));
		goto done;
	    }
	    if(!XDGAQueryExtension(_XGrDisplay, &event, &error)) {
		DBGPRINTF(DBG_DRIVER, ("can't query DGA extension\n"));
		goto done;
	    }
	    if(!XDGAQueryVersion(_XGrDisplay, &major, &minor)) {
		DBGPRINTF(DBG_DRIVER, ("can't query DGA version\n"));
		goto done;
	    }
	    if(major < 2) {
		DBGPRINTF(DBG_DRIVER,
                          ("required DGA version 2.0, detected %d.%d\n",
                           major, minor));
		goto done;
	    }
	}
	res = TRUE;
done:	if(!res) reset();
	GRX_RETURN(res);
}

#ifndef XF86DGA_FRAMEBUFFER
static void setbank(int bk)
{}

static void setrwbanks(int rb,int wb)
{}
#endif

static void loadcolor(int c,int r,int g,int b)
{
	XColor xcolor;

	if(_XGrColormap != None) {
	    xcolor.pixel = c;
	    xcolor.red   = r * 257;
	    xcolor.green = g * 257;
	    xcolor.blue  = b * 257;
	    xcolor.flags = DoRed | DoGreen | DoBlue;
	    XStoreColor(_XGrDisplay, _XGrColormap, &xcolor);
	}
}

#ifdef XF86DGA_FRAMEBUFFER
#define GRFRAMEDRIVER(bpp) GR_frameSVGA##bpp##_LFB
#else
#define GRFRAMEDRIVER(bpp) GR_frameXWIN##bpp
#endif

static int build_video_mode(XDGAMode *ip, GrVideoMode *mp, GrVideoModeExt *ep)
{
	mp->present    = TRUE;
	mp->bpp        = ip->depth;
	mp->width      = ip->viewportWidth;
	mp->height     = ip->viewportHeight;
	mp->mode       = ip->num;
	mp->lineoffset = ip->bytesPerScanline;
	mp->privdata   = 0;
	mp->extinfo    = NULL;

	ep->drv        = NULL;
	ep->frame      = NULL;
#ifdef XF86DGA_FRAMEBUFFER
	ep->flags      = GR_VMODEF_LINEAR;
#else
	ep->flags      = 0;
#endif
	ep->setup      = setmode;
	ep->setvsize   = NULL;
	ep->scroll     = NULL;
#ifdef XF86DGA_FRAMEBUFFER
	ep->setbank    = NULL;
	ep->setrwbanks = NULL;
#else
	ep->setbank    = setbank;
	ep->setrwbanks = setrwbanks;
#endif
	ep->loadcolor  = NULL;

	if(ip->visualClass != (mp->bpp == 8 ? PseudoColor : TrueColor)) {
	    DBGPRINTF(DBG_DRIVER, ("%d: visual class %s depth %d unsupported\n",
				   ip->num, _XGrClassNames[ip->visualClass],
				   mp->bpp));
	    return(FALSE);
	}

#ifndef XF86DGA_FRAMEBUFFER
	if(!(ip->flags & XDGAPixmap)) {
	    DBGPRINTF(DBG_DRIVER, ("%d: rendering with XLib impossible\n",
				   ip->num));
	    return(FALSE);
	}
#endif

	switch(mp->bpp) {
	    case 8 :
		ep->mode = GRFRAMEDRIVER(8);
		ep->cprec[0] =
		ep->cprec[1] =
		ep->cprec[2] = 6;
		ep->cpos[0]  =
		ep->cpos[1]  =
		ep->cpos[2]  = 0;
		ep->loadcolor = loadcolor;
		break;
	    case 15 :
		ep->mode = GRFRAMEDRIVER(16);
		ep->cprec[0] =
		ep->cprec[1] =
		ep->cprec[2] = 5;
		ep->cpos[0]  = 10;
		ep->cpos[1]  = 5;
		ep->cpos[2]  = 0;
		break;
	    case 16 :
		ep->mode = GRFRAMEDRIVER(16);
		ep->cprec[0] = 5;
		ep->cprec[1] = 6;
		ep->cprec[2] = 5;
		ep->cpos[0]  = 11;
		ep->cpos[1]  = 5;
		ep->cpos[2]  = 0;
		break;
	    case 24 :
		ep->cprec[0]  =
		ep->cprec[1]  =
		ep->cprec[2]  = 8;
		ep->cpos[0]   = 16;
		ep->cpos[1]   = 8;
		ep->cpos[2]   = 0;
		ep->mode = GRFRAMEDRIVER(24);
		if(ip->bitsPerPixel == 32) {
		    mp->bpp = 32;
		    ep->mode = GRFRAMEDRIVER(32L);
		    if(ip->redMask == 0xFF000000) {
			ep->cpos[0]   = 24;
			ep->cpos[1]   = 16;
			ep->cpos[2]   = 8;
			ep->mode = GRFRAMEDRIVER(32H);
		    }
		}
		break;
	    default:
		return(FALSE);
	}

	return(TRUE);
}

GrVideoModeExt grtextextdga = {
    GR_frameText,                       /* frame driver */
    NULL,                               /* frame driver override */
    NULL,                               /* frame buffer address */
    { 0, 0, 0 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    setmode,                            /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    NULL                                /* color loader */
};

#define  NUM_MODES    200               /* max # of supported modes */
#define  NUM_EXTS     10                /* max # of mode extensions */

static GrVideoModeExt exts[NUM_EXTS];
static GrVideoMode   modes[NUM_MODES] = {
    /* pres.  bpp wdt   hgt   BIOS   scan  priv. &ext  */
    {  TRUE,  8,   80,   25,  0x00,    80, 1,    &grtextextdga  },
    {  0  }
};

/* from svgalib.c, unmodified */
static void add_video_mode(
    GrVideoMode *mp,  GrVideoModeExt *ep,
    GrVideoMode **mpp,GrVideoModeExt **epp
) {
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
	int res;
	XDGAMode *modev = NULL;
	int modec, mindex;
	GrVideoMode mode, *modep = &modes[1];
	GrVideoModeExt ext, *extp = &exts[0];
	GRX_ENTER();
	res = FALSE;
	if(detect()) {
	    _XGrScreen = DefaultScreen(_XGrDisplay);
	    _XGrWindow = DefaultRootWindow(_XGrDisplay);
#ifdef XF86DGA_FRAMEBUFFER
	    if(!XDGAOpenFramebuffer(_XGrDisplay, _XGrScreen)) {
		DBGPRINTF(DBG_DRIVER, ("can't open framebuffer\n"));
		goto done;
	    }
#endif
	    modev = XDGAQueryModes(_XGrDisplay, _XGrScreen, &modec);
	    if(modev == NULL) {
		DBGPRINTF(DBG_DRIVER, ("can't query DGA modes"));
		goto done;
	    }
	    memzero(modep,(sizeof(modes) - sizeof(modes[0])));
	    for(mindex = 0; mindex < modec; mindex++) {
		if(!build_video_mode(&modev[mindex], &mode, &ext)) continue;
		add_video_mode(&mode,&ext,&modep,&extp);
	    }
	    _XGrWindowedMode = 0;
	    _XGrColormap = None;
#ifndef XF86DGA_FRAMEBUFFER
	    _XGrColorNumPixels = 0;
#endif
	    res = TRUE;
	}
done:	if(modev != NULL) XFree(modev);
	if(!res) reset();
	GRX_RETURN(res);
}

GrVideoDriver _GrVideoDriverXF86DGA = {
    "xf86dga",                          /* name */
    GR_XWIN,                            /* adapter type */
    NULL,                               /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    detect,                             /* detection routine */
    init,                               /* initialization routine */
    reset,                              /* reset routine */
    _gr_selectmode,                     /* standard mode select routine */
    0                                   /* no additional capabilities */
};

