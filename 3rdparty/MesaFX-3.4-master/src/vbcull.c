/* $Id: vbcull.c,v 1.11.4.1 2000/11/05 21:24:01 brianp Exp $ */

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

/*
 * New (3.1) transformation code written by Keith Whitwell.
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "context.h"
#include "macros.h"
#include "mem.h"
#include "types.h"
#include "vb.h"
#include "vbcull.h"
#include "vbrender.h"
#include "xform.h"
#endif


/* Culling for vertices and primitives.
 * (C) Keith Whitwell, December 1998.
 *
 * These functions build bitmasks which can be used to shortcircuit
 * lighting, fogging and texture calculations, and to convert
 * two-sided lighting into one-sided on a per-vertex basis.
 *
 * Each of these functions returns a value 'cullcount'.  The meaning
 * of this value just has to satisfy the requirements:
 *
 *    cullcount == VB->Count implies the entire vb has been culled and
 *    can be discarded.
 * 
 *    cullcount == 0 implies that every vertex in the buffer is required
 *    and that the cullmask can be ignored for transformation 
 *    and lighting.
 * 
 */

#ifdef MESA_DEBUG
const char *gl_prim_name[GL_POLYGON+2] = {
   "GL_POINTS",
   "GL_LINES",
   "GL_LINE_LOOP",
   "GL_LINE_STRIP",
   "GL_TRIANGLES",
   "GL_TRIANGLE_STRIP",
   "GL_TRIANGLE_FAN",
   "GL_QUADS",
   "GL_QUAD_STRIP",
   "GL_POLYGON",
   "culled primitive"
};
#endif

static GLuint gl_cull_points( struct vertex_buffer *VB,
			      GLuint start, 
			      GLuint count,
			      GLuint parity,
			      CONST GLfloat (*proj)[4])
{
   const GLubyte *clipmask = VB->ClipMask;
   GLubyte *cullmask = VB->CullMask;
   GLuint i, cullcount = 0;

   (void) parity;
   (void) proj;

   /* This is pretty pointless.  (arf arf)
    */
   for (i = start; i < count; i++) {
      if (clipmask[i] == 0) 
	 cullmask[i] = VERT_FACE_FRONT | PRIM_FACE_FRONT;
      else {
	 cullcount++;
      }
   }
   return cullcount;
}


/* Covers all the non-polygonal primitives when only area culling is
 * activated.
 */
static GLuint gl_cull_points_neither( struct vertex_buffer *VB,
				      GLuint start, 
				      GLuint count,
				      GLuint parity,
				      CONST GLfloat (*proj)[4])
{
   GLubyte unculled_prim = VERT_FACE_FRONT|PRIM_FACE_FRONT;
   (void) parity;
   (void) proj;
   MEMSET(VB->CullMask+start, unculled_prim, (count-start)*sizeof(GLubyte));
   return 0;
}



#define CULL_LINE( i1, i, nr )						 \
do {									 \
   GLubyte ClipOr = VB->ClipMask[i1] | VB->ClipMask[i];			 \
									 \
   if (ClipOr) {							 \
      if (VB->ClipMask[i1] & VB->ClipMask[i] & CLIP_ALL_BITS) {		 \
	 cullcount+=nr;							 \
      }									 \
      else {								 \
	 cullmask[i1] |= VERT_FACE_FRONT;				 \
	 cullmask[i] |= VERT_FACE_FRONT | PRIM_FACE_FRONT | PRIM_CLIPPED; \
      }									 \
   } else {								 \
      cullmask[i1] |= VERT_FACE_FRONT;					 \
      cullmask[i] |= VERT_FACE_FRONT | PRIM_FACE_FRONT;			 \
   }									 \
} while (0)



static GLuint gl_cull_lines( struct vertex_buffer *VB,
			     GLuint start, 
			     GLuint count, 
			     GLuint parity, 
			     CONST GLfloat (*proj)[4])
{
   GLubyte *cullmask = VB->CullMask;
   GLuint cullcount = 0;
   GLuint i;
   GLuint last = count - 1;

   (void) parity;
   (void) proj;

   for (i = start; i < last ; i += 2) {
      CULL_LINE(i, i+1, 2);
   }

   if (i != last)
      cullcount++;

   return cullcount;
}


