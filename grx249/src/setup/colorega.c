/**
 ** colorega.c ---- Alloc the standard EGA palette
 **
 ** Copyright (c) 1998  Hartmut Schirmer
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

static struct {
  GR_int8u r, g, b;
} EGArgb[16] = {
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
};

GrColor *GrAllocEgaColors(void) {
  static GrColor egapal[16];
  int i;
  for (i=0; i < 16; ++i)
    egapal[i] = GrAllocColor(EGArgb[i].r,EGArgb[i].g,EGArgb[i].b);
  return egapal;
}

