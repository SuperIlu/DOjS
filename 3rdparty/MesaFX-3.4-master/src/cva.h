/* $Id: cva.h,v 1.3 1999/11/11 01:22:26 brianp Exp $ */

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
 * New (3.1) transformation code written by Keith Whitwell.
 */


#ifndef _CVA_H_
#define _CVA_H_


extern void gl_merge_cva( struct vertex_buffer *VB,
			  struct vertex_buffer *cvaVB );

extern void gl_prepare_arrays_cva( struct vertex_buffer *cvaVB );

extern void gl_cva_compile_cassette( GLcontext *ctx, struct immediate *IM );

extern void gl_cva_init( GLcontext *ctx );

extern void gl_alloc_cva_store( struct gl_cva *cva, GLuint size );
extern void gl_free_cva_store( struct gl_cva *cva );


extern void gl_rescue_cva( GLcontext *ctx, struct immediate *IM );
extern void gl_cva_force_precalc( GLcontext *ctx );

extern void
_mesa_LockArraysEXT(GLint first, GLsizei count);

extern void
_mesa_UnlockArraysEXT( void );



#endif
