/* $Id: vbrender.c,v 1.20.4.1 2000/11/08 16:41:17 brianp Exp $ */

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
 * Render points, lines, and polygons.  The only entry point to this
 * file is the gl_render_vb() function.  This function is called after
 * the vertex buffer has filled up or a state change has occurred.
 *
 * This file basically only makes calls to the clipping functions and
 * the point, line and triangle rasterizers via the function pointers.
 *    context->Driver.PointsFunc()
 *    context->Driver.LineFunc()
 *    context->Driver.TriangleFunc()
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "clip.h"
#include "context.h"
#include "light.h"
#include "lines.h"
#include "macros.h"
#include "matrix.h"
#include "pb.h"
#include "points.h"
#include "pipeline.h"
#include "stages.h"
#include "types.h"
#include "vb.h"
#include "vbcull.h"
#include "vbrender.h"
#include "vbindirect.h"
#include "xform.h"
#endif


/*
 * This file implements rendering of points, lines and polygons defined by
 * vertices in the vertex buffer.
 */

/*
 * Render a line segment from VB[v1] to VB[v2] when either one or both
 * endpoints must be clipped.
 */
void gl_render_clipped_line( GLcontext *ctx, GLuint v1, GLuint v2 )
{
   GLuint pv = v2;
   struct vertex_buffer *VB = ctx->VB;
   GLubyte mask = (GLubyte) (VB->ClipMask[v1] | VB->ClipMask[v2]);
/*     GLubyte andmask = VB->ClipMask[v1] & VB->ClipMask[v2]; */

   if ((ctx->line_clip_tab[VB->ClipPtr->size])( VB, &v1, &v2, mask ))
      ctx->Driver.LineFunc( ctx, v1, v2, pv );
}

static INLINE void gl_render_clipped_line2( GLcontext *ctx, 
					    GLuint v1, GLuint v2 )
{
   GLuint pv = v2;
   struct vertex_buffer *VB = ctx->VB;
   GLubyte mask = (GLubyte) (VB->ClipMask[v1] | VB->ClipMask[v2]);

   if (!mask || (ctx->line_clip_tab[VB->ClipPtr->size])( VB, &v1, &v2, mask ))
      ctx->Driver.LineFunc( ctx, v1, v2, pv );
}



/*
 * Compute Z offsets for a polygon with plane defined by (A,B,C,D)
 * D is not needed.
 */
static void offset_polygon( GLcontext *ctx, GLfloat a, GLfloat b, GLfloat c )
{
   GLfloat ac, bc, m;
   GLfloat offset = 0.0F;

   if (c*c > 1e-16) {
      ac = a / c;
      bc = b / c;
      if (ac<0.0F)  ac = -ac;
      if (bc<0.0F)  bc = -bc;
      m = MAX2( ac, bc );
      offset = m * ctx->Polygon.OffsetFactor + ctx->Polygon.OffsetUnits;
   }

   ctx->PointZoffset   = ctx->Polygon.OffsetPoint ? offset : 0.0F;
   ctx->LineZoffset    = ctx->Polygon.OffsetLine  ? offset : 0.0F;
   ctx->PolygonZoffset = ctx->Polygon.OffsetFill  ? offset : 0.0F;
}

#define FLUSH_PRIM(prim)			\
do {						\
   if (ctx->PB->primitive != prim) {		\
      gl_reduced_prim_change( ctx, prim );	\
   }						\
} while(0)

#define FLUSH_POLY(prim)					\
do {								\
   if ((ctx->IndirectTriangles & DD_TRI_UNFILLED) == 0) {	\
      FLUSH_PRIM(prim);						\
   }								\
} while(0)


/*
 * When glPolygonMode() is used to specify that the front/back rendering
 * mode for polygons is not GL_FILL we end up calling this function.
 */
