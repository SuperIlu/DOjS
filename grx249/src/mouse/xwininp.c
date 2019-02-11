/**
 ** xwininp.c ---- mouse and keyboard interface for X Windows
 **
 ** Author:     Ulrich Leodolter
 ** E-mail:     ulrich@lab1.psy.univie.ac.at
 ** Date:       Thu Sep 28 20:22:16 1995
 ** Comment:    Implements the same GRX functions as dosinput.c
 ** RCSId:      $Id: xwininput.c 1.2 1995/11/19 19:32:30 ulrich Exp $
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
 ** Small changes by Dimitar Zhekov to work in fullscreen mode (DGA2).
 **
 ** Contributions by:
 ** 070505 M.Alvarez, Using a Pixmap for BackingStore.
 **
 **/

#include <stdlib.h>
#include <sys/time.h>   /* for select() */

#include "libgrx.h"
#include "libxwin.h"
#include <X11/keysym.h>
#include "grxkeys.h"
#include "allocate.h"
#include "arith.h"
#include "memcopy.h"
#include "memfill.h"
#include "mouse/input.h"

#ifdef _AIX
#include <sys/select.h>
#endif

static int  kbd_enabled = TRUE;
static int  kbd_lastmod = 0;
static int  mou_enabled = TRUE;
static int  mou_buttons = 0;
static Time MouseMoveTime = 0;
static Time evt_lasttime;
static int  evt_lasttime_ok = FALSE;

#if 0
long volatile   _XGrTickValue = -1;
static void     _XGrTickHandler (int signum)
{
  signal (signum, _XGrTickHandler);
  _XGrTickValue++;
}
#endif

static void uninit(void)
{
	if(MOUINFO->msstatus > 1) MOUINFO->msstatus = 1;
}

int GrMouseDetect(void)
{
	if(MOUINFO->msstatus == 0) {
	  if (_XGrDisplay) MOUINFO->msstatus = 1; /* present, but not initted */
	}
	return((MOUINFO->msstatus > 0) ? TRUE : FALSE);
}

void GrMouseInitN(int queue_size)
{
	uninit();
	queue_size = umax(4,umin(256,queue_size));
	init_queue(queue_size);
	if(GrMouseDetect()) {
	    GrMouseSetSpeed(1,1);
	    GrMouseSetAccel(100,1);
	    GrMouseSetLimits(0,0,SCRN->gc_xmax,SCRN->gc_ymax);
	    GrMouseWarp((SCRN->gc_xmax >> 1),(SCRN->gc_ymax >> 1));
	    _GrInitMouseCursor();
	    MOUINFO->msstatus = 2;
	    mou_buttons = 0;
	    /*
	     * Define an invisible X cursor for _XGrWindow
	     */
	    if(_XGrWindowedMode) {
	      static char cbits[8] = { 0,0,0,0,0,0,0,0, };
	      Pixmap csource, cmask;
	      XColor cfore, cback;
	      Cursor curs;

	      csource = cmask = XCreateBitmapFromData (_XGrDisplay,
						       _XGrWindow,
						       cbits,
						       8,
						       8
						       );
	      cfore.red = cfore.green = cfore.blue = 0;
	      cback.red = cback.green = cback.blue = 0;
	      curs = XCreatePixmapCursor (_XGrDisplay,
					  csource,
					  cmask,
					  &cfore,
					  &cback,
					  0,
					  0
					  );
	      XDefineCursor (_XGrDisplay, _XGrWindow, curs);
	    }
	}
	GrMouseEventEnable(TRUE,TRUE);
	evt_lasttime_ok = FALSE;
	MouseMoveTime = 0;
	MOUINFO->uninit = uninit;
}

void GrMouseSetSpeed(int spmult,int spdiv)
{
	MOUINFO->spmult = umin(16,umax(1,spmult));
	MOUINFO->spdiv  = umin(16,umax(1,spdiv));
}

void GrMouseSetAccel(int thresh,int accel)
{
	MOUINFO->thresh = umin(64,umax(1,thresh));
	MOUINFO->accel  = umin(16,umax(1,accel));
}

