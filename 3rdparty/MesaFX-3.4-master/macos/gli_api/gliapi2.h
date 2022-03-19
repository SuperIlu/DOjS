/* $Id: gliapi2.h,v 1.1 1999/10/31 01:02:10 miklos Exp $ */

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
#include "context.h"
#include "image.h"
#include "macros.h"
#include "matrix.h"
#include "teximage.h"
#include "types.h"
#include "vb.h"
#ifdef XFree86Server
#include "GL/xf86glx.h"
#endif
#endif


/*
 * Part 2 of API functions
 */


API_PREFIX void API_NAME(Ortho)(CTX_PREFIX  GLdouble left, GLdouble right,
                         GLdouble bottom, GLdouble top,
                         GLdouble nearval, GLdouble farval )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Ortho)(CC, left, right, bottom, top, nearval, farval);
}


API_PREFIX void API_NAME(PassThrough)(CTX_PREFIX  GLfloat token )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PassThrough)(CC, token);
}


API_PREFIX void API_NAME(PixelMapfv)(CTX_PREFIX  GLenum map, GLint mapsize, const GLfloat *values )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PixelMapfv)( CC, map, mapsize, values );
}


API_PREFIX void API_NAME(PixelMapuiv)(CTX_PREFIX  GLenum map, GLint mapsize, const GLuint *values )
{
   GLfloat fvalues[MAX_PIXEL_MAP_TABLE];
   GLint i;
   GET_CONTEXT;
   CHECK_CONTEXT;

   if (map==GL_PIXEL_MAP_I_TO_I || map==GL_PIXEL_MAP_S_TO_S) {
      for (i=0;i<mapsize;i++) {
         fvalues[i] = (GLfloat) values[i];
      }
   }
   else {
      for (i=0;i<mapsize;i++) {
         fvalues[i] = UINT_TO_FLOAT( values[i] );
      }
   }
   (*CC->API.PixelMapfv)( CC, map, mapsize, fvalues );
}



API_PREFIX void API_NAME(PixelMapusv)(CTX_PREFIX  GLenum map, GLint mapsize, const GLushort *values )
{
   GLfloat fvalues[MAX_PIXEL_MAP_TABLE];
   GLint i;
   GET_CONTEXT;
   CHECK_CONTEXT;

   if (map==GL_PIXEL_MAP_I_TO_I || map==GL_PIXEL_MAP_S_TO_S) {
      for (i=0;i<mapsize;i++) {
         fvalues[i] = (GLfloat) values[i];
      }
   }
   else {
      for (i=0;i<mapsize;i++) {
         fvalues[i] = USHORT_TO_FLOAT( values[i] );
      }
   }
   (*CC->API.PixelMapfv)( CC, map, mapsize, fvalues );
}


API_PREFIX void API_NAME(PixelStoref)(CTX_PREFIX  GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PixelStorei)( CC, pname, (GLint) param );
}


API_PREFIX void API_NAME(PixelStorei)(CTX_PREFIX  GLenum pname, GLint param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PixelStorei)( CC, pname, param );
}


API_PREFIX void API_NAME(PixelTransferf)(CTX_PREFIX  GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PixelTransferf)(CC, pname, param);
}


API_PREFIX void API_NAME(PixelTransferi)(CTX_PREFIX  GLenum pname, GLint param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PixelTransferf)(CC, pname, (GLfloat) param);
}


API_PREFIX void API_NAME(PixelZoom)(CTX_PREFIX  GLfloat xfactor, GLfloat yfactor )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PixelZoom)(CC, xfactor, yfactor);
}


API_PREFIX void API_NAME(PointSize)(CTX_PREFIX  GLfloat size )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PointSize)(CC, size);
}


API_PREFIX void API_NAME(PolygonMode)(CTX_PREFIX  GLenum face, GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PolygonMode)(CC, face, mode);
}


API_PREFIX void API_NAME(PolygonOffset)(CTX_PREFIX  GLfloat factor, GLfloat units )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PolygonOffset)( CC, factor, units );
}


/* GL_EXT_polygon_offset */
API_PREFIX void API_NAME(PolygonOffsetEXT)(CTX_PREFIX  GLfloat factor, GLfloat bias )
{
   glPolygonOffset( factor, bias * DEPTH_SCALE );
}


API_PREFIX void API_NAME(PolygonStipple)(CTX_PREFIX  const GLubyte *pattern )
{
   GLuint unpackedPattern[32];
   GET_CONTEXT;
   CHECK_CONTEXT;
   gl_unpack_polygon_stipple( CC, pattern, unpackedPattern );
   (*CC->API.PolygonStipple)(CC, unpackedPattern);
}


