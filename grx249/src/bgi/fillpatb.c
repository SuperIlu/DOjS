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

/* ----------------------------------------------------------------- */
void setfillpattern( char  *upattern, int color)
{
  _DO_INIT_CHECK;
  FILL  = color;
  FPATT = USER_FILL;
  memcpy(__gr_fpatterns[USER_FILL], upattern, 8);
  FILLP.gp_bmp_data = (unsigned char *)&__gr_fpatterns[USER_FILL];
}

