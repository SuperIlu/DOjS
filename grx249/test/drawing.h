/**
 ** DRAWING.H ---- a stupid little drawing used all over in test programs
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu] See "doc/copying.cb" for details.
 **/

#include "rand.h"

void drawing(int xpos,int ypos,int xsize,int ysize,long fg,long bg)
{
#   define XP(x)   (int)((((long)(x) * (long)xsize) / 100L) + xpos)
#   define YP(y)   (int)((((long)(y) * (long)ysize) / 100L) + ypos)
	int ii;
	if(bg != GrNOCOLOR) {
		GrFilledBox(xpos,ypos,xpos+xsize-1,ypos+ysize-1,bg);
	}
	GrLine(XP(10),YP(10),XP(40),YP(40),fg);
	GrLine(XP(40),YP(10),XP(10),YP(40),fg);
	GrLine(XP(35),YP(10),XP(65),YP(40),fg);
	GrLine(XP(35),YP(40),XP(65),YP(10),fg);
	GrLine(XP(70),YP(10),XP(90),YP(40),fg);
	GrLine(XP(70),YP(40),XP(90),YP(10),fg);
	for(ii = 0; ii < 5; ii++) {
		GrBox(XP(70+2*ii),YP(10+3*ii),XP(90-2*ii),YP(40-3*ii),fg);
	}
	GrFilledBox(XP(10),YP(50),XP(60),YP(90),fg);
	GrBox(XP(70),YP(50),XP(90),YP(90),fg);
	for(ii = 0; ii < 100; ii++) {
		GrPlot(XP((RND() % 20U) + 70),YP((RND() % 40U) + 50),fg);
	}
}

#undef XP
#undef YP

