/**
 ** mempeek.h ---- (far) memory read/write operations
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
 ** Basic memory peek and poke operations in byte, word and long sizes.
 ** The poke operations are available in WRITE, XOR, OR and AND versions.
 ** Intel CPU specific support is provided for the Turbo C and GNU C
 ** compilers. The i386 GCC version supports segment overrides. May
 ** work with other compilers and CPU-s, but is not optimized for them.
 **
 **/

#ifndef __MEMPEEK_H_INCLUDED__
#define __MEMPEEK_H_INCLUDED__

#ifndef __MEMMODE_H_INCLUDED__
#include "memmode.h"
#endif

#ifdef __GNUC__
#include "gcc/mempeek.h"
#elif defined(__TURBOC__)
#include "bcc/mempeek.h"
#elif defined(__WATCOMC__)
#include "watcom/mempeek.h"
#endif

#ifndef __INLINE_STD_PEEK__
#define __INLINE_STD_PEEK__(P,S,T)        (*(unsigned T *)(P))
#endif

#ifndef __INLINE_STD_POKE__
# ifdef NO_LEFTSIDE_PTR_CAST
#  define __INLINE_STD_POKE__(P,V,OP,I,S,T) do {                \
              register unsigned T *_ISPptr = (void *)(P);       \
              *_ISPptr OP (unsigned T)(V);                      \
	  } while (0)
# else
#  define __INLINE_STD_POKE__(P,V,OP,I,S,T) (*(unsigned T *)(P) OP (unsigned T)(V))
# endif
#endif

/* the volatile modifier ensures the video ram access is really done */
#ifndef __INLINE_FAR_PEEK__
#define __INLINE_FAR_PEEK__(P,S,T)        (*(volatile unsigned T *)(P))
#endif

#ifndef __INLINE_FAR_POKE__
# ifdef NO_LEFTSIDE_PTR_CAST
#  define __INLINE_FAR_POKE__(P,V,OP,I,S,T) do {                \
              register unsigned T *_ISPptr = (void *)(P);       \
              *_ISPptr OP (V);                                  \
	  } while (0)
# else
#  define __INLINE_FAR_POKE__(P,V,OP,I,S,T) (*(volatile unsigned T *)(P) OP (V))
# endif
#endif


/*
 * setup_far_selector(segment_selector)
 */
#ifndef setup_far_selector
#define setup_far_selector(S)
#endif

/*
 * peek_<SIZE>... (pointer)
 * poke_<SIZE>... (pointer,value)
 */

/* some processors (eg. DEC alpha) need special handling for word access */
#ifndef MISALIGNED_16bit_OK
#define peek_w(p) ( ((GR_int16u)(*((GR_int8u *)(p)  ))   )               \
		  | ((GR_int16u)(*((GR_int8u *)(p)+1))<<8) )
#define __SPLIT_16_POKE__(P,V,OP) ( (*((GR_int8u *)(P)  ) OP ((V)   )),  \
				    (*((GR_int8u *)(P)+1) OP ((V)>>8)) )
#define poke_w(p,v)       __SPLIT_16_POKE__(p,v,=)
#define poke_w_xor(p,v)   __SPLIT_16_POKE__(p,v,^=)
#define poke_w_or(p,v)    __SPLIT_16_POKE__(p,v,|=)
#define poke_w_and(p,v)   __SPLIT_16_POKE__(p,v,&=)
#define peek_w_f(p)       peek_w(p)
#define poke_w_f(p,v)     poke_w((p),(v))
#define poke_w_f_xor(p,v) poke_w_xor((p),(v))
#define poke_w_f_or(p,v)  poke_w_or((p),(v))
#define poke_w_f_and(p,v) poke_w_and((p),(v))
#endif


