/**
 ** rblit_14.h ---- ram to ram blit support functions for 1bpp and
 **                 4bpp RAM frame drivers
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

void _GR_rblit_14(GrFrame *dst,int dx,int dy,
		  GrFrame *src,int x,int y,int w,int h,
		  GrColor op, int planes, _GR_blitFunc bitblt);
