/**
 ** circtest.c ---- test circle and ellipse rendering
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
#include <math.h>

void drawellip(int xc,int yc,int xa,int ya,GrColor c1,GrColor c2,GrColor c3)
{
	double ddx = (double)xa;
	double ddy = (double)ya;
	double R2 = ddx*ddx*ddy*ddy;
	double SQ;
	int x1,x2,y1,y2;
	int dx,dy;

	GrFilledBox(xc-xa,yc-ya,xc+xa,yc+ya,c1);
	dx = xa;
	dy = 0;
	GrPlot(xc-dx,yc,c3);
	GrPlot(xc+dx,yc,c3);
	while(++dy <= ya) {
	    SQ = R2 - (double)dy * (double)dy * ddx * ddx;
	    dx = (int)(sqrt(SQ)/ddy + 0.5);
	    x1 = xc - dx;
	    x2 = xc + dx;
	    y1 = yc - dy;
	    y2 = yc + dy;
	    GrPlot(x1,y1,c3);
	    GrPlot(x2,y1,c3);
	    GrPlot(x1,y2,c3);
	    GrPlot(x2,y2,c3);
	}
	GrEllipse(xc,yc,xa,ya,c2);
}

TESTFUNC(circtest)
{
	int     xc,yc;
	int     xr,yr;
	GrColor c1,c2,c3;

	c1 = GrAllocColor(64,64,255);
	c2 = GrAllocColor(255,255,64);
	c3 = GrAllocColor(255,64,64);
	xc = GrSizeX() / 2;
	yc = GrSizeY() / 2;
	xr = 1;
	yr = 1;
	while((xr < 1000) || (yr < 1000)) {
	    drawellip(xc,yc,xr,yr,c1,c2,c3);
	    xr += xr/4+1;
	    yr += yr/4+1;
            GrSleep(200);
	}
	c1 = GrAllocColor(64,64,128);
	xr = 4;
	yr = 1;
	while((xr < 1000) || (yr < 1000)) {
	    drawellip(xc,yc,xr,yr,c1,c2,c3);
	    yr += yr/4+1;
	    xr = yr * 4;
            GrSleep(200);
	}
	c1 = GrAllocColor(64,64,64);
	xr = 1;
	yr = 4;
	while((xr < 1000) || (yr < 1000)) {
	    drawellip(xc,yc,xr,yr,c1,c2,c3);
	    xr += xr/4+1;
	    yr = xr * 4;
            GrSleep(200);
	}

        GrTextXY(0,0,"press any key to continue",GrWhite(),GrBlack());
        GrKeyRead();
}

