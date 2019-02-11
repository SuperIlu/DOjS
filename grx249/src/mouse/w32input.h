/**
 ** w32input.h ---- Win32 to GRX keys conversion tables
 **
 ** Copyright (C) 2001 Mariano Alvarez Fernandez
 ** [e-mail: malfer@teleline.es]
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

#ifndef _W32INPUT_H_
#define _W32INPUT_H_

typedef struct {
    int winkey;
    GrKeyType grkey;
} keytrans;

#define NSTDKEYS 22

static keytrans stdkeys[NSTDKEYS] = {
    {VK_PRIOR, GrKey_PageUp},
    {VK_NEXT, GrKey_PageDown},
    {VK_END, GrKey_End},
    {VK_HOME, GrKey_Home},
    {VK_LEFT, GrKey_Left},
    {VK_UP, GrKey_Up},
    {VK_RIGHT, GrKey_Right},
    {VK_DOWN, GrKey_Down},
    {VK_INSERT, GrKey_Insert},
    {VK_DELETE, GrKey_Delete},
    {VK_F1, GrKey_F1},
    {VK_F2, GrKey_F2},
    {VK_F3, GrKey_F3},
    {VK_F4, GrKey_F4},
    {VK_F5, GrKey_F5},
    {VK_F6, GrKey_F6},
    {VK_F7, GrKey_F7},
    {VK_F8, GrKey_F8},
    {VK_F9, GrKey_F9},
    {VK_F10, GrKey_F10},
    {VK_F11, GrKey_F11},
    {VK_F12, GrKey_F12}
};

static keytrans altstdkeys[NSTDKEYS] = {
    {VK_PRIOR, GrKey_Alt_PageUp},
    {VK_NEXT, GrKey_Alt_PageDown},
    {VK_END, GrKey_Alt_End},
    {VK_HOME, GrKey_Alt_Home},
    {VK_LEFT, GrKey_Alt_Left},
    {VK_UP, GrKey_Alt_Up},
    {VK_RIGHT, GrKey_Alt_Right},
    {VK_DOWN, GrKey_Alt_Down},
    {VK_INSERT, GrKey_Alt_Insert},
    {VK_DELETE, GrKey_Alt_Delete},
    {VK_F1, GrKey_Alt_F1},
    {VK_F2, GrKey_Alt_F2},
    {VK_F3, GrKey_Alt_F3},
    {VK_F4, GrKey_Alt_F4},
    {VK_F5, GrKey_Alt_F5},
    {VK_F6, GrKey_Alt_F6},
    {VK_F7, GrKey_Alt_F7},
    {VK_F8, GrKey_Alt_F8},
    {VK_F9, GrKey_Alt_F9},
    {VK_F10, GrKey_Alt_F10},
    {VK_F11, GrKey_Alt_F11},
    {VK_F12, GrKey_Alt_F12}
};

static keytrans controlstdkeys[NSTDKEYS] = {
    {VK_PRIOR, GrKey_Control_PageUp},
    {VK_NEXT, GrKey_Control_PageDown},
    {VK_END, GrKey_Control_End},
    {VK_HOME, GrKey_Control_Home},
    {VK_LEFT, GrKey_Control_Left},
    {VK_UP, GrKey_Control_Up},
    {VK_RIGHT, GrKey_Control_Right},
    {VK_DOWN, GrKey_Control_Down},
    {VK_INSERT, GrKey_Control_Insert},
    {VK_DELETE, GrKey_Control_Delete},
    {VK_F1, GrKey_Control_F1},
    {VK_F2, GrKey_Control_F2},
    {VK_F3, GrKey_Control_F3},
    {VK_F4, GrKey_Control_F4},
    {VK_F5, GrKey_Control_F5},
    {VK_F6, GrKey_Control_F6},
    {VK_F7, GrKey_Control_F7},
    {VK_F8, GrKey_Control_F8},
    {VK_F9, GrKey_Control_F9},
    {VK_F10, GrKey_Control_F10},
    {VK_F11, GrKey_Control_F11},
    {VK_F12, GrKey_Control_F12}
};

static keytrans shiftstdkeys[NSTDKEYS] = {
    {VK_PRIOR, GrKey_Shift_PageUp},
    {VK_NEXT, GrKey_Shift_PageDown},
    {VK_END, GrKey_Shift_End},
    {VK_HOME, GrKey_Shift_Home},
    {VK_LEFT, GrKey_Shift_Left},
    {VK_UP, GrKey_Shift_Up},
    {VK_RIGHT, GrKey_Shift_Right},
    {VK_DOWN, GrKey_Shift_Down},
    {VK_INSERT, GrKey_Shift_Insert},
    {VK_DELETE, 0},
    {VK_F1, GrKey_Shift_F1},
    {VK_F2, GrKey_Shift_F2},
    {VK_F3, GrKey_Shift_F3},
    {VK_F4, GrKey_Shift_F4},
    {VK_F5, GrKey_Shift_F5},
    {VK_F6, GrKey_Shift_F6},
    {VK_F7, GrKey_Shift_F7},
    {VK_F8, GrKey_Shift_F8},
    {VK_F9, GrKey_Shift_F9},
    {VK_F10, GrKey_Shift_F10},
    {VK_F11, GrKey_Shift_F11},
    {VK_F12, GrKey_Shift_F12}
};

static GrKeyType altletters[] = {
    GrKey_Alt_A,
    GrKey_Alt_B,
    GrKey_Alt_C,
    GrKey_Alt_D,
    GrKey_Alt_E,
    GrKey_Alt_F,
    GrKey_Alt_G,
    GrKey_Alt_H,
    GrKey_Alt_I,
    GrKey_Alt_J,
    GrKey_Alt_K,
    GrKey_Alt_L,
    GrKey_Alt_M,
    GrKey_Alt_N,
    GrKey_Alt_O,
    GrKey_Alt_P,
    GrKey_Alt_Q,
    GrKey_Alt_R,
    GrKey_Alt_S,
    GrKey_Alt_T,
    GrKey_Alt_U,
    GrKey_Alt_V,
    GrKey_Alt_W,
    GrKey_Alt_X,
    GrKey_Alt_Y,
    GrKey_Alt_Z
};

static GrKeyType altnumbers[] = {
    GrKey_Alt_0,
    GrKey_Alt_1,
    GrKey_Alt_2,
    GrKey_Alt_3,
    GrKey_Alt_4,
    GrKey_Alt_5,
    GrKey_Alt_6,
    GrKey_Alt_7,
    GrKey_Alt_8,
    GrKey_Alt_9
};

#endif
