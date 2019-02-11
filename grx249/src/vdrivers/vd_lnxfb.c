/**
 ** vd_lnxfb.c ---- Linux framebuffer driver
 **
 ** Copyright (c) 2001 Mariano Alvarez Fernandez
 ** [e-mail: malfer@telefonica.net]
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
 ** Contributions by Josu Onandia (jonandia@fagorautomation.es) 13/12/2002
 **   - Added the 8bpp paletted mode.
 ** Modifications by Mariano Alvarez Fernandez 21/12/2002
 **   - Added function to change virtual terminals, _SwitchConsoleLnxfbDriver
 **     to be called from the input driver.
 **   - Some cleanups, now the text screen shows ok on exit.
 ** Modifications by Mariano Alvarez Fernandez 1/3/2002
 **   - Added code to catch a signal when user wants to change virtual
 **     terminals. The driver sets _lnxfb_waiting_to_switch_console and the
 **     input driver must calls _LnxfbSwitchConsoleAndWait then.
 **   - _SwitchConsoleLnxfbDriver renamed to _LnxfbSwitchConsoleVt, not used,
 **     is here only for possible future use.
 **/

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/vt.h>

#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"
#include "memcopy.h"
#include "memfill.h"

#define  NUM_MODES    80	/* max # of supported modes */
#define  NUM_EXTS     15	/* max # of mode extensions */

static int initted = -1;
static int fbfd = -1;
static int ttyfd = -1;
static struct fb_fix_screeninfo fbfix;
static struct fb_var_screeninfo fbvar;
static char *fbuffer = NULL;
static int ingraphicsmode = 0;

int _lnxfb_waiting_to_switch_console = 0;

static int detect(void)
{
    char *fbname;
    char *default_fbname = "/dev/fb0";

    if (initted < 0) {
	initted = 0;
	fbname = getenv("FRAMEBUFFER");
	if (fbname == NULL)
	    fbname = default_fbname;
	fbfd = open(fbname, O_RDWR);
	if (fbfd == -1)
	    return FALSE;
	ioctl(fbfd, FBIOGET_FSCREENINFO, &fbfix);
	ioctl(fbfd, FBIOGET_VSCREENINFO, &fbvar);
	if (fbfix.type != FB_TYPE_PACKED_PIXELS)
	    return FALSE;
	ttyfd = open("/dev/tty", O_RDONLY);
	initted = 1;
    }
    return ((initted > 0) ? TRUE : FALSE);
}

static void reset(void)
{
    struct vt_mode vtm;

    if (fbuffer) {
	memzero(fbuffer, fbvar.yres * fbfix.line_length);
	munmap(fbuffer, fbfix.smem_len);
	fbuffer = NULL;
    }
    if (fbfd != -1) {
	close(fbfd);
        fbfd = -1;
    }
    if (ttyfd > -1) {
	ioctl(ttyfd, KDSETMODE, KD_TEXT);
        vtm.mode = VT_AUTO;
        vtm.relsig = 0;
        vtm.acqsig = 0;
        ioctl(ttyfd, VT_SETMODE, &vtm);
        signal(SIGUSR1, SIG_IGN);
	close(ttyfd);
        ttyfd = -1;
        ingraphicsmode = 0;
    }
    initted = -1;
}

void _LnxfbSwitchToConsoleVt(unsigned short vt)
{
    struct vt_stat vtst;
    unsigned short myvt;
    GrContext *grc;

    if (!ingraphicsmode) return;
    if (ttyfd < 0) return;
    if (ioctl(ttyfd, VT_GETSTATE, &vtst) < 0) return;
    myvt = vtst.v_active;
    if (vt == myvt) return; 

    grc = GrCreateContext(GrScreenX(), GrScreenY(), NULL, NULL);
    if (grc != NULL) {
	GrBitBlt(grc, 0, 0, GrScreenContext(), 0, 0,
	         GrScreenX()-1, GrScreenY()-1, GrWRITE);
    }
    ioctl(ttyfd, KDSETMODE, KD_TEXT);
    if (ioctl(ttyfd, VT_ACTIVATE, vt) == 0) {
        ioctl(ttyfd, VT_WAITACTIVE, vt);
        ioctl(ttyfd, VT_WAITACTIVE, myvt);
    }
    ioctl(ttyfd, KDSETMODE, KD_GRAPHICS);
    if (grc != NULL) {
	GrBitBlt(GrScreenContext(), 0, 0, grc, 0, 0,
	         GrScreenX()-1, GrScreenY()-1, GrWRITE);
	GrDestroyContext(grc);
    }
}

