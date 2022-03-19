/* $Id: span.h,v 1.5 2000/02/21 23:40:43 brianp Exp $ */

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





#ifndef SPAN_H
#define SPAN_H


#include "types.h"


extern void gl_write_index_span( GLcontext *ctx,
                                 GLuint n, GLint x, GLint y, const GLdepth z[],
				 GLuint index[], GLenum primitive );


extern void gl_write_monoindex_span( GLcontext *ctx,
                                     GLuint n, GLint x, GLint y,
                                     const GLdepth z[],
				     GLuint index, GLenum primitive );


extern void gl_write_rgba_span( GLcontext *ctx,
                                GLuint n, GLint x, GLint y, const GLdepth z[],
                                GLubyte rgba[][4], GLenum primitive );


extern void gl_write_monocolor_span( GLcontext *ctx,
                                     GLuint n, GLint x, GLint y,
                                     const GLdepth z[], const GLubyte color[4],
                                     GLenum primitive );


extern void gl_write_texture_span( GLcontext *ctx,
                                   GLuint n, GLint x, GLint y,
                                   const GLdepth z[],
				   const GLfloat s[], const GLfloat t[],
                                   const GLfloat u[], GLfloat lambda[],
				   GLubyte rgba[][4], CONST GLubyte spec[][4],
                                   GLenum primitive );


extern void
gl_write_multitexture_span( GLcontext *ctx, GLuint texSets,
                            GLuint n, GLint x, GLint y,
                            const GLdepth z[],
                            CONST GLfloat s[MAX_TEXTURE_UNITS][MAX_WIDTH],
                            CONST GLfloat t[MAX_TEXTURE_UNITS][MAX_WIDTH],
                            CONST GLfloat u[MAX_TEXTURE_UNITS][MAX_WIDTH],
                            GLfloat lambda[MAX_TEXTURE_UNITS][MAX_WIDTH],
                            GLubyte rgba[][4],
                            CONST GLubyte spec[][4],
                            GLenum primitive );


extern void gl_read_rgba_span( GLcontext *ctx, GLframebuffer *buffer,
                               GLuint n, GLint x, GLint y,
                               GLubyte rgba[][4] );


extern void gl_read_index_span( GLcontext *ctx, GLframebuffer *buffer,
                                GLuint n, GLint x, GLint y, GLuint indx[] );


#endif
