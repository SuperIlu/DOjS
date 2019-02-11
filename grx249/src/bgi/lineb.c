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

void __gr_line(int x1, int y1, int x2, int y2)
{
  _DO_INIT_CHECK;
  moveto( x2, y2);
  x1 += VL;    x2 += VL;
  y1 += VT+PY; y2 += VT+PY;
  if (__gr_lstyle == SOLID_LINE && LNE.lno_width == 1) {
    if (y1==y2)
      GrHLine(x1, x2, y1, COL|WR);
    else
    if (x1==x2)
      GrVLine(x1, y1, y2, COL|WR);
    else
      GrLine( x1, y1, x2, y2, COL|WR);
  } else {
    LNE.lno_color= COL|WR;
    GrCustomLine( x1, y1, x2, y2, &LNE);
  }
}
