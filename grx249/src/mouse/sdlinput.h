/**
 ** sdlinput.h ---- SDL to GRX keys conversion tables
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
 **/

#ifndef _SDLINPUT_H_
#define _SDLINPUT_H_

typedef struct {
    SDLKey sdlkey;
    GrKeyType grkey;
} keytrans;

#define NSTDKEYS 33

static keytrans stdkeys[NSTDKEYS] = {
    { SDLK_PAGEUP, GrKey_PageUp },
    { SDLK_PAGEDOWN, GrKey_PageDown },
    { SDLK_END, GrKey_End },
    { SDLK_HOME, GrKey_Home },
    { SDLK_LEFT, GrKey_Left },
    { SDLK_UP, GrKey_Up },
    { SDLK_RIGHT, GrKey_Right },
    { SDLK_DOWN, GrKey_Down },
    { SDLK_INSERT, GrKey_Insert },
    { SDLK_DELETE, GrKey_Delete },
    { SDLK_F1, GrKey_F1 },
    { SDLK_F2, GrKey_F2 },
    { SDLK_F3, GrKey_F3 },
    { SDLK_F4, GrKey_F4 },
    { SDLK_F5, GrKey_F5 },
    { SDLK_F6, GrKey_F6 },
    { SDLK_F7, GrKey_F7 },
    { SDLK_F8, GrKey_F8 },
    { SDLK_F9, GrKey_F9 },
    { SDLK_F10, GrKey_F10 },
    { SDLK_F11, GrKey_F11 },
    { SDLK_F12, GrKey_F12 },
    { SDLK_KP5, GrKey_Center },
    { SDLK_KP_DIVIDE, GrKey_Slash },
    { SDLK_KP_MULTIPLY, GrKey_Star },
    { SDLK_KP_MINUS, GrKey_Dash },
    { SDLK_KP_PLUS, GrKey_Plus },
    { SDLK_KP_ENTER, GrKey_Return },
    { SDLK_KP_EQUALS, GrKey_Equals },
    { SDLK_BACKSPACE, GrKey_BackSpace },
    { SDLK_TAB, GrKey_Tab },
    { SDLK_RETURN, GrKey_Return },
    { SDLK_ESCAPE, GrKey_Escape }
};

static keytrans altstdkeys[NSTDKEYS] = {
    { SDLK_PAGEUP, GrKey_Alt_PageUp },
    { SDLK_PAGEDOWN, GrKey_Alt_PageDown },
    { SDLK_END, GrKey_Alt_End },
    { SDLK_HOME, GrKey_Alt_Home },
    { SDLK_LEFT, GrKey_Alt_Left },
    { SDLK_UP, GrKey_Alt_Up },
    { SDLK_RIGHT, GrKey_Alt_Right },
    { SDLK_DOWN, GrKey_Alt_Down },
    { SDLK_INSERT, GrKey_Alt_Insert },
    { SDLK_DELETE, GrKey_Alt_Delete },
    { SDLK_F1, GrKey_Alt_F1 },
    { SDLK_F2, GrKey_Alt_F2 },
    { SDLK_F3, GrKey_Alt_F3 },
    { SDLK_F4, GrKey_Alt_F4 },
    { SDLK_F5, GrKey_Alt_F5 },
    { SDLK_F6, GrKey_Alt_F6 },
    { SDLK_F7, GrKey_Alt_F7 },
    { SDLK_F8, GrKey_Alt_F8 },
    { SDLK_F9, GrKey_Alt_F9 },
    { SDLK_F10, GrKey_Alt_F10 },
    { SDLK_F11, GrKey_Alt_F11 },
    { SDLK_F12, GrKey_Alt_F12 },
    { SDLK_KP5, GrKey_Alt_Center },
    { SDLK_KP_DIVIDE, GrKey_Alt_KPSlash },
    { SDLK_KP_MULTIPLY, GrKey_Alt_KPStar },
    { SDLK_KP_MINUS, GrKey_Alt_KPMinus },
    { SDLK_KP_PLUS, GrKey_Alt_KPPlus },
    { SDLK_KP_ENTER, GrKey_Alt_Enter },
    { SDLK_KP_EQUALS, GrKey_Alt_Equals },
    { SDLK_BACKSPACE, GrKey_Alt_Backspace },
    { SDLK_TAB, GrKey_Alt_Tab },
    { SDLK_RETURN, GrKey_Alt_Return },
    { SDLK_ESCAPE, GrKey_Alt_Escape }
};

