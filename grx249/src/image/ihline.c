/**
 ** ihline.c ---- Source Image Utility
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

void GrImageHLineAlign(int xo,int yo,int x,int y,int width,GrImage *p)
{
  int x2, widthimg, yimg, ximg, xdest, ydest, cpysize;
  GrColor optype;
  void (*bltfun)(GrFrame*,int,int,GrFrame*,int,int,int,int,GrColor);
  x2 = x+width;
  xo = min(xo, min(x,x2));
  yo = min(yo, y);
  clip_hline(CURC,x,x2,y);
  width = x2 - x;
  mouse_block(CURC,x,y,x2,y);
  widthimg = p->pxp_width;
  yimg = (y - yo) % p->pxp_height;
  ximg = (x - xo) % widthimg;
  xdest = x + CURC->gc_xoffset;
  ydest = y + CURC->gc_yoffset;
  cpysize = widthimg - ximg;
  optype = p->pxp_oper;
  if (CURC->gc_onscreen) bltfun = CURC->gc_driver->bltr2v;
  else                   bltfun = CURC->gc_driver->bitblt;
  while ( width > 0 ) {
    if ( cpysize > width ) cpysize = width;
	 (*bltfun)(&CURC->gc_frame,xdest,ydest,
		   &p->pxp_source,ximg,yimg,cpysize,1,
		   optype);
    width -= cpysize;
    ximg = 0;
    xdest += cpysize;
    cpysize = widthimg;
  };
  mouse_unblock();
}