void GrMouseSetLimits(int x1,int y1,int x2,int y2)
{
	isort(x1,x2);
	isort(y1,y2);
	MOUINFO->xmin = imax(0,imin(x1,SCRN->gc_xmax));
	MOUINFO->ymin = imax(0,imin(y1,SCRN->gc_ymax));
	MOUINFO->xmax = imax(0,imin(x2,SCRN->gc_xmax));
	MOUINFO->ymax = imax(0,imin(y2,SCRN->gc_ymax));
}

void GrMouseWarp(int x,int y)
{
    MOUINFO->xpos = imax(MOUINFO->xmin,imin(MOUINFO->xmax,x));
    MOUINFO->ypos = imax(MOUINFO->ymin,imin(MOUINFO->ymax,y));
    if (_XGrDisplay) {
	GrMouseUpdateCursor();
	/*
	 * Move the X cursor only if inside _XGrWindow
	 */
	XWarpPointer (_XGrDisplay,
		      _XGrWindow,
		      _XGrWindow,
		      0,
		      0,
		      GrScreenX(),
		      GrScreenY(),
		      MOUINFO->xpos,
		      MOUINFO->ypos);
    }
}

void GrMouseEventEnable(int enable_kb,int enable_ms)
{
	kbd_enabled = enable_kb;
	mou_enabled = enable_ms;
}

/* Keyboard Translation Table */

typedef struct {
  GrKeyType key;
  unsigned short state;
  KeySym keysym;
} KeyEntry;

