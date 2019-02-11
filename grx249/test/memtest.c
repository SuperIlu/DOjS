/**
 ** memtest.c ---- test memory driver
 **
 ** Copyright (C) 2001 Mariano Alvarez Fernandez
 ** [e-mail: malfer@teleline.es]
 **
 ** This is a test/demo file of the GRX graphics library.
 ** You can use GRX test/demo files as you want.
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

#include <stdio.h>
#include "grx20.h"
#include "drawing.h"

int main()
{
  int x, y, ww, wh;
  GrColor c;
  GrContext *w1, *w2, *w3, *w4;

  GrSetDriver( "memory gw 400 gh 400 nc 256" );
  GrSetMode( GR_default_graphics );

  x = GrSizeX();
  y = GrSizeY();
  ww = (x / 2) - 10;
  wh = (y / 2) - 10;
  w1 = GrCreateSubContext(5,5,ww+4,wh+4,NULL,NULL);
  w2 = GrCreateSubContext(15+ww,5,ww+ww+14,wh+4,NULL,NULL);
  w3 = GrCreateSubContext(5,15+wh,ww+4,wh+wh+14,NULL,NULL);
  w4 = GrCreateSubContext(15+ww,15+wh,ww+ww+14,wh+wh+14,NULL,NULL);

  GrSetContext(w1);
  c = GrAllocColor(200,100,100);
  drawing(0,0,ww,wh,c,GrBlack());
  GrBox(0,0,ww-1,wh-1,c);

  GrSetContext(w2);
  c = GrAllocColor(100,200,200);
  drawing(0,0,ww,wh,c,GrBlack());
  GrBox(0,0,ww-1,wh-1,c);

  GrSetContext(w3);
  c = GrAllocColor(200,200,0);
  drawing(0,0,ww,wh,c,GrBlack());
  GrBox(0,0,ww-1,wh-1,c);

  GrSetContext(w4);
  c = GrAllocColor(0,100,200);
  drawing(0,0,ww,wh,c,GrBlack());
  GrBox(0,0,ww-1,wh-1,c);

  GrSetContext( NULL );
//  GrSaveBmpImage( "memtest.bmp",NULL,0,0,639,479 );
  GrSaveContextToPpm( NULL,"memtest.ppm","GRX MemTest" );

  return 0;
}
