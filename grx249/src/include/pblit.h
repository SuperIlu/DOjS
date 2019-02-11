/**
 ** pblit.h ---- video->video; video->ram; ram->video blit support macros
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
 ** Contributions by: (See "doc/credits.doc" for details)
 ** Hartmut Schirmer (hsc@techfak.uni-kiel.de)
 **
 **/

#ifndef __PBLIT_H_INCLUDED__
#define __PBLIT_H_INCLUDED__

#ifndef __MEMCOPY_H_INCLUDED__
#include "memcopy.h"
#endif

/*
** single element (SIZE=b: 8bit, w:16bit, l:32bit) copy command.
** Source is 'sptr', destination 'dptr'. Read operation memory
** reference F is empty for standard memory access (near pointer)
** and '_f' for far pointer access. OP is the write command
** specifier: use _xor, _and, _or, _set and _image for near
** pointer writes and _f_xor, ... for far pointer destination.
**
** DO1COPY_FW: forward copy, pointers are incremented after
**             copy operation
** DO1COPY_RV: reverse copy, pointers are decremented before
**             copy operation
*/
#define DO1COPY_FW(SIZE,OP,F) do {                                   \
	 poke_##SIZE##OP((dptr),peek_##SIZE##F(sptr));               \
	 (dptr) += CPSIZE_##SIZE;                                    \
	 (sptr) += CPSIZE_##SIZE;                                    \
 } while(0)

#define DO1COPY_RV(SIZE,OP,F) do {                                   \
	 poke_##SIZE##OP((dptr),peek_##SIZE##F(sptr));               \
	 (dptr) -= CPSIZE_##SIZE;                                    \
	 (sptr) -= CPSIZE_##SIZE;                                    \
 } while(0)


/* special handling for image blits */
#define poke_b_image(p,v) do {                                           \
	 GR_int8u val = (v);                                             \
	 if(val != cval) poke_b(p,val);                                  \
 } while (0)

#define poke_b_f_image(p,v) do {                                         \
	 GR_int8u val = (v);                                             \
	 if(val != cval) poke_b_f(p,val);                                \
 } while (0)

#define poke_b_n_image  poke_b_image

#define __DOICOPY__(INC,WF,RF) do {                                      \
	 poke_b##WF##_image((dptr),peek_b##RF(sptr));                    \
	 (dptr) INC CPSIZE_b;                                            \
	 (sptr) INC CPSIZE_b;                                            \
 } while(0)

#define __DOICOPY_FW__(WF,RF) __DOICOPY__(+=,WF,RF)
#define __DOICOPY_RV__(WF,RF) __DOICOPY__(-=,WF,RF)
#define DOIMGCOPY(DIR,WF,RF,W) do __DOICOPY_##DIR##__(WF,RF); while(--(W) != 0)

#endif /* whole file */
