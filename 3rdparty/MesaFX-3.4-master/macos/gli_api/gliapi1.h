/* $Id: gliapi1.h,v 1.1 1999/10/31 01:00:27 miklos Exp $ */

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




#ifdef PC_HEADER
#include "all.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include "api.h"
#include "bitmap.h"
#include "context.h"
#include "drawpix.h"
#include "eval.h"
#include "image.h"
#include "light.h"
#include "macros.h"
#include "matrix.h"
#include "mmath.h"
#include "teximage.h"
#include "types.h"
#include "varray.h"
#include "vb.h"
#include "vbfill.h"
#ifdef XFree86Server
#include "GL/xf86glx.h"
#endif
#endif


/*
 * Part 1 of API functions
 */



API_PREFIX void API_NAME(Accum)(CTX_PREFIX  GLenum op, GLfloat value )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Accum)(CC, op, value);
}


API_PREFIX void API_NAME(AlphaFunc)(CTX_PREFIX  GLenum func, GLclampf ref )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.AlphaFunc)(CC, func, ref);
}


API_PREFIX GLboolean API_NAME(AreTexturesResident)(CTX_PREFIX  GLsizei n, 
                       const GLuint *textures, GLboolean *residences )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(GL_FALSE);
   return (*CC->API.AreTexturesResident)(CC, n, textures, residences);
}

/* Enough funny business going on in here it might be quicker to use a
 * function pointer.
 */
#define ARRAY_ELT( IM, i )					\
{								\
   GLuint count = IM->Count;					\
   IM->Elt[count] = i;						\
   IM->Flag[count] = ((IM->Flag[count] & IM->ArrayAndFlags) |	\
		      IM->ArrayOrFlags);			\
   IM->Flush |= IM->ArrayEltFlush;      			\
   IM->Count = count += IM->ArrayIncr;				\
   if (count == VB_MAX)						\
      IM->maybe_transform_vb( IM );				\
}


API_PREFIX void API_NAME(ArrayElement)(CTX_PREFIX  GLint i )
{
   GET_IMMEDIATE;
   ARRAY_ELT( IM, i );
}


API_PREFIX void API_NAME(ArrayElementEXT)(CTX_PREFIX  GLint i )
{
   GET_IMMEDIATE;
   ARRAY_ELT( IM, i );
}


void gl_ArrayElement( GLcontext *CC, GLint i )
{
   struct immediate *im = CC->input;
   ARRAY_ELT( im, i );
}


API_PREFIX void API_NAME(Begin)(CTX_PREFIX  GLenum mode )
{
   GET_CONTEXT;

   if (mode < GL_POINTS || mode > GL_POLYGON) {
      gl_compile_error( CC, GL_INVALID_ENUM, "glBegin" );
      return;		     
   }

   gl_Begin(CC,mode);
}


API_PREFIX void API_NAME(BindTexture)(CTX_PREFIX  GLenum target, GLuint texture )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.BindTexture)(CC, target, texture);
}


API_PREFIX void API_NAME(Bitmap)(CTX_PREFIX  GLsizei width, GLsizei height,
                        GLfloat xorig, GLfloat yorig,
                        GLfloat xmove, GLfloat ymove,
                        const GLubyte *bitmap )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   if (CC->CompileFlag || !gl_direct_bitmap( CC, width, height, xorig, yorig,
                                             xmove, ymove, bitmap)) {
      struct gl_image *image;
      image = gl_unpack_bitmap( CC, width, height, bitmap );
      (*CC->API.Bitmap)( CC, width, height, xorig, yorig,
                         xmove, ymove, image );
      if (image && image->RefCount==0) {
         gl_free_image( image );
      }
   }
}


API_PREFIX void API_NAME(BlendFunc)(CTX_PREFIX  GLenum sfactor, GLenum dfactor )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.BlendFunc)(CC, sfactor, dfactor);
}


API_PREFIX void API_NAME(CallList)(CTX_PREFIX  GLuint list )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CallList)(CC, list);
}


API_PREFIX void API_NAME(CallLists)(CTX_PREFIX  GLsizei n, GLenum type, 
			    const GLvoid *lists )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CallLists)(CC, n, type, lists);
}


API_PREFIX void API_NAME(Clear)(CTX_PREFIX  GLbitfield mask )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Clear)(CC, mask);
}


API_PREFIX void API_NAME(ClearAccum)(CTX_PREFIX  GLfloat red, GLfloat green,
                              GLfloat blue, GLfloat alpha )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ClearAccum)(CC, red, green, blue, alpha);
}



API_PREFIX void API_NAME(ClearIndex)(CTX_PREFIX  GLfloat c )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ClearIndex)(CC, c);
}


API_PREFIX void API_NAME(ClearColor)(CTX_PREFIX  GLclampf red, GLclampf green,
                              GLclampf blue, GLclampf alpha )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ClearColor)(CC, red, green, blue, alpha);
}


API_PREFIX void API_NAME(ClearDepth)(CTX_PREFIX  GLclampd depth )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ClearDepth)( CC, depth );
}


API_PREFIX void API_NAME(ClearStencil)(CTX_PREFIX  GLint s )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ClearStencil)(CC, s);
}


API_PREFIX void API_NAME(ClipPlane)(CTX_PREFIX  GLenum plane, const GLdouble *equation )
{
   GLfloat eq[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   eq[0] = (GLfloat) equation[0];
   eq[1] = (GLfloat) equation[1];
   eq[2] = (GLfloat) equation[2];
   eq[3] = (GLfloat) equation[3];
   (*CC->API.ClipPlane)(CC, plane, eq );
}


/* KW: Again, a stateless implementation of these functions.  The big
 * news here is the impact on color material.  This was previously
 * handled by swaping the function pointers that these API's used to
 * call.  This is no longer possible, and we have to pick up the
 * pieces later on and make them work with either color-color, or
 * color-material.
 *
 * But in truth, this is not a bad thing, because it was necessary
 * to implement that mechanism to get good performance from
 * color-material and vertex arrays.  
 */
#define COLOR( IM, r,g,b,a )			\
{						\
   GLuint count = IM->Count;			\
   IM->Flag[count] |= VERT_RGBA;		\
   IM->Color[count][0] = r;			\
   IM->Color[count][1] = g;			\
   IM->Color[count][2] = b;			\
   IM->Color[count][3] = a;			\
}

#if 0
#define COLOR4F( IM, r,g,b,a )				\
{							\
   GLuint count = IM->Count;				\
   IM->Flag[count] |= VERT_RGBA | VERT_FLOAT_RGBA;	\
   IM->FloatColor[count][0] = r;			\
   IM->FloatColor[count][1] = g;			\
   IM->FloatColor[count][2] = b;			\
   IM->FloatColor[count][3] = a;			\
}
#else
#define COLOR4F(IM, r, g, b, a)			\
{						\
   GLubyte col[4];				\
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], r);	\
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], g);	\
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], b);	\
   FLOAT_COLOR_TO_UBYTE_COLOR(col[3], a);	\
   COLORV( IM, col );				\
}
#endif



#define COLORV( IM, v )				\
{						\
   GLuint count = IM->Count;			\
   IM->Flag[count] |= VERT_RGBA;		\
   COPY_4UBV(IM->Color[count], v);		\
}


