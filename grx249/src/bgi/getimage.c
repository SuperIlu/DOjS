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
void getimage(int left, int top, int right, int bottom, void *bitmap)
{
  GrContext *gc;
  int w, h;
#ifdef GRX_VERSION
  char *memory[4];
  int i, np, ps;
#else
  char *memory;
#endif

  _DO_INIT_CHECK;
  GrSetContext(NULL);
  GrResetClipBox();
  gc = bitmap;
  w = __ABS(right-left)+1;
  h = __ABS(bottom-top)+1;
#ifdef GRX_VERSION
  np = GrNumPlanes();
  ps = GrPlaneSize(w,h);
  for (i=0; i < np; ++i)
    memory[i] = ((char *)bitmap) + (IMAGE_CONTEXT_SIZE + i*ps);
  while (i<4)
    memory[i++] = NULL;
#else
  memory = ((char *)bitmap) + IMAGE_CONTEXT_SIZE;
#endif
  GrCreateContext( w, h, memory, gc);
  GrBitBlt( gc, 0, 0, NULL, left+VL, top+VT+PY, right+VL, bottom+VT+PY, GrWRITE);
  __gr_Reset_ClipBox();
}
