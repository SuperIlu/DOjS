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


#define V1 VARS_XYZW
#define S1 DO_SETUP_XYZW
#define T1 COPY_XYZW_STRIDE
#define Z1 4

#if (IND & SETUP_RGBA)
#define V2 V1 VARS_RGBA
#define S2 S1 DO_SETUP_RGBA
#define T2 T1 COPY_RGBA_STRIDE
#define Z2 (Z1 + 4)
#else
#define V2 V1
#define S2 S1
#define T2 T1
#define Z2 Z1
#endif

#if (IND & SETUP_TMU0)
#define V3 V2 VARS_TMU0
#define S3 S2 DO_SETUP_TMU0(Z2)
#define T3 T2 COPY_TMU0_STRIDE(Z2)
#define Z3 (Z2 + 2)
#else
#define V3 V2
#define S3 S2
#define T3 T2
#define Z3 Z2
#endif

#if (IND & SETUP_TMU1)
#define V4 V3 VARS_TMU1
#define S4 S3 DO_SETUP_TMU1(Z3)
#define T4 T3 COPY_TMU1_STRIDE(Z3)
#define Z4 (Z3 + 2)
#else
#define V4 V3
#define S4 S3
#define T4 T3
#define Z4 Z3
#endif

#if (Z4 & 2)
#define SIZE (Z4+2)
#define COPY_STRIDE T4 COPY_NIL(Z4)
#else
#define SIZE Z4
#define COPY_STRIDE T4
#endif

#define VARS V4
#define SETUP S4

#define DRAW_LINE(tmp0, tmp1, width)	\
  do {					\
    GrVertex verts[4];			\
    float dx, dy, ix, iy;		\
					\
    dx = tmp0->x - tmp1->x;		\
    dy = tmp0->y - tmp1->y;		\
					\
    if (dx * dx > dy * dy) {		\
      iy = width;			\
      ix = 0;				\
    } else {				\
      iy = 0;				\
      ix = width;			\
    }					\
					\
   verts[0] = *tmp0;			\
   verts[1] = *tmp0;			\
   verts[2] = *tmp1;			\
   verts[3] = *tmp1;			\
					\
   verts[0].x = tmp0->x - ix;		\
   verts[0].y = tmp0->y - iy;		\
					\
   verts[1].x = tmp0->x + ix;		\
   verts[1].y = tmp0->y + iy;		\
					\
   verts[2].x = tmp1->x + ix;		\
   verts[2].y = tmp1->y + iy;		\
					\
   verts[3].x = tmp1->x - ix;		\
   verts[3].y = tmp1->y - iy;		\
	 				\
   FX_grDrawPolygonVertexList(4, verts); \
  } while (0)

static void TAG(fx_tri_view_clip)( struct vertex_buffer *VB, 
				   GLuint v[],
				   GLubyte mask )
{
   GLcontext *ctx = VB->ctx;
   fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
   GLfloat data[VB_MAX_CLIPPED_VERTS*12];
   GLfloat *vlist[VB_MAX_CLIPPED_VERTS];
   GrVertex *verts[VB_MAX_CLIPPED_VERTS];
   fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;
   GLfloat *out = data;
   GLfloat *mat = ctx->Viewport.WindowMap.m;
   GLuint i, n;
   GLubyte *clipmask = VB->ClipMask;

   GLuint tmu0_source = fxMesa->tmu_source[0];				
   GLuint tmu1_source = fxMesa->tmu_source[1];				
   GLvector4f *tc0_vec = VB->TexCoordPtr[tmu0_source];
   GLvector4f *tc1_vec = VB->TexCoordPtr[tmu1_source];

   (void) fxMesa;
   (void) tmu0_source; (void) tc0_vec;
   (void) tmu1_source; (void) tc1_vec;

   for (i = 0 ; i < 3 ; i++) {
      GLuint e = v[i];
      verts[i] = 0;
      if (!clipmask[e]) verts[i] = (GrVertex *)gWin[e].f;	 
      vlist[i] = out; 
      COPY_STRIDE;
      out += SIZE;
   }

   if ((n = fx_view_clip_triangle( ctx, vlist, verts, SIZE, mask )) >= 3)
   {
      GrVertex tmp[VB_MAX_CLIPPED_VERTS];
      GrVertex *v = tmp, *v2, *v3;
      VARS;
   
      for (i = 0 ; i < n ; i++)
	 if (!verts[i]) {
	    GLfloat *data = vlist[i];
	    SETUP;
	    verts[i] = v++;
	 }
	  
      v = verts[0];
      v2 = verts[1];
      v3 = verts[2];

      for (i = 2 ; i < n ; v2 = v3, v3=verts[++i])
	 FX_grDrawTriangle(v, v2, v3);
   }
}