static KeyEntry _KeyTable[] = {
  { GrKey_Alt_0                , Mod1Mask, XK_0 },
  { GrKey_Alt_1                , Mod1Mask, XK_1 },
  { GrKey_Alt_2                , Mod1Mask, XK_2 },
  { GrKey_Alt_3                , Mod1Mask, XK_3 },
  { GrKey_Alt_4                , Mod1Mask, XK_4 },
  { GrKey_Alt_5                , Mod1Mask, XK_5 },
  { GrKey_Alt_6                , Mod1Mask, XK_6 },
  { GrKey_Alt_7                , Mod1Mask, XK_7 },
  { GrKey_Alt_8                , Mod1Mask, XK_8 },
  { GrKey_Alt_9                , Mod1Mask, XK_9 },
  { GrKey_Alt_A                , Mod1Mask, XK_a },
  { GrKey_Alt_At               , Mod1Mask, XK_at },
  { GrKey_Alt_B                , Mod1Mask, XK_b },
  { GrKey_Alt_Backquote        , Mod1Mask, XK_quoteright },
  { GrKey_Alt_Backslash        , Mod1Mask, XK_backslash },
  { GrKey_Alt_Backspace        , Mod1Mask, XK_BackSpace },
  { GrKey_Alt_C                , Mod1Mask, XK_c },
#ifdef XK_KP_Begin
  { GrKey_Alt_Center           , Mod1Mask, XK_KP_Begin },
#endif
  { GrKey_Alt_Comma            , Mod1Mask, XK_comma },
  { GrKey_Alt_D                , Mod1Mask, XK_d },
  { GrKey_Alt_Dash             , Mod1Mask, XK_minus },
#ifdef XK_KP_Delete
  { GrKey_Alt_Delete           , Mod1Mask, XK_KP_Delete },
#endif
  { GrKey_Alt_Down             , Mod1Mask, XK_Down },
#ifdef XK_KP_Down
  { GrKey_Alt_Down             , Mod1Mask, XK_KP_Down },
#endif
  { GrKey_Alt_E                , Mod1Mask, XK_e },
#ifdef XK_KP_End
  { GrKey_Alt_End              , Mod1Mask, XK_KP_End },
#endif
#ifdef XK_KP_Enter
  { GrKey_Alt_Enter            , Mod1Mask, XK_KP_Enter },
#endif
  { GrKey_Alt_Equals           , Mod1Mask, XK_equal },
  { GrKey_Alt_Escape           , Mod1Mask, XK_Escape },
  { GrKey_Alt_F                , Mod1Mask, XK_f },
  { GrKey_Alt_F1               , Mod1Mask, XK_F1 },
  { GrKey_Alt_F2               , Mod1Mask, XK_F2 },
  { GrKey_Alt_F3               , Mod1Mask, XK_F3 },
  { GrKey_Alt_F4               , Mod1Mask, XK_F4 },
  { GrKey_Alt_F5               , Mod1Mask, XK_F5 },
  { GrKey_Alt_F6               , Mod1Mask, XK_F6 },
  { GrKey_Alt_F7               , Mod1Mask, XK_F7 },
  { GrKey_Alt_F8               , Mod1Mask, XK_F8 },
  { GrKey_Alt_F9               , Mod1Mask, XK_F9 },
  { GrKey_Alt_F10              , Mod1Mask, XK_F10 },
  { GrKey_Alt_F11              , Mod1Mask, XK_F11 },
  { GrKey_Alt_F12              , Mod1Mask, XK_F12 },
  { GrKey_Alt_G                , Mod1Mask, XK_g },
  { GrKey_Alt_H                , Mod1Mask, XK_h },
#ifdef XK_KP_Home
  { GrKey_Alt_Home             , Mod1Mask, XK_KP_Home },
#endif
  { GrKey_Alt_I                , Mod1Mask, XK_i },
#ifdef XK_KP_Insert
  { GrKey_Alt_Insert           , Mod1Mask, XK_KP_Insert },
#endif
  { GrKey_Alt_J                , Mod1Mask, XK_j },
  { GrKey_Alt_K                , Mod1Mask, XK_k },
#ifdef XK_KP_Subtract
  { GrKey_Alt_KPMinus          , Mod1Mask, XK_KP_Subtract },
#endif
#ifdef XK_KP_Add
  { GrKey_Alt_KPPlus           , Mod1Mask, XK_KP_Add },
#endif
#ifdef XK_KP_Divide
  { GrKey_Alt_KPSlash          , Mod1Mask, XK_KP_Divide },
#endif
#ifdef XK_KP_Multiply
  { GrKey_Alt_KPStar           , Mod1Mask, XK_KP_Multiply },
#endif
  { GrKey_Alt_KPStar           , Mod1Mask, XK_multiply },
  { GrKey_Alt_L                , Mod1Mask, XK_l },
  { GrKey_Alt_LAngle           , Mod1Mask, XK_less },
  { GrKey_Alt_LBrace           , Mod1Mask, XK_braceleft },
  { GrKey_Alt_LBracket         , Mod1Mask, XK_bracketleft },
#ifdef XK_KP_Left
  { GrKey_Alt_Left             , Mod1Mask, XK_KP_Left },
#endif
  { GrKey_Alt_Left             , Mod1Mask, XK_Left },
  { GrKey_Alt_M                , Mod1Mask, XK_m },
  { GrKey_Alt_N                , Mod1Mask, XK_n },
  { GrKey_Alt_O                , Mod1Mask, XK_o },
  { GrKey_Alt_P                , Mod1Mask, XK_p },
#ifdef XK_KP_Next
  { GrKey_Alt_PageDown         , Mod1Mask, XK_KP_Next },
#endif
#ifdef XK_KP_Page_Down
  { GrKey_Alt_PageDown         , Mod1Mask, XK_KP_Page_Down },
#endif
#ifdef XK_KP_Page_Up
  { GrKey_Alt_PageUp           , Mod1Mask, XK_KP_Page_Up },
#endif
#ifdef XK_KP_Prior
  { GrKey_Alt_PageUp           , Mod1Mask, XK_KP_Prior },
#endif
  { GrKey_Alt_Period           , Mod1Mask, XK_period },
  { GrKey_Alt_Pipe             , Mod1Mask, XK_bar },
  { GrKey_Alt_Q                , Mod1Mask, XK_q },
  { GrKey_Alt_Quote            , Mod1Mask, XK_quoteleft },
  { GrKey_Alt_R                , Mod1Mask, XK_r },
  { GrKey_Alt_RAngle           , Mod1Mask, XK_greater },
  { GrKey_Alt_RBrace           , Mod1Mask, XK_braceright },
  { GrKey_Alt_RBracket         , Mod1Mask, XK_bracketright },
  { GrKey_Alt_Return           , Mod1Mask, XK_Return },
#ifdef XK_KP_Right
  { GrKey_Alt_Right            , Mod1Mask, XK_KP_Right },
#endif
  { GrKey_Alt_Right            , Mod1Mask, XK_Right },
  { GrKey_Alt_S                , Mod1Mask, XK_s },
  { GrKey_Alt_Semicolon        , Mod1Mask, XK_semicolon },
  { GrKey_Alt_Slash            , Mod1Mask, XK_slash },
  { GrKey_Alt_T                , Mod1Mask, XK_t },
  { GrKey_Alt_Tab              , Mod1Mask, XK_Tab },
  { GrKey_Alt_U                , Mod1Mask, XK_u },
#ifdef XK_KP_Up
  { GrKey_Alt_Up               , Mod1Mask, XK_KP_Up },
#endif
  { GrKey_Alt_Up               , Mod1Mask, XK_Up },
  { GrKey_Alt_V                , Mod1Mask, XK_v },
  { GrKey_Alt_W                , Mod1Mask, XK_w },
  { GrKey_Alt_X                , Mod1Mask, XK_x },
  { GrKey_Alt_Y                , Mod1Mask, XK_y },
  { GrKey_Alt_Z                , Mod1Mask, XK_z },
#ifdef XK_ISO_Left_Tab
  { GrKey_BackTab              , ShiftMask, XK_ISO_Left_Tab },
#endif
  { GrKey_Center               , 0, XK_5 },
#ifdef XK_KP_Begin
  { GrKey_Center               , 0, XK_KP_Begin },
#endif
  { GrKey_Control_At           , ControlMask, XK_at },
  { GrKey_Control_Center       , ControlMask, XK_5 },
#ifdef XK_KP_Begin
  { GrKey_Control_Center       , ControlMask, XK_KP_Begin },
#endif
  { GrKey_Control_Delete       , ControlMask, XK_Delete },
#ifdef XK_KP_Delete
  { GrKey_Control_Delete       , ControlMask, XK_KP_Delete },
#endif
  { GrKey_Control_Down         , ControlMask, XK_Down },
#ifdef XK_KP_Down
  { GrKey_Control_Down         , ControlMask, XK_KP_Down },
#endif
  { GrKey_Control_End          , ControlMask, XK_End },
#ifdef XK_KP_End
  { GrKey_Control_End          , ControlMask, XK_KP_End },
#endif
  { GrKey_Control_F1           , ControlMask, XK_F1 },
  { GrKey_Control_F2           , ControlMask, XK_F2 },
  { GrKey_Control_F3           , ControlMask, XK_F3 },
  { GrKey_Control_F4           , ControlMask, XK_F4 },
  { GrKey_Control_F5           , ControlMask, XK_F5 },
  { GrKey_Control_F6           , ControlMask, XK_F6 },
  { GrKey_Control_F7           , ControlMask, XK_F7 },
  { GrKey_Control_F8           , ControlMask, XK_F8 },
  { GrKey_Control_F9           , ControlMask, XK_F9 },
  { GrKey_Control_F10          , ControlMask, XK_F10 },
  { GrKey_Control_F11          , ControlMask, XK_F11 },
  { GrKey_Control_F12          , ControlMask, XK_F12 },
  { GrKey_Control_Home         , ControlMask, XK_Home },
#ifdef XK_KP_Home
  { GrKey_Control_Home         , ControlMask, XK_KP_Home },
#endif
  { GrKey_Control_Insert       , ControlMask, XK_Insert },
#ifdef XK_KP_Insert
  { GrKey_Control_Insert       , ControlMask, XK_KP_Insert },
#endif
#ifdef XK_KP_Subtract
  { GrKey_Control_KPDash       , ControlMask, XK_KP_Subtract },
#endif
#ifdef XK_KP_Add
  { GrKey_Control_KPPlus       , ControlMask, XK_KP_Add },
#endif
  { GrKey_Control_KPSlash      , ControlMask, XK_slash },
  { GrKey_Control_KPStar       , ControlMask, XK_multiply },
  { GrKey_Control_Left         , ControlMask, XK_Left },
#ifdef XK_KP_Left
  { GrKey_Control_Left         , ControlMask, XK_KP_Left },
#endif
  { GrKey_Control_PageDown     , ControlMask, XK_Next },
#ifdef XK_KP_Next
  { GrKey_Control_PageDown     , ControlMask, XK_KP_Next },
#endif
  { GrKey_Control_PageUp       , ControlMask, XK_Prior },
#ifdef XK_KP_Prior
  { GrKey_Control_PageUp       , ControlMask, XK_KP_Prior },
#endif
  { GrKey_Control_Right        , ControlMask, XK_Right },
#ifdef XK_KP_Right
  { GrKey_Control_Right        , ControlMask, XK_KP_Right },
#endif
  { GrKey_Control_Up           , ControlMask, XK_Up },
#ifdef XK_KP_Up
  { GrKey_Control_Up           , ControlMask, XK_KP_Up },
#endif
#ifdef XK_KP_Subtract
  { GrKey_Dash                 , 0, XK_KP_Subtract },
#endif
  { GrKey_Delete               , 0, XK_Delete },
#ifdef XK_KP_Delete
  { GrKey_Delete               , 0, XK_KP_Delete },
#endif
  { GrKey_Down                 , 0, XK_Down },
#ifdef XK_KP_Down
  { GrKey_Down                 , 0, XK_KP_Down },
#endif
  { GrKey_End                  , 0, XK_End },
#ifdef XK_KP_End
  { GrKey_End                  , 0, XK_KP_End },
#endif
  { GrKey_F1                   , 0, XK_F1 },
  { GrKey_F2                   , 0, XK_F2 },
  { GrKey_F3                   , 0, XK_F3 },
  { GrKey_F4                   , 0, XK_F4 },
  { GrKey_F5                   , 0, XK_F5 },
  { GrKey_F6                   , 0, XK_F6 },
  { GrKey_F7                   , 0, XK_F7 },
  { GrKey_F8                   , 0, XK_F8 },
  { GrKey_F9                   , 0, XK_F9 },
  { GrKey_F10                  , 0, XK_F10 },
  { GrKey_F11                  , 0, XK_F11 },
  { GrKey_F12                  , 0, XK_F12 },
  { GrKey_Home                 , 0, XK_Home },
#ifdef XK_KP_Home
  { GrKey_Home                 , 0, XK_KP_Home },
#endif
  { GrKey_Insert               , 0, XK_Insert },
#ifdef XK_KP_Insert
  { GrKey_Insert               , 0, XK_KP_Insert },
#endif
  { GrKey_Left                 , 0, XK_Left },
#ifdef XK_KP_Left
  { GrKey_Left                 , 0, XK_KP_Left },
#endif
  { GrKey_PageDown             , 0, XK_Next },
#ifdef XK_KP_Next
  { GrKey_PageDown             , 0, XK_KP_Next },
#endif
  { GrKey_PageUp               , 0, XK_Prior },
#ifdef XK_KP_Prior
  { GrKey_PageUp               , 0, XK_KP_Prior },
#endif
#ifdef XK_KP_Add
  { GrKey_Plus                 , 0, XK_KP_Add },
#endif
  { GrKey_Print                , 0, XK_Print },
  { GrKey_Right                , 0, XK_Right },
#ifdef XK_KP_Right
  { GrKey_Right                , 0, XK_KP_Right },
#endif
  { GrKey_Shift_Down           , ShiftMask, XK_Down },
  { GrKey_Shift_End            , ShiftMask, XK_End },
  { GrKey_Shift_F1             , ShiftMask, XK_F1 },
  { GrKey_Shift_F2             , ShiftMask, XK_F2 },
  { GrKey_Shift_F3             , ShiftMask, XK_F3 },
  { GrKey_Shift_F4             , ShiftMask, XK_F4 },
  { GrKey_Shift_F5             , ShiftMask, XK_F5 },
  { GrKey_Shift_F6             , ShiftMask, XK_F6 },
  { GrKey_Shift_F7             , ShiftMask, XK_F7 },
  { GrKey_Shift_F8             , ShiftMask, XK_F8 },
  { GrKey_Shift_F9             , ShiftMask, XK_F9 },
  { GrKey_Shift_F10            , ShiftMask, XK_F10 },
  { GrKey_Shift_F11            , ShiftMask, XK_F11 },
  { GrKey_Shift_F12            , ShiftMask, XK_F12 },
  { GrKey_Shift_Home           , ShiftMask, XK_Home },
  { GrKey_Shift_Insert         , ShiftMask, XK_Insert },
  { GrKey_Shift_Left           , ShiftMask, XK_Left },
  { GrKey_Shift_PageDown       , ShiftMask, XK_Next },
  { GrKey_Shift_PageUp         , ShiftMask, XK_Prior },
  { GrKey_Shift_Right          , ShiftMask, XK_Right },
  { GrKey_Shift_Up             , ShiftMask, XK_Up },
  { GrKey_Up                   , 0, XK_Up },
#ifdef XK_KP_Up
  { GrKey_Up                   , 0, XK_KP_Up },
#endif
};

