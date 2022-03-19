/* $Id: mem.c,v 1.2 2000/06/27 22:10:00 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.3
 *
 * Copyright (C) 1999  Brian Paul   All Rights Reserved.
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


/*
 * Memory allocation functions.  Called via the MALLOC, CALLOC and
 * FREE macros when DEBUG symbol is defined.
 * You might want to set breakpoints on these functions or plug in
 * other memory allocation functions.  The Mesa sources should only
 * use the MALLOC and FREE macros (which could also be overriden).
 */

#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "macros.h"
#include "mem.h"
#endif



/*
 * Allocate memory (uninitialized)
 */
void *
_mesa_malloc(size_t bytes)
{
   return malloc(bytes);
}


/*
 * Allocate memory and initialize to zero.
 */
void *
_mesa_calloc(size_t bytes)
{
   return calloc(1, bytes);
}


/*
 * Free memory
 */
void
_mesa_free(void *ptr)
{
   free(ptr);
}



/*
 * N-byte aligned memory allocation functions.  Called via the ALIGN_MALLOC,
 * ALIGN_CALLOC and ALIGN_FREE macros.  Debug versions?
 * These functions allow dynamically allocated memory to be correctly
 * aligned for improved cache utilization and specialized assembly
 * support.
 */


/*
 * Allocate N-byte aligned memory (uninitialized)
 */
void *
_mesa_align_malloc(size_t bytes, unsigned long alignment)
{
   unsigned long ptr, buf;

   ASSERT( alignment > 0 );

   ptr = (unsigned long) MALLOC( bytes + alignment );

   buf = (ptr + alignment) & ~(unsigned long)(alignment - 1);
   *(unsigned long *)(buf - sizeof(void *)) = ptr;

#ifdef DEBUG
   /* mark the non-aligned area */
   while ( ptr < buf - sizeof(void *) ) {
      *(unsigned long *)ptr = 0xcdcdcdcd;
      ptr += sizeof(unsigned long);
   }
#endif

   return (void *)buf;
}


/*
 * Allocate N-byte aligned memory and initialize to zero
 */
void *
_mesa_align_calloc(size_t bytes, unsigned long alignment)
{
   unsigned long ptr, buf;

   ASSERT( alignment > 0 );

   ptr = (unsigned long) CALLOC( bytes + alignment );

   buf = (ptr + alignment) & ~(unsigned long)(alignment - 1);
   *(unsigned long *)(buf - sizeof(void *)) = ptr;

#ifdef DEBUG
   /* mark the non-aligned area */
   while ( ptr < buf - sizeof(void *) ) {
      *(unsigned long *)ptr = 0xcdcdcdcd;
      ptr += sizeof(unsigned long);
   }
#endif

   return (void *)buf;
}


/*
 * Free N-byte aligned memory
 */
void
_mesa_align_free(void *ptr)
{
#if 0
   FREE( (void *)(*(unsigned long *)((unsigned long)ptr - sizeof(void *))) );
#else
   /* The actuall address to free is stuffed in the word immediately
    * before the address the client sees.
    */
   void **cubbyHole = (void **) ((char *) ptr - sizeof(void *));
   void *realAddr = *cubbyHole;
   FREE(realAddr);
#endif
}
