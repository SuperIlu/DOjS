/**
 ** herc1.c ---- the mono Hercules frame driver
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu].
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
#include "grdriver.h"
#include "arith.h"
#include "mempeek.h"
#include "memcopy.h"
#include "memfill.h"

#define HRES 720
#define VRES 348

static unsigned int offtab[2 * VRES];

/* frame offset address calculation */
#define FOFS(x,y) ( offtab[y] + ((x) >> 3) )

static int init(GrVideoMode *mp)
{
    int i, res;
    GRX_ENTER();
    res = ( (mp->height == VRES) || (mp->height == 2*VRES) )
	 && (mp->width == HRES);
    if (res) {
	for(i = 0; i < VRES; i++) {
	    offtab[i] = ((i & 3) * 0x2000U) + ((i >> 2) * (HRES / 8));
	    offtab[i + VRES] = offtab[i] + 0x8000U;
	}
    }
    GRX_RETURN(res);
}

static INLINE
GrColor readpixel(GrFrame *c,int x,int y)
{
	char far *ptr;
	GRX_ENTER();
	ptr = &SCRN->gc_baseaddr[0][FOFS(x,y)];
	setup_far_selector(SCRN->gc_selector);
	GRX_RETURN((GrColor)(( peek_b_f(ptr) >> (7 - (x & 7)) ) & 1));
}

static INLINE
void drawpixel(int x,int y,GrColor color)
{
	char far *ptr;
	unsigned cval;
	GRX_ENTER();
	ptr = &CURC->gc_baseaddr[0][FOFS(x,y)];
	cval = ((unsigned)color & 1) << (7 - (x &= 7));
	setup_far_selector(CURC->gc_selector);
	switch(C_OPER(color)) {
	    case C_XOR: poke_b_f_xor(ptr,cval);  break;
	    case C_OR:  poke_b_f_or(ptr,cval);   break;
	    case C_AND: poke_b_f_and(ptr,~cval); break;
	    default:    poke_b_f(ptr,((peek_b_f(ptr) & (~0x80 >> x)) | cval));
	}
	GRX_LEAVE();
}

static
#include "fdrivers/generic/hline.c"

static
#include "fdrivers/generic/vline.c"

static
#include "fdrivers/generic/block.c"

static
#include "fdrivers/generic/line.c"

static
#include "fdrivers/generic/bitmap.c"

static
#include "fdrivers/generic/pattern.c"

static
#include "fdrivers/generic/bitblt.c"

GrFrameDriver _GrFrameDriverHERC1 = {
    GR_frameHERC1,              /* frame mode */
    GR_frameRAM1,               /* compatible RAM frame mode */
    TRUE,                       /* onscreen */
    4,                          /* line width alignment */
    1,                          /* number of planes */
    1,                          /* bits per pixel */
    64*1024L,                   /* max plane size the code can handle */
    init,
    readpixel,
    drawpixel,
    drawline,
    drawhline,
    drawvline,
    drawblock,
    drawbitmap,
    drawpattern,
    bitblt,
    _GrFrDrvGenericBitBlt,
    _GrFrDrvGenericBitBlt,
    _GrFrDrvGenericGetIndexedScanline,
    _GrFrDrvGenericPutScanline
};