static GLuint gl_cull_line_strip( struct vertex_buffer *VB,
				  GLuint start, 
				  GLuint count, 
				  GLuint parity, 
				  CONST GLfloat (*proj)[4])
{
   GLubyte *cullmask = VB->CullMask;
   GLuint cullcount = 0;
   GLuint i;
   GLuint last = count - 1;
   GLuint nr = 2;

   (void) parity;
   (void) proj;
   
   for (i = start; i < last ; i++, nr = 1) {
      CULL_LINE(i, i+1, nr);
   }

   if (i != last)
      cullcount++;

   return cullcount;
}


static GLuint gl_cull_line_loop( struct vertex_buffer *VB,
				 GLuint start, 
				 GLuint count, 
				 GLuint parity, 
				 CONST GLfloat (*proj)[4])
{
   GLuint cullcount = 0;
   GLubyte *cullmask = VB->CullMask;
   GLuint i, last = count - 1;
   (void) parity;
   (void) proj;
   if (count - start < 2) 
      return count - start;

   for (i = start; i < last ; i++) {
      CULL_LINE(i, i+1, 1);
   }

   CULL_LINE(last, start, 1);

   return cullcount;
}


#define CULL_TRI( do_clip, do_area, i2, i1, i0, parity, nr )		\
do {									\
   GLubyte ClipOr = 0;							\
   if (do_clip) {							\
      ClipOr = (VB->ClipMask[i2] |					\
		VB->ClipMask[i1] |					\
		VB->ClipMask[i0]);					\
   }									\
									\
   if (do_clip && (ClipOr&CLIP_ALL_BITS))				\
   {									\
      if (VB->ClipMask[i2] & VB->ClipMask[i1] & VB->ClipMask[i0] &	\
	  CLIP_ALL_BITS)						\
      {									\
	 cullcount+=nr;							\
      }									\
      else								\
      {									\
	 cullmask[i0]   = cull_faces | PRIM_CLIPPED;			\
	 cullmask[i1] |= cull_faces;					\
	 cullmask[i2] |= cull_faces;					\
      }									\
   }									\
   else									\
   {									\
      GLfloat area;							\
      GLubyte face_flags;						\
									\
      if (do_area) {							\
	 area = ((proj[i2][0] - proj[i0][0]) *				\
		 (proj[i1][1] - proj[i0][1]) -				\
		 (proj[i2][1] - proj[i0][1]) *				\
		 (proj[i1][0] - proj[i0][0]));				\
									\
	 face_flags = (((area<0.0F) ^ parity) + 1) & cull_faces;	\
      } else {								\
	 face_flags = cull_faces;					\
      }									\
									\
      if (!do_area || face_flags)					\
      {									\
	 cullmask[i0]  = face_flags | (face_flags << PRIM_FLAG_SHIFT);	\
	 cullmask[i1] |= face_flags;					\
	 cullmask[i2] |= face_flags;					\
	 if (do_clip && ClipOr) cullmask[i0] |= PRIM_CLIPPED;		\
      }									\
      else								\
      {									\
	 cullcount+=nr;							\
      }									\
   }									\
} while (0)





