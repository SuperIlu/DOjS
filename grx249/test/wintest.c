/**
 ** wintest.c ---- test window (context) mapping
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
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

TESTFUNC(wintest)
{
	int  x = GrSizeX();
	int  y = GrSizeY();
	int  ww = (x / 2) - 10;
	int  wh = (y / 2) - 10;
	GrColor c;
	GrContext *w1 = GrCreateSubContext(5,5,ww+4,wh+4,NULL,NULL);
	GrContext *w2 = GrCreateSubContext(15+ww,5,ww+ww+14,wh+4,NULL,NULL);
	GrContext *w3 = GrCreateSubContext(5,15+wh,ww+4,wh+wh+14,NULL,NULL);
	GrContext *w4 = GrCreateSubContext(15+ww,15+wh,ww+ww+14,wh+wh+14,NULL,NULL);

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

	GrKeyRead();
}