API_PREFIX void API_NAME(PopAttrib)( CTX_VOID )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PopAttrib)(CC);
}


API_PREFIX void API_NAME(PopClientAttrib)( CTX_VOID )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PopClientAttrib)(CC);
}


API_PREFIX void API_NAME(PopMatrix)( CTX_VOID )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PopMatrix)( CC );
}


API_PREFIX void API_NAME(PopName)( CTX_VOID )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PopName)(CC);
}


API_PREFIX void API_NAME(PrioritizeTextures)(CTX_PREFIX  GLsizei n, const GLuint *textures,
                                      const GLclampf *priorities )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PrioritizeTextures)(CC, n, textures, priorities);
}


API_PREFIX void API_NAME(PushMatrix)( CTX_VOID )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PushMatrix)( CC );
}


API_PREFIX void API_NAME(RasterPos2d)(CTX_PREFIX  GLdouble x, GLdouble y )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}


API_PREFIX void API_NAME(RasterPos2f)(CTX_PREFIX  GLfloat x, GLfloat y )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}


API_PREFIX void API_NAME(RasterPos2i)(CTX_PREFIX  GLint x, GLint y )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}


API_PREFIX void API_NAME(RasterPos2s)(CTX_PREFIX  GLshort x, GLshort y )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}


API_PREFIX void API_NAME(RasterPos3d)(CTX_PREFIX  GLdouble x, GLdouble y, GLdouble z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}


API_PREFIX void API_NAME(RasterPos3f)(CTX_PREFIX  GLfloat x, GLfloat y, GLfloat z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}


API_PREFIX void API_NAME(RasterPos3i)(CTX_PREFIX  GLint x, GLint y, GLint z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}


API_PREFIX void API_NAME(RasterPos3s)(CTX_PREFIX  GLshort x, GLshort y, GLshort z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}


API_PREFIX void API_NAME(RasterPos4d)(CTX_PREFIX  GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y,
                           (GLfloat) z, (GLfloat) w );
}


API_PREFIX void API_NAME(RasterPos4f)(CTX_PREFIX  GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, x, y, z, w );
}


API_PREFIX void API_NAME(RasterPos4i)(CTX_PREFIX  GLint x, GLint y, GLint z, GLint w )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y,
                           (GLfloat) z, (GLfloat) w );
}


API_PREFIX void API_NAME(RasterPos4s)(CTX_PREFIX  GLshort x, GLshort y, GLshort z, GLshort w )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y,
                           (GLfloat) z, (GLfloat) w );
}


API_PREFIX void API_NAME(RasterPos2dv)(CTX_PREFIX  const GLdouble *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1], 0.0F, 1.0F );
}


API_PREFIX void API_NAME(RasterPos2fv)(CTX_PREFIX  const GLfloat *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1], 0.0F, 1.0F );
}


API_PREFIX void API_NAME(RasterPos2iv)(CTX_PREFIX  const GLint *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1], 0.0F, 1.0F );
}


API_PREFIX void API_NAME(RasterPos2sv)(CTX_PREFIX  const GLshort *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1], 0.0F, 1.0F );
}


API_PREFIX void API_NAME(RasterPos3dv)(CTX_PREFIX  const GLdouble *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                           (GLfloat) v[2], 1.0F );
}


API_PREFIX void API_NAME(RasterPos3fv)(CTX_PREFIX  const GLfloat *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                               (GLfloat) v[2], 1.0F );
}


API_PREFIX void API_NAME(RasterPos3iv)(CTX_PREFIX  const GLint *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                           (GLfloat) v[2], 1.0F );
}


API_PREFIX void API_NAME(RasterPos3sv)(CTX_PREFIX  const GLshort *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                           (GLfloat) v[2], 1.0F );
}


API_PREFIX void API_NAME(RasterPos4dv)(CTX_PREFIX  const GLdouble *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                           (GLfloat) v[2], (GLfloat) v[3] );
}


API_PREFIX void API_NAME(RasterPos4fv)(CTX_PREFIX  const GLfloat *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, v[0], v[1], v[2], v[3] );
}


API_PREFIX void API_NAME(RasterPos4iv)(CTX_PREFIX  const GLint *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                           (GLfloat) v[2], (GLfloat) v[3] );
}


API_PREFIX void API_NAME(RasterPos4sv)(CTX_PREFIX  const GLshort *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                           (GLfloat) v[2], (GLfloat) v[3] );
}


API_PREFIX void API_NAME(ReadBuffer)(CTX_PREFIX  GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ReadBuffer)( CC, mode );
}


