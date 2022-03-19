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

#include "fxdrv.h"
#include "mmath.h"
#include "macros.h"
#include "vector.h"
#include "types.h"


#if 0 && defined(__i386__)
#define NEGATIVE(f)          ((*(int *)&f) < 0)
#define DIFFERENT_SIGNS(a,b) (((*(int *)&a)^(*(int *)&b)) < 0)
#else
#define NEGATIVE(f)          (f < 0)
#define DIFFERENT_SIGNS(a,b) ((a*b) < 0)
#endif

#define LINTERP( T, A, B )   ( (A) + (T) * ( (B) - (A) ) )


#define CLIP(sgn,v)						\
do {								\
   GLuint out = in ^ 1;						\
   GLfloat **indata = inlist[in];				\
   GrVertex **inverts = vertlist[in];				\
   GrVertex **outverts = vertlist[out];				\
   GLfloat **outdata = inlist[in = out];			\
   GLfloat *J = indata[n-1];					\
   GLfloat dpJ = (sgn J[v]) + J[3];				\
   GLuint nr = n;						\
								\
   for (i = n = 0 ; i < nr ; i++) {				\
      GLfloat *I = indata[i];					\
      GLfloat dpI = (sgn I[v]) + I[3];				\
								\
      if (DIFFERENT_SIGNS(dpI, dpJ)) {				\
	 GLuint j;						\
         GLfloat t = dpI / (dpI - dpJ);                         \
	 outverts[n] = 0;					\
	 outdata[n++] = store;					\
	 store[3] = LINTERP(t, I[3], J[3]);			\
	 store[v] = - sgn store[3];				\
	 if (v != 0) store[0] = LINTERP(t, I[0], J[0]);		\
	 if (v != 1) store[1] = LINTERP(t, I[1], J[1]);		\
	 if (v != 2) store[2] = LINTERP(t, I[2], J[2]);		\
	 store += 4;						\
	 for (j = 4 ; j < sz ; j+=4,store+=4) {			\
	    store[0] = LINTERP(t, I[j],   J[j] );		\
	    store[1] = LINTERP(t, I[j+1], J[j+1] );		\
	    store[2] = LINTERP(t, I[j+2], J[j+2] );		\
	    store[3] = LINTERP(t, I[j+3], J[j+3] );		\
	 }							\
      }								\
								\
      if (!NEGATIVE(dpI)) {					\
	 outverts[n] = inverts[i];				\
	 outdata[n++] = I;					\
      }								\
								\
								\
      J = I;							\
      dpJ = dpI;						\
   }								\
								\
   if (n < 3) return 0;						\
} while (0)


/* Originally used this for the viewclip planes as well, as in
 * CLIP(-1,0,0,1), which was just as fast, but tended to lead to
 * cracks.  I haven't figured out exactly why this is, but the above
 * code only really differs in the way it sets store[v] to +- w.
 */
#define UCLIP(a,b,c,d)					\
do {							\
   GLuint out = in ^ 1;					\
   GLfloat **indata = inlist[in];			\
   GrVertex **inverts = vertlist[in];			\
   GrVertex **outverts = vertlist[out];			\
   GLfloat **outdata = inlist[in = out];		\
   GLfloat *J = indata[n-1];				\
   GLfloat dpJ = DOT4V(J,a,b,c,d);			\
   GLuint nr = n;					\
							\
   for (i = n = 0 ; i < nr ; i++) {			\
      GLfloat *I = indata[i];				\
      GLfloat dpI = DOT4V(I,a,b,c,d);			\
							\
      if (DIFFERENT_SIGNS(dpI, dpJ)) {			\
	 GLuint j;					\
	 GLfloat t = dpI / (dpI - dpJ);			\
	 outverts[n] = 0;				\
	 outdata[n++] = store;				\
	 for (j = 0 ; j < sz ; j+=4,store+=4) {		\
	    store[0] = LINTERP(t, I[j],   J[j] );	\
	    store[1] = LINTERP(t, I[j+1], J[j+1] );	\
	    store[2] = LINTERP(t, I[j+2], J[j+2] );	\
	    store[3] = LINTERP(t, I[j+3], J[j+3] );	\
	 }						\
      }							\
							\
      if (!NEGATIVE(dpI)) {				\
	 outverts[n] = inverts[i];			\
	 outdata[n++] = I;				\
      }							\
							\
							\
      J = I;						\
      dpJ = dpI;					\
   }							\
							\
   if (n < 3) return 0;					\
} while (0)



/* Data parameter is organized as 4 clip coordinates followed by an
 * arbitary number (sz-4) of additional data.  The three original
 * vertices are packed together at the start, and there is room for at
 * least VB_MAX_CLIPPED_VERTS vertices of the same size in this
 * storage.
 *
 */
