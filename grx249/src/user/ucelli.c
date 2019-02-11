/**
 ** ucelli.c
 **
 ** Copyright (C) 1997, Michael Goffioul
 ** [goffioul@emic.ucl.ac.be]
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

void GrUsrCustomEllipse(int xc,int yc,int xa,int ya,const GrLineOption *lo)
{
	U2SX(xc,CURC);
	U2SY(yc,CURC);
	SCALE(xa,xa,CURC->gc_xmax,CURC->gc_usrwidth);
	SCALE(ya,ya,CURC->gc_ymax,CURC->gc_usrheight);
	GrCustomEllipse(xc,yc,xa,ya,lo);
}

