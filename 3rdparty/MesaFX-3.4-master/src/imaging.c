/* $Id: imaging.c,v 1.15.4.4 2001/05/09 22:24:55 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.4
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
 * Histogram, Min/max and convolution for GL_ARB_imaging subset
 *
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "context.h"
#include "image.h"
#include "imaging.h"
#include "mmath.h"
#include "span.h"
#endif



/**********************************************************************/
/****                  Histogram and Min/max                      *****/
/**********************************************************************/


static void
pack_histogram( GLcontext *ctx,
                GLuint n, CONST GLuint rgba[][4],
                GLenum format, GLenum type, GLvoid *destination,
                const struct gl_pixelstore_attrib *packing )
{
   const GLint comps = _mesa_components_in_format(format);
   GLuint luminance[MAX_WIDTH];

   if (format == GL_LUMINANCE || format == GL_LUMINANCE_ALPHA) {
      GLuint i;
      for (i = 0; i < n; i++) {
         luminance[i] = rgba[i][RCOMP] + rgba[i][GCOMP] + rgba[i][BCOMP];
      }
   }

#define PACK_MACRO(TYPE)					\
   {								\
      GLuint i;							\
      switch (format) {						\
         case GL_RED:						\
            for (i=0;i<n;i++)					\
               dst[i] = (TYPE) rgba[i][RCOMP];			\
            break;						\
         case GL_GREEN:						\
            for (i=0;i<n;i++)					\
               dst[i] = (TYPE) rgba[i][GCOMP];			\
            break;						\
         case GL_BLUE:						\
            for (i=0;i<n;i++)					\
               dst[i] = (TYPE) rgba[i][BCOMP];			\
            break;						\
         case GL_ALPHA:						\
            for (i=0;i<n;i++)					\
               dst[i] = (TYPE) rgba[i][ACOMP];			\
            break;						\
         case GL_LUMINANCE:					\
            for (i=0;i<n;i++)					\
               dst[i] = (TYPE) luminance[i];			\
            break;						\
         case GL_LUMINANCE_ALPHA:				\
            for (i=0;i<n;i++) {					\
               dst[i*2+0] = (TYPE) luminance[i];		\
               dst[i*2+1] = (TYPE) rgba[i][ACOMP];		\
            }							\
            break;						\
         case GL_RGB:						\
            for (i=0;i<n;i++) {					\
               dst[i*3+0] = (TYPE) rgba[i][RCOMP];		\
               dst[i*3+1] = (TYPE) rgba[i][GCOMP];		\
               dst[i*3+2] = (TYPE) rgba[i][BCOMP];		\
            }							\
            break;						\
         case GL_RGBA:						\
            for (i=0;i<n;i++) {					\
               dst[i*4+0] = (TYPE) rgba[i][RCOMP];		\
               dst[i*4+1] = (TYPE) rgba[i][GCOMP];		\
               dst[i*4+2] = (TYPE) rgba[i][BCOMP];		\
               dst[i*4+3] = (TYPE) rgba[i][ACOMP];		\
            }							\
            break;						\
         case GL_BGR:						\
            for (i=0;i<n;i++) {					\
               dst[i*3+0] = (TYPE) rgba[i][BCOMP];		\
               dst[i*3+1] = (TYPE) rgba[i][GCOMP];		\
               dst[i*3+2] = (TYPE) rgba[i][RCOMP];		\
            }							\
            break;						\
         case GL_BGRA:						\
            for (i=0;i<n;i++) {					\
               dst[i*4+0] = (TYPE) rgba[i][BCOMP];		\
               dst[i*4+1] = (TYPE) rgba[i][GCOMP];		\
               dst[i*4+2] = (TYPE) rgba[i][RCOMP];		\
               dst[i*4+3] = (TYPE) rgba[i][ACOMP];		\
            }							\
            break;						\
         case GL_ABGR_EXT:					\
            for (i=0;i<n;i++) {					\
               dst[i*4+0] = (TYPE) rgba[i][ACOMP];		\
               dst[i*4+1] = (TYPE) rgba[i][BCOMP];		\
               dst[i*4+2] = (TYPE) rgba[i][GCOMP];		\
               dst[i*4+3] = (TYPE) rgba[i][RCOMP];		\
            }							\
            break;						\
         default:						\
            gl_problem(ctx, "bad format in pack_histogram");	\
      }								\
   }

   switch (type) {
      case GL_UNSIGNED_BYTE:
         {
            GLubyte *dst = (GLubyte *) destination;
            PACK_MACRO(GLubyte);
         }
         break;
      case GL_BYTE:
         {
            GLbyte *dst = (GLbyte *) destination;
            PACK_MACRO(GLbyte);
         }
         break;
      case GL_UNSIGNED_SHORT:
         {
            GLushort *dst = (GLushort *) destination;
            PACK_MACRO(GLushort);
            if (packing->SwapBytes) {
               _mesa_swap2(dst, n * comps);
            }
         }
         break;
      case GL_SHORT:
         {
            GLshort *dst = (GLshort *) destination;
            PACK_MACRO(GLshort);
            if (packing->SwapBytes) {
               _mesa_swap2((GLushort *) dst, n * comps);
            }
         }
         break;
      case GL_UNSIGNED_INT:
         {
            GLuint *dst = (GLuint *) destination;
            PACK_MACRO(GLuint);
            if (packing->SwapBytes) {
               _mesa_swap4(dst, n * comps);
            }
         }
         break;
      case GL_INT:
         {
            GLint *dst = (GLint *) destination;
            PACK_MACRO(GLint);
            if (packing->SwapBytes) {
               _mesa_swap4((GLuint *) dst, n * comps);
            }
         }
         break;
      case GL_FLOAT:
         {
            GLfloat *dst = (GLfloat *) destination;
            PACK_MACRO(GLfloat);
            if (packing->SwapBytes) {
               _mesa_swap4((GLuint *) dst, n * comps);
            }
         }
         break;
      default:
         gl_problem(ctx, "Bad type in pack_histogram");
   }

#undef PACK_MACRO
}