#define CULL_QUAD( do_clip, do_area, i3, i2, i1, i, nr )		\
do {									\
   GLubyte ClipOr = 0;							\
									\
   if (do_clip) {							\
      ClipOr = (VB->ClipMask[i3] |					\
		VB->ClipMask[i2] | 					\
		VB->ClipMask[i1] | 					\
		VB->ClipMask[i]);					\
   }									\
									\
   if (do_clip && (ClipOr&CLIP_ALL_BITS))				\
   {									\
      if (VB->ClipMask[i3] & VB->ClipMask[i2] & 			\
	  VB->ClipMask[i1] & VB->ClipMask[i] &				\
	  CLIP_ALL_BITS) 						\
      {									\
	 cullcount+=nr;							\
      }									\
      else								\
      {									\
	 cullmask[i] = cull_faces | PRIM_CLIPPED;			\
	 cullmask[i1] = cull_faces | PRIM_CLIPPED; 			\
	 cullmask[i2] |= cull_faces; 			\
	 cullmask[i3] |= cull_faces; 					\
      }									\
   }									\
   else 								\
   {									\
      GLfloat area;							\
      GLubyte face_flags;						\
      									\
      if (do_area) {							\
	 area = ((proj[i1][0] - proj[i3][0]) * /* ex */			\
		 (proj[i ][1] - proj[i2][1]) - /* fy */			\
		 (proj[i1][1] - proj[i3][1]) * /* ey */			\
		 (proj[i ][0] - proj[i2][0])); /* fx */			\
	    								\
	 face_flags = (((area<0.0F) ^ face_bit) + 1) & cull_faces;	\
      } else {								\
	 face_flags = cull_faces;					\
      }									\
   									\
      if (do_area && !face_flags)					\
      {									\
	 cullcount+=nr;							\
      }									\
      else 								\
      {									\
	 cullmask[i]  = face_flags | (face_flags<<PRIM_FLAG_SHIFT);	\
	 cullmask[i1] = face_flags | (face_flags<<PRIM_FLAG_SHIFT);	\
	 cullmask[i2] |= face_flags;					\
	 cullmask[i3] |= face_flags;					\
	 if (ClipOr)							\
	    cullmask[i] |= PRIM_CLIPPED;				\
      } 								\
   }									\
} while(0)


#define DO_AREA 1
#define DO_CLIP 1
#define TAG(x) x
#include "cull_tmp.h"


#define DO_AREA 1
#define DO_CLIP 0
#define TAG(x) x##_area
#include "cull_tmp.h"

#define DO_AREA 0
#define DO_CLIP 1
#define TAG(x) x##_clip
#include "cull_tmp.h"



static GLuint gl_cull_dummy_prim( struct vertex_buffer *VB,
				  GLuint start, 
				  GLuint count, 
				  GLuint parity, 
				  CONST GLfloat (*proj)[4])
{
   (void) VB;
   (void) parity;
   (void) proj;
   return count - start;
}


/* KW:  Force the vertices which are used in both this and the
 * next vertex buffer to be fully transformed on their first 
 * appearance.  Need to do this because otherwise I would have
 * to try to copy the un-transformed (Obj, Normal) data.  This
 * is difficult in the case of shared normals, and seems
 * to be quite complex in the immediate mode case.  For now,
 * just force the copied vertices to be transformed.
 *
 * KW: Copy now organized to grow towards zero.
 */
static GLuint gl_copy_last_cull( struct vertex_buffer *VB, 
				 GLuint start, GLuint count,
				 GLuint ovf, CONST GLfloat (*proj)[4])
{
   const GLcontext *ctx = VB->ctx;
   GLubyte *cullmask = VB->CullMask;
   GLuint rv = 0;
   (void) start;
   (void) ovf;
   if (cullmask[count-1] == 0) {
      rv++;
      cullmask[count-1] = ctx->Polygon.CullBits;
   }
   VB->CopyCount = 1;
   VB->Copy[2] = count-1;
   COPY_4FV(VB->CopyProj[2], proj[count-1]);
   return rv;
}

static GLuint gl_copy_first_and_last_cull( struct vertex_buffer *VB, 
					   GLuint start, 
					   GLuint count, GLuint ovf,
					   CONST GLfloat (*proj)[4] )
{
   const GLcontext *ctx = VB->ctx;
   GLuint rv = 0;
   GLubyte *cullmask = VB->CullMask;
   (void) ovf;
   if (cullmask[count-1] == 0) {
      rv++;
      cullmask[count-1] = ctx->Polygon.CullBits;
   }
   if (cullmask[start] == 0) {
      rv++;
      cullmask[start] = ctx->Polygon.CullBits;
   }
   VB->CopyCount = 2;
   VB->Copy[1] = start;
   VB->Copy[2] = count-1;
   COPY_4FV(VB->CopyProj[1], proj[start]);
   COPY_4FV(VB->CopyProj[2], proj[count-1]);
   return rv;
}

/* Must be able to cope with zero or one overflow
 * 
 */
