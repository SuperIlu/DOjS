/* $Id: vbindirect.c,v 1.5.4.1 2000/11/08 16:41:17 brianp Exp $ */

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
 * New (3.1) transformation code written by Keith Whitwell.
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "pb.h"
#include "pipeline.h"
#include "stages.h"
#include "types.h"
#include "vb.h"
#include "vbcull.h"
#include "vbindirect.h"
#include "vbrender.h"
#endif



static struct gl_prim_state next_lines[3] = {
   { 1, 2, 0, 0, &next_lines[1] },
   { 1, 2, 1, 0, &next_lines[0] }
};

static struct gl_prim_state next_line_loop[3] = {
   { 1, 2, 0, 0, &next_line_loop[1] },
   { 1, 2, 1, 1, &next_line_loop[1] }
};

static struct gl_prim_state next_line_strip[3] = {
   { 1, 2, 0, 0, &next_line_strip[1] },
   { 1, 2, 1, 0, &next_line_strip[1] }
};

static struct gl_prim_state next_poly[3] = {
   { 1, 2, 0, 0, &next_poly[1] },
   { 1, 2, 0, 0, &next_poly[2] },
   { 0, 2, 1, 0, &next_poly[2] }
};

static struct gl_prim_state next_tris[3] = {
   { 1, 2, 0, 0, &next_tris[1] },
   { 1, 2, 0, 0, &next_tris[2] },
   { 1, 2, 1, 0, &next_tris[0] } 
};

static struct gl_prim_state next_tri_strip0[4] = {
   { 2, 1, 0, 0, &next_tri_strip0[1] },
   { 0, 2, 0, 0, &next_tri_strip0[2] },
   { 2, 1, 1, 0, &next_tri_strip0[3] },
   { 0, 2, 1, 0, &next_tri_strip0[2] },
};

static struct gl_prim_state next_tri_strip1[4] = {
   { 0, 2, 0, 0, &next_tri_strip1[1] },
   { 2, 1, 0, 0, &next_tri_strip1[2] },
   { 0, 2, 1, 0, &next_tri_strip1[3] },
   { 2, 1, 1, 0, &next_tri_strip1[2] },
};

static struct gl_prim_state next_quads[4] = {
   { 1, 2, 0, 0, &next_quads[1] },
   { 2, 1, 0, 0, &next_quads[2] },
   { 1, 2, 1, 0, &next_quads[3] }, 
   { 1, 2, 1, 0, &next_quads[0] }
};

static struct gl_prim_state next_quad_strip[4] = {
   { 2, 1, 0, 0, &next_quad_strip[1] },
   { 0, 2, 0, 0, &next_quad_strip[2] },
   { 2, 1, 1, 0, &next_quad_strip[3] },
   { 0, 2, 1, 0, &next_quad_strip[2] },
};







#define INIT(ctx, prim)					\
do {							\
   if (ctx->PB->count > 0) gl_flush_pb(ctx);		\
   if (ctx->PB->primitive != prim) {			\
      gl_reduced_prim_change( ctx, prim );		\
   }							\
} while(0)



/* Need better driver support for this case.
 */
static void 
indexed_render_points( struct vertex_buffer *VB,
		       const struct gl_prim_state *state,
		       const GLuint *elt,
		       GLuint start,
		       GLuint count )
{
   GLcontext *ctx = VB->ctx;
   GLuint i;
   (void) state;
   INIT(ctx, GL_POINTS);

   if (VB->ClipOrMask) {
      const GLubyte *clip = VB->ClipMask;
      for (i = start ; i < count ; i++) 
	 if (!clip[elt[i]]) 
	    ctx->Driver.PointsFunc( ctx, elt[i], elt[i]+1 );
   } else {
      for (i = start ; i < count ; i++) 
	 ctx->Driver.PointsFunc( ctx, elt[i], elt[i]+1 );
   }
}