static INLINE GLuint fx_clip_triangle( GLcontext *ctx,
				       GLfloat *inoutlist[],
				       GrVertex **verts,
				       GLuint sz,
				       GLuint mask )
{
   GLuint n = 3;
   GLfloat *store = inoutlist[n-1] + sz;
   GLfloat *vlist[VB_MAX_CLIPPED_VERTS];
   GrVertex *verts2[VB_MAX_CLIPPED_VERTS];
   GLfloat **inlist[2];
   GrVertex **vertlist[2];
   GLuint in = 0;
   GLuint i;

   inlist[0] = inoutlist;
   inlist[1] = vlist;

   vertlist[0] = verts;
   vertlist[1] = verts2;

   if (mask & CLIP_ALL_BITS)
   {
      if (mask & CLIP_RIGHT_BIT)
	 CLIP(-,0);

      if (mask & CLIP_LEFT_BIT)
	 CLIP(+,0);

      if (mask & CLIP_TOP_BIT)
	 CLIP(-,1);

      if (mask & CLIP_BOTTOM_BIT)
	 CLIP(+,1);

      if (mask & CLIP_FAR_BIT)
	 CLIP(-,2);

      if (mask & CLIP_NEAR_BIT)
	 CLIP(+,2);
   }

   if (mask & CLIP_USER_BIT) {
      GLuint bit;
      GLfloat (*plane)[4] = &ctx->Transform.ClipUserPlane[0];
      for (bit = 0x100 ; bit < mask ; plane++, bit *= 2) {
	 if (mask & bit) {
	    GLfloat a = plane[0][0];
	    GLfloat b = plane[0][1];
	    GLfloat c = plane[0][2];
	    GLfloat d = plane[0][3];
	    UCLIP(a,b,c,d);
	 }
      }
   }

   if (inlist[in] != inoutlist)
      for (i = 0 ; i < n ; i++) {
	 inoutlist[i] = inlist[in][i];
	 verts[i] = verts2[i];
      }

   return n;
}



static INLINE GLuint fx_view_clip_triangle( GLcontext *ctx,
					    GLfloat *inoutlist[],
					    GrVertex **verts,
					    GLuint sz,
					    GLubyte mask )
{
   GLuint n = 3;
   GLfloat *store = inoutlist[n-1] + sz;
   GLfloat *vlist[VB_MAX_CLIPPED_VERTS];
   GrVertex *verts2[VB_MAX_CLIPPED_VERTS];
   GLfloat **inlist[2];
   GrVertex **vertlist[2];
   GLuint in = 0;
   GLuint i;

   inlist[0] = inoutlist;
   inlist[1] = vlist;

   vertlist[0] = verts;
   vertlist[1] = verts2;

   if (mask & CLIP_RIGHT_BIT)
      CLIP(-,0);

   if (mask & CLIP_LEFT_BIT)
      CLIP(+,0);

   if (mask & CLIP_TOP_BIT)
      CLIP(-,1);

   if (mask & CLIP_BOTTOM_BIT)
      CLIP(+,1);

   if (mask & CLIP_FAR_BIT)
      CLIP(-,2);

   if (mask & CLIP_NEAR_BIT)
      CLIP(+,2);

   if (inlist[in] != inoutlist)
      for (i = 0 ; i < n ; i++) {
	 inoutlist[i] = inlist[in][i];
	 verts[i] = verts2[i];
      }

   return n;
}



#undef CLIP

#define CLIP(x,y,z,w)					\
do {							\
   GLfloat dpI = DOT4V(I,x,y,z,w);			\
   GLfloat dpJ = DOT4V(J,x,y,z,w);			\
							\
   if (DIFFERENT_SIGNS(dpI, dpJ)) {			\
      GLuint j;						\
      GLfloat t = dpI / (dpI - dpJ); 			\
      GLfloat *tmp = store;				\
							\
      for (j = 0 ; j < sz ; j+=2) {			\
	 *store++ = LINTERP(t, I[j],   J[j] );		\
	 *store++ = LINTERP(t, I[j+1], J[j+1] );	\
      }							\
							\
      if (NEGATIVE(dpI)) 				\
	 I = tmp;					\
      else						\
	 J = tmp;					\
							\
   } 							\
   else if (NEGATIVE(dpI))				\
      return 0;						\
							\
} while (0)