API_PREFIX void API_NAME(ReadPixels)(CTX_PREFIX  GLint x, GLint y, GLsizei width, GLsizei height,
                              GLenum format, GLenum type, GLvoid *pixels )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ReadPixels)( CC, x, y, width, height, format, type, pixels );
}


API_PREFIX void API_NAME(Rectd)(CTX_PREFIX  GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)( CC, (GLfloat) x1, (GLfloat) y1,
                     (GLfloat) x2, (GLfloat) y2 );
}


API_PREFIX void API_NAME(Rectf)(CTX_PREFIX  GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)( CC, x1, y1, x2, y2 );
}


API_PREFIX void API_NAME(Recti)(CTX_PREFIX  GLint x1, GLint y1, GLint x2, GLint y2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)( CC, (GLfloat) x1, (GLfloat) y1,
                         (GLfloat) x2, (GLfloat) y2 );
}


API_PREFIX void API_NAME(Rects)(CTX_PREFIX  GLshort x1, GLshort y1, GLshort x2, GLshort y2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)( CC, (GLfloat) x1, (GLfloat) y1,
                     (GLfloat) x2, (GLfloat) y2 );
}


API_PREFIX void API_NAME(Rectdv)(CTX_PREFIX  const GLdouble *v1, const GLdouble *v2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)(CC, (GLfloat) v1[0], (GLfloat) v1[1],
                    (GLfloat) v2[0], (GLfloat) v2[1]);
}


API_PREFIX void API_NAME(Rectfv)(CTX_PREFIX  const GLfloat *v1, const GLfloat *v2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)(CC, v1[0], v1[1], v2[0], v2[1]);
}


API_PREFIX void API_NAME(Rectiv)(CTX_PREFIX  const GLint *v1, const GLint *v2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)( CC, (GLfloat) v1[0], (GLfloat) v1[1],
                     (GLfloat) v2[0], (GLfloat) v2[1] );
}


API_PREFIX void API_NAME(Rectsv)(CTX_PREFIX  const GLshort *v1, const GLshort *v2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)(CC, (GLfloat) v1[0], (GLfloat) v1[1],
        (GLfloat) v2[0], (GLfloat) v2[1]);
}


API_PREFIX void API_NAME(Scissor)(CTX_PREFIX  GLint x, GLint y, GLsizei width, GLsizei height)
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Scissor)(CC, x, y, width, height);
}


API_PREFIX GLboolean API_NAME(IsEnabled)(CTX_PREFIX  GLenum cap )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(GL_FALSE);
   return (*CC->API.IsEnabled)( CC, cap );
}



API_PREFIX void API_NAME(PushAttrib)(CTX_PREFIX  GLbitfield mask )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PushAttrib)(CC, mask);
}


API_PREFIX void API_NAME(PushClientAttrib)(CTX_PREFIX  GLbitfield mask )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PushClientAttrib)(CC, mask);
}


API_PREFIX void API_NAME(PushName)(CTX_PREFIX  GLuint name )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PushName)(CC, name);
}


API_PREFIX GLint API_NAME(RenderMode)(CTX_PREFIX  GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(0);
   return (*CC->API.RenderMode)(CC, mode);
}


API_PREFIX void API_NAME(Rotated)(CTX_PREFIX  GLdouble angle, GLdouble x, GLdouble y, GLdouble z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rotatef)( CC, (GLfloat) angle,
                       (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


API_PREFIX void API_NAME(Rotatef)(CTX_PREFIX  GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rotatef)( CC, angle, x, y, z );
}


API_PREFIX void API_NAME(SelectBuffer)(CTX_PREFIX  GLsizei size, GLuint *buffer )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.SelectBuffer)(CC, size, buffer);
}


API_PREFIX void API_NAME(Scaled)(CTX_PREFIX  GLdouble x, GLdouble y, GLdouble z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Scalef)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


API_PREFIX void API_NAME(Scalef)(CTX_PREFIX  GLfloat x, GLfloat y, GLfloat z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Scalef)( CC, x, y, z );
}


API_PREFIX void API_NAME(ShadeModel)(CTX_PREFIX  GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ShadeModel)(CC, mode);
}


API_PREFIX void API_NAME(StencilFunc)(CTX_PREFIX  GLenum func, GLint ref, GLuint mask )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.StencilFunc)(CC, func, ref, mask);
}


API_PREFIX void API_NAME(StencilMask)(CTX_PREFIX  GLuint mask )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.StencilMask)(CC, mask);
}


