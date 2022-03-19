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


/* Build clip space vertices from object space data.
 */
static void TAG(fx_setup_full)( struct vertex_buffer *VB, GLuint do_clip )
{
   GLcontext *ctx = VB->ctx;
   GLfloat *f = (GLfloat *) FX_DRIVER_DATA(VB)->verts;
   fxMesaContext fxMesa = FX_CONTEXT(VB->ctx);
   GLuint count = VB->Count;
   GLuint i;

   const GLfloat * const m = ctx->ModelProjectMatrix.m;

#if (TYPE & SETUP_RGBA)
   GLubyte *color = (GLubyte *)VB->ColorPtr->data;
   GLuint color_stride = VB->ColorPtr->stride;
#endif

#if (TYPE & SETUP_TMU0)
   GLuint tmu0_source = fxMesa->tmu_source[0];
   struct gl_texture_unit *t0 = &ctx->Texture.Unit[tmu0_source];
   GLint s0scale = FX_TEXTURE_DATA(t0)->int_sScale;
   GLint t0scale = FX_TEXTURE_DATA(t0)->int_tScale;
   GLint *tmu0_int_data = (GLint *)VB->TexCoordPtr[tmu0_source]->data;
   GLuint tmu0_stride = VB->TexCoordPtr[tmu0_source]->stride;
#endif

#if (TYPE & SETUP_TMU1)
   GLuint tmu1_source = fxMesa->tmu_source[1];
   struct gl_texture_unit *t1 = &ctx->Texture.Unit[tmu1_source];
   GLint s1scale = FX_TEXTURE_DATA(t1)->int_sScale;
   GLint t1scale = FX_TEXTURE_DATA(t1)->int_tScale;
   GLint *tmu1_int_data = (GLint *)VB->TexCoordPtr[tmu1_source]->data;
   GLuint tmu1_stride = VB->TexCoordPtr[tmu1_source]->stride;
#endif

   (void) fxMesa;
   (void) ctx;
   (void) i;
   (void) f;

   /* Use 3 seperate loops because it's easier for assembly.  A
    * best-case solution might be to do all three in a single assembly
    * loop.
    */
   gl_xform_points3_v16_general(FX_DRIVER_DATA(VB)->verts[0].f,
				m,
				VB->ObjPtr->start,
				VB->ObjPtr->stride,
				count);

   if (do_clip)
   {
      VB->ClipAndMask = ~0; 
      VB->ClipOrMask = 0;
      gl_cliptest_points4_v16(FX_DRIVER_DATA(VB)->verts[0].f,
			      FX_DRIVER_DATA(VB)->verts[count].f,
			      &(VB->ClipOrMask),
			      &(VB->ClipAndMask),
			      VB->ClipMask);
   }


#if (TYPE)
   for (i = 0 ; i < count ; i++, f += 16) {
#if (TYPE & SETUP_RGBA)
      GLubyte *col = color; color += color_stride;
      UBYTE_COLOR_TO_FLOAT_255_COLOR2( f[CLIP_R], col[0] );
      UBYTE_COLOR_TO_FLOAT_255_COLOR2( f[CLIP_G], col[1] );
      UBYTE_COLOR_TO_FLOAT_255_COLOR2( f[CLIP_B], col[2] );
      UBYTE_COLOR_TO_FLOAT_255_COLOR2( f[CLIP_A], col[3] );
#endif
#if (TYPE & SETUP_TMU0)
      * (int *) &f[CLIP_S0] = s0scale + tmu0_int_data[0];
      * (int *) &f[CLIP_T0] = t0scale + tmu0_int_data[1];
      STRIDE_T(tmu0_int_data, GLint, tmu0_stride);
#endif
#if (TYPE & SETUP_TMU1)
      * (int *) &f[CLIP_S1] = s1scale + tmu1_int_data[0];
      * (int *) &f[CLIP_T1] = t1scale + tmu1_int_data[1];
      STRIDE_T(tmu1_int_data, GLint, tmu1_stride);
#endif
   }
#endif

   FX_DRIVER_DATA(VB)->last_vert = &(FX_DRIVER_DATA(VB)->verts[count]);
}


