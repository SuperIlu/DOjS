/**
 ** clrinfo.c ---- the color info data structure
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
#undef   GrColorInfo

const
struct _GR_colorInfo * const GrColorInfo = &_GrColorInfo;
struct _GR_colorInfo _GrColorInfo = {
    16,                         /* ncolors: initted for 16 color VGA modes */
    0,                          /* nfree */
    0,                          /* black */
    15,                         /* white */
    FALSE,                      /* RBG   */
    { 6,    6,    6     },      /* precision */
    { 5,    5,    5     },      /* bit position */
    { 0xfc, 0xfc, 0xfc  },      /* mask */
    { 2,    2,    2     },      /* round */
    { 2,    2,    2     },      /* shift */
    2,                          /* normalization */
    {                           /* first 16 EGA/VGA colors */
	{ 0,   0,   0   },      /* black */
	{ 0,   0,   170 },      /* blue */
	{ 0,   170, 0   },      /* green */
	{ 0,   170, 170 },      /* cyan */
	{ 170, 0,   0   },      /* red */
	{ 170, 0,   170 },      /* magenta */
	{ 170, 85,  0   },      /* brown */
	{ 170, 170, 170 },      /* light gray */
	{ 85,  85,  85  },      /* dark gray */
	{ 85,  85,  255 },      /* light blue */
	{ 85,  255, 85  },      /* light green */
	{ 85,  255, 255 },      /* light cyan */
	{ 255, 85,  85  },      /* light red */
	{ 255, 85,  255 },      /* light magenta */
	{ 255, 255, 85  },      /* yellow */
	{ 255, 255, 255 }       /* white */
    }
};

