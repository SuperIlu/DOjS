/**
 ** sdlmisc.c - miscellaneous functions for SDL
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

#include "libgrx.h"
#include <SDL/SDL.h>

void GrSetWindowTitle(char *title)
{
	SDL_WM_SetCaption(title, NULL);
}

void GrSleep(int msec)
{
	SDL_Delay(msec);
}

void GrFlush( void)
{
}