/* Do viewport map, device scale and perspective projection.
 *
 * Rearrange fxVertices to look like grVertices.
 */
static void TAG(fx_project_vertices)( GLfloat *first,
				      GLfloat *last,
				      const GLfloat *mat,
				      GLuint stride )
{
   GLfloat *f;
   VARS_XYZ;

   for ( f = first ; f != last ; STRIDE_F(f, stride)) 
   {
      GLfloat oow = 1.0f/f[CLIP_WCOORD];	/* urp! */

#if FX_USE_PARGB
      if (TYPE & SETUP_RGBA) {
         PACK_4F_ARGB(GET_PARGB(f),f[CLIP_A],f[CLIP_R],f[CLIP_G],f[CLIP_B]);
      }
#else
      if (TYPE & SETUP_RGBA) {
	f[RCOORD]=f[CLIP_R];
      }
#endif
      if (TYPE & SETUP_TMU1) {
	 f[S1COORD] = f[CLIP_S1] * oow;
	 f[T1COORD] = f[CLIP_T1] * oow;
      }

      if (TYPE & SETUP_TMU0) {
	 f[T0COORD] = f[CLIP_T0] * oow;
	 f[S0COORD] = f[CLIP_S0] * oow;
      }

      DO_SETUP_XYZ;

      f[OOWCOORD] = oow;
   }
}

static void TAG(fx_project_clipped_vertices)( GLfloat *first,
					      GLfloat *last,
					      const GLfloat *mat,
					      GLuint stride,
					      const GLubyte *mask )
{
   GLfloat *f;
   VARS_XYZ;

   for ( f = first ; f != last ; STRIDE_F(f, stride), mask++) {
      if (!*mask) {

	 GLfloat oow = 1.0f / f[CLIP_WCOORD];
#if FX_USE_PARGB
        if (TYPE & SETUP_RGBA) {
           const GLuint r  = f[CLIP_R];
           const GLuint g  = f[CLIP_G];
           const GLuint b  = f[CLIP_B];
           const GLuint a  = f[CLIP_A];
           /* ToDo Optimize */
           GET_PARGB(f) = a << 24 | r << 16 | g << 8 | b;
        }
#else
        if (TYPE & SETUP_RGBA) {
	  f[RCOORD]=f[CLIP_R];
        }
#endif

	 if (TYPE & SETUP_TMU1) {
	    f[S1COORD] = f[CLIP_S1] * oow;
	    f[T1COORD] = f[CLIP_T1] * oow;
	 }

	 if (TYPE & SETUP_TMU0) {
	    f[T0COORD] = f[CLIP_T0] * oow;
	    f[S0COORD] = f[CLIP_S0] * oow;
	 }

	 DO_SETUP_XYZ;

	 f[OOWCOORD] = oow;
      }
   }
}


static
#if (SIZE <= 8)
INLINE
#endif
void TAG(fx_tri_clip)( GLuint **p_elts,
		       fxVertex *verts,
		       GLubyte *clipmask,
		       GLuint *p_next_vert,
		       GLubyte mask )
{
   GLuint *elts = *p_elts;
   GLuint next_vert = *p_next_vert;
   GLuint vlist1[VB_MAX_CLIPPED_VERTS];
   GLuint vlist2[VB_MAX_CLIPPED_VERTS];
   GLuint *inlist[2];
   GLuint *out;
   GLuint in = 0;
   GLuint n = 3;
   GLuint i;

   inlist[0] = elts;
   inlist[1] = vlist2;

   CLIP(-,0,CLIP_RIGHT_BIT);
   CLIP(+,0,CLIP_LEFT_BIT);
   CLIP(-,1,CLIP_TOP_BIT);
   CLIP(+,1,CLIP_BOTTOM_BIT);
   CLIP(-,2,CLIP_FAR_BIT);
   CLIP(+,2,CLIP_NEAR_BIT);

   /* Convert the planar polygon to a list of triangles.
    */
   out = inlist[in];

   for (i = 2 ; i < n ; i++) {
      elts[0] = out[0];
      elts[1] = out[i-1];
      elts[2] = out[i];
      elts += 3;
   }

   *p_next_vert = next_vert;
   *p_elts = elts;
}


