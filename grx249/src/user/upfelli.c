/**
 ** upfelli.c
 **
 ** Copyright (C) 1997, Michael Goffioul
 ** [goffioul@emic.ucl.ac.be]
 **
 **/

#include "libgrx.h"
#include "usercord.h"

void GrUsrPatternFilledEllipse(int xc,int yc,int xa,int ya,GrPattern *p)
{
	U2SX(xc,CURC);
	U2SY(yc,CURC);
	SCALE(xa,xa,CURC->gc_xmax,CURC->gc_usrwidth);
	SCALE(ya,ya,CURC->gc_ymax,CURC->gc_usrheight);
	GrPatternFilledEllipse(xc,yc,xa,ya,p);
}
