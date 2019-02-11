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

static int GRX_textheight(int len, char *txt) {
  Style.txo_font = (GrFont *)Fonts[TXT.font];
  Style.txo_direct = GR_TEXT_RIGHT;
  return GrStringHeight(txt, len, &Style);
}

/* ----------------------------------------------------------------- */

int textheight(const char *textstring)
{
  _DO_INIT_CHECK_RV(0);
  __gr_text_init();
  if (TXT.font == DEFAULT_FONT)
    return 8*ZERO2ONE(TXT.charsize);
  if (TXT.font >= FirstGrxFont && TXT.font <= LastGrxFont)
    return GRX_textheight(strlen(textstring), (char *) textstring);
  return __gr_text_height * __gr_text_multy / __gr_text_divy;
}

/* ----------------------------------------------------------------- */

int __gr_text_Height(int len, const char *txt) {
  _DO_INIT_CHECK_RV(0);
  __gr_text_init();
  if (TXT.font == DEFAULT_FONT)
    return 8*ZERO2ONE(TXT.charsize);
  if (TXT.font >= FirstGrxFont && TXT.font <= LastGrxFont)
    return GRX_textheight(len, (char *) txt);
  return __gr_text_height * __gr_text_multy / __gr_text_divy;
}