API_PREFIX void API_NAME(Color3b)(CTX_PREFIX  GLbyte red, GLbyte green, GLbyte blue )
{
   GET_IMMEDIATE;
   COLOR( IM, 
	  BYTE_TO_UBYTE(red), 
	  BYTE_TO_UBYTE(green),
	  BYTE_TO_UBYTE(blue),
	  255 );
}


API_PREFIX void API_NAME(Color3d)(CTX_PREFIX  GLdouble red, GLdouble green, 
                                     GLdouble blue )
{
   GLubyte col[4];
   GLfloat r = red;
   GLfloat g = green;
   GLfloat b = blue;
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], r);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], g);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], b);
   col[3] = 255;
   COLORV( IM, col );

/*     COLOR4F( IM, red, green, blue, 1.0 ); */
}


API_PREFIX void API_NAME(Color3f)(CTX_PREFIX  GLfloat red, GLfloat green, GLfloat blue )
{
   GLubyte col[4];
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], red);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], green);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], blue);
   col[3] = 255;
   COLORV( IM, col );

/*     COLOR4F( IM, red, green, blue, 1.0 ); */
}


API_PREFIX void API_NAME(Color3i)(CTX_PREFIX  GLint red, GLint green, GLint blue )
{
   GET_IMMEDIATE;
   COLOR( IM, INT_TO_UBYTE(red), 
	  INT_TO_UBYTE(green),
	  INT_TO_UBYTE(blue),
	  255);
}


API_PREFIX void API_NAME(Color3s)(CTX_PREFIX  GLshort red, GLshort green, GLshort blue )
{
   GET_IMMEDIATE;
   COLOR( IM, SHORT_TO_UBYTE(red), 
	  SHORT_TO_UBYTE(green),
	  SHORT_TO_UBYTE(blue),
	  255);
}


API_PREFIX void API_NAME(Color3ub)(CTX_PREFIX  GLubyte red, GLubyte green, GLubyte blue )
{
   GET_IMMEDIATE;
   COLOR( IM, red, green, blue, 255 );
}


API_PREFIX void API_NAME(Color3ui)(CTX_PREFIX  GLuint red, GLuint green, GLuint blue )
{
   GET_IMMEDIATE;
   COLOR( IM, UINT_TO_UBYTE(red),
	  UINT_TO_UBYTE(green),
	  UINT_TO_UBYTE(blue),
	  255 );
}


API_PREFIX void API_NAME(Color3us)(CTX_PREFIX  GLushort red, GLushort green, 
                                      GLushort blue )
{
   GET_IMMEDIATE;
   COLOR( IM, USHORT_TO_UBYTE(red), USHORT_TO_UBYTE(green),
	  USHORT_TO_UBYTE(blue),
	  255 );
}


API_PREFIX void API_NAME(Color4b)(CTX_PREFIX  GLbyte red, GLbyte green, GLbyte blue, 
                                     GLbyte alpha )
{
   GET_IMMEDIATE;
   COLOR( IM, BYTE_TO_UBYTE(red), BYTE_TO_UBYTE(green),
	  BYTE_TO_UBYTE(blue), BYTE_TO_UBYTE(alpha) );
}


API_PREFIX void API_NAME(Color4d)(CTX_PREFIX  GLdouble red, GLdouble green, 
                                     GLdouble blue, GLdouble alpha )
{
   GLubyte col[4];
   GLfloat r = red;
   GLfloat g = green;
   GLfloat b = blue;
   GLfloat a = alpha;
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], r);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], g);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], b);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[3], a);
   COLORV( IM, col );

/*     COLOR4F( IM, red, green, blue, alpha ); */
}


API_PREFIX void API_NAME(Color4f)(CTX_PREFIX  GLfloat red, GLfloat green,
                           GLfloat blue, GLfloat alpha )
{
   GLubyte col[4];
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], red);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], green);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], blue);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[3], alpha);
   COLORV( IM, col );

/*     COLOR4F( IM, red, green, blue, alpha ); */
}

API_PREFIX void API_NAME(Color4i)(CTX_PREFIX  GLint red, GLint green, 
                                     GLint blue, GLint alpha )
{
   GET_IMMEDIATE;
   COLOR( IM, INT_TO_UBYTE(red), INT_TO_UBYTE(green),
	  INT_TO_UBYTE(blue), INT_TO_UBYTE(alpha) );
}


API_PREFIX void API_NAME(Color4s)(CTX_PREFIX  GLshort red, GLshort green,
                                     GLshort blue, GLshort alpha )
{
   GET_IMMEDIATE;
   COLOR( IM, SHORT_TO_UBYTE(red), SHORT_TO_UBYTE(green),
	  SHORT_TO_UBYTE(blue), SHORT_TO_UBYTE(alpha) );
}

API_PREFIX void API_NAME(Color4ub)(CTX_PREFIX  GLubyte red, GLubyte green,
                                      GLubyte blue, GLubyte alpha )
{
   GET_IMMEDIATE;
   COLOR( IM, red, green, blue, alpha );
}

API_PREFIX void API_NAME(Color4ui)(CTX_PREFIX  GLuint red, GLuint green,
                                      GLuint blue, GLuint alpha )
{
   GET_IMMEDIATE;
   COLOR( IM, UINT_TO_UBYTE(red), UINT_TO_UBYTE(green),
	  UINT_TO_UBYTE(blue), UINT_TO_UBYTE(alpha) );
}

API_PREFIX void API_NAME(Color4us)(CTX_PREFIX  GLushort red, GLushort green,
                                      GLushort blue, GLushort alpha )
{
   GET_IMMEDIATE;
   COLOR( IM, USHORT_TO_UBYTE(red), USHORT_TO_UBYTE(green),
	  USHORT_TO_UBYTE(blue), USHORT_TO_UBYTE(alpha) );
}


API_PREFIX void API_NAME(Color3bv)(CTX_PREFIX  const GLbyte *v )
{
   GET_IMMEDIATE;
   COLOR( IM, BYTE_TO_UBYTE(v[0]), BYTE_TO_UBYTE(v[1]),
	  BYTE_TO_UBYTE(v[2]), 255 );
}


API_PREFIX void API_NAME(Color3dv)(CTX_PREFIX  const GLdouble *v )
{
   GLubyte col[4];
   GLfloat r = v[0];
   GLfloat g = v[1];
   GLfloat b = v[2];
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], r);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], g);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], b);
   col[3]= 255;
   COLORV( IM, col );

/*     COLOR4F( IM, v[0], v[1], v[2], v[3] ); */
}


API_PREFIX void API_NAME(Color3fv)(CTX_PREFIX  const GLfloat *v )
{
   GLubyte col[4];
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], v[0]);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], v[1]);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], v[2]);
   col[3] = 255;
   COLORV( IM, col );

/*     COLOR4F( IM, v[0], v[1], v[2], v[3] ); */
}


API_PREFIX void API_NAME(Color3iv)(CTX_PREFIX  const GLint *v )
{
   GET_IMMEDIATE;
   COLOR( IM, INT_TO_UBYTE(v[0]), INT_TO_UBYTE(v[1]),
	  INT_TO_UBYTE(v[2]), 255 );
}


API_PREFIX void API_NAME(Color3sv)(CTX_PREFIX  const GLshort *v )
{
   GET_IMMEDIATE;
   COLOR( IM, SHORT_TO_UBYTE(v[0]), SHORT_TO_UBYTE(v[1]),
	  SHORT_TO_UBYTE(v[2]), 255 );
}


