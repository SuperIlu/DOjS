/**
 ** ordswap.c ---- multibyte value order swaping
 **
 ** Copyright (c) 1998 Hartmut Schirmer
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
#include "mempeek.h"
#include "ordswap.h"

#if 1
#define SWAPBYTE(ty,mb,src,dst) (((ty)((GR_int8u)((mb) >> (8*(src)))) << (8*(dst))))

void _GR_swap16(GR_int16 far *w) {
  GR_int16 res;
  GRX_ENTER();
  res  = SWAPBYTE(GR_int16,*w,1,0);
  res |= SWAPBYTE(GR_int16,*w,0,1);
  *w = res;
  GRX_LEAVE();
}

void _GR_swap32(GR_int32 far *l) {
  GR_int32 res;
  GRX_ENTER();
  res  = SWAPBYTE(GR_int32,*l,3,0);
  res |= SWAPBYTE(GR_int16,*l,0,3);
  res |= SWAPBYTE(GR_int16,*l,2,1);
  res |= SWAPBYTE(GR_int16,*l,1,2);
  *l = res;
  GRX_LEAVE();
}

#ifdef GR_int64

void _GR_swap64(GR_int64 far *h) {
  GR_int64 res;
  GRX_ENTER();
  res  = SWAPBYTE(GR_int64,*h,7,0);
  res |= SWAPBYTE(GR_int64,*h,0,7);
  res |= SWAPBYTE(GR_int64,*h,6,1);
  res |= SWAPBYTE(GR_int64,*h,1,6);
  res |= SWAPBYTE(GR_int64,*h,5,2);
  res |= SWAPBYTE(GR_int64,*h,2,5);
  res |= SWAPBYTE(GR_int64,*h,4,3);
  res |= SWAPBYTE(GR_int64,*h,3,4);
  *h = res;
  GRX_LEAVE();
}
#endif

#else
static void swapbytes(GR_int8 far *b1, GR_int8 far *b2) {
  GR_int8 b;
  GRX_ENTER();
  b = peek_b(b1);
  poke_b(b1, peek_b(b2));
  poke_b(b2, b);
  GRX_LEAVE();
}

void _GR_swap16(GR_int16 far *w) {
  GRX_ENTER();
  swapbytes((GR_int8 far *)w, ((GR_int8 far *)w)+1);
  GRX_LEAVE();
}

void _GR_swap32(GR_int32 far *l) {
  GRX_ENTER();
  swapbytes(((GR_int8 far *)l)  , ((GR_int8 far *)l)+3);
  swapbytes(((GR_int8 far *)l)+1, ((GR_int8 far *)l)+2); 
  GRX_LEAVE();
}

#ifdef GR_int64

void _GR_swap64(GR_int64 far *h) {
  GRX_ENTER();
  swapbytes(((GR_int8 far *)h)  , ((GR_int8 far *)h)+7);
  swapbytes(((GR_int8 far *)h)+1, ((GR_int8 far *)h)+6); 
  swapbytes(((GR_int8 far *)h)+2, ((GR_int8 far *)h)+5);
  swapbytes(((GR_int8 far *)h)+3, ((GR_int8 far *)h)+4); 
  GRX_LEAVE();
}

#endif


#endif