static void unfilled_polygon( GLcontext *ctx,
                              GLuint n, GLuint vlist[],
                              GLuint pv, GLuint facing )
{
   GLenum mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;
   struct vertex_buffer *VB = ctx->VB;
   GLubyte *edge_ptr = (GLubyte *)VB->EdgeFlagPtr->data;

   FLUSH_PRIM(mode);

   if (mode==GL_POINT) {
      GLuint i, j;
      for (i=0;i<n;i++) {
         j = vlist[i];
         if (edge_ptr[j] & 0x3) {
	    edge_ptr[j] &= ~0x3;
            (*ctx->Driver.PointsFunc)( ctx, j, j+1 );
         }
      }
   }
   else if (mode==GL_LINE) {
      GLuint i, j0, j1;

      /* draw the edges */
      for (i=0;i<n-1;i++) {
	 j0 = vlist[i];
	 j1 = vlist[i+1];
	
	 if (edge_ptr[j0] & 0x1) {
	    edge_ptr[j0] &= ~0x1;
	    (*ctx->Driver.LineFunc)( ctx, j0, j1, pv );
	 }
      }
      
      /* last edge is special */
      j0 = vlist[i];
      j1 = vlist[0];

      if (edge_ptr[j0] & 0x2) {
	 edge_ptr[j0] &= ~0x2;
	 (*ctx->Driver.LineFunc)( ctx, j0, j1, pv );
      }
   }
   else {
      /* Fill the polygon */
      GLuint j0, i;
      j0 = vlist[0];
      for (i=2;i<n;i++) {
         (*ctx->Driver.TriangleFunc)( ctx, j0, vlist[i-1], vlist[i], pv );
      }
   }
}




/*
 * Render a polygon in which at least one vertex has to be clipped.
 * Input:  n - number of vertices
 *         vlist - list of vertices in the polygon.
 *                 CCW order = front facing.
 */
void gl_render_clipped_triangle( GLcontext *ctx, GLuint n, GLuint vlist[], 
				 GLuint pv )
{
   struct vertex_buffer *VB = ctx->VB;
   GLubyte mask = 0;
   GLuint i;

   for (i = 0 ; i < n ; i++) 
      mask |= VB->ClipMask[vlist[i]];
   
   n = (ctx->poly_clip_tab[VB->ClipPtr->size])( VB, n, vlist, mask );

   for (i=2;i<n;i++) 
      ctx->TriangleFunc( ctx, *vlist, vlist[i-1], vlist[i], pv ); 
}


static INLINE void gl_render_clipped_triangle2( GLcontext *ctx, 
						GLuint v1, GLuint v2, GLuint v3,
						GLuint pv )
{
   struct vertex_buffer *VB = ctx->VB;
   GLubyte mask = (GLubyte) (VB->ClipMask[v1] | 
			     VB->ClipMask[v2] | 
			     VB->ClipMask[v3]);
   GLuint vlist[VB_MAX_CLIPPED_VERTS];
   GLuint i, n;

   if (!mask) {
      ctx->TriangleFunc( ctx, v1, v2, v3, pv ); 
      return;
   }

   if (CLIP_ALL_BITS & VB->ClipMask[v1] & VB->ClipMask[v2] & VB->ClipMask[v3]) 
      return;

   ASSIGN_3V(vlist, v1, v2, v3 );
   n = (ctx->poly_clip_tab[VB->ClipPtr->size])( VB, 3, vlist, mask );

   for (i=2;i<n;i++) 
      ctx->TriangleFunc( ctx, *vlist, vlist[i-1], vlist[i], pv ); 
}


static INLINE void gl_render_clipped_quad2( GLcontext *ctx, 
					    GLuint v1, GLuint v2, GLuint v3,
					    GLuint v4,
					    GLuint pv )
{
   struct vertex_buffer *VB = ctx->VB;
   GLubyte mask = (GLubyte) (VB->ClipMask[v1] | 
			     VB->ClipMask[v2] | 
			     VB->ClipMask[v3] |
			     VB->ClipMask[v4]);
   GLuint vlist[VB_MAX_CLIPPED_VERTS];
   GLuint i, n;

   if (!mask) {
      ctx->QuadFunc( ctx, v1, v2, v3, v4, pv ); 
      return;
   }

   if (CLIP_ALL_BITS & VB->ClipMask[v1] & 
       VB->ClipMask[v2] & VB->ClipMask[v3] &
       VB->ClipMask[v4]) 
      return;

   ASSIGN_4V(vlist, v1, v2, v3, v4 );
   n = (ctx->poly_clip_tab[VB->ClipPtr->size])( VB, 4, vlist, mask );

   for (i=2;i<n;i++) 
      ctx->TriangleFunc( ctx, *vlist, vlist[i-1], vlist[i], pv ); 
}


/* Implements triangle_rendering when (IndirectTriangles & DD_SW_SETUP)
 * is non-zero.
 */
