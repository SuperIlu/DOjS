/**
 ** buildfnt.c ---- allocate and fill a font descriptor
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
#include <stdio.h>

#include "libgrx.h"
#include "grfontdv.h"
#include "allocate.h"
#include "arith.h"
#include "memfill.h"

/*
 * amounts for various font conversions
 */
#define BOLDWDT(width)                  (((width)  + 3) / 10)
#define ITALWDT(height)                 (((height) + 1) / 7)
#define PROPGAP(width)                  (((width)  / 6) ? ((width) / 6) : 1)

/*
 * Font conversion option structure
 */
typedef struct {
    int     oldhgt;                     /* height of the source font */
    int     newhgt;                     /* height of the created font */
    int     boldwdt;                    /* shift and OR in X dir by this much */
    int     italwdt;                    /* tilt font by this much */
    int     dofix;                      /* convert to fixed width */
    int     fixwdt;                     /* width of the fixed cvt font */
    int     doprop;                     /* convert to proportional width */
    int     propgap;                    /* zero edge pixels for prop cvt */
} conv;

#ifdef DEBUG
static void bdump(char *hd,unsigned char *bmp,int w,int h,int pitch)
{
	char *s;
	DBGPRINTF(DBG_FONT, ("%s (%dx%d):\n",hd,w,h));
	setup_ALLOC();
	s = ALLOC((size_t) (w + 1));
	if(s == NULL) DBGPRINTF(DBG_FONT, ("no memory to show the char\n"));
	else {
	    while(--h >= 0) {
		int x;
		for(x = 0; x < w; x++) s[x] = bmp[x] ? '#' : '.';
		s[x] = '\0';
		DBGPRINTF(DBG_FONT, ("%s\n", s));
		bmp += pitch;
	    }
	    FREE(s);
	}
	reset_ALLOC();
}
#define BDUMP(hd,bmp,w,h,pitch) bdump(hd,bmp,w,h,pitch)
#else
#define BDUMP(hd,bmp,w,h,pitch)
#endif

static int cvtbitmap(int oldw,int neww,conv *c,char *bmp)
{
	int wofs,i,j,x,y,w,h;
	unsigned char *work_base;
	setup_ALLOC();
	wofs = umax(oldw,neww) + c->boldwdt + c->italwdt + c->propgap;
	wofs = umax(wofs,c->fixwdt);
	wofs = (wofs + 15) & ~3;
	i    = wofs * (umax(c->newhgt,c->oldhgt) + 2);
	work_base = ALLOC((size_t)i);
	if(work_base) {
	    unsigned char *work = work_base;
	    memzero(work,i);
	    work += wofs;
	    w = neww - c->boldwdt - c->italwdt;
	    h = c->newhgt;
	    if((w == oldw) && (h == c->oldhgt)) {
		/* No resizing is needed */
		for(y = 0; y < h; y++) {
		    i = ((w + 7) & ~7) * y;
		    j = wofs * y;
		    for(x = 0; x < w; x++,i++,j++) {
			work[j] = bmp[i >> 3] & (0x80 >> (i & 7));
		    }
		}
		BDUMP("after unpacking",work,w,h,wofs);
	    }
	    else {
		/* Resize the bitmap */
		/* The algorithm first looks for rectangles of '1' pixels */
		/* and then maps these onto the coordinate space of the */
		/* resized bitmap. This seems to work better than simple */
		/* pixel sampling. (No feature loss) */
		for(y = 0; y < c->oldhgt; y++) {
		    unsigned int pos = 0,ones = 0;
		    i = ((oldw + 7) & ~7) * y;
		    for(x = 0; x < oldw; x++,i++) {
			if(bmp[i >> 3] & (0x80 >> (i & 7))) {
			    if(!ones) pos = x;
			    ones++;
			    if(x != (oldw - 1)) continue;
			}
			if(ones > 0) {
			    unsigned int x1 = urscale(pos,w,oldw);
			    unsigned int x2 = urscale((pos + ones),w,oldw);
			    unsigned int y1 = urscale(y,h,c->oldhgt);
			    unsigned int y2 = urscale((y + 1),h,c->oldhgt);
			    do {
				unsigned int xx = x1;
				j = (y1 * wofs) + xx;
				do {
				    work[j++] = 1;
				} while(++xx < x2);
			    } while(++y1 < y2);
			    ones = 0;
			}
		    }
		}
		BDUMP("after resize",work,w,h,wofs);
	    }
	    /* bold conversion */
	    if(c->boldwdt > 0) {
		for(y = 0; y < h; y++) {
		    unsigned char *row = &work[wofs * y];
		    unsigned char *p1  = &row[w];
		    unsigned char *p2  = &row[w + c->boldwdt];
		    while(p1 > row) *--p2 |= *--p1;
		}
		w += c->boldwdt;
		BDUMP("after boldify",work,w,h,wofs);
	    }
	    /* italic conversion */
	    if(c->italwdt > 0) {
		int ymax = h - 1;
		int yrnd = ymax >> 1;
		for(y = 0; y < h; y++) {
		    int   tilt = ((c->italwdt * (ymax - y)) + yrnd) / ymax;
		    unsigned char *row = &work[wofs * y];
		    unsigned char *p1  = &row[w];
		    unsigned char *p2  = &row[w + tilt];
		    while(p1 > row) *--p2 = *--p1;
		    while(p2 > row) *--p2 = 0;
		}
		w += c->italwdt;
		BDUMP("after italicize",work,w,h,wofs);
	    }
	    x = 0;
	    /* proportional or fixed width adjustment */
	    if(c->dofix || c->doprop) {
		int minx = w;
		int maxx = 0;
		for(y = 0; y < h; y++) {
		    i = y * wofs;
		    for(j = 0; j < w; j++,i++) {
			if(work[i] && (j < minx)) minx = j;
			if(work[i] && (j > maxx)) maxx = j;
		    }
		}
		if(minx > maxx) {
		    minx = 0;
		    maxx = w - 1;
		}
		if(c->dofix) {
		    neww = c->fixwdt;
		    w    = maxx - minx + 1;
		    x    = (neww - w) >> 1;
		}
		if(c->doprop) {
		    w    = maxx - minx + 1;
		    x    = minx - (c->propgap >> 1);
		    neww = w + c->propgap;
		}
	    }
	    for(y = 0,i = (neww + 7) >> 3; y < h; y++) {
		char  *bp = &bmp[y * i];
		unsigned char *wp = &work[y * wofs];
		int  xpos = x;
		memfill_b(bp,0,i);
		for(j = 0; j < neww; j++,xpos++) {
		    if(wp[xpos]) bp[j >> 3] |= (0x80 >> (j & 7));
		}
	    }
	    FREE(work_base);
	}
	reset_ALLOC();
	return(neww);
}