static GLuint gl_copy_last_two_cull( struct vertex_buffer *VB,
				     GLuint start, 
				     GLuint count, GLuint ovf,
				     CONST GLfloat (*proj)[4] )
{
   const GLcontext *ctx = VB->ctx;
   GLubyte *cullmask = VB->CullMask;
   GLuint rv = 0;
   (void) start;
   if (cullmask[count-1] == 0) {
      rv++;
      cullmask[count-1] = ctx->Polygon.CullBits;
   }
   if (cullmask[count-2] == 0) {
      rv++;
      cullmask[count-2] = ctx->Polygon.CullBits;
   }
   
   VB->CopyCount = 2;
   VB->Copy[1] = count-2;
   VB->Copy[2] = count-1;
   COPY_4FV(VB->CopyProj[1], proj[count-2]);
   COPY_4FV(VB->CopyProj[2], proj[count-1]);

   if (ovf == 1) {
      if (cullmask[count-3] == 0) {
	 rv++;
	 cullmask[count-3] = ctx->Polygon.CullBits;
      }
      VB->CopyCount = 3;
      VB->Copy[0] = count-3;
      COPY_4FV(VB->CopyProj[0], proj[count-3]);
   }

   return rv;
}


/* Eg, quads - just copy the overflow, guarenteed to all be culled.
 */
static GLuint gl_copy_overflow_cull( struct vertex_buffer *VB, 
				     GLuint start, GLuint count,
				     GLuint ovf, CONST GLfloat (*proj)[4] )
{
   const GLcontext *ctx = VB->ctx;
   GLubyte *cullmask = VB->CullMask;
   GLuint src_offset = count - ovf;
   GLuint dst_offset = 3 - ovf;
   GLuint i;
   (void) start;
   VB->CopyCount = ovf;
   for (i = 0 ; i < ovf ; i++) {
      cullmask[i+src_offset] = ctx->Polygon.CullBits;
      VB->Copy[i+dst_offset] = i+src_offset;
      COPY_4FV(VB->CopyProj[i+dst_offset], proj[i+src_offset]);
   }
   return ovf;
}


static GLuint gl_copy_last( struct vertex_buffer *VB,
			    GLuint start, GLuint count,
			    GLuint ovf, CONST GLfloat (*proj)[4])
{
   (void) start;
   (void) ovf;
   VB->CopyCount = 1;
   VB->Copy[2] = count-1;
   COPY_4FV(VB->CopyProj[2], proj[count-1]);
   return 0;
}

static GLuint gl_copy_first_and_last( struct vertex_buffer *VB, 
				      GLuint start, 
				      GLuint count,
				      GLuint ovf, CONST GLfloat (*proj)[4])
{
   (void) ovf;
   VB->CopyCount = 2;
   VB->Copy[1] = start;
   VB->Copy[2] = count-1;
   COPY_4FV(VB->CopyProj[1], proj[start]);
   COPY_4FV(VB->CopyProj[2], proj[count-1]);
   return 0;
}

static GLuint gl_copy_last_two( struct vertex_buffer *VB, 
				GLuint start, GLuint count, 
				GLuint ovf, CONST GLfloat (*proj)[4])
{
   (void) start;
   VB->CopyCount = 2;
   VB->Copy[1] = count-2;
   VB->Copy[2] = count-1;
   COPY_4FV(VB->CopyProj[1], proj[count-2]);
   COPY_4FV(VB->CopyProj[2], proj[count-1]);

   if (ovf == 1) {
      VB->CopyCount = 3;
      VB->Copy[0] = count-3;
      COPY_4FV(VB->CopyProj[0], proj[count-3]);
   }

   return 0;
}

static GLuint gl_copy_overflow( struct vertex_buffer *VB, 
				GLuint start, 
				GLuint count,
				GLuint ovf,
				CONST GLfloat (*proj)[4] )
{
   GLuint src_offset = count - ovf;
   GLuint dst_offset = 3 - ovf;
   GLuint i;
   (void) start;
   VB->CopyCount = ovf;
   for (i = 0 ; i < ovf ; i++) {
      VB->Copy[i+dst_offset] = i+src_offset;
      COPY_4FV(VB->CopyProj[i+dst_offset], proj[i+src_offset]);
   }

   return 0;
}