static void
pack_minmax( GLcontext *ctx, CONST GLfloat minmax[2][4],
             GLenum format, GLenum type, GLvoid *destination,
             const struct gl_pixelstore_attrib *packing )
{
   const GLint comps = _mesa_components_in_format(format);
   GLfloat luminance[2];

   if (format == GL_LUMINANCE || format == GL_LUMINANCE_ALPHA) {
      luminance[0] = minmax[0][RCOMP] + minmax[0][GCOMP] + minmax[0][BCOMP];
      luminance[1] = minmax[1][RCOMP] + minmax[1][GCOMP] + minmax[1][BCOMP];
   }

#define PACK_MACRO(TYPE, CONVERSION)				\
   {								\
      GLuint i;							\
      switch (format) {						\
         case GL_RED:						\
            for (i=0;i<2;i++)					\
               dst[i] = CONVERSION (minmax[i][RCOMP]);		\
            break;						\
         case GL_GREEN:						\
            for (i=0;i<2;i++)					\
               dst[i] = CONVERSION (minmax[i][GCOMP]);		\
            break;						\
         case GL_BLUE:						\
            for (i=0;i<2;i++)					\
               dst[i] = CONVERSION (minmax[i][BCOMP]);		\
            break;						\
         case GL_ALPHA:						\
            for (i=0;i<2;i++)					\
               dst[i] = CONVERSION (minmax[i][ACOMP]);		\
            break;						\
         case GL_LUMINANCE:					\
            for (i=0;i<2;i++)					\
               dst[i] = CONVERSION (luminance[i]);		\
            break;						\
         case GL_LUMINANCE_ALPHA:				\
            for (i=0;i<2;i++) {					\
               dst[i*2+0] = CONVERSION (luminance[i]);		\
               dst[i*2+1] = CONVERSION (minmax[i][ACOMP]);	\
            }							\
            break;						\
         case GL_RGB:						\
            for (i=0;i<2;i++) {					\
               dst[i*3+0] = CONVERSION (minmax[i][RCOMP]);	\
               dst[i*3+1] = CONVERSION (minmax[i][GCOMP]);	\
               dst[i*3+2] = CONVERSION (minmax[i][BCOMP]);	\
            }							\
            break;						\
         case GL_RGBA:						\
            for (i=0;i<2;i++) {					\
               dst[i*4+0] = CONVERSION (minmax[i][RCOMP]);	\
               dst[i*4+1] = CONVERSION (minmax[i][GCOMP]);	\
               dst[i*4+2] = CONVERSION (minmax[i][BCOMP]);	\
               dst[i*4+3] = CONVERSION (minmax[i][ACOMP]);	\
            }							\
            break;						\
         case GL_BGR:						\
            for (i=0;i<2;i++) {					\
               dst[i*3+0] = CONVERSION (minmax[i][BCOMP]);	\
               dst[i*3+1] = CONVERSION (minmax[i][GCOMP]);	\
               dst[i*3+2] = CONVERSION (minmax[i][RCOMP]);	\
            }							\
            break;						\
         case GL_BGRA:						\
            for (i=0;i<2;i++) {					\
               dst[i*4+0] = CONVERSION (minmax[i][BCOMP]);	\
               dst[i*4+1] = CONVERSION (minmax[i][GCOMP]);	\
               dst[i*4+2] = CONVERSION (minmax[i][RCOMP]);	\
               dst[i*4+3] = CONVERSION (minmax[i][ACOMP]);	\
            }							\
            break;						\
         case GL_ABGR_EXT:					\
            for (i=0;i<2;i++) {					\
               dst[i*4+0] = CONVERSION (minmax[i][ACOMP]);	\
               dst[i*4+1] = CONVERSION (minmax[i][BCOMP]);	\
               dst[i*4+2] = CONVERSION (minmax[i][GCOMP]);	\
               dst[i*4+3] = CONVERSION (minmax[i][RCOMP]);	\
            }							\
            break;						\
         default:						\
            gl_problem(ctx, "bad format in pack_minmax");	\
      }								\
   }

   switch (type) {
      case GL_UNSIGNED_BYTE:
         {
            GLubyte *dst = (GLubyte *) destination;
            PACK_MACRO(GLubyte, FLOAT_TO_UBYTE);
         }
         break;
      case GL_BYTE:
         {
            GLbyte *dst = (GLbyte *) destination;
            PACK_MACRO(GLbyte, FLOAT_TO_BYTE);
         }
         break;
      case GL_UNSIGNED_SHORT:
         {
            GLushort *dst = (GLushort *) destination;
            PACK_MACRO(GLushort, FLOAT_TO_USHORT);
            if (packing->SwapBytes) {
               _mesa_swap2(dst, 2 * comps);
            }
         }
         break;
      case GL_SHORT:
         {
            GLshort *dst = (GLshort *) destination;
            PACK_MACRO(GLshort, FLOAT_TO_SHORT);
            if (packing->SwapBytes) {
               _mesa_swap2((GLushort *) dst, 2 * comps);
            }
         }
         break;
      case GL_UNSIGNED_INT:
         {
            GLuint *dst = (GLuint *) destination;
            PACK_MACRO(GLuint, FLOAT_TO_UINT);
            if (packing->SwapBytes) {
               _mesa_swap4(dst, 2 * comps);
            }
         }
         break;
      case GL_INT:
         {
            GLint *dst = (GLint *) destination;
            PACK_MACRO(GLint, FLOAT_TO_INT);
            if (packing->SwapBytes) {
               _mesa_swap4((GLuint *) dst, 2 * comps);
            }
         }
         break;
      case GL_FLOAT:
         {
            GLfloat *dst = (GLfloat *) destination;
            PACK_MACRO(GLfloat, (GLfloat));
            if (packing->SwapBytes) {
               _mesa_swap4((GLuint *) dst, 2 * comps);
            }
         }
         break;
      default:
         gl_problem(ctx, "Bad type in pack_minmax");
   }

#undef PACK_MACRO
}


/*
 * Given an internalFormat token passed to glHistogram or glMinMax,
 * return the corresponding base format.
 * Return -1 if invalid token.
 */
static GLint
base_histogram_format( GLenum format )
{
   switch (format) {
      case GL_ALPHA:
      case GL_ALPHA4:
      case GL_ALPHA8:
      case GL_ALPHA12:
      case GL_ALPHA16:
         return GL_ALPHA;
      case GL_LUMINANCE:
      case GL_LUMINANCE4:
      case GL_LUMINANCE8:
      case GL_LUMINANCE12:
      case GL_LUMINANCE16:
         return GL_LUMINANCE;
      case GL_LUMINANCE_ALPHA:
      case GL_LUMINANCE4_ALPHA4:
      case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:
      case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:
      case GL_LUMINANCE16_ALPHA16:
         return GL_LUMINANCE_ALPHA;
      case GL_RGB:
      case GL_R3_G3_B2:
      case GL_RGB4:
      case GL_RGB5:
      case GL_RGB8:
      case GL_RGB10:
      case GL_RGB12:
      case GL_RGB16:
         return GL_RGB;
      case GL_RGBA:
      case GL_RGBA2:
      case GL_RGBA4:
      case GL_RGB5_A1:
      case GL_RGBA8:
      case GL_RGB10_A2:
      case GL_RGBA12:
      case GL_RGBA16:
         return GL_RGBA;
      default:
         return -1;  /* error */
   }
}


/*
 * Given an internalFormat token passed to glConvolutionFilter
 * or glSeparableFilter, return the corresponding base format.
 * Return -1 if invalid token.
 */
