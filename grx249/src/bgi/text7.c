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

static void _outtextxy(int *xx, int *yy, int XX, int YY,
                                      int len, const uchar *textstring);

/* ----------------------------------------------------------------- */
void outtext(const char *textstring)
{
  _outtextxy(&X, &Y, X+VL, Y+VT+PY, strlen(textstring), (uchar *)textstring);
}

/* ----------------------------------------------------------------- */
void outtextxy(int x, int y, const char *textstring)
{
  _outtextxy( &x, &y, x+VL, y+VT+PY, strlen(textstring), (uchar *)textstring);
}

/* ----------------------------------------------------------------- */
static void _outtextxy(int *xx, int *yy, int XX, int YY,
				      int len, const uchar *textstring)
{
  _DO_INIT_CHECK;
  __gr_text_init();
#ifdef GRX_VERSION
  if (TXT.font==DEFAULT_FONT) {
    if (DefaultFonts[TXT.charsize] == NULL)
      DefaultFonts[TXT.charsize] =
	GrBuildConvertedFont(
	  DefaultFonts[1],
	  GR_FONTCVT_RESIZE,
	  8*ZERO2ONE(TXT.charsize),
	  8*ZERO2ONE(TXT.charsize),
	  0, 0);
    __gr_text_bit(DefaultFonts[TXT.charsize],xx,yy,XX,YY,len,(char *) textstring);
  } else
#endif
  if (BITMAP(TXT.font))
    __gr_text_bit((GrFont *)Fonts[TXT.font],xx,yy,XX,YY,len,(char *) textstring);
  else
    __gr_text_vec(xx,yy,XX,YY,len,(char *) textstring);
}
