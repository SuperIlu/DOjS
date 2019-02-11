/**
 ** celli.c ---- draw dashed and/or wide ellipses
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
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

void GrCustomEllipse(int xc,int yc,int xa,int ya,const GrLineOption *o)
{
    int (*pnts)[2];
    setup_ALLOC();
    pnts = ALLOC(sizeof(int) * 2 * GR_MAX_ELLIPSE_POINTS);
    if (pnts != NULL)
    {
	GrFillArg fval;
	int npts  = GrGenerateEllipse(xc,yc,xa,ya,pnts);
	fval.color = o->lno_color;
	_GrDrawCustomPolygon(npts,pnts,o,&_GrSolidFiller,fval,TRUE,TRUE);
	FREE(pnts);
    }
    reset_ALLOC();
}
