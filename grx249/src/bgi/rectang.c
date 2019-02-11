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

void rectangle(int left, int top, int right, int bottom)
{
  _DO_INIT_CHECK;
  if (__gr_lstyle == SOLID_LINE && LNE.lno_width == 1)
    GrBox( left+VL, top+VT+PY, right+VL, bottom+VT+PY, COL|WR);
  else {
    LNE.lno_color = COL|WR;
    GrCustomBox( left+VL, top+VT+PY, right+VL, bottom+VT+PY, &__gr_Line);
  }
}

