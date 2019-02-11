/**
 ** usetwin.c
 **
 ** Copyright (C) 1992, Csaba Biegl
 **   820 Stirrup Dr, Nashville, TN, 37221
 **   csaba@vuse.vanderbilt.edu
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

#include "libgrx.h"

void GrSetUserWindow(int x1,int y1,int x2,int y2)
{
	CURC->gc_usrxbase  = x1;
	CURC->gc_usrybase  = y1;
	CURC->gc_usrwidth  = x2 - x1;
	CURC->gc_usrheight = y2 - y1;
}
