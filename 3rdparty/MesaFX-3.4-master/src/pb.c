/* $Id: pb.c,v 1.19.2.1 2000/11/05 21:24:01 brianp Exp $ */

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
 * Pixel buffer:
 *
 * As fragments are produced (by point, line, and bitmap drawing) they
 * are accumlated in a buffer.  When the buffer is full or has to be
 * flushed (glEnd), we apply all enabled rasterization functions to the
 * pixels and write the results to the display buffer.  The goal is to
 * maximize the number of pixels processed inside loops and to minimize
 * the number of function calls.
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "alpha.h"
#include "alphabuf.h"
#include "blend.h"
#include "depth.h"
#include "fog.h"
#include "logic.h"
#include "mem.h"
#include "masking.h"
#include "pb.h"
#include "scissor.h"
#include "stencil.h"
#include "texture.h"
#include "types.h"
#endif



/*
 * Allocate and initialize a new pixel buffer structure.
 */
struct pixel_buffer *gl_alloc_pb(void)
{
   struct pixel_buffer *pb;
   pb = CALLOC_STRUCT(pixel_buffer);
   if (pb) {
      int i, j;
      /* set non-zero fields */
      pb->primitive = GL_BITMAP;
      pb->mono = GL_TRUE;

      /* Set all lambda values to 0.0 since we don't do mipmapping for
       * points or lines and want to use the level 0 texture image.
       */
      for (j=0;j<MAX_TEXTURE_UNITS;j++) {
         for (i=0; i<PB_SIZE; i++) {
            pb->lambda[j][i] = 0.0;
         }
      }
   }
   return pb;
}



/*
 * Draw to more than one color buffer (or none).
 */
static void multi_write_index_pixels( GLcontext *ctx, GLuint n,
                                      const GLint x[], const GLint y[],
                                      const GLuint indexes[],
                                      const GLubyte mask[] )
{
   GLuint bufferBit;

   if (ctx->Color.DrawBuffer == GL_NONE)
      return;

   /* loop over four possible dest color buffers */
   for (bufferBit = 1; bufferBit <= 8; bufferBit = bufferBit << 1) {
      if (bufferBit & ctx->Color.DrawDestMask) {
         GLuint indexTmp[MAX_WIDTH];
         ASSERT(n < MAX_WIDTH);

         if (bufferBit == FRONT_LEFT_BIT)
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_FRONT_LEFT);
         else if (bufferBit == FRONT_RIGHT_BIT)
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_FRONT_RIGHT);
         else if (bufferBit == BACK_LEFT_BIT)
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_BACK_LEFT);
         else
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_BACK_RIGHT);

         /* make copy of incoming indexes */
         MEMCPY( indexTmp, indexes, n * sizeof(GLuint) );
         if (ctx->Color.SWLogicOpEnabled) {
            _mesa_logicop_ci_pixels( ctx, n, x, y, indexTmp, mask );
         }
         if (ctx->Color.SWmasking) {
            _mesa_mask_index_pixels( ctx, n, x, y, indexTmp, mask );
         }
         (*ctx->Driver.WriteCI32Pixels)( ctx, n, x, y, indexTmp, mask );
      }
   }

   /* restore default dest buffer */
   (void) (*ctx->Driver.SetDrawBuffer)( ctx, ctx->Color.DriverDrawBuffer);
}



/*
 * Draw to more than one RGBA color buffer (or none).
 */
