/**
 ** vd_sdl.c -- SDL driver (interface to SDL)
 **
 ** Copyright (C) 2004 Dimitar Zhekov
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
 ** Passing SDL_SWSURFACE or SDL_HWSURFACE is a good wish only, the
 ** real value is only available in the flags of a surface returned
 ** from SDL_SetVideoMode().
 **
 ** We assume that any SDL_FULLSCREEN | SDL_HWSURFACE screen surface
 ** has pixels pointing to the screen and a dummy SDL_UpdateRect().
 ** This is true for WIN32 and DGA2.
 **
 ** We also assume that such pixels value, once locked, will remain
 ** valid after any number of subsequent unlocks-&-locks (sdlinp.c).
 ** FIXME: update frame after unlock?
 **
 **/

#include "libsdl.h"
#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"
#include "memcopy.h"
#include "memfill.h"

SDL_Surface *_SGrScreen = NULL;
#if defined(__WIN32__)
void *_SGrBackup = NULL;
int _SGrLength;
int _SGrActive = TRUE;
static HWND window;
static WNDPROC wndproc;
static int fullscreen = TRUE;
#else
static int fullscreen = FALSE;
#endif

#if defined(__WIN32__)
static int MaxWidth, MaxHeight;
#elif defined(__XWIN__)
static int MaxWidth, MaxHeight;
#else
#define MaxWidth 9600
#define MaxHeight 7200
#endif

static void reset(void);
static int detect(void);
static void loadcolor(int c, int r, int g, int b);

static int filter(const SDL_Event *event)
{
	if(event->type == SDL_KEYDOWN) {
	    if(event->key.keysym.sym == SDLK_SCROLLOCK)
		SDL_SetModState(SDL_GetModState() ^ KMOD_SCROLL);
	    else if(event->key.keysym.sym == SDLK_INSERT)
		SDL_SetModState(SDL_GetModState() ^ KMOD_INSERT);
	}

	if((event->type == SDL_KEYDOWN || event->type == SDL_KEYUP)
	   && event->key.keysym.sym >= SDLK_NUMLOCK) return(0);

	return(1);
}

#if defined(__WIN32__)
LONG CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	    case WM_KILLFOCUS :
		SDL_EnableKeyRepeat(0, 0);
		_SGrActive = FALSE;
		break;
	    case WM_SYSCOMMAND : if (wParam != SC_RESTORE) break;
	    case WM_SETFOCUS :
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
                                    SDL_DEFAULT_REPEAT_INTERVAL);
		_SGrActive = TRUE;
		break;
	    case WM_ACTIVATEAPP :
		if(_SGrActive && !SDL_LockSurface(_SGrScreen)) {
		    if(_SGrBackup != NULL) free(_SGrBackup);
		    _SGrLength = _SGrScreen->pitch * _SGrScreen->h;
		    _SGrBackup = malloc(_SGrLength);
		    if(_SGrBackup != NULL)
		        memcpy(_SGrBackup, _SGrScreen->pixels, _SGrLength);
		    SDL_UnlockSurface(_SGrScreen);
		}
	}

	return (CallWindowProc(wndproc, hwnd, msg, wParam, lParam));
}
#endif

static int setmode(GrVideoMode *mp,int noclear)
{
	int res;
	GrVideoModeExt *ep = mp->extinfo;
	int fullscreen = mp->privdata & SDL_FULLSCREEN;
#if defined (__WIN32__)
	SDL_SysWMinfo info;
#endif
	SDL_PixelFormat *vfmt;
	GRX_ENTER();
	res = FALSE;
	if(mp->mode != 0) {
	    if(!detect()) {
		DBGPRINTF(DBG_DRIVER, ("SDL re-detect() failed\n"));
		goto done;
	    }

	    _SGrScreen = SDL_SetVideoMode(mp->width, mp->height, mp->bpp,
                                          mp->privdata);
	    if(_SGrScreen == NULL) {
		DBGPRINTF(DBG_DRIVER, ("SDL_SetVideoMode() failed\n"));
		goto done;
	    }
	    if((_SGrScreen->flags & SDL_FULLSCREEN) != fullscreen) {
		DBGPRINTF(DBG_DRIVER, ("SDL_FULLSCREEN mismatch\n"));
		goto done;
	    }

	    SDL_ShowCursor(SDL_DISABLE);
	    SDL_EnableUNICODE(1);
	    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
                                SDL_DEFAULT_REPEAT_INTERVAL);
	    SDL_SetEventFilter(filter);

	    if(SDL_MUSTLOCK(_SGrScreen)) {
		if(!fullscreen) {
		    DBGPRINTF(DBG_DRIVER, ("hardware windows not supported\n"));
		    goto done;
		}
		if(SDL_LockSurface(_SGrScreen)) {
		    DBGPRINTF(DBG_DRIVER, ("SDL_LockSurface() failed\n"));
		    goto done;
		}
#if defined(__WIN32__)
		SDL_VERSION(&info.version);
		if(!SDL_GetWMInfo(&info)) {
		    DBGPRINTF(DBG_DRIVER, ("SDL_GetWMInfo() failed\n"));
		    goto done;
		}
		window = info.window;
		wndproc = (WNDPROC)GetWindowLong(window, GWL_WNDPROC);
		SetWindowLong(window, GWL_WNDPROC, (LONG) WndProc);
		_SGrActive = TRUE;
#endif
	    }

	    mp->lineoffset = _SGrScreen->pitch;
	    ep->frame = _SGrScreen->pixels;

	    if(mp->bpp >= 15 && fullscreen) {
		vfmt = _SGrScreen->format;
		ep->cprec[0] = 8 - vfmt->Rloss;
		ep->cprec[1] = 8 - vfmt->Gloss;
		ep->cprec[2] = 8 - vfmt->Bloss;
		ep->cpos[0]  = vfmt->Rshift;
		ep->cpos[1]  = vfmt->Gshift;
		ep->cpos[2]  = vfmt->Bshift;
		if(mp->bpp == 32 && vfmt->Rshift == 24)
		    ep->mode = GR_frameSVGA32H_LFB;
	    }

	    if(!noclear) {
		if(mp->bpp == 8) loadcolor(0, 0, 0, 0);
		SDL_FillRect(_SGrScreen, NULL, 0);
		SDL_UpdateRect(_SGrScreen, 0, 0, 0, 0);
	    }

	    res = TRUE;
	}
