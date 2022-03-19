/* $Id: winpos.h,v 1.3 2000/02/11 21:14:26 brianp Exp $ */

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


#ifndef WINPOS_H
#define WINPOS_H


#include "glheader.h"


extern void
_mesa_WindowPos2dMESA(GLdouble x, GLdouble y);

extern void
_mesa_WindowPos2fMESA(GLfloat x, GLfloat y);

extern void
_mesa_WindowPos2iMESA(GLint x, GLint y);

extern void
_mesa_WindowPos2sMESA(GLshort x, GLshort y);

extern void
_mesa_WindowPos3dMESA(GLdouble x, GLdouble y, GLdouble z);

extern void
_mesa_WindowPos3fMESA(GLfloat x, GLfloat y, GLfloat z);

extern void
_mesa_WindowPos3iMESA(GLint x, GLint y, GLint z);

extern void
_mesa_WindowPos3sMESA(GLshort x, GLshort y, GLshort z);

extern void
_mesa_WindowPos4dMESA(GLdouble x, GLdouble y, GLdouble z, GLdouble w);

extern void
_mesa_WindowPos4fMESA(GLfloat x, GLfloat y, GLfloat z, GLfloat w);

extern void
_mesa_WindowPos4iMESA(GLint x, GLint y, GLint z, GLint w);

extern void
_mesa_WindowPos4sMESA(GLshort x, GLshort y, GLshort z, GLshort w);

extern void
_mesa_WindowPos2dvMESA(const GLdouble *v);

extern void
_mesa_WindowPos2fvMESA(const GLfloat *v);

extern void
_mesa_WindowPos2ivMESA(const GLint *v);

extern void
_mesa_WindowPos2svMESA(const GLshort *v);

extern void
_mesa_WindowPos3dvMESA(const GLdouble *v);

extern void
_mesa_WindowPos3fvMESA(const GLfloat *v);

extern void
_mesa_WindowPos3ivMESA(const GLint *v);

extern void
_mesa_WindowPos3svMESA(const GLshort *v);

extern void
_mesa_WindowPos4dvMESA(const GLdouble *v);

extern void
_mesa_WindowPos4fvMESA(const GLfloat *v);

extern void
_mesa_WindowPos4ivMESA(const GLint *v);

extern void
_mesa_WindowPos4svMESA(const GLshort *v);


#endif
