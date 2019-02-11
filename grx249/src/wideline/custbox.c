/**
 ** custbox.c ---- wide and/or dashed box outline
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
#include "shapes.h"

void GrCustomBox(int x1,int y1,int x2,int y2,const GrLineOption *o)
{
	GrFillArg fval;
	int pt[4][2];
	pt[0][0] = x1; pt[0][1] = y1;
	pt[1][0] = x2; pt[1][1] = y1;
	pt[2][0] = x2; pt[2][1] = y2;
	pt[3][0] = x1; pt[3][1] = y2;
	fval.color = o->lno_color;
	_GrDrawCustomPolygon(4,pt,o,&_GrSolidFiller,fval,TRUE,FALSE);
}

