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

CharInfo *fntptr = NULL;
char *StdFonts[BOLD_FONT+1] = {
  "", "TRIP.CHR", "LITT.CHR", "SANS.CHR", "GOTH.CHR", "SCRI.CHR",
      "SIMP.CHR", "TSCR.CHR", "LCOM.CHR", "EURO.CHR", "BOLD.CHR" };

int  __gr_text_height;
int  __gr_text_multx, __gr_text_divx,
     __gr_text_multy, __gr_text_divy;
int  __gr_text_usr_multx=1, __gr_text_usr_divx=1,
     __gr_text_usr_multy=1, __gr_text_usr_divy=1;

void *Fonts[NrFonts];
struct textsettingstype __gr_text_setting;
GrTextOption Style;

#ifdef GRX_VERSION
GrFont *DefaultFonts[11];
#endif


void __gr_text_init(void)
{
  static int Init = FALSE;
  int i;

  if (Init) return;
  for (i=0; i < NrFonts; ++i)
    Fonts[i] = NULL;

#ifdef GRX_VERSION
  for (i=2; i < sizeof(DefaultFonts)/sizeof(GrFont *); ++i)
    DefaultFonts[i] = NULL;
  Fonts[DEFAULT_FONT] =
  DefaultFonts[0] =
  DefaultFonts[1] =
  #ifdef LOAD_8x8_FONT
    Fonts[DEFAULT_FONT] = (void *) GrLoadFont("pc8x8.fnt");
  #else
    &GrFont_PC8x8;
  #endif
#else
  Fonts[DEFAULT_FONT] = (void *) GrLoadFont("@:pc8x8.fnt");
#endif
  Style.txo_font    = (GrFont *)Fonts[DEFAULT_FONT];
  Style.txo_chrtype = GR_BYTE_TEXT;

  TXT.font      = DEFAULT_FONT;
  TXT.direction = HORIZ_DIR;
  TXT.charsize  = 1;
  TXT.horiz     = LEFT_TEXT;
  TXT.vert      = TOP_TEXT;
  __gr_text_usr_multx = __gr_text_usr_divx =
  __gr_text_usr_multy = __gr_text_usr_divy =
      __gr_text_multx =    __gr_text_divx  =
      __gr_text_multy =    __gr_text_divy  = 1;

  Init = TRUE;
}
