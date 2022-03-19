/* $Id: vbfill.h,v 1.4 2000/04/05 14:40:04 brianp Exp $ */

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


#ifndef VBFILL_H
#define VBFILL_H


#include "types.h"


extern void gl_Begin( GLcontext *ctx, GLenum p );
extern void gl_End( GLcontext *ctx );
extern void gl_Vertex2f( GLcontext *ctx, GLfloat x, GLfloat y );


extern void
_mesa_Begin( GLenum p );

extern void
_mesa_End( void );



extern void _mesa_Color3b(GLbyte red, GLbyte green, GLbyte blue);

extern void _mesa_Color3d(GLdouble red, GLdouble green, GLdouble blue);

extern void _mesa_Color3f(GLfloat red, GLfloat green, GLfloat blue);

extern void _mesa_Color3i(GLint red, GLint green, GLint blue);

extern void _mesa_Color3s(GLshort red, GLshort green, GLshort blue);

extern void _mesa_Color3ub(GLubyte red, GLubyte green, GLubyte blue);

extern void _mesa_Color3ui(GLuint red, GLuint green, GLuint blue);

extern void _mesa_Color3us(GLushort red, GLushort green, GLushort blue);

extern void _mesa_Color4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);

extern void _mesa_Color4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);

extern void _mesa_Color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

extern void _mesa_Color4i(GLint red, GLint green, GLint blue, GLint alpha);

extern void _mesa_Color4s(GLshort red, GLshort green, GLshort blue, GLshort alpha);

extern void _mesa_Color4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);

extern void _mesa_Color4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha);

extern void _mesa_Color4us(GLushort red, GLushort green, GLushort blue, GLushort alpha);

extern void _mesa_Color3bv(const GLbyte *v);

extern void _mesa_Color3dv(const GLdouble *v);

extern void _mesa_Color3fv(const GLfloat *v);

extern void _mesa_Color3iv(const GLint *v);

extern void _mesa_Color3sv(const GLshort *v);

extern void _mesa_Color3ubv(const GLubyte *v);

extern void _mesa_Color3uiv(const GLuint *v);

extern void _mesa_Color3usv(const GLushort *v);

extern void _mesa_Color4bv(const GLbyte *v);

extern void _mesa_Color4dv(const GLdouble *v);

extern void _mesa_Color4fv(const GLfloat *v);

extern void _mesa_Color4iv(const GLint *v);

extern void _mesa_Color4sv(const GLshort *v);

extern void _mesa_Color4ubv(const GLubyte *v);

extern void _mesa_Color4uiv(const GLuint *v);

extern void _mesa_Color4usv(const GLushort *v);



extern void _mesa_EdgeFlag( GLboolean flag );

extern void _mesa_EdgeFlagv( const GLboolean *flag );



extern void _mesa_Indexd(GLdouble c);

extern void _mesa_Indexdv(const GLdouble *c);

extern void _mesa_Indexf(GLfloat c);

extern void _mesa_Indexfv(const GLfloat *c);

extern void _mesa_Indexi(GLint c);

extern void _mesa_Indexiv(const GLint *c);

extern void _mesa_Indexs(GLshort c);

extern void _mesa_Indexsv(const GLshort *c);

extern void _mesa_Indexub(GLubyte b);

extern void _mesa_Indexubv(const GLubyte *b);



extern void _mesa_Normal3b(GLbyte nx, GLbyte ny, GLbyte nz);

extern void _mesa_Normal3bv(const GLbyte *v);

extern void _mesa_Normal3d(GLdouble nx, GLdouble ny, GLdouble nz);

extern void _mesa_Normal3dv(const GLdouble *v);

extern void _mesa_Normal3f(GLfloat nx, GLfloat ny, GLfloat nz);

extern void _mesa_Normal3fv(const GLfloat *v);

extern void _mesa_Normal3i(GLint nx, GLint ny, GLint nz);

extern void _mesa_Normal3iv(const GLint *v);

extern void _mesa_Normal3s(GLshort nx, GLshort ny, GLshort nz);

extern void _mesa_Normal3sv(const GLshort *v);



extern void _mesa_TexCoord1d(GLdouble s);

extern void _mesa_TexCoord1dv(const GLdouble *v);

extern void _mesa_TexCoord1f(GLfloat s);

extern void _mesa_TexCoord1fv(const GLfloat *v);

extern void _mesa_TexCoord1i(GLint s);

extern void _mesa_TexCoord1iv(const GLint *v);

extern void _mesa_TexCoord1s(GLshort s);

extern void _mesa_TexCoord1sv(const GLshort *v);

extern void _mesa_TexCoord2d(GLdouble s, GLdouble t);

extern void _mesa_TexCoord2dv(const GLdouble *v);

extern void _mesa_TexCoord2f(GLfloat s, GLfloat t);

extern void _mesa_TexCoord2fv(const GLfloat *v);

extern void _mesa_TexCoord2s(GLshort s, GLshort t);

extern void _mesa_TexCoord2sv(const GLshort *v);

extern void _mesa_TexCoord2i(GLint s, GLint t);

extern void _mesa_TexCoord2iv(const GLint *v);

extern void _mesa_TexCoord3d(GLdouble s, GLdouble t, GLdouble r);

extern void _mesa_TexCoord3dv(const GLdouble *v);