/* ------------------------------------- near memory (RAM) access */
#ifndef peek_b
#define peek_b(p)           __INLINE_STD_PEEK__(p,OP8b,GR_int8)
#endif
#ifndef peek_w
#define peek_w(p)           __INLINE_STD_PEEK__(p,OP16b,GR_int16)
#endif
#ifndef peek_l
#define peek_l(p)           __INLINE_STD_PEEK__(p,OP32b,GR_int32)
#endif
#if defined(GR_int64) && !defined(peek_h)
#define peek_h(p)           __INLINE_STD_PEEK__(p,OP64b,GR_int64)
#endif

#ifndef poke_b
#define poke_b(p,v)         __INLINE_STD_POKE__(p,v,=,MOV_INS,OP8b,GR_int8)
#endif
#ifndef poke_w
#define poke_w(p,v)         __INLINE_STD_POKE__(p,v,=,MOV_INS,OP16b,GR_int16)
#endif
#ifndef poke_l
#define poke_l(p,v)         __INLINE_STD_POKE__(p,v,=,MOV_INS,OP32b,GR_int32)
#endif
#if defined(GR_int64) && !defined(poke_h)
#define poke_h(p,v)         __INLINE_STD_POKE__(p,v,=,MOV_INS,OP64b,GR_int64)
#endif

#define poke_b_set poke_b
#define poke_w_set poke_w
#define poke_l_set poke_l
#ifdef poke_h
#define poke_h_set poke_h
#endif

#ifndef poke_b_xor
#define poke_b_xor(p,v)     __INLINE_STD_POKE__(p,v,^=,XOR_INS,OP8b,GR_int8)
#endif
#ifndef poke_w_xor
#define poke_w_xor(p,v)     __INLINE_STD_POKE__(p,v,^=,XOR_INS,OP16b,GR_int16)
#endif
#ifndef poke_l_xor
#define poke_l_xor(p,v)     __INLINE_STD_POKE__(p,v,^=,XOR_INS,OP32b,GR_int32)
#endif
#if defined(GR_int64) && !defined(poke_h_xor)
#define poke_h_xor(p,v)     __INLINE_STD_POKE__(p,v,^=,XOR_INS,OP64b,GR_int64)
#endif

#ifndef poke_b_or
#define poke_b_or(p,v)      __INLINE_STD_POKE__(p,v,|=,OR_INS,OP8b,GR_int8)
#endif
#ifndef poke_w_or
#define poke_w_or(p,v)      __INLINE_STD_POKE__(p,v,|=,OR_INS,OP16b,GR_int16)
#endif
#ifndef poke_l_or
#define poke_l_or(p,v)      __INLINE_STD_POKE__(p,v,|=,OR_INS,OP32b,GR_int32)
#endif
#if defined(GR_int64) && !defined(poke_h_or)
#define poke_h_or(p,v)      __INLINE_STD_POKE__(p,v,|=,OR_INS,OP64b,GR_int64)
#endif

#ifndef poke_b_and
#define poke_b_and(p,v)     __INLINE_STD_POKE__(p,v,&=,AND_INS,OP8b,GR_int8)
#endif
#ifndef poke_w_and
#define poke_w_and(p,v)     __INLINE_STD_POKE__(p,v,&=,AND_INS,OP16b,GR_int16)
#endif
#ifndef poke_l_and
#define poke_l_and(p,v)     __INLINE_STD_POKE__(p,v,&=,AND_INS,OP32b,GR_int32)
#endif
#if defined(GR_int64) && !defined(poke_h_and)
#define poke_h_and(p,v)     __INLINE_STD_POKE__(p,v,&=,AND_INS,OP64b,GR_int64)
#endif

/* ------------------------------------- far memory (video) access */
#ifndef peek_b_f
#define peek_b_f(p)         __INLINE_FAR_PEEK__(p,OP8b,GR_int8)
#endif
#ifndef peek_w_f
#define peek_w_f(p)         __INLINE_FAR_PEEK__(p,OP16b,GR_int16)
#endif
#ifndef peek_l_f
#define peek_l_f(p)         __INLINE_FAR_PEEK__(p,OP32b,GR_int32)
#endif
#if defined(GR_int64) && !defined(peek_h_f)
#define peek_h_f(p)         __INLINE_FAR_PEEK__(p,OP64b,GR_int64)
#endif

