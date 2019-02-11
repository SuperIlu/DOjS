/**
 ** drwstrg.c ---- draw a character string
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
#include "clipping.h"
#include "text/text.h"

static void FdrvDrawBitmap(int x,int y,int w,int h,int ox, int oy,
			   char far *bmp,int pitch,int start,
			   GrColor fg,GrColor bg,GrPattern *p)
{
  GRX_ENTER();
  (*FDRV->drawbitmap)(x,y,w,h,bmp,pitch,start,fg,bg);
  GRX_LEAVE();
}

void GrDrawString(void *text,int length,int x,int y,const GrTextOption *opt)
{
  GRX_ENTER();
  _GrDrawString(text,length,x,y,opt,NULL,FdrvDrawBitmap);
  GRX_LEAVE();
}

void GrDrawChar(int chr,int x,int y,const GrTextOption *opt)
{
    char  cbuff[2];
    short sbuff[2];

    GRX_ENTER();
    switch(opt->txo_chrtype) {
      case GR_WORD_TEXT:
      case GR_ATTR_TEXT:
	sbuff[0] = chr;
	GrDrawString(sbuff,1,x,y,opt);
	break;
      default:
	cbuff[0] = chr;
	GrDrawString(cbuff,1,x,y,opt);
	break;
    }
   GRX_LEAVE();
}
