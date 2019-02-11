/**
 ** arith.h ---- some common integer arithmetic macros/inline functions
 **              Special Borland-C++ handling
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

/*
 * arithmetic stuff
 */

/* prototype for __emit__() */
#include <dos.h>

/*
 * [i|u]mul32(x,y)
 * multiply two int-s for a long result
 */
extern long _GR_imul32(int x, int y);
extern unsigned long _GR_umul32(int x, int y);
#define imul32(X,Y) _GR_imul32((X),(Y))
#define umul32(X,Y) _GR_umul32((X),(Y))

/*
 * umuladd32(x,y,z)
 * multiply two unsigned-s for a long result and add an unsigned
 */
extern unsigned long _GR_umuladd32(unsigned x, unsigned y, unsigned z);
#define umuladd32(X,Y,Z) _GR_umuladd32((X),(Y),(Z))

/*
 * [i|u]scale(X,N,D)
 * scale an integer with long intermediate result but without using long
 * arithmetic all the way
 */
extern int _GR_iscale(int x,int n,int d);
extern unsigned _GR_uscale(int x,int n,int d);
extern int _GR_irscale(int x, int n, int d);
#define iscale(X,N,D) _GR_iscale((X),(N),(D))
#define uscale(X,N,D) _GR_uscale((X),(N),(D))
#define irscale(X,N,D) _GR_irscale((X),(N),(D))

/*
 * replicate_<FROM>2<TO>(byte_or_word_value)
 * copy the lower byte(s) of a byte or word into the upper byte(s)
 */
#define replicate_b2w(BYTE) (                                   \
	_AL = (char)(BYTE),                                     \
	_AH = _AL,                                              \
	(int)_AX                                                \
)
#define replicate_w2l(WORD) (                                   \
	_AX = (int)(WORD),                                      \
	_DX = _AX,                                              \
	(long)((void _seg *)_DX + (void near *)_AX)             \
)
#define replicate_b2l(BYTE) (                                   \
	_AL = (char)(BYTE),                                     \
	_AH = _AL,                                              \
	_DX = _AX,                                              \
	(long)((void _seg *)_DX + (void near *)_AX)             \
)
