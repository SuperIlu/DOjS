/**
 ** libsdl.h - GRX library SDL private include file
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

#ifndef _LIBSDL_H_

#include <SDL/SDL.h>

#define KMOD_SCROLL ((SDLMod) 0x10)
#define KMOD_INSERT ((SDLMod) 0x20)

extern SDL_Surface *_SGrScreen;
#if defined(__WIN32__)
extern void *_SGrBackup;
extern int _SGrLength;
extern int _SGrActive;
#endif

extern int _GrIsKbdEnabled(void);
extern int _GrKeyPressed(void);

/* _keyssdlpool used only when GrMouseEventEnable( 0,x ) is set */

#define  _MAXKEYSSDLPOOL 16
extern int _nkeyssdlpool;
extern int _keyssdlpool[_MAXKEYSSDLPOOL];

#if defined(__WIN32__)
#include <SDL/SDL_syswm.h>
#include <windows.h>
#elif defined(__XWIN__)
#include <unistd.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#define _LIBSDL_H_ 1
#endif
