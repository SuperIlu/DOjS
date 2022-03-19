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


static void TAG(cva_render_points)( struct vertex_buffer *cvaVB,
				    struct vertex_buffer *VB,
				    const struct gl_prim_state *state,
				    GLuint start,
				    GLuint count ) 
{
   GLcontext *ctx = VB->ctx;
   fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
   fxVertex *gWin = FX_DRIVER_DATA(cvaVB)->verts;
   const GLuint *elt = VB->EltPtr->data;
   GLuint i;

   VARS;
   INIT;
   (void) fxMesa;

   if (cvaVB->ClipOrMask) {
      const GLubyte *clip = cvaVB->ClipMask;
      for (i = start ; i < count ; i++ INCR) {
	 GLuint e = elt[i];
	 if (!clip[e]) {
	    GLfloat *v = gWin[e].f; (void) v;
	    if (!DIRECT) { MERGE_VB; }
	    MERGE_RAST;
	    DRAW_POINT;
	 }
      }
   } else {
      for (i = start ; i < count ; i++ INCR) {
	 GLuint e = elt[i];
	 GLfloat *v = gWin[e].f; (void) v;
	 if (!DIRECT) { MERGE_VB; }
	 MERGE_RAST;
	 DRAW_POINT;
      }
   }
}

static void TAG(cva_render_lines)( struct vertex_buffer *cvaVB,
				   struct vertex_buffer *VB,
				   const struct gl_prim_state *state,
				   GLuint start,
				   GLuint count ) 
{
   GLcontext *ctx = VB->ctx;
   fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
   fxVertex *gWin = FX_DRIVER_DATA(cvaVB)->verts;
   const GLuint *elt = VB->EltPtr->data;
   GLuint i;

   VARS;
   INIT;
   (void) fxMesa;

   if (cvaVB->ClipOrMask) {
      const GLubyte *clip = cvaVB->ClipMask;
      GLuint prev = 0;
      GLfloat *prev_v = 0;

      (void) ctx;
   
      for (i = start ; i < count ; i++ INCR) {
	 GLuint e = elt[i];
	 GLfloat *v = gWin[e].f;

	 MERGE_VB;

	 if (!clip[e])
	    MERGE_RAST;

	 if (state->draw) {
	    if (clip[e] | clip[prev]) 
	       CLIP_LINE;
	    else
	       DRAW_LINE;
	 }

	 prev = e;	 
	 prev_v = v;
	 state = state->next;
      }
      if (state->finish_loop) {
	 GLuint e = elt[start];
	 GLfloat *v = gWin[e].f; (void) v;

	 if (!DIRECT) { MERGE_VB; }
	 MERGE_RAST;

	 if (clip[e] | clip[prev]) 
	    CLIP_LINE;
	 else
	    DRAW_LINE;
      }
   } else {
      GLuint prev = 0;
      GLfloat *prev_v = 0;
      
      for (i = start ; i < count ; i++ INCR) {
	 GLuint e = elt[i];
	 GLfloat *v = gWin[e].f;

	 if (!DIRECT) { MERGE_VB; }
	 MERGE_RAST;
	 if (state->draw) DRAW_LINE;
	 prev = e;
	 prev_v = v;
	 state = state->next;
      }
      if (state->finish_loop) {
	 GLuint e = elt[start];
	 GLfloat *v = gWin[e].f; (void) v;

	 if (!DIRECT) { MERGE_VB; }
	 MERGE_RAST;
	 DRAW_LINE;
      }
   }      
}


static void TAG(cva_render_tris)( struct vertex_buffer *cvaVB,
				  struct vertex_buffer *VB,
				  const struct gl_prim_state *state,
				  GLuint start,
				  GLuint count ) 
{
   GLcontext *ctx = VB->ctx;
   fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
   fxVertex *gWin = FX_DRIVER_DATA(cvaVB)->verts;
   const GLuint *elt = VB->EltPtr->data;
   GLuint i;
   VARS;
   INIT;
   (void) fxMesa;

   if (cvaVB->ClipOrMask) {      
      GLuint vlist[VB_MAX_CLIPPED_VERTS];
      GLuint l[3];
      const GLubyte *clip = cvaVB->ClipMask; 
      
      (void) vlist;

      for (i = start ; i < count ; i++ INCR) {
	 GLuint e = l[2] = elt[i];
	 GLfloat *v = gWin[e].f; (void) v;

	 MERGE_VB;		/* needed for clip-interp */

	 if (!clip[e]) {
	    MERGE_RAST;
	 }

	 if (state->draw) 
	    CLIP_OR_DRAW_TRI;


	 l[0] = l[state->v0];
	 l[1] = l[state->v1];
	 state = state->next;
      }
   } else if (DIRECT) {
      GrVertex *vl[3];
      for (i = start ; i < count ; i++ INCR) {
	 GLuint e = elt[i];
	 GLfloat *v = gWin[elt[i]].f;

	 vl[2] = (GrVertex *)v;

	 (void) v; 
	 (void) e;

	 MERGE_RAST;

	 if (state->draw) 
	    DRAW_TRI2;

	 vl[0] = vl[state->v0];
	 vl[1] = vl[state->v1];
	 state = state->next;
      }
   } else {
      GLuint l[3];
      for (i = start ; i < count ; i++ INCR) {
	 GLuint e = l[2] = elt[i];
	 GLfloat *v = gWin[e].f; (void) v;

	 MERGE_VB;		/* needed for ctx->trianglefunc? */
	 MERGE_RAST;
	 
	 if (state->draw) 
	    DRAW_TRI;

	 l[0] = l[state->v0];
	 l[1] = l[state->v1];
	 state = state->next;
      }
   }
}


static void TAG(init_cva)( void )
{
   merge_and_render_tab[DIRECT][IDX][PRIM_POINTS] = TAG(cva_render_points);
   merge_and_render_tab[DIRECT][IDX][PRIM_LINES] = TAG(cva_render_lines);
   merge_and_render_tab[DIRECT][IDX][PRIM_TRIS] = TAG(cva_render_tris);
   merge_and_render_tab[DIRECT][IDX][PRIM_CULLED] = fxCvaRenderNoop;
}


#undef IDX
#undef MERGE_RAST
#undef MERGE_VB
#undef VARS
#undef INIT
#undef INCR
#undef TAG

