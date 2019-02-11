/**
 ** w32inp.c ---- DOS (TCC/BCC/DJGPP: "conio.h") style keyboard utilities
 **
 ** Author:     Gernot Graeff
 ** E-mail:     gernot.graeff@t-online.de
 ** Date:       02-11-99
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
 ** Contributions by M.Alvarez (malfer@teleline.es) 18/11/2001
 **   - Better keys handling using translation tables (w32input.h).
 **
 ** Contributions by M.Alvarez (malfer@teleline.es) 02/02/2002
 **   - The w32 imput queue implemented as a circular queue.
 **   - All the input related code moved here from vd_win32.c
 **
 ** Contribution by M. Lombardi 05/08/2007
 ** Do not treat WM_PAINT messages here. They are delt with in vd_win32.c.
 ** This produced saturation of GRX event queue	and gobbling of
 ** keyboard/mouse events there (compare behavior of test/mousetst)
 **
 ** Contribution by Richard Sanders (richard@dogcreek.ca) 02/04/2009
 ** Synchronisation of windows and grx mouse cursors
 **/

#include "libwin32.h"
#include "libgrx.h"
#include "grxkeys.h"
#include "input.h"
#include "arith.h"
#include "memcopy.h"
#include "w32input.h"

int _nkeysw32pool = 0;
int _keysw32pool[_MAXKEYSW32POOL];

static int kbd_enabled = TRUE;
static int kbd_lastmod = 0;
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
        return (_nkeysw32pool > 0);
}

int _GrKeyStat(void)
{
    return kbd_lastmod;
}

static void uninit(void)
{
    if (MOUINFO->msstatus > 1) {
	MOUINFO->msstatus = 1;
    }
}

int GrMouseDetect(void)
{
    return GetSystemMetrics(SM_MOUSEPRESENT);
}

static void init_w32queue(int queue_size)
{
    EnterCriticalSection(&_csEventQueue);
    if (_W32EventQueueSize != queue_size) {
	if (_W32EventQueue != NULL)
	    free(_W32EventQueue);
        _W32EventQueue = (W32Event *)malloc(sizeof(W32Event) * queue_size);
	_W32EventQueueSize = _W32EventQueue ? queue_size : 0;
    }
    _W32EventQueueRead = 0;
    _W32EventQueueWrite = 0;
    _W32EventQueueLength = 0;
    LeaveCriticalSection(&_csEventQueue);
}

void GrMouseInitN(int queue_size)
{
    uninit();
    queue_size = umax(4, umin(256, queue_size));
    init_queue(queue_size);
    init_w32queue(queue_size);
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
    POINT point;

    MOUINFO->xpos = imax(MOUINFO->xmin, imin(MOUINFO->xmax, x));
    MOUINFO->ypos = imax(MOUINFO->ymin, imin(MOUINFO->ymax, y));
    GrMouseUpdateCursor();
    point.x = MOUINFO->xpos;
    point.y = MOUINFO->ypos;
    ClientToScreen(hGRXWnd, &point);
    SetCursorPos(point.x, point.y);
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
			  GR_M_LEFT, GR_M_MIDDLE, GR_M_RIGHT, GR_M_P4, GR_M_P5, kbd_lastmod);
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
	    Sleep(10);
            if ((tout -= 10) < 0L) tout = 0L;
	}
    }
}

static GrKeyType StdKeyTranslate(int winkey, int fkbState)
{
    keytrans *k;
    int i;

    if (fkbState & GR_KB_ALT)
	k = altstdkeys;
    else if (fkbState & GR_KB_CTRL)
	k = controlstdkeys;
    else if (fkbState & GR_KB_SHIFT)
	k = shiftstdkeys;
    else
	k = stdkeys;

    for (i = 0; i < NSTDKEYS; i++) {
	if (winkey == k[i].winkey)
	    return k[i].grkey;
    }

    return 0;
}

