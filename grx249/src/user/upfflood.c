/**
 ** upfflood.c ---- pattern fill an arbitrary area in user coordinates
 **
 ** Copyright (c) 1997 Hartmut Schirmer
 ** [e-mail: hsc@techfak.uni-kiel.de]
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

void GrUsrPatternFloodFill(int x, int y, GrColor border, GrPattern *p)
{
	U2SX(x,CURC);
	U2SY(y,CURC);
	GrPatternFloodFill(x,y,border,p);
}