GrFont *_GrBuildFont(
    const GrFontHeader *h,
    int  cvt,
    int  wdt,
    int  hgt,
    int  cmin,
    int  cmax,
    int  (*charwdt)(int chr),
    int  (*bitmap)(int chr,int w,int h,char *buffer),
    int  scaled
){
	GrFont *f    = NULL;
	unsigned int  fprop  = h->proportional;
	unsigned int  chrcv  = GR_FONTCVT_NONE;
	unsigned int  bmpcv  = GR_FONTCVT_NONE;
	unsigned long totwdt = 0L;
	unsigned long bmplen = 0L;
	unsigned int  bmpofs = 0;
	unsigned int  numch,i,chr;
	char  *bmp = NULL;
	conv  cv;
	setup_ALLOC();
	sttzero(&cv);
	if(cvt & GR_FONTCVT_SKIPCHARS) {
	    unsigned int lastfntchr = h->minchar + h->numchars - 1;
	    cmin = umin(umax(cmin,h->minchar),lastfntchr);
	    cmax = umin(umax(cmax,h->minchar),lastfntchr);
	    if(cmin > cmax) goto error;
	    if((unsigned int)cmin > h->minchar) chrcv = GR_FONTCVT_SKIPCHARS;
	    if((unsigned int)cmax < lastfntchr) chrcv = GR_FONTCVT_SKIPCHARS;
	}
	else {
	    cmin = h->minchar;
	    cmax = h->minchar + h->numchars - 1;
	}
	if(cvt & GR_FONTCVT_RESIZE) {
	    if((unsigned int)(wdt = imax(2,wdt)) != h->width)  bmpcv=GR_FONTCVT_RESIZE;
	    if((unsigned int)(hgt = imax(2,hgt)) != h->height) bmpcv=GR_FONTCVT_RESIZE;
	}
	else {
	    wdt = h->width;
	    hgt = h->height;
	}
	numch = cmax - cmin + 1;
	i = sizeof(GrFont) + ((numch - 1) * sizeof(GrFontChrInfo));
	f = malloc(i);
	if(!f) goto error;
	memzero(f,i);
	f->h.name   = malloc(strlen(h->name)   + 1);
	f->h.family = malloc(strlen(h->family) + 1);
	if(!f->h.name || !f->h.family) goto error;
	strcpy(f->h.name  ,h->name);
	strcpy(f->h.family,h->family);
	f->minwidth = 0x7fff;
	f->maxwidth = 0;
	for(chr = cmin,i = 0; i < numch; chr++,i++) {
	    int  oldw = (*charwdt)(chr);
	    unsigned int neww = urscale(oldw,wdt,h->width);
	    if(oldw < 0) goto error;
	    if(f->minwidth > neww) f->minwidth = neww;
	    if(f->maxwidth < neww) f->maxwidth = neww;
	    f->chrinfo[i].width  = oldw;
	    bmplen += ((neww + 7) >> 3) * hgt;
	}
	cv.oldhgt = scaled ? hgt : h->height;
	cv.newhgt = hgt;
	if(cvt & GR_FONTCVT_BOLDIFY) {
	    cv.boldwdt = BOLDWDT(wdt);
	    cv.boldwdt = imin(cv.boldwdt,(f->minwidth - 1));
	    cv.boldwdt = imax(cv.boldwdt,0);
	    if(cv.boldwdt > 0) bmpcv |= GR_FONTCVT_BOLDIFY;
	}
	if(cvt & GR_FONTCVT_ITALICIZE) {
	    cv.italwdt = ITALWDT(hgt);
	    cv.italwdt = imin(cv.italwdt,(f->minwidth - cv.boldwdt - 1));
	    cv.italwdt = imax(cv.italwdt,0);
	    if(cv.italwdt > 0) bmpcv |= GR_FONTCVT_ITALICIZE;
	}
	if((cvt & GR_FONTCVT_FIXIFY) && fprop) {
	    bmpcv     |= GR_FONTCVT_FIXIFY;
	    cv.fixwdt  = f->maxwidth;
	    bmplen     = umul32((hgt * ((cv.fixwdt + 7) >> 3)),numch);
	    cv.dofix   = TRUE;
	    fprop      = FALSE;
	}
	if((cvt & GR_FONTCVT_PROPORTION) && !fprop) {
	    bmpcv     |= GR_FONTCVT_PROPORTION;
	    cv.propgap = imax(0,(PROPGAP(wdt) - cv.italwdt));
	    bmplen     = umul32((hgt * ((wdt + cv.propgap + 7) >> 3)),numch);
	    cv.doprop  = TRUE;
	    fprop      = TRUE;
	}
	if((unsigned long)(unsigned int)bmplen != bmplen) goto error;
	f->bitmap = farmalloc(bmplen);
	if(!f->bitmap) goto error;
	memzero(f->bitmap,(unsigned int)bmplen);
	i = f->maxwidth;
	if(h->width > (unsigned int)wdt) i = urscale(i,h->width,wdt);
	i = ((i + 31) >> 3) * umax(hgt,h->height);
	bmp = ALLOC((size_t)i);
	if(!bmp) goto error;
	f->minwidth = 0x7fff;
	f->maxwidth = 0;
	for(chr = cmin,i = 0; i < numch; chr++,i++) {
	    unsigned int oldw = f->chrinfo[i].width;
	    unsigned int neww = imax(urscale(oldw,wdt,h->width),1);
	    unsigned int size;
	    if(scaled) {
		unsigned int raww = neww - cv.boldwdt - cv.italwdt;
		if(!(*bitmap)(chr,raww,hgt,bmp)) goto error;
		if(bmpcv & ~GR_FONTCVT_RESIZE) neww = cvtbitmap(oldw,raww,&cv,bmp);
	    }
	    else {
		if(!(*bitmap)(chr,oldw,h->height,bmp)) goto error;
		if(bmpcv) neww = cvtbitmap(oldw,neww,&cv,bmp);
	    }
	    if(f->minwidth > neww) f->minwidth = neww;
	    if(f->maxwidth < neww) f->maxwidth = neww;
	    size = ((neww + 7) >> 3) * hgt;
	    memcpy(&f->bitmap[bmpofs],bmp,size);
	    f->chrinfo[i].width  = neww;
	    f->chrinfo[i].offset = bmpofs;
	    bmpofs += size;
	    totwdt += neww;
	}
	f->h.proportional = fprop;
	f->h.scalable     = h->scalable;
	f->h.preloaded    = FALSE;
	f->h.modified     = h->modified | chrcv | bmpcv;
	f->h.minchar      = cmin;
	f->h.numchars     = numch;
	f->h.width        = (cv.dofix || cv.doprop) ? (unsigned int)(totwdt / numch) : wdt;
	f->h.height       = hgt;
	f->h.baseline     = urscale(h->baseline,hgt,h->height);
	f->h.ulpos        = urscale(h->ulpos,   hgt,h->height);
	f->h.ulheight     = urscale(h->ulheight,hgt,h->height);
	goto done;
      error:
	if(f) {
	    if(f->h.name)   free(f->h.name);
	    if(f->h.family) free(f->h.family);
	    if(f->bitmap)   farfree(f->bitmap);
	    free(f);
	    f = NULL;
	}
      done:
	if (bmp) FREE(bmp);
	reset_ALLOC();
	return(f);
}

