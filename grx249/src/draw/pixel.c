/**
 ** PIXEL.C ---- pixel read routine
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
#include "clipping.h"

GrColor GrPixel(int x,int y)
{
	GrColor retval;
	cxclip_dot_(CURC,x,y,return(GrNOCOLOR));
	mouse_block(CURC,x,y,x,y);
	retval = (*FDRV->readpixel)(
	    &CURC->gc_frame,
	    x + CURC->gc_xoffset,
	    y + CURC->gc_yoffset
	);
	mouse_unblock();
	return(retval);
}