static int _XKeyEventToGrKey (XKeyEvent *xkey)
{
  KeyEntry *kp;
  unsigned int state;
  char buffer[20];
  KeySym keysym;
  int count;

  state = xkey->state & (ShiftMask | ControlMask | Mod1Mask);
  count = XLookupString (xkey,
			 buffer,
			 sizeof(buffer),
			 &keysym, (XComposeStatus *) NULL);

  if ((count == 1) && ((state & Mod1Mask) == 0))
    return (unsigned char) buffer[0];

  for (kp = _KeyTable;
       kp < &_KeyTable[sizeof(_KeyTable)/sizeof(_KeyTable[0])];
       kp = kp + 1)
    {
      if (keysym == kp->keysym && state == kp->state)
	return kp->key;
    }
  /* printf("Unknown: 0x%x\n", (unsigned) keysym); */
  return EOF;
}

static INLINE
unsigned int _XButtonEventToGrButton (XButtonEvent *event)
{
  unsigned int state = event->state;
  unsigned int mask = 0;

  switch (event->button) {
  case Button1: mask = Button1Mask; break;
  case Button2: mask = Button2Mask; break;
  case Button3: mask = Button3Mask; break;
  case Button4: mask = Button4Mask; break;
  case Button5: mask = Button5Mask; break;
  }
  switch (event->type) {
  case ButtonPress:     state |= mask; break;
  case ButtonRelease:   state &= ~mask; break;
  }
  return (  ((state & Button1Mask) ? GR_M_LEFT : 0)
	  | ((state & Button2Mask) ? GR_M_MIDDLE : 0)
	  | ((state & Button3Mask) ? GR_M_RIGHT : 0)
	  | ((state & Button4Mask) ? GR_M_P4 : 0)
	  | ((state & Button5Mask) ? GR_M_P5 : 0));
}