API_PREFIX void API_NAME(Color3ubv)(CTX_PREFIX  const GLubyte *v )
{
   GET_IMMEDIATE;
   COLOR( IM, v[0], v[1], v[2], 255 );
}


API_PREFIX void API_NAME(Color3uiv)(CTX_PREFIX  const GLuint *v )
{
   GET_IMMEDIATE;
   COLOR( IM, UINT_TO_UBYTE(v[0]), UINT_TO_UBYTE(v[1]),
	  UINT_TO_UBYTE(v[2]), 255 );
}


API_PREFIX void API_NAME(Color3usv)(CTX_PREFIX  const GLushort *v )
{
   GET_IMMEDIATE;
   COLOR( IM, USHORT_TO_UBYTE(v[0]), USHORT_TO_UBYTE(v[1]),
	  USHORT_TO_UBYTE(v[2]), 255 );

}


API_PREFIX void API_NAME(Color4bv)(CTX_PREFIX  const GLbyte *v )
{
   GET_IMMEDIATE;
   COLOR( IM, BYTE_TO_UBYTE(v[0]), BYTE_TO_UBYTE(v[1]),
	  BYTE_TO_UBYTE(v[2]), BYTE_TO_UBYTE(v[3]) );
}


API_PREFIX void API_NAME(Color4dv)(CTX_PREFIX  const GLdouble *v )
{
   GLubyte col[4];
   GLfloat r = v[0];
   GLfloat g = v[1];
   GLfloat b = v[2];
   GLfloat a = v[3];
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], r);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], g);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], b);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[3], a);
   COLORV( IM, col );

/*     COLOR4F( IM, v[0], v[1], v[2], v[3] ); */
}


API_PREFIX void API_NAME(Color4fv)(CTX_PREFIX  const GLfloat *v )
{
   GLubyte col[4];
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], v[0]);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], v[1]);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], v[2]);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[3], v[3]);
   COLORV( IM, col );

/*     COLOR4F( IM, v[0], v[1], v[2], v[3] ); */
}


API_PREFIX void API_NAME(Color4iv)(CTX_PREFIX  const GLint *v )
{
   GET_IMMEDIATE;
   COLOR( IM, INT_TO_UBYTE(v[0]), INT_TO_UBYTE(v[1]),
	  INT_TO_UBYTE(v[2]), INT_TO_UBYTE(v[3]) );
}


API_PREFIX void API_NAME(Color4sv)(CTX_PREFIX  const GLshort *v )
{
   GET_IMMEDIATE;
   COLOR( IM, SHORT_TO_UBYTE(v[0]), SHORT_TO_UBYTE(v[1]),
	  SHORT_TO_UBYTE(v[2]), SHORT_TO_UBYTE(v[3]) );
}


API_PREFIX void API_NAME(Color4ubv)(CTX_PREFIX  const GLubyte *v )
{
   GET_IMMEDIATE;
   COLORV( IM, v );
}


API_PREFIX void API_NAME(Color4uiv)(CTX_PREFIX  const GLuint *v )
{
   GET_IMMEDIATE;
   COLOR( IM, UINT_TO_UBYTE(v[0]), UINT_TO_UBYTE(v[1]),
	  UINT_TO_UBYTE(v[2]), UINT_TO_UBYTE(v[3]) );
}


API_PREFIX void API_NAME(Color4usv)(CTX_PREFIX  const GLushort *v )
{
   GET_IMMEDIATE;
   COLOR( IM, USHORT_TO_UBYTE(v[0]), USHORT_TO_UBYTE(v[1]),
	  USHORT_TO_UBYTE(v[2]), USHORT_TO_UBYTE(v[3]) );
}


API_PREFIX void API_NAME(ColorMask)(CTX_PREFIX  GLboolean red, GLboolean green,
                             GLboolean blue, GLboolean alpha )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ColorMask)(CC, red, green, blue, alpha);
}


API_PREFIX void API_NAME(ColorMaterial)(CTX_PREFIX  GLenum face, GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ColorMaterial)(CC, face, mode);
}


#if 0
API_PREFIX void API_NAME(ColorPointer)(CTX_PREFIX  GLint size, GLenum type, 
                                          GLsizei stride, const GLvoid *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ColorPointer)(CC, size, type, stride, ptr);
}
#endif

API_PREFIX void API_NAME(ColorTableEXT)(CTX_PREFIX  GLenum target, GLenum internalFormat,
                                 GLsizei width, GLenum format, GLenum type,
                                 const GLvoid *table )
{
   struct gl_image *image;
   GET_CONTEXT;
   CHECK_CONTEXT;
   image = gl_unpack_image( CC, width, 1, format, type, table );
   (*CC->API.ColorTable)( CC, target, internalFormat, image );
   if (image->RefCount == 0)
      gl_free_image(image);
}


API_PREFIX void API_NAME(ColorSubTableEXT)(CTX_PREFIX  GLenum target, GLsizei start,
                                    GLsizei count, GLenum format, GLenum type,
                                    const GLvoid *data )
{
   struct gl_image *image;
   GET_CONTEXT;
   CHECK_CONTEXT;
   image = gl_unpack_image( CC, count, 1, format, type, data );
   (*CC->API.ColorSubTable)( CC, target, start, image );
   if (image->RefCount == 0)
      gl_free_image(image);
}



API_PREFIX void API_NAME(CopyPixels)(CTX_PREFIX  GLint x, GLint y, GLsizei width, 
                                        GLsizei height, GLenum type )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CopyPixels)(CC, x, y, width, height, type);
}


API_PREFIX void API_NAME(CopyTexImage1D)(CTX_PREFIX  GLenum target, GLint level,
                                  GLenum internalformat,
                                  GLint x, GLint y,
                                  GLsizei width, GLint border )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CopyTexImage1D)( CC, target, level, internalformat,
                              x, y, width, border );
}


API_PREFIX void API_NAME(CopyTexImage2D)(CTX_PREFIX  GLenum target, GLint level,
                                  GLenum internalformat,
                                  GLint x, GLint y,
                                  GLsizei width, GLsizei height, GLint border )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CopyTexImage2D)( CC, target, level, internalformat,
                              x, y, width, height, border );
}


API_PREFIX void API_NAME(CopyTexSubImage1D)(CTX_PREFIX  GLenum target, GLint level,
                                     GLint xoffset, GLint x, GLint y,
                                     GLsizei width )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CopyTexSubImage1D)( CC, target, level, xoffset, x, y, width );
}


API_PREFIX void API_NAME(CopyTexSubImage2D)(CTX_PREFIX  GLenum target, GLint level,
                                     GLint xoffset, GLint yoffset,
                                     GLint x, GLint y,
                                     GLsizei width, GLsizei height )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CopyTexSubImage2D)( CC, target, level, xoffset, yoffset,
                                 x, y, width, height );
}


/* 1.2 */
API_PREFIX void API_NAME(CopyTexSubImage3D)(CTX_PREFIX  GLenum target, GLint level, 
                                    GLint xoffset, GLint yoffset, GLint zoffset,
                                    GLint x, GLint y, GLsizei width,
                                    GLsizei height )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CopyTexSubImage3DEXT)( CC, target, level, xoffset, yoffset,
                                    zoffset, x, y, width, height );
}



API_PREFIX void API_NAME(CullFace)(CTX_PREFIX  GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CullFace)(CC, mode);
}


API_PREFIX void API_NAME(DepthFunc)(CTX_PREFIX  GLenum func )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DepthFunc)( CC, func );
}


