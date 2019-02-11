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
#include <stdio.h>

char *getmodename(int mode_number)
{
  static char result[50];
  char cols[20];
  int xw, yw;
  long nc;

/*  _DO_INIT_CHECK_RV(NULL); */
  __gr_set_up_modes();
  if (mode_number < 0 || mode_number > MM) {
    ERR = grInvalidMode;
    return NULL;
  }
  switch (mode_number) {
    case GRX_DEFAULT_GRAPHICS               : return "default graphics mode";
    case GRX_BIGGEST_NONINTERLACED_GRAPHICS : return "biggest non interlaced graphics mode";
    case GRX_BIGGEST_GRAPHICS               : return "biggest graphics mode";
    case GRX_BGI_EMULATION                  : return "BGI emulation mode";
  }
  if (!__gr_getmode_whc(mode_number, &xw, &yw, &nc))
    return NULL;
  switch (nc) {
    case 1L<<15 : strcpy(cols,"32K"); break;
    case 1L<<16 : strcpy(cols,"64K"); break;
    case 1L<<24 : strcpy(cols,"16M"); break;
    default    : sprintf(cols, "%ld", nc);
		 break;
  }
  sprintf(result, "%d x %d x %s", xw, yw, cols);
  return result;
}