static INLINE
unsigned int _XGrModifierKey (KeySym keysym, int type)
{
  if (type == KeyPress) {
    switch (keysym) {
    case XK_Shift_L:    kbd_lastmod |= GR_KB_LEFTSHIFT; break;
    case XK_Shift_R:    kbd_lastmod |= GR_KB_RIGHTSHIFT; break;
    case XK_Control_L:
    case XK_Control_R:  kbd_lastmod |= GR_KB_CTRL; break;
    case XK_Alt_L:
    case XK_Alt_R:
    case XK_Meta_L:
    case XK_Meta_R:     kbd_lastmod |= GR_KB_ALT; break;
    case XK_Num_Lock:   kbd_lastmod |= GR_KB_NUMLOCK; break;
    case XK_Caps_Lock:  kbd_lastmod |= GR_KB_CAPSLOCK; break;
    case XK_Insert:     kbd_lastmod |= GR_KB_INSERT; break;
    }
  }
  if (type == KeyRelease) {
    switch (keysym) {
    case XK_Shift_L:    kbd_lastmod &= ~GR_KB_LEFTSHIFT; break;
    case XK_Shift_R:    kbd_lastmod &= ~GR_KB_RIGHTSHIFT; break;
    case XK_Control_L:
    case XK_Control_R:  kbd_lastmod &= ~GR_KB_CTRL; break;
    case XK_Alt_L:
    case XK_Alt_R:
    case XK_Meta_L:
    case XK_Meta_R:     kbd_lastmod &= ~GR_KB_ALT; break;
    case XK_Num_Lock:   kbd_lastmod &= ~GR_KB_NUMLOCK; break;
    case XK_Caps_Lock:  kbd_lastmod &= ~GR_KB_CAPSLOCK; break;
    case XK_Insert:     kbd_lastmod &= ~GR_KB_INSERT; break;
    }
  }
  return kbd_lastmod;
}