static GLint
base_filter_format( GLenum format )
{
   switch (format) {
      case GL_ALPHA:
      case GL_ALPHA4:
      case GL_ALPHA8:
      case GL_ALPHA12:
      case GL_ALPHA16:
         return GL_ALPHA;
      case GL_LUMINANCE:
      case GL_LUMINANCE4:
      case GL_LUMINANCE8:
      case GL_LUMINANCE12:
      case GL_LUMINANCE16:
         return GL_LUMINANCE;
      case GL_LUMINANCE_ALPHA:
      case GL_LUMINANCE4_ALPHA4:
      case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:
      case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:
      case GL_LUMINANCE16_ALPHA16:
         return GL_LUMINANCE_ALPHA;
      case GL_INTENSITY:
      case GL_INTENSITY4:
      case GL_INTENSITY8:
      case GL_INTENSITY12:
      case GL_INTENSITY16:
         return GL_INTENSITY;
      case GL_RGB:
      case GL_R3_G3_B2:
      case GL_RGB4:
      case GL_RGB5:
      case GL_RGB8:
      case GL_RGB10:
      case GL_RGB12:
      case GL_RGB16:
         return GL_RGB;
      case 4:
      case GL_RGBA:
      case GL_RGBA2:
      case GL_RGBA4:
      case GL_RGB5_A1:
      case GL_RGBA8:
      case GL_RGB10_A2:
      case GL_RGBA12:
      case GL_RGBA16:
         return GL_RGBA;
      default:
         return -1;  /* error */
   }
}


void
_mesa_GetMinmax(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glGetHistogram");

   if (target != GL_MINMAX) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetMinmax(target)");
      return;
   }

   if (format != GL_RED &&
       format != GL_GREEN &&
       format != GL_BLUE &&
       format != GL_ALPHA &&
       format != GL_RGB &&
       format != GL_RGBA &&
       format != GL_ABGR_EXT &&
       format != GL_LUMINANCE &&
       format != GL_LUMINANCE_ALPHA) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetMinmax(format)");
      return;
   }

   if (type != GL_UNSIGNED_BYTE &&
       type != GL_BYTE &&
       type != GL_UNSIGNED_SHORT &&
       type != GL_SHORT &&
       type != GL_UNSIGNED_INT &&
       type != GL_INT &&
       type != GL_FLOAT) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetMinmax(type)");
      return;
   }

   if (!values)
      return;

   {
      GLfloat minmax[2][4];
      minmax[0][RCOMP] = CLAMP(ctx->MinMax.Min[RCOMP], 0.0F, 1.0F);
      minmax[0][GCOMP] = CLAMP(ctx->MinMax.Min[GCOMP], 0.0F, 1.0F);
      minmax[0][BCOMP] = CLAMP(ctx->MinMax.Min[BCOMP], 0.0F, 1.0F);
      minmax[0][ACOMP] = CLAMP(ctx->MinMax.Min[ACOMP], 0.0F, 1.0F);
      minmax[1][RCOMP] = CLAMP(ctx->MinMax.Max[RCOMP], 0.0F, 1.0F);
      minmax[1][GCOMP] = CLAMP(ctx->MinMax.Max[GCOMP], 0.0F, 1.0F);
      minmax[1][BCOMP] = CLAMP(ctx->MinMax.Max[BCOMP], 0.0F, 1.0F);
      minmax[1][ACOMP] = CLAMP(ctx->MinMax.Max[ACOMP], 0.0F, 1.0F);
      pack_minmax(ctx, (CONST GLfloat (*)[4]) minmax,
                  format, type, values, &ctx->Pack);
   }

   if (reset) {
      _mesa_ResetMinmax(GL_MINMAX);
   }
}


void
_mesa_GetHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glGetHistogram");

   if (target != GL_HISTOGRAM) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetHistogram(target)");
      return;
   }

   if (format != GL_RED &&
       format != GL_GREEN &&
       format != GL_BLUE &&
       format != GL_ALPHA &&
       format != GL_RGB &&
       format != GL_RGBA &&
       format != GL_ABGR_EXT &&
       format != GL_LUMINANCE &&
       format != GL_LUMINANCE_ALPHA) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetHistogram(format)");
      return;
   }

   if (type != GL_UNSIGNED_BYTE &&
       type != GL_BYTE &&
       type != GL_UNSIGNED_SHORT &&
       type != GL_SHORT &&
       type != GL_UNSIGNED_INT &&
       type != GL_INT &&
       type != GL_FLOAT) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetHistogram(type)");
      return;
   }

   if (!values)
      return;

   pack_histogram(ctx, ctx->Histogram.Width,
                  (CONST GLuint (*)[4]) ctx->Histogram.Count,
                  format, type, values, &ctx->Pack);

   if (reset) {
      GLuint i;
      for (i = 0; i < HISTOGRAM_TABLE_SIZE; i++) {
         ctx->Histogram.Count[i][0] = 0;
         ctx->Histogram.Count[i][1] = 0;
         ctx->Histogram.Count[i][2] = 0;
         ctx->Histogram.Count[i][3] = 0;
      }
   }
}


void
_mesa_GetHistogramParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glGetHistogramParameterfv");

   if (target != GL_HISTOGRAM && target != GL_PROXY_HISTOGRAM) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetHistogramParameterfv(target)");
      return;
   }

   switch (pname) {
      case GL_HISTOGRAM_WIDTH:
         *params = (GLfloat) ctx->Histogram.Width;
         break;
      case GL_HISTOGRAM_FORMAT:
         *params = (GLfloat) ctx->Histogram.Format;
         break;
      case GL_HISTOGRAM_RED_SIZE:
         *params = (GLfloat) ctx->Histogram.RedSize;
         break;
      case GL_HISTOGRAM_GREEN_SIZE:
         *params = (GLfloat) ctx->Histogram.GreenSize;
         break;
      case GL_HISTOGRAM_BLUE_SIZE:
         *params = (GLfloat) ctx->Histogram.BlueSize;
         break;
      case GL_HISTOGRAM_ALPHA_SIZE:
         *params = (GLfloat) ctx->Histogram.AlphaSize;
         break;
      case GL_HISTOGRAM_LUMINANCE_SIZE:
         *params = (GLfloat) ctx->Histogram.LuminanceSize;
         break;
      case GL_HISTOGRAM_SINK:
         *params = (GLfloat) ctx->Histogram.Sink;
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glGetHistogramParameterfv(pname)");
   }
}


void
_mesa_GetHistogramParameteriv(GLenum target, GLenum pname, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glGetHistogramParameteriv");

   if (target != GL_HISTOGRAM && target != GL_PROXY_HISTOGRAM) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetHistogramParameteriv(target)");
      return;
   }

   switch (pname) {
      case GL_HISTOGRAM_WIDTH:
         *params = (GLint) ctx->Histogram.Width;
         break;
      case GL_HISTOGRAM_FORMAT:
         *params = (GLint) ctx->Histogram.Format;
         break;
      case GL_HISTOGRAM_RED_SIZE:
         *params = (GLint) ctx->Histogram.RedSize;
         break;
      case GL_HISTOGRAM_GREEN_SIZE:
         *params = (GLint) ctx->Histogram.GreenSize;
         break;
      case GL_HISTOGRAM_BLUE_SIZE:
         *params = (GLint) ctx->Histogram.BlueSize;
         break;
      case GL_HISTOGRAM_ALPHA_SIZE:
         *params = (GLint) ctx->Histogram.AlphaSize;
         break;
      case GL_HISTOGRAM_LUMINANCE_SIZE:
         *params = (GLint) ctx->Histogram.LuminanceSize;
         break;
      case GL_HISTOGRAM_SINK:
         *params = (GLint) ctx->Histogram.Sink;
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glGetHistogramParameteriv(pname)");
   }
}


