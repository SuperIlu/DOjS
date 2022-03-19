/* $Id: bbox.c,v 1.2.4.1 2000/11/05 21:24:00 brianp Exp $ */

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

/*
 * New (3.1) transformation code written by Keith Whitwell
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "bbox.h"
#include "types.h"
#include "xform.h"
#endif



static void cliptest_bounds( GLubyte *orMask, 
			     GLubyte *andMask,
			     CONST GLfloat (*bounds)[4],
			     GLuint count )
{
   GLubyte tmpOrMask = 0;
   GLubyte tmpAndMask = ~0;
   GLuint i;   

   for (i = 0 ; i < count ; i++) {
      const GLfloat cx = bounds[i][0];
      const GLfloat cy = bounds[i][1];
      const GLfloat cz = bounds[i][2];
      const GLfloat cw = bounds[i][3];
      GLubyte mask = 0;

      if (cx > cw)       mask |= CLIP_RIGHT_BIT;
      else if (cx < -cw) mask |= CLIP_LEFT_BIT;
      if (cy > cw)       mask |= CLIP_TOP_BIT;
      else if (cy < -cw) mask |= CLIP_BOTTOM_BIT;
      if (cz > cw)       mask |= CLIP_FAR_BIT;
      else if (cz < -cw) mask |= CLIP_NEAR_BIT;

      tmpOrMask |= mask;
      tmpAndMask &= mask;
   }

   *orMask = tmpOrMask;
   *andMask = tmpAndMask;
}


static void transform_bounds3( GLubyte *orMask, GLubyte *andMask,
			       const GLmatrix *mat,
			       CONST GLfloat src[][3] )
{
   GLuint i;
   GLfloat dx[4], dy[4], dz[4];
   GLfloat data[8][4];
   const GLfloat *m = mat->m;

   /* Do the first transform */
   gl_transform_point_sz( data[0], mat->m, src[0], 3 );

   for (i = 1 ; i < 8 ; i++)
      COPY_4FV( data[i], data[0] );

   dx[0] = m[ 0]*src[1][0];
   dx[1] = m[ 1]*src[1][0];
   dx[2] = m[ 2]*src[1][0];
   dx[3] = m[ 3]*src[1][0];
   
   for (i = 4 ; i < 8 ; i++)
      ACC_4V( data[i], dx );
   
   dy[0] = m[ 4]*src[1][1];
   dy[1] = m[ 5]*src[1][1];
   dy[2] = m[ 6]*src[1][1];
   dy[3] = m[ 7]*src[1][1];

   ACC_4V( data[2], dy );
   ACC_4V( data[3], dy );
   ACC_4V( data[6], dy );
   ACC_4V( data[7], dy );

   dz[0] = m[ 8]*src[1][2];
   dz[1] = m[ 9]*src[1][2];
   dz[2] = m[10]*src[1][2];
   dz[3] = m[11]*src[1][2];

   for (i = 1 ; i < 8 ; i+=2) 
      ACC_4V( data[i], dz );

   
   cliptest_bounds( orMask, andMask, (CONST GLfloat (*)[4])data, 8 );
}

static void transform_bounds2( GLubyte *orMask, GLubyte *andMask, 
			       const GLmatrix *mat,
			       CONST GLfloat src[][3] )
{
   GLuint i;
   GLfloat dx[4], dy[4];
   GLfloat data[4][4];
   const GLfloat *m = mat->m;

   /* Do the first transform */
   gl_transform_point_sz( data[0], mat->m, src[0], 2 );

   for (i = 1 ; i < 4 ; i++)
      COPY_4FV( data[i], data[0] );

   dx[0] = m[0]*src[1][0];
   dx[1] = m[1]*src[1][0];
   dx[2] = m[2]*src[1][0];
   dx[3] = m[3]*src[1][0];
   
   ACC_4V( data[1], dx );
   ACC_4V( data[3], dx );
   
   dy[0] = m[4]*src[1][1];
   dy[1] = m[5]*src[1][1];
   dy[2] = m[6]*src[1][1];
   dy[3] = m[7]*src[1][1];

   ACC_4V( data[2], dy );
   ACC_4V( data[3], dy );

   cliptest_bounds( orMask, andMask, (CONST GLfloat (*)[4])data, 4 );
}

/* Dummy
 */
static void transform_bounds4( GLubyte *orMask, GLubyte *andMask,
			       const GLmatrix *mat,
			       CONST GLfloat src[][3] )
{
   (void) mat;
   (void) src;
   *orMask = ~0;
   *andMask = 0;
}

static void calculate_bounds3( GLfloat (*bounds)[3],
                               const GLvector4f *obj )
{
   CONST GLfloat (*data)[4] = (CONST GLfloat (*)[4])obj->start;
   GLfloat xmin = data[0][0];
   GLfloat ymin = data[0][1];
   GLfloat zmin = data[0][2];
   GLfloat xmax = data[0][0];
   GLfloat ymax = data[0][1];
   GLfloat zmax = data[0][2];
   GLuint i, count = obj->count;
   
   for (i = 1 ; i < count ; i++) {
      GLfloat t;
      t = data[i][0]; if (t > xmax) xmax = t; else if (t < xmin) xmin = t;
      t = data[i][1]; if (t > ymax) ymax = t; else if (t < ymin) ymin = t;
      t = data[i][2]; if (t > zmax) zmax = t; else if (t < zmin) zmin = t;
   }
   
   bounds[0][0] = xmin;
   bounds[0][1] = ymin;
   bounds[0][2] = zmin;
   bounds[1][0] = xmax - xmin;
   bounds[1][1] = ymax - ymin;
   bounds[1][2] = zmax - zmin;
}

static void calculate_bounds2( GLfloat (*bounds)[3],
                               const GLvector4f *obj )
{
   CONST GLfloat (*data)[4] = (CONST GLfloat (*)[4])obj->start;
   GLfloat xmin = data[0][0];
   GLfloat ymin = data[0][1];
   GLfloat xmax = data[0][0];
   GLfloat ymax = data[0][1];
   GLuint i, count = obj->count;
   
   for (i = 1 ; i < count ; i++) {
      GLfloat t;
      t = data[i][0]; if (t > xmax) xmax = t; if (t < xmin) xmin = t;
      t = data[i][1]; if (t > ymax) ymax = t; if (t < ymin) ymin = t;
   }
   
   bounds[0][0] = xmin;
   bounds[0][1] = ymin;
   bounds[0][2] = 0;
   bounds[1][0] = xmax - xmin;
   bounds[1][1] = ymax - ymin;
   bounds[1][2] = 0;
}


calc_bound_func gl_calc_bound_tab[5] = 
{
   0,
   0,
   calculate_bounds2,
   calculate_bounds3,
   0				/* would 4d bounds make sense? */
};

test_bound_func gl_test_bound_tab[5] = 
{
   0,
   0,
   transform_bounds2,
   transform_bounds3,
   transform_bounds4
};

