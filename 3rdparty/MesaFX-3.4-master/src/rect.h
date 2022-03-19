/* $Id: rect.h,v 1.2 1999/11/11 01:22:27 brianp Exp $ */

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


#ifndef RECT_H
#define RECT_H


#include "glheader.h"


extern void
_mesa_Rectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);

extern void
_mesa_Rectdv(const GLdouble *v1, const GLdouble *v2);

extern void
_mesa_Rectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);

extern void
_mesa_Rectfv(const GLfloat *v1, const GLfloat *v2);

extern void
_mesa_Recti(GLint x1, GLint y1, GLint x2, GLint y2);

extern void
_mesa_Rectiv(const GLint *v1, const GLint *v2);

extern void
_mesa_Rects(GLshort x1, GLshort y1, GLshort x2, GLshort y2);

extern void
_mesa_Rectsv(const GLshort *v1, const GLshort *v2);


#endif
