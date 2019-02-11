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

#include "text.h"

void __gr_text_bit(GrFont *fnt, int *xx, int *yy, int XX, int YY,
					       int len, uchar *txt)
{
  if ( (Style.txo_font = fnt) == NULL) {
    ERR = grError;
    return;
  }
#ifndef GRX_VERSION
  Style.txo_xmag      =
  Style.txo_ymag      = ZERO2ONE( TXT.charsize);
#endif
  Style.txo_fgcolor.v = COL;
  Style.txo_bgcolor.v = GrNOCOLOR;
  Style.txo_direct    = (TXT.direction == HORIZ_DIR) ? GR_TEXT_RIGHT:GR_TEXT_UP;
  switch (TXT.horiz) {
    case LEFT_TEXT   : Style.txo_xalign = GR_ALIGN_LEFT;   break;
    case RIGHT_TEXT  : Style.txo_xalign = GR_ALIGN_RIGHT;  break;
    case CENTER_TEXT :
    default          : Style.txo_xalign = GR_ALIGN_CENTER; break;
  }
  switch (TXT.vert) {
    case BOTTOM_TEXT : Style.txo_yalign = GR_ALIGN_BOTTOM; break;
    case TOP_TEXT    : Style.txo_yalign = GR_ALIGN_TOP;    break;
    case CENTER_TEXT :
    default          : Style.txo_yalign = GR_ALIGN_CENTER; break;
  }
  GrDrawString( txt, len, XX, YY, &Style);
  if (TXT.direction == HORIZ_DIR)
    *xx += GrStringWidth(txt, len, &Style);
  else {
    Style.txo_direct = GR_TEXT_RIGHT;
    *yy -= GrStringWidth(txt,len, &Style);
  }
}

