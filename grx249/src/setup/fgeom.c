/**
 ** fgeom.c ---- frame geometry and memory allocation utilities
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
#include "arith.h"

int GrFrameNumPlanes(GrFrameMode md)
{
	GrFrameDriver *dp = _GrFindRAMframeDriver(md);
	return(dp ? dp->num_planes : 0);
}

int GrFrameLineOffset(GrFrameMode md,int width)
{
	GrFrameDriver *dp = _GrFindRAMframeDriver(md);
	if(dp) {
	    unsigned int w = (unsigned int)width 
	                   * (dp->bits_per_pixel / dp->num_planes);
	    w = ((w + 7) >> 3);
	    w = ((w + dp->row_align - 1) / dp->row_align) * dp->row_align;
	    return(w);
	}
	return(0);
}

long GrFramePlaneSize(GrFrameMode md,int w,int h)
{
	return(umul32(GrFrameLineOffset(md,w),h));
}

long GrFrameContextSize(GrFrameMode md,int w,int h)
{
	return(umul32(GrFrameLineOffset(md,w),(GrFrameNumPlanes(md) * h)));
}

