/**
 ** bccarith.c ---- some common integer arithmetic functions
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

#include "libgrx.h"
#include "arith.h"

#ifdef __TURBOC__
/* prototype for __emit__() */
#include <dos.h>
#endif

#ifdef _MSC_VER
#define __emit__(x) __asm{ __emit (x) }
#endif

/*
 * [i|u]mul32(x,y)
 * multiply two int-s for a long result
 */
long _GR_imul32(int x, int y) {
	_DX = (int)(x);
	_AX = (int)(y);
	__emit__((char)(0xf7));               /* imul dx */
	__emit__((char)(0xea));
	return (long)((void _seg *)_DX + (void near *)_AX);
}

unsigned long _GR_umul32(int x, int y) {
	_DX = (int)(x);
	_AX = (int)(y);
	__emit__((char)(0xf7));               /* mul dx  */
	__emit__((char)(0xe2));
	return (unsigned long)((void _seg *)_DX + (void near *)_AX);
}

/*
 * umuladd32(x,y,z)
 * multiply two int-s for a long result and add
 */
unsigned long _GR_umuladd32(unsigned x, unsigned y, unsigned z) {
	_DX = x;
	_AX = y;
	_BX = z;
	__emit__((char)(0xf7));               /* mul dx  */
	__emit__((char)(0xe2));

	__emit__((char)(0x01));               /* add ax,bx */
	__emit__((char)(0xd8));

	__emit__((char)(0x83));               /* adc dx,0 */
	__emit__((char)(0xd2));
	__emit__((char)(0x00));
	return (unsigned long)((void _seg *)_DX + (void near *)_AX);
}

/*
 * [i|u]scale(X,N,D)
 * scale an integer with long intermediate result but without using long
 * arithmetic all the way
 */
int _GR_iscale(int x,int n,int d) {
	_CX = (int)(d);
	_DX = (int)(n);
	_AX = (int)(x);
	__emit__((char)(0xf7));                     /* imul dx */
	    __emit__((char)(0xea));
	__emit__((char)(0xf7));                     /* idiv cx */
	    __emit__((char)(0xf9));
	return (int)_AX;
}

unsigned int _GR_uscale(int x,int n,int d) {
	_CX = (int)(d);
	_DX = (int)(n);
	_AX = (int)(x);
	__emit__((char)(0xf7));                     /* mul dx */
	    __emit__((char)(0xe2));
	__emit__((char)(0xf7));                     /* div cx */
	    __emit__((char)(0xf1));
	return (unsigned int)_AX;
}

int _GR_irscale(int x, int n, int d) {
	_AX = (int)(x<<1);
	_CX = (int)(d);
	_DX = (int)(n);
	__emit__((char)(0xf7));                     /* imul dx */
	    __emit__((char)(0xea));
	__emit__((char)(0xf7));                     /* idiv cx */
	    __emit__((char)(0xf9));

	_DX = _AX;
	__emit__((char)(0x03));                       /* add dx,dx */
	    __emit__((char)(0xd2));
	__emit__((char)(0x1d));                       /* sbc ax,0xffff */
	    __emit__((char)(-1));
	    __emit__((char)(-1));
	__emit__((char)(0xd1));                       /* sar ax,1 */
	    __emit__((char)(0xf8));

	return (int)_AX;
}

#if 0
These are Csaba's inline macros.
The _?X register loading fails in some cases.

#define imul32(X,Y) (                                           \
	_AX = (int)(X),                                         \
	__emit__((char)(0x50)),               /* push ax */     \
	_AX = (int)(Y),                                         \
	__emit__((char)(0x5a)),               /* pop dx */      \
	__emit__((char)(0xf7)),               /* imul dx */     \
	    __emit__((char)(0xea)),                             \
	_BX = _AX,                                              \
	_CX = _DX,                                              \
	(long)((void _seg *)_CX + (void near *)_BX)             \
)
#define umul32(X,Y) (                                           \
	_AX = (int)(X),                                         \
	__emit__((char)(0x50)),               /* push ax */     \
	_AX = (int)(Y),                                         \
	__emit__((char)(0x5a)),               /* pop dx */      \
	__emit__((char)(0xf7)),               /* mul dx */      \
	    __emit__((char)(0xe2)),                             \
	_BX = _AX,                                              \
	_CX = _DX,                                              \
	(unsigned long)((void _seg *)_CX + (void near *)_BX)    \
)

#define iscale(X,N,D) (                                             \
	_AX = (int)(D),                                             \
	__emit__((char)(0x50)),                     /* push ax */   \
	_AX = (int)(N),                                             \
	__emit__((char)(0x50)),                     /* push ax */   \
	_AX = (int)(X),                                             \
	__emit__((char)(0x5a)),                     /* pop dx */    \
	__emit__((char)(0x59)),                     /* pop cx */    \
	__emit__((char)(0xf7)),                     /* imul dx */   \
	    __emit__((char)(0xea)),                                 \
	__emit__((char)(0xf7)),                     /* idiv cx */   \
	    __emit__((char)(0xf9)),                                 \
	(int)_AX                                                    \
)
#define uscale(X,N,D) (                                             \
	_AX = (int)(D),                                             \
	__emit__((char)(0x50)),                     /* push ax */   \
	_AX = (int)(N),                                             \
	__emit__((char)(0x50)),                     /* push ax */   \
	_AX = (int)(X),                                             \
	__emit__((char)(0x5a)),                     /* pop dx */    \
	__emit__((char)(0x59)),                     /* pop cx */    \
	__emit__((char)(0xf7)),                     /* mul dx */    \
	    __emit__((char)(0xe2)),                                 \
	__emit__((char)(0xf7)),                     /* div cx */    \
	    __emit__((char)(0xf1)),                                 \
	(unsigned int)_AX                                           \
)
#define irscale(X,N,D) (                                                  \
	_DX = iscale(((int)(X) << 1),N,D),                                \
	__emit__((char)(0x03)),                       /* add dx,dx */     \
	    __emit__((char)(0xd2)),                                       \
	__emit__((char)(0x1d)),                       /* sbc ax,0xffff */ \
	    __emit__((char)(-1)),                                         \
	    __emit__((char)(-1)),                                         \
	__emit__((char)(0xd1)),                       /* sar ax,1 */      \
	    __emit__((char)(0xf8)),                                       \
	(int)_AX                                                          \
)

#endif
