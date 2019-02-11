/**
 ** viewport.c ---- set display start address for virtual screen
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
#include "arith.h"

int GrSetViewport(int x,int y)
{
	int res[2];
	if(!GrScreenIsVirtual())	      return(FALSE);
	if(!DRVINFO->actmode.extinfo->scroll) return(FALSE);
	x = imax(0,imin((GrVirtualX() - GrScreenX()),x));
	y = imax(0,imin((GrVirtualY() - GrScreenY()),y));
	if((x == GrViewportX()) && (y == GrViewportY())) return(TRUE);
	if((*DRVINFO->actmode.extinfo->scroll)(&DRVINFO->actmode,x,y,res)) {
	    DRVINFO->vposx = res[0];
	    DRVINFO->vposy = res[1];
	    return(TRUE);
	}
	return(FALSE);
}




