/**
 ** flodspil.c ---- Floodspill is a color replacer
 **
 ** Copyright (c) 2007 Richard
 ** [e-mail: richard at dogcreek.ca].
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

#include <stdlib.h>
#include "libgrx.h"


void GrFloodSpill(int x1, int y1, int x2, int y2,
                  GrColor old_c, GrColor new_c)
{
  int y;
  int x;

  GrColor *scanline;

  for(y = y1; y <= y2; ++y)
  {
    if ((scanline = (GrColor *)GrGetScanline(x1, x2, y)) != NULL)
    {
      for(x = x1; x <= x2; ++x)
      {
        if (scanline[x] == old_c)
          scanline[x] = new_c;
      }
      GrPutScanline(x1, x2, y, scanline, GrWRITE);
    }
  }
}

void GrFloodSpill2(int x1, int y1, int x2, int y2,
                  GrColor old_c1, GrColor new_c1,
                  GrColor old_c2, GrColor new_c2)
{
  int y;
  int x;

  GrColor *scanline;

  for(y = y1; y <= y2; ++y)
  {
    if ((scanline = (GrColor *)GrGetScanline(x1, x2, y)) != NULL)
    {
      for(x = x1; x <= x2; ++x)
      {
        if (scanline[x] == old_c1)
          scanline[x] = new_c1;
        else if(scanline[x] == old_c2)
          scanline[x] = new_c2;
      }
      GrPutScanline(x1, x2, y, scanline, GrWRITE);
    }
  }
}

void GrFloodSpillC(GrContext *ctx, int x1, int y1, int x2, int y2,
                   GrColor old_c, GrColor new_c)
{
  int y;
  int x;
  GrContext *ctx_save = GrCurrentContext();
  GrColor *scanline;

  GrSetContext(ctx);

  for(y = y1; y <= y2; ++y)
  {
    if ((scanline = (GrColor *)GrGetScanline(x1, x2, y)) != NULL)
    {
      for(x = x1; x <= x2; ++x)
      {
        if (scanline[x] == old_c)
          scanline[x] = new_c;
      }
      GrPutScanline(x1, x2, y, scanline, GrWRITE);
    }
  }
  GrSetContext(ctx_save);
}

void GrFloodSpillC2(GrContext *ctx, int x1, int y1, int x2, int y2,
                  GrColor old_c1, GrColor new_c1,
                  GrColor old_c2, GrColor new_c2)
{
  int y;
  int x;
  GrContext *ctx_save = GrCurrentContext();
  GrColor *scanline;

  GrSetContext(ctx);

  for(y = y1; y <= y2; ++y)
  {
    if ((scanline = (GrColor *)GrGetScanline(x1, x2, y)) != NULL)
    {
      for(x = x1; x <= x2; ++x)
      {
        if (scanline[x] == old_c1)
          scanline[x] = new_c1;
        else if(scanline[x] == old_c2)
          scanline[x] = new_c2;
      }
      GrPutScanline(x1, x2, y, scanline, GrWRITE);
    }
  }
  GrSetContext(ctx_save);
}
