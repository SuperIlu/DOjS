/**
 ** memfill.h ---- inline assembly memory fill macros
 **                Borland-C++ special version
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
 ** Basic and optimized memory block fill operations in byte, word and
 ** long sizes. The fills are available in WRITE, XOR, OR and AND modes.
 **/

#include "bcc/asmsupp.h"

#define __INLINE_STD_ROWFILL__(P,V,C,FMODE,SIZE,TYPE) do {              \
	_ES = (unsigned)(void _seg *)(void far *)(P);                   \
	_DI = (unsigned)(void near *)(P);                               \
	_CX = (int)(C);                                                 \
	_AX = (int)(V);                                                 \
	__emit__((char)(0xfc));                     /* cld     */       \
	__emit__((char)(0xf3));                     /* rep     */       \
	__emit__((char)(0xaa + sizeof(TYPE) - 1));  /* stosB|W */       \
	(unsigned)(void near *)(P) = _DI;                               \
} while(0)

#define __INLINE_BCC_COLFILL__(P,V,C,SKIP,INS,SIZE) do {                \
   (P) = _GR_colfill_##SIZE##_##INS((P),(SKIP),(V),(C));                \
} while(0)

#define __INLINE_STD_COLFILL__(P,V,C,SKIP,FMODE,INS,SIZE,TYPE)          \
	__INLINE_BCC_COLFILL__(P,V,C,SKIP,INS,SIZE)

#define __INLINE_FAR_ROWFILL__(P,V,C,FMODE,SIZE,TYPE)                   \
	__INLINE_STD_ROWFILL__(P,V,C,FMODE,SIZE,TYPE)

#define __INLINE_B_REPFILL__(P,V,C,FMODE)       rowfill_b##FMODE(P,V,C)
#define __INLINE_W_REPFILL__(P,V,C,FMODE)       rowfill_w##FMODE(P,V,C)
#define __INLINE_L_REPFILL__(P,V,C,FMODE)       rowfill_l##FMODE(P,V,C)

void far *_GR_colfill_b_mov(void far *P, int O, unsigned char V, unsigned C);
void far *_GR_colfill_b_xor(void far *P, int O, unsigned char V, unsigned C);
void far *_GR_colfill_b_or( void far *P, int O, unsigned char V, unsigned C);
void far *_GR_colfill_b_and(void far *P, int O, unsigned char V, unsigned C);

void far *_GR_colfill_w_mov(void far *P, int O, unsigned V, unsigned C);
void far *_GR_colfill_w_xor(void far *P, int O, unsigned V, unsigned C);
void far *_GR_colfill_w_or( void far *P, int O, unsigned V, unsigned C);
void far *_GR_colfill_w_and(void far *P, int O, unsigned V, unsigned C);

void far *_GR_colfill_l_mov(void far *P, int O, unsigned long V, unsigned C);
void far *_GR_colfill_l_xor(void far *P, int O, unsigned long V, unsigned C);
void far *_GR_colfill_l_or( void far *P, int O, unsigned long V, unsigned C);
void far *_GR_colfill_l_and(void far *P, int O, unsigned long V, unsigned C);

#define rowfill_l(p,v,c)     do (p)=_GR_colfill_l_mov((p),4,(v),(c)); while(0)
#define rowfill_l_xor(p,v,c) do (p)=_GR_colfill_l_xor((p),4,(v),(c)); while(0)
#define rowfill_l_or(p,v,c)  do (p)=_GR_colfill_l_or( (p),4,(v),(c)); while(0)
#define rowfill_l_and(p,v,c) do (p)=_GR_colfill_l_and((p),4,(v),(c)); while(0)

#define rowfill_l_f(p,v,c)     rowfill_l(    (p),(v),(c))
#define rowfill_l_f_xor(p,v,c) rowfill_l_xor((p),(v),(c))
#define rowfill_l_f_or(p,v,c)  rowfill_l_or( (p),(v),(c))
#define rowfill_l_f_and(p,v,c) rowfill_l_and((p),(v),(c))


void far *_GR_repfill_b(    void far *P,unsigned int V, unsigned int C);
void far *_GR_repfill_b_xor(void far *P,unsigned int V, unsigned int C);
void far *_GR_repfill_b_or( void far *P,unsigned int V, unsigned int C);
void far *_GR_repfill_b_and(void far *P,unsigned int V, unsigned int C);

#define repfill_b(p,v,c)     do (p) = _GR_repfill_b(    (p),(v),(c)); while(0)
#define repfill_b_xor(p,v,c) do (p) = _GR_repfill_b_xor((p),(v),(c)); while(0)
#define repfill_b_or(p,v,c)  do (p) = _GR_repfill_b_or( (p),(v),(c)); while(0)
#define repfill_b_and(p,v,c) do (p) = _GR_repfill_b_and((p),(v),(c)); while(0)

#define repfill_b_f(p,v,c)     repfill_b(    (p),(v),(c))
#define repfill_b_f_xor(p,v,c) repfill_b_xor((p),(v),(c))
#define repfill_b_f_or(p,v,c)  repfill_b_or( (p),(v),(c))
#define repfill_b_f_and(p,v,c) repfill_b_and((p),(v),(c))


#define __INLINE_MEMFILL__(P,V,C,SIZE,TYPE,FMODE) do {                  \
	void     far *_PTR = (void far *)(P);                           \
	register int  _VAL = (int)(V);                                  \
	register int  _CNT = (int)(C);                                  \
	rowfill_##SIZE##FMODE(_PTR,_VAL,_CNT);                          \
} while(0)


void far *_GR_repfill_24_set(void far *P,unsigned long V, unsigned int B);
void far *_GR_repfill_24_xor(void far *P,unsigned long V, unsigned int B);
void far *_GR_repfill_24_or( void far *P,unsigned long V, unsigned int B);
void far *_GR_repfill_24_and(void far *P,unsigned long V, unsigned int B);

#define GRX_HAVE_FAST_REPFILL24

#define repfill_24(p,v,b)     do (p)=_GR_repfill_24_set((p),(v),(b)); while(0)
#define repfill_24_xor(p,v,b) do (p)=_GR_repfill_24_xor((p),(v),(b)); while(0)
#define repfill_24_or(p,v,b)  do (p)=_GR_repfill_24_or( (p),(v),(b)); while(0)
#define repfill_24_and(p,v,b) do (p)=_GR_repfill_24_and((p),(v),(b)); while(0)

#define repfill_24_f(p,v,b)     repfill_24(    (p),(v),(b))
#define repfill_24_f_xor(p,v,b) repfill_24_xor((p),(v),(b))
#define repfill_24_f_or(p,v,b)  repfill_24_or( (p),(v),(b))
#define repfill_24_f_and(p,v,b) repfill_24_and((p),(v),(b))
