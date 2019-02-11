/**
 ** cxtinfo.c ---- the context info data structure
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
#include "memmode.h"
#undef   GrContextInfo

const
struct _GR_contextInfo * const GrContextInfo = &_GrContextInfo;
struct _GR_contextInfo _GrContextInfo = {
    {                                   /* the current context */
	{                               /* frame */
	    {                           /* frame start addresses */
#ifdef __MSDOS__
		LINP_PTR(MK_FP(0xb800,0)),
		LINP_PTR(MK_FP(0xb800,0)),
		LINP_PTR(MK_FP(0xb800,0)),
		LINP_PTR(MK_FP(0xb800,0))
#else
		NULL,NULL,NULL,NULL
#endif
	    },
	    0,                          /* selector */
	    TRUE,                       /* onscreen */
	    0,                          /* memflags */
	    0,                          /* lineoffset */
	    &DRVINFO->tdriver           /* frame driver */
	},
	NULL                            /* root */
    },
    {                                   /* the screen context */
	{                               /* frame */
	    {                           /* frame start addresses */
#ifdef __MSDOS__
		LINP_PTR(MK_FP(0xb800,0)),
		LINP_PTR(MK_FP(0xb800,0)),
		LINP_PTR(MK_FP(0xb800,0)),
		LINP_PTR(MK_FP(0xb800,0))
#else
		NULL,NULL,NULL,NULL
#endif
	    },
	    0,                          /* selector */
	    TRUE,                       /* onscreen */
	    0,                          /* memflags */
	    0,                          /* lineoffset */
	    &DRVINFO->tdriver           /* frame driver */
	},
	NULL                            /* root */
    }
};