API_PREFIX void API_NAME(DepthMask)(CTX_PREFIX  GLboolean flag )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DepthMask)( CC, flag );
}


API_PREFIX void API_NAME(DepthRange)(CTX_PREFIX  GLclampd near_val, GLclampd far_val )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DepthRange)( CC, near_val, far_val );
}


API_PREFIX void API_NAME(DeleteLists)(CTX_PREFIX  GLuint list, GLsizei range )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DeleteLists)(CC, list, range);
}


API_PREFIX void API_NAME(DeleteTextures)(CTX_PREFIX  GLsizei n, const GLuint *textures )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DeleteTextures)(CC, n, textures);
}


API_PREFIX void API_NAME(Disable)(CTX_PREFIX  GLenum cap )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Disable)( CC, cap );
}


API_PREFIX void API_NAME(DisableClientState)(CTX_PREFIX  GLenum cap )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DisableClientState)( CC, cap );
}


API_PREFIX void API_NAME(DrawArrays)(CTX_PREFIX  GLenum mode, GLint first, GLsizei count )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   gl_DrawArrays(CC, mode, first, count);
}


API_PREFIX void API_NAME(DrawBuffer)(CTX_PREFIX  GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DrawBuffer)(CC, mode);
}


#if 0
API_PREFIX void API_NAME(DrawElements)(CTX_PREFIX  GLenum mode, GLsizei count,
                                GLenum type, const GLvoid *indices )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   gl_DrawElements( CC, mode, count, type, indices );
}
#endif


API_PREFIX void API_NAME(DrawPixels)(CTX_PREFIX  GLsizei width, GLsizei height, 
                                        GLenum format,GLenum type, 
                                        const GLvoid *pixels )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   if (CC->CompileFlag || !gl_direct_DrawPixels(CC, &CC->Unpack, width, height,
                                                format, type, pixels)) {
      struct gl_image *image;
      image = gl_unpack_image( CC, width, height, format, type, pixels );
      (*CC->API.DrawPixels)( CC, image );
      if (image->RefCount==0) {
         /* image not in display list */
         gl_free_image( image );
      }
   }
}


#if 0
/* GL_VERSION_1_2 */
API_PREFIX void API_NAME(DrawRangeElements)(CTX_PREFIX  GLenum mode, GLuint start, 
                                               GLuint end,  GLsizei count, 
                                               GLenum type,
                                               const GLvoid *indices )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   gl_DrawRangeElements( CC, mode, start, end, count, type, indices );
}
#endif


API_PREFIX void API_NAME(Enable)(CTX_PREFIX  GLenum cap )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Enable)( CC, cap );
}


API_PREFIX void API_NAME(EnableClientState)(CTX_PREFIX  GLenum cap )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EnableClientState)( CC, cap );
}


/* KW: Both streams now go to the outside-begin-end state.  Raise
 *     errors for either stream if it was not in the inside state.
 */
API_PREFIX void API_NAME(End)(CTX_VOID  )
{
   GLuint state;
   GLuint inflags;
   GET_IMMEDIATE;

   if (MESA_VERBOSE&VERBOSE_API)
      fprintf(stderr, "glEnd\n");

   state = IM->BeginState;
   inflags = (~state) & (VERT_BEGIN_0|VERT_BEGIN_1);
   state |= inflags << 2;	/* errors */
   
   if (inflags != (VERT_BEGIN_0|VERT_BEGIN_1))
   {
      GLuint count = IM->Count;
      state &= ~(VERT_BEGIN_0|VERT_BEGIN_1); /* update state */
      IM->Flag[count] |= VERT_END;
      IM->NextPrimitive[IM->LastPrimitive] = count;
      IM->LastPrimitive = count;
      IM->Primitive[count] = GL_POLYGON+1;
   }

   IM->BeginState = state;      

   if (IM->Flush || (MESA_DEBUG_FLAGS&DEBUG_ALWAYS_FLUSH))
      IM->maybe_transform_vb( IM );
}


void gl_End( GLcontext *ctx )
{
   struct immediate *IM = ctx->input;
   GLuint state = IM->BeginState;
   GLuint inflags = (~state) & (VERT_BEGIN_0|VERT_BEGIN_1);

   state |= inflags << 2;	/* errors */

   if (inflags != (VERT_BEGIN_0|VERT_BEGIN_1))
   {
      GLuint count = IM->Count;
      state &= ~(VERT_BEGIN_0|VERT_BEGIN_1); /* update state */
      IM->Flag[count] |= VERT_END;
      IM->NextPrimitive[IM->LastPrimitive] = count;
      IM->LastPrimitive = count;
      IM->Primitive[count] = GL_POLYGON+1;
   }

   IM->BeginState = state;      

   /* You can set this flag to get the old 'flush vb on glEnd()'
    * behaviour.
    */
   if (IM->Flush || (MESA_DEBUG_FLAGS&DEBUG_ALWAYS_FLUSH))
      IM->maybe_transform_vb( IM );
}

API_PREFIX void API_NAME(EndList)(CTX_VOID  )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EndList)(CC);
}

/* KW: If are compiling, we don't know whether eval will produce a
 *     vertex when it is run in the future.  If this is pure immediate
 *     mode, eval is a noop if neither vertex map is enabled.
 *
 *     Thus we need to have a check in the display list code or
 *     elsewhere for eval(1,2) vertices in the case where
 *     map(1,2)_vertex is disabled, and to purge those vertices from
 *     the vb.  This is currently done
 *     via  modifications to the cull_vb and render_vb operations, and
 *     by using the existing cullmask mechanism for all other operations.  
 */


/* KW: Because the eval values don't become 'current', fixup will flow
 *     through these vertices, and then evaluation will write on top
 *     of the fixup results.  
 *
 *     This is a little inefficient, but at least it is correct.  This
 *     could be short-circuited in the case where all vertices are
 *     eval-vertices, or more generally by a cullmask in fixup.
 *
 *     Note: using Obj to hold eval coord data.  This data is actually
 *     transformed if eval is disabled.  But disabling eval & sending
 *     eval coords is stupid, right?
 */


#define EVALCOORD1(IM, x)				\
{							\
   GLuint count = IM->Count++;				\
   IM->Flag[count] |= VERT_EVAL_C1;			\
   ASSIGN_4V(IM->Obj[count], x, 0, 0, 1);		\
   if (count == VB_MAX-1)				\
      IM->maybe_transform_vb( IM );			\
}

#define EVALCOORD2(IM, x, y)				\
{							\
   GLuint count = IM->Count++;				\
   IM->Flag[count] |= VERT_EVAL_C2;			\
   ASSIGN_4V(IM->Obj[count], x, y, 0, 1);		\
   if (count == VB_MAX-1)				\
      IM->maybe_transform_vb( IM );			\
}

#define EVALPOINT1(IM, x)				\
{							\
   GLuint count = IM->Count++;				\
   IM->Flag[count] |= VERT_EVAL_P1;			\
   ASSIGN_4V(IM->Obj[count], x, 0, 0, 1);		\
   if (count == VB_MAX-1)				\
      IM->maybe_transform_vb( IM );			\
}
 
#define EVALPOINT2(IM, x, y)				\
{							\
   GLuint count = IM->Count++;				\
   IM->Flag[count] |= VERT_EVAL_P2;			\
   ASSIGN_4V(IM->Obj[count], x, y, 0, 1);		\
   if (count == VB_MAX-1)				\
      IM->maybe_transform_vb( IM );			\
}


