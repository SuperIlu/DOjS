/* -*- mode: C; tab-width:8; c-basic-offset:2 -*- */

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
 *
 *
 * Original Mesa / 3Dfx device driver (C) 1999 David Bucciarelli, by the
 * terms stated above.
 *
 * Thank you for your contribution, David!
 *
 * Please make note of the above copyright/license statement.  If you
 * contributed code or bug fixes to this code under the previous (GNU
 * Library) license and object to the new license, your code will be
 * removed at your request.  Please see the Mesa docs/COPYRIGHT file
 * for more information.
 *
 * Additional Mesa/3Dfx driver developers:
 *   Daryll Strauss <daryll@precisioninsight.com>
 *   Keith Whitwell <keith@precisioninsight.com>
 *
 * See fxapi.h for more revision/author details.
 */


#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif


#if defined(FX)

#include "types.h"
#include "cva.h"
#include "mmath.h"
#include "fxdrv.h"
#include "vertices.h"
#include "X86/common_x86_asm.h"


#if 0 && defined(__i386__)
#define NEGATIVE(f)          ((*(int *)&f) < 0)
#define DIFFERENT_SIGNS(a,b) (((*(int *)&a)^(*(int *)&b)) < 0)
#else
#define NEGATIVE(f)          (f < 0)
#define DIFFERENT_SIGNS(a,b) ((a*b) < 0)
#endif

#define LINTERP( T, A, B )   ( (A) + (T) * ( (B) - (A) ) )


#define CLIP(sgn,v,PLANE)					\
if (mask & PLANE) {						\
   GLuint *indata = inlist[in];					\
   GLuint *outdata = inlist[in ^= 1];				\
   GLuint nr = n;						\
   GLfloat *J = verts[indata[nr-1]].f;				\
   GLfloat dpJ = (sgn J[v]) + J[CLIP_WCOORD];			\
								\
   inlist[0] = vlist1;						\
   for (i = n = 0 ; i < nr ; i++) {				\
      GLuint elt_i = indata[i];					\
      GLfloat *I = verts[elt_i].f;				\
      GLfloat dpI = (sgn I[v]) + I[CLIP_WCOORD];		\
								\
      if (DIFFERENT_SIGNS(dpI, dpJ)) {				\
	 GLfloat *O = verts[next_vert].f;			\
         GLfloat t = dpI / (dpI - dpJ);				\
	 GLuint j;						\
								\
	 clipmask[next_vert] = 0;				\
	 outdata[n++] = next_vert++;				\
								\
         for (j = 0 ; j < SIZE ; j += 2) {			\
	    O[j]   = LINTERP(t, I[j],   J[j]);			\
	    O[j+1] = LINTERP(t, I[j+1], J[j+1]);	       	\
	 }							\
      }								\
								\
      clipmask[elt_i] |= PLANE;		/* don't set up */	\
								\
      if (!NEGATIVE(dpI)) {					\
	 outdata[n++] = elt_i;					\
	 clipmask[elt_i] &= ~PLANE;	/* set up after all */	\
      }								\
								\
      J = I;							\
      dpJ = dpI;						\
   }								\
								\
   if (n < 3) return;						\
}

#define LINE_CLIP(x,y,z,w,PLANE)				\
if (mask & PLANE) {						\
   GLfloat dpI = DOT4V(I,x,y,z,w);				\
   GLfloat dpJ = DOT4V(J,x,y,z,w);				\
								\
   if (DIFFERENT_SIGNS(dpI, dpJ)) {				\
      GLfloat *O = verts[next_vert].f;				\
      GLfloat t = dpI / (dpI - dpJ);				\
      GLuint j;							\
								\
      for (j = 0 ; j < SIZE ; j += 2) {				\
         O[j]   = LINTERP(t, I[j],   J[j]);			\
	 O[j+1] = LINTERP(t, I[j+1], J[j+1]);	       		\
      }								\
								\
      clipmask[next_vert] = 0;					\
								\
      if (NEGATIVE(dpI)) {					\
  	 clipmask[elts[0]] |= PLANE;				\
	 I = O; elts[0] = next_vert++;				\
      } else {							\
  	 clipmask[elts[1]] |= PLANE;				\
	 J = O;	elts[1] = next_vert++;				\
      }								\
   }								\
   else if (NEGATIVE(dpI))					\
      return;							\
}


#define CLIP_POINT( e )				\
   if (mask[e])					\
      *out++ = e

#define CLIP_LINE( e1, e0 )						\
do {									\
   GLubyte ormask = mask[e0] | mask[e1];				\
   out[0] = e1;								\
   out[1] = e0;								\
   out+=2;								\
   if (ormask) {							\
      out-=2;								\
      if (!(mask[e0] & mask[e1])) {					\
	 TAG(fx_line_clip)( &out, verts, mask, &next_vert, ormask);	\
      }									\
   }									\
} while (0)

