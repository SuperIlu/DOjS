/*
 * BCC2GRX  -  Interfacing Borland based graphics programs to LIBGRX
 * Copyright (C) 1993-97 by Hartmut Schirmer
 *
 * This library is copyrighted (see above). It might be used and
 * distributed freely as long as all copyright notices are left
 * intact.
 *
 * You may not distribute any changed versions of BCC2GRX without
 * written permission by Hartmut Schirmer.
 *
 * You are permitted to distribute an application linked with BCC2GRX
 * in binary only, provided that the documentation of the program:
 *
 *    a)   informs the user that BCC2GRX is used in the program, AND
 *
 *    b)   provides the user with the necessary information about
 *         how to obtain BCC2GRX. (i.e. ftp site, etc..)
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Contact :                Hartmut Schirmer
 *                          Feldstrasse 118
 *                  D-24105 Kiel
 *                          Germany
 *
 * e-mail : hsc@techfak.uni-kiel.de
 */

#include "bccgrx00.h"

int __gr_getmodemaxcolor(int mode) {
  int  w, h;
  long c;
  _DO_INIT_CHECK_RV(-1);
  if (!__gr_getmode_whc(mode, &w, &h, &c)) {
    ERR = grInvalidMode;
    return -1;
  }
  return (int)(c-1);
}
