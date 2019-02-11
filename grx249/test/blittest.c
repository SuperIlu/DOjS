/**
 ** blittest.c ---- test various bitblt-s
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

#include <stdlib.h>
#include <string.h>

#include "test.h"

#define BHH     (GrScreenY() / 10)
int     BWW =   83;

void drbox(GrContext *src,int x,int y)
{
	GrColor c1 = GrAllocColor(0,0,255);
	GrColor c2 = GrAllocColor(255,0,0);
	int  xx;

	GrClearScreen(c1);
	GrSetContext(src);
	GrSetClipBox(x-10,y-10,x+BWW-1+10,y+BHH-1+10);
	GrClearClipBox(c2);
	GrSetClipBox(x,y,x+BWW-1,y+BHH-1);
	GrClearClipBox(GrBlack());
	GrBox(x,y,x+BWW-1,y+BHH-1,GrWhite());
	for(xx = x; xx < x+BWW; xx += 5) {
	    GrLine(xx,y,xx+BHH,y+BHH,GrWhite());
	    GrLine(xx,y,xx-BHH,y+BHH,GrWhite());
	}
	GrSetContext(NULL);
	GrResetClipBox();
}

void doblits(GrContext *src,int x,int y)
{
	int xx = (GrSizeX() - BWW)/ 2;
	int yy = 2;
	int ii;

	for(ii = 0; ii < 8; ii++) {
	    GrBitBlt(NULL,xx,yy,src,x,y,x+BWW-1,y+BHH-1,GrWRITE);
	    xx++;
	    yy += (BHH + 2);
	}
/*
  {
	GrColor xc = GrAllocColor(255,255,255) | GrXOR;
        GrKeyRead();
	xx = (GrSizeX() - BWW)/ 2;
	yy = 2;
	for(ii = 0; ii < 8; ii++) {
	    GrFilledBox(xx,yy,xx+BWW-1,yy+BHH-1,xc);
	    xx++;
	    yy += (BHH + 2);
	}
  }
*/
}

void bltest(GrContext *src,int x,int y)
{
	int ii;

	for(ii = 0; ii < 8; ii++) {
	    drbox(src,x,y);
            doblits(src,x,y);
            GrKeyRead();
	    x++;
	}
}

void blxtest(void)
{
	GrContext memc;
	int cw = (BWW + 28) & ~7;
	int ch = BHH + 20;

	bltest(NULL,GrScreenX()-BWW-8,GrScreenY()-BHH);
	bltest(NULL,0,0);
	GrCreateContext(cw,ch,NULL,&memc);
	bltest(&memc,cw-BWW-8,ch-BHH);
}