static void render_triangle( GLcontext *ctx,
                             GLuint v0, GLuint v1, GLuint v2, GLuint pv )
{
   struct vertex_buffer *VB = ctx->VB;
   GLfloat (*win)[4] = VB->Win.data;
   GLfloat ex = win[v1][0] - win[v0][0];
   GLfloat ey = win[v1][1] - win[v0][1];
   GLfloat fx = win[v2][0] - win[v0][0];
   GLfloat fy = win[v2][1] - win[v0][1];
   GLfloat c = ex*fy-ey*fx;
   GLuint facing;
   GLuint tricaps;

   if (c * ctx->backface_sign > 0)
      return;

   facing = (c<0.0F) ^ (ctx->Polygon.FrontFace==GL_CW);
   tricaps = ctx->TriangleCaps;
   
   if (tricaps & DD_TRI_OFFSET) {
      /* finish computing plane equation of polygon, compute offset */
      GLfloat fz = win[v2][2] - win[v0][2];
      GLfloat ez = win[v1][2] - win[v0][2];
      GLfloat a = ey*fz-ez*fy;
      GLfloat b = ez*fx-ex*fz;
      offset_polygon( ctx, a, b, c );
   }

   if (tricaps & DD_TRI_LIGHT_TWOSIDE) {
      VB->Specular = VB->Spec[facing];
      VB->ColorPtr = VB->Color[facing];
      VB->IndexPtr = VB->Index[facing];
   }

   if (tricaps & DD_TRI_UNFILLED) {
      GLuint vlist[3];
      vlist[0] = v0;
      vlist[1] = v1;
      vlist[2] = v2;
      unfilled_polygon( ctx, 3, vlist, pv, facing );
   }
   else {
      (*ctx->Driver.TriangleFunc)( ctx, v0, v1, v2, pv );
   }

   if (tricaps & DD_TRI_OFFSET) {
      /* reset Z offsets now */
      ctx->PointZoffset   = 0.0;
      ctx->LineZoffset    = 0.0;
      ctx->PolygonZoffset = 0.0;
   }
}


static void null_triangle( GLcontext *ctx,
			   GLuint v0, GLuint v1, GLuint v2, GLuint pv )
{
}


/* Implements triangle_rendering when (IndirectTriangles & DD_SW_SETUP)
 * is non-zero.
 */
static void render_quad( GLcontext *ctx, GLuint v0, GLuint v1,
                         GLuint v2, GLuint v3, GLuint pv )
{
   struct vertex_buffer *VB = ctx->VB;
   GLfloat (*win)[4] = VB->Win.data;
   GLfloat ex = win[v2][0] - win[v0][0];
   GLfloat ey = win[v2][1] - win[v0][1];
   GLfloat fx = win[v3][0] - win[v1][0];
   GLfloat fy = win[v3][1] - win[v1][1];
   GLfloat c = ex*fy-ey*fx;
   GLuint facing;
   GLuint tricaps = ctx->TriangleCaps;

   if (c * ctx->backface_sign > 0)
      return;

   facing = (c<0.0F) ^ (ctx->Polygon.FrontFace==GL_CW);

   if (tricaps & DD_TRI_OFFSET) {
      GLfloat ez = win[v2][2] - win[v0][2];
      GLfloat fz = win[v3][2] - win[v1][2];
      GLfloat a = ey*fz-ez*fy;
      GLfloat b = ez*fx-ex*fz;
      offset_polygon( ctx, a, b, c );
   }


   if (tricaps & DD_TRI_LIGHT_TWOSIDE) {
      VB->Specular = VB->Spec[facing];
      VB->ColorPtr = VB->Color[facing];
      VB->IndexPtr = VB->Index[facing];
   }


   /* Render the quad! */
   if (tricaps & DD_TRI_UNFILLED) {
      GLuint vlist[4];
      vlist[0] = v0;
      vlist[1] = v1;
      vlist[2] = v2;
      vlist[3] = v3;
      unfilled_polygon( ctx, 4, vlist, pv, facing );
   }
   else {
      (*ctx->Driver.QuadFunc)( ctx, v0, v1, v2, v3, pv );
   }

   if (tricaps & DD_TRI_OFFSET) {
      /* reset Z offsets now */
      ctx->PointZoffset   = 0.0;
      ctx->LineZoffset    = 0.0;
      ctx->PolygonZoffset = 0.0;
   }
}


#if 0
static void null_quad( GLcontext *ctx, GLuint v0, GLuint v1,
		       GLuint v2, GLuint v3, GLuint pv )
{
}
#endif