extern void _mesa_TexCoord3f(GLfloat s, GLfloat t, GLfloat r);

extern void _mesa_TexCoord3fv(const GLfloat *v);

extern void _mesa_TexCoord3i(GLint s, GLint t, GLint r);

extern void _mesa_TexCoord3iv(const GLint *v);

extern void _mesa_TexCoord3s(GLshort s, GLshort t, GLshort r);

extern void _mesa_TexCoord3sv(const GLshort *v);

extern void _mesa_TexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q);

extern void _mesa_TexCoord4dv(const GLdouble *v);

extern void _mesa_TexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);

extern void _mesa_TexCoord4fv(const GLfloat *v);

extern void _mesa_TexCoord4i(GLint s, GLint t, GLint r, GLint q);

extern void _mesa_TexCoord4iv(const GLint *v);

extern void _mesa_TexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q);

extern void _mesa_TexCoord4sv(const GLshort *v);



extern void _mesa_Vertex2d(GLdouble x, GLdouble y);

extern void _mesa_Vertex2dv(const GLdouble *v);

extern void _mesa_Vertex2f(GLfloat x, GLfloat y);

extern void _mesa_Vertex2fv(const GLfloat *v);

extern void _mesa_Vertex2i(GLint x, GLint y);

extern void _mesa_Vertex2iv(const GLint *v);

extern void _mesa_Vertex2s(GLshort x, GLshort y);

extern void _mesa_Vertex2sv(const GLshort *v);

extern void _mesa_Vertex3d(GLdouble x, GLdouble y, GLdouble z);

extern void _mesa_Vertex3dv(const GLdouble *v);

extern void _mesa_Vertex3f(GLfloat x, GLfloat y, GLfloat z);

extern void _mesa_Vertex3fv(const GLfloat *v);

extern void _mesa_Vertex3i(GLint x, GLint y, GLint z);

extern void _mesa_Vertex3iv(const GLint *v);

extern void _mesa_Vertex3s(GLshort x, GLshort y, GLshort z);

extern void _mesa_Vertex3sv(const GLshort *v);

extern void _mesa_Vertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);

extern void _mesa_Vertex4dv(const GLdouble *v);

extern void _mesa_Vertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);

extern void _mesa_Vertex4fv(const GLfloat *v);

extern void _mesa_Vertex4i(GLint x, GLint y, GLint z, GLint w);

extern void _mesa_Vertex4iv(const GLint *v);

extern void _mesa_Vertex4s(GLshort x, GLshort y, GLshort z, GLshort w);

extern void _mesa_Vertex4sv(const GLshort *v);



extern void _mesa_MultiTexCoord1dARB(GLenum target, GLdouble s);

extern void _mesa_MultiTexCoord1dvARB(GLenum target, const GLdouble *v);

extern void _mesa_MultiTexCoord1fARB(GLenum target, GLfloat s);

extern void _mesa_MultiTexCoord1fvARB(GLenum target, const GLfloat *v);

extern void _mesa_MultiTexCoord1iARB(GLenum target, GLint s);

extern void _mesa_MultiTexCoord1ivARB(GLenum target, const GLint *v);

extern void _mesa_MultiTexCoord1sARB(GLenum target, GLshort s);

extern void _mesa_MultiTexCoord1svARB(GLenum target, const GLshort *v);

extern void _mesa_MultiTexCoord2dARB(GLenum target, GLdouble s, GLdouble t);

extern void _mesa_MultiTexCoord2dvARB(GLenum target, const GLdouble *v);

extern void _mesa_MultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t);

extern void _mesa_MultiTexCoord2fvARB(GLenum target, const GLfloat *v);

extern void _mesa_MultiTexCoord2iARB(GLenum target, GLint s, GLint t);

extern void _mesa_MultiTexCoord2ivARB(GLenum target, const GLint *v);

extern void _mesa_MultiTexCoord2sARB(GLenum target, GLshort s, GLshort t);

extern void _mesa_MultiTexCoord2svARB(GLenum target, const GLshort *v);

extern void _mesa_MultiTexCoord3dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r);

extern void _mesa_MultiTexCoord3dvARB(GLenum target, const GLdouble *v);

extern void _mesa_MultiTexCoord3fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r);

extern void _mesa_MultiTexCoord3fvARB(GLenum target, const GLfloat *v);

extern void _mesa_MultiTexCoord3iARB(GLenum target, GLint s, GLint t, GLint r);

extern void _mesa_MultiTexCoord3ivARB(GLenum target, const GLint *v);

extern void _mesa_MultiTexCoord3sARB(GLenum target, GLshort s, GLshort t, GLshort r);

extern void _mesa_MultiTexCoord3svARB(GLenum target, const GLshort *v);

extern void _mesa_MultiTexCoord4dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);

extern void _mesa_MultiTexCoord4dvARB(GLenum target, const GLdouble *v);

extern void _mesa_MultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);

extern void _mesa_MultiTexCoord4fvARB(GLenum target, const GLfloat *v);

extern void _mesa_MultiTexCoord4iARB(GLenum target, GLint s, GLint t, GLint r, GLint q);

extern void _mesa_MultiTexCoord4ivARB(GLenum target, const GLint *v);

extern void _mesa_MultiTexCoord4sARB(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);

extern void _mesa_MultiTexCoord4svARB(GLenum target, const GLshort *v);


#endif