void _GrUpdateInputs(void)
{
  int count;

#if 0
  if (_XGrTickValue == -1) {
    struct itimerval it;

    _XGrTickHandler (SIGALRM);
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = MS_PER_TICK * 1000L;
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = MS_PER_TICK * 1000L;
    setitimer (ITIMER_REAL, &it, NULL);
  }
#endif

  if (_XGrDisplay) {
    count = XEventsQueued (_XGrDisplay, QueuedAfterReading);
    if (count <= 0) {
      XFlush (_XGrDisplay);
      return;
    }
    while (--count >= 0) {
      GrMouseEvent ev;
      XEvent xev;
      KeySym keysym;
      int btn;

      XNextEvent (_XGrDisplay, &xev);
      switch (xev.type) {
      case Expose:
	  _XGrCopyBStore(xev.xexpose.x, xev.xexpose.y,
	    xev.xexpose.width, xev.xexpose.height);
	  break;

      case MotionNotify:
	if (mou_enabled && (MOUINFO->msstatus == 2)) {
	  if (_XGrWindowedMode) {
	    MOUINFO->xpos = xev.xmotion.x;
	    MOUINFO->ypos = xev.xmotion.y;
	  }
	  else {
	    MOUINFO->xpos += xev.xmotion.x;
	    MOUINFO->ypos += xev.xmotion.y;
	  }
	  MOUINFO->moved  = TRUE;
	  MouseMoveTime   = xev.xmotion.time;
	}
	break;

      case ButtonPress:
      case ButtonRelease:
	if (mou_enabled && (MOUINFO->msstatus == 2)) {
	  btn = _XButtonEventToGrButton (&xev.xbutton);
	  if(btn != mou_buttons) {
	    fill_mouse_ev(
			  ev,
			  mou_buttons,btn,
			  GR_M_LEFT,
			  GR_M_MIDDLE,
			  GR_M_RIGHT,
			  GR_M_P4,
			  GR_M_P5,
			  kbd_lastmod
			  );
	    if (evt_lasttime_ok)
	      ev.dtime = xev.xbutton.time - evt_lasttime;
	    else {
	      ev.dtime = 1;
	      evt_lasttime_ok = TRUE;
	    }
	    evt_lasttime = xev.xbutton.time;
	    enqueue_event(ev);
	    MOUINFO->moved = FALSE;
	    mou_buttons = btn;
	  }
	}
	break;

      case KeyPress:
	keysym = XKeycodeToKeysym (_XGrDisplay, xev.xkey.keycode, 0);
	if (IsModifierKey (keysym)) {
	  _XGrModifierKey (keysym, xev.type);
	}
	else if (kbd_enabled) {
	  fill_keybd_ev(
			ev,
			_XKeyEventToGrKey (&xev.xkey),
			kbd_lastmod
			);
	  if (evt_lasttime_ok)
	    ev.dtime = xev.xkey.time - evt_lasttime;
	  else {
	    ev.dtime = 1;
	    evt_lasttime_ok = TRUE;
	  }
	  evt_lasttime = xev.xkey.time;
	  enqueue_event(ev);
	  MOUINFO->moved = FALSE;
	}
	break;

      case KeyRelease:
	keysym = XKeycodeToKeysym (_XGrDisplay, xev.xkey.keycode, 0);
	if (IsModifierKey (keysym)) {
	  _XGrModifierKey (keysym, xev.type);
	}
	break;
      }
    }
  }
}