#ifndef poke_b_f
#define poke_b_f(p,v)       __INLINE_FAR_POKE__(p,v,=,MOV_INS,OP8b,GR_int8)
#endif
#ifndef poke_w_f
#define poke_w_f(p,v)       __INLINE_FAR_POKE__(p,v,=,MOV_INS,OP16b,GR_int16)
#endif
#ifndef poke_l_f
#define poke_l_f(p,v)       __INLINE_FAR_POKE__(p,v,=,MOV_INS,OP32b,GR_int32)
#endif
#if defined(GR_int64) && !defined(poke_h_f)
#define poke_h_f(p,v)       __INLINE_FAR_POKE__(p,v,=,MOV_INS,OP64b,GR_int64)
#endif

#define poke_b_f_set poke_b_f
#define poke_w_f_set poke_w_f
#define poke_l_f_set poke_l_f
#ifdef poke_h_f
#define poke_h_f_set poke_h_f
#endif

#ifndef poke_b_f_xor
#define poke_b_f_xor(p,v)   __INLINE_FAR_POKE__(p,v,^=,XOR_INS,OP8b,GR_int8)
#endif
#ifndef poke_w_f_xor
#define poke_w_f_xor(p,v)   __INLINE_FAR_POKE__(p,v,^=,XOR_INS,OP16b,GR_int16)
#endif
#ifndef poke_l_f_xor
#define poke_l_f_xor(p,v)   __INLINE_FAR_POKE__(p,v,^=,XOR_INS,OP32b,GR_int32)
#endif
#if defined(GR_int64) && !defined(poke_h_f_xor)
#define poke_h_f_xor(p,v)   __INLINE_FAR_POKE__(p,v,^=,XOR_INS,OP64b,GR_int64)
#endif

#ifndef poke_b_f_or
#define poke_b_f_or(p,v)    __INLINE_FAR_POKE__(p,v,|=,OR_INS,OP8b,GR_int8)
#endif
#ifndef poke_w_f_or
#define poke_w_f_or(p,v)    __INLINE_FAR_POKE__(p,v,|=,OR_INS,OP16b,GR_int16)
#endif
#ifndef poke_l_f_or
#define poke_l_f_or(p,v)    __INLINE_FAR_POKE__(p,v,|=,OR_INS,OP32b,GR_int32)
#endif
#if defined(GR_int64) && !defined(poke_h_f_or)
#define poke_h_f_or(p,v)    __INLINE_FAR_POKE__(p,v,|=,OR_INS,OP64b,GR_int64)
#endif

#ifndef poke_b_f_and
#define poke_b_f_and(p,v)   __INLINE_FAR_POKE__(p,v,&=,AND_INS,OP8b,GR_int8)
#endif
#ifndef poke_w_f_and
#define poke_w_f_and(p,v)   __INLINE_FAR_POKE__(p,v,&=,AND_INS,OP16b,GR_int16)
#endif
#ifndef poke_l_f_and
#define poke_l_f_and(p,v)   __INLINE_FAR_POKE__(p,v,&=,AND_INS,OP32b,GR_int32)
#endif
#if defined(GR_int64) && !defined(poke_h_f_and)
#define poke_h_f_and(p,v)   __INLINE_FAR_POKE__(p,v,&=,AND_INS,OP64b,GR_int64)
#endif

/* ------------------------------------------- special 24bpp handling --- */