static keytrans controlstdkeys[NSTDKEYS] = {
    { SDLK_PAGEUP, GrKey_Control_PageUp },
    { SDLK_PAGEDOWN, GrKey_Control_PageDown },
    { SDLK_END, GrKey_Control_End },
    { SDLK_HOME, GrKey_Control_Home },
    { SDLK_LEFT, GrKey_Control_Left },
    { SDLK_UP, GrKey_Control_Up },
    { SDLK_RIGHT, GrKey_Control_Right },
    { SDLK_DOWN, GrKey_Control_Down },
    { SDLK_INSERT, GrKey_Control_Insert },
    { SDLK_DELETE, GrKey_Control_Delete },
    { SDLK_F1, GrKey_Control_F1 },
    { SDLK_F2, GrKey_Control_F2 },
    { SDLK_F3, GrKey_Control_F3 },
    { SDLK_F4, GrKey_Control_F4 },
    { SDLK_F5, GrKey_Control_F5 },
    { SDLK_F6, GrKey_Control_F6 },
    { SDLK_F7, GrKey_Control_F7 },
    { SDLK_F8, GrKey_Control_F8 },
    { SDLK_F9, GrKey_Control_F9 },
    { SDLK_F10, GrKey_Control_F10 },
    { SDLK_F11, GrKey_Control_F11 },
    { SDLK_F12, GrKey_Control_F12 },
    { SDLK_KP5, GrKey_Control_Center },
    { SDLK_KP_DIVIDE, GrKey_Control_KPSlash },
    { SDLK_KP_MULTIPLY, GrKey_Control_KPStar },
    { SDLK_KP_MINUS, GrKey_Control_KPDash },
    { SDLK_KP_PLUS, GrKey_Control_KPPlus },
    { SDLK_KP_ENTER, GrKey_LineFeed },
    { SDLK_KP_EQUALS, GrKey_NoKey },
    { SDLK_BACKSPACE, GrKey_Control_Backspace },
    { SDLK_TAB, GrKey_Control_Tab },
    { SDLK_RETURN, GrKey_LineFeed },
    { SDLK_ESCAPE, GrKey_NoKey }
};

static keytrans shiftstdkeys[NSTDKEYS] = {
    { SDLK_PAGEUP, GrKey_Shift_PageUp },
    { SDLK_PAGEDOWN, GrKey_Shift_PageDown },
    { SDLK_END, GrKey_Shift_End },
    { SDLK_HOME, GrKey_Shift_Home },
    { SDLK_LEFT, GrKey_Shift_Left },
    { SDLK_UP, GrKey_Shift_Up },
    { SDLK_RIGHT, GrKey_Shift_Right },
    { SDLK_DOWN, GrKey_Shift_Down },
    { SDLK_INSERT, GrKey_Shift_Insert },
    { SDLK_DELETE, GrKey_NoKey },
    { SDLK_F1, GrKey_Shift_F1 },
    { SDLK_F2, GrKey_Shift_F2 },
    { SDLK_F3, GrKey_Shift_F3 },
    { SDLK_F4, GrKey_Shift_F4 },
    { SDLK_F5, GrKey_Shift_F5 },
    { SDLK_F6, GrKey_Shift_F6 },
    { SDLK_F7, GrKey_Shift_F7 },
    { SDLK_F8, GrKey_Shift_F8 },
    { SDLK_F9, GrKey_Shift_F9 },
    { SDLK_F10, GrKey_Shift_F10 },
    { SDLK_F11, GrKey_Shift_F11 },
    { SDLK_F12, GrKey_Shift_F12 },
    { SDLK_KP5, GrKey_NoKey },
    { SDLK_KP_DIVIDE, GrKey_Slash },
    { SDLK_KP_MULTIPLY, GrKey_Star },
    { SDLK_KP_MINUS, GrKey_Dash },
    { SDLK_KP_PLUS, GrKey_Plus },
    { SDLK_KP_ENTER, GrKey_Return },
    { SDLK_KP_EQUALS, GrKey_Equals },
    { SDLK_BACKSPACE, GrKey_BackSpace },
    { SDLK_TAB, GrKey_BackTab },
    { SDLK_RETURN, GrKey_Return },
    { SDLK_ESCAPE, GrKey_Escape }
};

static char *numchars = "0123456789.";

static GrKeyType numkeys[] = {
    SDLK_INSERT,
    SDLK_END,
    SDLK_DOWN,
    SDLK_PAGEDOWN,
    SDLK_LEFT,
    SDLK_KP5,
    SDLK_RIGHT,
    SDLK_HOME,
    SDLK_UP,
    SDLK_PAGEUP,
    SDLK_DELETE
};

#endif