void GrMouseGetEventT(int flags,GrMouseEvent *ev,long tout)
{
  int  msdraw;
  ev->flags = 0;
  if (MOUINFO->msstatus == 0) GrMouseInit();
  if (MOUINFO->msstatus == 0) return;
  msdraw = !MOUINFO->displayed && !(flags & GR_M_NOPAINT);
  if (msdraw) GrMouseDisplayCursor();
  /* Note: avoid zero timeout for select */
  /* I don't agree. Zero timeout is still needed when I want to display soft */
  /* real time data while looking for some keypress at the same time (A.Pavenis) */
  /* if (tout <= 0L) tout = 1L; */
  if (tout < 0L) tout = 0L;
  for( ; _XGrDisplay ; ) {
    struct timeval tval;
    fd_set readfds;
    int fd;
    /*
     * Note: The select call with nonzero timeout avoids CPU usage
     *       of nearly 100%
     */
    fd = ConnectionNumber(_XGrDisplay);
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    tval.tv_sec = tout / 1000L;
    tval.tv_usec = (tout % 1000) * 1000L;
    select (fd + 1,
	    &readfds,
	    (fd_set *) 0,
	    (fd_set *) 0,
	    &tval);
    tout = tval.tv_sec * 1000L + tval.tv_usec / 1000L;

    _GrUpdateInputs();
    GrMouseUpdateCursor();
    while (MOUINFO->qlength > 0) {
      dequeue_event((*ev));
      if (ev->flags & flags) {
	if (msdraw) GrMouseEraseCursor();
	return;
      }
    }
    if ((flags & GR_M_POLL) ||
	(tout == 0L) ||
	(MOUINFO->moved && (flags & GR_M_MOTION))) {
      fill_mouse_ev(
		    (*ev),
		    mou_buttons,mou_buttons,
		    GR_M_LEFT,
		    GR_M_MIDDLE,
		    GR_M_RIGHT,
		    GR_M_P4,
		    GR_M_P5,
		    kbd_lastmod
		    );
      if ( ev->flags ) {
	/* something happend */
	if (MOUINFO->moved) {
	  if (evt_lasttime_ok && MouseMoveTime) {
	    ev->dtime = MouseMoveTime - evt_lasttime;
	    evt_lasttime = MouseMoveTime;
	  } else
	    ev->dtime = 1;
	}
	/* otherwise the ev->dtime is ok */
      } else
	ev->dtime = -1; /* special time if nothing happend */
      MOUINFO->moved = FALSE;
      MouseMoveTime = 0;
      if (msdraw) GrMouseEraseCursor();
      return;
    }
    /* Make sure we don't use all the CPU */
    /* Is this the right way, Andris? */
    if (tout == 0L) tout = 1L;
  }
}


