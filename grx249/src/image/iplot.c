/**
 ** iplot.c ---- Source Image Utility
 **
 ** by Michal Stencl Copyright (c) 1998
 ** <e-mail>    - [stenclpmd@ba.telecom.sk]
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
 ** modifications by Hartmut Schirmer (c) 1998
 **
 **/

#include "libgrx.h"
#include "clipping.h"
#include "image/image.h"

void GrImagePlotAlign(int xo,int yo,int x,int y,GrImage *p)
{
   int xp, yp;
   GrColor col;

   xo = min(xo, x);
   yo = min(yo, y);
   clip_dot(CURC,x,y);
   xp = (x - xo) % p->pxp_width;
   yp = (y - yo) % p->pxp_height;
   mouse_block(CURC,x,y,x,y);
   col = (*p->pxp_source.gf_driver->readpixel)(&p->pxp_source,xp,yp);
   (*CURC->gc_driver->drawpixel)(x + CURC->gc_xoffset, y + CURC->gc_yoffset, col);
   mouse_unblock();
}
