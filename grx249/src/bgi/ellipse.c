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

static struct arccoordstype  ac;

#ifdef GRX_VERSION
#define GrGetLastArcCoords GrLastArcCoords
#endif

/* ----------------------------------------------------------------- */
void __gr_ellipse(int x, int y, int st, int en,
				int xradius, int yradius  )
{
  _DO_INIT_CHECK;
  st *= 10;
  en *= 10;
  GrEllipseArc(x+VL,y+VT+PY,XR(xradius),YR(yradius),st,en,
	       #ifdef GRX_VERSION
	       (en-st!=0&&(en-st)%3600==0) ? GR_ARC_STYLE_CLOSE1 : GR_ARC_STYLE_OPEN,
	       #endif
	       COL);
  {
#if defined(GR_DRV_VER_GRD) || defined(GRX_VERSION)
    /* GRX 1.03 or newer */
    int xs, ys, xc, yc, xe, ye;
    GrGetLastArcCoords(&xs,&ys,&xe,&ye,&xc,&yc);
#else
    /* GRX 1.02+ */
# define xs _grx_arc_xs
# define ys _grx_arc_ys
# define xc _grx_arc_xc
# define yc _grx_arc_yc
# define xe _grx_arc_xe
# define ye _grx_arc_ye
#endif
    ac.x      = xc - VL;
    ac.y      = yc - VT - PY;
    ac.xstart = xs - VL;
    ac.ystart = ys - VT - PY;
    ac.xend   = xe - VL;
    ac.yend   = ye - VT - PY;
  }
}

/* ----------------------------------------------------------------- */
void getarccoords(struct arccoordstype  *arccoords)
{
  _DO_INIT_CHECK;
  memcpy( arccoords, &ac, sizeof(ac));
}