static void TAG(fx_tri_clip_stride)( struct vertex_buffer *VB, 
				     GLuint v[],
				     GLuint mask )
{
   GLcontext *ctx = VB->ctx;
   fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
   GLfloat data[VB_MAX_CLIPPED_VERTS*12];
   GLfloat *vlist[VB_MAX_CLIPPED_VERTS];
   GrVertex *verts[VB_MAX_CLIPPED_VERTS];
   fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;
   GLfloat *out = data;
   GLfloat *mat = ctx->Viewport.WindowMap.m;
   GLuint i, n;
   GLubyte *clipmask = VB->ClipMask;

   GLuint tmu0_source = fxMesa->tmu_source[0];				
   GLuint tmu1_source = fxMesa->tmu_source[1];				
   GLvector4f *tc0_vec = VB->TexCoordPtr[tmu0_source];
   GLvector4f *tc1_vec = VB->TexCoordPtr[tmu1_source];

   (void) fxMesa;
   (void) tmu0_source; (void) tc0_vec;
   (void) tmu1_source; (void) tc1_vec;

   for (i = 0 ; i < 3 ; i++) {
      GLuint e = v[i];
      verts[i] = 0;
      if (!clipmask[e]) verts[i] = (GrVertex *)gWin[e].f;	 
      vlist[i] = out; 
      COPY_STRIDE;
      out += SIZE;
   }

   if (VB->ClipPtr->size < 4) {
      vlist[0][3] = vlist[1][3] = vlist[2][3] = 1.0;
      if (VB->ClipPtr->size == 2) 
	 vlist[0][2] = vlist[1][2] = vlist[2][2] = 0.0;
   }
      
   if ((n = fx_clip_triangle( ctx, vlist, verts, SIZE, mask )) >= 3)
   {
      GrVertex tmp[VB_MAX_CLIPPED_VERTS];
      GrVertex *v = tmp, *v2, *v3;
      VARS;
   
      for (i = 0 ; i < n ; i++)
	 if (!verts[i]) {
	    GLfloat *data = vlist[i];
	    SETUP;
	    verts[i] = v++;
	 }
	  
      v = verts[0];
      v2 = verts[1];
      v3 = verts[2];

      for (i = 2 ; i < n ; v2 = v3, v3=verts[++i])
	 FX_grDrawTriangle(v, v2, v3);
   }
}



static void TAG(fx_line_clip)( struct vertex_buffer *VB, 
			       GLuint v1, GLuint v2,
			       GLubyte mask )
{
   GLcontext *ctx = VB->ctx;
   fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
   GLfloat data[VB_MAX_CLIPPED_VERTS*12];
   GLfloat *vlist[VB_MAX_CLIPPED_VERTS];
   GLfloat *out = data;
   GLfloat *mat = ctx->Viewport.WindowMap.m;
   GLfloat w = ctx->Line.Width*.5;
   GLuint  e, n;

   GLuint tmu0_source = fxMesa->tmu_source[0];				
   GLuint tmu1_source = fxMesa->tmu_source[1];				
   GLvector4f *tc0_vec = VB->TexCoordPtr[tmu0_source];
   GLvector4f *tc1_vec = VB->TexCoordPtr[tmu1_source];

   VARS;

   (void) fxMesa;
   (void) tmu0_source; (void) tc0_vec;
   (void) tmu1_source; (void) tc1_vec;

   vlist[0] = out; 
   e = v1;
   COPY_STRIDE;
   out += SIZE;

   vlist[1] = out; 
   e = v2;
   COPY_STRIDE;
   out += SIZE;

   if (VB->ClipPtr->size < 4) {
      vlist[0][3] = vlist[1][3] = 1.0;
      if (VB->ClipPtr->size == 2) 
	 vlist[0][2] = vlist[1][2] = 0.0;
   }
      
   if ((n = fx_clip_line( ctx, vlist, SIZE, mask )) != 0)
   {
      GrVertex gWin[2];
      GrVertex *v;
      GLfloat *data;

      v = gWin;
      data = vlist[0];
      SETUP;

      v++;
      data = vlist[1];
      SETUP;

      DRAW_LINE(gWin, v, w);      
   }
}



#undef V1 
#undef S1 
#undef C1 
#undef Z1 
#undef T1

#undef V2 
#undef S2 
#undef C2 
#undef Z2 
#undef T2

#undef V3 
#undef S3 
#undef C3 
#undef Z3 
#undef T3

#undef V4 
#undef S4 
#undef C4 
#undef Z4 
#undef T4

#undef VARS 
#undef SETUP 
#undef COPY 
#undef COPY_STRIDE
#undef SIZE
#undef IND
#undef TAG
