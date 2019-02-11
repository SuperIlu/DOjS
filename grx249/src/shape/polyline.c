/**
 ** POLYLINE.C ---- draw an open ended polygon
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu] See "doc/copying.cb" for details.
 **/

#include "libgrx.h"
#include "shapes.h"

void GrPolyLine(int n,int pt[][2],GrColor c)
{
	GrFillArg fval;
	fval.color = c;
	_GrDrawPolygon(n,pt,&_GrSolidFiller,fval,FALSE);
}