static void gl_fast_copy_noop( struct vertex_buffer *VB,
			       GLuint start, GLuint count,
			       GLuint ovf)
{
   (void) start;
   (void) ovf;
   (void) VB;
   (void) count;
}

static void gl_fast_copy_last( struct vertex_buffer *VB,
			       GLuint start, GLuint count,
			       GLuint ovf)
{
   (void) start;
   (void) ovf;
   VB->CopyCount = 1;
   VB->Copy[2] = count-1;
}

static void gl_fast_copy_first_and_last( struct vertex_buffer *VB, 
					 GLuint start, 
					 GLuint count,
					 GLuint ovf)
{
   (void) ovf;
   VB->CopyCount = 2;
   VB->Copy[1] = start;
   VB->Copy[2] = count-1;
}

static void gl_fast_copy_last_two( struct vertex_buffer *VB, 
				   GLuint start, GLuint count, 
				   GLuint ovf )
{
   (void) start;
   VB->CopyCount = 2;
   VB->Copy[1] = count-2;
   VB->Copy[2] = count-1;

   if (ovf == 1) {
      VB->CopyCount = 3;
      VB->Copy[0] = count-3;
   }
}

static void gl_fast_copy_overflow( struct vertex_buffer *VB, 
				   GLuint start, 
				   GLuint count,
				   GLuint ovf )
{
   GLuint src_offset = count - ovf;
   GLuint dst_offset = 3 - ovf;
   GLuint i;
   (void) start;
   VB->CopyCount = ovf;
   for (i = 0 ; i < ovf ; i++) 
      VB->Copy[i+dst_offset] = i+src_offset;
}


typedef void (*fast_copy_func)( struct vertex_buffer *VB, 
				GLuint start, 
				GLuint count,
				GLuint ovf );




typedef GLuint (*cull_func)( struct vertex_buffer *VB, 
			     GLuint start,
			     GLuint count,
			     GLuint parity,
			     CONST GLfloat (*proj)[4]);


typedef GLuint (*copy_func)( struct vertex_buffer *VB, 
			     GLuint start, 
			     GLuint end,
			     GLuint ovf,
			     CONST GLfloat (*proj)[4] );



static cull_func cull_tab_both[GL_POLYGON+2] = 
{
   gl_cull_points,
   gl_cull_lines,
   gl_cull_line_loop,
   gl_cull_line_strip,
   gl_cull_triangles,
   gl_cull_triangle_strip,
   gl_cull_triangle_fan,
   gl_cull_quads,
   gl_cull_quad_strip,
   gl_cull_triangle_fan,
   gl_cull_dummy_prim
};

static cull_func cull_tab_area[GL_POLYGON+2] = 
{
   gl_cull_points_neither,
   gl_cull_points_neither,
   gl_cull_points_neither,
   gl_cull_points_neither,
   gl_cull_triangles_area,
   gl_cull_triangle_strip_area,
   gl_cull_triangle_fan_area,
   gl_cull_quads_area,
   gl_cull_quad_strip_area,
   gl_cull_triangle_fan_area,
   gl_cull_dummy_prim
};

static cull_func cull_tab_clip[GL_POLYGON+2] = 
{
   gl_cull_points,
   gl_cull_lines,
   gl_cull_line_loop,
   gl_cull_line_strip,
   gl_cull_triangles_clip,
   gl_cull_triangle_strip_clip,
   gl_cull_triangle_fan_clip,
   gl_cull_quads_clip,
   gl_cull_quad_strip_clip,
   gl_cull_triangle_fan_clip,
   gl_cull_dummy_prim
};

static cull_func *cull_tab[4] = 
{
   0,
   cull_tab_area,
   cull_tab_clip,
   cull_tab_both
};


static copy_func copy_tab_cull[GL_POLYGON+2] = 
{
   0,
   gl_copy_overflow_cull,
   gl_copy_first_and_last_cull,
   gl_copy_last_cull,
   gl_copy_overflow_cull,
   gl_copy_last_two_cull,
   gl_copy_first_and_last_cull,
   gl_copy_overflow_cull,
   gl_copy_last_two_cull,
   gl_copy_first_and_last_cull,
   0,
};