#define CLIP_TRIANGLE( e2, e1, e0 )					\
do {									\
   GLubyte ormask;							\
   out[0] = e2;								\
   out[1] = e1;								\
   out[2] = e0;								\
   out += 3;								\
   ormask = mask[e2] | mask[e1] | mask[e0];				\
   if (ormask) {							\
      out -= 3;								\
      if ( !(mask[e2] & mask[e1] & mask[e0])) {				\
	 TAG(fx_tri_clip)( &out, verts, mask, &next_vert, ormask );	\
      }									\
   }									\
} while (0)

#if defined(FX_V2) || defined(DRIVERTS)

#define VARS_XYZ				\
  GLfloat vsx = mat[MAT_SX];			\
  GLfloat vsy = mat[MAT_SY];			\
  GLfloat vsz = mat[MAT_SZ];			\
  GLfloat vtx = mat[MAT_TX];			\
  GLfloat vty = mat[MAT_TY];			\
  GLfloat vtz = mat[MAT_TZ];

#define DO_SETUP_XYZ				\
  f[XCOORD] = f[0] * oow * vsx + vtx;		\
  f[YCOORD] = f[1] * oow * vsy + vty;		\
  f[ZCOORD] = f[2] * oow * vsz + vtz;

#else
#if defined(HAVE_FAST_MATH)

#define VARS_XYZ				\
  GLfloat vsx = mat[MAT_SX];			\
  GLfloat vsy = mat[MAT_SY];			\
  GLfloat vsz = mat[MAT_SZ];			\
  const GLfloat snapper = (3L << 18);		\
  GLfloat vtx = mat[MAT_TX] + snapper;		\
  GLfloat vty = mat[MAT_TY] + snapper;		\
  GLfloat vtz = mat[MAT_TZ];

#define DO_SETUP_XYZ				\
  f[XCOORD] = f[0] * oow * vsx + vtx;		\
  f[XCOORD] -= snapper;				\
  f[YCOORD] = f[1] * oow * vsy + vty;		\
  f[YCOORD] -= snapper;				\
  f[ZCOORD] = f[2] * oow * vsz + vtz;

#else

#define VARS_XYZ				\
  GLfloat vsx = mat[MAT_SX] * 16.0f;		\
  GLfloat vsy = mat[MAT_SY] * 16.0f;		\
  GLfloat vsz = mat[MAT_SZ];			\
  GLfloat vtx = mat[MAT_TX] * 16.0f;		\
  GLfloat vty = mat[MAT_TY] * 16.0f;		\
  GLfloat vtz = mat[MAT_TZ];

#define DO_SETUP_XYZ					\
  f[XCOORD] = ((int)(f[0]*oow*vsx+vtx)) * (1.0f/16.0f);	\
  f[YCOORD] = ((int)(f[1]*oow*vsy+vty)) * (1.0f/16.0f);	\
  f[ZCOORD] = f[2]*oow*vsz + vtz;


#endif
#endif



struct fx_fast_tab
{
   void (*build_vertices)( struct vertex_buffer *VB, GLuint do_clip );

   void (*clip[GL_POLYGON+2])( struct vertex_buffer *VB,
			       GLuint start,
			       GLuint count,
			       GLuint parity );

   void (*project_clipped_vertices)( GLfloat *first,
				     GLfloat *last,
				     const GLfloat *mat,
				     GLuint stride,
				     const GLubyte *mask );

   void (*project_vertices)( GLfloat *first,
			     GLfloat *last,
			     const GLfloat *mat,
			     GLuint stride );
};

/* Pack either rgba or texture into the remaining half of a 32 byte vertex.
 */
#define CLIP_R  CLIP_RCOORD
#define CLIP_G  CLIP_GCOORD
#define CLIP_B  CLIP_BCOORD
#define CLIP_A  CLIP_ACOORD
#define CLIP_S0 4
#define CLIP_T0 5
#define CLIP_S1 6
#define CLIP_T1 7

#define SIZE 4
#define TYPE (0)
#define TAG(x) x
#include "fxfasttmp.h"

#define SIZE 8
#define TYPE (SETUP_RGBA)
#define TAG(x) x##_RGBA
#include "fxfasttmp.h"

#define SIZE 6
#define TYPE (SETUP_TMU0)
#define TAG(x) x##_TMU0
#include "fxfasttmp.h"

#define SIZE 8
#define TYPE (SETUP_TMU0|SETUP_TMU1)
#define TAG(x) x##_TMU0_TMU1
#include "fxfasttmp.h"

#undef CLIP_S1
#undef CLIP_T1
#define CLIP_S1 4
#define CLIP_T1 5

