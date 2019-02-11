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

static unsigned short usr_pat = 0x0000;

#define user_len (2*16+1)
static unsigned char user[user_len];

void getlinesettings(struct linesettingstype  *lineinfo)
{
  _DO_INIT_CHECK;
  lineinfo->linestyle = __gr_lstyle;
  lineinfo->upattern  = usr_pat;
  lineinfo->thickness = LNE.lno_width;
}

/* ----------------------------------------------------------------- */
void setlinestyle(int linestyle, unsigned upattern, int thickness)
{
  int i, j;

  _DO_INIT_CHECK;
  switch (linestyle) {
    case SOLID_LINE  : LNE.lno_pattlen = 0;
		       LNE.lno_dashpat = NULL;
		       break;
    case DOTTED_LINE : LNE.lno_pattlen = 4;
		       LNE.lno_dashpat = "\0\2\2\0";
		       break;
    case CENTER_LINE : LNE.lno_pattlen = 6;
		       LNE.lno_dashpat = "\0\3\4\3\6\0";
		       break;
    case DASHED_LINE : LNE.lno_pattlen = 6;
		       LNE.lno_dashpat = "\0\3\5\3\5\0";
		       break;
    case USERBIT_LINE: usr_pat = upattern;
		       if (upattern == 0xFFFF) {
			 LNE.lno_pattlen = 0;
			 LNE.lno_dashpat = NULL;
			 break;
		       }
		       j = 0;
		       user[0] = 0;
		       for (i=0; i < 16; ++i) {
			 if ( (upattern & 1) == 0) {
			   if ( (j&1) == 0) {
			     ++j;
			     user[j] = 0;
			   }
			   ++user[j];
			 } else {
			   if ( (j&1) != 0) {
			     ++j;
			     user[j] = 0;
			   }
			   ++user[j];
			 }
			 upattern >>= 1;
		       }
#ifdef GRX_VERSION
		       if (j==1 && user[0]==0)
			   j = 0;
		       else
#endif
		       if ( (j&1) == 0)
			 user[++j] = 0;
		       LNE.lno_pattlen = j+1;
		       LNE.lno_dashpat = user;
		       break;
    default          : ERR = grError;
		       return;
  }
  __gr_lstyle     = linestyle;
  LNE.lno_width   = thickness;
}