void
_mesa_GetMinmaxParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glGetMinmaxParameterfv");

   if (target != GL_MINMAX) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetMinmaxParameterfv(target)");
      return;
   }
   if (pname == GL_MINMAX_FORMAT) {
      *params = (GLfloat) ctx->MinMax.Format;
   }
   else if (pname == GL_MINMAX_SINK) {
      *params = (GLfloat) ctx->MinMax.Sink;
   }
   else {
      gl_error(ctx, GL_INVALID_ENUM, "glGetMinMaxParameterfv(pname)");
   }
}


void
_mesa_GetMinmaxParameteriv(GLenum target, GLenum pname, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glGetMinmaxParameteriv");

   if (target != GL_MINMAX) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetMinmaxParameteriv(target)");
      return;
   }
   if (pname == GL_MINMAX_FORMAT) {
      *params = (GLint) ctx->MinMax.Format;
   }
   else if (pname == GL_MINMAX_SINK) {
      *params = (GLint) ctx->MinMax.Sink;
   }
   else {
      gl_error(ctx, GL_INVALID_ENUM, "glGetMinMaxParameteriv(pname)");
   }
}


void
_mesa_Histogram(GLenum target, GLsizei width, GLenum internalFormat, GLboolean sink)
{
   GLuint i;
   GLboolean error = GL_FALSE;
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glHistogram");

   if (target != GL_HISTOGRAM && target != GL_PROXY_HISTOGRAM) {
      gl_error(ctx, GL_INVALID_ENUM, "glHistogram(target)");
      return;
   }

   if (width < 0 || width > HISTOGRAM_TABLE_SIZE) {
      if (target == GL_PROXY_HISTOGRAM) {
         error = GL_TRUE;
      }
      else {
         if (width < 0)
            gl_error(ctx, GL_INVALID_VALUE, "glHistogram(width)");
         else
            gl_error(ctx, GL_TABLE_TOO_LARGE, "glHistogram(width)");
         return;
      }
   }

   if (width != 0 && _mesa_bitcount(width) != 1) {
      if (target == GL_PROXY_HISTOGRAM) {
         error = GL_TRUE;
      }
      else {
         gl_error(ctx, GL_INVALID_VALUE, "glHistogram(width)");
         return;
      }
   }

   if (base_histogram_format(internalFormat) < 0) {
      if (target == GL_PROXY_HISTOGRAM) {
         error = GL_TRUE;
      }
      else {
         gl_error(ctx, GL_INVALID_ENUM, "glHistogram(internalFormat)");
         return;
      }
   }

   /* reset histograms */
   for (i = 0; i < HISTOGRAM_TABLE_SIZE; i++) {
      ctx->Histogram.Count[i][0] = 0;
      ctx->Histogram.Count[i][1] = 0;
      ctx->Histogram.Count[i][2] = 0;
      ctx->Histogram.Count[i][3] = 0;
   }

   if (error) {
      ctx->Histogram.Width = 0;
      ctx->Histogram.Format = 0;
      ctx->Histogram.RedSize       = 0;
      ctx->Histogram.GreenSize     = 0;
      ctx->Histogram.BlueSize      = 0;
      ctx->Histogram.AlphaSize     = 0;
      ctx->Histogram.LuminanceSize = 0;
   }
   else {
      ctx->Histogram.Width = width;
      ctx->Histogram.Format = internalFormat;
      ctx->Histogram.Sink = sink;
      ctx->Histogram.RedSize       = 0xffffffff;
      ctx->Histogram.GreenSize     = 0xffffffff;
      ctx->Histogram.BlueSize      = 0xffffffff;
      ctx->Histogram.AlphaSize     = 0xffffffff;
      ctx->Histogram.LuminanceSize = 0xffffffff;
   }
}


void
_mesa_Minmax(GLenum target, GLenum internalFormat, GLboolean sink)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glMinmax");

   if (target != GL_MINMAX) {
      gl_error(ctx, GL_INVALID_ENUM, "glMinMax(target)");
      return;
   }

   if (base_histogram_format(internalFormat) < 0) {
      gl_error(ctx, GL_INVALID_ENUM, "glMinMax(internalFormat)");
      return;
   }

   ctx->MinMax.Sink = sink;
}


void
_mesa_ResetHistogram(GLenum target)
{
   GLuint i;
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glResetHistogram");

   if (target != GL_HISTOGRAM) {
      gl_error(ctx, GL_INVALID_ENUM, "glResetHistogram(target)");
      return;
   }

   for (i = 0; i < HISTOGRAM_TABLE_SIZE; i++) {
      ctx->Histogram.Count[i][0] = 0;
      ctx->Histogram.Count[i][1] = 0;
      ctx->Histogram.Count[i][2] = 0;
      ctx->Histogram.Count[i][3] = 0;
   }
}


void
_mesa_ResetMinmax(GLenum target)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glResetMinmax");

   if (target != GL_MINMAX) {
      gl_error(ctx, GL_INVALID_ENUM, "glResetMinMax(target)");
      return;
   }

   ctx->MinMax.Min[RCOMP] = 1000;    ctx->MinMax.Max[RCOMP] = -1000;
   ctx->MinMax.Min[GCOMP] = 1000;    ctx->MinMax.Max[GCOMP] = -1000;
   ctx->MinMax.Min[BCOMP] = 1000;    ctx->MinMax.Max[BCOMP] = -1000;
   ctx->MinMax.Min[ACOMP] = 1000;    ctx->MinMax.Max[ACOMP] = -1000;
}



/*
 * Update the min/max values from an array of fragment colors.
 */
void
_mesa_update_minmax(GLcontext *ctx, GLuint n, const GLfloat rgba[][4])
{
   GLuint i;
   for (i = 0; i < n; i++) {
      /* update mins */
      if (rgba[i][RCOMP] < ctx->MinMax.Min[RCOMP])
         ctx->MinMax.Min[RCOMP] = rgba[i][RCOMP];
      if (rgba[i][GCOMP] < ctx->MinMax.Min[GCOMP])
         ctx->MinMax.Min[GCOMP] = rgba[i][GCOMP];
      if (rgba[i][BCOMP] < ctx->MinMax.Min[BCOMP])
         ctx->MinMax.Min[BCOMP] = rgba[i][BCOMP];
      if (rgba[i][ACOMP] < ctx->MinMax.Min[ACOMP])
         ctx->MinMax.Min[ACOMP] = rgba[i][ACOMP];

      /* update maxs */
      if (rgba[i][RCOMP] > ctx->MinMax.Max[RCOMP])
         ctx->MinMax.Max[RCOMP] = rgba[i][RCOMP];
      if (rgba[i][GCOMP] > ctx->MinMax.Max[GCOMP])
         ctx->MinMax.Max[GCOMP] = rgba[i][GCOMP];
      if (rgba[i][BCOMP] > ctx->MinMax.Max[BCOMP])
         ctx->MinMax.Max[BCOMP] = rgba[i][BCOMP];
      if (rgba[i][ACOMP] > ctx->MinMax.Max[ACOMP])
         ctx->MinMax.Max[ACOMP] = rgba[i][ACOMP];
   }
}


