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
#include "allocate.h"

#include <ctype.h>
static void StrLwrCpy(char *d, const char *s, int n)
{
  while (n-- > 0) {
    *(d++) = tolower(*s);
    ++s;
  }
  *d = '\0';
}

static int _installgrxfont(int start, int stop, const char *name)
{
  __gr_text_init();
  while (start < stop && Fonts[start] != NULL) ++start;
  if (start >= stop)
    return grNoFontMem;
#ifdef GRX_VERSION
  if ( name[0] == '@' && name[1] == ':')
    name += 2;
#endif
  Fonts[start] = (void *) GrLoadFont((char *) name);
  if (Fonts[start] == NULL)
    return grFontNotFound;
  return start;
}

static int _installuserfont(int len, const char *name)
{
  int res;
  char *loc_name = ALLOC(len+5);

  if ( !loc_name )
	  return grNoFontMem;

  StrLwrCpy(loc_name, name, len);
  if (strstr(loc_name, ".fnt") != NULL)
    res = _installgrxfont( FirstGrxFont, LastGrxFont, name);
  else
	res = __gr_text_installfont( FirstUserFont, LastUserFont, loc_name);

  FREE(loc_name);
  return res;
}

int installuserfont(const char *name)
{
  return _installuserfont(strlen(name), name);
}