API_PREFIX void API_NAME(EvalCoord1d)(CTX_PREFIX  GLdouble u )
{
   GET_IMMEDIATE;
   EVALCOORD1( IM, (GLfloat) u );
}


API_PREFIX void API_NAME(EvalCoord1f)(CTX_PREFIX  GLfloat u )
{
   GET_IMMEDIATE;
   EVALCOORD1( IM, u );
}


/* Lame internal function:
 */
void gl_EvalCoord1f( GLcontext *CC, GLfloat u )
{
   struct immediate *i = CC->input;
   EVALCOORD1( i, u );
}


API_PREFIX void API_NAME(EvalCoord1dv)(CTX_PREFIX  const GLdouble *u )
{
   GET_IMMEDIATE;
   EVALCOORD1( IM, (GLfloat) *u );
}


API_PREFIX void API_NAME(EvalCoord1fv)(CTX_PREFIX  const GLfloat *u )
{
   GET_IMMEDIATE;
   EVALCOORD1( IM, (GLfloat) *u );
}


API_PREFIX void API_NAME(EvalCoord2d)(CTX_PREFIX  GLdouble u, GLdouble v )
{
   GET_IMMEDIATE;
   EVALCOORD2( IM, (GLfloat) u, (GLfloat) v );
}


API_PREFIX void API_NAME(EvalCoord2f)(CTX_PREFIX  GLfloat u, GLfloat v )
{
   GET_IMMEDIATE;
   EVALCOORD2( IM, u, v );
}


/* Lame internal function:
 */
void gl_EvalCoord2f( GLcontext *CC, GLfloat u, GLfloat v )
{
   struct immediate *i = CC->input;
   EVALCOORD2( i, u, v );
}


API_PREFIX void API_NAME(EvalCoord2dv)(CTX_PREFIX  const GLdouble *u )
{
   GET_IMMEDIATE;
   EVALCOORD2( IM, (GLfloat) u[0], (GLfloat) u[1] );
}


API_PREFIX void API_NAME(EvalCoord2fv)(CTX_PREFIX  const GLfloat *u )
{
   GET_IMMEDIATE;
   EVALCOORD2( IM, u[0], u[1] );
}


API_PREFIX void API_NAME(EvalPoint1)(CTX_PREFIX  GLint i )
{
   GET_IMMEDIATE;
   EVALPOINT1( IM, i );
}


API_PREFIX void API_NAME(EvalPoint2)(CTX_PREFIX  GLint i, GLint j )
{
   GET_IMMEDIATE;
   EVALPOINT2( IM, i, j );
}


API_PREFIX void API_NAME(EvalMesh1)(CTX_PREFIX  GLenum mode, GLint i1, GLint i2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalMesh1)( CC, mode, i1, i2 );
}


API_PREFIX void API_NAME(EdgeFlag)(CTX_PREFIX  GLboolean flag )
{
   GLuint count;
   GET_IMMEDIATE;
   count = IM->Count;
   IM->EdgeFlag[count] = flag;
   IM->Flag[count] |= VERT_EDGE;
}


API_PREFIX void API_NAME(EdgeFlagv)(CTX_PREFIX  const GLboolean *flag )
{
   GLuint count;
   GET_IMMEDIATE;
   count = IM->Count;
   IM->EdgeFlag[count] = *flag;
   IM->Flag[count] |= VERT_EDGE;
}


#if 0
API_PREFIX void API_NAME(EdgeFlagPointer)(CTX_PREFIX  GLsizei stride, const GLvoid *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EdgeFlagPointer)(CC, stride, (const GLboolean *) ptr);
}
#endif


API_PREFIX void API_NAME(EvalMesh2)(CTX_PREFIX  GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalMesh2)( CC, mode, i1, i2, j1, j2 );
}


API_PREFIX void API_NAME(FeedbackBuffer)(CTX_PREFIX  GLsizei size, GLenum type, 
                                            GLfloat *buffer )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.FeedbackBuffer)(CC, size, type, buffer);
}


API_PREFIX void API_NAME(Finish)( CTX_VOID )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Finish)(CC);
}


API_PREFIX void API_NAME(Flush)( CTX_VOID )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Flush)(CC);
}


API_PREFIX void API_NAME(Fogf)(CTX_PREFIX  GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Fogfv)(CC, pname, &param);
}


API_PREFIX void API_NAME(Fogi)(CTX_PREFIX  GLenum pname, GLint param )
{
   GLfloat fparam = (GLfloat) param;
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Fogfv)(CC, pname, &fparam);
}


API_PREFIX void API_NAME(Fogfv)(CTX_PREFIX  GLenum pname, const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Fogfv)(CC, pname, params);
}


API_PREFIX void API_NAME(Fogiv)(CTX_PREFIX  GLenum pname, const GLint *params )
{
   GLfloat p[4];
   GET_CONTEXT;
   CHECK_CONTEXT;

   switch (pname) {
      case GL_FOG_MODE:
      case GL_FOG_DENSITY:
      case GL_FOG_START:
      case GL_FOG_END:
      case GL_FOG_INDEX:
	 p[0] = (GLfloat) *params;
	 break;
      case GL_FOG_COLOR:
	 p[0] = INT_TO_FLOAT( params[0] );
	 p[1] = INT_TO_FLOAT( params[1] );
	 p[2] = INT_TO_FLOAT( params[2] );
	 p[3] = INT_TO_FLOAT( params[3] );
	 break;
      default:
         /* Error will be caught later in gl_Fogfv */
         ;
   }
   (*CC->API.Fogfv)( CC, pname, p );
}


API_PREFIX void API_NAME(FrontFace)(CTX_PREFIX  GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.FrontFace)(CC, mode);
}


API_PREFIX void API_NAME(Frustum)(CTX_PREFIX  GLdouble left, GLdouble right,
                                     GLdouble bottom, GLdouble top,
                                     GLdouble nearval, GLdouble farval )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Frustum)(CC, left, right, bottom, top, nearval, farval);
}


API_PREFIX GLuint API_NAME(GenLists)(CTX_PREFIX  GLsizei range )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(0);
   return (*CC->API.GenLists)(CC, range);
}


API_PREFIX void API_NAME(GenTextures)(CTX_PREFIX  GLsizei n, GLuint *textures )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GenTextures)(CC, n, textures);
}


API_PREFIX void API_NAME(GetBooleanv)(CTX_PREFIX  GLenum pname, GLboolean *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetBooleanv)(CC, pname, params);
}


API_PREFIX void API_NAME(GetClipPlane)(CTX_PREFIX  GLenum plane, GLdouble *equation )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetClipPlane)(CC, plane, equation);
}


API_PREFIX void API_NAME(GetColorTableEXT)(CTX_PREFIX  GLenum target, GLenum format,
                                              GLenum type, GLvoid *table )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetColorTable)(CC, target, format, type, table);
}


API_PREFIX void API_NAME(GetColorTableParameterivEXT)(CTX_PREFIX  GLenum target, 
                                                         GLenum pname,
                                                         GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetColorTableParameteriv)(CC, target, pname, params);
}


API_PREFIX void API_NAME(GetColorTableParameterfvEXT)(CTX_PREFIX  GLenum target, 
                                                         GLenum pname,
                                                         GLfloat *params )
{
   GLint iparams;
   glGetColorTableParameterivEXT( target, pname, &iparams );
   *params = (GLfloat) iparams;
}


