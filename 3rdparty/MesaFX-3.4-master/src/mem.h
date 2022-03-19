/* $Id: mem.h,v 1.5 2000/06/27 22:10:00 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.3
 *
 * Copyright (C) 1999-2000  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef MEM_H
#define MEM_H


#include "glheader.h"


/*
 * Memory allocation
 */
extern void *_mesa_malloc(size_t bytes);
extern void *_mesa_calloc(size_t bytes);
extern void _mesa_free(void *ptr);

extern void *_mesa_align_malloc(size_t bytes, unsigned long alignment);
extern void *_mesa_align_calloc(size_t bytes, unsigned long alignment);
extern void _mesa_align_free(void *ptr);


#ifdef DEBUG

/* call Mesa memory functions */
#define MALLOC(BYTES)      _mesa_malloc(BYTES)
#define CALLOC(BYTES)      _mesa_calloc(BYTES)
#define MALLOC_STRUCT(T)   (struct T *) _mesa_malloc(sizeof(struct T))
#define CALLOC_STRUCT(T)   (struct T *) _mesa_calloc(sizeof(struct T))
#define FREE(PTR)          _mesa_free(PTR)

#else

/* directly call C lib memory functions */
#define MALLOC(BYTES)      (void *) malloc(BYTES)
#define CALLOC(BYTES)      (void *) calloc(1, BYTES)
#define MALLOC_STRUCT(T)   (struct T *) malloc(sizeof(struct T))
#define CALLOC_STRUCT(T)   (struct T *) calloc(1,sizeof(struct T))
#define FREE(PTR)          free(PTR)

#endif

/* call Mesa N-byte aligned memory functions */
#define ALIGN_MALLOC(BYTES, N)     (void *) _mesa_align_malloc(BYTES, N)
#define ALIGN_CALLOC(BYTES, N)     (void *) _mesa_align_calloc(BYTES, N)
#define ALIGN_MALLOC_STRUCT(T, N)  (struct T *) _mesa_align_malloc(sizeof(struct T), N)
#define ALIGN_CALLOC_STRUCT(T, N)  (struct T *) _mesa_align_calloc(sizeof(struct T), N)
#define ALIGN_FREE(PTR)            _mesa_align_free(PTR)


/* Memory copy: */
#ifdef SUNOS4
#define MEMCPY( DST, SRC, BYTES) \
	memcpy( (char *) (DST), (char *) (SRC), (int) (BYTES) )
#else
#define MEMCPY( DST, SRC, BYTES) \
	memcpy( (void *) (DST), (void *) (SRC), (size_t) (BYTES) )
#endif


/* Memory set: */
#ifdef SUNOS4
#define MEMSET( DST, VAL, N ) \
	memset( (char *) (DST), (int) (VAL), (int) (N) )
#else
#define MEMSET( DST, VAL, N ) \
	memset( (void *) (DST), (int) (VAL), (size_t) (N) )
#endif


/* On some systems we might want to use bzero() (but is bzero portable?) */
#if defined(__FreeBSD__)
#define BZERO( ADDR, N ) \
	bzero( (void *) (ADDR), (size_t) (N) )
#else
#define BZERO( ADDR, N ) \
	memset( (void *) (ADDR), 0, (size_t) (N) )
#endif


/* MACs and BeOS don't support static larger than 32kb, so... */
#if defined(macintosh) && !defined(__MRC__)
  extern char *AGLAlloc(int size);
  extern void AGLFree(char* ptr);
#  define DEFARRAY(TYPE,NAME,SIZE)  			TYPE *NAME = (TYPE*)AGLAlloc(sizeof(TYPE)*(SIZE))
#  define DEFMARRAY(TYPE,NAME,SIZE1,SIZE2)		TYPE (*NAME)[SIZE2] = (TYPE(*)[SIZE2])AGLAlloc(sizeof(TYPE)*(SIZE1)*(SIZE2))
#  define CHECKARRAY(NAME,CMD)				do {if (!(NAME)) {CMD;}} while (0) 
#  define UNDEFARRAY(NAME)          			do {if ((NAME)) {AGLFree((char*)NAME);}  }while (0)
#elif defined(__BEOS__)
#  define DEFARRAY(TYPE,NAME,SIZE)  			TYPE *NAME = (TYPE*)malloc(sizeof(TYPE)*(SIZE))
#  define DEFMARRAY(TYPE,NAME,SIZE1,SIZE2)  		TYPE (*NAME)[SIZE2] = (TYPE(*)[SIZE2])malloc(sizeof(TYPE)*(SIZE1)*(SIZE2))
#  define CHECKARRAY(NAME,CMD)				do {if (!(NAME)) {CMD;}} while (0)
#  define UNDEFARRAY(NAME)          			do {if ((NAME)) {free((char*)NAME);}  }while (0)
#else
#  define DEFARRAY(TYPE,NAME,SIZE)  			TYPE NAME[SIZE]
#  define DEFMARRAY(TYPE,NAME,SIZE1,SIZE2)		TYPE NAME[SIZE1][SIZE2]
#  define CHECKARRAY(NAME,CMD)				do {} while(0)
#  define UNDEFARRAY(NAME)
#endif




#endif
