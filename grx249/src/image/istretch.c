/**
 ** istretch.c ---- Source Image Utility
 **
 ** Copyright (c) 1998 by Michal Stencl and Hartmut Schirmer
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
#include "grdriver.h"
#include "clipping.h"
#include "image/image.h"

GrImage *GrImageStretch(GrImage *p,int nwidth,int nheight)
{
  GrContext  ctx;
  GrImage   *img;

  GRX_ENTER();
  img = _GrImageAllocate(&ctx,nwidth,nheight);
  if ( img )
    _GrFrDrvGenericStretchBlt(&ctx.gc_frame,0,0,nwidth,nheight,
			      &p->pxp_source,0,0,p->pxp_width,p->pxp_height,
			      p->pxp_oper);
  GRX_RETURN(img);
}