static void 
indexed_render_lines( struct vertex_buffer *VB,
		      const struct gl_prim_state *state,
		      const GLuint *elt,
		      GLuint start,
		      GLuint count )
{
   GLcontext *ctx = VB->ctx;
   GLuint i;

   INIT(ctx, GL_LINES);

   if (VB->ClipOrMask) {
      const GLubyte *clip = VB->ClipMask;
      GLuint prev = 0;
   
      for (i = start ; i < count ; i++) {
	 GLuint curr = elt[i];
	 if (state->draw) {
	    if (clip[curr] | clip[prev]) 
	       gl_render_clipped_line( ctx, prev, curr ); 
	    else
	       ctx->Driver.LineFunc( ctx, prev, curr, curr );	 
         }
	 prev = curr;	 
	 state = state->next;
      }
      if (state->finish_loop) {
	 GLuint curr = elt[start];
	 if (clip[curr] | clip[prev]) 
	    gl_render_clipped_line( ctx, prev, curr ); 
	 else
	    ctx->Driver.LineFunc( ctx, prev, curr, curr );
      }
   } else {
      GLuint prev = 0;
      for (i = start ; i < count ; i++) {
	 GLuint curr = elt[i];
	 if (state->draw) ctx->Driver.LineFunc( ctx, prev, curr, curr ); 
	 prev = curr;
	 state = state->next;
      }
      if (state->finish_loop) {
	 GLuint curr = elt[start];
	 ctx->Driver.LineFunc( ctx, prev, curr, curr );
      }
   }      
}



static void 
indexed_render_tris( struct vertex_buffer *VB,
		     const struct gl_prim_state *state,
		     const GLuint *elt,
		     GLuint start,
		     GLuint count ) 
{
   GLcontext *ctx = VB->ctx;
   GLuint i;

   INIT(ctx, GL_POLYGON);

   if (VB->ClipOrMask) {      
      GLuint v[3];
      GLuint vlist[VB_MAX_CLIPPED_VERTS];

      const GLubyte *clip = VB->ClipMask;
      for (i = start ; i < count ; i++) {
	 v[2] = elt[i];
	 if (state->draw) { 
	    if (clip[v[0]] | clip[v[1]] | clip[v[2]]) {
	       if (!(clip[v[0]] & clip[v[1]] & clip[v[2]] & CLIP_ALL_BITS)) {
		  COPY_3V(vlist, v);
		  gl_render_clipped_triangle( ctx, 3, vlist, vlist[2] );
	       }
	    }
	    else
	       ctx->TriangleFunc( ctx, v[0], v[1], v[2], v[2] );
	 }
	 v[0] = v[state->v0];
	 v[1] = v[state->v1];
	 state = state->next;
      }
   } else {
      GLuint v[3];
      const triangle_func tf = ctx->TriangleFunc;

      for (i = start ; i < count ; i++) {
	 v[2] = elt[i];
	 if (state->draw)
	    tf( ctx, v[0], v[1], v[2], v[2] );
	 v[0] = v[state->v0];
	 v[1] = v[state->v1];
	 state = state->next;
      }
   }
}


static void 
indexed_render_noop( struct vertex_buffer *VB,
		     const struct gl_prim_state *state,
		     const GLuint *elt,
		     GLuint start,
		     GLuint count ) 
{
	(void) VB;
	(void) state;
	(void) elt;
	(void) start;
	(void) count;
}


typedef void (*indexed_render_func)( struct vertex_buffer *VB,
				     const struct gl_prim_state *state,
				     const GLuint *elt,
				     GLuint start,
				     GLuint count );


indexed_render_func prim_func[GL_POLYGON+2] = {
   indexed_render_points,
   indexed_render_lines,
   indexed_render_lines,
   indexed_render_lines,
   indexed_render_tris,
   indexed_render_tris,
   indexed_render_tris,
   indexed_render_tris,
   indexed_render_tris,
   indexed_render_tris,
   indexed_render_noop
};
   