static INLINE void TAG(fx_line_clip)( GLuint **p_elts,
				      fxVertex *verts,
				      GLubyte *clipmask,
				      GLuint *p_next_vert,
				      GLubyte mask )
{
   GLuint *elts = *p_elts;
   GLfloat *I = verts[elts[0]].f;
   GLfloat *J = verts[elts[1]].f;
   GLuint next_vert = *p_next_vert;

   LINE_CLIP(1,0,0,-1,CLIP_LEFT_BIT);
   LINE_CLIP(-1,0,0,1,CLIP_RIGHT_BIT);
   LINE_CLIP(0,1,0,-1,CLIP_TOP_BIT);
   LINE_CLIP(0,-1,0,1,CLIP_BOTTOM_BIT);
   LINE_CLIP(0,0,1,-1,CLIP_FAR_BIT);
   LINE_CLIP(0,0,-1,1,CLIP_NEAR_BIT);

   *p_next_vert = next_vert;
   *p_elts += 2;
}


/* Build a table of functions to clip each primitive type.
 */
#define LOCAL_VARS							\
   GLuint *elt = VB->EltPtr->data;					\
   fxVertex *verts = FX_DRIVER_DATA(VB)->verts;				\
   GLuint next_vert = VB->Count;					\
   GLuint *out = FX_DRIVER_DATA(VB)->clipped_elements.data;		\
   GLubyte *mask = VB->ClipMask;                                        \


#define POSTFIX							\
   FX_DRIVER_DATA(VB)->clipped_elements.count = 		\
          out - FX_DRIVER_DATA(VB)->clipped_elements.data;	\
   FX_DRIVER_DATA(VB)->last_vert = &verts[next_vert];

#define INIT(x)

#define RENDER_POINTS(start, count)			\
do {							\
   GLuint i;						\
   for (i = start ; i < count ; i++ )			\
      CLIP_POINT( elt[i] );				\
} while (0)

#define RENDER_LINE(i1, i0) 			\
   CLIP_LINE(elt[i1], elt[i0])

#define RENDER_TRI(i2, i1, i0, pv, parroty)		\
do {							\
   GLuint e2 = elt[i2], e1 = elt[i1], e0 = elt[i0];	\
   if (parroty) e2 = elt[i1], e1 = elt[i2];		\
   CLIP_TRIANGLE( e2, e1, e0 );				\
} while (0)

#define RENDER_QUAD(i3, i2, i1, i0, pv)		\
  CLIP_TRIANGLE(elt[i3], elt[i2], elt[i0]);	\
  CLIP_TRIANGLE(elt[i2], elt[i1], elt[i0])

#define PRESERVE_TAG
#include "render_tmp.h"


static void TAG(fx_init_fastpath)( struct fx_fast_tab *tab )
{
   GLuint i;

   /* Use the render templates to do clipping.
    */
   TAG(render_init)();
   for (i = 0 ; i < GL_POLYGON+2 ; i++)
      tab->clip[i] = TAG(render_tab)[i];

   tab->build_vertices = TAG(fx_setup_full);
   tab->project_vertices = TAG(fx_project_vertices);
   tab->project_clipped_vertices = TAG(fx_project_clipped_vertices);

#if defined(USE_3DNOW_ASM) 
   if (gl_x86_cpu_features & X86_FEATURE_3DNOW) {
      extern void TAG(fx_3dnow_project_vertices)( GLfloat *first,
						  GLfloat *last,
						  const GLfloat *mat,
						  GLuint stride );

      extern void TAG(fx_3dnow_project_clipped_vertices)( GLfloat *first,
							  GLfloat *last,
							  const GLfloat *mat,
							  GLuint stride,
							  const GLubyte *mask );

      tab->project_vertices = TAG(fx_3dnow_project_vertices);
      tab->project_clipped_vertices = TAG(fx_3dnow_project_clipped_vertices);
   }
#endif
}

#undef TYPE
#undef TAG
#undef SIZE
