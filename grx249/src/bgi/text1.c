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


/* -------------------------------------------------------------- */

int __gr_text_registerfont( int start, int stop, void *font)
{
  int    i;
  char  *Header;
  char  *name;

  __gr_text_init();
  Header = (char *)font + PreSkip;
  if ( memcmp( font, "PK\x8\x8",4)!=0 || *Header != '+')
    return grInvalidFont;

  name = (char *) font;
  while (*name != '\x1a') {
    if ( name-(char *)font > 128)
      return grInvalidFont;
    ++name;
  }
  name += 3;

  for (i=1; i <= BOLD_FONT; ++i)
    if (memcmp( name, StdFonts[i], 4) == 0)
      break;
  if (i > BOLD_FONT) {
    i = start;
    while ( i <= stop && Fonts[i] != NULL)
      ++i;
    if (i > stop)
      return grNoFontMem;
  }
  Fonts[i] = font;
  return i;
}

int __gr_text_installfont( int start, int stop, const char *name)
{
  FILE *ff;
  long  size;
  void *font;
  int   res;
  char *temp = alloca(strlen(name)+1+4);
  char *temp1;

#ifdef __linux__
#  define CHG_CHAR '\\'
#  define NEW_CHAR '/'
#else
#  define CHG_CHAR '/'
#  define NEW_CHAR '\\'
#endif

  if (temp != NULL) {
    int have_ext = FALSE;
    strcpy(temp, name);
    temp1 = temp;
    while (*temp != '\0') {
      if (*temp == CHG_CHAR) *temp = NEW_CHAR;
			else *temp = (tolower)(*temp);
      if (*temp == NEW_CHAR) have_ext = FALSE;
			else have_ext |= *temp == '.';
      ++temp;
    }
    if (!have_ext)
      strcat(temp1, ".chr");
    ff = fopen(temp1, "rb");
  }
  else
    ff = NULL;

  if (ff == NULL)
    return grFileNotFound;
  fseek( ff, 0, SEEK_END);
  size = ftell(ff);
  fseek( ff, 0, SEEK_SET);
  font = malloc( (size_t) size);
  if (font == NULL) {
    fclose( ff);
    return grNoFontMem;
  }
  fread( font, (size_t) size, 1, ff);
  fclose( ff);
  res = __gr_text_registerfont(start, stop, font);
  if (res < 0)
    free( font);
  return res;
}

