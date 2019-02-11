/**
 ** solidfil.c ---- wrapper for solid filling
 **
 ** Copyright (c) 1998 Hartmut Schirmer
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
 */

#include "libgrx.h"
#include "shapes.h"

static void pixel(int x,int y,GrFillArg fval) {
  GRX_ENTER();
  FDRV->drawpixel(x,y,fval.color);
  GRX_LEAVE();
}

static void line(int x,int y,int dx,int dy,GrFillArg fval) {
  GRX_ENTER();
  FDRV->drawline(x,y,dx,dy,fval.color);
  GRX_LEAVE();
}

static void scan(int x,int y,int w,GrFillArg fval) {
  GRX_ENTER();
  FDRV->drawhline(x,y,w,fval.color);
  GRX_LEAVE();
}

GrFiller _GrSolidFiller = {
    pixel, line, scan
};

