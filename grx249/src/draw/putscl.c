/**
 ** putscl.c ---- put scanline pixels
 **
 ** Copyright (c) 1998 Hartmut Schirmer
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

void GrPutScanline(int x1,int x2,int yy,const GrColor *c, GrColor op)
/* Input   x1 : first x coordinate to be set                        */
/*         x2 : last  x coordinate to be set                        */
/*         yy : y coordinate                                        */
/*         c  : c[0..(|x2-x1|] hold the pixel data                  */
/*         op : Operation (GrWRITE/GrXOR/GrOR/GrAND/GrIMAGE)        */
/*                                                                  */
/* Note    c[..] data must fit GrCVALUEMASK otherwise the results   */
/*         are implementation dependend.                            */
/*         => You can't supply operation code with the pixel data!  */
{
	int xs;
	isort(x1,x2);
	xs = x1;
	clip_hline(CURC,x1,x2,yy);
	mouse_block(CURC,x1,yy,x2,yy);
	(*FDRV->putscanline)(
	    x1 + CURC->gc_xoffset,
	    yy + CURC->gc_yoffset,
	    x2 - x1 + 1,
	    &c[x1-xs],  /* adjust pixel pointer when clipped */
	    op
	);
	mouse_unblock();
}