done:	if (res != TRUE) {
	    reset();
	    res = mp->mode == 0;
	}
	GRX_RETURN(res);
}

static void reset(void)
{
	GRX_ENTER();
#if defined(__WIN32__)
	_SGrActive = TRUE;
	if(_SGrBackup != NULL) free(_SGrBackup);
	_SGrBackup = NULL;
#endif
	if(_SGrScreen != NULL)
	    if(SDL_MUSTLOCK(_SGrScreen)) SDL_UnlockSurface(_SGrScreen);
	if(SDL_WasInit(SDL_INIT_VIDEO)) SDL_Quit();
	_SGrScreen = NULL;
	GRX_LEAVE();
}

static int detect(void)
{
	int res;
	GRX_ENTER();
	res = SDL_WasInit(SDL_INIT_VIDEO) || SDL_Init(SDL_INIT_VIDEO) == 0;
	GRX_RETURN(res);
}

static void loadcolor(int c, int r, int g, int b)
{
	SDL_Color color;

	if(_SGrScreen != NULL) {
	    color.r = r;
	    color.g = g;
	    color.b = b;
	    SDL_SetPalette(_SGrScreen, SDL_PHYSPAL, &color, c, 1);
	}
}

static int build_video_mode(int mode, int flags, SDL_Rect *rect,
			    SDL_PixelFormat *vfmt, GrVideoMode *mp,
                            GrVideoModeExt *ep)
{
	mp->present    = TRUE;
	mp->bpp        = vfmt->BitsPerPixel;
	mp->width      = rect->w;
	mp->height     = rect->h;
	mp->mode       = mode;
	mp->lineoffset = 0;
	mp->privdata   = flags | SDL_HWPALETTE;
	mp->extinfo    = NULL;

	ep->drv        = NULL;
	ep->frame      = NULL;
	ep->flags      = GR_VMODEF_LINEAR;
	ep->setup      = setmode;
	ep->setvsize   = NULL;
	ep->scroll     = NULL;
	ep->setbank    = NULL;
	ep->setrwbanks = NULL;
	ep->loadcolor  = NULL;

	switch(mp->bpp) {
	    case 8 :
		ep->cprec[0] =
		ep->cprec[1] =
		ep->cprec[2] = 6;
		ep->cpos[0]  =
		ep->cpos[1]  =
		ep->cpos[2]  = 0;
		ep->mode = GR_frameSDL8;
		ep->loadcolor = loadcolor;
		break;
	    case 15 : 
	    case 16 : ep->mode = GR_frameSDL16; break;
	    case 24 : ep->mode = GR_frameSDL24; break;
	    case 32 : ep->mode = GR_frameSDL32L; break;
	    default : return(FALSE);
	}

	if(mp->width == MaxWidth && mp->height == MaxHeight)
	    mp->privdata |= SDL_NOFRAME;

	if(mp->bpp >= 15) {
	    if(flags & SDL_FULLSCREEN) {
		ep->cprec[0] = 
		ep->cprec[1] = 
		ep->cprec[2] = 0;
		ep->cpos[0]  = 
		ep->cpos[1]  = 
		ep->cpos[2]  = 0;
	    }
	    else {
		ep->cprec[0] = 8 - vfmt->Rloss;
		ep->cprec[1] = 8 - vfmt->Gloss;
		ep->cprec[2] = 8 - vfmt->Bloss;
		ep->cpos[0]  = vfmt->Rshift;
		ep->cpos[1]  = vfmt->Gshift;
		ep->cpos[2]  = vfmt->Bshift;
		if(mp->bpp == 32 && vfmt->Rshift == 24)
		    ep->mode = GR_frameSDL32H;
	    }
	}

	return(TRUE);
}

