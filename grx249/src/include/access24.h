/**
 ** access24.h ---- 16M color (24bit) access macros
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
 **/

#ifndef __ACCESS24_H_INCLUDED__
#define __ACCESS24_H_INCLUDED__

#ifndef __LIBGRX_H_INCLUDED__
#include "libgrx.h"
#endif

#ifndef __MEMPEEK_H_INCLUDED__
#include "mempeek.h"
#endif

#if BYTE_ORDER==LITTLE_ENDIAN
/* read color component from 32bit variable */
#define RD24BYTE(p,idx)    peek_b(((GR_int8u *)(&p))+idx)
/* write color componet to 32bit variable */
#define WR24BYTE(p,idx,cc) poke_b(((GR_int8u *)(&p))+idx,cc)
#else
/* read color component from 32bit variable */
#define RD24BYTE(p,idx)    peek_b(((GR_int8u *)(&p))+idx+1)
/* write color componet to 32bit variable */
#define WR24BYTE(p,idx,cc) poke_b(((GR_int8u *)(&p))+idx+1,cc)
#endif

#endif
