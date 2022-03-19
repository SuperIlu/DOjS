/* $Id: rect.c,v 1.6 2000/07/10 13:22:01 keithw Exp $ */

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
#include "glheader.h"
#include "context.h"
#include "macros.h"
#include "rect.h"
#include "varray.h"
#include "vbfill.h"
#endif



/*
 * Execute a glRect*() function.
 */
void
_mesa_Rectf( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END(ctx, "glRect"); 
   RESET_IMMEDIATE(ctx);
   gl_Begin( ctx, GL_QUADS );
   gl_Vertex2f( ctx, x1, y1 );
   gl_Vertex2f( ctx, x2, y1 );
   gl_Vertex2f( ctx, x2, y2 );
   gl_Vertex2f( ctx, x1, y2 );
   gl_End( ctx );

   /* If compiling, flush these vertices so that they aren't saved
    * by the normal vertex compilation methods.
    */
   if (ctx->CompileFlag) 
   {
      ctx->CompileFlag = 0;
      ctx->input->maybe_transform_vb( ctx->input );
      ctx->CompileFlag = GL_TRUE;
   }
}


void
_mesa_Rectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
   _mesa_Rectf(x1, y1, x2, y2);
}

void
_mesa_Rectdv(const GLdouble *v1, const GLdouble *v2)
{
   _mesa_Rectf(v1[0], v1[1], v2[0], v2[1]);
}

void
_mesa_Rectfv(const GLfloat *v1, const GLfloat *v2)
{
   _mesa_Rectf(v1[0], v1[1], v2[0], v2[1]);
}

void
_mesa_Recti(GLint x1, GLint y1, GLint x2, GLint y2)
{
   _mesa_Rectf(x1, y1, x2, y2);
}

void
_mesa_Rectiv(const GLint *v1, const GLint *v2)
{
   _mesa_Rectf(v1[0], v1[1], v2[0], v2[1]);
}

void
_mesa_Rects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
   _mesa_Rectf(x1, y1, x2, y2);
}

void
_mesa_Rectsv(const GLshort *v1, const GLshort *v2)
{
   _mesa_Rectf(v1[0], v1[1], v2[0], v2[1]);
}

