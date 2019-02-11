/**
 ** memfill.h ---- inline assembly memory fill macros
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
 **
 **/

#ifndef __MEMFILL_H_INCLUDED__
#define __MEMFILL_H_INCLUDED__

#ifndef __MEMMODE_H_INCLUDED__
#include "memmode.h"
#endif
#ifndef __MEMPEEK_H_INCLUDED__
#include "mempeek.h"
#endif
#ifndef __ARITH_H_INCLUDED__
#include "arith.h"
#endif

#ifdef __GNUC__
#  include "gcc/memfill.h"
#elif defined(__TURBOC__)
#  include "bcc/memfill.h"
#elif defined(__WATCOMC__)
#  include "watcom/memfill.h"
#endif

#if !defined(GR_int64) && !defined(NO_64BIT_FILL)
#define NO_64BIT_FILL
#endif
#if !defined(GR_int32) && !defined(NO_32BIT_FILL)
#define NO_32BIT_FILL
#endif
#if !defined(GR_int16) && !defined(NO_16BIT_FILL)
#define NO_16BIT_FILL
#endif

#if !defined(CPSIZE_b) && defined(GR_int8)
#define CPSIZE_b     sizeof(GR_int8)
#endif
#if !defined(CPSIZE_w) && defined(GR_int16)
#define CPSIZE_w     sizeof(GR_int16)
#endif
#if !defined(CPSIZE_l) && defined(GR_int32)
#define CPSIZE_l     sizeof(GR_int32)
#endif
#if !defined(CPSIZE_h) && defined(GR_int64)
#define CPSIZE_h     sizeof(GR_int64)
#endif

#ifndef __INLINE_STD_COLFILL__
#define __INLINE_STD_COLFILL__(P,V,C,SKIP,FMODE,INS,SIZE,TYPE) do {          \
	poke##FMODE((P),(V));                                                \
	ptrinc((P),(SKIP));                                                  \
} while(--(C))
#endif

#ifndef __INLINE_STD_OPRFILL__
#define __INLINE_STD_OPRFILL__(P,V,C,FMODE,INS,SIZE,TYPE)                    \
	__INLINE_STD_COLFILL__(P,V,C,sizeof(TYPE),FMODE,INS,SIZE,TYPE)
#endif

#ifndef __INLINE_STD_ROWFILL__
#define __INLINE_STD_ROWFILL__(P,V,C,FMODE,SIZE,TYPE)                        \
	__INLINE_STD_OPRFILL__(P,V,C,FMODE,MOV_INS,SIZE,TYPE)
#endif

#ifndef __INLINE_FAR_COLFILL__
#define __INLINE_FAR_COLFILL__(P,V,C,SKIP,FMODE,INS,SIZE,TYPE)               \
	__INLINE_STD_COLFILL__(P,V,C,SKIP,FMODE,INS,SIZE,TYPE)
#endif

#ifndef __INLINE_FAR_OPRFILL__
#define __INLINE_FAR_OPRFILL__(P,V,C,FMODE,INS,SIZE,TYPE)                    \
	__INLINE_FAR_COLFILL__(P,V,C,sizeof(TYPE),FMODE,INS,SIZE,TYPE)
#endif

#ifndef __INLINE_FAR_ROWFILL__
#define __INLINE_FAR_ROWFILL__(P,V,C,FMODE,SIZE,TYPE)                        \
	__INLINE_FAR_OPRFILL__(P,V,C,FMODE,MOV_INS,SIZE,TYPE)
#endif

/*
 * rowfill_<SIZE>[_<FAR>][_<OPER>](pointer,value,count)
 */
