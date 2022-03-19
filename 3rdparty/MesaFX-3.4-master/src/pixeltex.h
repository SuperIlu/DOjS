/* $Id: pixeltex.h,v 1.2 2000/04/14 07:42:46 joukj Exp $ */

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
 * The above copyright noti_mesa_PixelTexGenParameterfvce and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef PIXELTEX_H
#define PIXELTEX_H


extern void
_mesa_PixelTexGenSGIX(GLenum mode);

extern void
_mesa_PixelTexGenParameterfSGIS(GLenum target, GLfloat value);

#ifdef VMS
#define _mesa_PixelTexGenParameterfvSGIS _mesa_PixelTexGenParameterfv
#endif
extern void
_mesa_PixelTexGenParameterfvSGIS(GLenum target, const GLfloat *value);

extern void
_mesa_PixelTexGenParameteriSGIS(GLenum target, GLint value);

#ifdef VMS
#define _mesa_PixelTexGenParameterivSGIS _mesa_PixelTexGenParameteriv
#endif
extern void
_mesa_PixelTexGenParameterivSGIS(GLenum target, const GLint *value);

#ifdef VMS
#define _mesa_GetPixelTexGenParameterfvSGIS _mesa_GetPixelTexGenParameterfv
#endif
extern void
_mesa_GetPixelTexGenParameterfvSGIS(GLenum target, GLfloat *value);

#ifdef VMS
#define _mesa_GetPixelTexGenParameterivSGIS _mesa_GetPixelTexGenParameteriv
#endif
extern void
_mesa_GetPixelTexGenParameterivSGIS(GLenum target, GLint *value);

extern void
_mesa_pixeltexgen(GLcontext *ctx, GLuint n, const GLubyte rgba[][4],
                  GLfloat s[], GLfloat t[], GLfloat r[], GLfloat q[]);


#endif
