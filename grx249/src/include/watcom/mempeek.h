/**
 ** mempeek.h ---- (far) memory read/write operations
 **                Watcom-C++ code
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
 **
 **/

/* candidate for speed ups */
#define __INLINE_STD_PEEK__(P,S,T)        (*((unsigned T *)(P)))
#define __INLINE_STD_POKE__(P,V,OP,I,S,T) ((*((unsigned T *)(P))) OP (V))

#define __INLINE_FAR_PEEK__(P,S,T)        (*((volatile unsigned T *)(P)))
#define __INLINE_FAR_POKE__(P,V,OP,I,S,T) ((*((volatile unsigned T *)(P))) OP (V))