/*
 * Update the histogram values from an array of fragment colors.
 */
void
_mesa_update_histogram(GLcontext *ctx, GLuint n, const GLfloat rgba[][4])
{
   const GLint max = ctx->Histogram.Width - 1;
   GLfloat w = (GLfloat) max;
   GLuint i;

   if (ctx->Histogram.Width == 0)
      return;
   
   for (i = 0; i < n; i++) {
      GLint ri = (GLint) (rgba[i][RCOMP] * w + 0.5F);
      GLint gi = (GLint) (rgba[i][GCOMP] * w + 0.5F);
      GLint bi = (GLint) (rgba[i][BCOMP] * w + 0.5F);
      GLint ai = (GLint) (rgba[i][ACOMP] * w + 0.5F);
      ri = CLAMP(ri, 0, max);
      gi = CLAMP(gi, 0, max);
      bi = CLAMP(bi, 0, max);
      ai = CLAMP(ai, 0, max);
      ctx->Histogram.Count[ri][RCOMP]++;
      ctx->Histogram.Count[gi][GCOMP]++;
      ctx->Histogram.Count[bi][BCOMP]++;
      ctx->Histogram.Count[ai][ACOMP]++;
   }
}



/**********************************************************************/
/****                        Convolution                          *****/
/**********************************************************************/

void
_mesa_ConvolutionFilter1D(GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *image)
{
   GLint baseFormat;
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glConvolutionFilter1D");

   if (target != GL_CONVOLUTION_1D) {
      gl_error(ctx, GL_INVALID_ENUM, "glConvolutionFilter1D(target)");
      return;
   }

   baseFormat = base_filter_format(internalFormat);
   if (baseFormat < 0 || baseFormat == GL_COLOR_INDEX) {
      gl_error(ctx, GL_INVALID_ENUM, "glConvolutionFilter1D(internalFormat)");
      return;
   }

   if (width < 0 || width > MAX_CONVOLUTION_WIDTH) {
      gl_error(ctx, GL_INVALID_VALUE, "glConvolutionFilter1D(width)");
      return;
   }

   if (!_mesa_is_legal_format_and_type(format, type) ||
       format == GL_COLOR_INDEX ||
       format == GL_STENCIL_INDEX ||
       format == GL_DEPTH_COMPONENT ||
       format == GL_INTENSITY ||
       type == GL_BITMAP) {
      gl_error(ctx, GL_INVALID_ENUM, "glConvolutionFilter1D(format or type)");
      return;
   }

   ctx->Convolution1D.Format = format;
   ctx->Convolution1D.InternalFormat = internalFormat;
   ctx->Convolution1D.Width = width;
   ctx->Convolution1D.Height = 1;

   /* unpack filter image and apply scale and bias */
   _mesa_unpack_float_color_span(ctx, width, GL_RGBA,
                                 ctx->Convolution1D.Filter,
                                 format, type, image, &ctx->Unpack,
                                 GL_FALSE, GL_FALSE);
   {
      const GLfloat *scale = ctx->Pixel.ConvolutionFilterScale[0];
      const GLfloat *bias = ctx->Pixel.ConvolutionFilterBias[0];
      GLint i;
      for (i = 0; i < width; i++) {
         GLfloat r = ctx->Convolution1D.Filter[i * 4 + 0];
         GLfloat g = ctx->Convolution1D.Filter[i * 4 + 1];
         GLfloat b = ctx->Convolution1D.Filter[i * 4 + 2];
         GLfloat a = ctx->Convolution1D.Filter[i * 4 + 3];
         r = r * scale[0] + bias[0];
         g = g * scale[1] + bias[1];
         b = b * scale[2] + bias[2];
         a = a * scale[3] + bias[3];
         ctx->Convolution1D.Filter[i * 4 + 0] = r;
         ctx->Convolution1D.Filter[i * 4 + 1] = g;
         ctx->Convolution1D.Filter[i * 4 + 2] = b;
         ctx->Convolution1D.Filter[i * 4 + 3] = a;
      }
   }
}


void
_mesa_ConvolutionFilter2D(GLenum target, GLenum internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image)
{
   GLint baseFormat;
   GLint i, components;
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glConvolutionFilter2D");

   if (target != GL_CONVOLUTION_2D) {
      gl_error(ctx, GL_INVALID_ENUM, "glConvolutionFilter2D(target)");
      return;
   }

   baseFormat = base_filter_format(internalFormat);
   if (baseFormat < 0 || baseFormat == GL_COLOR_INDEX) {
      gl_error(ctx, GL_INVALID_ENUM, "glConvolutionFilter2D(internalFormat)");
      return;
   }

   if (width < 0 || width > MAX_CONVOLUTION_WIDTH) {
      gl_error(ctx, GL_INVALID_VALUE, "glConvolutionFilter2D(width)");
      return;
   }
   if (height < 0 || height > MAX_CONVOLUTION_HEIGHT) {
      gl_error(ctx, GL_INVALID_VALUE, "glConvolutionFilter2D(height)");
      return;
   }

   if (!_mesa_is_legal_format_and_type(format, type) ||
       format == GL_COLOR_INDEX ||
       format == GL_STENCIL_INDEX ||
       format == GL_DEPTH_COMPONENT ||
       format == GL_INTENSITY ||
       type == GL_BITMAP) {
      gl_error(ctx, GL_INVALID_ENUM, "glConvolutionFilter2D(format or type)");
      return;
   }

   components = _mesa_components_in_format(format);
   assert(components > 0);  /* this should have been caught earlier */

   ctx->Convolution2D.Format = format;
   ctx->Convolution2D.InternalFormat = internalFormat;
   ctx->Convolution2D.Width = width;
   ctx->Convolution2D.Height = height;

   /* unpack filter image and apply scale and bias */
   for (i = 0; i < height; i++) {
      const GLvoid *src = _mesa_image_address(&ctx->Unpack, image, width,
                                              height, format, type, 0, i, 0);
      GLfloat *dst = ctx->Convolution2D.Filter + i * width * components;
      _mesa_unpack_float_color_span(ctx, width, GL_RGBA, dst,
                                    format, type, src, &ctx->Unpack,
                                    GL_FALSE, GL_FALSE);
   }

   {
      const GLfloat *scale = ctx->Pixel.ConvolutionFilterScale[1];
      const GLfloat *bias = ctx->Pixel.ConvolutionFilterBias[1];
      for (i = 0; i < width * height; i++) {
         GLfloat r = ctx->Convolution2D.Filter[i * 4 + 0];
         GLfloat g = ctx->Convolution2D.Filter[i * 4 + 1];
         GLfloat b = ctx->Convolution2D.Filter[i * 4 + 2];
         GLfloat a = ctx->Convolution2D.Filter[i * 4 + 3];
         r = r * scale[0] + bias[0];
         g = g * scale[1] + bias[1];
         b = b * scale[2] + bias[2];
         a = a * scale[3] + bias[3];
         ctx->Convolution2D.Filter[i * 4 + 0] = r;
         ctx->Convolution2D.Filter[i * 4 + 1] = g;
         ctx->Convolution2D.Filter[i * 4 + 2] = b;
         ctx->Convolution2D.Filter[i * 4 + 3] = a;
      }
   }
}