CONST struct gl_prim_state *gl_prim_state_machine[GL_POLYGON+2][2] = {
   { 0, 0 },
   { next_lines,      next_lines },
   { next_line_loop,  next_line_loop },
   { next_line_strip, next_line_strip },
   { next_tris,       next_tris },
   { next_tri_strip0, next_tri_strip1 },
   { next_poly,       next_poly },
   { next_quads,      next_quads },
   { next_quad_strip, next_quad_strip },
   { next_poly,       next_poly },
};




void gl_render_elts( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   struct vertex_buffer *saved_vb = ctx->VB;
   GLenum prim = ctx->CVA.elt_mode;
   GLuint *elt = VB->EltPtr->start;
   GLuint nr = VB->EltPtr->count;
   GLuint p = 0;

   gl_import_client_data( VB, ctx->RenderFlags,
			  (VB->ClipOrMask 
			   ? VEC_WRITABLE|VEC_GOOD_STRIDE
			   : VEC_GOOD_STRIDE));

   ctx->VB = VB;

   if (ctx->Driver.RenderStart)
      ctx->Driver.RenderStart( ctx );
   
   do {
      const struct gl_prim_state *state = gl_prim_state_machine[prim][0];
      indexed_render_func func = prim_func[prim];
      
      func( VB, state, elt, 0, nr );

      if (ctx->TriangleCaps & DD_TRI_LIGHT_TWOSIDE) {
	 VB->Specular = VB->Spec[0];
	 VB->ColorPtr = VB->Color[0];
	 VB->IndexPtr = VB->Index[0];
      }
      
   } while (ctx->Driver.MultipassFunc &&
	    ctx->Driver.MultipassFunc( VB, ++p ));

   
   if (ctx->PB->count > 0) 
      gl_flush_pb(ctx);

   if (ctx->Driver.RenderFinish)
      ctx->Driver.RenderFinish( ctx );

   ctx->VB = saved_vb;
}



void gl_render_vb_indirect( struct vertex_buffer *VB )
{
   GLuint i, next, prim;
   GLuint parity = VB->Parity;
   GLuint count = VB->Count;
   const struct gl_prim_state *state;
   indexed_render_func func;
   GLcontext *ctx = VB->ctx;
   struct vertex_buffer *cvaVB = ctx->CVA.VB;
   struct vertex_buffer *saved_vb = ctx->VB;
   GLuint p = 0;

   gl_import_client_data( cvaVB, ctx->RenderFlags,
			  (VB->ClipOrMask 
			   ? VEC_WRITABLE|VEC_GOOD_STRIDE
			   : VEC_GOOD_STRIDE));

   ctx->VB = cvaVB;

   if (!VB->CullDone)
      gl_fast_copy_vb( VB );

   if (/*  ctx->Current.Primitive == GL_POLYGON+1 &&  */ctx->Driver.RenderStart)
      ctx->Driver.RenderStart( ctx );

   do {
      for (i = VB->CopyStart ; i < count ; parity = 0, i = next ) {
	 prim = VB->Primitive[i];
	 next = VB->NextPrimitive[i];

	 state = gl_prim_state_machine[prim][parity];
	 func = prim_func[prim];
      
	 func( cvaVB, state, VB->EltPtr->data, i, next );
      
	 if (ctx->TriangleCaps & DD_TRI_LIGHT_TWOSIDE) {
	    cvaVB->Specular = cvaVB->Spec[0];
	    cvaVB->ColorPtr = cvaVB->Color[0];
	    cvaVB->IndexPtr = cvaVB->Index[0];
	 }
      }

   } while (ctx->Driver.MultipassFunc &&
	    ctx->Driver.MultipassFunc( VB, ++p ));


   if (ctx->PB->count > 0) 
      gl_flush_pb(ctx);

   if (/*  ctx->Current.Primitive == GL_POLYGON+1 &&  */ctx->Driver.RenderFinish)
      ctx->Driver.RenderFinish( ctx );

   ctx->VB = saved_vb;
}
