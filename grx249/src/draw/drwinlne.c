/**
 ** drvinlne.c ---- the drawing inline functions
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu].
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

void (GrPlotNC)(int x,int y,GrColor c)
{
	GrPlotNC(x,y,c);
}

GrColor (GrPixelNC)(int x,int y)
{
	return(GrPixelNC(x,y));
}

GrColor (GrPixelCNC)(GrContext *c,int x,int y)
{
	return(GrPixelCNC(c,x,y));
}