static int DequeueW32Event(GrMouseEvent * ev)
{
    W32Event evaux;
    int key;
    int buttons;

    if (_W32EventQueueLength < 1){
	Sleep(1); /* yield */
	return 0;
    }

    EnterCriticalSection(&_csEventQueue);
//    if (!TryEnterCriticalSection(&_csEventQueue))
//        return 0;

    evaux = _W32EventQueue[_W32EventQueueRead];
    if (++_W32EventQueueRead == _W32EventQueueSize)
	_W32EventQueueRead = 0;
    _W32EventQueueLength--;
    LeaveCriticalSection(&_csEventQueue);

    switch (evaux.uMsg) {

    case WM_CHAR:
        fill_keybd_ev((*ev), evaux.wParam, evaux.kbstat);
        kbd_lastmod = evaux.kbstat;
        return 1;

    case WM_SYSCHAR:
	key = 0;
	if (evaux.wParam >= 'a' && evaux.wParam <= 'z')
	    key = altletters[evaux.wParam - 'a'];
	if (evaux.wParam >= 'A' && evaux.wParam <= 'Z')
	    key = altletters[evaux.wParam - 'A'];
	if (evaux.wParam >= '0' && evaux.wParam <= '9')
	    key = altnumbers[evaux.wParam - '0'];
	if (key == 0)
	    return -1;
	fill_keybd_ev((*ev), key, evaux.kbstat);
	kbd_lastmod = evaux.kbstat;
	return 1;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
	key = StdKeyTranslate(evaux.wParam, evaux.kbstat);
	if (key == 0)
	    return -1;
	fill_keybd_ev((*ev), key, evaux.kbstat);
	kbd_lastmod = evaux.kbstat;
	return 1;

    case WM_COMMAND:
        fill_cmd_ev((*ev), evaux.wParam, evaux.kbstat);
        return 1;

    case WM_LBUTTONDOWN:
	buttons = GR_M_LEFT | mou_buttons;
	MOUINFO->xpos = LOWORD(evaux.lParam);
	MOUINFO->ypos = HIWORD(evaux.lParam);
	fill_mouse_ev((*ev), mou_buttons, buttons, GR_M_LEFT,
		      GR_M_MIDDLE, GR_M_RIGHT, GR_M_P4, GR_M_P5, evaux.kbstat);
	mou_buttons = buttons;
	MOUINFO->moved = FALSE;
	kbd_lastmod = evaux.kbstat;
	return 1;

    case WM_MBUTTONDOWN:
	buttons = GR_M_MIDDLE | mou_buttons;
	MOUINFO->xpos = LOWORD(evaux.lParam);
	MOUINFO->ypos = HIWORD(evaux.lParam);
	fill_mouse_ev((*ev), mou_buttons, buttons, GR_M_LEFT,
		      GR_M_MIDDLE, GR_M_RIGHT, GR_M_P4, GR_M_P5, evaux.kbstat);
	mou_buttons = buttons;
	MOUINFO->moved = FALSE;
	kbd_lastmod = evaux.kbstat;
	return 1;

    case WM_RBUTTONDOWN:
	buttons = GR_M_RIGHT | mou_buttons;
	MOUINFO->xpos = LOWORD(evaux.lParam);
	MOUINFO->ypos = HIWORD(evaux.lParam);
	fill_mouse_ev((*ev), mou_buttons, buttons, GR_M_LEFT,
		      GR_M_MIDDLE, GR_M_RIGHT, GR_M_P4, GR_M_P5, evaux.kbstat);
	mou_buttons = buttons;
	MOUINFO->moved = FALSE;
	kbd_lastmod = evaux.kbstat;
	return 1;

    case WM_LBUTTONUP:
	buttons = ~GR_M_LEFT & mou_buttons;
	MOUINFO->xpos = LOWORD(evaux.lParam);
	MOUINFO->ypos = HIWORD(evaux.lParam);
	fill_mouse_ev((*ev), mou_buttons, buttons, GR_M_LEFT,
		      GR_M_MIDDLE, GR_M_RIGHT, GR_M_P4, GR_M_P5, evaux.kbstat);
	mou_buttons = buttons;
	MOUINFO->moved = FALSE;
	kbd_lastmod = evaux.kbstat;
	return 1;

    case WM_MBUTTONUP:
	buttons = ~GR_M_MIDDLE & mou_buttons;
	MOUINFO->xpos = LOWORD(evaux.lParam);
	MOUINFO->ypos = HIWORD(evaux.lParam);
	fill_mouse_ev((*ev), mou_buttons, buttons, GR_M_LEFT,
		      GR_M_MIDDLE, GR_M_RIGHT, GR_M_P4, GR_M_P5, evaux.kbstat);
	mou_buttons = buttons;
	MOUINFO->moved = FALSE;
	kbd_lastmod = evaux.kbstat;
	return 1;

    case WM_RBUTTONUP:
	buttons = ~GR_M_RIGHT & mou_buttons;
	MOUINFO->xpos = LOWORD(evaux.lParam);
	MOUINFO->ypos = HIWORD(evaux.lParam);
	fill_mouse_ev((*ev), mou_buttons, buttons, GR_M_LEFT,
		      GR_M_MIDDLE, GR_M_RIGHT, GR_M_P4, GR_M_P5, evaux.kbstat);
	mou_buttons = buttons;
	MOUINFO->moved = FALSE;
	kbd_lastmod = evaux.kbstat;
	return 1;

    case WM_MOUSEWHEEL:
	buttons = mou_buttons ^ (((short)HIWORD(evaux.wParam) > 0) ?
                                   GR_M_P4 : GR_M_P5);
	MOUINFO->xpos = LOWORD(evaux.lParam);
	MOUINFO->ypos = HIWORD(evaux.lParam);
	fill_mouse_ev((*ev), mou_buttons, buttons, GR_M_LEFT,
		      GR_M_MIDDLE, GR_M_RIGHT, GR_M_P4, GR_M_P5, evaux.kbstat);
	mou_buttons = buttons;
	MOUINFO->moved = FALSE;
	kbd_lastmod = evaux.kbstat;
	return 1;

    case WM_MOUSEMOVE:
	MOUINFO->xpos = LOWORD(evaux.lParam);
	MOUINFO->ypos = HIWORD(evaux.lParam);
	MOUINFO->moved = TRUE;
        ev->kbstat = evaux.kbstat;
	kbd_lastmod = evaux.kbstat;
	return -1;

    default:
	return -1;

    }
}

void _GrUpdateInputs(void)
{
    GrMouseEvent ev;
    int r;

    while ((r = DequeueW32Event(&ev)) != 0) {
	if (r > 0) {
            if (ev.flags & GR_M_KEYPRESS && !kbd_enabled){
                if (_nkeysw32pool < _MAXKEYSW32POOL)
                    _keysw32pool[_nkeysw32pool++] = ev.key;
            }
            else{
                real_dtime(ev.dtime, evt_lasttime);
                enqueue_event(ev);
                if (ev.flags & GR_M_KEYPRESS) kbd_hitcount++;
            }
	}
    }
}