static void multi_write_rgba_pixels( GLcontext *ctx, GLuint n,
                                     const GLint x[], const GLint y[],
                                     CONST GLubyte rgba[][4],
                                     const GLubyte mask[] )
{
   GLuint bufferBit;

   if (ctx->Color.DrawBuffer == GL_NONE)
      return;

   /* loop over four possible dest color buffers */
   for (bufferBit = 1; bufferBit <= 8; bufferBit = bufferBit << 1) {
      if (bufferBit & ctx->Color.DrawDestMask) {
         GLubyte rgbaTmp[MAX_WIDTH][4];
         ASSERT(n < MAX_WIDTH);

         if (bufferBit == FRONT_LEFT_BIT) {
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_FRONT_LEFT);
            ctx->DrawBuffer->Alpha = ctx->DrawBuffer->FrontLeftAlpha;
         }
         else if (bufferBit == FRONT_RIGHT_BIT) {
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_FRONT_RIGHT);
            ctx->DrawBuffer->Alpha = ctx->DrawBuffer->FrontRightAlpha;
         }
         else if (bufferBit == BACK_LEFT_BIT) {
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_BACK_LEFT);
            ctx->DrawBuffer->Alpha = ctx->DrawBuffer->BackLeftAlpha;
         }
         else {
            (void) (*ctx->Driver.SetDrawBuffer)( ctx, GL_BACK_RIGHT);
            ctx->DrawBuffer->Alpha = ctx->DrawBuffer->BackRightAlpha;
         }

         /* make copy of incoming colors */
         MEMCPY( rgbaTmp, rgba, 4 * n * sizeof(GLubyte) );

         if (ctx->Color.SWLogicOpEnabled) {
            _mesa_logicop_rgba_pixels( ctx, n, x, y, rgbaTmp, mask );
         }
         else if (ctx->Color.BlendEnabled) {
            _mesa_blend_pixels( ctx, n, x, y, rgbaTmp, mask );
         }
         if (ctx->Color.SWmasking) {
            _mesa_mask_rgba_pixels( ctx, n, x, y, rgbaTmp, mask );
         }

         (*ctx->Driver.WriteRGBAPixels)( ctx, n, x, y, 
					 (CONST GLubyte (*)[4])rgbaTmp, mask );
         if (ctx->RasterMask & ALPHABUF_BIT) {
            _mesa_write_alpha_pixels( ctx, n, x, y, 
                                      (CONST GLubyte (*)[4])rgbaTmp, mask );
         }
      }
   }

   /* restore default dest buffer */
   (void) (*ctx->Driver.SetDrawBuffer)( ctx, ctx->Color.DriverDrawBuffer);
}



/*
 * Add specular color to primary color.  This is used only when
 * GL_LIGHT_MODEL_COLOR_CONTROL = GL_SEPARATE_SPECULAR_COLOR.
 */
static void add_colors( GLuint n, GLubyte rgba[][4], CONST GLubyte spec[][3] )
{
   GLuint i;
   for (i=0; i<n; i++) {
      GLint r = rgba[i][RCOMP] + spec[i][RCOMP];
      GLint g = rgba[i][GCOMP] + spec[i][GCOMP];
      GLint b = rgba[i][BCOMP] + spec[i][BCOMP];
      rgba[i][RCOMP] = MIN2(r, 255);
      rgba[i][GCOMP] = MIN2(g, 255);
      rgba[i][BCOMP] = MIN2(b, 255);
   }
}



/*
 * When the pixel buffer is full, or needs to be flushed, call this
 * function.  All the pixels in the pixel buffer will be subjected
 * to texturing, scissoring, stippling, alpha testing, stenciling,
 * depth testing, blending, and finally written to the frame buffer.
 */
