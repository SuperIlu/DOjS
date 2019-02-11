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

void __gr_bar(int left, int top, int right, int bottom)
{
  _DO_INIT_CHECK;

  if (left > right) SWAP(int,left,right);
  if (bottom < top) SWAP(int,bottom,top);

  left += VL;    right  += VL;
  top  += VT+PY; bottom += VT+PY;

  switch (FPATT) {
    case SOLID_FILL : GrFilledBox( left, top, right, bottom, FILL);
		      break;
    case EMPTY_FILL : GrFilledBox( left, top, right, bottom, COLBG);
		      break;
    default         : FILLP.gp_bmp_fgcolor = FILL;
		      FILLP.gp_bmp_bgcolor = COLBG;
		      GrPatternFilledBox( left, top, right, bottom, &FILLP);
		      break;
  }
}


