/* $Id: winpos.c,v 1.5 2000/02/21 16:32:15 brianp Exp $ */

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
 * GL_MESA_window_pos extension
 *
 * This extension offers a set of functions named glWindowPos*MESA() which
 * directly set the current raster position to the given window coordinate.
 * glWindowPos*MESA() are similar to glRasterPos*() but bypass the
 * modelview, projection and viewport transformations.
 *
 * These functions should be very handy in conjunction with glDrawPixels()
 * and glCopyPixels().
 *
 * If your application uses glWindowPos*MESA() and may be compiled with
 * a real OpenGL instead of Mesa you can simply copy the winpos.[ch] files
 * into your source tree and compile them with the rest of your code
 * since glWindowPos*MESA() can, and is, implemented in terms of standard
 * OpenGL commands when not using Mesa.  In your source files which use
 * glWindowPos*MESA() just #include "winpos.h".
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "context.h"
#include "feedback.h"
#include "mmath.h"
#include "rastpos.h"
#include "winpos.h"
#endif


/*
 * This is a MESA extension function.  Pretty much just like glRasterPos
 * except we don't apply the modelview or projection matrices; specify a
 * window coordinate directly.
 * Caller:  context->API.WindowPos4fMESA pointer.
 */
void
_mesa_WindowPos4fMESA( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH( ctx, "glWindowPosMESA" );

   /* set raster position */
   ctx->Current.RasterPos[0] = x;
   ctx->Current.RasterPos[1] = y;
   ctx->Current.RasterPos[2] = CLAMP( z, 0.0F, 1.0F );
   ctx->Current.RasterPos[3] = w;

   ctx->Current.RasterPosValid = GL_TRUE;
   ctx->Current.RasterDistance = 0.0F;

   /* raster color = current color or index */
   if (ctx->Visual->RGBAflag) {
      UBYTE_RGBA_TO_FLOAT_RGBA(ctx->Current.RasterColor, 
                               ctx->Current.ByteColor);
   }
   else {
      ctx->Current.RasterIndex = ctx->Current.Index;
   }

   /* raster texcoord = current texcoord */
   {
      GLuint texSet;
      for (texSet=0; texSet<MAX_TEXTURE_UNITS; texSet++) {
         COPY_4FV( ctx->Current.RasterMultiTexCoord[texSet],
                  ctx->Current.Texcoord[texSet] );
      }
   }

   if (ctx->RenderMode==GL_SELECT) {
      gl_update_hitflag( ctx, ctx->Current.RasterPos[2] );
   }
}




void
_mesa_WindowPos2dMESA(GLdouble x, GLdouble y)
{
   _mesa_WindowPos4fMESA(x, y, 0.0F, 1.0F);
}

void
_mesa_WindowPos2fMESA(GLfloat x, GLfloat y)
{
   _mesa_WindowPos4fMESA(x, y, 0.0F, 1.0F);
}

void
_mesa_WindowPos2iMESA(GLint x, GLint y)
{
   _mesa_WindowPos4fMESA(x, y, 0.0F, 1.0F);
}

void
_mesa_WindowPos2sMESA(GLshort x, GLshort y)
{
   _mesa_WindowPos4fMESA(x, y, 0.0F, 1.0F);
}

void
_mesa_WindowPos3dMESA(GLdouble x, GLdouble y, GLdouble z)
{
   _mesa_WindowPos4fMESA(x, y, z, 1.0F);
}

void
_mesa_WindowPos3fMESA(GLfloat x, GLfloat y, GLfloat z)
{
   _mesa_WindowPos4fMESA(x, y, z, 1.0F);
}

void
_mesa_WindowPos3iMESA(GLint x, GLint y, GLint z)
{
   _mesa_WindowPos4fMESA(x, y, z, 1.0F);
}

void
_mesa_WindowPos3sMESA(GLshort x, GLshort y, GLshort z)
{
   _mesa_WindowPos4fMESA(x, y, z, 1.0F);
}

void
_mesa_WindowPos4dMESA(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
   _mesa_WindowPos4fMESA(x, y, z, w);
}

void
_mesa_WindowPos4iMESA(GLint x, GLint y, GLint z, GLint w)
{
   _mesa_WindowPos4fMESA(x, y, z, w);
}

void
_mesa_WindowPos4sMESA(GLshort x, GLshort y, GLshort z, GLshort w)
{
   _mesa_WindowPos4fMESA(x, y, z, w);
}

void
_mesa_WindowPos2dvMESA(const GLdouble *v)
{
   _mesa_WindowPos4fMESA(v[0], v[1], 0.0F, 1.0F);
}

void
_mesa_WindowPos2fvMESA(const GLfloat *v)
{
   _mesa_WindowPos4fMESA(v[0], v[1], 0.0F, 1.0F);
}

void
_mesa_WindowPos2ivMESA(const GLint *v)
{
   _mesa_WindowPos4fMESA(v[0], v[1], 0.0F, 1.0F);
}

void
_mesa_WindowPos2svMESA(const GLshort *v)
{
   _mesa_WindowPos4fMESA(v[0], v[1], 0.0F, 1.0F);
}

void
_mesa_WindowPos3dvMESA(const GLdouble *v)
{
   _mesa_WindowPos4fMESA(v[0], v[1], v[2], 1.0F);
}

void
_mesa_WindowPos3fvMESA(const GLfloat *v)
{
   _mesa_WindowPos4fMESA(v[0], v[1], v[2], 1.0F);
}

void
_mesa_WindowPos3ivMESA(const GLint *v)
{
   _mesa_WindowPos4fMESA(v[0], v[1], v[2], 1.0F);
}

void
_mesa_WindowPos3svMESA(const GLshort *v)
{
   _mesa_WindowPos4fMESA(v[0], v[1], v[2], 1.0F);
}

void
_mesa_WindowPos4dvMESA(const GLdouble *v)
{
   _mesa_WindowPos4fMESA(v[0], v[1], v[2], v[3]);
}

void
_mesa_WindowPos4fvMESA(const GLfloat *v)
{
   _mesa_WindowPos4fMESA(v[0], v[1], v[2], v[3]);
}

void
_mesa_WindowPos4ivMESA(const GLint *v)
{
   _mesa_WindowPos4fMESA(v[0], v[1], v[2], v[3]);
}

void
_mesa_WindowPos4svMESA(const GLshort *v)
{
   _mesa_WindowPos4fMESA(v[0], v[1], v[2], v[3]);
}



#if 0

/*
 * OpenGL implementation of glWindowPos*MESA()
 */
void glWindowPos4fMESA( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   GLfloat fx, fy;

   /* Push current matrix mode and viewport attributes */
   glPushAttrib( GL_TRANSFORM_BIT | GL_VIEWPORT_BIT );

   /* Setup projection parameters */
   glMatrixMode( GL_PROJECTION );
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode( GL_MODELVIEW );
   glPushMatrix();
   glLoadIdentity();

   glDepthRange( z, z );
   glViewport( (int) x - 1, (int) y - 1, 2, 2 );

   /* set the raster (window) position */
   fx = x - (int) x;
   fy = y - (int) y;
   glRasterPos4f( fx, fy, 0.0, w );

   /* restore matrices, viewport and matrix mode */
   glPopMatrix();
   glMatrixMode( GL_PROJECTION );
   glPopMatrix();

   glPopAttrib();
}

#endif
