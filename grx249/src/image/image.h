/**
 ** image.h ---- Source Image Utility
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

#ifndef __IMAGE_H_INCLUDED__
#define __IMAGE_H_INCLUDED__

#ifndef __LIBGRX_H_INCLUDED__
#include "libgrx.h"
#endif

#ifndef GrImage
#define GrImage GrPixmap
#endif

int _GrImageTestSize(int wdt,int hgt);
GrImage *_GrImageAllocate(GrContext *ctx, int nwidth,int nheight);

#endif
