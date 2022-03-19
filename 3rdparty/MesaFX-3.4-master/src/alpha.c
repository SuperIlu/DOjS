/* $Id: alpha.c,v 1.5 2000/02/02 21:58:24 brianp Exp $ */

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


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "alpha.h"
#include "context.h"
#include "types.h"
#include "macros.h"
#include "mmath.h"
#endif



void
_mesa_AlphaFunc( GLenum func, GLclampf ref )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glAlphaFunc");

   switch (func) {
      case GL_NEVER:
      case GL_LESS:
      case GL_EQUAL:
      case GL_LEQUAL:
      case GL_GREATER:
      case GL_NOTEQUAL:
      case GL_GEQUAL:
      case GL_ALWAYS:
         ctx->Color.AlphaFunc = func;
         if (ref <= 0.0)
            ctx->Color.AlphaRef = (GLubyte) 0;
         else if (ref >= 1.0)
            ctx->Color.AlphaRef = (GLubyte) 255;
         else
            CLAMPED_FLOAT_COLOR_TO_UBYTE_COLOR(ctx->Color.AlphaRef, ref);
         if (ctx->Driver.AlphaFunc) {
            (*ctx->Driver.AlphaFunc)(ctx, func, ctx->Color.AlphaRef);
         }
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glAlphaFunc(func)" );
         break;
   }
}



/*
 * Apply the alpha test to a span of pixels.
 * In:  rgba - array of pixels
 * In/Out:  mask - current pixel mask.  Pixels which fail the alpha test
 *                 will set the corresponding mask flag to 0.
 * Return:  0 = all pixels in the span failed the alpha test.
 *          1 = one or more pixels passed the alpha test.
 */
GLint
_mesa_alpha_test( const GLcontext *ctx,
                  GLuint n, CONST GLubyte rgba[][4], GLubyte mask[] )
{
   GLuint i;
   GLubyte ref = ctx->Color.AlphaRef;

   /* switch cases ordered from most frequent to less frequent */
   switch (ctx->Color.AlphaFunc) {
      case GL_LESS:
         for (i=0;i<n;i++) {
	    mask[i] &= (rgba[i][ACOMP] < ref);
	 }
	 return 1;
      case GL_LEQUAL:
         for (i=0;i<n;i++) {
	    mask[i] &= (rgba[i][ACOMP] <= ref);
	 }
	 return 1;
      case GL_GEQUAL:
         for (i=0;i<n;i++) {
	    mask[i] &= (rgba[i][ACOMP] >= ref);
	 }
	 return 1;
      case GL_GREATER:
         for (i=0;i<n;i++) {
	    mask[i] &= (rgba[i][ACOMP] > ref);
	 }
	 return 1;
      case GL_NOTEQUAL:
         for (i=0;i<n;i++) {
	    mask[i] &= (rgba[i][ACOMP] != ref);
	 }
	 return 1;
      case GL_EQUAL:
         for (i=0;i<n;i++) {
	    mask[i] &= (rgba[i][ACOMP] == ref);
	 }
	 return 1;
      case GL_ALWAYS:
	 /* do nothing */
	 return 1;
      case GL_NEVER:
         /* caller should check for zero! */
	 return 0;
      default:
	 gl_problem( ctx, "Invalid alpha test in gl_alpha_test" );
         return 0;
   }
   /* Never get here */
   /*return 1;*/
}
