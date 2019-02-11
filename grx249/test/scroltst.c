/**
 ** scroltst.c ---- test virtual screen set/scroll
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

TESTFUNC(scrolltest)
{
	int  wdt = GrScreenX();
	int  hgt = GrScreenY();
	GrColor nc  = GrNumColors();
	int  txh = GrDefaultFont.h.height + 2;
	for( ; ; ) {
	    char buff[100];
	    char *l1 = "Screen resolution: %dx%d";
	    char *l2 = "Virtual resolution: %dx%d";
	    char *l3 = "Current screen start: x=%-4d y=%-4d";
	    char *l4 = "Commands: q -- exit program,";
	    char *l5 = "w W h H -- shrink/GROW screen width or height,";
	    char *l6 = "x X y Y -- decrease/INCREASE screen start position";
	    GrColor bgc = GrAllocColor(0,0,128);
	    GrColor fgc = GrAllocColor(200,200,0);
	    GrColor txc = GrAllocColor(255,0,255);
	    int vw = GrVirtualX();
	    int vh = GrVirtualY();
	    int vx = GrViewportX();
	    int vy = GrViewportY();
	    int x  = (vw / 3) - (strlen(l6) * GrDefaultFont.h.width / 2);
	    int y  = (vh / 3) - (3 * txh);
	    GrClearScreen(bgc);
	    drawing(0,0,vw,vh,fgc,bgc);
	    sprintf(buff,l1,wdt,hgt); GrTextXY(x,y,buff,txc,bgc); y += txh;
	    sprintf(buff,l2,vw, vh ); GrTextXY(x,y,buff,txc,bgc); y += txh;
	    for( ; ; GrSetViewport(vx,vy)) {
		int yy = y;
		vx = GrViewportX();
		vy = GrViewportY();
		sprintf(buff,l3,vx,vy); GrTextXY(x,yy,buff,txc,bgc); yy += txh;
		GrTextXY(x,yy,l4,txc,bgc); yy += txh;
		GrTextXY(x,yy,l5,txc,bgc); yy += txh;
		GrTextXY(x,yy,l6,txc,bgc); yy += txh;
		switch(GrKeyRead()) {
		    case 'w': vw -= 8; break;
		    case 'W': vw += 8; break;
		    case 'h': vh -= 8; break;
		    case 'H': vh += 8; break;
		    case 'x': vx--; continue;
		    case 'X': vx++; continue;
		    case 'y': vy--; continue;
		    case 'Y': vy++; continue;
		    case 'q': return;
		    case 'Q': return;
		    default:  continue;
		}
		GrSetMode(GR_custom_graphics,wdt,hgt,nc,vw,vh);
		break;
	    }
	}
}

