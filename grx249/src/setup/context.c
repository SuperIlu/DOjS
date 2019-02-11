/**
 ** context.c ----- context creation and manipulation functions
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

#include <string.h>

#include "libgrx.h"
#include "allocate.h"
#include "clipping.h"
#include "memcopy.h"
#include "memfill.h"

#define  MYCONTEXT      1
#define  MYFRAME        2

GrContext *GrCreateFrameContext(GrFrameMode md,int w,int h,char far *memory[4],GrContext *where)
{
	GrFrameDriver *fd = _GrFindRAMframeDriver(md);
	int  ii,offset,flags = 0;
	char far *mymem[4];
	long psize;

	if(!fd) return(NULL);
	offset = GrFrameLineOffset(md,w);
	psize  = GrFramePlaneSize(md,w,h);
	if(psize <= 0L) return(NULL);
	if(psize >  fd->max_plane_size) return(NULL);
	if(!where) {
	    where = malloc(sizeof(GrContext));
	    if(!where) return(NULL);
	    flags = MYCONTEXT;
	}
	sttzero(where);
	if(!memory) {
	    for(ii = 0; ii < fd->num_planes; ii++) {
		mymem[ii] = farmalloc((size_t)psize);
		if(!mymem[ii]) {
		    while(--ii >= 0) farfree(mymem[ii]);
		    if(flags) free(where);
		    return(NULL);
		}
	    }
	    while(ii < 4) mymem[ii++] = NULL;
	    memory = mymem;
	    flags |= MYFRAME;
	}
	where->gc_driver      = fd;
	where->gc_baseaddr[0] = memory[0];
	where->gc_baseaddr[1] = memory[1];
	where->gc_baseaddr[2] = memory[2];
	where->gc_baseaddr[3] = memory[3];
	where->gc_lineoffset  = offset;
	where->gc_memflags    = flags;
	where->gc_xcliphi     = where->gc_xmax = w - 1;
	where->gc_ycliphi     = where->gc_ymax = h - 1;
	return(where);
}

GrContext *GrCreateSubContext(
    int x1,int y1,int x2,int y2,
    const GrContext *parent,
    GrContext *where
){
	int flags = 0;

	if(!parent) parent = SCRN;
	if(parent->gc_root) {
	    x1 += parent->gc_xoffset;
	    y1 += parent->gc_yoffset;
	    x2 += parent->gc_xoffset;
	    y2 += parent->gc_yoffset;
	    parent = parent->gc_root;
	}
	cxclip_box_(parent,x1,y1,x2,y2,return(NULL),CLIP_EMPTY_MACRO_ARG);
	if(!where) {
	    where = malloc(sizeof(GrContext));
	    if(!where) return(NULL);
	    flags = MYCONTEXT;
	}
	sttzero(where);
	sttcopy(&where->gc_frame,&parent->gc_frame);
	where->gc_memflags = flags;
	where->gc_xoffset  = x1;
	where->gc_yoffset  = y1;
	where->gc_xcliphi  = where->gc_xmax = x2 - x1;
	where->gc_ycliphi  = where->gc_ymax = y2 - y1;
	where->gc_root     = (GrContext *)parent;
	return(where);
}

void GrResizeSubContext(GrContext *context,int x1,int y1,int x2,int y2)
{
	GrContext *parent = context->gc_root;

	if((parent = context->gc_root) == NULL) return;
/*
	x1 += context->gc_xoffset;
	y1 += context->gc_yoffset;
	x2 += context->gc_xoffset;
	y2 += context->gc_yoffset;
*/
	cxclip_box(parent,x1,y1,x2,y2);
	context->gc_xoffset = x1;
	context->gc_yoffset = y1;
	context->gc_xcliphi = context->gc_xmax = x2 - x1;
	context->gc_ycliphi = context->gc_ymax = y2 - y1;
	context->gc_xcliplo = 0;
	context->gc_ycliplo = 0;
}

void GrDestroyContext(GrContext *cxt)
{
	if(cxt && (cxt != CURC) && (cxt != SCRN)) {
	    if(cxt->gc_memflags & MYFRAME) {
		int ii = cxt->gc_driver->num_planes;
		while(--ii >= 0) farfree(cxt->gc_baseaddr[ii]);
	    }
	    if(cxt->gc_memflags & MYCONTEXT) free(cxt);
	}
}

void GrSetContext(const GrContext *context)
{
	if(!context) context = SCRN;
	sttcopy(CURC,context);
	sttcopy(FDRV,context->gc_driver);
}

GrContext *GrSaveContext(GrContext *where)
{
	int flags = 0;

	if(!where) {
	    where = malloc(sizeof(GrContext));
	    if(!where) return(NULL);
	    flags = MYCONTEXT;
	}
	sttcopy(where,CURC);
	where->gc_memflags = flags;
	return(where);
}

