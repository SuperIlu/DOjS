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

int __gr_text_ChrFontInfo(void *Font, CharInfo *fntptr, int *height) {
  int i, LstChar;
  char *cp;
  _ushort *Offsets;
  uchar *Widths;
  char *Data;
  FontFileHeader *ffh;
  FontHeaderTyp *fht;

  cp = (char *)Font;
  i = 256;
  while (*cp != '\x1a' ) { /* \x1a = ^Z */
    ++cp;
    --i;
    if (i == 0)   /* Error, no ^Z at end of copyright */
      return FALSE;
  }
  ++cp;
  ffh = (FontFileHeader *)cp;
  fht = (FontHeaderTyp *)((char *)Font + ffh->header_size);
  if (fht->sig != '+')
    return FALSE; /* Magic failed */
  if (fht->scan_flag) {
    /* font may have DO_SCAN op, anything we should do ? */
  }
  Offsets = (_ushort *)((char *)fht + sizeof(FontHeaderTyp));
  Widths  = (uchar *)Offsets + 2 * (int)fht->nchrs;
  Data    = (char *)Font + fht->cdefs + ffh->header_size;
  LstChar = fht->firstch + fht->nchrs - 1;

  *height = (int)fht->org_to_cap - (int)fht->org_to_dec;
  for (i=fht->firstch; i <= LstChar; ++i) {
    fntptr[i].width = Widths[i - fht->firstch];
    fntptr[i].cmd   = (_ushort *)(Data + Offsets[i - fht->firstch]);
  }
  return TRUE;
}