#define SIZE 6
#define TYPE (SETUP_TMU1)
#define TAG(x) x##_TMU1
#include "fxfasttmp.h"

/* These three need to use a full 64 byte clip-space vertex.
 */
#undef CLIP_S0
#undef CLIP_T0
#undef CLIP_S1
#undef CLIP_T1

#define CLIP_S0 8
#define CLIP_T0 9
#define CLIP_S1 10
#define CLIP_T1 11

#define SIZE 10
#define TYPE (SETUP_RGBA|SETUP_TMU0)
#define TAG(x) x##_RGBA_TMU0
#include "fxfasttmp.h"

#define SIZE 12
#define TYPE (SETUP_RGBA|SETUP_TMU0|SETUP_TMU1)
#define TAG(x) x##_RGBA_TMU0_TMU1
#include "fxfasttmp.h"

#undef CLIP_S1
#undef CLIP_T1
#define CLIP_S1 8
#define CLIP_T1 9

#define SIZE 10
#define TYPE (SETUP_RGBA|SETUP_TMU1)
#define TAG(x) x##_RGBA_TMU1
#include "fxfasttmp.h"

static struct fx_fast_tab fxFastTab[0x8];

void fxDDFastPathInit()
{
   fx_init_fastpath( &fxFastTab[0] );
   fx_init_fastpath_RGBA( &fxFastTab[SETUP_RGBA] );
   fx_init_fastpath_TMU0( &fxFastTab[SETUP_TMU0] );
   fx_init_fastpath_TMU1( &fxFastTab[SETUP_TMU1] );
   fx_init_fastpath_RGBA_TMU0( &fxFastTab[SETUP_RGBA|SETUP_TMU0] );
   fx_init_fastpath_RGBA_TMU1( &fxFastTab[SETUP_RGBA|SETUP_TMU1] );
   fx_init_fastpath_TMU0_TMU1( &fxFastTab[SETUP_TMU0|SETUP_TMU1] );
   fx_init_fastpath_RGBA_TMU0_TMU1( &fxFastTab[SETUP_RGBA|SETUP_TMU0|
					       SETUP_TMU1] );
}



void fxDDFastPath( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLenum prim = ctx->CVA.elt_mode;
   struct tfxMesaContext *fxMesa = FX_CONTEXT(ctx);
   struct fx_fast_tab *tab = &fxFastTab[fxMesa->setupindex & 0x7];
   GLuint do_clip = 1;
   struct tfxMesaVertexBuffer *fxVB = FX_DRIVER_DATA(VB);
#ifdef DRIVERTS
   GLfloat tx, ty;
#endif

   fxVertex *first;
   GLfloat *mat = ctx->Viewport.WindowMap.m;

   gl_prepare_arrays_cva( VB );	                /* still need this */

   if (VB->EltPtr->count * 12 > fxVB->size) {
      fxDDResizeVB( VB, VB->EltPtr->count * 12 );
      do_clip = 1;
   }

   tab->build_vertices( VB, do_clip );          /* object->clip space */

   first = FX_DRIVER_DATA(VB)->verts;

#ifdef DRIVERTS
   tx=mat[MAT_TX];
   ty=mat[MAT_TY];
   mat[MAT_TX]=tx+fxMesa->x_offset;
   mat[MAT_TY]=ty+fxMesa->y_delta;
#endif

   if (VB->ClipOrMask) {
      if (!VB->ClipAndMask) {
	 GLubyte tmp = VB->ClipOrMask;

	 tab->clip[prim]( VB, 0, VB->EltPtr->count, 0 );   /* clip */

	 tab->project_clipped_vertices( fxVB->verts->f,
					fxVB->last_vert->f,
					mat, 16 * 4,
					VB->ClipMask );

	 ctx->CVA.elt_mode = gl_reduce_prim[prim];
	 VB->EltPtr = &(FX_DRIVER_DATA(VB)->clipped_elements);

	 VB->ClipOrMask = 0;
	 fxDDRenderElementsDirect( VB );        /* render using new list */
	 VB->ClipOrMask = tmp;
      }
   } else {
      tab->project_vertices(  fxVB->verts->f,
			      fxVB->last_vert->f,
			      mat, 16 * 4 );

      fxDDRenderElementsDirect( VB );           /* render using orig list */
   }

#ifdef DRIVERTS
   mat[MAT_TX]=tx;
   mat[MAT_TY]=ty;
#endif

   /* This indicates that there is no cached data to reuse.
    */
   VB->pipeline->data_valid = 0;
   VB->pipeline->pipeline_valid = 0;
}


#else

/*
 * Need this to provide at least one external definition.
 */
int gl_fxfastpath_dummy(void)
{
  return 0;
}

#endif