void
_mesa_ConvolutionParameterf(GLenum target, GLenum pname, GLfloat param)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint c;

   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glConvolutionParameterf");

   switch (target) {
      case GL_CONVOLUTION_1D:
         c = 0;
         break;
      case GL_CONVOLUTION_2D:
         c = 1;
         break;
      case GL_SEPARABLE_2D:
         c = 2;
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glConvolutionParameterf(target)");
         return;
   }

   switch (pname) {
      case GL_CONVOLUTION_BORDER_MODE:
         if (param == (GLfloat) GL_REDUCE ||
             param == (GLfloat) GL_CONSTANT_BORDER ||
             param == (GLfloat) GL_REPLICATE_BORDER) {
            ctx->Pixel.ConvolutionBorderMode[c] = (GLenum) param;
         }
         else {
            gl_error(ctx, GL_INVALID_ENUM, "glConvolutionParameterf(params)");
            return;
         }
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glConvolutionParameterf(pname)");
         return;
   }
}


void
_mesa_ConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_convolution_attrib *conv;
   GLuint c;

   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glConvolutionParameterfv");

   switch (target) {
      case GL_CONVOLUTION_1D:
         c = 0;
         conv = &ctx->Convolution1D;
         break;
      case GL_CONVOLUTION_2D:
         c = 1;
         conv = &ctx->Convolution2D;
         break;
      case GL_SEPARABLE_2D:
         c = 2;
         conv = &ctx->Separable2D;
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glConvolutionParameterfv(target)");
         return;
   }

   switch (pname) {
      case GL_CONVOLUTION_BORDER_COLOR:
         COPY_4V(ctx->Pixel.ConvolutionBorderColor[c], params);
         break;
      case GL_CONVOLUTION_BORDER_MODE:
         if (params[0] == (GLfloat) GL_REDUCE ||
             params[0] == (GLfloat) GL_CONSTANT_BORDER ||
             params[0] == (GLfloat) GL_REPLICATE_BORDER) {
            ctx->Pixel.ConvolutionBorderMode[c] = (GLenum) params[0];
         }
         else {
            gl_error(ctx, GL_INVALID_ENUM, "glConvolutionParameterfv(params)");
            return;
         }
         break;
      case GL_CONVOLUTION_FILTER_SCALE:
         COPY_4V(ctx->Pixel.ConvolutionFilterScale[c], params);
         break;
      case GL_CONVOLUTION_FILTER_BIAS:
         COPY_4V(ctx->Pixel.ConvolutionFilterBias[c], params);
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glConvolutionParameterfv(pname)");
         return;
   }
}


void
_mesa_ConvolutionParameteri(GLenum target, GLenum pname, GLint param)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint c;

   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glConvolutionParameteri");

   switch (target) {
      case GL_CONVOLUTION_1D:
         c = 0;
         break;
      case GL_CONVOLUTION_2D:
         c = 1;
         break;
      case GL_SEPARABLE_2D:
         c = 2;
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glConvolutionParameteri(target)");
         return;
   }

   switch (pname) {
      case GL_CONVOLUTION_BORDER_MODE:
         if (param == (GLint) GL_REDUCE ||
             param == (GLint) GL_CONSTANT_BORDER ||
             param == (GLint) GL_REPLICATE_BORDER) {
            ctx->Pixel.ConvolutionBorderMode[c] = (GLenum) param;
         }
         else {
            gl_error(ctx, GL_INVALID_ENUM, "glConvolutionParameteri(params)");
            return;
         }
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glConvolutionParameteri(pname)");
         return;
   }
}


void
_mesa_ConvolutionParameteriv(GLenum target, GLenum pname, const GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_convolution_attrib *conv;
   GLuint c;

   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glConvolutionParameteriv");

   switch (target) {
      case GL_CONVOLUTION_1D:
         c = 0;
         conv = &ctx->Convolution1D;
         break;
      case GL_CONVOLUTION_2D:
         c = 1;
         conv = &ctx->Convolution2D;
         break;
      case GL_SEPARABLE_2D:
         c = 2;
         conv = &ctx->Separable2D;
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glConvolutionParameteriv(target)");
         return;
   }

   switch (pname) {
      case GL_CONVOLUTION_BORDER_COLOR:
	 ctx->Pixel.ConvolutionBorderColor[c][0] = INT_TO_FLOAT(params[0]);
	 ctx->Pixel.ConvolutionBorderColor[c][1] = INT_TO_FLOAT(params[1]);
	 ctx->Pixel.ConvolutionBorderColor[c][2] = INT_TO_FLOAT(params[2]);
	 ctx->Pixel.ConvolutionBorderColor[c][3] = INT_TO_FLOAT(params[3]);
         break;
      case GL_CONVOLUTION_BORDER_MODE:
         if (params[0] == (GLint) GL_REDUCE ||
             params[0] == (GLint) GL_CONSTANT_BORDER ||
             params[0] == (GLint) GL_REPLICATE_BORDER) {
            ctx->Pixel.ConvolutionBorderMode[c] = (GLenum) params[0];
         }
         else {
            gl_error(ctx, GL_INVALID_ENUM, "glConvolutionParameteriv(params)");
            return;
         }
         break;
      case GL_CONVOLUTION_FILTER_SCALE:
         COPY_4V(ctx->Pixel.ConvolutionFilterScale[c], params);
         break;
      case GL_CONVOLUTION_FILTER_BIAS:
         COPY_4V(ctx->Pixel.ConvolutionFilterBias[c], params);
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glConvolutionParameteriv(pname)");
         return;
   }
}


void
_mesa_CopyConvolutionFilter1D(GLenum target, GLenum internalFormat, GLint x, GLint y, GLsizei width)
{
   GLint baseFormat;
   GLubyte rgba[MAX_CONVOLUTION_WIDTH][4];
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glCopyConvolutionFilter1D");

   if (target != GL_CONVOLUTION_1D) {
      gl_error(ctx, GL_INVALID_ENUM, "glCopyConvolutionFilter1D(target)");
      return;
   }

   baseFormat = base_filter_format(internalFormat);
   if (baseFormat < 0 || baseFormat == GL_COLOR_INDEX) {
      gl_error(ctx, GL_INVALID_ENUM, "glCopyConvolutionFilter1D(internalFormat)");
      return;
   }

   if (width < 0 || width > MAX_CONVOLUTION_WIDTH) {
      gl_error(ctx, GL_INVALID_VALUE, "glCopyConvolutionFilter1D(width)");
      return;
   }

   /* read pixels from framebuffer */
   RENDER_START(ctx);
   gl_read_rgba_span(ctx, ctx->ReadBuffer, width, x, y, (GLubyte (*)[4]) rgba);
   RENDER_FINISH(ctx);

   /* store as convolution filter */
   _mesa_ConvolutionFilter1D(target, internalFormat, width,
                             GL_RGBA, GL_UNSIGNED_BYTE, rgba);
}


