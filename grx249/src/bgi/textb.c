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

void setusercharsize(int multx, int divx, int multy, int divy)
{
  _DO_INIT_CHECK;
  __gr_text_init();
  if (divx <= 0 || divy <= 0 || multx < 0 || multy < 0 || BITMAP(TXT.font)) {
    ERR = grError;
    return;
  }
  TXT.charsize = USER_CHAR_SIZE;
  __gr_text_multx = __gr_text_usr_multx = multx;
  __gr_text_divx  = __gr_text_usr_divx  = divx;
  __gr_text_multy = __gr_text_usr_multy = multy;
  __gr_text_divy  = __gr_text_usr_divy  = divy;
}

