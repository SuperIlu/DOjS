/**
 ** x11misc.c - miscellaneous functions for X11
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

#include <unistd.h>
#include "libgrx.h"
#include "libxwin.h"
#include "mouse/input.h"

void GrSetWindowTitle( char *title )
{
  XStoreName( _XGrDisplay,_XGrWindow,title );
  XSetIconName( _XGrDisplay,_XGrWindow,title );
}

void GrSleep( int msec )
{
  usleep( msec*1000L );
}

void GrFlush( void )
{
  _GrUpdateInputs();
  XFlush(_XGrDisplay);
}
