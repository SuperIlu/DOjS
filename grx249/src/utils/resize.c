/**
 ** resize.c ---- function to resize a two-D map of gray (0..255) pixels
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

#include <stdio.h>
#include <stdlib.h>

#include "libgrx.h"

static void near shrink(unsigned char far *ptr,int pitch,unsigned int oldlen,unsigned int newlen)
{
	register unsigned char far *dst = ptr;
	register unsigned char far *src = ptr;
	int  count  = newlen;
	unsigned int weight = newlen;
	do {
	    unsigned int collect = oldlen;
	    unsigned int pixsum  = 0;
	    do {
		unsigned int factor;
		if(weight == 0) weight = newlen,src += pitch;
		if((factor = weight) > collect) factor = collect;
		pixsum  += factor * (*src);
		weight  -= factor;
		collect -= factor;
	    } while(collect > 0);
	    *dst = pixsum / oldlen;
	    dst += pitch;
	} while(--count > 0);
#ifdef DEBUG
	if(dst != (ptr + (newlen * pitch))) {
	    fprintf(stderr,"resize: dst error (shrink)\n");
	    exit(1);
	}
	if(src != (ptr + ((oldlen - 1) * pitch))) {
	    fprintf(stderr,"resize: src error (shrink)\n");
	    exit(1);
	}
#endif
}

static void grow(unsigned char far *ptr,int pitch,unsigned int oldlen,unsigned int newlen)
{
	register unsigned char far *dst = ptr + (--newlen * pitch);
	register unsigned char far *src = ptr + (--oldlen * pitch);
	unsigned int rpix  = *src;
	unsigned int lpix  = rpix;
	int  count = newlen;
	int  scale = oldlen;
	do {
	    if((scale -= oldlen) < 0) {
		rpix   = lpix;
		lpix   = *(src -= pitch);
		scale += newlen;
	    }
	    *dst = ((lpix * (newlen - scale)) + (rpix * scale)) / newlen;
	    dst -= pitch;
	} while(--count >= 0);
#ifdef DEBUG
	if(dst != (ptr - pitch)) {
	    fprintf(stderr,"resize: dst error (grow)\n");
	    exit(1);
	}
	if(src != ptr) {
	    fprintf(stderr,"resize: src error (grow)\n");
	    exit(1);
	}
#endif
}

void GrResizeGrayMap(unsigned char far *map,int pitch,int ow,int oh,int nw,int nh)
{
	if(ow != nw) {
	    unsigned char far *ptr = map;
	    int	  cnt = oh;
	    if((unsigned int)ow > (unsigned int)nw) do {
		shrink(ptr,1,ow,nw);
		ptr += pitch;
	    } while(--cnt > 0);
	    else do {
		grow(ptr,1,ow,nw);
		ptr += pitch;
	    } while(--cnt > 0);
	}
	if(oh != nh) {
	    int cnt = nw;
	    if((unsigned int)oh > (unsigned int)nh) do {
		shrink(map,pitch,oh,nh);
		map++;
	    } while(--cnt > 0);
	    else do {
		grow(map,pitch,oh,nh);
		map++;
	    } while(--cnt > 0);
	}
}
