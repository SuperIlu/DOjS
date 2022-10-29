/**
 ** allocate.h ---- some allocate functions
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

#if defined(__MINGW32__) && !defined(alloca)
#define alloca __builtin_alloca
#else
#include <stdlib.h>
#endif

#ifndef setup_alloca
#define setup_alloca()
#define reset_alloca()
#endif

/* ALLOC / FREE : use alloca if possible */
#define ALLOC(sze) alloca(sze)
#define FREE(p)
#define setup_ALLOC setup_alloca
#define reset_ALLOC reset_alloca

/* temp buffer for blits etc. */
extern void *_GrTempBuffer;
extern unsigned _GrTempBufferBytes;
#define _GrTempBufferAlloc(b) (((unsigned)(b) <= _GrTempBufferBytes) ? _GrTempBuffer : _GrTempBufferAlloc_(b))
extern void *_GrTempBufferAlloc_(size_t bytes);
extern void _GrTempBufferFree(void);
