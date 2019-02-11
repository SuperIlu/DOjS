/**
 ** ialloc.c ---- Source Image Utility
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
 ** modifications by Hartmut Schirmer Copyright (c) 1998
 **
 **/

#include "libgrx.h"
#include "allocate.h"
#include "image/image.h"

int _GrImageTestSize(int wdt,int hgt)
{
  long total;
  GRX_ENTER();
  total = GrContextSize(wdt,hgt);
# ifdef _MAXMEMPLANESIZE
  if ( total > _MAXMEMPLANESIZE ) total = 0L;
# endif
  GRX_RETURN(total);
}

GrImage *_GrImageAllocate(GrContext *ctx, int nwidth,int nheight)
{
  GrImage   *img;

  GRX_ENTER();
  img = NULL;
  if ( _GrImageTestSize(nwidth, nheight) <= 0 ) goto done;
  if (!GrCreateContext(nwidth, nheight, NULL, ctx)) goto done;
  img = (GrImage *)malloc(sizeof(GrImage));
  if ( !img ) {
    GrDestroyContext(ctx);
    goto done;
  }
  img->pxp_ispixmap = 1;
  img->pxp_width  = nwidth;
  img->pxp_height = nheight;
  img->pxp_oper   = 0;
  img->pxp_source = ctx->gc_frame;
  img->pxp_source.gf_memflags =  3;/* MY_CONTEXT & MY_MEMORY */
done:
  GRX_RETURN(img);
}