static GLuint fx_clip_line( GLcontext *ctx,
			    GLfloat *inoutlist[],
			    GLuint sz,
			    GLubyte clipor )
{
   GLfloat *I = inoutlist[0];
   GLfloat *J = inoutlist[1];
   GLfloat *store = J + sz;

   if (clipor & CLIP_ALL_BITS)
   {
      if (clipor & CLIP_LEFT_BIT)
	 CLIP(1,0,0,1);
 
      if (clipor & CLIP_RIGHT_BIT)
	 CLIP(-1,0,0,1);
 
      if (clipor & CLIP_TOP_BIT)
	 CLIP(0,-1,0,1);
 
      if (clipor & CLIP_BOTTOM_BIT)
	 CLIP(0,1,0,1);
 
      if (clipor & CLIP_FAR_BIT)
	 CLIP(0,0,-1,1);
 
      if (clipor & CLIP_NEAR_BIT)
	 CLIP(0,0,1,1);
   }

   if (clipor & CLIP_USER_BIT) {
      GLuint i;
      for (i = 0 ; i < MAX_CLIP_PLANES ; i++) {
	 if (ctx->Transform.ClipEnabled[i]) {
	    GLfloat a = ctx->Transform.ClipUserPlane[i][0];
	    GLfloat b = ctx->Transform.ClipUserPlane[i][1];
	    GLfloat c = ctx->Transform.ClipUserPlane[i][2];
	    GLfloat d = ctx->Transform.ClipUserPlane[i][3];
	    CLIP(a,b,c,d);
	 }
      }
   }

   inoutlist[0] = I;
   inoutlist[1] = J;

   return 2;
}






#if defined(FX_V2)

#define VARS_XYZW				\
  GLfloat vsx = mat[MAT_SX];			\
  GLfloat vsy = mat[MAT_SY];			\
  GLfloat vsz = mat[MAT_SZ];			\
  GLfloat vtx = mat[MAT_TX];			\
  GLfloat vty = mat[MAT_TY];			\
  GLfloat vtz = mat[MAT_TZ];			

#define DO_SETUP_XYZW				\
{						\
  GLfloat oow = 1.0 / data[3];			\
  v->x   = data[0]*oow*vsx + vtx;		\
  v->y   = data[1]*oow*vsy + vty;		\
  v->ooz = data[2]*oow*vsz + vtz;		\
  v->oow = oow; 				\
}
#else

#if defined(DRIVERTS)

#define VARS_XYZW				\
  GLfloat vsx = mat[MAT_SX];			\
  GLfloat vsy = mat[MAT_SY];			\
  GLfloat vsz = mat[MAT_SZ];			\
  GLfloat vtx = mat[MAT_TX]+fxMesa->x_offset;	\
  GLfloat vty = mat[MAT_TY]+fxMesa->y_delta;	\
  GLfloat vtz = mat[MAT_TZ];

#define DO_SETUP_XYZW				\
{						\
  GLfloat oow = 1.0 / data[3];			\
  v->x   = data[0]*oow*vsx + vtx;		\
  v->y   = data[1]*oow*vsy + vty;		\
  v->ooz = data[2]*oow*vsz + vtz;		\
  v->oow = oow;					\
}

#else
#define VARS_XYZW 				\
  GLfloat vsx = mat[MAT_SX];			\
  GLfloat vsy = mat[MAT_SY];			\
  GLfloat vsz = mat[MAT_SZ];			\
  const GLfloat snapper = (3L << 18);		\
  GLfloat snap_tx = mat[MAT_TX] + snapper;	\
  GLfloat snap_ty = mat[MAT_TY] + snapper;	\
  GLfloat vtz = mat[MAT_TZ];

#define DO_SETUP_XYZW				\
{						\
  GLfloat oow = 1.0 / data[3];			\
  v->x = data[0]*oow*vsx + snap_tx;		\
  v->y = data[1]*oow*vsy + snap_ty;		\
  v->ooz = data[2]*oow*vsz + vtz;		\
  v->oow = oow;					\
  v->x -= snapper;				\
  v->y -= snapper;				\
}

#endif
#endif

#define COPY_XYZW_STRIDE				\
  { GLfloat *clip = VEC_ELT(VB->ClipPtr, GLfloat, e);	\
    COPY_4FV(out, clip); }

#define VARS_RGBA

#define COPY_RGBA_STRIDE					\
  { GLubyte *color = VEC_ELT(VB->ColorPtr, GLubyte, e);		\
    UBYTE_RGBA_TO_FLOAT_255_RGBA((out+4), color); }

#if FX_USE_PARGB
#define DO_SETUP_RGBA				\
  GET_PARGB(v) = (((int)data[4+3]) << 24) | 	\
  		 (((int)data[4+0]) << 16) | 	\
  		 (((int)data[4+1]) << 8)  |	\
  		 (((int)data[4+2]) << 0);	
#else
#define DO_SETUP_RGBA				\
  v->r = data[4+0];				\
  v->g = data[4+1];				\
  v->b = data[4+2];				\
  v->a = data[4+3];
#endif /* FX_USE_PARGB */

