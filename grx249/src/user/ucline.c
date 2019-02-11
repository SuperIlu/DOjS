/**
 ** ucline.c
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


void GrUsrCustomLine(int x1,int y1,int x2,int y2,const GrLineOption *lo)
{
	U2SX(x1,CURC);
	U2SX(x2,CURC);
	U2SY(y1,CURC);
	U2SY(y2,CURC);
	GrCustomLine(x1,y1,x2,y2,lo);
}
