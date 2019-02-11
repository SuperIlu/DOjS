/**
 ** mousetst.c ---- test mouse cursor and mouse/keyboard input
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

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "test.h"

TESTFUNC(mousetest)
{
	GrMouseEvent evt;
	GrColor bgc = GrAllocColor(0,0,128);
	GrColor fgc = GrAllocColor(255,255,0);
	int  testmotion = 0;
	int  ii,mode;

	if(GrMouseDetect()) {
	    GrMouseEventMode(1);
	    GrMouseInit();
	    GrMouseSetColors(GrAllocColor(255,0,0),GrBlack());
	    GrMouseDisplayCursor();
	    GrClearScreen(bgc);
	    ii = 0;
	    mode = GR_M_CUR_NORMAL;
	    GrTextXY(
		10,(GrScreenY() - 20),
		"Commands: 'N' -- next mouse mode, 'Q' -- exit",
		GrWhite(),
		bgc
	    );
	    for( ; ; ) {
		char msg[200];
		drawing(ii,ii,(GrSizeX() - 20),(GrSizeY() - 20),((fgc ^ bgc) | GrXOR),GrNOCOLOR);
		GrMouseGetEventT(GR_M_EVENT,&evt,0L);
		if(evt.flags & (GR_M_KEYPRESS | GR_M_BUTTON_CHANGE | testmotion)) {
		    strcpy(msg,"Got event(s): ");
#                   define mend (&msg[strlen(msg)])
		    if(evt.flags & GR_M_MOTION)      strcpy( mend,"[moved] ");
		    if(evt.flags & GR_M_LEFT_DOWN)   strcpy( mend,"[left down] ");
		    if(evt.flags & GR_M_MIDDLE_DOWN) strcpy( mend,"[middle down] ");
		    if(evt.flags & GR_M_RIGHT_DOWN)  strcpy( mend,"[right down] ");
		    if(evt.flags & GR_M_P4_DOWN)     strcpy( mend,"[p4 down] ");
		    if(evt.flags & GR_M_P5_DOWN)     strcpy( mend,"[p5 down] ");
		    if(evt.flags & GR_M_LEFT_UP)     strcpy( mend,"[left up] ");
		    if(evt.flags & GR_M_MIDDLE_UP)   strcpy( mend,"[middle up] ");
		    if(evt.flags & GR_M_RIGHT_UP)    strcpy( mend,"[right up] ");
		    if(evt.flags & GR_M_P4_UP)       strcpy( mend,"[p4 up] ");
		    if(evt.flags & GR_M_P5_UP)       strcpy( mend,"[p5 up] ");
		    if(evt.flags & GR_M_KEYPRESS)    sprintf(mend,"[key (0x%03x)] ",evt.key);
		    sprintf(mend,"at X=%d, Y=%d, ",evt.x,evt.y);
		    sprintf(mend,
			"buttons=%c%c%c, ",
			(evt.buttons & GR_M_LEFT)   ? 'L' : 'l',
			(evt.buttons & GR_M_MIDDLE) ? 'M' : 'm',
			(evt.buttons & GR_M_RIGHT)  ? 'R' : 'r'
		    );
		    sprintf(mend,"deltaT=%ld (ms)",evt.dtime);
		    strcpy (mend,"                         ");
		    GrTextXY(10,(GrScreenY() - 40),msg,GrWhite(),bgc);
		    testmotion = evt.buttons ? GR_M_MOTION : 0;
		}
		if(evt.flags & GR_M_KEYPRESS) {
		    int key = evt.key;
		    if((key == 'Q') || (key == 'q')) break;
		    if((key != 'N') && (key != 'n')) continue;
		    GrMouseEraseCursor();
		    switch(mode = (mode + 1) & 3) {
		      case GR_M_CUR_RUBBER:
			GrMouseSetCursorMode(GR_M_CUR_RUBBER,evt.x,evt.y,GrWhite() ^ bgc);
			break;
		      case GR_M_CUR_LINE:
			GrMouseSetCursorMode(GR_M_CUR_LINE,evt.x,evt.y,GrWhite() ^ bgc);
			break;
		      case GR_M_CUR_BOX:
			GrMouseSetCursorMode(GR_M_CUR_BOX,-20,-10,20,10,GrWhite() ^ bgc);
			break;
		      default:
			GrMouseSetCursorMode(GR_M_CUR_NORMAL);
			break;
		    }
		    GrMouseDisplayCursor();
		}
		if((ii += 7) > 20) ii -= 20;
	    }
	    GrMouseUnInit();
	} else {
	    GrClearScreen(bgc);
	    ii = 0;
	    mode = GR_M_CUR_NORMAL;
	    GrTextXY(
		(GrScreenX()/3),(GrScreenY() - 20),
		"Sorry, no mouse found !",
		GrWhite(),
		bgc
	    );
	}
}
