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

void fillellipse( int x, int y, int xradius, int yradius)
{
  _DO_INIT_CHECK;
  x += VL;
  y += VT+PY;
  xradius = XR(xradius);
  yradius = YR(yradius);
  switch (FPATT) {
    case SOLID_FILL :
      GrFilledEllipse( x, y, xradius, yradius, FILL);
      if (COL != FILL)
	GrEllipse( x, y, xradius, yradius, COL);
      break;
    case EMPTY_FILL :
      GrFilledEllipse( x, y, xradius, yradius, COLBG);
      if (COL != COLBG)
	GrEllipse( x, y, xradius, yradius, COL);
      break;
    default :
      FILLP.gp_bmp_fgcolor = FILL;
      FILLP.gp_bmp_bgcolor = COLBG;
      GrPatternFilledEllipse( x, y, xradius, yradius, &FILLP);
      GrEllipse( x, y, xradius, yradius, COL);
      break;
  }
}

