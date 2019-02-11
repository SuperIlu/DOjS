/**
 ** sdlinp.c ---- mouse and keyboard interface for SDL
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
 ** FIXME: sdlinp/xwininp: the i18n (cyrillic etc.) keys don't work.
 ** FIXME: win32: Alt-Tab-Alt-Shift-Tab nails the Alt state to unset.
 **
 **/

#include "libsdl.h"
#include "libgrx.h"
#include "grxkeys.h"
#include "input.h"
#include "arith.h"
#include "memcopy.h"
#include "sdlinput.h"

int _nkeyssdlpool = 0;
int _keyssdlpool[_MAXKEYSSDLPOOL];

static int kbd_enabled = TRUE;
static int kbd_hitcount = 0;
static int mou_enabled = TRUE;
static int mou_buttons = 0;
static long evt_lasttime;

int _GrIsKbdEnabled(void)
{
    return kbd_enabled;
}

int _GrKeyPressed(void)
{
    _GrUpdateInputs();
    if (kbd_enabled)
        return (kbd_hitcount > 0);
    else
        return (_nkeyssdlpool > 0);
}

static void uninit(void)
{
    if (MOUINFO->msstatus > 1) {
	MOUINFO->msstatus = 1;
    }
}

int GrMouseDetect(void)
{
#if defined(__WIN32__)
    return(GetSystemMetrics(SM_MOUSEPRESENT));
#else
    return TRUE;
#endif
}

void GrMouseInitN(int queue_size)
{
    uninit();
    queue_size = umax(4, umin(256, queue_size));
    init_queue(queue_size);
    kbd_hitcount = 0;
    if (GrMouseDetect()) {
	GrMouseSetSpeed(1, 1);
	GrMouseSetAccel(100, 1);
	GrMouseSetLimits(0, 0, SCRN->gc_xmax, SCRN->gc_ymax);
        GrMouseWarp((SCRN->gc_xmax >> 1), (SCRN->gc_ymax >> 1));
	_GrInitMouseCursor();
	MOUINFO->msstatus = 2;
	mou_buttons = 0;
    }
    GrMouseEventEnable(TRUE, TRUE);
    real_time(evt_lasttime);
    MOUINFO->uninit = uninit;
}

void GrMouseSetSpeed(int spmult, int spdiv)
{
    MOUINFO->spmult = umin(16, umax(1, spmult));
    MOUINFO->spdiv = umin(16, umax(1, spdiv));
}

void GrMouseSetAccel(int thresh, int accel)
{
    MOUINFO->thresh = umin(64, umax(1, thresh));
    MOUINFO->accel = umin(16, umax(1, accel));
}

void GrMouseSetLimits(int x1, int y1, int x2, int y2)
{
    isort(x1, x2);
    isort(y1, y2);
    MOUINFO->xmin = imax(0, imin(x1, SCRN->gc_xmax));
    MOUINFO->ymin = imax(0, imin(y1, SCRN->gc_ymax));
    MOUINFO->xmax = imax(0, imin(x2, SCRN->gc_xmax));
    MOUINFO->ymax = imax(0, imin(y2, SCRN->gc_ymax));
}

void GrMouseWarp(int x, int y)
{
    MOUINFO->xpos = imax(MOUINFO->xmin, imin(MOUINFO->xmax, x));
    MOUINFO->ypos = imax(MOUINFO->ymin, imin(MOUINFO->ymax, y));
    GrMouseUpdateCursor();
    SDL_WarpMouse(MOUINFO->xpos, MOUINFO->ypos);
}

void GrMouseEventEnable(int enable_kb, int enable_ms)
{
    kbd_enabled = enable_kb;
    mou_enabled = enable_ms;
}

void GrMouseGetEventT(int flags, GrMouseEvent * ev, long tout)
{
    int msdraw;

    if (MOUINFO->msstatus == 0) GrMouseInit();

    msdraw = !MOUINFO->displayed && !(flags & GR_M_NOPAINT);
    if (msdraw) GrMouseDisplayCursor();

    if (tout <= 0L) tout = 1L;

    for (;;) {
	_GrUpdateInputs();
	GrMouseUpdateCursor();
	while (MOUINFO->qlength > 0) {
	    dequeue_event((*ev));
            if (ev->flags & GR_M_KEYPRESS) kbd_hitcount--;
	    if (ev->flags & flags) {
                if (msdraw) GrMouseEraseCursor();
		return;
	    }
	}
	if ((flags & GR_M_POLL) ||
	    (tout == 0L) || (MOUINFO->moved && (flags & GR_M_MOTION))) {
	    fill_mouse_ev((*ev),
			  mou_buttons, mou_buttons,
			  GR_M_LEFT, GR_M_MIDDLE, GR_M_RIGHT, GR_M_P4, GR_M_P5, GrKeyStat());
	    if (ev->flags)	/* something happend */
		real_dtime(ev->dtime, evt_lasttime);
	    else
		ev->dtime = -1;	/* special time if nothing happend */
	    MOUINFO->moved = FALSE;
	    if (msdraw) {
		GrMouseEraseCursor();
	    }
	    return;
	}
	if (tout > 0L) {
	    SDL_Delay(10);
            if ((tout -= 10) < 0L) tout = 0L;
	}
    }
}

#if defined(__XWIN__)
#define DSC 0x08
#else
#define DSC 0x00
#endif

