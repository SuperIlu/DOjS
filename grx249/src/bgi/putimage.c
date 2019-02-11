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

/* ----------------------------------------------------------------- */
static void invert_image (GrContext *gc)
{
  int i, j, psize = GrPlaneSize(gc->gc_xmax + 1,gc->gc_ymax + 1);
  for (i = 0; i < 4; ++i)
    {
      char *p = gc->gc_baseaddr[i];
      if (p)
        for (j = 0; j < psize; ++j)
          p[j] ^= 0xff;
    }
}

void putimage(int left, int top, void *bitmap, int op)
{
  GrContext *gc;
  GrColor gr_op;

  _DO_INIT_CHECK;
  GrSetContext(NULL);
  GrResetClipBox();
  gc = bitmap;
  switch (op) {
    case XOR_PUT  : gr_op = GrXorModeColor(0);   break;
    case OR_PUT   : gr_op = GrOrModeColor(0);    break;
    case AND_PUT  : gr_op = GrAndModeColor(0);   break;
    default       : gr_op = GrWriteModeColor(0); break;
  }
  if (op == NOT_PUT)
    invert_image (gc);
  GrBitBlt( NULL, left+VL, top+VT+PY, gc, 0, 0, gc->gc_xmax, gc->gc_ymax, gr_op);
  if (op == NOT_PUT)
    invert_image (gc);
  __gr_Reset_ClipBox();
}

