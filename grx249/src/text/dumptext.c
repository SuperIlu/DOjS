/**
 ** dumptext.c ---- optimized fixed font text drawing
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

void GrDumpText(int col,int row,int wdt,int hgt,const GrTextRegion *r)
{
	GrColorTableP fgcp = r->txr_fgcolor.p;
	GrColorTableP bgcp = r->txr_bgcolor.p;
	GrColor fgcv = r->txr_fgcolor.v;
	GrColor bgcv = r->txr_bgcolor.v;
	int  undl = (fgcv & GR_UNDERLINE_TEXT) ? 1 : 0;
	GrFont *f = r->txr_font;
	char *ptr = r->txr_buffer;
	char *bpt = r->txr_backup;
	int  cofs = GR_TEXTCHR_SIZE(r->txr_chrtype);
	int  offs = r->txr_lineoffset;
	int  fast = bpt ? TRUE : FALSE;
	int  chrw,chrh,bmpw;
	int  xpos,ypos;
	if((f == NULL) || f->h.proportional)   return;
	if((unsigned int)col >= (unsigned int)r->txr_width)  return;
	if((unsigned int)row >= (unsigned int)r->txr_height) return;
	wdt = umin(wdt,(r->txr_width  - col));
	hgt = umin(hgt,(r->txr_height - row));
	if((wdt <= 0) || (hgt <= 0)) return;
	chrw = f->h.width;
	chrh = f->h.height;
	bmpw = (chrw + 7) >> 3;
	xpos = r->txr_xpos + (chrw * col);
	ypos = r->txr_ypos + (chrh * row);
	if(xpos < GrLowX()) {
	    int clip = (GrLowX() - xpos + chrw - 1) / chrw;
	    if((wdt -= clip) <= 0) return;
	    col += clip;
	}
	if(ypos < GrLowY()) {
	    int clip = (GrLowY() - ypos + chrh - 1) / chrh;
	    if((hgt -= clip) <= 0) return;
	    row += clip;
	}
	xpos = r->txr_xpos + (chrw * (col + wdt)) - 1;
	ypos = r->txr_ypos + (chrh * (row + hgt)) - 1;
	if(xpos > GrHighX()) {
	    int clip = (xpos - GrHighX() + chrw - 1) / chrw;
	    if((wdt -= clip) <= 0) return;
	}
	if(ypos > GrHighY()) {
	    int clip = (ypos - GrHighY() + chrh - 1) / chrh;
	    if((hgt -= clip) <= 0) return;
	}
	ptr += (row * offs) + (col * cofs);
	bpt += (row * offs) + (col * cofs);
	xpos = r->txr_xpos  + (col * chrw);
	ypos = r->txr_ypos  + (row * chrh);
	mouse_block(
	    CURC,
	    xpos,ypos,
	    (xpos + (wdt * chrw) - 1),
	    (ypos + (hgt * chrh) - 1)
	);
	for( ; --hgt >= 0; ptr += offs,bpt += offs,ypos += chrh) {
	    char *pt2 = ptr;
	    char *bp2 = bpt;
	    int   wd2 = wdt;
	    int   xp2 = xpos;
	    for( ; --wd2 >= 0; pt2 += cofs,bp2 += cofs,xp2 += chrw) {
		int  chr,attr;
		char far *bmp;
		switch(r->txr_chrtype) {
		  case GR_WORD_TEXT:
		    chr = *((unsigned short *)(pt2));
		    if(fast) {
			if(*((unsigned short *)(bp2)) == chr) continue;
			*((unsigned short *)(bp2)) = chr;
		    }
		    break;
		  case GR_ATTR_TEXT:
		    chr = *((unsigned short *)(pt2));
		    if(fast) {
			if(*((unsigned short *)(bp2)) == chr) continue;
			*((unsigned short *)(bp2)) = chr;
		    }
		    attr = GR_TEXTCHR_ATTR(chr,GR_ATTR_TEXT);
		    chr  = GR_TEXTCHR_CODE(chr,GR_ATTR_TEXT);
		    fgcv = GR_CTABLE_COLOR(fgcp,GR_ATTR_FGCOLOR(attr));
		    bgcv = GR_CTABLE_COLOR(bgcp,GR_ATTR_BGCOLOR(attr));
		    undl = GR_ATTR_UNDERLINE(attr);
		    break;
		  default:
		    chr = *((unsigned char *)(pt2));
		    if(fast) {
			if(*((unsigned char *)(bp2)) == chr) continue;
			*((unsigned char *)(bp2)) = chr;
		    }
		    break;
		}
		bmp = GrFontCharAuxBmp(f,chr,GR_TEXT_RIGHT,undl);
		if(bmp) (*FDRV->drawbitmap)(
		    (xp2  + CURC->gc_xoffset),
		    (ypos + CURC->gc_yoffset),
		    chrw,chrh,
		    bmp,bmpw,0,
		    fgcv,bgcv
		);
		else (*FDRV->drawblock)(
		    (xp2  + CURC->gc_xoffset),
		    (ypos + CURC->gc_yoffset),
		    chrw,chrh,
		    bgcv
		);
	    }
	}
	mouse_unblock();
}

void GrDumpTextRegion(const GrTextRegion *r)
{
	GrDumpText(0,0,r->txr_width,r->txr_height,r);
}

void GrDumpChar(int chr,int col,int row,const GrTextRegion *r)
{
	int offs;
	if((unsigned int)col >= (unsigned int)r->txr_width)  return;
	if((unsigned int)row >= (unsigned int)r->txr_height) return;
	switch(r->txr_chrtype) {
	  case GR_WORD_TEXT:
	  case GR_ATTR_TEXT:
	    offs = (row * r->txr_lineoffset) + (col * sizeof(short));
	    *((short *)((char *)r->txr_buffer + offs)) = chr;
	    break;
	  default:
	    offs = (row * r->txr_lineoffset) + (col * sizeof(char));
	    *((char *)((char *)r->txr_buffer + offs)) = chr;
	    break;
	}
	GrDumpText(col,row,1,1,r);
}