#define __INTERN_24_PEEK__(P,F)                                             \
	 ( peek_b##F(P)                                                     \
	 | ((GR_int16u)peek_b##F(((GR_int8 *)(P))+1)<<8)                    \
	 | ((GR_int32u)peek_b##F(((GR_int8 *)(P))+2)<<16))

#define __INTERN_24_POKE__(P,C,F,OP) do {                                   \
		poke_b##F##OP((P),(GR_int8)(C));                            \
		poke_b##F##OP(((GR_int8 *)(P))+1,(GR_int8)((C)>>8));        \
		poke_b##F##OP(((GR_int8 *)(P))+2,(GR_int8)((C)>>16));       \
	} while (0)

#ifndef __INLINE_24_PEEK__
#define __INLINE_24_PEEK__(p) \
	__INTERN_24_PEEK__(p,_n)
#endif
#ifndef __INLINE_24_FAR_PEEK__
#define __INLINE_24_FAR_PEEK__(p) \
	__INTERN_24_PEEK__(p,_f)
#endif

#ifndef __INLINE_24_POKE__
#define __INLINE_24_POKE__(p,c,op,INS) \
	__INTERN_24_POKE__(p,c,_n,op)
#endif

#ifndef __INLINE_24_FAR_POKE__
#define __INLINE_24_FAR_POKE__(p,c,op,INS) \
	__INTERN_24_POKE__(p,c,_f,op)
#endif

#ifndef peek_24
#define peek_24(p)         __INLINE_24_PEEK__(p)
#endif
#ifndef peek_24_f
#define peek_24_f(p)       __INLINE_24_FAR_PEEK__(p)
#endif

#ifndef poke_24_set
#define poke_24_set(p,c)   __INLINE_24_POKE__(p,c,_set,MOV_INS)
#endif
#ifndef poke_24_xor
#define poke_24_xor(p,c)   __INLINE_24_POKE__(p,c,_xor,XOR_INS)
#endif
#ifndef poke_24_or
#define poke_24_or(p,c)    __INLINE_24_POKE__(p,c,_or,OR_INS)
#endif
#ifndef poke_24_and
#define poke_24_and(p,c)   __INLINE_24_POKE__(p,c,_and,AND_INS)
#endif
#define poke_24 poke_24_set

#ifndef poke_24_f_set
#define poke_24_f_set(p,c)   __INLINE_24_FAR_POKE__(p,c,_set,MOV_INS)
#endif
#ifndef poke_24_f_xor
#define poke_24_f_xor(p,c)   __INLINE_24_FAR_POKE__(p,c,_xor,XOR_INS)
#endif
#ifndef poke_24_f_or
#define poke_24_f_or(p,c)    __INLINE_24_FAR_POKE__(p,c,_or,OR_INS)
#endif
#ifndef poke_24_f_and
#define poke_24_f_and(p,c)   __INLINE_24_FAR_POKE__(p,c,_and,AND_INS)
#endif
#define poke_24_f poke_24_f_set

/* ..._n is used in some makros to keep the preprocessor happy.
** Mapped to standard near memory commands :                    */
#define peek_b_n          peek_b
#define peek_w_n          peek_w
#define peek_l_n          peek_l
#define poke_b_n          poke_b
#define poke_w_n          poke_w
#define poke_l_n          poke_l
#define poke_b_n_set      poke_b
#define poke_w_n_set      poke_w
#define poke_l_n_set      poke_l
#define poke_b_n_xor      poke_b_xor
#define poke_w_n_xor      poke_w_xor
#define poke_l_n_xor      poke_l_xor
#define poke_b_n_or       poke_b_or
#define poke_w_n_or       poke_w_or
#define poke_l_n_or       poke_l_or
#define poke_b_n_and      poke_b_and
#define poke_w_n_and      poke_w_and
#define poke_l_n_and      poke_l_and
#ifdef poke_h
#define peek_h_n          peek_h
#define poke_h_n          poke_h
#define poke_h_n_set      poke_h
#define poke_h_n_xor      poke_h_xor
#define poke_h_n_or       poke_h_or
#define poke_h_n_and      poke_h_and
#endif

#define peek_24_n         peek_24
#define poke_24_n         poke_24_set
#define poke_24_n_set     poke_24_set
#define poke_24_n_xor     poke_24_xor
#define poke_24_n_or      poke_24_or
#define poke_24_n_and     poke_24_and

#endif  /* whole file */

