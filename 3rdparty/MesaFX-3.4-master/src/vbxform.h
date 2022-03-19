/* $Id: vbxform.h,v 1.4 1999/11/09 17:00:25 keithw Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.1
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





#ifndef VBXFORM_H
#define VBXFORM_H

extern void gl_internal_flush( GLcontext *ctx );

extern void gl_maybe_transform_vb( struct immediate *IM );

extern void gl_compute_orflag( struct immediate *IM );

extern void gl_reset_vb( struct vertex_buffer *VB );

extern void gl_fixup_input( GLcontext *ctx, struct immediate *IM );
extern void gl_fixup_cassette( GLcontext *ctx, struct immediate *IM );


extern void gl_copy_prev_vertices( struct vertex_buffer *VB, 
				   struct immediate *prev,
				   struct immediate *next );

extern void gl_execute_cassette( GLcontext *ctx, struct immediate *IM );

extern void gl_reset_input( GLcontext *ctx );
extern void gl_execute_vb( GLcontext *ctx );

extern void gl_copy_to_current( GLcontext *ctx, struct immediate *IM );
extern void gl_copy_all_to_current( GLcontext *ctx, struct immediate *IM );


extern struct immediate *gl_purge_immediate( GLcontext *ctx, 
					     struct immediate *IM,
					     GLuint flag );

extern void gl_print_cassette( struct immediate *IM );
extern void gl_print_cassette_flags( struct immediate *IM, GLuint *flags );


extern GLuint gl_texcoord_size( GLuint flag, GLuint unit );

#endif