#ifndef rowfill_b
#define rowfill_b(p,v,c)          \
	__INLINE_STD_ROWFILL__(p,v,c,_b,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(rowfill_w)
#define rowfill_w(p,v,c)          \
	__INLINE_STD_ROWFILL__(p,v,c,_w,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(rowfill_l)
#define rowfill_l(p,v,c)          \
	__INLINE_STD_ROWFILL__(p,v,c,_l,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(rowfill_h)
#define rowfill_h(p,v,c)          \
	__INLINE_STD_ROWFILL__(p,v,c,_h,OP64b,GR_int64)
#endif

#ifndef rowfill_b_xor
#define rowfill_b_xor(p,v,c)      \
	__INLINE_STD_OPRFILL__(p,v,c,_b_xor,XOR_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(rowfill_w_xor)
#define rowfill_w_xor(p,v,c)      \
	__INLINE_STD_OPRFILL__(p,v,c,_w_xor,XOR_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(rowfill_l_xor)
#define rowfill_l_xor(p,v,c)      \
	__INLINE_STD_OPRFILL__(p,v,c,_l_xor,XOR_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(rowfill_h_xor)
#define rowfill_h_xor(p,v,c)      \
	__INLINE_STD_OPRFILL__(p,v,c,_h_xor,XOR_INS,OP64b,GR_int64)
#endif

#ifndef rowfill_b_or
#define rowfill_b_or(p,v,c)       \
	__INLINE_STD_OPRFILL__(p,v,c,_b_or,OR_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(rowfill_w_or)
#define rowfill_w_or(p,v,c)       \
	__INLINE_STD_OPRFILL__(p,v,c,_w_or,OR_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(rowfill_l_or)
#define rowfill_l_or(p,v,c)       \
	__INLINE_STD_OPRFILL__(p,v,c,_l_or,OR_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(rowfill_h_or)
#define rowfill_h_or(p,v,c)       \
	__INLINE_STD_OPRFILL__(p,v,c,_h_or,OR_INS,OP64b,GR_int64)
#endif

#ifndef rowfill_b_and
#define rowfill_b_and(p,v,c)      \
	__INLINE_STD_OPRFILL__(p,v,c,_b_and,AND_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(rowfill_w_and)
#define rowfill_w_and(p,v,c)      \
	__INLINE_STD_OPRFILL__(p,v,c,_w_and,AND_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(rowfill_l_and)
#define rowfill_l_and(p,v,c)      \
	__INLINE_STD_OPRFILL__(p,v,c,_l_and,AND_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(rowfill_h_and)
#define rowfill_h_and(p,v,c)      \
	__INLINE_STD_OPRFILL__(p,v,c,_h_and,AND_INS,OP64b,GR_int64)
#endif

#define rowfill_b_n      rowfill_b
#define rowfill_w_n      rowfill_w
#define rowfill_l_n      rowfill_l
#define rowfill_h_n      rowfill_h
#define rowfill_b_xor_n  rowfill_b_xor
#define rowfill_w_xor_n  rowfill_w_xor
#define rowfill_l_xor_n  rowfill_l_xor
#define rowfill_h_xor_n  rowfill_h_xor
#define rowfill_b_or_n   rowfill_b_or
#define rowfill_w_or_n   rowfill_w_or
#define rowfill_l_or_n   rowfill_l_or
#define rowfill_h_or_n   rowfill_h_or
#define rowfill_b_and_n  rowfill_b_and
#define rowfill_w_and_n  rowfill_w_and
#define rowfill_l_and_n  rowfill_l_and
#define rowfill_h_and_n  rowfill_h_and



#ifndef rowfill_b_f
#define rowfill_b_f(p,v,c)        \
	__INLINE_FAR_ROWFILL__(p,v,c,_b_f,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(rowfill_w_f)
#define rowfill_w_f(p,v,c)        \
	__INLINE_FAR_ROWFILL__(p,v,c,_w_f,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(rowfill_l_f)
#define rowfill_l_f(p,v,c)        \
	__INLINE_FAR_ROWFILL__(p,v,c,_l_f,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(rowfill_h_f)
#define rowfill_h_f(p,v,c)        \
	__INLINE_FAR_ROWFILL__(p,v,c,_h_f,OP64b,GR_int64)
#endif

#ifndef rowfill_b_f_xor
#define rowfill_b_f_xor(p,v,c)    \
	__INLINE_FAR_OPRFILL__(p,v,c,_b_f_xor,XOR_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(rowfill_w_f_xor)
#define rowfill_w_f_xor(p,v,c)    \
	__INLINE_FAR_OPRFILL__(p,v,c,_w_f_xor,XOR_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(rowfill_l_f_xor)
#define rowfill_l_f_xor(p,v,c)    \
	__INLINE_FAR_OPRFILL__(p,v,c,_l_f_xor,XOR_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(rowfill_h_f_xor)
#define rowfill_h_f_xor(p,v,c)    \
	__INLINE_FAR_OPRFILL__(p,v,c,_h_f_xor,XOR_INS,OP64b,GR_int64)
#endif

#ifndef rowfill_b_f_or
#define rowfill_b_f_or(p,v,c)     \
	__INLINE_FAR_OPRFILL__(p,v,c,_b_f_or,OR_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(rowfill_w_f_or)
#define rowfill_w_f_or(p,v,c)     \
	__INLINE_FAR_OPRFILL__(p,v,c,_w_f_or,OR_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(rowfill_l_f_or)
#define rowfill_l_f_or(p,v,c)     \
	__INLINE_FAR_OPRFILL__(p,v,c,_l_f_or,OR_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(rowfill_h_f_or)
#define rowfill_h_f_or(p,v,c)     \
	__INLINE_FAR_OPRFILL__(p,v,c,_h_f_or,OR_INS,OP64b,GR_int64)
#endif

#ifndef rowfill_b_f_and
#define rowfill_b_f_and(p,v,c)    \
	__INLINE_FAR_OPRFILL__(p,v,c,_b_f_and,AND_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(rowfill_w_f_and)
#define rowfill_w_f_and(p,v,c)    \
	__INLINE_FAR_OPRFILL__(p,v,c,_w_f_and,AND_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(rowfill_l_f_and)
#define rowfill_l_f_and(p,v,c)    \
	__INLINE_FAR_OPRFILL__(p,v,c,_l_f_and,AND_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(rowfill_h_f_and)
#define rowfill_h_f_and(p,v,c)    \
	__INLINE_FAR_OPRFILL__(p,v,c,_h_f_and,AND_INS,OP64b,GR_int64)
#endif

/*
 * colfill_<SIZE>[_<FAR>][_<OPER>](pointer,offset,value,count)
 */
#ifndef colfill_b
#define colfill_b(p,o,v,c)        \
	__INLINE_STD_COLFILL__(p,v,c,o,_b,MOV_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(colfill_w)
#define colfill_w(p,o,v,c)        \
	__INLINE_STD_COLFILL__(p,v,c,o,_w,MOV_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(colfill_l)
#define colfill_l(p,o,v,c)        \
	__INLINE_STD_COLFILL__(p,v,c,o,_l,MOV_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(colfill_h)
#define colfill_h(p,o,v,c)        \
	__INLINE_STD_COLFILL__(p,v,c,o,_h,MOV_INS,OP64b,GR_int64)
#endif

#ifndef colfill_b_xor
#define colfill_b_xor(p,o,v,c)    \
	__INLINE_STD_COLFILL__(p,v,c,o,_b_xor,XOR_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(colfill_w_xor)
#define colfill_w_xor(p,o,v,c)    \
	__INLINE_STD_COLFILL__(p,v,c,o,_w_xor,XOR_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(colfill_l_xor)
#define colfill_l_xor(p,o,v,c)    \
	__INLINE_STD_COLFILL__(p,v,c,o,_l_xor,XOR_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(colfill_h_xor)
#define colfill_h_xor(p,o,v,c)    \
	__INLINE_STD_COLFILL__(p,v,c,o,_h_xor,XOR_INS,OP64b,GR_int64)
#endif

#ifndef colfill_b_or
#define colfill_b_or(p,o,v,c)     \
	__INLINE_STD_COLFILL__(p,v,c,o,_b_or,OR_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(colfill_w_or)
#define colfill_w_or(p,o,v,c)     \
	__INLINE_STD_COLFILL__(p,v,c,o,_w_or,OR_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(colfill_l_or)
#define colfill_l_or(p,o,v,c)     \
	__INLINE_STD_COLFILL__(p,v,c,o,_l_or,OR_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(colfill_h_or)
#define colfill_h_or(p,o,v,c)     \
	__INLINE_STD_COLFILL__(p,v,c,o,_h_or,OR_INS,OP64b,GR_int64)
#endif

#ifndef colfill_b_and
#define colfill_b_and(p,o,v,c)    \
	__INLINE_STD_COLFILL__(p,v,c,o,_b_and,AND_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(colfill_w_and)
#define colfill_w_and(p,o,v,c)    \
	__INLINE_STD_COLFILL__(p,v,c,o,_w_and,AND_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(colfill_l_and)
#define colfill_l_and(p,o,v,c)    \
	__INLINE_STD_COLFILL__(p,v,c,o,_l_and,AND_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(colfill_h_and)
#define colfill_h_and(p,o,v,c)    \
	__INLINE_STD_COLFILL__(p,v,c,o,_h_and,AND_INS,OP64b,GR_int64)
#endif

#ifndef colfill_b_f
#define colfill_b_f(p,o,v,c)      \
	__INLINE_FAR_COLFILL__(p,v,c,o,_b_f,MOV_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(colfill_w_f)
#define colfill_w_f(p,o,v,c)      \
	__INLINE_FAR_COLFILL__(p,v,c,o,_w_f,MOV_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(colfill_l_f)
#define colfill_l_f(p,o,v,c)      \
	__INLINE_FAR_COLFILL__(p,v,c,o,_l_f,MOV_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(colfill_h_f)
#define colfill_h_f(p,o,v,c)      \
	__INLINE_FAR_COLFILL__(p,v,c,o,_h_f,MOV_INS,OP64b,GR_int64)
#endif

#ifndef colfill_b_f_xor
#define colfill_b_f_xor(p,o,v,c)  \
	__INLINE_FAR_COLFILL__(p,v,c,o,_b_f_xor,XOR_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(colfill_w_f_xor)
#define colfill_w_f_xor(p,o,v,c)  \
	__INLINE_FAR_COLFILL__(p,v,c,o,_w_f_xor,XOR_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(colfill_l_f_xor)
#define colfill_l_f_xor(p,o,v,c)  \
	__INLINE_FAR_COLFILL__(p,v,c,o,_l_f_xor,XOR_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(colfill_h_f_xor)
#define colfill_h_f_xor(p,o,v,c)  \
	__INLINE_FAR_COLFILL__(p,v,c,o,_h_f_xor,XOR_INS,OP64b,GR_int64)
#endif

#ifndef colfill_b_f_or
#define colfill_b_f_or(p,o,v,c)   \
	__INLINE_FAR_COLFILL__(p,v,c,o,_b_f_or,OR_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(colfill_w_f_or)
#define colfill_w_f_or(p,o,v,c)   \
	__INLINE_FAR_COLFILL__(p,v,c,o,_w_f_or,OR_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(colfill_l_f_or)
#define colfill_l_f_or(p,o,v,c)   \
	__INLINE_FAR_COLFILL__(p,v,c,o,_l_f_or,OR_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(colfill_h_f_or)
#define colfill_h_f_or(p,o,v,c)   \
	__INLINE_FAR_COLFILL__(p,v,c,o,_h_f_or,OR_INS,OP64b,GR_int64)
#endif

#ifndef colfill_b_f_and
#define colfill_b_f_and(p,o,v,c)  \
	__INLINE_FAR_COLFILL__(p,v,c,o,_b_f_and,AND_INS,OP8b,GR_int8)
#endif
#if !defined(NO_16BIT_FILL) && !defined(colfill_w_f_and)
#define colfill_w_f_and(p,o,v,c)  \
	__INLINE_FAR_COLFILL__(p,v,c,o,_w_f_and,AND_INS,OP16b,GR_int16)
#endif
#if !defined(NO_32BIT_FILL) && !defined(colfill_l_f_and)
#define colfill_l_f_and(p,o,v,c)  \
	__INLINE_FAR_COLFILL__(p,v,c,o,_l_f_and,AND_INS,OP32b,GR_int32)
#endif
#if !defined(NO_64BIT_FILL) && !defined(colfill_h_f_and)
#define colfill_h_f_and(p,o,v,c)  \
	__INLINE_FAR_COLFILL__(p,v,c,o,_h_f_and,AND_INS,OP64b,GR_int64)
#endif

/*
 * Optimized row fill operations in byte, word and long sizes.
 * The idea is that the bulk of the fill operation is done using
 * aligned long fills with a few possible shorter alignment and/or pad
 * fills at start and/or end. The smaller size routines (byte, word)
 * still assume a long fill argument with the fill value replicated
 * in the upper bytes.
 */

/* generic single element fill */
#ifndef __INLINE_1_FILL__
#define __INLINE_1_FILL__(P,V,WOP,SZ) do {                                    \
    poke_##SZ##WOP((P),(V));                                                  \
    ptrinc((P),(CPSIZE_##SZ));                                                \
} while (0)
#endif

/* fill and step to next higher alignment boundary */
#ifndef __INLINE_ALIGN_FILL__
#define __INLINE_ALIGN_FILL__(P,V,C,WOP,SZ,BASE)                              \
  if ( ((GR_PtrInt)(P)) & CPSIZE_##SZ ) {                                           \
    __INLINE_1_FILL__(P,V,WOP,SZ);                                            \
    (C) -= (CPSIZE_##SZ/CPSIZE_##BASE);                                       \
    if ( ! ((GR_PtrInt)(C)) ) break;                                                \
  }
#endif

/* fill && step remaining bytes after otimal fill */
#ifndef __INLINE_TAIL_FILL__
#define __INLINE_TAIL_FILL__(P,V,C,WOP,SZ,BASE) do {                          \
  if ( ((GR_PtrInt)(C)) & (CPSIZE_##SZ/CPSIZE_##BASE) )                             \
    __INLINE_1_FILL__(P,V,WOP,SZ);                                            \
} while (0)
#endif

#ifndef __INLINE_STD_OPT_FILL__
#define __INLINE_STD_OPT_FILL__(P,V,C,WOP,SZ,BASE) do {                       \
      if ((unsigned GR_PtrInt)(C) >= 2*(CPSIZE_##SZ/CPSIZE_##BASE)-1 ) {            \
	unsigned GR_PtrInt _c_ = (-((GR_PtrInt)(P))) & ((CPSIZE_##SZ/CPSIZE_##BASE)-1);   \
	if (_c_) {                                                            \
	  (C) -= _c_;                                                         \
	  rowfill_##BASE##WOP(P,V,_c_);                                       \
	}                                                                     \
	_c_ = ((unsigned GR_PtrInt)(C)) / (CPSIZE_##SZ/CPSIZE_##BASE);              \
	rowfill_##SZ##WOP(P,V,_c_);                                           \
	(C) &= ((CPSIZE_##SZ/CPSIZE_##BASE)-1);                               \
      }                                                                       \
      if ( (GR_PtrInt)(C) )                                                         \
	rowfill_b##WOP(P,V,C);                                                \
} while(0)
#endif


#ifndef __INLINE_B_REPFILL__
# if !defined(NO_64BIT_FILL)
#  if defined(MISALIGNED_32bit_OK) && defined(MISALIGNED_16bit_OK)
#   define __INLINE_B_REPFILL__(P,V,C,FMODE) do {                             \
	if ((unsigned GR_PtrInt)(C) >= 7 ) {                                        \
	   __INLINE_ALIGN_FILL__(P,V,C,FMODE,b,b);                            \
	   __INLINE_ALIGN_FILL__(P,V,C,FMODE,w,b);                            \
	   __INLINE_ALIGN_FILL__(P,V,C,FMODE,l,b);                            \
	   { unsigned GR_PtrInt _c64_ = ((unsigned GR_PtrInt)(C)) >> 3;                   \
	     if (_c64_) rowfill_h##FMODE(P,V,_c64_);         }                \
	}                                                                     \
	__INLINE_TAIL_FILL__(P,V,C,FMODE,l,b);                                \
	__INLINE_TAIL_FILL__(P,V,C,FMODE,w,b);                                \
	__INLINE_TAIL_FILL__(P,V,C,FMODE,b,b);                                \
    } while (0)
#  else
#   define __INLINE_B_REPFILL__(P,V,C,FMODE) \
	   __INLINE_STD_OPT_FILL__(P,V,C,FMODE,h,b)
#  endif
# elif !defined(NO_32BIT_FILL)
#  if defined(MISALIGNED_16bit_OK)
#   define __INLINE_B_REPFILL__(P,V,C,FMODE) do {                             \
	if ((unsigned GR_PtrInt)(C) >= 3 ) {                                        \
	   __INLINE_ALIGN_FILL__(P,V,C,FMODE,b,b);                            \
	   __INLINE_ALIGN_FILL__(P,V,C,FMODE,w,b);                            \
	   { unsigned GR_PtrInt _c32_ = ((unsigned GR_PtrInt)(C)) >> 2;                   \
	     if (_c32_) rowfill_l##FMODE(P,V,_c32_);         }                \
	}                                                                     \
	__INLINE_TAIL_FILL__(P,V,C,FMODE,w,b);                                \
	__INLINE_TAIL_FILL__(P,V,C,FMODE,b,b);                                \
    } while (0)
#  else
#   define __INLINE_B_REPFILL__(P,V,C,FMODE) \
	   __INLINE_STD_OPT_FILL__(P,V,C,FMODE,l,b)
#  endif
# elif !defined(NO_16BIT_FILL)
#   define __INLINE_B_REPFILL__(P,V,C,FMODE) do {                             \
	__INLINE_ALIGN_FILL__(P,V,C,FMODE,b,b);                               \
	{ unsigned GR_PtrInt _c16_ = ((unsigned GR_PtrInt)(C)) >> 1;                      \
	  if (_c16_) rowfill_w##FMODE(P,V,_c16_);            }                \
	__INLINE_TAIL_FILL__(P,V,C,FMODE,b,b);                                \
    } while (0)
# else
#  define __INLINE_B_REPFILL__(P,V,C,FMODE) \
	  rowfill_b##FMODE(P,V,C)
# endif
#endif /* !__INLINE_B_REPFILL__ */

#if !defined(NO_16BIT_FILL) && !defined(__INLINE_W_REPFILL__)
# if !defined(NO_64BIT_FILL)
#  if defined(MISALIGNED_32bit_OK)
#   define __INLINE_W_REPFILL__(P,V,C,FMODE) do {                             \
	if ((unsigned GR_PtrInt)(C) >= 3 ) {                                        \
	   __INLINE_ALIGN_FILL__(P,V,C,FMODE,w,w);                            \
	   __INLINE_ALIGN_FILL__(P,V,C,FMODE,l,w);                            \
	   { unsigned GR_PtrInt _c64_ = ((unsigned GR_PtrInt)(C)) >> 2;                   \
	     if (_c64_) rowfill_h##FMODE(P,V,_c64_);         }                \
	}                                                                     \
	__INLINE_TAIL_FILL__(P,V,C,FMODE,l,w);                                \
	__INLINE_TAIL_FILL__(P,V,C,FMODE,w,w);                                \
    } while (0)
#  else
#   define __INLINE_W_REPFILL__(P,V,C,FMODE) \
	   __INLINE_STD_OPT_FILL__(P,V,C,FMODE,h,w)
#  endif
# elif !defined(NO_32BIT_FILL)
#   define __INLINE_W_REPFILL__(P,V,C,FMODE) do {                             \
	__INLINE_ALIGN_FILL__(P,V,C,FMODE,w,w);                               \
	{ unsigned GR_PtrInt _c32_ = ((unsigned GR_PtrInt)(C)) >> 1;                      \
	  if (_c32_) rowfill_l##FMODE(P,V,_c32_);            }                \
	__INLINE_TAIL_FILL__(P,V,C,FMODE,w,w);                                \
    } while (0)
# else
#  define __INLINE_W_REPFILL__(P,V,C,FMODE) \
	  rowfill_w##FMODE(P,V,C)
# endif
#endif /* !__INLINE_W_REPFILL__ */

#if !defined(NO_32BIT_FILL) && !defined(__INLINE_L_REPFILL__)
# if !defined(NO_64BIT_FILL)
#   define __INLINE_L_REPFILL__(P,V,C,FMODE) do {                             \
	__INLINE_ALIGN_FILL__(P,V,C,FMODE,l,l);                               \
	{ unsigned GR_PtrInt _c64_ = ((unsigned GR_PtrInt)(C)) >> 1;                      \
	  if (_c64_) rowfill_h##FMODE(P,V,_c64_);            }                \
	__INLINE_TAIL_FILL__(P,V,C,FMODE,l,l);                                \
    } while (0)
# else
#  define __INLINE_L_REPFILL__(P,V,C,FMODE) \
	  rowfill_l##FMODE(P,V,C)
# endif
#endif /* !__INLINE_L_REPFILL__ */

#if !defined(NO_64BIT_FILL) && !defined(__INLINE_H_REPFILL__)
#define __INLINE_H_REPFILL__(P,V,C,FMODE) do {                          \
	rowfill_h##FMODE((P),(V),(C));                                  \
} while(0)
#endif /* !__INLINE_H_REPFILL__ */

/*
 * repfill_<SIZE>[_<FAR>][_<OPER>](pointer,value,count)
 */
#ifndef repfill_b
#define repfill_b(p,v,c)        __INLINE_B_REPFILL__(p,v,c,_n)
#endif
#if defined(__INLINE_W_REPFILL__) && !defined(repfill_w)
#define repfill_w(p,v,c)        __INLINE_W_REPFILL__(p,v,c,_n)
#endif
#if defined(__INLINE_L_REPFILL__) && !defined(repfill_l)
#define repfill_l(p,v,c)        __INLINE_L_REPFILL__(p,v,c,_n)
#endif
#if defined(__INLINE_H_REPFILL__) && !defined(repfill_h)
#define repfill_h(p,v,c)        __INLINE_H_REPFILL__(p,v,c,_n)
#endif

#ifndef repfill_b_xor
#define repfill_b_xor(p,v,c)    __INLINE_B_REPFILL__(p,v,c,_xor)
#endif
#if defined(__INLINE_W_REPFILL__) && !defined(repfill_w_xor)
#define repfill_w_xor(p,v,c)    __INLINE_W_REPFILL__(p,v,c,_xor)
#endif
#if defined(__INLINE_L_REPFILL__) && !defined(repfill_l_xor)
#define repfill_l_xor(p,v,c)    __INLINE_L_REPFILL__(p,v,c,_xor)
#endif
#if defined(__INLINE_H_REPFILL__) && !defined(repfill_h_xor)
#define repfill_h_xor(p,v,c)    __INLINE_H_REPFILL__(p,v,c,_xor)
#endif

#ifndef repfill_b_or
#define repfill_b_or(p,v,c)     __INLINE_B_REPFILL__(p,v,c,_or)
#endif
#if defined(__INLINE_W_REPFILL__) && !defined(repfill_w_or)
#define repfill_w_or(p,v,c)     __INLINE_W_REPFILL__(p,v,c,_or)
#endif
#if defined(__INLINE_L_REPFILL__) && !defined(repfill_l_or)
#define repfill_l_or(p,v,c)     __INLINE_L_REPFILL__(p,v,c,_or)
#endif
#if defined(__INLINE_H_REPFILL__) && !defined(repfill_h_or)
#define repfill_h_or(p,v,c)     __INLINE_H_REPFILL__(p,v,c,_or)
#endif

#ifndef repfill_b_and
#define repfill_b_and(p,v,c)    __INLINE_B_REPFILL__(p,v,c,_and)
#endif
#if defined(__INLINE_W_REPFILL__) && !defined(repfill_w_and)
#define repfill_w_and(p,v,c)    __INLINE_W_REPFILL__(p,v,c,_and)
#endif
#if defined(__INLINE_L_REPFILL__) && !defined(repfill_l_and)
#define repfill_l_and(p,v,c)    __INLINE_L_REPFILL__(p,v,c,_and)
#endif
#if defined(__INLINE_H_REPFILL__) && !defined(repfill_h_and)
#define repfill_h_and(p,v,c)    __INLINE_H_REPFILL__(p,v,c,_and)
#endif

#ifndef repfill_b_f
#define repfill_b_f(p,v,c)      __INLINE_B_REPFILL__(p,v,c,_f)
#endif
#if defined(__INLINE_W_REPFILL__) && !defined(repfill_w_f)
#define repfill_w_f(p,v,c)      __INLINE_W_REPFILL__(p,v,c,_f)
#endif
#if defined(__INLINE_L_REPFILL__) && !defined(repfill_l_f)
#define repfill_l_f(p,v,c)      __INLINE_L_REPFILL__(p,v,c,_f)
#endif
#if defined(__INLINE_H_REPFILL__) && !defined(repfill_h_f)
#define repfill_h_f(p,v,c)      __INLINE_H_REPFILL__(p,v,c,_f)
#endif

#ifndef repfill_b_f_xor
#define repfill_b_f_xor(p,v,c)  __INLINE_B_REPFILL__(p,v,c,_f_xor)
#endif
#if defined(__INLINE_W_REPFILL__) && !defined(repfill_w_f_xor)
#define repfill_w_f_xor(p,v,c)  __INLINE_W_REPFILL__(p,v,c,_f_xor)
#endif
#if defined(__INLINE_L_REPFILL__) && !defined(repfill_l_f_xor)
#define repfill_l_f_xor(p,v,c)  __INLINE_L_REPFILL__(p,v,c,_f_xor)
#endif
#if defined(__INLINE_H_REPFILL__) && !defined(repfill_h_f_xor)
#define repfill_h_f_xor(p,v,c)  __INLINE_H_REPFILL__(p,v,c,_f_xor)
#endif

#ifndef repfill_b_f_or
#define repfill_b_f_or(p,v,c)   __INLINE_B_REPFILL__(p,v,c,_f_or)
#endif
#if defined(__INLINE_W_REPFILL__) && !defined(repfill_w_f_or)
#define repfill_w_f_or(p,v,c)   __INLINE_W_REPFILL__(p,v,c,_f_or)
#endif
#if defined(__INLINE_L_REPFILL__) && !defined(repfill_l_f_or)
#define repfill_l_f_or(p,v,c)   __INLINE_L_REPFILL__(p,v,c,_f_or)
#endif
#if defined(__INLINE_H_REPFILL__) && !defined(repfill_h_f_or)
#define repfill_h_f_or(p,v,c)   __INLINE_H_REPFILL__(p,v,c,_f_or)
#endif

#ifndef repfill_b_f_and
#define repfill_b_f_and(p,v,c)  __INLINE_B_REPFILL__(p,v,c,_f_and)
#endif
#if defined(__INLINE_W_REPFILL__) && !defined(repfill_w_f_and)
#define repfill_w_f_and(p,v,c)  __INLINE_W_REPFILL__(p,v,c,_f_and)
#endif
#if defined(__INLINE_L_REPFILL__) && !defined(repfill_l_f_and)
#define repfill_l_f_and(p,v,c)  __INLINE_L_REPFILL__(p,v,c,_f_and)
#endif
#if defined(__INLINE_H_REPFILL__) && !defined(repfill_h_f_and)
#define repfill_h_f_and(p,v,c)  __INLINE_H_REPFILL__(p,v,c,_f_and)
#endif

#define repfill_b_n      repfill_b
#define repfill_w_n      repfill_w
#define repfill_l_n      repfill_l
#define repfill_h_n      repfill_h
#define repfill_b_xor_n  repfill_b_xor
#define repfill_w_xor_n  repfill_w_xor
#define repfill_l_xor_n  repfill_l_xor
#define repfill_h_xor_n  repfill_h_xor
#define repfill_b_or_n   repfill_b_or
#define repfill_w_or_n   repfill_w_or
#define repfill_l_or_n   repfill_l_or
#define repfill_h_or_n   repfill_h_or
#define repfill_b_and_n  repfill_b_and
#define repfill_w_and_n  repfill_w_and
#define repfill_l_and_n  repfill_l_and
#define repfill_h_and_n  repfill_h_and


/*
 * Another set of optimized fills which also do the replication.
 */
#if !defined(NO_64BIT_FILL)
#define GR_repl GR_int64u
#define freplicate_b(V)         replicate_b2h(V)
#define freplicate_w(V)         replicate_w2h(V)
#define freplicate_l(V)         replicate_l2h(V)
#elif !defined(NO_32BIT_FILL)
#define GR_repl GR_int32u
#define freplicate_b(V)         replicate_b2l(V)
#define freplicate_w(V)         replicate_w2l(V)
#define freplicate_l(V)         (V)
#elif !defined(NO_16BIT_FILL)
#define GR_repl GR_int16u
#define freplicate_b(V)         replicate_b2w(V)
#define freplicate_w(V)         (V)
#else
#define GR_repl GR_int8u
#define freplicate_b(V)         (V)
#endif

/*
 * optfill_<SIZE>[_<FAR>][_<OPER>](pointer,value,count)
 */
#define optfill_b(p,v,c)        repfill_b(p,freplicate_b(v),c)
#define optfill_w(p,v,c)        repfill_w(p,freplicate_w(v),c)
#define optfill_l(p,v,c)        repfill_l(p,freplicate_l(v),c)

#define optfill_b_xor(p,v,c)    repfill_b_xor(p,freplicate_b(v),c)
#define optfill_w_xor(p,v,c)    repfill_w_xor(p,freplicate_w(v),c)
#define optfill_l_xor(p,v,c)    repfill_l_xor(p,freplicate_l(v),c)

#define optfill_b_or(p,v,c)     repfill_b_or(p,freplicate_b(v),c)
#define optfill_w_or(p,v,c)     repfill_w_or(p,freplicate_w(v),c)
#define optfill_l_or(p,v,c)     repfill_l_or(p,freplicate_l(v),c)

#define optfill_b_and(p,v,c)    repfill_b_and(p,freplicate_b(v),c)
#define optfill_w_and(p,v,c)    repfill_w_and(p,freplicate_w(v),c)
#define optfill_l_and(p,v,c)    repfill_l_and(p,freplicate_l(v),c)

#define optfill_b_f(p,v,c)      repfill_b_f(p,freplicate_b(v),c)
#define optfill_w_f(p,v,c)      repfill_w_f(p,freplicate_w(v),c)
#define optfill_l_f(p,v,c)      repfill_l_f(p,freplicate_l(v),c)

#define optfill_b_f_xor(p,v,c)  repfill_b_f_xor(p,freplicate_b(v),c)
#define optfill_w_f_xor(p,v,c)  repfill_w_f_xor(p,freplicate_w(v),c)
#define optfill_l_f_xor(p,v,c)  repfill_l_f_xor(p,freplicate_l(v),c)

#define optfill_b_f_or(p,v,c)   repfill_b_f_or(p,freplicate_b(v),c)
#define optfill_w_f_or(p,v,c)   repfill_w_f_or(p,freplicate_w(v),c)
#define optfill_l_f_or(p,v,c)   repfill_l_f_or(p,freplicate_l(v),c)

#define optfill_b_f_and(p,v,c)  repfill_b_f_and(p,freplicate_b(v),c)
#define optfill_w_f_and(p,v,c)  repfill_w_f_and(p,freplicate_w(v),c)
#define optfill_l_f_and(p,v,c)  repfill_l_f_and(p,freplicate_l(v),c)

/*
 * A set of optimized fills which preserves the address and count arguments
 * and performs small constant sized fills unrolled if the compiler supports
 * this (e.g. GCC)
 */

#ifndef __INLINE_MEMFILL__
#define __INLINE_MEMFILL__(P,V,C,SIZE,TYPE,FMODE) do {                  \
	register void     *_FP = (void *)(P);                           \
	register GR_repl   _FV = freplicate_##SIZE((TYPE)(V));          \
	register GR_PtrInt _FC = (GR_PtrInt)(C);                        \
	repfill_##SIZE##FMODE(_FP,_FV,_FC);                             \
} while(0)
#endif

/*
 * memfill_<SIZE>[_<FAR>][_<OPER>](pointer,value,count)
 */
#define memfill_b(p,v,c)        __INLINE_MEMFILL__(p,v,c,b,GR_int8,_n)
#define memfill_w(p,v,c)        __INLINE_MEMFILL__(p,v,c,w,GR_int16,_n)
#define memfill_l(p,v,c)        __INLINE_MEMFILL__(p,v,c,l,GR_int32,_n)

#define memfill_b_xor(p,v,c)    __INLINE_MEMFILL__(p,v,c,b,GR_int8,_xor)
#define memfill_w_xor(p,v,c)    __INLINE_MEMFILL__(p,v,c,w,GR_int16,_xor)
#define memfill_l_xor(p,v,c)    __INLINE_MEMFILL__(p,v,c,l,GR_int32,_xor)

#define memfill_b_or(p,v,c)     __INLINE_MEMFILL__(p,v,c,b,GR_int8,_or)
#define memfill_w_or(p,v,c)     __INLINE_MEMFILL__(p,v,c,w,GR_int16,_or)
#define memfill_l_or(p,v,c)     __INLINE_MEMFILL__(p,v,c,l,GR_int32,_or)

#define memfill_b_and(p,v,c)    __INLINE_MEMFILL__(p,v,c,b,GR_int8,_and)
#define memfill_w_and(p,v,c)    __INLINE_MEMFILL__(p,v,c,w,GR_int16,_and)
#define memfill_l_and(p,v,c)    __INLINE_MEMFILL__(p,v,c,l,GR_int32,_and)

#define memfill_b_f(p,v,c)      __INLINE_MEMFILL__(p,v,c,b,GR_int8,_f)
#define memfill_w_f(p,v,c)      __INLINE_MEMFILL__(p,v,c,w,GR_int16,_f)
#define memfill_l_f(p,v,c)      __INLINE_MEMFILL__(p,v,c,l,GR_int32,_f)

#define memfill_b_f_xor(p,v,c)  __INLINE_MEMFILL__(p,v,c,b,GR_int8,_f_xor)
#define memfill_w_f_xor(p,v,c)  __INLINE_MEMFILL__(p,v,c,w,GR_int16,_f_xor)
#define memfill_l_f_xor(p,v,c)  __INLINE_MEMFILL__(p,v,c,l,GR_int32,_f_xor)

#define memfill_b_f_or(p,v,c)   __INLINE_MEMFILL__(p,v,c,b,GR_int8,_f_or)
#define memfill_w_f_or(p,v,c)   __INLINE_MEMFILL__(p,v,c,w,GR_int16,_f_or)
#define memfill_l_f_or(p,v,c)   __INLINE_MEMFILL__(p,v,c,l,GR_int32,_f_or)

#define memfill_b_f_and(p,v,c)  __INLINE_MEMFILL__(p,v,c,b,GR_int8,_f_and)
#define memfill_w_f_and(p,v,c)  __INLINE_MEMFILL__(p,v,c,w,GR_int16,_f_and)
#define memfill_l_f_and(p,v,c)  __INLINE_MEMFILL__(p,v,c,l,GR_int32,_f_and)

/*
 * 24bpp special support functions
 */

#ifndef __INLINE_24_REPFILL__
#define __INLINE_24_REPFILL__(P,C,B,FMODE,INS) do {                \
    GR_int32u _cl24_ = (GR_int32u)(C);                             \
    GR_PtrInt _b24_ = (GR_PtrInt)(B);                                          \
    while ( _b24_ >= 3) {                                          \
	poke_24##FMODE((P), _cl24_);                               \
	ptrinc((P),3);                                             \
	_b24_ -= 3;                                                \
    }                                                              \
    /* B = 0..2 */                                                 \
    if (! _b24_ ) break;                                           \
    poke_b##FMODE((P), (GR_int8u)_cl24_);                          \
    ptrinc((P),1);                                                 \
    if (! --_b24_ ) break;                                         \
    poke_b##FMODE((P), (GR_int8u)(_cl24_>>8));                     \
    ptrinc((P),1);                                                 \
} while (0)
#endif

#ifndef __INLINE_24_FAR_REPFILL__
#define __INLINE_24_FAR_REPFILL__(P,C,B,FMODE,INS) \
	__INLINE_24_REPFILL__(P,C,B,FMODE,INS)
#endif

#ifndef repfill_24_set
#define repfill_24_set(p,c,b)   __INLINE_24_REPFILL__(p,c,b,_set,MOV_INS)
#endif
#ifndef repfill_24_xor
#define repfill_24_xor(p,c,b)   __INLINE_24_REPFILL__(p,c,b,_xor,XOR_INS)
#endif
#ifndef repfill_24_or
#define repfill_24_or(p,c,b)    __INLINE_24_REPFILL__(p,c,b,_or,OR_INS)
#endif
#ifndef repfill_24_and
#define repfill_24_and(p,c,b)   __INLINE_24_REPFILL__(p,c,b,_and,AND_INS)
#endif

#ifndef repfill_24_f_set
#define repfill_24_f_set(p,c,b) __INLINE_24_FAR_REPFILL__(p,c,b,_f_set,MOV_INS)
#endif
#ifndef repfill_24_f_xor
#define repfill_24_f_xor(p,c,b) __INLINE_24_FAR_REPFILL__(p,c,b,_f_xor,XOR_INS)
#endif
#ifndef repfill_24_f_or
#define repfill_24_f_or(p,c,b)  __INLINE_24_FAR_REPFILL__(p,c,b,_f_or,OR_INS)
#endif
#ifndef repfill_24_f_and
#define repfill_24_f_and(p,c,b) __INLINE_24_FAR_REPFILL__(p,c,b,_f_and,AND_INS)
#endif

#ifndef __INLINE_24_MEMFILL
#define __INLINE_24_MEMFILL__(p,c,b,FMODE) do {                    \
    void *_p24 = (p);                                              \
    unsigned long _c24 = (c);                                      \
    unsigned _b24 = (b);                                           \
    repfill_24##FMODE(_p24,_c24,_b24);                             \
} while (0)
#endif

#ifndef memfill_24_set
#define memfill_24_set(p,c,b)   __INLINE_24_MEMFILL__(p,c,b,_set)
#endif
#ifndef memfill_24_xor
#define memfill_24_xor(p,c,b)   __INLINE_24_MEMFILL__(p,c,b,_xor)
#endif
#ifndef memfill_24_or
#define memfill_24_or(p,c,b)    __INLINE_24_MEMFILL__(p,c,b,_or)
#endif
#ifndef memfill_24_and
#define memfill_24_and(p,c,b)   __INLINE_24_MEMFILL__(p,c,b,_and)
#endif

#ifndef memfill_24_set_f
#define memfill_24_set_f(p,c,b) __INLINE_24_MEMFILL__(p,c,b,_set_f)
#endif
#ifndef memfill_24_xor_f
#define memfill_24_xor_f(p,c,b) __INLINE_24_MEMFILL__(p,c,b,_xor_f)
#endif
#ifndef memfill_24_or_f
#define memfill_24_or_f(p,c,b)  __INLINE_24_MEMFILL__(p,c,b,_or_f)
#endif
#ifndef memfill_24_and_f
#define memfill_24_and_f(p,c,b) __INLINE_24_MEMFILL__(p,c,b,_and_f)
#endif

#define memfill_24   memfill_24_set
#define memfill_24_f memfill_24_set_f


/*
 * stuff to clear arrays, structures
 */
/*
#ifdef  __TURBOC__
#define memzero(p,s)            memfill_w((p),0,((s) >> 1))
#endif
#ifndef memzero
#define memzero(p,s)            memfill_l((p),0,((s) >> 2))
#endif
*/
#define memzero(p,s) do {                                               \
	register void far *_FP = (void far *)(p);                       \
	register GR_repl   _FV = 0;                                     \
	register unsigned  _FC = (unsigned)(s);                         \
        DBGPRINTF(DBG_COPYFILL,("memzero size=%u\n",_FC));              \
	repfill_b(_FP,_FV,_FC);                                         \
} while(0)


#define sttzero(p)              memzero((p),sizeof(*(p)))

#if (defined(__WATCOMC__) && !defined(__386__))
/*
 * These are not really here!
 */
#undef  rowfill_l
#undef  rowfill_l_xor
#undef  rowfill_l_or
#undef  rowfill_l_and
#undef  rowfill_l_f
#undef  rowfill_l_f_xor
#undef  rowfill_l_f_or
#undef  rowfill_l_f_and

#undef  repfill_l
#undef  repfill_l_xor
#undef  repfill_l_or
#undef  repfill_l_and
#undef  repfill_l_f
#undef  repfill_l_f_xor
#undef  repfill_l_f_or
#undef  repfill_l_f_and

#undef  optfill_l
#undef  optfill_l_xor
#undef  optfill_l_or
#undef  optfill_l_and
#undef  optfill_l_f
#undef  optfill_l_f_xor
#undef  optfill_l_f_or
#undef  optfill_l_f_and

#undef  memfill_l
#undef  memfill_l_xor
#undef  memfill_l_or
#undef  memfill_l_and
#undef  memfill_l_f
#undef  memfill_l_f_xor
#undef  memfill_l_f_or
#undef  memfill_l_f_and

#undef  colfill_l
#undef  colfill_l_xor
#undef  colfill_l_or
#undef  colfill_l_and
#undef  colfill_l_f
#undef  colfill_l_f_xor
#undef  colfill_l_f_or
#undef  colfill_l_f_and

#endif

#endif  /* whole file */