/*
 * Either the vertex buffer is full (VB->Count==VB_MAX) or glEnd() has been
 * called.  Render the primitives defined by the vertices and reset the
 * buffer.
 *
 * This function won't be called if the device driver implements a
 * RenderVB() function.
 */

#define NEED_EDGEFLAG_SETUP (ctx->TriangleCaps & DD_TRI_UNFILLED)

#define EDGEFLAG_TRI( i2, i1, i, pv, parity)	\
do {						\
  eflag[i2] = eflag[i1] = 1; eflag[i] = 2;	\
} while (0)

#define EDGEFLAG_QUAD( i3, i2, i1, i, pv)		\
do {							\
  eflag[i3] = eflag[i2] = eflag[i1] = 1; eflag[i] = 2;	\
} while (0)


#define EDGEFLAG_POLY_TRI_PRE( i2, i1, i, pv)	\
do {						\
  eflag[i2] |= (eflag[i2] >> 2) & 1; 		\
  eflag[i1] |= (eflag[i1] >> 2) & 1; 		\
  eflag[i] |= (eflag[i] >> 2) & 2;		\
} while (0)

#define EDGEFLAG_POLY_TRI_POST( i2, i1, i, pv)	\
do {						\
  eflag[i2] &= ~(4|1); 		\
  eflag[i1] &= ~(4|1); 		\
  eflag[i] &= ~(8|2); 		\
} while (0)



/* Culled and possibly clipped primitives.
 */
#define RENDER_POINTS( start, count )			\
   (void) cullmask;					\
   (*ctx->Driver.PointsFunc)( ctx, start, count );


#define RENDER_LINE( i1, i )			\
do {						\
   const GLubyte flags = cullmask[i];		\
						\
   if (!(flags & PRIM_NOT_CULLED))		\
      continue;					\
						\
   if (flags & PRIM_ANY_CLIP)			\
      gl_render_clipped_line( ctx, i1, i );	\
   else						\
      ctx->Driver.LineFunc( ctx, i1, i, i );	\
} while (0)



#define RENDER_TRI( i2, i1, i, pv, parity)		\
do {							\
   const GLubyte flags = cullmask[i];			\
							\
   if (!(flags & PRIM_NOT_CULLED))			\
      continue;						\
							\
   if (flags & PRIM_ANY_CLIP) {				\
      if (parity) {					\
	 vlist[0] = i1;					\
	 vlist[1] = i2;					\
	 vlist[2] = i;					\
      } else {						\
	 vlist[0] = i2;					\
	 vlist[1] = i1;					\
	 vlist[2] = i;					\
      }							\
      gl_render_clipped_triangle( ctx, 3, vlist, pv );	\
   } else if (parity) 					\
      ctx->TriangleFunc( ctx, i1, i2, i, pv );		\
   else							\
      ctx->TriangleFunc( ctx, i2, i1, i, pv );		\
							\
} while (0)


#define RENDER_QUAD( i3, i2, i1, i, pv)			\
do {							\
   const GLubyte flags = cullmask[i];			\
							\
   if (!(flags & PRIM_NOT_CULLED))			\
      continue;						\
							\
   if (flags&PRIM_ANY_CLIP) {				\
      vlist[0] = i3;					\
      vlist[1] = i2;					\
      vlist[2] = i1;					\
      vlist[3] = i;					\
      gl_render_clipped_triangle( ctx, 4, vlist, pv );	\
   } else						\
      ctx->QuadFunc( ctx, i3, i2, i1, i, pv );		\
} while (0)


#define LOCAL_VARS				\
    GLcontext *ctx = VB->ctx;			\
    const GLubyte *cullmask = VB->CullMask;	\
    GLuint vlist[VB_SIZE];			\
    GLubyte *eflag = VB->EdgeFlagPtr->data;    	\
    GLuint *stipplecounter = &VB->ctx->StippleCounter; \
    (void) vlist; (void) eflag; (void) stipplecounter;

#define TAG(x) x##_cull
#define INIT(x)  if (x == GL_POLYGON) FLUSH_POLY(x); else FLUSH_PRIM(x)
#define RESET_STIPPLE *stipplecounter = 0
#include "render_tmp.h"







/* Direct, no clipping or culling.
 */
#define RENDER_POINTS( start, count ) \
   (*ctx->Driver.PointsFunc)( ctx, start, count )

#define RENDER_LINE( i1, i ) \
   (*ctx->Driver.LineFunc)( ctx, i1, i, i )