static copy_func copy_tab_no_cull[GL_POLYGON+2] = 
{
   0,
   gl_copy_overflow,
   gl_copy_first_and_last,
   gl_copy_last,
   gl_copy_overflow,
   gl_copy_last_two,
   gl_copy_first_and_last,
   gl_copy_overflow,
   gl_copy_last_two,
   gl_copy_first_and_last,
   0,
};


static fast_copy_func fast_copy_tab[GL_POLYGON+2] = 
{
   gl_fast_copy_noop,
   gl_fast_copy_overflow,
   gl_fast_copy_first_and_last,
   gl_fast_copy_last,
   gl_fast_copy_overflow,
   gl_fast_copy_last_two,
   gl_fast_copy_first_and_last,
   gl_fast_copy_overflow,
   gl_fast_copy_last_two,
   gl_fast_copy_first_and_last,
   gl_fast_copy_noop,
};





/* Do this after (?) primitive fixup on buffers containing unwanted
 * eval coords.  No early culling will be done on these vertices (it's
 * a degenerate case & not worth the code), and the vertices will be
 * rendered by a special 'indirect' case in gl_render_vb.  Otherwise,
 * the cullmask will force (some) transforms to skip these vertices.
 */
void gl_purge_vertices( struct vertex_buffer *VB )
{
   GLuint *flags = VB->Flag;
   GLubyte *cullmask = VB->CullMask;
   GLuint *indirect = VB->Indirect = VB->EltPtr->start;
   GLuint *in_prim = VB->Primitive;
   GLuint *out_prim = VB->IM->Primitive;
   GLuint *in_nextprim = VB->NextPrimitive;
   GLuint *out_nextprim = VB->IM->NextPrimitive;
   GLuint count = VB->Count;
   GLuint purge = VB->PurgeFlags;
   GLuint next, start;
   GLuint j;

   /*
    */
   for ( j = start = VB->CopyStart ; start < count ; start = next ) {
      GLuint startj = j;
      GLuint i;

      next = in_nextprim[start]; 
      out_prim[j] = in_prim[start];

      for ( i = start ; i < next ; i++) {
	 if (~(flags[i] & purge)) {
	    indirect[j] = i;
	    cullmask[i] = PRIM_CLIPPED;	/* nonzero */
	    j++;
	 }
	 
      }

      out_nextprim[startj] = j;
   }
   
   VB->Primitive = out_prim;
   VB->NextPrimitive = out_nextprim;
   VB->IndirectCount = j;
}

#define VERT_NOT_CLIPPED 0x80


static void build_clip_vert_bits( GLubyte *clipmask, const GLubyte *cullmask,
				  GLuint count )
{
   GLuint i = 0;
   for (;;)
      if (cullmask[++i]==0) {
	 clipmask[i] |= CLIP_CULLED_BIT;
	 if (&cullmask[i] == &cullmask[count]) return;
      }
}

