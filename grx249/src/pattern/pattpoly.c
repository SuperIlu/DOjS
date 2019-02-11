/**
 ** pattpoly.c
 **
 ** Copyright (C) 1997, Michael Goffioul
 ** [e-mail : goffioul@emic.ucl.ac.be]
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
#include "shapes.h"

void GrPatternedPolyLine(int numpts,int points[][2],GrLinePattern *lp)
{
	GrFillArg fval;

	fval.p = lp->lnp_pattern;
	_GrDrawCustomPolygon(numpts,points,lp->lnp_option,
			     &_GrPatternFiller,fval,FALSE,FALSE);
}

void GrPatternedPolygon(int numpts,int points[][2],GrLinePattern *lp)
{
	GrFillArg fval;

	fval.p = lp->lnp_pattern;
	_GrDrawCustomPolygon(numpts,points,lp->lnp_option,
			     &_GrPatternFiller,fval,TRUE,FALSE);
}

void GrPatternedBox(int x1,int y1,int x2,int y2,GrLinePattern *lp)
{
	GrFillArg fval;
	int points[4][2];

	points[0][0] = x1; points[0][1] = y1;
	points[1][0] = x1; points[1][1] = y2;
	points[2][0] = x2; points[2][1] = y2;
	points[3][0] = x2; points[3][1] = y1;
	fval.p = lp->lnp_pattern;
	_GrDrawCustomPolygon(4,points,lp->lnp_option,
			     &_GrPatternFiller,fval,TRUE,FALSE);
}