API_PREFIX void API_NAME(StencilOp)(CTX_PREFIX  GLenum fail, GLenum zfail, GLenum zpass )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.StencilOp)(CC, fail, zfail, zpass);
}

#define TEXCOORD1(s)				\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   GET_IMMEDIATE;				\
   count = IM->Count;				\
   IM->Flag[count] |= VERT_TEX0_1;		\
   tc = IM->TexCoord[0][count];			\
   ASSIGN_4V(tc,s,0,0,1);			\
}

#define TEXCOORD2(s,t)				\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   GET_IMMEDIATE;				\
   count = IM->Count;				\
   IM->Flag[count] |= VERT_TEX0_12;		\
   tc = IM->TexCoord[0][count];			\
   ASSIGN_4V(tc, s,t,0,1);		        \
}

#define TEXCOORD3(s,t,u)			\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   GET_IMMEDIATE;				\
   count = IM->Count;				\
   IM->Flag[count] |= VERT_TEX0_123;		\
   tc = IM->TexCoord[0][count];			\
   ASSIGN_4V(tc, s,t,u,1);			\
}

#define TEXCOORD4(s,t,u,v)			\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   GET_IMMEDIATE;				\
   count = IM->Count;				\
   IM->Flag[count] |= VERT_TEX0_1234;		\
   tc = IM->TexCoord[0][count];			\
   ASSIGN_4V(tc, s,t,u,v);			\
}


API_PREFIX void API_NAME(TexCoord1d)(CTX_PREFIX  GLdouble s )
{
   TEXCOORD1(s);
}


API_PREFIX void API_NAME(TexCoord1f)(CTX_PREFIX  GLfloat s )
{
   TEXCOORD1(s);
}


API_PREFIX void API_NAME(TexCoord1i)(CTX_PREFIX  GLint s )
{
   TEXCOORD1(s);
}


API_PREFIX void API_NAME(TexCoord1s)(CTX_PREFIX  GLshort s )
{
   TEXCOORD1(s);
}


API_PREFIX void API_NAME(TexCoord2d)(CTX_PREFIX  GLdouble s, GLdouble t )
{
   TEXCOORD2(s,t);
}

API_PREFIX void API_NAME(TexCoord2f)(CTX_PREFIX  GLfloat s, GLfloat t )
{
   TEXCOORD2(*(&s),*&t); 
}


API_PREFIX void API_NAME(TexCoord2s)(CTX_PREFIX  GLshort s, GLshort t )
{
   TEXCOORD2(s,t);
}

API_PREFIX void API_NAME(TexCoord2i)(CTX_PREFIX  GLint s, GLint t )
{
   TEXCOORD2(s,t);
}


API_PREFIX void API_NAME(TexCoord3d)(CTX_PREFIX  GLdouble s, GLdouble t, GLdouble r )
{
   TEXCOORD3(s,t,r);
}


API_PREFIX void API_NAME(TexCoord3f)(CTX_PREFIX  GLfloat s, GLfloat t, GLfloat r )
{
   TEXCOORD3(s,t,r);
}


API_PREFIX void API_NAME(TexCoord3i)(CTX_PREFIX  GLint s, GLint t, GLint r )
{
   TEXCOORD3(s,t,r);
}


API_PREFIX void API_NAME(TexCoord3s)(CTX_PREFIX  GLshort s, GLshort t, GLshort r )
{
   TEXCOORD3(s,t,r);
}


API_PREFIX void API_NAME(TexCoord4d)(CTX_PREFIX  GLdouble s, GLdouble t, GLdouble r, GLdouble q )
{
   TEXCOORD4(s,t,r,q)
}


API_PREFIX void API_NAME(TexCoord4f)(CTX_PREFIX  GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
   TEXCOORD4(s,t,r,q)
}


API_PREFIX void API_NAME(TexCoord4i)(CTX_PREFIX  GLint s, GLint t, GLint r, GLint q )
{
   TEXCOORD4(s,t,r,q)
}


API_PREFIX void API_NAME(TexCoord4s)(CTX_PREFIX  GLshort s, GLshort t, GLshort r, GLshort q )
{
   TEXCOORD4(s,t,r,q)
}


API_PREFIX void API_NAME(TexCoord1dv)(CTX_PREFIX  const GLdouble *v )
{
   TEXCOORD1(v[0]);
}


API_PREFIX void API_NAME(TexCoord1fv)(CTX_PREFIX  const GLfloat *v )
{
   TEXCOORD1(v[0]);
}


API_PREFIX void API_NAME(TexCoord1iv)(CTX_PREFIX  const GLint *v )
{
   TEXCOORD1(v[0]);
}


