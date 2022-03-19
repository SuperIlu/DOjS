/* $Id: scissor.c,v 1.7 1999/12/03 19:11:46 brianp Exp $ */

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


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "context.h"
#include "macros.h"
#include "scissor.h"
#include "types.h"
#endif


void
_mesa_Scissor( GLint x, GLint y, GLsizei width, GLsizei height )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glScissor");

   if (width < 0 || height < 0) {
      gl_error( ctx, GL_INVALID_VALUE, "glScissor" );
      return;
   }

   if (MESA_VERBOSE & VERBOSE_API)
      fprintf(stderr, "glScissor %d %d %d %d\n", x, y, width, height);

   if (x != ctx->Scissor.X ||
       y != ctx->Scissor.Y ||
       width != ctx->Scissor.Width ||
       height != ctx->Scissor.Height) {
      ctx->Scissor.X = x;
      ctx->Scissor.Y = y;
      ctx->Scissor.Width = width;
      ctx->Scissor.Height = height;
      ctx->NewState |= NEW_RASTER_OPS;
   }

   if (ctx->Driver.Scissor)
      ctx->Driver.Scissor( ctx, x, y, width, height );
}



/*
 * Apply the scissor test to a span of pixels.
 * Return:  0 = all pixels in the span are outside the scissor box.
 *          1 = one or more pixels passed the scissor test.
 */
GLint gl_scissor_span( GLcontext *ctx,
                       GLuint n, GLint x, GLint y, GLubyte mask[] )
{
   /* first check if whole span is outside the scissor box */
   if (y < ctx->DrawBuffer->Ymin
       || y > ctx->DrawBuffer->Ymax
       || x > ctx->DrawBuffer->Xmax
       || x + (GLint) n - 1 < ctx->DrawBuffer->Xmin) {
      return 0;
   }
   else {
      const GLint xMin = ctx->DrawBuffer->Xmin;
      const GLint xMax = ctx->DrawBuffer->Xmax;
      GLint i;
      for (i = 0; x + i < xMin; i++) {
         mask[i] = 0;
      }
      for (i = (GLint) n - 1; x + i > xMax; i--) {
         mask[i] = 0;
      }

      return 1;
   }
}




/*
 * Apply the scissor test to an array of pixels.
 */
GLuint gl_scissor_pixels( GLcontext *ctx,
                          GLuint n, const GLint x[], const GLint y[],
                          GLubyte mask[] )
{
   const GLint xmin = ctx->DrawBuffer->Xmin;
   const GLint xmax = ctx->DrawBuffer->Xmax;
   const GLint ymin = ctx->DrawBuffer->Ymin;
   const GLint ymax = ctx->DrawBuffer->Ymax;
   GLuint i;

   for (i=0;i<n;i++) {
      mask[i] &= (x[i]>=xmin) & (x[i]<=xmax) & (y[i]>=ymin) & (y[i]<=ymax);
   }

   return 1;
}