void _LnxfbSwitchConsoleAndWait(void)
{
    struct vt_stat vtst;
    unsigned short myvt;
    GrContext *grc;

    _lnxfb_waiting_to_switch_console = 0;
    if (!ingraphicsmode) return;
    if (ttyfd < 0) return;
    if (ioctl(ttyfd, VT_GETSTATE, &vtst) < 0) return;
    myvt = vtst.v_active;

    grc = GrCreateContext(GrScreenX(), GrScreenY(), NULL, NULL);
    if (grc != NULL) {
	GrBitBlt(grc, 0, 0, GrScreenContext(), 0, 0,
	         GrScreenX()-1, GrScreenY()-1, GrWRITE);
    }

    ioctl(ttyfd, KDSETMODE, KD_TEXT);

    ioctl(ttyfd, VT_RELDISP, 1);
    ioctl(ttyfd, VT_WAITACTIVE, myvt);

    ioctl(ttyfd, KDSETMODE, KD_GRAPHICS);

    if (grc != NULL) {
	GrBitBlt(GrScreenContext(), 0, 0, grc, 0, 0,
	         GrScreenX()-1, GrScreenY()-1, GrWRITE);
	GrDestroyContext(grc);
    }
}

void _LnxfbRelsigHandle(int sig)
{
    _lnxfb_waiting_to_switch_console = 1;
    signal(SIGUSR1, _LnxfbRelsigHandle);
}

static void loadcolor(int c, int r, int g, int b)
{
    __u16 red, green, blue, transp;
    struct fb_cmap cmap;

    red = (r << 8);
    green = (g << 8);
    blue = (b << 8);
    transp = 0;
    cmap.start = c;
    cmap.len = 1;
    cmap.red = &red;
    cmap.green = &green;
    cmap.blue = &blue;
    cmap.transp = &transp;
    ioctl(fbfd, FBIOPUTCMAP, &cmap);
}

static int setmode(GrVideoMode * mp, int noclear)
{
    struct vt_mode vtm;

    fbuffer = mp->extinfo->frame = mmap(0,
					fbfix.smem_len,
					PROT_READ | PROT_WRITE,
					MAP_SHARED, fbfd, 0);
    if (mp->extinfo->frame && ttyfd > -1) {
	ioctl(ttyfd, KDSETMODE, KD_GRAPHICS);
        vtm.mode = VT_PROCESS;
        vtm.relsig = SIGUSR1;
        vtm.acqsig = 0;
        ioctl(ttyfd, VT_SETMODE, &vtm);
        signal(SIGUSR1, _LnxfbRelsigHandle);
        ingraphicsmode = 1;
    }
    if (mp->extinfo->frame && !noclear)
	memzero(mp->extinfo->frame, fbvar.yres * fbfix.line_length);
    return ((mp->extinfo->frame) ? TRUE : FALSE);
}

static int settext(GrVideoMode * mp, int noclear)
{
    struct vt_mode vtm;

    if (fbuffer) {
	memzero(fbuffer, fbvar.yres * fbfix.line_length);
	munmap(fbuffer, fbfix.smem_len);
	fbuffer = NULL;
    }
    if (ttyfd > -1) {
        ioctl(ttyfd, KDSETMODE, KD_TEXT);
        vtm.mode = VT_AUTO;
        vtm.relsig = 0;
        vtm.acqsig = 0;
        ioctl(ttyfd, VT_SETMODE, &vtm);
        signal(SIGUSR1, SIG_IGN);
        ingraphicsmode = 0;
    }
    return TRUE;
}

GrVideoModeExt grtextextfb = {
    GR_frameText,		/* frame driver */
    NULL,			/* frame driver override */
    NULL,			/* frame buffer address */
    {6, 6, 6},                  /* color precisions */
    {0, 0, 0},                  /* color component bit positions */
    0,				/* mode flag bits */
    settext,			/* mode set */
    NULL,			/* virtual size set */
    NULL,			/* virtual scroll */
    NULL,			/* bank set function */
    NULL,			/* double bank set function */
    NULL,			/* color loader */
};

static GrVideoModeExt exts[NUM_EXTS];
static GrVideoMode modes[NUM_MODES] = {
    /* pres.  bpp wdt   hgt   mode   scan  priv. &ext                             */
    {TRUE, 4, 80, 25, 0, 160, 0, &grtextextfb},
    {0}
};

