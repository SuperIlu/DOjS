/* $Id: quads.c,v 1.5 1999/11/11 01:22:27 brianp Exp $ */

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


/*
 * Quadrilateral rendering functions.
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "quads.h"
#include "types.h"
#endif



/*
 * At this time there is no quadrilateral optimization.  Just call the
 * triangle function twice.
 * v0, v1, v2, v3 in CCW order = front facing.
 */
static void basic_quad( GLcontext *ctx,
                        GLuint v0, GLuint v1, GLuint v2, GLuint v3, GLuint pv )
{
/*     (*ctx->Driver.TriangleFunc)( ctx, v0, v1, v3, pv ); */
/*     (*ctx->Driver.TriangleFunc)( ctx, v1, v2, v3, pv ); */
   (*ctx->Driver.TriangleFunc)( ctx, v0, v1, v2, pv );
   (*ctx->Driver.TriangleFunc)( ctx, v0, v2, v3, pv );
}



/*
 * Draw nothing (NULL raster mode)
 */
static void null_quad( GLcontext *ctx,
                       GLuint v0, GLuint v1, GLuint v2, GLuint v3, GLuint pv )
{
   (void) ctx;
   (void) v0;
   (void) v1;
   (void) v2;
   (void) v3;
   (void) pv;
}



void gl_set_quad_function( GLcontext *ctx )
{
   if (ctx->RenderMode==GL_RENDER) {
      if (ctx->NoRaster) {
         ctx->Driver.QuadFunc = null_quad;
      }
      else if (ctx->Driver.QuadFunc) {
         /* Device driver will draw quads. */
	 return;
      }
      else 
         ctx->Driver.QuadFunc = basic_quad;
      
   }
   else {
      /* if in feedback or selection mode we can fall back to triangle code */
      ctx->Driver.QuadFunc = basic_quad;
   }      
}


