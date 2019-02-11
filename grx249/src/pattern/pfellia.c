/**
 ** pfellia.c
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
 **
 **  Copyright (C) 1992, Csaba Biegl
 **    820 Stirrup Dr, Nashville, TN, 37221
 **    csaba@vuse.vanderbilt.edu
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

#include "libgrx.h"
#include "allocate.h"
#include "shapes.h"

void GrPatternFilledEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,GrPattern *p)
{
   int (*points)[2];
   setup_ALLOC();
   points = ALLOC(sizeof(int) * 2 * (GR_MAX_ELLIPSE_POINTS + 1));
   if (points != NULL)
   {
      int numpts = GrGenerateEllipseArc(xc,yc,xa,ya,start,end,points);
      GrFillArg fa;

      if (style == GR_ARC_STYLE_CLOSE2) {
	      points[numpts][0] = xc;
	      points[numpts][1] = yc;
	      numpts++;
      }
      fa.p = p;
      if(numpts < 0) _GrScanConvexPoly((-numpts),points,&_GrPatternFiller,fa);
      else           _GrScanPolygon(     numpts, points,&_GrPatternFiller,fa);
      FREE(points);
   }
   reset_ALLOC();
}
