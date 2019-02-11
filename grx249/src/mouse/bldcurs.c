/**
 ** bldcurs.c ---- create and destroy cursor data structures
 **
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
#include "allocate.h"
#include "memfill.h"

GrCursor *GrBuildCursor(char far *pixels,int pitch,int w,int h,int xo,int yo,const GrColorTableP C)
{
	GrCursor  *curs;
	GrContext  save;
	int  wrkw2 = (w + 7) & ~7;
	int  workw = wrkw2 << 1;
	int  workh = ((h + 7) & ~7) << 1;
	int  xx,yy;
	curs = malloc(sizeof(GrCursor));
	if(!curs) return(NULL);
	sttzero(curs);
	if(!GrCreateContext(workw,((workh << 1) + h),NULL,&curs->work)) {
	    free(curs);
	    return(NULL);
	}
	curs->xsize = w;
	curs->ysize = h;
	curs->xoffs = xo;
	curs->yoffs = yo;
	curs->xwork = workw;
	curs->ywork = workh;
	GrSaveContext(&save);
	GrSetContext(&curs->work);
	GrFilledBoxNC(0,0,(workw - 1),(h - 1),0L);
	for(yy = 0; yy < h; yy++) {
	    unsigned char far *p = (unsigned char far *)pixels + (yy * pitch);
	    for(xx = 0; xx < w; xx++,p++) {
		if(*p) GrPlotNC(xx,yy,GrColorValue(GR_CTABLE_COLOR(C,(*p - 1))));
		else   GrPlotNC((xx + wrkw2),yy,GrColorValue(-1L));
	    }
	}
	GrSetContext(&save);
	return(curs);
}

void GrDestroyCursor(GrCursor *cursor)
{
	if(cursor && (cursor != MOUINFO->cursor)) {
	    GrEraseCursor(cursor);
	    GrDestroyContext(&cursor->work);
	    free(cursor);
	}
}
