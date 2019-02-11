/**
 ** generic/line.c ---- generic (=slow) line draw routine
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu].
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

void drawline(int x,int y,int dx,int dy,GrColor c)
{
	int cnt,err,yoff;
	GRX_ENTER();
	yoff = 1;
	if(dx < 0) {
	    x += dx; dx = (-dx);
	    y += dy; dy = (-dy);
	}
	if(dy < 0) {
	    yoff = (-1);
	    dy   = (-dy);
	}
	if(dx > dy) {
	    err = (cnt = dx) >> 1;
	    do {
		drawpixel(x,y,c);
		if((err -= dy) < 0) err += dx,y += yoff;
		x++;
	    } while(--cnt >= 0);
	}
	else {
	    err = (cnt = dy) >> 1;
	    do {
		drawpixel(x,y,c);
		if((err -= dx) < 0) err += dy,x++;
		y += yoff;
	    } while(--cnt >= 0);
	}
	GRX_LEAVE();
}