void
_mesa_CopyConvolutionFilter2D(GLenum target, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height)
{
   GLint baseFormat;
   GLint i;
   struct gl_pixelstore_attrib packSave;
   GLubyte rgba[MAX_CONVOLUTION_HEIGHT][MAX_CONVOLUTION_WIDTH][4];
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glCopyConvolutionFilter2D");

   if (target != GL_CONVOLUTION_2D) {
      gl_error(ctx, GL_INVALID_ENUM, "glCopyConvolutionFilter2D(target)");
      return;
   }

   baseFormat = base_filter_format(internalFormat);
   if (baseFormat < 0 || baseFormat == GL_COLOR_INDEX) {
      gl_error(ctx, GL_INVALID_ENUM, "glCopyConvolutionFilter2D(internalFormat)");
      return;
   }

   if (width < 0 || width > MAX_CONVOLUTION_WIDTH) {
      gl_error(ctx, GL_INVALID_VALUE, "glCopyConvolutionFilter2D(width)");
      return;
   }
   if (height < 0 || height > MAX_CONVOLUTION_HEIGHT) {
      gl_error(ctx, GL_INVALID_VALUE, "glCopyConvolutionFilter2D(height)");
      return;
   }

   /* read pixels from framebuffer */
   RENDER_START(ctx);
   for (i = 0; i < height; i++) {
      gl_read_rgba_span(ctx, ctx->ReadBuffer, width, x, y + i,
                        (GLubyte (*)[4]) rgba[i]);
   }
   RENDER_FINISH(ctx);

   /*
    * store as convolution filter
    */
   packSave = ctx->Unpack;  /* save pixel packing params */

   ctx->Unpack.Alignment = 1;
   ctx->Unpack.RowLength = MAX_CONVOLUTION_WIDTH;
   ctx->Unpack.SkipPixels = 0;
   ctx->Unpack.SkipRows = 0;
   ctx->Unpack.ImageHeight = 0;
   ctx->Unpack.SkipImages = 0;
   ctx->Unpack.SwapBytes = GL_FALSE;
   ctx->Unpack.LsbFirst = GL_FALSE;

   _mesa_ConvolutionFilter2D(target, internalFormat, width, height,
                             GL_RGBA, GL_UNSIGNED_BYTE, rgba);

   ctx->Unpack = packSave;  /* restore pixel packing params */
}


void
_mesa_GetConvolutionFilter(GLenum target, GLenum format, GLenum type, GLvoid *image)
{
   const struct gl_convolution_attrib *filter;
   GLint row;
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glGetConvolutionFilter");

   if (target != GL_CONVOLUTION_1D && target != GL_CONVOLUTION_2D) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetConvolutionFilter(target)");
      return;
   }

   if (!_mesa_is_legal_format_and_type(format, type) ||
       format == GL_COLOR_INDEX ||
       format == GL_STENCIL_INDEX ||
       format == GL_DEPTH_COMPONENT ||
       format == GL_INTENSITY ||
       type == GL_BITMAP) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetConvolutionFilter(format or type)");
      return;
   }

   switch (target) {
      case GL_CONVOLUTION_1D:
         filter = &(ctx->Convolution1D);
         break;
      case GL_CONVOLUTION_2D:
         filter = &(ctx->Convolution2D);
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glGetConvolutionFilter(target)");
         return;
   }

   for (row = 0; row < filter->Height; row++) {
      GLvoid *dst = _mesa_image_address( &ctx->Pack, image, filter->Width,
                                         filter->Height, format, type,
                                         0, row, 0);
      const GLfloat *src = filter->Filter + row * filter->Width * 4;
      GLubyte img[MAX_WIDTH * 4];
      GLint i;
      for (i = 0; i < filter->Width * 4; i++) {
         img[i] = (GLint) (CLAMP(src[i], 0.0, 1.0) * 255.0);
      }
      _mesa_pack_rgba_span(ctx, filter->Width, (const GLubyte (*)[4]) img,
                           format, type, dst, &ctx->Pack, 0);
   }
}


void
_mesa_GetConvolutionParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
   GET_CURRENT_CONTEXT(ctx);
   const struct gl_convolution_attrib *conv;
   GLuint c;

   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glGetConvolutionParameterfv");

   switch (target) {
      case GL_CONVOLUTION_1D:
         c = 0;
         conv = &ctx->Convolution1D;
         break;
      case GL_CONVOLUTION_2D:
         c = 1;
         conv = &ctx->Convolution2D;
         break;
      case GL_SEPARABLE_2D:
         c = 2;
         conv = &ctx->Separable2D;
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glGetConvolutionParameterfv(target)");
         return;
   }

   switch (pname) {
      case GL_CONVOLUTION_BORDER_COLOR:
         COPY_4V(params, ctx->Pixel.ConvolutionBorderColor[c]);
         break;
      case GL_CONVOLUTION_BORDER_MODE:
         *params = (GLfloat) ctx->Pixel.ConvolutionBorderMode[c];
         break;
      case GL_CONVOLUTION_FILTER_SCALE:
         COPY_4V(params, ctx->Pixel.ConvolutionFilterScale[c]);
         break;
      case GL_CONVOLUTION_FILTER_BIAS:
         COPY_4V(params, ctx->Pixel.ConvolutionFilterBias[c]);
         break;
      case GL_CONVOLUTION_FORMAT:
         *params = (GLfloat) conv->Format;
         break;
      case GL_CONVOLUTION_WIDTH:
         *params = (GLfloat) conv->Width;
         break;
      case GL_CONVOLUTION_HEIGHT:
         *params = (GLfloat) conv->Height;
         break;
      case GL_MAX_CONVOLUTION_WIDTH:
         *params = (GLfloat) ctx->Const.MaxConvolutionWidth;
         break;
      case GL_MAX_CONVOLUTION_HEIGHT:
         *params = (GLfloat) ctx->Const.MaxConvolutionHeight;
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glGetConvolutionParameterfv(pname)");
         return;
   }
}


