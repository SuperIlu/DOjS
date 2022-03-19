
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
 * Software alpha planes.  Many frame buffers don't have alpha bits so
 * we simulate them in software.
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "alphabuf.h"
#include "context.h"
#include "mem.h"
#include "types.h"
#endif



#define ALPHA_DRAW_ADDR(X,Y) \
   (ctx->DrawBuffer->Alpha + (Y) * ctx->DrawBuffer->Width + (X))

#define ALPHA_READ_ADDR(X,Y) \
   (ctx->ReadBuffer->Alpha + (Y) * ctx->ReadBuffer->Width + (X))



/*
 * Allocate new front/back/left/right alpha buffers.
 * Input: ctx - the context
 *
 */
static void 
alloc_alpha_buffers( GLcontext *ctx, GLframebuffer *buf )
{
   GLint bytes = buf->Width * buf->Height * sizeof(GLubyte);

   ASSERT(ctx->DrawBuffer->UseSoftwareAlphaBuffers);

   if (buf->FrontLeftAlpha) {
      FREE( buf->FrontLeftAlpha );
   }
   buf->FrontLeftAlpha = (GLubyte *) MALLOC( bytes );
   if (!buf->FrontLeftAlpha) {
      /* out of memory */
      gl_error( ctx, GL_OUT_OF_MEMORY,
                "Couldn't allocate front-left alpha buffer" );
   }

   if (ctx->Visual->DBflag) {
      if (buf->BackLeftAlpha) {
         FREE( buf->BackLeftAlpha );
      }
      buf->BackLeftAlpha = (GLubyte *) MALLOC( bytes );
      if (!buf->BackLeftAlpha) {
         /* out of memory */
         gl_error( ctx, GL_OUT_OF_MEMORY,
                      "Couldn't allocate back-left alpha buffer" );
      }
   }

   if (ctx->Visual->StereoFlag) {
      if (buf->FrontRightAlpha) {
         FREE( buf->FrontRightAlpha );
      }
      buf->FrontRightAlpha = (GLubyte *) MALLOC( bytes );
      if (!buf->FrontRightAlpha) {
         /* out of memory */
         gl_error( ctx, GL_OUT_OF_MEMORY,
                   "Couldn't allocate front-right alpha buffer" );
      }

      if (ctx->Visual->DBflag) {
         if (buf->BackRightAlpha) {
            FREE( buf->BackRightAlpha );
         }
         buf->BackRightAlpha = (GLubyte *) MALLOC( bytes );
         if (!buf->BackRightAlpha) {
            /* out of memory */
            gl_error( ctx, GL_OUT_OF_MEMORY,
                      "Couldn't allocate back-right alpha buffer" );
         }
      }
   }

   if (ctx->Color.DriverDrawBuffer == GL_FRONT_LEFT)
      buf->Alpha = buf->FrontLeftAlpha;
   else if (ctx->Color.DriverDrawBuffer == GL_BACK_LEFT)
      buf->Alpha = buf->BackLeftAlpha;
   else if (ctx->Color.DriverDrawBuffer == GL_FRONT_RIGHT)
      buf->Alpha = buf->FrontRightAlpha;
   else if (ctx->Color.DriverDrawBuffer == GL_BACK_RIGHT)
      buf->Alpha = buf->BackRightAlpha;
}


/*
 * Allocate a new front and back alpha buffer.
 */
void
_mesa_alloc_alpha_buffers( GLcontext *ctx )
{
   alloc_alpha_buffers( ctx, ctx->DrawBuffer );
   if (ctx->ReadBuffer != ctx->DrawBuffer) {
      alloc_alpha_buffers( ctx, ctx->ReadBuffer );
   }
}


/*
 * Clear all the alpha buffers
 */
