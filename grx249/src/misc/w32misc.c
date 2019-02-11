/**
 ** w32misc.c - miscellaneous functions for Win32
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

#include "libgrx.h"
#include "libwin32.h"

void GrSetWindowTitle( char *title )
{
  SetWindowText( hGRXWnd,title );
}

void GrSleep( int msec )
{
  Sleep( msec );
}

void GrFlush( void)
{
}

