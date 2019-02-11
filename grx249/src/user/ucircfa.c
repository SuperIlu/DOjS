/**
 ** ucircfa.c
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
 ** Note : compiling the library with -DUSR_KEEP_SHAPE makes a circle
 **        looks like a circle on the screen
 **
 **/

#include "libgrx.h"
#include "usercord.h"

void GrUsrFilledCircleArc(int xc,int yc,int r,int start,int end,int style,GrColor c)
{
#ifdef USR_KEEP_SHAPE
	U2SX(xc,CURC);
	U2SY(yc,CURC);
	SCALE(r,r,CURC->gc_xmax,CURC->gc_usrwidth);
	GrFilledCircleArc(xc,yc,r,start,end,style,c);
#else
	GrUsrFilledEllipseArc(xc,yc,r,r,start,end,style,c);
#endif /* USR_KEEP_SHAPE */
}

