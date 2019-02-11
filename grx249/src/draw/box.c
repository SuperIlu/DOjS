/**
 ** box.c ---- draw a rectangle
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

void GrBox(int x1,int y1,int x2,int y2,GrColor c)
{
	int ox1,oy1,ox2,oy2;
	isort(x1,x2); ox1 = x1; ox2 = x2;
	isort(y1,y2); oy1 = y1; oy2 = y2;
	clip_ordbox(CURC,x1,y1,x2,y2);
	mouse_block(CURC,x1,y1,x2,y2);
	if(!(oy1 -= y1)) (*FDRV->drawhline)(
	    x1 + CURC->gc_xoffset,
	    y1 + CURC->gc_yoffset,
	    x2 - x1 + 1,
	    c
	);
	if(!(oy2 -= y2) && ((y1 != y2) || oy1)) (*FDRV->drawhline)(
	    x1 + CURC->gc_xoffset,
	    y2 + CURC->gc_yoffset,
	    x2 - x1 + 1,
	    c
	);
	if((y2 = y2 - (oy1 ? y1 : ++y1) + (oy2 ? 1 : 0)) > 0) {
	    if(!(ox1 -= x1)) (*FDRV->drawvline)(
		x1 + CURC->gc_xoffset,
		y1 + CURC->gc_yoffset,
		y2,
		c
	    );
	    if((ox2 == x2) && ((x1 != x2) || ox1)) (*FDRV->drawvline)(
		x2 + CURC->gc_xoffset,
		y1 + CURC->gc_yoffset,
		y2,
		c
	    );
	}
	mouse_unblock();
}

