/**
 ** BCC2GRX  -  Interfacing Borland based graphics programs to LIBGRX
 ** Copyright (C) 1993-97 by Hartmut Schirmer
 **
 **
 ** Contact :                Hartmut Schirmer
 **                          Feldstrasse 118
 **                  D-24105 Kiel
 **                          Germany
 **
 ** e-mail : hsc@techfak.uni-kiel.de
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

#include "bccgrx00.h"

/*
   - Take care : Borland C defines polypoints as int * but
   -             assumes struct pointtype *  .
   -             GRX requires int points[][2] !
   - The good news are : Both definitions are compatible !
*/

void __gr_drawpol(int numpoints, void *polypoints, int close)
{
  int *pp, x, y, sx, sy, nx, ny, fast;

  _DO_INIT_CHECK;
  LNE.lno_color = COL|WR;
  fast = (__gr_lstyle == SOLID_LINE) && (LNE.lno_width == 1);
  pp = (int *)polypoints;
  while (numpoints > 0) {
    x = sx = *(pp++)+VL;
    y = sy = *(pp++)+VT+PY;
    --numpoints;
    while (numpoints > 0)  {
      nx = *(pp++) + VL;
      ny = *(pp++) + VT + PY;
      if (fast) GrLine( x, y, nx, ny, LNE.lno_color);
	   else GrCustomLine( x, y, nx, ny, &LNE);
      x = nx; y = ny;
      --numpoints;
      if ( x==sx && y==sy)
	break;
    }
    if ( close && (x != sx || y != sy))
    {
      if (fast) GrLine( x, y, sx, sy, LNE.lno_color);
	   else GrCustomLine( x, y, sx, sy, &LNE);
    }
  }
}

void __gr_drawpoly(int numpoints, void *polypoints) {
  drawpoly(numpoints, polypoints);
}

