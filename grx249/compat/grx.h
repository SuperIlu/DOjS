/**
 ** grx.h ---- GRX 2.0 -> 1.0x backward compatibility declarations
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

#ifndef __GRX_H_INCLUDED__
#define __GRX_H_INCLUDED__

#ifndef __GRX20_H_INCLUDED__
#include "grx20.h"
#endif

/*
 * old style context creation
 */
static  char far *_context_memory_[4] = { 0 };
#ifdef  GrCreateContext
#undef  GrCreateContext
#endif
#define GrCreateContext(w,h,mem,where) (					\
    _context_memory_[0] = (char far *)(mem),					\
    GrCreateFrameContext(							\
	GrCoreFrameMode(),							\
	(w),(h),								\
	(((GrCurrentFrameDriver()->num_planes == 1) && _context_memory_[0]) ?	\
	    _context_memory_ :							\
	    (char far **)0							\
	),									\
	(where)									\
    )										\
)

/*
 * drawing stuff
 */
#define GR_MAX_POLY_VERTICES			GR_MAX_POLYGON_POINTS
#define GrCircleArc(x,y,r,s,e,c)		(GrCircleArc)((x),(y),(r),(s),(e),GR_ARC_STYLE_OPEN,(c))
#define GrEllipseArc(x,y,w,h,s,e,c)		(GrEllipseArc)((x),(y),(w),(h),(s),(e),GR_ARC_STYLE_OPEN,(c))
#define GrFilledCircleArc(x,y,r,s,e,c)		(GrFilledCircleArc)((x),(y),(r),(s),(e),GR_ARC_STYLE_CLOSE2,(c))
#define GrFilledEllipseArc(x,y,w,h,s,e,c)	(GrFilledEllipseArc)((x),(y),(w),(h),(s),(e),GR_ARC_STYLE_CLOSE2,(c))
#define GrGetLastArcCoords			GrLastArcCoords

/*
 * text stuff
 */
#define GrLoadBIOSFont				GrLoadFont	/* I don't know whether this is a good idea */
#define GrFontWidth(opt)			((opt)->txo_font->h.width)
#define GrFontHeight(opt)			((opt)->txo_font->h.height)

#endif  /* whole file */

