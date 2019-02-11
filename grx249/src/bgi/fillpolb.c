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

#include <stdlib.h>
#include "bccgrx00.h"

/*
   - Take care : Borland C defines polypoints as int * but
   -             assumes struct pointtype *  .
   -             GRX requires int points[][2] !
   - The good news are : Both definitions are compatible !
*/

void fillpoly(int numpoints, void *polypoints)
{
  void *cpp;

  _DO_INIT_CHECK;
  if (VL != 0 || VT+PY != 0) {
    int i, *ppd, *pps;

    pps = polypoints;
    ppd = cpp = alloca( sizeof(int) * 2 * numpoints);
    if (cpp==NULL) {
      ERR = grNoScanMem;
      return;
    }
    for (i=0; i < numpoints; ++i) {
      *(ppd++) = *(pps++) + VL;
      *(ppd++) = *(pps++) + VT+PY;
    }
  } else
    cpp = polypoints;

  switch (FPATT) {
    case SOLID_FILL :
      GrFilledPolygon(numpoints, cpp, FILL);
      if (COL != FILL)
	__gr_drawpol(numpoints, polypoints, TRUE);
      break;
    case EMPTY_FILL :
      GrFilledPolygon(numpoints, cpp, COLBG);
      if (COL != COLBG)
	__gr_drawpol(numpoints, polypoints, TRUE);
      break;
    default :
      FILLP.gp_bmp_fgcolor = FILL;
      FILLP.gp_bmp_bgcolor = COLBG;
      GrPatternFilledPolygon( numpoints, cpp, &FILLP);
      __gr_drawpol( numpoints, polypoints, TRUE);
      break;
  }
}