#define RENDER_TRI( i2, i1, i, pv, parity )	\
do {						\
   if (parity)					\
      ctx->TriangleFunc( ctx, i1, i2, i, pv );	\
   else						\
      ctx->TriangleFunc( ctx, i2, i1, i, pv );	\
} while (0)


#define RENDER_QUAD( i3, i2, i1, i, pv )	\
   ctx->QuadFunc( ctx, i3, i2, i1, i, i );

#define TAG(x) x##_raw

#define LOCAL_VARS  				\
    GLcontext *ctx = VB->ctx;			\
    GLubyte *eflag = VB->EdgeFlagPtr->data;	\
    GLuint *stipplecounter = &VB->ctx->StippleCounter; \
    (void) eflag; (void) stipplecounter;

#define INIT(x)  if (x == GL_POLYGON) FLUSH_POLY(x); else FLUSH_PRIM(x)
#define RESET_STIPPLE *stipplecounter = 0
#include "render_tmp.h"




/* Direct, with the possibility of clipping.
 */ 
#define RENDER_POINTS( start, count )			\
   (*ctx->Driver.PointsFunc)( ctx, start, count )

#define RENDER_LINE( i1, i )			\
   gl_render_clipped_line2( ctx, i1, i )

#define RENDER_TRI( i2, i1, i, pv, parity)	\
do {						\
  GLuint e2=i2, e1=i1;				\
  if (parity) { GLuint t=e2; e2=e1; e1=t; }	\
  gl_render_clipped_triangle2(ctx,e2,e1,i,pv);	\
} while (0)

#define RENDER_QUAD( i3, i2, i1, i, pv)		\
do {						\
  gl_render_clipped_quad2(ctx,i3,i2,i1,i,pv);	\
} while (0)


#define LOCAL_VARS  				\
    GLcontext *ctx = VB->ctx;			\
    GLubyte *eflag = VB->EdgeFlagPtr->data;	\
    GLuint *stipplecounter = &VB->ctx->StippleCounter; \
    (void) eflag; (void) stipplecounter;

#define INIT(x)  if (x == GL_POLYGON) FLUSH_POLY(x); else FLUSH_PRIM(x)
#define TAG(x) x##_clipped
#define RESET_STIPPLE *stipplecounter = 0

#include "render_tmp.h"

/* Bits:
 *    0x1 - draw this edge if it is first or second in a triangle.
 *    0x2 - draw this edge if it is last in a triangle.
 *    0x4 - placeholder for multipass rendering.
 *
 * Bits 0x1 and 0x2 are cleared after they are used.  Bit 0x4 is used
 * to stop these values going to zero in multipass rendering. 
 *
 * Clipping introduces vertices on outgoing edges with edgeflag 0x1.
 * Incoming edges retain the value of the clipped vertex, with the following 
 * masks:
 *    - If the incoming edge originates from the last vertex in the
 *      clipped primitive (triangle or quad), the introduced vertex 
 *      retains both bits (0x3) of the original flag.
 *    - All other introduced vertices retain only bit 1 (0x1).
 *
 * In a horrible hack I've had to push tristrips, fans & quadstrip handling 
 * into render_tmp.h...
 *
 * Keith. 
 */
static void 
setup_edgeflag( struct vertex_buffer *VB, 
                GLenum prim, 
                GLuint start, 
                GLuint count,
                GLuint parity )
{
   GLubyte *flag = VB->EdgeFlagPtr->data + start;
   GLuint n = count - start;
   GLuint i;
   (void) parity;

   switch (prim) {
   case GL_TRIANGLES:
      for (i = 0 ; i < n-2 ; i+=3) {
	 if (flag[i])   flag[i]   = 0x1;
	 if (flag[i+1]) flag[i+1] = 0x1;
	 if (flag[i+2]) flag[i+2] = 0x3;
      }
      break;
   case GL_QUADS:
      for (i = 0 ; i < n-3 ; i+=4) {
	 if (flag[i])   flag[i]   = 0x1;
	 if (flag[i+1]) flag[i+1] = 0x1;
	 if (flag[i+2]) flag[i+2] = 0x1;
	 if (flag[i+3]) flag[i+3] = 0x3;
      }
      break;
   case GL_POLYGON:
      if (flag[0]) flag[0] = 0x1;
      for (i = 1 ; i < n-1 ; i++) {
	 if (flag[i]) flag[i] = 0x4;
      }
      if (flag[i]) flag[i] = 0x8;
      break;
   default:
      break;
   }
}



