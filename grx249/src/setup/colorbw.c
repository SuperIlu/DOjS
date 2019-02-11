/**
 ** colorbw.c ---- standard colors: black and white
 **
 ** Copyright (c) 1998 Hartmut Schirmer
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
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

#ifdef GrBlack
#undef GrBlack
#endif
GrColor GrBlack(void)
{
        GRX_ENTER();
	if(CLRINFO->black == GrNOCOLOR) CLRINFO->black = GrAllocColor(0,0,0);
       	GRX_RETURN(CLRINFO->black);
}

#ifdef GrWhite
#undef GrWhite
#endif
GrColor GrWhite(void)
{
        GRX_ENTER();
	if(CLRINFO->white == GrNOCOLOR) CLRINFO->white = GrAllocColor(255,255,255);
	GRX_RETURN(CLRINFO->white);
}