GrVideoModeExt grtextextsdl = {
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
    {  TRUE,  8,   80,   25,  0x00,    80, 1,    &grtextextsdl  },
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

#define NUM_RESOS 7

struct {
    int w, h;
}
resos[NUM_RESOS] = {
    { 320, 240 },
    { 640, 480 },
    { 800, 600 },
    { 1024, 768 },
    { 1280, 1024 },
    { 1600, 1200 },
    { 9999, 9999 }
};

static int init(char *options)
{
	int res;
	SDL_Rect **rects;
	int *bpp, n;
	int bpps[] = { 8, 15, 16, 24, 32, 0 };
	SDL_PixelFormat fmt;
	const SDL_VideoInfo *vi;
#if defined(__XWIN__)
	Display *dsp;
#endif
	SDL_Rect rect = { 0, 0, 0, 0 };
	int i;
	GrVideoMode mode, *modep = &modes[1];
	GrVideoModeExt ext, *extp = &exts[0];
	GRX_ENTER();
	res = FALSE;
	if(detect()) {
	    if(options) {
		if(!strncmp(options, "fs", 2)) fullscreen = TRUE;
		else if(!strncmp(options, "ww", 2)) fullscreen = FALSE;
	    }
	    memzero(modep,(sizeof(modes) - sizeof(modes[0])));
	    if(fullscreen) {
		memzero(&fmt, sizeof fmt);
		for(bpp = bpps; *bpp; bpp++) {
		    fmt.BitsPerPixel = *bpp;
		    rects = SDL_ListModes(&fmt, SDL_HWSURFACE|SDL_FULLSCREEN);
		    if(rects != NULL && rects != (SDL_Rect **) -1) {
			for(n = 0; rects[n] != NULL; n++);
			for(i = n - 1; i >= 0; i--) {
			    if(!build_video_mode(n-i,
                                                 SDL_HWSURFACE|SDL_FULLSCREEN,
                                                 rects[i], &fmt, &mode, &ext))
				continue;
			    add_video_mode(&mode,&ext,&modep,&extp);
			}
		    }
		}
	    }
	    if(modep == &modes[1]) {
		if((vi = SDL_GetVideoInfo()) == NULL) {
		    DBGPRINTF(DBG_DRIVER, ("SDL_GetVideoInfo() failed\n"));
		    goto done;
		}
#if defined(__WIN32__)
		MaxWidth = GetSystemMetrics(SM_CXSCREEN);
		MaxHeight = GetSystemMetrics(SM_CYSCREEN);
#elif defined(__XWIN__)
		if((dsp = XOpenDisplay("")) != NULL) {
		    MaxWidth = DisplayWidth(dsp, DefaultScreen(dsp));
		    MaxHeight = DisplayHeight(dsp, DefaultScreen(dsp));
		    XCloseDisplay(dsp);
		}
		else {
		    MaxWidth = 9600;
		    MaxHeight = 7200;
		}
#endif
		for(i = 0; i < NUM_RESOS; i++) {
		    rect.w = resos[i].w;
		    rect.h = resos[i].h;
		    if(!build_video_mode(i+1, SDL_SWSURFACE, &rect, vi->vfmt,
                                         &mode, &ext))
			continue;
		    mode.present = rect.w <= MaxWidth && rect.h <= MaxHeight;
		    add_video_mode(&mode,&ext,&modep,&extp);
		}
	    }
	    res = TRUE;
	}
done:	if(!res) reset();
	GRX_RETURN(res);
}

static GrVideoMode *selectmode(GrVideoDriver *drv, int w, int h,
                                    int bpp, int txt, unsigned int *ep)
{
	int i;

	if(!txt && !(modes[1].privdata & SDL_FULLSCREEN)) {
	    for(i = 1; i < NUM_RESOS; i++)
		if(modes[i].width == w && modes[i].height == h) goto done;
	    if(w <= MaxWidth && h <= MaxHeight) {
	        modes[i].present = TRUE;
	        modes[i].width = w;
	        modes[i].height = h;
	    }
	    else modes[i].present = FALSE;
	}
done:	return(_gr_selectmode(drv, w, h, bpp, txt, ep));
}

GrVideoDriver _GrVideoDriverSDL = {
    "sdl",                              /* name */
    GR_SDL,                             /* adapter type */
    NULL,                               /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    detect,                             /* detection routine */
    init,                               /* initialization routine */
    reset,                              /* reset routine */
    selectmode,                         /* special mode select routine */
    0                                   /* no additional capabilities */
};