API_PREFIX void API_NAME(TexCoord1sv)(CTX_PREFIX  const GLshort *v )
{
   TEXCOORD1(v[0]);
}


API_PREFIX void API_NAME(TexCoord2dv)(CTX_PREFIX  const GLdouble *v )
{
   TEXCOORD2(v[0],v[1]);
}


API_PREFIX void API_NAME(TexCoord2fv)(CTX_PREFIX  const GLfloat *v )
{
   TEXCOORD2(v[0],v[1]);
}


API_PREFIX void API_NAME(TexCoord2iv)(CTX_PREFIX  const GLint *v )
{
   TEXCOORD2(v[0],v[1]);
}


API_PREFIX void API_NAME(TexCoord2sv)(CTX_PREFIX  const GLshort *v )
{
   TEXCOORD2(v[0],v[1]);
}


API_PREFIX void API_NAME(TexCoord3dv)(CTX_PREFIX  const GLdouble *v )
{
   TEXCOORD2(v[0],v[1]);
}


API_PREFIX void API_NAME(TexCoord3fv)(CTX_PREFIX  const GLfloat *v )
{
   TEXCOORD3(v[0],v[1],v[2]);
}


API_PREFIX void API_NAME(TexCoord3iv)(CTX_PREFIX  const GLint *v )
{
   TEXCOORD3(v[0],v[1],v[2]);
}


API_PREFIX void API_NAME(TexCoord3sv)(CTX_PREFIX  const GLshort *v )
{
   TEXCOORD3(v[0],v[1],v[2]);
}


API_PREFIX void API_NAME(TexCoord4dv)(CTX_PREFIX  const GLdouble *v )
{
   TEXCOORD4(v[0],v[1],v[2],v[3]);
}


API_PREFIX void API_NAME(TexCoord4fv)(CTX_PREFIX  const GLfloat *v )
{

   TEXCOORD4(v[0],v[1],v[2],v[3]);
}


API_PREFIX void API_NAME(TexCoord4iv)(CTX_PREFIX  const GLint *v )
{
   TEXCOORD4(v[0],v[1],v[2],v[3]);
}


API_PREFIX void API_NAME(TexCoord4sv)(CTX_PREFIX  const GLshort *v )
{
   TEXCOORD4(v[0],v[1],v[2],v[3]);
}


#if 0
API_PREFIX void API_NAME(TexCoordPointer)(CTX_PREFIX  GLint size, GLenum type, GLsizei stride,
                                   const GLvoid *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexCoordPointer)(CC, size, type, stride, ptr);
}
#endif


API_PREFIX void API_NAME(TexGend)(CTX_PREFIX  GLenum coord, GLenum pname, GLdouble param )
{
   GLfloat p = (GLfloat) param;
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexGenfv)( CC, coord, pname, &p );
}


API_PREFIX void API_NAME(TexGenf)(CTX_PREFIX  GLenum coord, GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexGenfv)( CC, coord, pname, &param );
}


API_PREFIX void API_NAME(TexGeni)(CTX_PREFIX  GLenum coord, GLenum pname, GLint param )
{
   GLfloat p = (GLfloat) param;
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexGenfv)( CC, coord, pname, &p );
}


API_PREFIX void API_NAME(TexGendv)(CTX_PREFIX  GLenum coord, GLenum pname, const GLdouble *params )
{
   GLfloat p[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   p[0] = params[0];
   p[1] = params[1];
   p[2] = params[2];
   p[3] = params[3];
   (*CC->API.TexGenfv)( CC, coord, pname, p );
}


API_PREFIX void API_NAME(TexGeniv)(CTX_PREFIX  GLenum coord, GLenum pname, const GLint *params )
{
   GLfloat p[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   p[0] = params[0];
   p[1] = params[1];
   p[2] = params[2];
   p[3] = params[3];
   (*CC->API.TexGenfv)( CC, coord, pname, p );
}


API_PREFIX void API_NAME(TexGenfv)(CTX_PREFIX  GLenum coord, GLenum pname, const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexGenfv)( CC, coord, pname, params );
}




API_PREFIX void API_NAME(TexEnvf)(CTX_PREFIX  GLenum target, GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexEnvfv)( CC, target, pname, &param );
}



API_PREFIX void API_NAME(TexEnvi)(CTX_PREFIX  GLenum target, GLenum pname, GLint param )
{
   GLfloat p[4];
   GET_CONTEXT;
   p[0] = (GLfloat) param;
   p[1] = p[2] = p[3] = 0.0;
   CHECK_CONTEXT;
   (*CC->API.TexEnvfv)( CC, target, pname, p );
}