void
_mesa_clear_alpha_buffers( GLcontext *ctx )
{
   const GLubyte aclear = (GLint) (ctx->Color.ClearColor[3] * 255.0F);
   GLuint bufferBit;

   ASSERT(ctx->DrawBuffer->UseSoftwareAlphaBuffers);
   ASSERT(ctx->Color.ColorMask[ACOMP]);

   /* loop over four possible alpha buffers */
   for (bufferBit = 1; bufferBit <= 8; bufferBit = bufferBit << 1) {
      if (bufferBit & ctx->Color.DrawDestMask) {
         GLubyte *buffer;
         if (bufferBit == FRONT_LEFT_BIT) {
            buffer = ctx->DrawBuffer->FrontLeftAlpha;
         }
         else if (bufferBit == FRONT_RIGHT_BIT) {
            buffer = ctx->DrawBuffer->FrontRightAlpha;
         }
         else if (bufferBit == BACK_LEFT_BIT) {
            buffer = ctx->DrawBuffer->BackLeftAlpha;
         }
         else {
            buffer = ctx->DrawBuffer->BackRightAlpha;
         }

         if (ctx->Scissor.Enabled) {
            /* clear scissor region */
            GLint j;
            GLint rowLen = ctx->DrawBuffer->Xmax - ctx->DrawBuffer->Xmin + 1;
            GLint rows = ctx->DrawBuffer->Ymax - ctx->DrawBuffer->Ymin + 1;
            GLint width = ctx->DrawBuffer->Width;
            GLubyte *aptr = buffer
                          + ctx->DrawBuffer->Ymin * ctx->DrawBuffer->Width
                          + ctx->DrawBuffer->Xmin;
            for (j = 0; j < rows; j++) {
               MEMSET( aptr, aclear, rowLen );
               aptr += width;
            }
         }
         else {
            /* clear whole buffer */
            GLuint bytes = ctx->DrawBuffer->Width * ctx->DrawBuffer->Height;
            MEMSET( buffer, aclear, bytes );
         }
      }
   }
}



void
_mesa_write_alpha_span( GLcontext *ctx, GLuint n, GLint x, GLint y,
                        CONST GLubyte rgba[][4], const GLubyte mask[] )
{
   GLubyte *aptr = ALPHA_DRAW_ADDR( x, y );
   GLuint i;

   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            *aptr = rgba[i][ACOMP];
         }
         aptr++;
      }
   }
   else {
      for (i=0;i<n;i++) {
         *aptr++ = rgba[i][ACOMP];
      }
   }
}


void
_mesa_write_mono_alpha_span( GLcontext *ctx, GLuint n, GLint x, GLint y,
                             GLubyte alpha, const GLubyte mask[] )
{
   GLubyte *aptr = ALPHA_DRAW_ADDR( x, y );
   GLuint i;

   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            *aptr = alpha;
         }
         aptr++;
      }
   }
   else {
      for (i=0;i<n;i++) {
         *aptr++ = alpha;
      }
   }
}


void
_mesa_write_alpha_pixels( GLcontext *ctx,
                          GLuint n, const GLint x[], const GLint y[],
                          CONST GLubyte rgba[][4], const GLubyte mask[] )
{
   GLuint i;

   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            GLubyte *aptr = ALPHA_DRAW_ADDR( x[i], y[i] );
            *aptr = rgba[i][ACOMP];
         }
      }
   }
   else {
      for (i=0;i<n;i++) {
         GLubyte *aptr = ALPHA_DRAW_ADDR( x[i], y[i] );
         *aptr = rgba[i][ACOMP];
      }
   }
}


void
_mesa_write_mono_alpha_pixels( GLcontext *ctx,
                               GLuint n, const GLint x[], const GLint y[],
                               GLubyte alpha, const GLubyte mask[] )
{
   GLuint i;

   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            GLubyte *aptr = ALPHA_DRAW_ADDR( x[i], y[i] );
            *aptr = alpha;
         }
      }
   }
   else {
      for (i=0;i<n;i++) {
         GLubyte *aptr = ALPHA_DRAW_ADDR( x[i], y[i] );
         *aptr = alpha;
      }
   }
}



void
_mesa_read_alpha_span( GLcontext *ctx,
                       GLuint n, GLint x, GLint y, GLubyte rgba[][4] )
{
   GLubyte *aptr = ALPHA_READ_ADDR( x, y );
   GLuint i;
   for (i=0;i<n;i++) {
      rgba[i][ACOMP] = *aptr++;
   }
}


void
_mesa_read_alpha_pixels( GLcontext *ctx,
                         GLuint n, const GLint x[], const GLint y[],
                         GLubyte rgba[][4], const GLubyte mask[] )
{
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *aptr = ALPHA_READ_ADDR( x[i], y[i] );
         rgba[i][ACOMP] = *aptr;
      }
   }
}



