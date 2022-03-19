/* $Id: vertices.c,v 1.9.4.1 2000/10/22 23:10:49 gareth Exp $ */

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
#include "types.h"
#include "vertices.h"
#endif


#if defined(USE_X86_ASM)
#include "X86/common_x86_asm.h"
#endif


/* The start of a bunch of vertex oriented geometry routines.  These
 * are expected to support the production of driver-specific fast paths
 * for CVA and eventually normal processing.
 *
 * These have been taken from fxfastpath.c, and are now also used in
 * the mga driver.
 *
 * These should grow to include:
 *     - choice of 8/16 dword vertices
 *     - ??
 *     - use of portable assembly layouts.
 *
 * More tentatively:
 *     - more (all?) matrix types
 *     - more (?) vertex sizes
 *
 * -- Keith Whitwell.
 */

/* The inline 3dnow code seems to give problems with some peoples
 * compiler/binutils.
 */
/*  #undef USE_3DNOW_ASM */


#if defined(USE_X86_ASM) && defined(__GNUC__)


#endif


static void _PROJAPI transform_v16( GLfloat *f,
				    const GLfloat *m,
				    const GLfloat *obj,
				    GLuint obj_stride,
				    GLuint count )
{
   GLuint i;

   for (i = 0 ; i < count ; i++, STRIDE_F(obj, obj_stride), f+=16)
   {
      const GLfloat ox = obj[0], oy = obj[1], oz = obj[2];
      f[0] = m[0] * ox + m[4] * oy + m[8]  * oz + m[12];
      f[1] = m[1] * ox + m[5] * oy + m[9]  * oz + m[13];
      f[2] = m[2] * ox + m[6] * oy + m[10] * oz + m[14];
      f[3] = m[3] * ox + m[7] * oy + m[11] * oz + m[15];
   }
}




static void _PROJAPI cliptest_v16( GLfloat *first,
				   GLfloat *last,
				   GLubyte *p_clipOr,
				   GLubyte *p_clipAnd,
				   GLubyte *clipmask )
{
   GLubyte clipAnd = (GLubyte) ~0;
   GLubyte clipOr = 0;
   GLfloat *f = first;
   static int i;
   i = 0;

   for ( ; f != last ; f+=16, clipmask++, i++)
   {
      const GLfloat cx = f[0];
      const GLfloat cy = f[1];
      const GLfloat cz = f[2];
      const GLfloat cw = f[3];
      GLubyte mask = 0;

      if (cx >  cw) mask |= CLIP_RIGHT_BIT;
      if (cx < -cw) mask |= CLIP_LEFT_BIT;
      if (cy >  cw) mask |= CLIP_TOP_BIT;
      if (cy < -cw) mask |= CLIP_BOTTOM_BIT;
      if (cz >  cw) mask |= CLIP_FAR_BIT;
      if (cz < -cw) mask |= CLIP_NEAR_BIT;

      *clipmask = mask;
      clipAnd &= mask;
      clipOr |= mask;
   }

   (*p_clipOr) |= clipOr;
   (*p_clipAnd) &= clipAnd;
}



/* Project all vertices upto but not including last.  Guarenteed to be
 * at least one such vertex.
 */
static void _PROJAPI project_verts( GLfloat *first,
				    GLfloat *last,
				    const GLfloat *m,
				    GLuint stride )
{
   const GLfloat sx = m[0], sy = m[5], sz = m[10];
   const GLfloat tx = m[12], ty = m[13], tz = m[14];
   GLfloat *f;

   for ( f = first ; f != last ; STRIDE_F(f,stride))
   {
      const GLfloat oow = 1.0F / f[3];
      f[0] = sx * f[0] * oow + tx;
      f[1] = sy * f[1] * oow + ty;
      f[2] = sz * f[2] * oow + tz;
      f[3] = oow;
   }
}

static void _PROJAPI project_clipped_verts( GLfloat *first,
					    GLfloat *last,
					    const GLfloat *m,
					    GLuint stride,
					    const GLubyte *clipmask )
{
   const GLfloat sx = m[0], sy = m[5], sz = m[10];
   const GLfloat tx = m[12], ty = m[13], tz = m[14];
   GLfloat *f;

   for ( f = first ; f != last ; STRIDE_F(f,stride), clipmask++)
   {
      if (!*clipmask) {
	 const GLfloat oow = 1.0F / f[3];
	 f[0] = sx * f[0] * oow + tx;
	 f[1] = sy * f[1] * oow + ty;
	 f[2] = sz * f[2] * oow + tz;
	 f[3] = oow;
      }
   }
}


GLenum gl_reduce_prim[GL_POLYGON+1] = {
   GL_POINTS,
   GL_LINES,
   GL_LINES,
   GL_LINES,
   GL_TRIANGLES,
   GL_TRIANGLES,
   GL_TRIANGLES,
   GL_TRIANGLES,
   GL_TRIANGLES,
   GL_TRIANGLES,
};


gl_transform_func	gl_xform_points3_v16_general;
gl_cliptest_func	gl_cliptest_points4_v16;
gl_project_func		gl_project_v16;
gl_project_clipped_func gl_project_clipped_v16;


void gl_init_vertices( void )
{
   gl_xform_points3_v16_general	= transform_v16;
   gl_cliptest_points4_v16	= cliptest_v16;
   gl_project_v16		= project_verts;
   gl_project_clipped_v16	= project_clipped_verts;

#if 0
   /* GH: Add tests/benchmarks for the vertex asm */
   gl_test_all_vertex_functions( "default" );
#endif

#ifdef USE_X86_ASM
   gl_init_all_x86_vertex_asm();
#endif
}
