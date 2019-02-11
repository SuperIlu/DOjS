/**
 ** uframbox.c
 **
 ** Copyright (C) 1992, Csaba Biegl
 **   820 Stirrup Dr, Nashville, TN, 37221
 **   csaba@vuse.vanderbilt.edu
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
#include "arith.h"
#include "usercord.h"

void GrUsrFramedBox(int x1,int y1,int x2,int y2,int wdt,GrFBoxColors *c)
{
	int w1,w2;

	U2SX(x1,CURC);
	U2SY(y1,CURC);
	U2SX(x2,CURC);
	U2SY(y2,CURC);
	SCALE(w1,wdt,CURC->gc_xmax,CURC->gc_usrwidth);
	SCALE(w2,wdt,CURC->gc_ymax,CURC->gc_usrheight);
	wdt = (iabs((int)w1) + iabs((int)w2)) >> 1;
	if(wdt == 0)
	    GrFilledBox(x1,y1,x2,y2,c->fbx_intcolor);
	else GrFramedBox(x1,y1,x2,y2,wdt,c);
}
