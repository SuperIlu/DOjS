/**
 ** linetest.c ---- test wide and patterned lines
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

TESTFUNC(test1)
{
	GrLineOption o1,o2,o3,o4;
	int i;
	for(i = 0; i < 2; i++) {
	    o1.lno_color   = GrAllocColor(255,0,0);
	    o1.lno_width   = 1;
	    o1.lno_pattlen = 4 * i;
	    o1.lno_dashpat = "\5\5\24\24";
	    o2.lno_color   = GrAllocColor(255,255,0);
	    o2.lno_width   = 2;
	    o2.lno_pattlen = 6 * i;
	    o2.lno_dashpat = "\5\5\24\24\2\2";
	    o3.lno_color   = GrAllocColor(0,255,255);
	    o3.lno_width   = 30;
	    o3.lno_pattlen = 8 * i;
	    o3.lno_dashpat = "\5\5\24\24\2\2\40\40";
	    o4.lno_color   = GrAllocColor(255,0,255);
	    o4.lno_width   = 4;
	    o4.lno_pattlen = 6 * i;
	    o4.lno_dashpat = "\2\2\2\2\10\10";
	    GrClearScreen(GrBlack());
	    GrCustomLine(10,10,100,100,&o1);
	    GrCustomLine(10,50,100,140,&o1);
	    GrCustomLine(10,90,100,180,&o1);
	    GrCustomLine(110,10,200,100,&o2);
	    GrCustomLine(110,50,200,140,&o2);
	    GrCustomLine(110,90,200,180,&o2);
	    GrCustomLine(210,10,300,100,&o3);
	    GrCustomLine(210,50,300,140,&o3);
	    GrCustomLine(210,90,300,180,&o3);
	    GrCustomLine(20,300,600,300,&o4);
	    GrCustomLine(20,320,600,340,&o4);
	    GrCustomLine(20,380,600,360,&o4);
	    GrCustomLine(400,100,400,300,&o4);
	    GrCustomLine(420,100,440,300,&o4);
	    GrCustomLine(480,100,460,300,&o4);
	    GrCustomLine(600,200,500,300,&o4);
	    GrKeyRead();
	    GrClearScreen(GrBlack());
	    GrCustomBox(50,50,550,350,&o3);
	    GrCustomCircle(300,200,50,&o2);
	    GrKeyRead();
	}
}

