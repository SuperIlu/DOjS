/**
 ** iinverse.c ---- Source Image Utility
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

GrImage *GrImageInverse(GrImage *p,int flag)
{
  GrContext  ctx, save;
  GrColor    col;
  GrImage   *img;
  int yy, xx, sidex, sidey, width, height, xs, ys = 0;
  width = p->pxp_width;
  height = p->pxp_height;
  img = _GrImageAllocate(&ctx,width,height);
  if ( !img ) return(NULL);
  save = *CURC;
  *CURC = ctx;
  sidex  = ( flag & GR_IMAGE_INVERSE_LR ) ? -1 : 1;
  sidey  = ( flag & GR_IMAGE_INVERSE_TD ) ? -1 : 1;
  yy     = ( flag & GR_IMAGE_INVERSE_TD ) ? height-1 : 0;
  do {
    xx = ( flag & GR_IMAGE_INVERSE_LR ) ? width-1 : 0;
    xs = 0;
    do {
      col = (*p->pxp_source.gf_driver->readpixel)(&p->pxp_source,xs,ys);
      (*CURC->gc_driver->drawpixel)(xx, yy, col);
      xx += sidex;
    } while(++xs < width);
    yy += sidey;
  } while(++ys < height);
  *CURC = save;
  img->pxp_ispixmap = 1;
  img->pxp_width  = width;
  img->pxp_height = height;
  img->pxp_oper   = 0;
  img->pxp_source = ctx.gc_frame;
  img->pxp_source.gf_memflags =  3;/* MY_CONTEXT & MY_MEMORY */
  return(img);
}