API_PREFIX void API_NAME(TexEnvfv)(CTX_PREFIX  GLenum target, GLenum pname, const GLfloat *param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexEnvfv)( CC, target, pname, param );
}



API_PREFIX void API_NAME(TexEnviv)(CTX_PREFIX  GLenum target, GLenum pname, const GLint *param )
{
   GLfloat p[4];
   GET_CONTEXT;
   p[0] = INT_TO_FLOAT( param[0] );
   p[1] = INT_TO_FLOAT( param[1] );
   p[2] = INT_TO_FLOAT( param[2] );
   p[3] = INT_TO_FLOAT( param[3] );
   CHECK_CONTEXT;
   (*CC->API.TexEnvfv)( CC, target, pname, p );
}


API_PREFIX void API_NAME(TexImage1D)(CTX_PREFIX  GLenum target, GLint level, GLint internalformat,
                              GLsizei width, GLint border,
                              GLenum format, GLenum type, const GLvoid *pixels )
{
   struct gl_image *teximage;
   GET_CONTEXT;
   CHECK_CONTEXT;
   teximage = gl_unpack_image( CC, width, 1, format, type, pixels );
   (*CC->API.TexImage1D)( CC, target, level, internalformat,
                          width, border, format, type, teximage );
}


API_PREFIX void API_NAME(TexImage2D)(CTX_PREFIX  GLenum target, GLint level, GLint internalformat,
                              GLsizei width, GLsizei height, GLint border,
                              GLenum format, GLenum type, const GLvoid *pixels )
{
   struct gl_image *teximage;
   GET_CONTEXT;
   CHECK_CONTEXT;
   teximage = gl_unpack_image( CC, width, height, format, type, pixels );
   (*CC->API.TexImage2D)( CC, target, level, internalformat,
			 width, height, border, format, type, teximage );
}


API_PREFIX void API_NAME(TexImage3D)(CTX_PREFIX  GLenum target, GLint level, GLint internalformat,
                              GLsizei width, GLsizei height, GLsizei depth,
                              GLint border, GLenum format, GLenum type,
                              const GLvoid *pixels )
{
   struct gl_image *teximage;
   GET_CONTEXT;
   CHECK_CONTEXT;
   teximage = gl_unpack_image3D( CC, width, height, depth, format, type, pixels);
   (*CC->API.TexImage3DEXT)( CC, target, level, internalformat,
                             width, height, depth, border, format, type, 
                             teximage );
}


API_PREFIX void API_NAME(TexParameterf)(CTX_PREFIX  GLenum target, GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexParameterfv)( CC, target, pname, &param );
}


API_PREFIX void API_NAME(TexParameteri)(CTX_PREFIX  GLenum target, GLenum pname, GLint param )
{
   GLfloat fparam[4];
   GET_CONTEXT;
   fparam[0] = (GLfloat) param;
   fparam[1] = fparam[2] = fparam[3] = 0.0;
   CHECK_CONTEXT;
   (*CC->API.TexParameterfv)( CC, target, pname, fparam );
}


API_PREFIX void API_NAME(TexParameterfv)(CTX_PREFIX  GLenum target, GLenum pname, const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexParameterfv)( CC, target, pname, params );
}


