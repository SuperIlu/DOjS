/* $Id: pixeltex.c,v 1.2 2000/04/07 18:55:31 brianp Exp $ */

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


/*
 * This file implements both the GL_SGIX_pixel_texture and
 * GL_SIGS_pixel_texture extensions. Luckily, they pretty much
 * overlap in functionality so we use the same state variables
 * and execution code for both.
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "context.h"
#include "pixeltex.h"
#endif


void
_mesa_PixelTexGenSGIX(GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glPixelTexGenSGIX");

   switch (mode) {
      case GL_NONE:
         ctx->Pixel.FragmentRgbSource = GL_PIXEL_GROUP_COLOR_SGIS;
         ctx->Pixel.FragmentAlphaSource = GL_PIXEL_GROUP_COLOR_SGIS;
         break;
      case GL_ALPHA:
         ctx->Pixel.FragmentRgbSource = GL_PIXEL_GROUP_COLOR_SGIS;
         ctx->Pixel.FragmentAlphaSource = GL_CURRENT_RASTER_COLOR;
         break;
      case GL_RGB:
         ctx->Pixel.FragmentRgbSource = GL_CURRENT_RASTER_COLOR;
         ctx->Pixel.FragmentAlphaSource = GL_PIXEL_GROUP_COLOR_SGIS;
         break;
      case GL_RGBA:
         ctx->Pixel.FragmentRgbSource = GL_CURRENT_RASTER_COLOR;
         ctx->Pixel.FragmentAlphaSource = GL_CURRENT_RASTER_COLOR;
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glPixelTexGenSGIX(mode)");
         return;
   }
}


void
_mesa_PixelTexGenParameterfSGIS(GLenum target, GLfloat value)
{
   _mesa_PixelTexGenParameteriSGIS(target, (GLint) value);
}


void
_mesa_PixelTexGenParameterfvSGIS(GLenum target, const GLfloat *value)
{
   _mesa_PixelTexGenParameteriSGIS(target, (GLint) *value);
}


void
_mesa_PixelTexGenParameteriSGIS(GLenum target, GLint value)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glPixelTexGenParameterSGIS");

   if (value != GL_CURRENT_RASTER_COLOR && value != GL_PIXEL_GROUP_COLOR_SGIS) {
      gl_error(ctx, GL_INVALID_ENUM, "glPixelTexGenParameterSGIS(value)");
      return;
   }

   if (target == GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS) {
      ctx->Pixel.FragmentRgbSource = (GLenum) value;
   }
   else if (target == GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS) {
      ctx->Pixel.FragmentAlphaSource = (GLenum) value;
   }
   else {
      gl_error(ctx, GL_INVALID_ENUM, "glPixelTexGenParameterSGIS(target)");
   }
}


void
_mesa_PixelTexGenParameterivSGIS(GLenum target, const GLint *value)
{
  _mesa_PixelTexGenParameteriSGIS(target, *value);
}


void
_mesa_GetPixelTexGenParameterfvSGIS(GLenum target, GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glGetPixelTexGenParameterfvSGIS");

   if (target == GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS) {
      *value = (GLfloat) ctx->Pixel.FragmentRgbSource;
   }
   else if (target == GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS) {
      *value = (GLfloat) ctx->Pixel.FragmentAlphaSource;
   }
   else {
      gl_error(ctx, GL_INVALID_ENUM, "glGetPixelTexGenParameterfvSGIS(target)");
   }
}


void
_mesa_GetPixelTexGenParameterivSGIS(GLenum target, GLint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glGetPixelTexGenParameterivSGIS");

   if (target == GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS) {
      *value = (GLint) ctx->Pixel.FragmentRgbSource;
   }
   else if (target == GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS) {
      *value = (GLint) ctx->Pixel.FragmentAlphaSource;
   }
   else {
      gl_error(ctx, GL_INVALID_ENUM, "glGetPixelTexGenParameterivSGIS(target)");
   }
}



/*
 * Convert RGBA values into strq texture coordinates.
 */
void
_mesa_pixeltexgen(GLcontext *ctx, GLuint n, const GLubyte rgba[][4],
                  GLfloat s[], GLfloat t[], GLfloat r[], GLfloat q[])
{
   static GLboolean firstCall = GL_TRUE;
   static GLfloat byteToFloat[256];

   if (firstCall) {
      GLuint i;
      for (i = 0; i < 256; i++) {
         byteToFloat[i] = i / 255.0F;
      }
      firstCall = GL_FALSE;
   }

   if (ctx->Pixel.FragmentRgbSource == GL_CURRENT_RASTER_COLOR) {
      GLuint i;
      for (i = 0; i < n; i++) {
         s[i] = ctx->Current.RasterColor[RCOMP];
         t[i] = ctx->Current.RasterColor[GCOMP];
         r[i] = ctx->Current.RasterColor[BCOMP];
      }
   }
   else {
      GLuint i;
      ASSERT(ctx->Pixel.FragmentRgbSource == GL_PIXEL_GROUP_COLOR_SGIS);
      for (i = 0; i < n; i++) {
         s[i] = byteToFloat[rgba[i][RCOMP]];
         t[i] = byteToFloat[rgba[i][GCOMP]];
         r[i] = byteToFloat[rgba[i][BCOMP]];
      }
   }

   if (ctx->Pixel.FragmentAlphaSource == GL_CURRENT_RASTER_COLOR) {
      GLuint i;
      for (i = 0; i < n; i++) {
         q[i] = ctx->Current.RasterColor[ACOMP];
      }
   }
   else {
      GLuint i;
      ASSERT(ctx->Pixel.FragmentAlphaSource == GL_PIXEL_GROUP_COLOR_SGIS);
      for (i = 0; i < n; i++) {
         q[i] = byteToFloat[rgba[i][ACOMP]];
      }
   }
}