/* Could eventually generalize to handle changes of rasterization
 * state other than change-of-primitive.  An example might be 
 * some bind-texture calls.
 */
void gl_render_vb( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLuint i, next, prim;
   GLuint parity = VB->Parity;
   render_func *tab;
   GLuint count = VB->Count;
   GLint p = 0;

   if (VB->Indirect) { 
/*        gl_render_vb_indirect( VB, VB );   */
      return; 
   } else if (VB->CullMode & CULL_MASK_ACTIVE) {
      tab = ctx->Driver.RenderVBCulledTab;
   } else if (VB->CullMode & CLIP_MASK_ACTIVE) {
      tab = ctx->Driver.RenderVBClippedTab;
   } else {
      tab = ctx->Driver.RenderVBRawTab;
   }

   if (!VB->CullDone)
      gl_fast_copy_vb( VB );

   if (ctx->TriangleCaps & DD_TRI_UNFILLED)
      gl_import_client_data( VB, VERT_EDGE, VEC_WRITABLE|VEC_GOOD_STRIDE );

   gl_import_client_data( VB, ctx->RenderFlags,
			  (VB->ClipOrMask
			   ? VEC_WRITABLE|VEC_GOOD_STRIDE
			   : VEC_GOOD_STRIDE));

   if (/*  ctx->Current.Primitive == GL_POLYGON+1 &&  */
      ctx->Driver.RenderStart)
      ctx->Driver.RenderStart( ctx );

   do
   {      
      for ( i= VB->CopyStart ; i < count ; parity = 0, i = next ) 
      {
	 prim = VB->Primitive[i];
	 next = VB->NextPrimitive[i];

	 if (ctx->TriangleCaps & DD_TRI_UNFILLED) 
	    setup_edgeflag(VB, prim, i, next, parity);

	 tab[prim]( VB, i, next, parity );

	 if (ctx->TriangleCaps & DD_TRI_LIGHT_TWOSIDE) {
	    VB->Specular = VB->Spec[0];
	    VB->ColorPtr = VB->Color[0];
	    VB->IndexPtr = VB->Index[0];
	 }
      }

   } while (ctx->Driver.MultipassFunc &&
	    ctx->Driver.MultipassFunc( VB, ++p ));

   if (ctx->PB->count > 0) 
      gl_flush_pb(ctx);

   if (/*  ctx->Current.Primitive == GL_POLYGON+1 &&  */
      ctx->Driver.RenderFinish)
      ctx->Driver.RenderFinish( ctx );
}


void gl_reduced_prim_change( GLcontext *ctx, GLenum prim )
{
   if (ctx->PB->count > 0) 
      gl_flush_pb(ctx);	

   if (ctx->PB->primitive != prim) {
      ctx->PB->primitive = prim;			

      if (ctx->Driver.ReducedPrimitiveChange)
	 ctx->Driver.ReducedPrimitiveChange( ctx, prim );
   }
}


void gl_set_render_vb_function( GLcontext *ctx )
{
   if (ctx->Driver.RenderVBCulledTab == 0)
      ctx->Driver.RenderVBCulledTab = render_tab_cull;

   if (ctx->Driver.RenderVBClippedTab == 0)
      ctx->Driver.RenderVBClippedTab = render_tab_clipped;

   if (ctx->Driver.RenderVBRawTab == 0)
      ctx->Driver.RenderVBRawTab = render_tab_raw;

   ctx->TriangleFunc = ctx->Driver.TriangleFunc;
   ctx->QuadFunc = ctx->Driver.QuadFunc;
   ctx->ClippedTriangleFunc = ctx->TriangleFunc;

   if (ctx->IndirectTriangles & DD_SW_SETUP) {

      ctx->ClippedTriangleFunc = render_triangle;

      if (ctx->IndirectTriangles & (DD_SW_SETUP & ~DD_TRI_CULL)) {
	 if (ctx->IndirectTriangles & DD_TRI_CULL_FRONT_BACK) {
	    ctx->TriangleFunc = null_triangle;
	    ctx->QuadFunc = render_quad;
	    ctx->ClippedTriangleFunc = null_triangle;
	 } else {
	    ctx->TriangleFunc = render_triangle;
	    ctx->QuadFunc = render_quad;
	 }
      }
   }
}

void gl_init_vbrender( void )
{
   render_init_raw();
   render_init_cull();
   render_init_clipped();
}




