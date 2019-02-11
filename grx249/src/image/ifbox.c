/**
 ** ifbox.c ---- Source Image Utility
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

void GrImageFilledBoxAlign(int xo,int yo,int x1,int y1,int x2,int y2,GrImage *p)
{
  int iwdt, ihgt, xoff, yoff, yy, xx, copyh, copyw;
  void (*bltfun)(GrFrame*,int,int,GrFrame*,int,int,int,int,GrColor);
  xo = min(xo, min(x1,x2));
  yo = min(yo, min(y1,y2));
  clip_box(CURC,x1,y1,x2,y2);
  iwdt = p->pxp_width;
  ihgt = p->pxp_height;
  if ( (y2-y1) <= 0 || (x2-x1) <= 0 || iwdt <= 0 || ihgt <= 0) return;
  if (CURC->gc_onscreen) bltfun = CURC->gc_driver->bltr2v;
  else                   bltfun = CURC->gc_driver->bitblt;
  while (xo > x1) xo -= iwdt;
  while (yo > y1) yo -= ihgt;
  yoff = (y1-yo)%ihgt;
  yy = y1;
  mouse_block(CURC,x1,y1,x2,y2);
  x2++;
  y2++;
  do {
    xx = x1;
    copyh = min(y2-yy,ihgt-yoff);
    xoff = (x1-xo)%iwdt;
    do {
      copyw = min(x2-xx,iwdt-xoff);
      (*bltfun)( &CURC->gc_frame, xx + CURC->gc_xoffset, yy + CURC->gc_yoffset,
		 &p->pxp_source,xoff,yoff,copyw,copyh,
		 p->pxp_oper
      );
      xx += iwdt-xoff;
      xoff = 0;
    } while ( xx < x2 );
    yy += ihgt-yoff;
    yoff = 0;
  } while ( yy < y2 );
  mouse_unblock();
}

void  GrImageDisplay(int x,int y,GrImage *p)
{
  GRX_ENTER();
  GrImageFilledBoxAlign(x,y,x,y,x+p->pxp_width-1,y+p->pxp_height-1,p);
  GRX_LEAVE();
}

void  GrImageDisplayExt(int x1,int y1,int x2,int y2,GrImage* p)
{
  GRX_ENTER();
  GrImageFilledBoxAlign(x1,y1,x1,y1,x2,y2,p);
  GRX_LEAVE();
}


