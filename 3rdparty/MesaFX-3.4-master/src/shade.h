/* $Id: shade.h,v 1.1.1.1.6.1 2000/11/30 02:44:20 gareth Exp $ */

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





#ifndef SHADE_H
#define SHADE_H


#include "types.h"



extern void gl_update_lighting_function( GLcontext *ctx );

extern void gl_init_shade( void );

extern gl_shade_func gl_shade_tab[0x10];
extern gl_shade_func gl_shade_fast_tab[0x10];
extern gl_shade_func gl_shade_fast_single_tab[0x10];
extern gl_shade_func gl_shade_spec_tab[0x10];
extern gl_shade_func gl_shade_ci_tab[0x10];


void gl_shade_rastpos( GLcontext *ctx,
		       GLfloat vertex[4],
		       GLfloat normal[3],
		       GLfloat Rcolor[4],
		       GLuint *index );

#endif
