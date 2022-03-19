/* $Id: masking.c,v 1.7 2000/04/11 21:36:29 brianp Exp $ */

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
 * Implement the effect of glColorMask and glIndexMask in software.
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "alphabuf.h"
#include "context.h"
#include "enums.h"
#include "macros.h"
#include "masking.h"
#include "pb.h"
#include "span.h"
#include "types.h"
#endif



void
_mesa_IndexMask( GLuint mask )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glIndexMask");
   ctx->Color.IndexMask = mask;
   ctx->NewState |= NEW_RASTER_OPS;
}



void
_mesa_ColorMask( GLboolean red, GLboolean green,
                 GLboolean blue, GLboolean alpha )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glColorMask");

   if (MESA_VERBOSE & VERBOSE_API)
      fprintf(stderr, "glColorMask %d %d %d %d\n", red, green, blue, alpha);

   ctx->Color.ColorMask[RCOMP] = red    ? 0xff : 0x0;
   ctx->Color.ColorMask[GCOMP] = green  ? 0xff : 0x0;
   ctx->Color.ColorMask[BCOMP] = blue   ? 0xff : 0x0;
   ctx->Color.ColorMask[ACOMP] = alpha  ? 0xff : 0x0;

   if (ctx->Driver.ColorMask) 
      ctx->Driver.ColorMask( ctx, red, green, blue, alpha );

   ctx->NewState |= NEW_RASTER_OPS;
}




/*
 * Apply glColorMask to a span of RGBA pixels.
 */
void
_mesa_mask_rgba_span( GLcontext *ctx,
                      GLuint n, GLint x, GLint y, GLubyte rgba[][4] )
{
   GLubyte dest[MAX_WIDTH][4];
   GLuint srcMask = *((GLuint*)ctx->Color.ColorMask);
   GLuint dstMask = ~srcMask;
   GLuint *rgba32 = (GLuint *) rgba;
   GLuint *dest32 = (GLuint *) dest;
   GLuint i;

   gl_read_rgba_span( ctx, ctx->DrawBuffer, n, x, y, dest );

   for (i=0; i<n; i++) {
      rgba32[i] = (rgba32[i] & srcMask) | (dest32[i] & dstMask);
   }
}



/*
 * Apply glColorMask to an array of RGBA pixels.
 */
void
_mesa_mask_rgba_pixels( GLcontext *ctx,
                        GLuint n, const GLint x[], const GLint y[],
                        GLubyte rgba[][4], const GLubyte mask[] )
{
   GLubyte dest[PB_SIZE][4];
   GLuint srcMask = *((GLuint*)ctx->Color.ColorMask);
   GLuint dstMask = ~srcMask;
   GLuint *rgba32 = (GLuint *) rgba;
   GLuint *dest32 = (GLuint *) dest;
   GLuint i;

   (*ctx->Driver.ReadRGBAPixels)( ctx, n, x, y, dest, mask );
   if (ctx->RasterMask & ALPHABUF_BIT) {
      _mesa_read_alpha_pixels( ctx, n, x, y, dest, mask );
   }

   for (i=0; i<n; i++) {
      rgba32[i] = (rgba32[i] & srcMask) | (dest32[i] & dstMask);
   }
}



/*
 * Apply glIndexMask to a span of CI pixels.
 */
void
_mesa_mask_index_span( GLcontext *ctx,
                       GLuint n, GLint x, GLint y, GLuint index[] )
{
   GLuint i;
   GLuint fbindexes[MAX_WIDTH];
   GLuint msrc, mdest;

   gl_read_index_span( ctx, ctx->DrawBuffer, n, x, y, fbindexes );

   msrc = ctx->Color.IndexMask;
   mdest = ~msrc;

   for (i=0;i<n;i++) {
      index[i] = (index[i] & msrc) | (fbindexes[i] & mdest);
   }
}



/*
 * Apply glIndexMask to an array of CI pixels.
 */
void
_mesa_mask_index_pixels( GLcontext *ctx,
                         GLuint n, const GLint x[], const GLint y[],
                         GLuint index[], const GLubyte mask[] )
{
   GLuint i;
   GLuint fbindexes[PB_SIZE];
   GLuint msrc, mdest;

   (*ctx->Driver.ReadCI32Pixels)( ctx, n, x, y, fbindexes, mask );

   msrc = ctx->Color.IndexMask;
   mdest = ~msrc;

   for (i=0;i<n;i++) {
      index[i] = (index[i] & msrc) | (fbindexes[i] & mdest);
   }
}