API_PREFIX void API_NAME(TexParameteriv)(CTX_PREFIX  GLenum target, GLenum pname, const GLint *params )
{
   GLfloat p[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   if (pname==GL_TEXTURE_BORDER_COLOR) {
      p[0] = INT_TO_FLOAT( params[0] );
      p[1] = INT_TO_FLOAT( params[1] );
      p[2] = INT_TO_FLOAT( params[2] );
      p[3] = INT_TO_FLOAT( params[3] );
   }
   else {
      p[0] = (GLfloat) params[0];
      p[1] = (GLfloat) params[1];
      p[2] = (GLfloat) params[2];
      p[3] = (GLfloat) params[3];
   }
   (*CC->API.TexParameterfv)( CC, target, pname, p );
}


API_PREFIX void API_NAME(TexSubImage1D)(CTX_PREFIX  GLenum target, GLint level, GLint xoffset,
                               GLsizei width, GLenum format,
                               GLenum type, const GLvoid *pixels )
{
   struct gl_image *image;
   GET_CONTEXT;
   CHECK_CONTEXT;
   image = gl_unpack_texsubimage( CC, width, 1, format, type, pixels );
   (*CC->API.TexSubImage1D)( CC, target, level, xoffset, width,
                             format, type, image );
}


API_PREFIX void API_NAME(TexSubImage2D)(CTX_PREFIX  GLenum target, GLint level,
                                 GLint xoffset, GLint yoffset,
                                 GLsizei width, GLsizei height,
                                 GLenum format, GLenum type,
                                 const GLvoid *pixels )
{
   struct gl_image *image;
   GET_CONTEXT;
   CHECK_CONTEXT;
   image = gl_unpack_texsubimage( CC, width, height, format, type, pixels );
   (*CC->API.TexSubImage2D)( CC, target, level, xoffset, yoffset,
                             width, height, format, type, image );
}


API_PREFIX void API_NAME(TexSubImage3D)(CTX_PREFIX  GLenum target, GLint level, GLint xoffset,
                                 GLint yoffset, GLint zoffset, GLsizei width,
                                 GLsizei height, GLsizei depth, GLenum format,
                                 GLenum type, const GLvoid *pixels )
{
   struct gl_image *image;
   GET_CONTEXT;
   CHECK_CONTEXT;
   image = gl_unpack_texsubimage3D( CC, width, height, depth, format, type,
                                    pixels );
   (*CC->API.TexSubImage3DEXT)( CC, target, level, xoffset, yoffset, zoffset,
                                width, height, depth, format, type, image );
}


API_PREFIX void API_NAME(Translated)(CTX_PREFIX  GLdouble x, GLdouble y, GLdouble z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Translatef)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


API_PREFIX void API_NAME(Translatef)(CTX_PREFIX  GLfloat x, GLfloat y, GLfloat z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Translatef)( CC, x, y, z );
}

/* KW: Run into bad problems in reset_vb/fixup_input if we don't fully pad
 *     the incoming vertices.
 */
#define VERTEX2(IM, x,y)			\
{						\
   GLuint count = IM->Count++;			\
   GLfloat *dest = IM->Obj[count];		\
   IM->Flag[count] |= VERT_OBJ_2;		\
   ASSIGN_4V(dest, x, y, 0, 1);			\
   if (dest == IM->Obj[VB_MAX-1])		\
      IM->maybe_transform_vb( IM );		\
}

#define VERTEX3(IM,x,y,z)			\
{						\
   GLuint count = IM->Count++;			\
   GLfloat *dest = IM->Obj[count];		\
   IM->Flag[count] |= VERT_OBJ_23;		\
   ASSIGN_4V(dest, x, y, z, 1);			\
   if (dest == IM->Obj[VB_MAX-1])		\
      IM->maybe_transform_vb( IM );		\
}

#define VERTEX4(IM, x,y,z,w)			\
{						\
   GLuint count = IM->Count++;			\
   GLfloat *dest = IM->Obj[count];		\
   IM->Flag[count] |= VERT_OBJ_234;		\
   ASSIGN_4V(dest, x, y, z, w);			\
   if (dest == IM->Obj[VB_MAX-1])		\
      IM->maybe_transform_vb( IM );		\
}


API_PREFIX void API_NAME(Vertex2d)(CTX_PREFIX  GLdouble x, GLdouble y )
{
   GET_IMMEDIATE;
   VERTEX2( IM, (GLfloat) x, (GLfloat) y );
}


API_PREFIX void API_NAME(Vertex2f)(CTX_PREFIX  GLfloat x, GLfloat y )
{
   GET_IMMEDIATE;
   VERTEX2( IM, *(&x), *(&y) );
}

/* Internal use:
 */
void gl_Vertex2f( GLcontext *ctx, GLfloat x, GLfloat y )
{
   struct immediate *im = ctx->input;
   VERTEX2( im, x, y );
}

API_PREFIX void API_NAME(Vertex2i)(CTX_PREFIX  GLint x, GLint y )
{
   GET_IMMEDIATE;
   VERTEX2( IM, (GLfloat) x, (GLfloat) y );
}

API_PREFIX void API_NAME(Vertex2s)(CTX_PREFIX  GLshort x, GLshort y )
{
   GET_IMMEDIATE;
   VERTEX2( IM, (GLfloat) x, (GLfloat) y );
}