#define VARS_TMU0							\
  struct gl_texture_unit *t0 = &ctx->Texture.Unit[tmu0_source];		\
  GLfloat sScale0 = fxTMGetTexInfo(t0->Current)->sScale;	\
  GLfloat tScale0 = fxTMGetTexInfo(t0->Current)->tScale;	\


#define COPY_TMU0_STRIDE(offset)					\
  { GLfloat *tc0 = VEC_ELT(tc0_vec, GLfloat, e);	\
    COPY_2V((out+offset), tc0); }


#define DO_SETUP_TMU0(offset)				\
  v->tmuvtx[0].sow = data[offset+0]*sScale0*v->oow;	\
  v->tmuvtx[0].tow = data[offset+1]*tScale0*v->oow;

#define VARS_TMU1							\
  struct gl_texture_unit *t1 = &ctx->Texture.Unit[tmu1_source];		\
  GLfloat sScale1 = fxTMGetTexInfo(t1->Current)->sScale;	\
  GLfloat tScale1 = fxTMGetTexInfo(t1->Current)->tScale;

#define COPY_TMU1_STRIDE(offset)					\
  { GLfloat *tc1 = VEC_ELT(tc1_vec, GLfloat, e);	\
    COPY_2V((out+offset), tc1); }


#define DO_SETUP_TMU1(offset)				\
  v->tmuvtx[1].sow = data[offset+0]*sScale1*v->oow;	\
  v->tmuvtx[1].tow = data[offset+1]*tScale1*v->oow;

#define COPY_NIL(offset) ASSIGN_2V((out+offset), 0, 0);

#define IND 0
#define TAG(x) x##_nil
#include "fxcliptmp.h"

#define IND SETUP_RGBA
#define TAG(x) x##_RGBA
#include "fxcliptmp.h"

#define IND SETUP_TMU0
#define TAG(x) x##_TMU0
#include "fxcliptmp.h"

#define IND (SETUP_TMU0|SETUP_TMU1)
#define TAG(x) x##_TMU0_TMU1
#include "fxcliptmp.h"

#define IND (SETUP_RGBA|SETUP_TMU0)
#define TAG(x) x##_RGBA_TMU0
#include "fxcliptmp.h"

#define IND (SETUP_RGBA|SETUP_TMU0|SETUP_TMU1)
#define TAG(x) x##_RGBA_TMU0_TMU1
#include "fxcliptmp.h"

tfxTriViewClipFunc fxTriViewClipTab[0x8];
tfxTriClipFunc fxTriClipStrideTab[0x8];
tfxLineClipFunc fxLineClipTab[0x8];


void fxDDClipInit()
{
   fxTriViewClipTab[0] = fx_tri_view_clip_nil;
   fxTriViewClipTab[SETUP_RGBA] = fx_tri_view_clip_RGBA;
   fxTriViewClipTab[SETUP_TMU0] = fx_tri_view_clip_TMU0;
   fxTriViewClipTab[SETUP_TMU0|SETUP_TMU1] = fx_tri_view_clip_TMU0_TMU1;
   fxTriViewClipTab[SETUP_RGBA|SETUP_TMU0] = fx_tri_view_clip_RGBA_TMU0;
   fxTriViewClipTab[SETUP_RGBA|SETUP_TMU0|SETUP_TMU1] = fx_tri_view_clip_RGBA_TMU0_TMU1;

   fxTriClipStrideTab[0] = fx_tri_clip_stride_nil;
   fxTriClipStrideTab[SETUP_RGBA] = fx_tri_clip_stride_RGBA;
   fxTriClipStrideTab[SETUP_TMU0] = fx_tri_clip_stride_TMU0;
   fxTriClipStrideTab[SETUP_TMU0|SETUP_TMU1] = fx_tri_clip_stride_TMU0_TMU1;
   fxTriClipStrideTab[SETUP_RGBA|SETUP_TMU0] = fx_tri_clip_stride_RGBA_TMU0;
   fxTriClipStrideTab[SETUP_RGBA|SETUP_TMU0|SETUP_TMU1] = fx_tri_clip_stride_RGBA_TMU0_TMU1;

   fxLineClipTab[0] = fx_line_clip_nil;
   fxLineClipTab[SETUP_RGBA] = fx_line_clip_RGBA;
   fxLineClipTab[SETUP_TMU0] = fx_line_clip_TMU0;
   fxLineClipTab[SETUP_TMU0|SETUP_TMU1] = fx_line_clip_TMU0_TMU1;
   fxLineClipTab[SETUP_RGBA|SETUP_TMU0] = fx_line_clip_RGBA_TMU0;
   fxLineClipTab[SETUP_RGBA|SETUP_TMU0|SETUP_TMU1] = fx_line_clip_RGBA_TMU0_TMU1;
}

#else

/*
 * Need this to provide at least one external definition.
 */
int gl_fxclip_dummy(void)
{
  return 0;
}

#endif