static int build_video_mode(GrVideoMode * mp, GrVideoModeExt * ep)
{
    mp->present = TRUE;
    mp->width = fbvar.xres;
    mp->height = fbvar.yres;
    mp->lineoffset = fbfix.line_length;
    mp->extinfo = NULL;
    mp->privdata = 0;
    ep->drv = NULL;
    ep->frame = NULL;		/* filled in after mode set */
    ep->flags = 0;
    ep->setup = setmode;
    ep->setvsize = NULL;	/* tbd */
    ep->scroll = NULL;		/* tbd */
    ep->setbank = NULL;
    ep->setrwbanks = NULL;
    ep->loadcolor = NULL;
    switch (fbvar.bits_per_pixel) {
    case 8:
	if (fbfix.visual != FB_VISUAL_PSEUDOCOLOR)
	    return FALSE;
	mp->bpp = 8;
	ep->mode = GR_frameSVGA8_LFB;
	ep->flags |= GR_VMODEF_LINEAR;
	ep->cprec[0] = fbvar.red.length;
	ep->cprec[1] = fbvar.green.length;
	ep->cprec[2] = fbvar.blue.length;
	ep->cpos[0] = fbvar.red.offset;
	ep->cpos[1] = fbvar.green.offset;
	ep->cpos[2] = fbvar.blue.offset;
	ep->loadcolor = loadcolor;
	break;
    case 15:
	if (fbfix.visual != FB_VISUAL_TRUECOLOR)
	    return FALSE;
	mp->bpp = 15;
	ep->mode = GR_frameSVGA16_LFB;
	ep->flags |= GR_VMODEF_LINEAR;
	ep->cprec[0] = fbvar.red.length;
	ep->cprec[1] = fbvar.green.length;
	ep->cprec[2] = fbvar.blue.length;
	ep->cpos[0] = fbvar.red.offset;
	ep->cpos[1] = fbvar.green.offset;
	ep->cpos[2] = fbvar.blue.offset;
	break;
    case 16:
	if (fbfix.visual != FB_VISUAL_TRUECOLOR)
	    return FALSE;
	mp->bpp = 16;
	ep->mode = GR_frameSVGA16_LFB;
	ep->flags |= GR_VMODEF_LINEAR;
	ep->cprec[0] = fbvar.red.length;
	ep->cprec[1] = fbvar.green.length;
	ep->cprec[2] = fbvar.blue.length;
	ep->cpos[0] = fbvar.red.offset;
	ep->cpos[1] = fbvar.green.offset;
	ep->cpos[2] = fbvar.blue.offset;
	break;
    case 24:
	if (fbfix.visual != FB_VISUAL_TRUECOLOR)
	    return FALSE;
	mp->bpp = 24;
	ep->mode = GR_frameSVGA24_LFB;
	ep->flags |= GR_VMODEF_LINEAR;
	ep->cprec[0] = fbvar.red.length;
	ep->cprec[1] = fbvar.green.length;
	ep->cprec[2] = fbvar.blue.length;
	ep->cpos[0] = fbvar.red.offset;
	ep->cpos[1] = fbvar.green.offset;
	ep->cpos[2] = fbvar.blue.offset;
	break;
    default:
	return (FALSE);
    }
    return (TRUE);
}

static void add_video_mode(GrVideoMode * mp, GrVideoModeExt * ep,
			   GrVideoMode ** mpp, GrVideoModeExt ** epp)
{
    if (*mpp < &modes[NUM_MODES]) {
	if (!mp->extinfo) {
	    GrVideoModeExt *etp = &exts[0];
	    while (etp < *epp) {
		if (memcmp(etp, ep, sizeof(GrVideoModeExt)) == 0) {
		    mp->extinfo = etp;
		    break;
		}
		etp++;
	    }
	    if (!mp->extinfo) {
		if (etp >= &exts[NUM_EXTS])
		    return;
		sttcopy(etp, ep);
		mp->extinfo = etp;
		*epp = ++etp;
	    }
	}
	sttcopy(*mpp, mp);
	(*mpp)++;
    }
}

static int init(char *options)
{
    if (detect()) {
	GrVideoMode mode, *modep = &modes[1];
	GrVideoModeExt ext, *extp = &exts[0];
	memzero(modep, (sizeof(modes) - sizeof(modes[0])));
	if ((build_video_mode(&mode, &ext))) {
	    add_video_mode(&mode, &ext, &modep, &extp);
	}
	return (TRUE);
    }
    return (FALSE);
}

GrVideoDriver _GrVideoDriverLINUXFB = {
    "linuxfb",			/* name */
    GR_LNXFB,			/* adapter type */
    NULL,			/* inherit modes from this driver */
    modes,			/* mode table */
    itemsof(modes),		/* # of modes */
    detect,			/* detection routine */
    init,			/* initialization routine */
    reset,			/* reset routine */
    _gr_selectmode,		/* standard mode select routine */
    0				/* no additional capabilities */
};