static GrKeyType StdKeyTranslate(SDL_keysym *keysym)
{
	keytrans *k;
	int i;

	/*printf("scancode = %x, unicode = %d , sym = %d, mod = %x\n",
               keysym->scancode, keysym->unicode, keysym->sym, keysym->mod);*/

	if (keysym->mod & KMOD_ALT) {
	    if(keysym->scancode >= 0x10+DSC && keysym->scancode <= 0x35+DSC)
		return(keysym->scancode + 0x100-DSC);
	    if(keysym->scancode >= 0x02+DSC && keysym->scancode <= 0x0d+DSC)
		return(keysym->scancode + 0x176-DSC);
	    k = altstdkeys;
	}
	else if (keysym->mod & KMOD_CTRL) k = controlstdkeys;
	else if (keysym->mod & KMOD_SHIFT) k = shiftstdkeys;
	else k = stdkeys;

	if(keysym->sym >= SDLK_KP0 && keysym->sym <= SDLK_KP_PERIOD) {
	    if(k == stdkeys && (keysym->mod & KMOD_NUM))
		return(numchars[keysym->sym - SDLK_KP0]);
	    keysym->sym = numkeys[keysym->sym - SDLK_KP0];
	}

	for (i = 0; i < NSTDKEYS; i++)
	    if (keysym->sym == k[i].sdlkey) return k[i].grkey;

	return(keysym->unicode & 0xFF);
}

static int DequeueSDLEvent(GrMouseEvent *ev)
{
    SDL_Event event;
    GrKeyType key;
    int buttons;
    static int quit = FALSE;

#if defined(__WIN32__)
    if(!_SGrActive) {
	do SDL_WaitEvent(&event); while(!_SGrActive);
	if(_SGrBackup != NULL) {
	    if(!SDL_LockSurface(_SGrScreen)) {
		memcpy(_SGrScreen->pixels, _SGrBackup, _SGrLength);
		SDL_UnlockSurface(_SGrScreen);
	    }
	    free(_SGrBackup);
	    _SGrBackup = NULL;
	}
    }
#endif

    if(SDL_PollEvent(&event) == 0)
	return(0);

    switch(event.type) {

    case SDL_KEYDOWN :
	if((key = StdKeyTranslate(&event.key.keysym)) != 0) {
	    fill_keybd_ev((*ev), key, event.key.keysym.mod);
	    return(1);
	}
	return(-1);

    case SDL_MOUSEBUTTONDOWN :
	switch(event.button.button) {
	case SDL_BUTTON_LEFT   : buttons = GR_M_LEFT | mou_buttons; break;
	case SDL_BUTTON_RIGHT  : buttons = GR_M_RIGHT | mou_buttons; break;
	case SDL_BUTTON_MIDDLE : buttons = GR_M_MIDDLE | mou_buttons; break;
	case SDL_BUTTON_WHEELUP: buttons = GR_M_P4 | mou_buttons; break;
	case SDL_BUTTON_WHEELDOWN : buttons = GR_M_P5 | mou_buttons; break;
	default : return(-1);
	}
	fill_mouse_ev((*ev), mou_buttons, buttons, GR_M_LEFT,
                      GR_M_MIDDLE, GR_M_RIGHT, GR_M_P4, GR_M_P5, GrKeyStat());
	mou_buttons = buttons;
	MOUINFO->moved = FALSE;
	return(1);

    case SDL_MOUSEBUTTONUP :
	switch(event.button.button) {
	case SDL_BUTTON_LEFT   : buttons = ~GR_M_LEFT & mou_buttons; break;
	case SDL_BUTTON_RIGHT  : buttons = ~GR_M_RIGHT & mou_buttons; break;
	case SDL_BUTTON_MIDDLE : buttons = ~GR_M_MIDDLE & mou_buttons; break;
	case SDL_BUTTON_WHEELUP: buttons = ~GR_M_P4 & mou_buttons; break;
	case SDL_BUTTON_WHEELDOWN : buttons = ~GR_M_P5 & mou_buttons; break;
	default : return(-1);
	}
	fill_mouse_ev((*ev), mou_buttons, buttons, GR_M_LEFT,
                      GR_M_MIDDLE, GR_M_RIGHT, GR_M_P4, GR_M_P5, GrKeyStat());
	mou_buttons = buttons;
	MOUINFO->moved = FALSE;
	return(1);

    case SDL_MOUSEMOTION :
	MOUINFO->xpos = event.motion.x;
	MOUINFO->ypos = event.motion.y;
	MOUINFO->moved = TRUE;
	return(-1);

    case SDL_QUIT :
#if defined(__WIN32__)
	if((_SGrScreen->flags & SDL_HWSURFACE) == SDL_HWSURFACE) {
	    if(!quit) {
		MessageBeep(0xFFFFFFFF);
		quit = TRUE;
		return(-1);
	    }
	}
	else if(MessageBox(NULL, "Abort the program?", "GRX", MB_APPLMODAL |
                           MB_ICONQUESTION | MB_YESNO) != IDYES) return(-1);
#elif defined(__XWIN__)
	if(!quit) {
	    if(isatty(fileno(stderr)))
		fputs("GRX: request quit again to abort the program.\a\n",
		stderr);
	    quit = TRUE;
	    return(-1);
	}
#endif
	exit(1);

    default :
	return(-1);

    }
}

void _GrUpdateInputs(void)
{
    GrMouseEvent ev;
    int r;

    if(SDL_MUSTLOCK(_SGrScreen)) SDL_UnlockSurface(_SGrScreen);
    while ((r = DequeueSDLEvent(&ev)) != 0) {
	if (r > 0) {
            if (ev.flags & GR_M_KEYPRESS && !kbd_enabled){
                if (_nkeyssdlpool < _MAXKEYSSDLPOOL)
                    _keyssdlpool[_nkeyssdlpool++] = ev.key;
            }
            else{
                real_dtime(ev.dtime, evt_lasttime);
                enqueue_event(ev);
                if (ev.flags & GR_M_KEYPRESS) kbd_hitcount++;
            }
	}
    }
    if(SDL_MUSTLOCK(_SGrScreen)) SDL_LockSurface(_SGrScreen);
}