API_PREFIX void API_NAME(GetDoublev)(CTX_PREFIX  GLenum pname, GLdouble *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetDoublev)(CC, pname, params);
}


API_PREFIX GLenum API_NAME(GetError)(CTX_VOID  )
{
   GET_CONTEXT;
   if (!CC) {
      /* No current context */
      return (GLenum) GL_NO_ERROR;
   }
   return (*CC->API.GetError)(CC);
}


API_PREFIX void API_NAME(GetFloatv)(CTX_PREFIX  GLenum pname, GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetFloatv)(CC, pname, params);
}


API_PREFIX void API_NAME(GetIntegerv)(CTX_PREFIX  GLenum pname, GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetIntegerv)(CC, pname, params);
}


API_PREFIX void API_NAME(GetLightfv)(CTX_PREFIX  GLenum light, GLenum pname, 
                                        GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetLightfv)(CC, light, pname, params);
}


API_PREFIX void API_NAME(GetLightiv)(CTX_PREFIX  GLenum light, GLenum pname, 
                                        GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetLightiv)(CC, light, pname, params);
}


API_PREFIX void API_NAME(GetMapdv)(CTX_PREFIX  GLenum target, GLenum query, 
                                      GLdouble *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetMapdv)( CC, target, query, v );
}


API_PREFIX void API_NAME(GetMapfv)(CTX_PREFIX  GLenum target, GLenum query, GLfloat *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetMapfv)( CC, target, query, v );
}


API_PREFIX void API_NAME(GetMapiv)(CTX_PREFIX  GLenum target, GLenum query, GLint *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetMapiv)( CC, target, query, v );
}


API_PREFIX void API_NAME(GetMaterialfv)(CTX_PREFIX  GLenum face, GLenum pname, 
                                           GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetMaterialfv)(CC, face, pname, params);
}


API_PREFIX void API_NAME(GetMaterialiv)(CTX_PREFIX  GLenum face, GLenum pname, 
                                           GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetMaterialiv)(CC, face, pname, params);
}


API_PREFIX void API_NAME(GetPixelMapfv)(CTX_PREFIX  GLenum map, GLfloat *values )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetPixelMapfv)(CC, map, values);
}


API_PREFIX void API_NAME(GetPixelMapuiv)(CTX_PREFIX  GLenum map, GLuint *values )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetPixelMapuiv)(CC, map, values);
}


API_PREFIX void API_NAME(GetPixelMapusv)(CTX_PREFIX  GLenum map, GLushort *values )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetPixelMapusv)(CC, map, values);
}


API_PREFIX void API_NAME(GetPointerv)(CTX_PREFIX  GLenum pname, GLvoid **params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetPointerv)(CC, pname, params);
}


API_PREFIX void API_NAME(GetPolygonStipple)(CTX_PREFIX  GLubyte *mask )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetPolygonStipple)(CC, mask);
}


API_PREFIX const GLubyte * API_NAME(GetString)(CTX_PREFIX  GLenum name )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(NULL);
   return (*CC->API.GetString)(CC, name);
}


API_PREFIX void API_NAME(GetTexEnvfv)(CTX_PREFIX  GLenum target, GLenum pname, 
                                         GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexEnvfv)(CC, target, pname, params);
}


API_PREFIX void API_NAME(GetTexEnviv)(CTX_PREFIX  GLenum target, GLenum pname, 
                                         GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexEnviv)(CC, target, pname, params);
}


API_PREFIX void API_NAME(GetTexGeniv)(CTX_PREFIX  GLenum coord, GLenum pname, 
                                         GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexGeniv)(CC, coord, pname, params);
}


API_PREFIX void API_NAME(GetTexGendv)(CTX_PREFIX  GLenum coord, GLenum pname, 
					 GLdouble *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexGendv)(CC, coord, pname, params);
}


API_PREFIX void API_NAME(GetTexGenfv)(CTX_PREFIX  GLenum coord, GLenum pname, 
                                         GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexGenfv)(CC, coord, pname, params);
}


API_PREFIX void API_NAME(GetTexImage)(CTX_PREFIX  GLenum target, GLint level, 
                                         GLenum format, GLenum type, 
                                         GLvoid *pixels )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexImage)(CC, target, level, format, type, pixels);
}


API_PREFIX void API_NAME(GetTexLevelParameterfv)(CTX_PREFIX  GLenum target, GLint level,
                                                    GLenum pname, GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexLevelParameterfv)(CC, target, level, pname, params);
}


API_PREFIX void API_NAME(GetTexLevelParameteriv)(CTX_PREFIX  GLenum target, GLint level,
                                                    GLenum pname, GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexLevelParameteriv)(CC, target, level, pname, params);
}


API_PREFIX void API_NAME(GetTexParameterfv)(CTX_PREFIX  GLenum target, GLenum pname,
                                               GLfloat *params)
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexParameterfv)(CC, target, pname, params);
}


API_PREFIX void API_NAME(GetTexParameteriv)(CTX_PREFIX  GLenum target, GLenum pname,
                                               GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexParameteriv)(CC, target, pname, params);
}


API_PREFIX void API_NAME(Hint)(CTX_PREFIX  GLenum target, GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   if (mode!=GL_DONT_CARE && mode!=GL_FASTEST && mode!=GL_NICEST) 
      (*CC->API.Error)( CC, GL_INVALID_ENUM, "glHint(mode)" );
   else 
      (*CC->API.Hint)(CC, target, mode);
}


#define INDEX( c )				\
{						\
   GLuint count;				\
   GET_IMMEDIATE;				\
   count = IM->Count;				\
   IM->Index[count] = c;			\
   IM->Flag[count] |= VERT_INDEX;		\
}


API_PREFIX void API_NAME(Indexd)(CTX_PREFIX  GLdouble c )
{
   INDEX( (GLuint) (GLint) c );
}


API_PREFIX void API_NAME(Indexf)(CTX_PREFIX  GLfloat c )
{
   INDEX( (GLuint) (GLint) c );
}


API_PREFIX void API_NAME(Indexi)(CTX_PREFIX  GLint c )
{
   INDEX( (GLuint) c );
}


API_PREFIX void API_NAME(Indexs)(CTX_PREFIX  GLshort c )
{
   INDEX( (GLuint) c );
}


/* GL_VERSION_1_1 */
API_PREFIX void API_NAME(Indexub)(CTX_PREFIX  GLubyte c )
{
   INDEX( (GLuint) c );
}


API_PREFIX void API_NAME(Indexdv)(CTX_PREFIX  const GLdouble *c )
{
   INDEX( (GLuint) (GLint) *c );
}


API_PREFIX void API_NAME(Indexfv)(CTX_PREFIX  const GLfloat *c )
{
   INDEX( (GLuint) (GLint) *c );
}


API_PREFIX void API_NAME(Indexiv)(CTX_PREFIX  const GLint *c )
{
   INDEX( *c );
}


API_PREFIX void API_NAME(Indexsv)(CTX_PREFIX  const GLshort *c )
{
   INDEX( (GLuint) (GLint) *c );
}


/* GL_VERSION_1_1 */
API_PREFIX void API_NAME(Indexubv)(CTX_PREFIX  const GLubyte *c )
{
   INDEX( (GLuint) *c );
}


