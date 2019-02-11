/**
 ** rgbtest.c ---- show 256 color RGB palette 
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

TESTFUNC(rgbtest)
{
	int x = GrSizeX();
	int y = GrSizeY();
	int ww = (x-10)/32;
	int wh = (y-10)/8;
	int ii,jj;

	GrSetRGBcolorMode();
	for(ii = 0; ii < 8; ii++) {
	    for(jj = 0; jj < 32; jj++) {
		GrFilledBox(5+jj*ww,5+ii*wh,5+jj*ww+ww-1,5+ii*wh+wh-1,ii*32+jj);
	    }
	}
	GrKeyRead();
}