TESTFUNC(blittest)
{
	GrFBoxColors bcolors,ocolors,icolors;
	GrColor c,bg;
	int  x = GrSizeX();
	int  y = GrSizeY();
	int  ww = (x * 2) / 3;
	int  wh = (y * 2) / 3;
	int  ii,jj;
	int  wdt = ww / 150;
	int  bw = x / 17;
	int  bh = y / 17;
	int  bx,by;
	int  cnt;

	GrContext *save = GrCreateSubContext(0,0,GrMaxX(),GrMaxY(),NULL,NULL);
	GrContext *tile = GrCreateContext(bw,bh,NULL,NULL);

	blxtest();
        GrKeyRead();

	BWW = 3;
	blxtest();
        GrKeyRead();

	bcolors.fbx_intcolor = GrAllocColor(160,100,30);
	bcolors.fbx_topcolor = GrAllocColor(240,150,45);
	bcolors.fbx_leftcolor = GrAllocColor(240,150,45);
	bcolors.fbx_rightcolor = GrAllocColor(80,50,15);
	bcolors.fbx_bottomcolor = GrAllocColor(80,50,15);

	ocolors.fbx_intcolor = GrAllocColor(0,120,100);
	ocolors.fbx_topcolor = GrAllocColor(0,180,150);
	ocolors.fbx_leftcolor = GrAllocColor(0,180,150);
	ocolors.fbx_rightcolor = GrAllocColor(0,90,60);
	ocolors.fbx_bottomcolor = GrAllocColor(0,90,60);

	icolors.fbx_intcolor = bg = GrAllocColor(30,30,30);
	icolors.fbx_bottomcolor = GrAllocColor(0,180,150);
	icolors.fbx_rightcolor = GrAllocColor(0,180,150);
	icolors.fbx_leftcolor = GrAllocColor(0,90,60);
	icolors.fbx_topcolor = GrAllocColor(0,90,60);

	c = GrAllocColor(250,250,0);

	for(ii = 0,by = -(bh/3); ii < 19; ii++) {
	    for(jj = 0,bx = -(bw/2); jj < 19; jj++) {
		GrFramedBox(bx+2*wdt,by+2*wdt,bx+bw-2*wdt-1,by+bh-2*wdt-1,2*wdt,&bcolors);
		bx += bw;
	    }
	    by += bh;
	}

	GrFramedBox(ww/4-5*wdt-1,wh/4-5*wdt-1,ww/4+5*wdt+ww+1,wh/4+5*wdt+wh+1,wdt,&ocolors);
	GrFramedBox(ww/4-1,wh/4-1,ww/4+ww+1,wh/4+wh+1,wdt,&icolors);

	GrSetClipBox(ww/4,wh/4,ww/4+ww,wh/4+wh);
	drawing(ww/4,wh/4,ww,wh,c,bg);
        GrKeyRead();

	GrClearScreen(0);
	GrSetContext(save);

	bx = -(bw/2) + 15*bw;
	by = -(bh/3) + 15*bh;

	GrFramedBox(bx+2*wdt,by+2*wdt,bx+bw-2*wdt-1,by+bh-2*wdt-1,2*wdt,&bcolors);

	for (cnt=0; cnt<3; cnt++) {
	  for(ii = 0,by = -(bh/3); ii < 19; ii++) {
	    for(jj = 0,bx = -(bw/2); jj < 19; jj++) {
		if((ii != 15) || (jj != 15)) {
		    GrBitBlt(save,
			bx,by,
			save,
			-(bw/2) + 15*bw,
			-(bh/3) + 15*bh,
			-(bw/2) + 15*bw + bw - 1,
			-(bh/3) + 15*bh + bh - 1,
			cnt==1 ? GrXOR : GrWRITE
		    );
		}
		bx += bw;
	    }
	    by += bh;
	  }
	}

	GrFramedBox(ww/4-5*wdt-1,wh/4-5*wdt-1,ww/4+5*wdt+ww+1,wh/4+5*wdt+wh+1,wdt,&ocolors);
	GrFramedBox(ww/4-1,wh/4-1,ww/4+ww+1,wh/4+wh+1,wdt,&icolors);

	GrSetClipBox(ww/4,wh/4,ww/4+ww,wh/4+wh);
	drawing(ww/4,wh/4,ww,wh,c,bg);
        GrKeyRead();


	GrBitBlt(tile,
	    0,0,
	    save,
	    -(bw/2) + 15*bw,
	    -(bh/3) + 15*bh,
	    -(bw/2) + 15*bw + bw - 1,
	    -(bh/3) + 15*bh + bh - 1,
	    GrWRITE
	);
	GrSetContext(tile);
	GrFramedBox(2*wdt,2*wdt,bw-2*wdt-1,bh-2*wdt-1,2*wdt,&bcolors);

	GrClearScreen(0);
	GrSetContext(save);

	for(ii = 0,by = -(bh/3); ii < 19; ii++) {
	    for(jj = 0,bx = -(bw/2); jj < 19; jj++) {
		GrBitBlt(save,
		    bx,by,
		    tile,
		    0,0,
		    bw-1,bh-1,
		    GrWRITE
		);
		bx += bw;
	    }
	    by += bh;
	}

	GrFramedBox(ww/4-5*wdt-1,wh/4-5*wdt-1,ww/4+5*wdt+ww+1,wh/4+5*wdt+wh+1,wdt,&ocolors);
	GrFramedBox(ww/4-1,wh/4-1,ww/4+ww+1,wh/4+wh+1,wdt,&icolors);

	GrSetClipBox(ww/4,wh/4,ww/4+ww,wh/4+wh);
	drawing(ww/4,wh/4,ww,wh,c,bg);

        GrKeyRead();
	GrResetClipBox();
	GrBitBlt(NULL,
	   60,60,
	   NULL,
	   20,20,
	   GrSizeX() - 40,
	   GrSizeY() - 40,
	   GrWRITE
	);

        GrKeyRead();

	GrBitBlt(NULL,
	   10,10,
	   NULL,
	   60,60,
	   GrSizeX() - 40,
	   GrSizeY() - 40,
	   GrWRITE
	);

        GrKeyRead();

	GrSetContext(tile);
	GrClearContext(0);

	GrBitBlt(tile,
	    0,0,
	    save,
	    -(bw/2),
	    -(bh/3),
	    -(bw/2) + 15*bw + bw - 1,
	    -(bh/3) + 15*bh + bh - 1,
	    GrWRITE
	);

	GrSetContext(save);
	GrClearScreen(0);

	for(ii = 0,by = -(bh/3); ii < 18; ii++) {
	    for(jj = 0,bx = -(bw/2); jj < 18; jj++) {
		GrBitBlt(save,
		    bx,by,
		    tile,
		    0,0,
		    bw-1,bh-1,
		    GrWRITE
		);
		bx += bw;
	    }
	    by += bh;
	}

	GrFramedBox(ww/4-5*wdt-1,wh/4-5*wdt-1,ww/4+5*wdt+ww+1,wh/4+5*wdt+wh+1,wdt,&ocolors);
	GrFramedBox(ww/4-1,wh/4-1,ww/4+ww+1,wh/4+wh+1,wdt,&icolors);

	GrSetClipBox(ww/4,wh/4,ww/4+ww,wh/4+wh);
	drawing(ww/4,wh/4,ww,wh,c,bg);

        GrKeyRead();

}

