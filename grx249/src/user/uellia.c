/**
 ** uellia.c
 **
 ** Copyright (C), Michael Goffioul
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
#include "usercord.h"

void GrUsrEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,GrColor c)
{
	U2SX(xc,CURC);
	U2SY(yc,CURC);
	SCALE(xa,xa,CURC->gc_xmax,CURC->gc_usrwidth);
	SCALE(ya,ya,CURC->gc_ymax,CURC->gc_usrheight);
	GrEllipseArc(xc,yc,xa,ya,start,end,style,c);
}