API_PREFIX void API_NAME(IndexMask)(CTX_PREFIX  GLuint mask )
{
   GET_CONTEXT;
   (*CC->API.IndexMask)(CC, mask);
}


#if 0
API_PREFIX void API_NAME(IndexPointer)(CTX_PREFIX  GLenum type, GLsizei stride, const GLvoid *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.IndexPointer)(CC, type, stride, ptr);
}
#endif

#if 0
API_PREFIX void API_NAME(InterleavedArrays)(CTX_PREFIX  GLenum format, GLsizei stride,
                                               const GLvoid *pointer )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   gl_InterleavedArrays( CC, format, stride, pointer );
}
#endif

API_PREFIX void API_NAME(InitNames)(CTX_VOID  )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.InitNames)(CC);
}


API_PREFIX GLboolean API_NAME(IsList)(CTX_PREFIX  GLuint list )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(GL_FALSE);
   return (*CC->API.IsList)(CC, list);
}


API_PREFIX GLboolean API_NAME(IsTexture)(CTX_PREFIX  GLuint texture )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(GL_FALSE);
   return (*CC->API.IsTexture)(CC, texture);
}


API_PREFIX void API_NAME(Lightf)(CTX_PREFIX  GLenum light, GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Lightfv)( CC, light, pname, &param, 1 );
}


API_PREFIX void API_NAME(Lighti)(CTX_PREFIX  GLenum light, GLenum pname, GLint param )
{
   GLfloat fparam = (GLfloat) param;
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Lightfv)( CC, light, pname, &fparam, 1 );
}


API_PREFIX void API_NAME(Lightfv)(CTX_PREFIX  GLenum light, GLenum pname, 
                                     const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Lightfv)( CC, light, pname, params, 4 );
}


API_PREFIX void API_NAME(Lightiv)(CTX_PREFIX  GLenum light, GLenum pname, 
                                     const GLint *params )
{
   GLfloat fparam[4];
   GET_CONTEXT;
   CHECK_CONTEXT;

   switch (pname) {
      case GL_AMBIENT:
      case GL_DIFFUSE:
      case GL_SPECULAR:
         fparam[0] = INT_TO_FLOAT( params[0] );
         fparam[1] = INT_TO_FLOAT( params[1] );
         fparam[2] = INT_TO_FLOAT( params[2] );
         fparam[3] = INT_TO_FLOAT( params[3] );
         break;
      case GL_POSITION:
         fparam[0] = (GLfloat) params[0];
         fparam[1] = (GLfloat) params[1];
         fparam[2] = (GLfloat) params[2];
         fparam[3] = (GLfloat) params[3];
         break;
      case GL_SPOT_DIRECTION:
         fparam[0] = (GLfloat) params[0];
         fparam[1] = (GLfloat) params[1];
         fparam[2] = (GLfloat) params[2];
         break;
      case GL_SPOT_EXPONENT:
      case GL_SPOT_CUTOFF:
      case GL_CONSTANT_ATTENUATION:
      case GL_LINEAR_ATTENUATION:
      case GL_QUADRATIC_ATTENUATION:
         fparam[0] = (GLfloat) params[0];
         break;
      default:
         /* error will be caught later in gl_Lightfv */
         ;
   }
   (*CC->API.Lightfv)( CC, light, pname, fparam, 4 );
}


API_PREFIX void API_NAME(LightModelf)(CTX_PREFIX  GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LightModelfv)( CC, pname, &param );
}


API_PREFIX void API_NAME(LightModeli)(CTX_PREFIX  GLenum pname, GLint param )
{
   GLfloat fparam[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   fparam[0] = (GLfloat) param;
   (*CC->API.LightModelfv)( CC, pname, fparam );
}


API_PREFIX void API_NAME(LightModelfv)(CTX_PREFIX  GLenum pname, const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LightModelfv)( CC, pname, params );
}


API_PREFIX void API_NAME(LightModeliv)(CTX_PREFIX  GLenum pname, const GLint *params )
{
   GLfloat fparam[4];
   GET_CONTEXT;
   CHECK_CONTEXT;

   switch (pname) {
      case GL_LIGHT_MODEL_AMBIENT:
         fparam[0] = INT_TO_FLOAT( params[0] );
         fparam[1] = INT_TO_FLOAT( params[1] );
         fparam[2] = INT_TO_FLOAT( params[2] );
         fparam[3] = INT_TO_FLOAT( params[3] );
         break;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
      case GL_LIGHT_MODEL_TWO_SIDE:
         fparam[0] = (GLfloat) params[0];
         break;
      default:
         /* Error will be caught later in gl_LightModelfv */
         ;
   }
   (*CC->API.LightModelfv)( CC, pname, fparam );
}


API_PREFIX void API_NAME(LineWidth)(CTX_PREFIX  GLfloat width )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LineWidth)(CC, width);
}


API_PREFIX void API_NAME(LineStipple)(CTX_PREFIX  GLint factor, GLushort pattern )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LineStipple)(CC, factor, pattern);
}


API_PREFIX void API_NAME(ListBase)(CTX_PREFIX  GLuint base )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ListBase)(CC, base);
}


API_PREFIX void API_NAME(LoadIdentity)( CTX_VOID )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LoadIdentity)( CC );
}


API_PREFIX void API_NAME(LoadMatrixd)(CTX_PREFIX  const GLdouble *m )
{
   GLfloat fm[16];
   GLuint i;
   GET_CONTEXT;
   CHECK_CONTEXT;

   for (i=0;i<16;i++) {
	  fm[i] = (GLfloat) m[i];
   }

   (*CC->API.LoadMatrixf)( CC, fm );
}


API_PREFIX void API_NAME(LoadMatrixf)(CTX_PREFIX  const GLfloat *m )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LoadMatrixf)( CC, m );
}


API_PREFIX void API_NAME(LoadName)(CTX_PREFIX  GLuint name )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LoadName)(CC, name);
}


API_PREFIX void API_NAME(LogicOp)(CTX_PREFIX  GLenum opcode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LogicOp)(CC, opcode);
}



API_PREFIX void API_NAME(Map1d)(CTX_PREFIX  GLenum target, GLdouble u1, GLdouble u2, 
                                   GLint stride, GLint order, 
                                   const GLdouble *points )
{
   GLfloat *pnts;
   GLboolean retain;
   GET_CONTEXT;
   CHECK_CONTEXT;

   pnts = gl_copy_map_points1d( target, stride, order, points );
   retain = CC->CompileFlag;
   (*CC->API.Map1f)( CC, target, u1, u2, stride, order, pnts, retain );
}


API_PREFIX void API_NAME(Map1f)(CTX_PREFIX  GLenum target, GLfloat u1, GLfloat u2, 
                                   GLint stride,  GLint order, 
                                   const GLfloat *points )
{
   GLfloat *pnts;
   GLboolean retain;
   GET_CONTEXT;
   CHECK_CONTEXT;

   pnts = gl_copy_map_points1f( target, stride, order, points );
   retain = CC->CompileFlag;
   (*CC->API.Map1f)( CC, target, u1, u2, stride, order, pnts, retain );
}


API_PREFIX void API_NAME(Map2d)(CTX_PREFIX  GLenum target,
                         GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
                         GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
                         const GLdouble *points )
{
   GLfloat *pnts;
   GLboolean retain;
   GET_CONTEXT;
   CHECK_CONTEXT;

   pnts = gl_copy_map_points2d( target, ustride, uorder,
                                vstride, vorder, points );
   retain = CC->CompileFlag;
   (*CC->API.Map2f)( CC, target, u1, u2, ustride, uorder,
                     v1, v2, vstride, vorder, pnts, retain );
}


