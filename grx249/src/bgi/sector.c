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

#ifdef GRX_VERSION
#define   FilledEllipseArc(x,y,xr,yr,st,en,fi) \
	GrFilledEllipseArc(x,y,xr,yr,st,en,GR_ARC_STYLE_CLOSE2,fi)
#define   PatternFilledEllipseArc(x,y,xr,yr,st,en,fp) \
	GrPatternFilledEllipseArc(x,y,xr,yr,st,en,GR_ARC_STYLE_CLOSE2,fp)
#define   EllipseArc(x,y,xr,yr,st,en,co) \
	GrEllipseArc(x,y,xr,yr,st,en,GR_ARC_STYLE_CLOSE2,co)
#else
#define FilledEllipseArc GrFilledEllipseArc
#define PatternFilledEllipseArc GrPatternFilledEllipseArc
#define EllipseArc GrEllipseArc
#endif

void __gr_sector(int x,int y,int stangle,int endangle,int xradius,int yradius)
{
  _DO_INIT_CHECK;
  x += VL;
  y += VT + PY;
  stangle *= 10;
  endangle *= 10;
  xradius = XR(xradius);
  yradius = YR(yradius);
  switch (FPATT) {
    case SOLID_FILL :
      FilledEllipseArc(x,y,xradius,yradius,stangle,endangle, FILL);
      if (COL != FILL)
	goto frame;
      break;
    case EMPTY_FILL :
      FilledEllipseArc(x,y,xradius,yradius,stangle,endangle, COLBG);
      if (COL != COLBG)
	goto frame;
      break;
    default :
      FILLP.gp_bmp_fgcolor = FILL;
      FILLP.gp_bmp_bgcolor = COLBG;
      PatternFilledEllipseArc(x,y,xradius,yradius,stangle,endangle, &FILLP);
frame:
      EllipseArc( x, y, xradius, yradius, stangle, endangle, COL);
#ifndef GRX_VERSION
      {
#ifdef GR_DRV_VER_GRD
	 /* GRX 1.03 */
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
	GrLine( xs, ys, xc, yc, COL);
	GrLine( xc, yc, xe, ye, COL);
      }
#endif
      break;
  }
}
