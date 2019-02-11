/**
 ** imgtest.c ---- test image functions mapping
 **
 ** Copyright (c) 1998 Hartmut Schirmer
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

#include "test.h"

#define PARTS 4

TESTFUNC(imgtest)
{
	int  x = GrSizeX();
	int  y = GrSizeY();
	int  ww = (x / PARTS)-1;
	int  wh = (y / PARTS)-1;
	int m1, m2, d1, d2;
	GrColor c1, c2, c3;
	GrContext ctx;
	GrImage *img1;
	GrImage *img2;
	if (! GrCreateContext(ww,wh,NULL,&ctx)) return;

	GrSetContext(&ctx);
	c1 = GrAllocColor(255,100,0);
	c2 = GrAllocColor(0,0,(GrNumColors() >= 16 ? 180 : 63));
	c3 = GrAllocColor(0,255,0);
	drawing(0,0,ww,wh,c1,c2);
	GrBox(0,0,ww-1,wh-1,c1);

	GrSetContext(NULL);

	img1 = GrImageFromContext(&ctx);
	if (!img1) return;

	GrFilledBox(0,0,ww+1,wh+1,c3);
	GrFilledBox(ww+15,0,2*ww+16,wh+1,c3);

	GrBitBlt(NULL,1,1,&ctx,0,0,ww-1,wh-1,0);
	GrImageDisplay(ww+16,1,img1);
	GrImageDisplayExt(0,wh+4,x-1,y-1, img1);

	GrKeyRead();

	for (m1=1; m1 <= PARTS ; m1<<=1) {
	  for (d1=1; d1 <= PARTS; d1 <<= 1) {
	    for (m2=1; m2 <= PARTS ; m2<<=1) {
	      for (d2=1; d2 <= PARTS; d2 <<= 1) {
		img2 = GrImageStretch(img1,(m1*ww)/d1, (m2*wh)/d2);
		if (img2) {
		  GrImageDisplayExt(0,0,x-1,y-1,img2);
		  GrImageDestroy(img2);
		}
	      }
	    }
	  }
	}
	GrKeyRead();

	/* let's finish with some GrGetScanline / GrPutScanline tests */
	for (d1 = 1; d1 < 32; ++d1) {
	  for (m1 = wh; m1 < y-wh-d1-1; ++m1) {
	    const GrColor *cp;
	    cp = GrGetScanline(ww+1,x-ww-d1,m1+1);
	    if (cp) {
	      GrPutScanline(ww,x-ww-d1-1,m1,cp,GrIMAGE|c2);
	    }
	  }
	}

	GrKeyRead();
}

