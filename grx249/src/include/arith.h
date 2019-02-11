/**
 ** arith.h ---- some common integer arithmetic macros/inline functions
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
 ** Intel CPU specific support is provided for the Turbo C and GNU C. May
 ** work with other compilers and CPU-s, but is not optimized for them.
 **
 **/

#ifndef __ARITH_H_INCLUDED__
#define __ARITH_H_INCLUDED__

#ifdef __GNUC__
#include "gcc/arith.h"
#elif defined(__TURBOC__)
#include "bcc/arith.h"
#endif

#ifdef _MSC_VER
#define __emit__(x) __asm{ __emit (x) }
#endif

/*
 * old standbys
 */
#ifndef min
#define min(x,y)    (((x) < (y)) ?  (x) : (y))
#endif
#ifndef max
#define max(x,y)    (((x) > (y)) ?  (x) : (y))
#endif
#ifndef __ABS
#define __ABS(x)    (((x) < (0)) ? -(x) : (x))
#endif
#ifndef abs
#define abs(x)      __ABS(x)
#endif
#ifndef scale
#define scale(x,n,d)    (((x) * (n)) / (d))
#endif
#ifndef imin
#define imin(x,y)   min((int)(x),(int)(y))
#endif
#ifndef imax
#define imax(x,y)   max((int)(x),(int)(y))
#endif
#ifndef iabs
#define iabs(x)     abs((int)(x))
#endif
#ifndef umin
#define umin(x,y)   min((unsigned int)(x),(unsigned int)(y))
#endif
#ifndef umax
#define umax(x,y)   max((unsigned int)(x),(unsigned int)(y))
#endif
#ifndef lmin
#define lmin(x,y)   min((long)(x),(long)(y))
#endif
#ifndef lmax
#define lmax(x,y)   max((long)(x),(long)(y))
#endif
#ifndef labs
#define labs(x)     __ABS((long)(x))
#endif
#ifndef ulmin
#define ulmin(x,y)  min((unsigned long)(x),(unsigned long)(y))
#endif
#ifndef ulmax
#define ulmax(x,y)  max((unsigned long)(x),(unsigned long)(y))
#endif

/*
 * swap and sort stuff
 */
#define iswap(x,y) {                                            \
	int _swap_tmpval_ = (x);                                \
	(x) = (y);                                              \
	(y) = _swap_tmpval_;                                    \
}

#define lswap(x,y) {                                            \
	long _swap_tmpval_ = (x);                               \
	(x) = (y);                                              \
	(y) = _swap_tmpval_;                                    \
}

#define isort(x,y) {                                            \
	if((int)(x) > (int)(y)) iswap(x,y)                      \
}

#define usort(x,y) {                                            \
	if((unsigned int)(x) > (unsigned int)(y)) iswap(x,y)    \
}

#define lsort(x,y) {                                            \
	if((long)(x) > (long)(y)) lswap(x,y)                    \
}

#define ulsort(x,y) {                                           \
	if((unsigned long)(x) > (unsigned long)(y)) lswap(x,y)  \
}


/*
 * couple of 'sizeof'-like useful macros
 */
#ifndef bsizeof
#define bsizeof(s)  (sizeof(s) / sizeof(char))
#endif
#ifndef wsizeof
#define wsizeof(s)  (sizeof(s) / sizeof(short))
#endif
#ifndef lsizeof
#define lsizeof(s)  (sizeof(s) / sizeof(long))
#endif
#ifndef bitsof
#define bitsof(s)   (sizeof(s) * 8)
#endif
#ifndef bytesof
#define bytesof(s)  ((sizeof(s) + sizeof(char) - 1) / sizeof(char))
#endif
#ifndef wordsof
#define wordsof(s)  ((sizeof(s) + sizeof(short) - 1) / sizeof(short))
#endif
#ifndef longsof
#define longsof(s)  ((sizeof(s) + sizeof(long) - 1) / sizeof(long))
#endif
#ifndef itemsof
#define itemsof(arr)    (sizeof(arr) / sizeof((arr)[0]))
#endif
#ifndef offsetof
#define offsetof(structype,field) (int)(                        \
	(char *)(&((structype *)(0))->field) -                  \
	(char *)(0)                                             \
)
#endif

/*
 * [i|u]mul32(x,y)
 * multiply two int-s for a long result
 */
#ifndef imul32
#define imul32(X,Y) ((long)(X) * (long)(Y))
#endif
#ifndef umul32
#define umul32(X,Y) ((unsigned long)(X) * (unsigned long)(Y))
#endif


/*
 * umuladd32(x,y,z)
 * multiply two unsigned-s for a long result and add an unsigned
 */
#ifndef umuladd32
#define umuladd32(X,Y,Z) (umul32((X),(Y))+(Z))
#endif


/*
 * [i|u]scale(X,N,D)
 * scale an integer with long intermediate result but without using long
 * arithmetic all the way
 */
#ifndef iscale
#define iscale(X,N,D)   (int)(imul32(X,N) / (long)(D))
#endif
#ifndef uscale
#define uscale(X,N,D)   (unsigned int)(umul32(X,N) / (unsigned long)(D))
#endif
#ifndef irscale
#define irscale(X,N,D)  ((                                              \
	iscale(((int)(X) << 1),N,D) +                                   \
	(((int)(X) ^ (int)(N) ^ (int)(D)) >> (bitsof(int) - 1)) +       \
	1                                                               \
) >> 1)
#endif
#ifndef urscale
#define urscale(X,N,D)  ((uscale(((int)(X) << 1),N,D) + 1) >> 1)
#endif

/*
 * replicate_<FROM>2<TO>(byte_or_word_value)
 * copy the lower byte(s) of a byte or word into the upper byte(s)
 */
#define __INLINE_REPLICATE__(V,TFROM,TTO) (                             \
	((unsigned TTO)(unsigned TFROM)(V)) |                           \
	((unsigned TTO)(unsigned TFROM)(V) << (sizeof(TFROM) * 8))      \
)

#ifndef replicate_b2b
#define replicate_b2b(BYTE) (BYTE)
#endif
#ifndef replicate_b2w
#define replicate_b2w(BYTE) __INLINE_REPLICATE__(BYTE,GR_int8,GR_int16)
#endif
#ifndef replicate_b2l
#define replicate_b2l(BYTE) replicate_w2l(replicate_b2w(BYTE))
#endif
#ifndef replicate_b2h
#define replicate_b2h(BYTE) replicate_l2h(replicate_w2l(replicate_b2w(BYTE)))
#endif

#ifndef replicate_w2w
#define replicate_w2w(WORD) (WORD)
#endif
#ifndef replicate_w2l
#define replicate_w2l(WORD) __INLINE_REPLICATE__(WORD,GR_int16,GR_int32)
#endif
#ifndef replicate_w2h
#define replicate_w2h(WORD) replicate_l2h(replicate_w2l(WORD))
#endif

#ifndef replicate_l2l
#define replicate_l2l(LONG) (LONG)
#endif
#ifndef replicate_l2h
#define replicate_l2h(LONG) __INLINE_REPLICATE__(LONG,GR_int32,GR_int64)
#endif

#ifndef replicate_h2h
#define replicate_h2h(LLONG) (LLONG)
#endif

#endif  /* whole file */