void
_mesa_GetConvolutionParameteriv(GLenum target, GLenum pname, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   const struct gl_convolution_attrib *conv;
   GLuint c;

   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glGetConvolutionParameteriv");

   switch (target) {
      case GL_CONVOLUTION_1D:
         c = 0;
         conv = &ctx->Convolution1D;
         break;
      case GL_CONVOLUTION_2D:
         c = 1;
         conv = &ctx->Convolution2D;
         break;
      case GL_SEPARABLE_2D:
         c = 2;
         conv = &ctx->Separable2D;
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glGetConvolutionParameteriv(target)");
         return;
   }

   switch (pname) {
      case GL_CONVOLUTION_BORDER_COLOR:
         params[0] = FLOAT_TO_INT(ctx->Pixel.ConvolutionBorderColor[c][0]);
         params[1] = FLOAT_TO_INT(ctx->Pixel.ConvolutionBorderColor[c][1]);
         params[2] = FLOAT_TO_INT(ctx->Pixel.ConvolutionBorderColor[c][2]);
         params[3] = FLOAT_TO_INT(ctx->Pixel.ConvolutionBorderColor[c][3]);
         break;
      case GL_CONVOLUTION_BORDER_MODE:
         *params = (GLint) ctx->Pixel.ConvolutionBorderMode[c];
         break;
      case GL_CONVOLUTION_FILTER_SCALE:
         params[0] = (GLint) ctx->Pixel.ConvolutionFilterScale[c][0];
         params[1] = (GLint) ctx->Pixel.ConvolutionFilterScale[c][1];
         params[2] = (GLint) ctx->Pixel.ConvolutionFilterScale[c][2];
         params[3] = (GLint) ctx->Pixel.ConvolutionFilterScale[c][3];
         break;
      case GL_CONVOLUTION_FILTER_BIAS:
         params[0] = (GLint) ctx->Pixel.ConvolutionFilterBias[c][0];
         params[1] = (GLint) ctx->Pixel.ConvolutionFilterBias[c][1];
         params[2] = (GLint) ctx->Pixel.ConvolutionFilterBias[c][2];
         params[3] = (GLint) ctx->Pixel.ConvolutionFilterBias[c][3];
         break;
      case GL_CONVOLUTION_FORMAT:
         *params = (GLint) conv->Format;
         break;
      case GL_CONVOLUTION_WIDTH:
         *params = (GLint) conv->Width;
         break;
      case GL_CONVOLUTION_HEIGHT:
         *params = (GLint) conv->Height;
         break;
      case GL_MAX_CONVOLUTION_WIDTH:
         *params = (GLint) ctx->Const.MaxConvolutionWidth;
         break;
      case GL_MAX_CONVOLUTION_HEIGHT:
         *params = (GLint) ctx->Const.MaxConvolutionHeight;
         break;
      default:
         gl_error(ctx, GL_INVALID_ENUM, "glGetConvolutionParameteriv(pname)");
         return;
   }
}


void
_mesa_GetSeparableFilter(GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glGetSeparableFilter");

   if (target != GL_SEPARABLE_2D) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetSeparableFilter(target)");
      return;
   }

   if (!_mesa_is_legal_format_and_type(format, type) ||
       format == GL_COLOR_INDEX ||
       format == GL_STENCIL_INDEX ||
       format == GL_DEPTH_COMPONENT ||
       format == GL_INTENSITY ||
       type == GL_BITMAP) {
      gl_error(ctx, GL_INVALID_ENUM, "glGetConvolutionFilter(format or type)");
      return;
   }

   (void) row;
   (void) column;
   (void) span;
}


void
_mesa_SeparableFilter2D(GLenum target, GLenum internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column)
{
   const GLint colStart = MAX_CONVOLUTION_WIDTH * 4;
   GLint baseFormat;
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glSeparableFilter2D");

   if (target != GL_SEPARABLE_2D) {
      gl_error(ctx, GL_INVALID_ENUM, "glSeparableFilter2D(target)");
      return;
   }

   baseFormat = base_filter_format(internalFormat);
   if (baseFormat < 0 || baseFormat == GL_COLOR_INDEX) {
      gl_error(ctx, GL_INVALID_ENUM, "glSeparableFilter2D(internalFormat)");
      return;
   }

   if (width < 0 || width > MAX_CONVOLUTION_WIDTH) {
      gl_error(ctx, GL_INVALID_VALUE, "glSeparableFilter2D(width)");
      return;
   }
   if (height < 0 || height > MAX_CONVOLUTION_HEIGHT) {
      gl_error(ctx, GL_INVALID_VALUE, "glSeparableFilter2D(height)");
      return;
   }

   if (!_mesa_is_legal_format_and_type(format, type) ||
       format == GL_COLOR_INDEX ||
       format == GL_STENCIL_INDEX ||
       format == GL_DEPTH_COMPONENT ||
       format == GL_INTENSITY ||
       type == GL_BITMAP) {
      gl_error(ctx, GL_INVALID_ENUM, "glSeparableFilter2D(format or type)");
      return;
   }

   ctx->Separable2D.Format = format;
   ctx->Separable2D.InternalFormat = internalFormat;
   ctx->Separable2D.Width = width;
   ctx->Separable2D.Height = height;

   /* unpack row filter */
   _mesa_unpack_float_color_span(ctx, width, GL_RGBA,
                                 ctx->Separable2D.Filter,
                                 format, type, row, &ctx->Unpack,
                                 GL_FALSE, GL_FALSE);
   {
      const GLfloat *scale = ctx->Pixel.ConvolutionFilterScale[2];
      const GLfloat *bias = ctx->Pixel.ConvolutionFilterBias[2];
      GLint i;
      for (i = 0; i < width; i++) {
         GLfloat r = ctx->Separable2D.Filter[i * 4 + 0];
         GLfloat g = ctx->Separable2D.Filter[i * 4 + 1];
         GLfloat b = ctx->Separable2D.Filter[i * 4 + 2];
         GLfloat a = ctx->Separable2D.Filter[i * 4 + 3];
         r = r * scale[0] + bias[0];
         g = g * scale[1] + bias[1];
         b = b * scale[2] + bias[2];
         a = a * scale[3] + bias[3];
         ctx->Separable2D.Filter[i * 4 + 0] = r;
         ctx->Separable2D.Filter[i * 4 + 1] = g;
         ctx->Separable2D.Filter[i * 4 + 2] = b;
         ctx->Separable2D.Filter[i * 4 + 3] = a;
      }
   }

   /* unpack column filter */
   _mesa_unpack_float_color_span(ctx, width, GL_RGBA,
                                 &ctx->Separable2D.Filter[colStart],
                                 format, type, column, &ctx->Unpack,
                                 GL_FALSE, GL_FALSE);
   {
      const GLfloat *scale = ctx->Pixel.ConvolutionFilterScale[2];
      const GLfloat *bias = ctx->Pixel.ConvolutionFilterBias[2];
      GLint i;
      for (i = 0; i < width; i++) {
         GLfloat r = ctx->Separable2D.Filter[i * 4 + 0 + colStart];
         GLfloat g = ctx->Separable2D.Filter[i * 4 + 1 + colStart];
         GLfloat b = ctx->Separable2D.Filter[i * 4 + 2 + colStart];
         GLfloat a = ctx->Separable2D.Filter[i * 4 + 3 + colStart];
         r = r * scale[0] + bias[0];
         g = g * scale[1] + bias[1];
         b = b * scale[2] + bias[2];
         a = a * scale[3] + bias[3];
         ctx->Separable2D.Filter[i * 4 + 0 + colStart] = r;
         ctx->Separable2D.Filter[i * 4 + 1 + colStart] = g;
         ctx->Separable2D.Filter[i * 4 + 2 + colStart] = b;
         ctx->Separable2D.Filter[i * 4 + 3 + colStart] = a;
      }
   }
}