API_PREFIX void API_NAME(Vertex3d)(CTX_PREFIX  GLdouble x, GLdouble y, GLdouble z )
{
   GET_IMMEDIATE;
   VERTEX3( IM, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


API_PREFIX void API_NAME(Vertex3f)(CTX_PREFIX  GLfloat x, GLfloat y, GLfloat z )
{
   GET_IMMEDIATE;
   VERTEX3( IM, *(&x), *(&y), *(&z) ); 
}

API_PREFIX void API_NAME(Vertex3i)(CTX_PREFIX  GLint x, GLint y, GLint z )
{
   GET_IMMEDIATE;
   VERTEX3( IM, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


API_PREFIX void API_NAME(Vertex3s)(CTX_PREFIX  GLshort x, GLshort y, GLshort z )
{
   GET_IMMEDIATE;
   VERTEX3( IM, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


API_PREFIX void API_NAME(Vertex4d)(CTX_PREFIX  GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
   GET_IMMEDIATE;
   VERTEX4( IM, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}


API_PREFIX void API_NAME(Vertex4f)(CTX_PREFIX  GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   GET_IMMEDIATE;
   VERTEX4( IM, *(&x), *(&y), *(&z), *(&w) );
}


API_PREFIX void API_NAME(Vertex4i)(CTX_PREFIX  GLint x, GLint y, GLint z, GLint w )
{
   GET_IMMEDIATE;
   VERTEX4( IM, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}


API_PREFIX void API_NAME(Vertex4s)(CTX_PREFIX  GLshort x, GLshort y, GLshort z, GLshort w )
{
   GET_IMMEDIATE;
   VERTEX4( IM, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}


API_PREFIX void API_NAME(Vertex2dv)(CTX_PREFIX  const GLdouble *v )
{
   GET_IMMEDIATE;
   VERTEX2( IM, (GLfloat) v[0], (GLfloat) v[1] );
}


API_PREFIX void API_NAME(Vertex2fv)(CTX_PREFIX  const GLfloat *v )
{
   GET_IMMEDIATE;
   VERTEX2( IM, v[0], v[1] );
}


API_PREFIX void API_NAME(Vertex2iv)(CTX_PREFIX  const GLint *v )
{
   GET_IMMEDIATE;
   VERTEX2( IM, (GLfloat) v[0], (GLfloat) v[1] );
}


API_PREFIX void API_NAME(Vertex2sv)(CTX_PREFIX  const GLshort *v )
{
   GET_IMMEDIATE;
   VERTEX2( IM, (GLfloat) v[0], (GLfloat) v[1] );
}


API_PREFIX void API_NAME(Vertex3dv)(CTX_PREFIX  const GLdouble *v )
{
   GET_IMMEDIATE;
   VERTEX3( IM, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2] );
}


API_PREFIX void API_NAME(Vertex3fv)(CTX_PREFIX  const GLfloat *v )
{
   GET_IMMEDIATE;
   VERTEX3( IM, v[0], v[1], v[2] );
}

API_PREFIX void API_NAME(Vertex3iv)(CTX_PREFIX  const GLint *v )
{
   GET_IMMEDIATE;
   VERTEX3( IM, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2] );
}


API_PREFIX void API_NAME(Vertex3sv)(CTX_PREFIX  const GLshort *v )
{
   GET_IMMEDIATE;
   VERTEX3( IM, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2] );
}


API_PREFIX void API_NAME(Vertex4dv)(CTX_PREFIX  const GLdouble *v )
{
   GET_IMMEDIATE;
   VERTEX4( IM, 
	    (GLfloat) v[0], (GLfloat) v[1], 
	    (GLfloat) v[2], (GLfloat) v[3] );
}


API_PREFIX void API_NAME(Vertex4fv)(CTX_PREFIX  const GLfloat *v )
{
   GET_IMMEDIATE;
   VERTEX4( IM, v[0], v[1], v[2], v[3] );
}


API_PREFIX void API_NAME(Vertex4iv)(CTX_PREFIX  const GLint *v )
{
   GET_IMMEDIATE;
   VERTEX4( IM, 
	    (GLfloat) v[0], (GLfloat) v[1], 
	    (GLfloat) v[2], (GLfloat) v[3] );
}


API_PREFIX void API_NAME(Vertex4sv)(CTX_PREFIX  const GLshort *v )
{
   GET_IMMEDIATE;
   VERTEX4( IM, 
	    (GLfloat) v[0], (GLfloat) v[1], 
	    (GLfloat) v[2], (GLfloat) v[3] );
}


#if 0
API_PREFIX void API_NAME(VertexPointer)(CTX_PREFIX  GLint size, GLenum type, GLsizei stride,
                                 const GLvoid *ptr )
{
   GET_CONTEXT;
   (*CC->API.VertexPointer)(CC, size, type, stride, ptr);
}
#endif

API_PREFIX void API_NAME(Viewport)(CTX_PREFIX  GLint x, GLint y, GLsizei width, GLsizei height )
{
   GET_CONTEXT;
   (*CC->API.Viewport)( CC, x, y, width, height );
}

