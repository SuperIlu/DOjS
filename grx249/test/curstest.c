/**
 ** curstest.c ---- test cursors
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

char p16d[] = {
    0,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,
    1,2,1,0,0,0,0,0,0,0,0,1,2,2,1,0,
    1,2,2,1,0,0,0,0,0,0,1,2,0,0,2,1,
    1,2,2,2,1,0,0,0,0,0,1,2,0,0,2,1,
    1,2,2,2,2,1,0,0,0,0,0,1,2,2,1,0,
    1,2,2,2,2,2,1,0,0,0,0,0,1,1,0,0,
    1,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,
    1,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0,
    1,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0,
    1,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0,
    1,2,2,2,2,2,2,2,2,2,2,1,0,0,0,0,
    1,2,2,2,2,1,1,1,1,1,1,0,0,0,0,0,
    1,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

TESTFUNC(cursortest)
{
	GrColor bgc = GrAllocColor(0,0,128);
	GrColor fgc = GrAllocColor(255,255,0);
	GrColor msc[3];
	GrCursor *cur;
	int x,y;

	msc[0] = 2;
	msc[1] = GrWhite();
	msc[2] = GrAllocColor(255,0,0);
	cur = GrBuildCursor(p16d,16,16,16,1,1,msc);
	x = GrScreenX() / 2;
	y = GrScreenY() / 2;
	GrMoveCursor(cur,x,y);
	GrClearScreen(bgc);
	GrSetColor((GrNumColors() - 1),255,255,255);
	drawing(0,0,GrSizeX(),GrSizeY(),fgc,GrNOCOLOR);
	GrFilledBox(0,0,320,120,GrAllocColor(0,255,255));
	GrTextXY( 10,90,"ANDmask",GrBlack(),GrNOCOLOR);
	GrTextXY( 90,90,"ORmask", GrBlack(),GrNOCOLOR);
	GrTextXY(170,90,"Save",   GrBlack(),GrNOCOLOR);
	GrTextXY(250,90,"Work",   GrBlack(),GrNOCOLOR);
	GrDisplayCursor(cur);
	for( ; ; ) {
	    GrBitBlt(
		NULL,10,10,
		&cur->work,cur->xwork/2,0,cur->xwork/2+cur->xsize-1,cur->ysize-1,
		GrWRITE
	    );
	    GrBitBlt(
		NULL,90,10,
		&cur->work,0,0,cur->xsize-1,cur->ysize-1,
		GrWRITE
	    );
	    GrBitBlt(
		NULL,170,10,
		&cur->work,0,cur->ysize,cur->xwork-1,cur->ysize+cur->ywork-1,
		GrWRITE
	    );
	    GrBitBlt(
		NULL,250,10,
		&cur->work,0,cur->ysize+cur->ywork,cur->xwork-1,cur->ysize+2*cur->ywork-1,
		GrWRITE
	    );
	    GrTextXY(0,GrMaxY()-20,"Type u d l r U D L R or q to quit",GrWhite(),GrNOCOLOR);
	    switch(GrKeyRead()) {
		case 'u': y--; break;
		case 'd': y++; break;
		case 'l': x--; break;
		case 'r': x++; break;
		case 'U': y -= 10; break;
		case 'D': y += 10; break;
		case 'L': x -= 10; break;
		case 'R': x += 10; break;
		case 'q': return;
		default:  continue;
	    }
	    if(x < 0) x = 0;
	    if(x > GrScreenX()) x = GrScreenX();
	    if(y < 100) y = 100;
	    if(y > GrScreenY()) y = GrScreenY();
	    GrMoveCursor(cur,x,y);
	}
}

