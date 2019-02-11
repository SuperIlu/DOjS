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

/* ----------------------------------------------------------------- */
#define xoff(x) ((int)(signed char)(((_ushort) (x)) << 1) >> 1)
#define yoff(y) (-((int)(signed char)(((_ushort) (y)) >> 7) >> 1))

/* ----------------------------------------------------------------- */
void __gr_text_vec(int *xx, int *yy, int XX, int YY, int len, uchar *txt)
{
  if (__gr_TextLineStyle)
    LNE.lno_color= COL|WR;
  if (TXT.direction == HORIZ_DIR) {
    int     _XX, x, y, nx, ny, w;
    _ushort *dc;

    switch (TXT.horiz) {
      case CENTER_TEXT : XX -= __gr_text_Width(len, txt) / 2; break;
      case RIGHT_TEXT  : XX -= __gr_text_Width(len, txt);     break;
      default          : break;
    }
    switch (TXT.vert) {
      case CENTER_TEXT : YY += __gr_text_Height(len, txt) / 2; break;
      case TOP_TEXT    : YY += __gr_text_Height(len, txt);     break;
      default          : break;
    }
    _XX = XX;
    x = y = 0;
    while (len-- > 0) {
      w  = fntptr[*txt].width;
      dc = fntptr[*(txt++)].cmd;
      while (dc != NULL) {
	switch ( *dc & 0x8080) {
	  case 0x0000 : dc = NULL;
			XX += w * __gr_text_multx / __gr_text_divx;
			break;
	  case 0x8000 : /* DO_SCAN op, any font using this ? */
			++dc;
			break;
	  case 0x0080 : x = xoff(*dc) * __gr_text_multx / __gr_text_divx;
			y = yoff(*dc) * __gr_text_multy / __gr_text_divy;
			++dc;
			break;
	  case 0x8080 : nx = xoff(*dc) * __gr_text_multx / __gr_text_divx;
			ny = yoff(*dc) * __gr_text_multy / __gr_text_divy;
			if (__gr_TextLineStyle)
			  GrCustomLine( XX+x, YY+y, XX+nx, YY+ny, &LNE);
			else
			  GrLine( XX+x, YY+y, XX+nx, YY+ny, COL);
			x = nx;
			y = ny;
			++dc;
			break;
	}
      }
    }
    *xx += XX-_XX;
  } else {
    int     _YY, x, y, nx, ny, w;
    _ushort *dc;

    switch (TXT.horiz) {
      case LEFT_TEXT   : XX += __gr_text_Height(len, txt);     break;
      case CENTER_TEXT : XX += __gr_text_Height(len, txt) / 2; break;
      default          : break;
    }
    switch (TXT.vert) {
      case CENTER_TEXT : YY += __gr_text_Width(len,txt) / 2; break;
      case TOP_TEXT    : YY += __gr_text_Width(len,txt);     break;
      default          : break;
    }
    _YY = YY;
    x = y = 0;
    while (len-- > 0) {
      w  = fntptr[*txt].width;
      dc = fntptr[*(txt++)].cmd;
      while (dc != NULL) {
	switch ( *dc & 0x8080) {
	  case 0x0000 : dc = NULL;
			YY -= w * __gr_text_multx / __gr_text_divx;
			break;
	  case 0x8000 : /* DO_SCAN op, any font using this ? */
			++dc;
			break;
	  case 0x0080 : y = -xoff(*dc) * __gr_text_multx / __gr_text_divx;
			x =  yoff(*dc) * __gr_text_multy / __gr_text_divy;
			++dc;
			break;
	  case 0x8080 : ny = -xoff(*dc) * __gr_text_multx / __gr_text_divx;
			nx =  yoff(*dc) * __gr_text_multy / __gr_text_divy;
			if (__gr_TextLineStyle)
			  GrCustomLine( XX+x, YY+y, XX+nx, YY+ny, &LNE);
			else
			  GrLine( XX+x, YY+y, XX+nx, YY+ny, COL);
			x = nx;
			y = ny;
			++dc;
			break;
	}
      }
    }
    *yy += YY-_YY;
  }
}

