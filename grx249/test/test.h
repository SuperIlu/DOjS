/**
 ** test.h ---- common declarations for test programs
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

#ifndef __TEST_H_INCLUDED__
#define __TEST_H_INCLUDED__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grx20.h"
#include "grxkeys.h"
#include "drawing.h"

extern void (*testfunc)(void);
char   exit_message[2000] = { "" };
int    Argc;
char **Argv;

#define TESTFUNC(name)      \
void name(void);        \
void (*testfunc)(void) = name;  \
void name(void)

int main(int argc,char **argv)
{
	int  x = 0;
	int  y = 0;
	long c = 0;
	int xv = 0;
	int yv = 0;

	Argc = argc - 1;
	Argv = argv + 1;
	if((Argc >= 2) &&
	   (sscanf(Argv[0],"%d",&x) == 1) && (x >= 320) &&
	   (sscanf(Argv[1],"%d",&y) == 1) && (y >= 200)) {
		Argc -= 2;
		Argv += 2;
		if (Argc > 0) {
		   char *endp;
		   c = strtol(Argv[0], &endp, 0);
		   switch (*endp) {
		     case 'k':
		     case 'K': c <<= 10; break;
		     case 'm':
		     case 'M': c <<= 20; break;
		   }
		   Argc--;
		   Argv++;
		}
	}
	if((Argc >= 2) &&
	   (sscanf(Argv[0],"%d",&xv) == 1) && (xv >= x) &&
	   (sscanf(Argv[1],"%d",&yv) == 1) && (yv >= y)) {
		Argc -= 2;
		Argv += 2;
	}
	if((xv >= x) && (yv >= y) && (c >= 2))
		GrSetMode(GR_custom_graphics,x,y,c,xv,yv);
	else if(c >= 2)
		GrSetMode(GR_width_height_color_graphics,x,y,c);
	else if((x >= 320) && (y >= 200))
		GrSetMode(GR_width_height_graphics,x,y);
	else GrSetMode(GR_default_graphics);
	(*testfunc)();
	GrSetMode(GR_default_text);
	if(strlen(exit_message) > 0) {
		puts(exit_message);
	}
	return(0);
}

#endif /* _TEST_H_ */

