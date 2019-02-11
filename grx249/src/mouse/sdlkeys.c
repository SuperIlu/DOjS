/**
 ** sdlkeys.c ---- DOS (TCC/BCC/DJGPP: "conio.h") style keyboard utilities
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

#include "libsdl.h"
#include "libgrx.h"
#include "input.h"
#include "grxkeys.h"

int GrKeyPressed(void)
{
    if (MOUINFO->msstatus < 2) {
	GrMouseInit();
        GrMouseEventEnable(1, 0);
    }
    return _GrKeyPressed();
}

GrKeyType GrKeyRead(void)
{
    GrMouseEvent ev;
    int i, key;

    if (!_GrIsKbdEnabled()) {
	while (_nkeyssdlpool < 1)
	    _GrUpdateInputs();
	key = _keyssdlpool[0];
	for (i = 0; i < _nkeyssdlpool; i++)
	    _keyssdlpool[i] = _keyssdlpool[i + 1];
	_nkeyssdlpool--;
	return key;
    }

    if (MOUINFO->msstatus < 2) {
	GrMouseInit();
	GrMouseEventEnable(1, 0);
    }
    for (;;) {
	GrMouseGetEvent((GR_M_EVENT | GR_M_NOPAINT), &ev);
	if (ev.flags & GR_M_KEYPRESS) {
	    return (ev.key);
	}
    }
}

int GrKeyStat(void)
{
    static SDLMod trans[8] = {
	KMOD_RSHIFT, KMOD_LSHIFT, KMOD_CTRL, KMOD_ALT,
	KMOD_SCROLL, KMOD_NUM, KMOD_CAPS, KMOD_INSERT
    };

    SDLMod mod = SDL_GetModState();
    int i, state = 0;

    for(i = 0; i < 8; i++)
	if (mod & trans[i]) state |= 1 << i;

    return(state);
}

int kbhit(void)
{
    return GrKeyPressed();
}

int getkey(void)
{
    return (int)GrKeyRead();
}

int getch(void)
{
    static int lastkey = (-1);
    int key;

    if (lastkey != (-1)) {
	key = lastkey;
	lastkey = (-1);
	return (key);
    }
    key = getkey();
    if (key < 0x100) {
	return (key);
    }
    lastkey = key & 0xff;

    return (0);
}

int getkbstat(void)
{
    return GrKeyStat();
}