API_PREFIX void API_NAME(Map2f)(CTX_PREFIX  GLenum target,
                         GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
                         GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
                         const GLfloat *points )
{
   GLfloat *pnts;
   GLboolean retain;
   GET_CONTEXT;
   CHECK_CONTEXT;

   pnts = gl_copy_map_points2f( target, ustride, uorder,
                                vstride, vorder, points );
   retain = CC->CompileFlag;
   (*CC->API.Map2f)( CC, target, u1, u2, ustride, uorder,
                     v1, v2, vstride, vorder, pnts, retain );
}


API_PREFIX void API_NAME(MapGrid1d)(CTX_PREFIX  GLint un, GLdouble u1, GLdouble u2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.MapGrid1f)( CC, un, (GLfloat) u1, (GLfloat) u2 );
}


API_PREFIX void API_NAME(MapGrid1f)(CTX_PREFIX  GLint un, GLfloat u1, GLfloat u2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.MapGrid1f)( CC, un, u1, u2 );
}


API_PREFIX void API_NAME(MapGrid2d)(CTX_PREFIX  GLint un, GLdouble u1, GLdouble u2,
                                       GLint vn, GLdouble v1, GLdouble v2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.MapGrid2f)( CC, un, (GLfloat) u1, (GLfloat) u2,
                         vn, (GLfloat) v1, (GLfloat) v2 );
}


API_PREFIX void API_NAME(MapGrid2f)(CTX_PREFIX  GLint un, GLfloat u1, GLfloat u2,
                                       GLint vn, GLfloat v1, GLfloat v2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.MapGrid2f)( CC, un, u1, u2, vn, v1, v2 );
}


API_PREFIX void API_NAME(Materialf)(CTX_PREFIX  GLenum face, GLenum pname, 
				       GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   gl_Materialfv( CC, face, pname, &param );
}



API_PREFIX void API_NAME(Materiali)(CTX_PREFIX  GLenum face, GLenum pname, GLint param )
{
   GLfloat fparam[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   fparam[0] = (GLfloat) param;
   gl_Materialfv( CC, face, pname, fparam );
}


API_PREFIX void API_NAME(Materialfv)(CTX_PREFIX  GLenum face, GLenum pname, 
					const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   gl_Materialfv( CC, face, pname, params );
}


API_PREFIX void API_NAME(Materialiv)(CTX_PREFIX  GLenum face, GLenum pname, 
				        const GLint *params )
{
   GLfloat fparam[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   switch (pname) {
      case GL_AMBIENT:
      case GL_DIFFUSE:
      case GL_SPECULAR:
      case GL_EMISSION:
      case GL_AMBIENT_AND_DIFFUSE:
         fparam[0] = INT_TO_FLOAT( params[0] );
         fparam[1] = INT_TO_FLOAT( params[1] );
         fparam[2] = INT_TO_FLOAT( params[2] );
         fparam[3] = INT_TO_FLOAT( params[3] );
         break;
      case GL_SHININESS:
         fparam[0] = (GLfloat) params[0];
         break;
      case GL_COLOR_INDEXES:
         fparam[0] = (GLfloat) params[0];
         fparam[1] = (GLfloat) params[1];
         fparam[2] = (GLfloat) params[2];
         break;
      default:
         /* Error will be caught later in gl_Materialfv */
         ;
   }
   gl_Materialfv( CC, face, pname, fparam );
}


API_PREFIX void API_NAME(MatrixMode)(CTX_PREFIX  GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.MatrixMode)( CC, mode );
}


API_PREFIX void API_NAME(MultMatrixd)(CTX_PREFIX  const GLdouble *m )
{
   GLfloat fm[16];
   GLuint i;
   GET_CONTEXT;
   CHECK_CONTEXT;

   for (i=0;i<16;i++) {
	  fm[i] = (GLfloat) m[i];
   }

   (*CC->API.MultMatrixf)( CC, fm );
}


API_PREFIX void API_NAME(MultMatrixf)(CTX_PREFIX  const GLfloat *m )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.MultMatrixf)( CC, m );
}


API_PREFIX void API_NAME(NewList)(CTX_PREFIX  GLuint list, GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.NewList)(CC, list, mode);
}


/* KW: Now that we build buffers for display lists the same way we
 *     fill the vb, we can do the work here without a second function
 *     call.  The Flag member allows the identification of missing
 *     (ie shared) normals.
 */
#define NORMAL( x,y,z )				\
{						\
   GLuint count;				\
   GLfloat *normal;				\
   GET_IMMEDIATE;				\
   count = IM->Count;				\
   IM->Flag[count] |= VERT_NORM;		\
   normal = IM->Normal[count];			\
   ASSIGN_3V(normal, x,y,z);			\
}


API_PREFIX void API_NAME(Normal3b)(CTX_PREFIX  GLbyte nx, GLbyte ny, GLbyte nz )
{
   NORMAL( BYTE_TO_FLOAT(nx),
	   BYTE_TO_FLOAT(ny),
	   BYTE_TO_FLOAT(nz) );
}


API_PREFIX void API_NAME(Normal3d)(CTX_PREFIX  GLdouble nx, GLdouble ny, GLdouble nz )
{
   NORMAL(nx,ny,nz);
}


API_PREFIX void API_NAME(Normal3f)(CTX_PREFIX  GLfloat nx, GLfloat ny, GLfloat nz )
{
   NORMAL(nx,ny,nz);
}


API_PREFIX void API_NAME(Normal3i)(CTX_PREFIX  GLint nx, GLint ny, GLint nz )
{
   NORMAL( INT_TO_FLOAT(nx),
	   INT_TO_FLOAT(ny),
	   INT_TO_FLOAT(nz) );
}


API_PREFIX void API_NAME(Normal3s)(CTX_PREFIX  GLshort nx, GLshort ny, GLshort nz )
{
   NORMAL( SHORT_TO_FLOAT(nx),
	   SHORT_TO_FLOAT(ny),
	   SHORT_TO_FLOAT(nz) );
}


API_PREFIX void API_NAME(Normal3bv)(CTX_PREFIX  const GLbyte *v )
{
   NORMAL( BYTE_TO_FLOAT(v[0]),
	   BYTE_TO_FLOAT(v[1]),
	   BYTE_TO_FLOAT(v[2]) );
}


API_PREFIX void API_NAME(Normal3dv)(CTX_PREFIX  const GLdouble *v )
{
   NORMAL( v[0], v[1], v[2] );
}


API_PREFIX void API_NAME(Normal3fv)(CTX_PREFIX  const GLfloat *v )
{
   NORMAL( v[0], v[1], v[2] );
}


API_PREFIX void API_NAME(Normal3iv)(CTX_PREFIX  const GLint *v )
{
   NORMAL( INT_TO_FLOAT(v[0]),
	   INT_TO_FLOAT(v[1]), 
	   INT_TO_FLOAT(v[2]) );
}


API_PREFIX void API_NAME(Normal3sv)(CTX_PREFIX  const GLshort *v )
{
   NORMAL( SHORT_TO_FLOAT(v[0]),
	   SHORT_TO_FLOAT(v[1]),
	   SHORT_TO_FLOAT(v[2]) );
}


