/**
 ** allocate.h ---- common ground for malloc & friends in 16 & 32 bit envs
 **                 stack based temporary memory allocation
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
 * I have TC++ 1.01 (quite old). It is possible that newer TC++ versions
 * have a built-in alloca.
 */
#if defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)
extern unsigned int __brklvl;
#define alloca(size) (                                                      \
	((_AX = _SP - (((unsigned int)(size) + 3) & ~3)) > __brklvl) ?      \
	(void near *)(_SP = _AX) :                                          \
	(void near *)(0)                                                    \
)
#else
extern unsigned int _stklen;
#define alloca(size) (                                                      \
	((_AX = _SP - (((unsigned int)(size) + 3) & ~3)) < _stklen) ?       \
	(void far *)((void _seg *)(_SS) + (void near *)(_SP = _AX)) :       \
	(void far *)(0)                                                     \
)
#endif

#define setup_alloca() { unsigned int __saved_SP__ = _SP;
#define reset_alloca() _SP = __saved_SP__; }

