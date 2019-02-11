/**
 ** cliptest.c ---- test clipping
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
#include "rand.h"

TESTFUNC(cliptest)
{
	long delay;
	int x = GrSizeX();
	int y = GrSizeY();
	int ww = (x * 2) / 3;
	int wh = (y * 2) / 3;
	GrColor c;

	c = GrAllocColor(200,100,100);
	GrBox(ww/4-1,wh/4-1,ww/4+ww+1,wh/4+wh+1,GrWhite());
	GrSetClipBox(ww/4,wh/4,ww/4+ww,wh/4+wh);

	drawing(0,0,ww,wh,c,GrBlack());
	GrKeyRead();

	while(!GrKeyPressed()) {
	    GrFilledBox(0,0,x,y,GrBlack());
	    drawing(-(RND()%(2*ww))+ww/2,
		-(RND()%(2*wh))+wh/2,
		RND()%(3*ww)+10,
		RND()%(3*wh)+10,
		c,
		GrNOCOLOR
	    );
	    for(delay = 200000L; delay > 0L; delay--);
	}
	GrKeyRead();
}