int _XGrKeyboardHit (void)
{
  XEvent xev;
  KeySym keysym;

  if (_XGrDisplay) {
    if (XEventsQueued (_XGrDisplay, QueuedAfterReading) <= 0)   {
      XFlush (_XGrDisplay);
      return FALSE;
    }
    while (XCheckMaskEvent (_XGrDisplay, KeyPressMask|KeyReleaseMask, &xev)) {
      keysym = XKeycodeToKeysym (_XGrDisplay, xev.xkey.keycode, 0);
      if (IsModifierKey (keysym)) {
	_XGrModifierKey (keysym, xev.type);
	continue;
      }
      if (xev.type == KeyPress) {
	XPutBackEvent (_XGrDisplay, &xev);
	return TRUE;
      }
    }
  }
  return FALSE;
}

int _XGrKeyboardGetKey (void)
{
  XEvent xev;
  KeySym keysym;

  if (_XGrDisplay) {
    while (XMaskEvent (_XGrDisplay, KeyPressMask|KeyReleaseMask, &xev)) {
      keysym = XKeycodeToKeysym (_XGrDisplay, xev.xkey.keycode, 0);
      if (IsModifierKey (keysym)) {
	_XGrModifierKey (keysym, xev.type);
	continue;
      }
      if (xev.type == KeyPress) {
	return _XKeyEventToGrKey (&xev.xkey);
      }
    }
  }
  return 0;
}

int _XGrKeyboardGetState (void)
{
  return kbd_lastmod;
}

