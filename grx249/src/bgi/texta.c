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

static int _mult[11] = { 1, 3, 2, 3, 1, 4, 5, 2, 5, 3, 4};
static int _div[11]  = { 1, 5, 3, 4, 1, 3, 3, 1, 2, 1, 1};
static CharInfo *chi[LastUserFont+1] = { NULL };
static int      Heights[LastUserFont+1];

static int SetFont(int fnt)
{
  if (Fonts[fnt] == NULL)
    return FALSE;

  if (chi[fnt] == NULL) {
    chi[fnt] = calloc(256,sizeof(CharInfo));
    if (chi[fnt] == NULL)
       return FALSE;
    fntptr = chi[fnt];

    if (!__gr_text_ChrFontInfo(Fonts[fnt], fntptr, &Heights[fnt]))
      return FALSE;
  }
  fntptr = chi[fnt];
  __gr_text_height = Heights[fnt];
  return TRUE;
}

void settextstyle(int font, int direction, int charsize)
{
  _DO_INIT_CHECK;
  __gr_text_init();
  if (font < DEFAULT_FONT || font >= NrFonts ||
      (font > BOLD_FONT && Fonts[font] == NULL)) {
    ERR = grInvalidFontNum;
    return;
  }
  if (BITMAP(font) && charsize < 1) charsize = 1;
  if (charsize < 0)  charsize =  4; /* 100% */
  if (charsize > 10) charsize = 10;

  if (!BITMAP(font)) {
    if (Fonts[font] == NULL) {
      char fn[256], *cp;

      strcpy(fn, (*__gr_BGICHR != '\0' ? __gr_BGICHR : ".\\"));
      cp = fn;
      while ( *cp != '\0') ++cp;
      if ( *(--cp) != '\\' && *cp != '/') {
	*(++cp) = '\\';
	*(cp+1) = '\0';
      }
      strcat( fn, StdFonts[font]);
      __gr_text_installfont( font, font, fn);
    }
    if (!SetFont( font)) {
      ERR = grFontNotFound;
      font = DEFAULT_FONT;
      charsize = 1;
    }
  }
  TXT.font         = font;
  TXT.direction    = direction;
  TXT.charsize     = charsize;
  if (charsize == USER_CHAR_SIZE) {
    __gr_text_multx = __gr_text_usr_multx;
    __gr_text_divx  = __gr_text_usr_divx;
    __gr_text_multy = __gr_text_usr_multy;
    __gr_text_divy  = __gr_text_usr_divy;
  } else {
    __gr_text_multx = __gr_text_multy  = _mult[charsize];
    __gr_text_divx  = __gr_text_divy   = _div[charsize];
  }
}


