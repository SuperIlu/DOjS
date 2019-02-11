/**
 ** libwin32.h - GRX library Win32-API private include file
 **
 ** Author:	Gernot Graeff
 ** E-mail:	gernot.graeff@t-online.de
 ** Date:	13.11.98
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

#ifndef _LIBWIN32_H_
#define _LIBWIN32_H_

#include <windows.h>
#include "grx20.h"

typedef struct _W32Event {
    UINT uMsg;
    WPARAM wParam;
    LPARAM lParam;
    int kbstat;
} W32Event;

extern CRITICAL_SECTION _csEventQueue;
extern W32Event *_W32EventQueue;
extern volatile int _W32EventQueueSize;
extern volatile int _W32EventQueueRead;
extern volatile int _W32EventQueueWrite;
extern volatile int _W32EventQueueLength;

extern HWND hGRXWnd;
extern HDC hDCMem;
extern HANDLE windowThread;

extern int _GrIsKbdEnabled(void);
extern int _GrKeyPressed(void);
extern int _GrKeyStat(void);

/* _keysw32pool used only when GrMouseEventEnable( 0,x ) is set */

#define  _MAXKEYSW32POOL 16
extern int _nkeysw32pool;
extern int _keysw32pool[_MAXKEYSW32POOL];

#endif
