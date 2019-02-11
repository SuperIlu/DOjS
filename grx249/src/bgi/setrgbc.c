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

int __gr_setrgbcolor(int r, int g, int b) {
#ifdef GRX_VERSION
  COL = (int) GrBuildRGBcolorT(r,g,b);
  return COL;
#else
  switch (GrNumColors()) {
    case 1<<15 : return COL =  ((r&0xf8)<<7)
			     | ((g&0xf8)<<2)
			     | ((b&0xf8)>>3);
    case 1<<16 : return COL =   ((r&0xf8)<<8)
			     | ((g&0xfc)<<3)
			     | ((b&0xf8)>>3);
    case 1<<24 : return COL =  ((r&0xff)<<16)
			     | ((g&0xff)<< 8)
			     | ((b&0xff)    );
  }
  ERR = grError;
  return -1;
#endif
}
