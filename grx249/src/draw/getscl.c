/**
 ** getscl.c ---- get scanline pixels
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

const GrColor *GrGetScanlineC(GrContext *ctx,int x1,int x2,int yy)
/* Input   ctx: source context, if NULL the current context is used */
/*         x1 : first x coordinate read                             */
/*         x2 : last  x coordinate read                             */
/*         yy : y coordinate                                        */
/* Output  NULL     : error / no data (clipping occured)            */
/*         else                                                     */
/*           p[0..w-1]: pixel values read                           */
/*           p[w]     : GrNOCOLOR end marker                        */
/*                      (w = |x2-y1|+1)                             */
/*           Output data is valid until next GRX call !             */
{
	GrColor *res = NULL;
	if (ctx == NULL) ctx = CURC;
	/* don't accept any clipping .... */
	clip_hline_(ctx,x1,x2,yy,goto done,goto done);
	mouse_block(ctx,x1,yy,x2,yy);
	res = (*ctx->gc_driver->getindexedscanline)(
	    &ctx->gc_frame,
	    x1 + ctx->gc_xoffset,
	    yy + ctx->gc_yoffset,
	    x2 - x1 + 1,
	    NULL
	);
	mouse_unblock();
done:   return res;
}

const GrColor *(GrGetScanline)(int x1,int x2,int yy) {
  return GrGetScanlineC(NULL, x1,x2,yy);
}