GLuint gl_cull_vb( struct vertex_buffer *VB )
{
   const GLcontext *ctx = VB->ctx;
   GLuint i, next, prim = 0xffffffff, n;
   GLfloat (*proj)[4] = VB->Projected->data;
   GLuint *in_prim = VB->Primitive;
   GLuint *out_prim = VB->IM->Primitive;
   GLuint cullcount = 0;
   GLuint lastprim = in_prim[VB->LastPrimitive];
   GLuint first = VB->CopyStart;
   GLuint parity = VB->Parity;
   cull_func *cull_funcs;
   GLuint idx = 0;

   if (VB->CullDone)
      return 0;

   if (VB->ClipOrMask)
      idx |= 0x2;

   if (ctx->IndirectTriangles & DD_ANY_CULL)
      idx |= 0x1;
   
   cull_funcs = cull_tab[idx];

   for (i = VB->CopyStart ; i < VB->Start ; i++) {
      COPY_4FV(proj[i], VB->CopyProj[i]);
   }

   VB->CopyCount = 0;

   MEMSET(VB->CullMask, 0, (VB->Count+1) * sizeof(GLubyte));

   for ( i = VB->CopyStart ; i < VB->Count ; i = next, parity = 0 ) 
   {
      first = i;
      next = VB->NextPrimitive[i];
      prim = in_prim[i];
      n = cull_funcs[prim]( VB, i, next, parity, (CONST GLfloat (*)[4])proj );

      if (n == next - i)
	 out_prim[i] = GL_POLYGON+1; 
      else 
	 out_prim[i] = prim;

      cullcount += n;
   }

   if (VB->LastPrimitive <  VB->Count) {
      if (copy_tab_cull[lastprim]) 
	 cullcount -= copy_tab_cull[prim]( VB, 
					   first, 
					   VB->Count, 
					   VB->Ovf, 
					   (CONST GLfloat (*)[4])proj );     
   }

   VB->Primitive = out_prim;
   
   VB->CullFlag[0] = VB->CullFlag[1] = (GLubyte) 0;

   if (cullcount > 0 || (ctx->IndirectTriangles & DD_LIGHTING_CULL)) {
      VB->CullMode |= CULL_MASK_ACTIVE;
      VB->CullFlag[0] = VB->CullFlag[1] = CLIP_CULLED_BIT&ctx->AllowVertexCull;

      if (cullcount < VB->Count)
	 build_clip_vert_bits( VB->ClipMask, VB->CullMask, VB->Count );
   }

   if (VB->ClipOrMask) {
      VB->CullMode |= CLIP_MASK_ACTIVE;
      VB->CullFlag[1] |= (CLIP_ALL_BITS|CLIP_USER_BIT) & ctx->AllowVertexCull; 
   }

   VB->CullDone = 1;
   return cullcount;
}


/* Need to set up to copy some vertices at the end of the buffer, even
 * if culling is disabled or the entire vb has been culled by clipping.
 */
void gl_dont_cull_vb( struct vertex_buffer *VB )
{
   GLfloat (*proj)[4] = VB->Projected->data;
   GLuint i;

   if (VB->CullDone)
      return;

   for (i = VB->CopyStart ; i < VB->Start ; i++) 
      COPY_4FV(proj[i], VB->CopyProj[i]);
   
   VB->CopyCount = 0;

   if (VB->LastPrimitive < VB->Count) 
   {
      GLuint first = VB->LastPrimitive;
      GLuint prim = VB->Primitive[first];

      if (first == VB_START)
	 first = VB->Start;

      if (copy_tab_no_cull[prim]) 
	 copy_tab_no_cull[prim]( VB, 
				 first, 
				 VB->Count, 
				 VB->Ovf,
				 (CONST GLfloat (*)[4])proj );
   }

   VB->CullDone = 1;
}

void gl_fast_copy_vb( struct vertex_buffer *VB )
{
   VB->CopyCount = 0;

   if (VB->LastPrimitive < VB->Count) 
   {
      GLuint first = VB->LastPrimitive;
      GLuint prim = VB->Primitive[first];

      if (first == VB_START) 
	 first = VB->Start;

      fast_copy_tab[prim]( VB, first, VB->Count, VB->Ovf );
   }

   VB->CullDone = 1;
}





void gl_make_normal_cullmask( struct vertex_buffer *VB )
{
   GLuint i;

   if (VB->CullMode & COMPACTED_NORMALS) {
      /* The shared normals case - never for cva.
       */
      MEMSET(VB->NormCullMask, 0, VB->Count * sizeof(GLubyte));      
      VB->NormCullStart = VB->NormCullMask + VB->Start;

      if (VB->CullMode & CULL_MASK_ACTIVE) {
	 GLubyte *lastnorm = VB->NormCullStart;

	 for (i = VB->Start ;;) {
	    *lastnorm |= VB->CullMask[i];
	    if (VB->Flag[++i] & (VERT_NORM|VERT_END_VB)) {
	       lastnorm = &VB->NormCullMask[i];
	       if (VB->Flag[i] & VERT_END_VB) return;
	    }
	 }
      } else {
	 VB->NormCullMask[VB->Start] = ~0;
	 for (i = VB->Start ;; ) 
	    if (VB->Flag[++i] & (VERT_NORM|VERT_END_VB)) {
	       VB->NormCullMask[i] = ~0;
	       if (VB->Flag[i] & VERT_END_VB) return;
	    }
      }
   } else  {
      /*  Non-shared normals.
       */
      VB->NormCullStart = VB->CullMask + VB->Start;      
   }   
}




