/* $Id: vertices.h,v 1.8.4.1 2000/10/22 23:10:49 gareth Exp $ */

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


#ifndef __VERTICES_H__
#define __VERTICES_H__

#ifdef USE_X86_ASM
#define _PROJAPI _ASMAPI
#define _PROJAPIP _ASMAPIP
#else
#define _PROJAPI
#define _PROJAPIP *
#endif

typedef void (_PROJAPIP gl_transform_func)( GLfloat *first_vert,
					    const GLfloat *m,
					    const GLfloat *src,
					    GLuint src_stride,
					    GLuint count );

typedef void (_PROJAPIP gl_cliptest_func)( GLfloat *first_vert,
					   GLfloat *last_vert, /* use count instead? */
					   GLubyte *or_mask,
					   GLubyte *and_mask,
					   GLubyte *clip_mask );

typedef void (_PROJAPIP gl_project_func)( GLfloat *first,
					  GLfloat *last,
					  const GLfloat *m,
					  GLuint stride );

typedef void (_PROJAPIP gl_project_clipped_func)( GLfloat *first,
						  GLfloat *last,
						  const GLfloat *m,
						  GLuint stride,
						  const GLubyte *clipmask );

typedef void (_PROJAPIP gl_vertex_interp_func)( GLfloat t,
						GLfloat *result,
						const GLfloat *in,
						const GLfloat *out );


/* At the moment these are used by fastpaths in the FX and MGA drivers.
 */
extern gl_transform_func       gl_xform_points3_v16_general;
extern gl_cliptest_func        gl_cliptest_points4_v16;
extern gl_project_func         gl_project_v16;
extern gl_project_clipped_func gl_project_clipped_v16;


extern GLenum gl_reduce_prim[];

extern void gl_init_vertices( void );

#endif