void gl_flush_pb( GLcontext *ctx )
{
   /* Pixel colors may be changed if any of these raster ops enabled */
   const GLuint modBits = FOG_BIT | TEXTURE_BIT | BLEND_BIT |
                          MASKING_BIT | LOGIC_OP_BIT;
   struct pixel_buffer *PB = ctx->PB;
   GLubyte mask[PB_SIZE];

   if (PB->count == 0)
      goto CleanUp;

   /* initialize mask array and clip pixels simultaneously */
   {
      GLint xmin = ctx->DrawBuffer->Xmin;
      GLint xmax = ctx->DrawBuffer->Xmax;
      GLint ymin = ctx->DrawBuffer->Ymin;
      GLint ymax = ctx->DrawBuffer->Ymax;
      GLint *x = PB->x;
      GLint *y = PB->y;
      GLuint i, n = PB->count;
      for (i=0;i<n;i++) {
         mask[i] = (x[i]>=xmin) & (x[i]<=xmax) & (y[i]>=ymin) & (y[i]<=ymax);
      }
   }

   if (ctx->Visual->RGBAflag) {
      /*
       * RGBA COLOR PIXELS
       */

      /* If each pixel can be of a different color... */
      if ((ctx->RasterMask & modBits) || !PB->mono) {

	 if (ctx->Texture.ReallyEnabled) {
            GLubyte primary_rgba[PB_SIZE][4];
            GLint texUnit;

            /* must make a copy of primary colors since they may be modified */
            MEMCPY(primary_rgba, PB->rgba, 4 * PB->count * sizeof(GLubyte));

            for (texUnit=0;texUnit<MAX_TEXTURE_UNITS;texUnit++) {
               gl_texture_pixels( ctx, texUnit,
                                  PB->count, PB->s[texUnit], PB->t[texUnit],
                                  PB->u[texUnit], PB->lambda[texUnit],
                                  primary_rgba, PB->rgba );
            }
	 }

         if (ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR
             && ctx->Light.Enabled && ctx->Texture.ReallyEnabled) {
            /* add specular color to primary color */
            add_colors( PB->count, PB->rgba, (CONST GLubyte (*)[3]) PB->spec );
         }

	 if (ctx->Fog.Enabled
             && (ctx->Hint.Fog==GL_NICEST || PB->primitive==GL_BITMAP
                 || ctx->Texture.ReallyEnabled)) {
	    _mesa_fog_rgba_pixels( ctx, PB->count, PB->z, PB->rgba );
	 }

         /* Scissoring already done above */

	 if (ctx->Color.AlphaEnabled) {
	    if (_mesa_alpha_test( ctx, PB->count, 
                                  (CONST GLubyte (*)[4]) PB->rgba, mask )==0) {
	       goto CleanUp;
	    }
	 }

	 if (ctx->Stencil.Enabled) {
	    /* first stencil test */
	    if (_mesa_stencil_and_ztest_pixels(ctx, PB->count,
                                       PB->x, PB->y, PB->z, mask) == 0) {
	       goto CleanUp;
	    }
	 }
	 else if (ctx->Depth.Test) {
	    /* regular depth testing */
	    _mesa_depth_test_pixels( ctx, PB->count, PB->x, PB->y, PB->z, mask );
	 }

         
         if (ctx->RasterMask & MULTI_DRAW_BIT) {
            multi_write_rgba_pixels( ctx, PB->count, PB->x, PB->y,
                                     (CONST GLubyte (*)[4])PB->rgba, mask );
         }
         else {
            /* normal case: write to exactly one buffer */

            if (ctx->Color.SWLogicOpEnabled) {
               _mesa_logicop_rgba_pixels( ctx, PB->count, PB->x, PB->y,
                                          PB->rgba, mask);
            }
            else if (ctx->Color.BlendEnabled) {
               _mesa_blend_pixels( ctx, PB->count, PB->x, PB->y, PB->rgba, mask);
            }
            if (ctx->Color.SWmasking) {
               _mesa_mask_rgba_pixels(ctx, PB->count, PB->x, PB->y, PB->rgba, mask);
            }

            (*ctx->Driver.WriteRGBAPixels)( ctx, PB->count, PB->x, PB->y,
                                            (CONST GLubyte (*)[4]) PB->rgba,
					    mask );
            if (ctx->RasterMask & ALPHABUF_BIT) {
               _mesa_write_alpha_pixels( ctx, PB->count, PB->x, PB->y, 
				      (CONST GLubyte (*)[4]) PB->rgba, mask );
            }
         }
      }
      else {
	 /* Same color for all pixels */

         /* Scissoring already done above */

	 if (ctx->Color.AlphaEnabled) {
	    if (_mesa_alpha_test( ctx, PB->count, 
                                  (CONST GLubyte (*)[4]) PB->rgba, mask )==0) {
	       goto CleanUp;
	    }
	 }

	 if (ctx->Stencil.Enabled) {
	    /* first stencil test */
	    if (_mesa_stencil_and_ztest_pixels(ctx, PB->count,
                                       PB->x, PB->y, PB->z, mask) == 0) {
	       goto CleanUp;
	    }
	 }
	 else if (ctx->Depth.Test) {
	    /* regular depth testing */
	    _mesa_depth_test_pixels( ctx, PB->count, PB->x, PB->y, PB->z, mask );
	 }

         if (ctx->Color.DrawBuffer == GL_NONE) {
            goto CleanUp;
         }

         if (ctx->RasterMask & MULTI_DRAW_BIT) {
            /* Copy mono color to all pixels */
            multi_write_rgba_pixels( ctx, PB->count, PB->x, PB->y,
                                     (CONST GLubyte (*)[4]) PB->rgba, mask );
         }
         else {
            /* normal case: write to exactly one buffer */
            GLubyte red   = PB->currentColor[RCOMP];
            GLubyte green = PB->currentColor[GCOMP];
            GLubyte blue  = PB->currentColor[BCOMP];
            GLubyte alpha = PB->currentColor[ACOMP];
	    (*ctx->Driver.Color)( ctx, red, green, blue, alpha );

            (*ctx->Driver.WriteMonoRGBAPixels)( ctx, PB->count, PB->x, PB->y, mask );
            if (ctx->RasterMask & ALPHABUF_BIT) {
               _mesa_write_mono_alpha_pixels( ctx, PB->count, PB->x, PB->y,
                                              PB->currentColor[ACOMP], mask );
            }
         }
         /*** ALL DONE ***/
      }
   }
   else {
      /*
       * COLOR INDEX PIXELS
       */

      /* If we may be writting pixels with different indexes... */
      if ((ctx->RasterMask & modBits) || !PB->mono) {

	 if (ctx->Fog.Enabled
             && (ctx->Hint.Fog==GL_NICEST || PB->primitive==GL_BITMAP)) {
	    _mesa_fog_ci_pixels( ctx, PB->count, PB->z, PB->index );
	 }

         /* Scissoring already done above */

	 if (ctx->Stencil.Enabled) {
	    /* first stencil test */
	    if (_mesa_stencil_and_ztest_pixels(ctx, PB->count,
                                       PB->x, PB->y, PB->z, mask) == 0) {
	       goto CleanUp;
	    }
	 }
	 else if (ctx->Depth.Test) {
	    /* regular depth testing */
	    _mesa_depth_test_pixels( ctx, PB->count, PB->x, PB->y, PB->z, mask );
	 }

         if (ctx->RasterMask & MULTI_DRAW_BIT) {
            multi_write_index_pixels( ctx, PB->count, PB->x, PB->y, PB->index, mask );
         }
         else {
            /* normal case: write to exactly one buffer */

            if (ctx->Color.SWLogicOpEnabled) {
               _mesa_logicop_ci_pixels( ctx, PB->count, PB->x, PB->y, PB->index, mask );
            }
            if (ctx->Color.SWmasking) {
               _mesa_mask_index_pixels( ctx, PB->count, PB->x, PB->y, PB->index, mask );
            }

            (*ctx->Driver.WriteCI32Pixels)( ctx, PB->count, PB->x, PB->y,
                                            PB->index, mask );
         }

         /*** ALL DONE ***/
      }
      else {
	 /* Same color index for all pixels */

         /* Scissoring already done above */

	 if (ctx->Stencil.Enabled) {
	    /* first stencil test */
	    if (_mesa_stencil_and_ztest_pixels(ctx, PB->count,
                                       PB->x, PB->y, PB->z, mask) == 0) {
	       goto CleanUp;
	    }
	 }
	 else if (ctx->Depth.Test) {
	    /* regular depth testing */
	    _mesa_depth_test_pixels( ctx, PB->count, PB->x, PB->y, PB->z, mask );
	 }
         
         if (ctx->RasterMask & MULTI_DRAW_BIT) {
            multi_write_index_pixels( ctx, PB->count, PB->x, PB->y, PB->index, mask );
         }
         else {
            /* normal case: write to exactly one buffer */

            (*ctx->Driver.Index)( ctx, PB->currentIndex );
            (*ctx->Driver.WriteMonoCIPixels)( ctx, PB->count, PB->x, PB->y, mask );
         }
      }
   }

CleanUp:
   PB->count = 0;
   PB->mono = GL_TRUE;
}


