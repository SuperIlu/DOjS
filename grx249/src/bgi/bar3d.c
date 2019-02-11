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

void bar3d(int left,int top,int right,int bottom,int depth, int topflag)
{
  int yofs, l_d, r_d, t_y, fast, col;

  _DO_INIT_CHECK;

  if (left > right) SWAP(int,left,right);
  if (bottom < top) SWAP(int,bottom,top);

  __gr_bar(left,top,right,bottom);

  left   += VL;
  top    += VT+PY;
  right  += VL;
  bottom += VT+PY;
  if (left > right) SWAP(int,left,right);
  if (bottom < top) SWAP(int,bottom,top);

  fast = (__gr_lstyle == SOLID_LINE) && (LNE.lno_width == 1);
  LNE.lno_color = col = COL|WR;
  if (fast) GrBox( left, top, right, bottom, col);
       else GrCustomBox( left, top, right, bottom, &LNE);

  if (depth == 0) return;

  yofs = -depth * getmaxy() / getmaxx();
  r_d = right+depth;
  t_y = top + yofs;
  if (fast) {
    GrLine( right, bottom, r_d, bottom+yofs, col);
    GrVLine(r_d, bottom+yofs, t_y, col);
  } else {
    GrCustomLine( right, bottom, r_d, bottom+yofs, &LNE);
    GrCustomLine( r_d, bottom+yofs, r_d, t_y, &LNE);
  }
  if (topflag) {
    l_d = left+depth;
    if (fast) {
      GrHLine( r_d, l_d, t_y, col);
      GrLine( l_d, t_y, left, top, col);
      GrLine( r_d, t_y, right, top, col);
    } else {
      GrCustomLine( r_d, t_y, l_d, t_y, &LNE);
      GrCustomLine( l_d, t_y, left, top, &LNE);
      GrCustomLine( r_d, t_y, right, top, &LNE);
    }
  }
}
