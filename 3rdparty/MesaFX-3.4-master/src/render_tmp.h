/* $Id: render_tmp.h,v 1.11 2000/07/10 13:22:01 keithw Exp $ */

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
 * New (3.1) transformation code written by Keith Whitwell.
 */

#ifndef POSTFIX
#define POSTFIX
#endif

#ifndef INIT
#define INIT(x)  
#endif

#ifndef NEED_EDGEFLAG_SETUP
#define NEED_EDGEFLAG_SETUP 0
#define EDGEFLAG_TRI(a,b,c,d,e)
#define EDGEFLAG_POLY_TRI_PRE(a,b,c,d)
#define EDGEFLAG_POLY_TRI_POST(a,b,c,d)
#define EDGEFLAG_QUAD(a,b,c,d,e)
#endif

#ifndef RESET_STIPPLE
#define RESET_STIPPLE
#endif


static void TAG(render_vb_points)( struct vertex_buffer *VB,
				   GLuint start,
				   GLuint count,
				   GLuint parity )
{
   LOCAL_VARS;
   (void) parity;
   VB->ctx->OcclusionResult = GL_TRUE;

   INIT(GL_POINTS);
   RENDER_POINTS( start, count );
   POSTFIX;
}

static void TAG(render_vb_lines)( struct vertex_buffer *VB,
				  GLuint start,
				  GLuint count,
				  GLuint parity )
{
   GLuint j;
   LOCAL_VARS;
   (void) parity;
   VB->ctx->OcclusionResult = GL_TRUE;

   INIT(GL_LINES);
   for (j=start+1; j<count; j+=2 ) {
      RENDER_LINE( j-1, j );
      RESET_STIPPLE;
   }
   POSTFIX;
}


static void TAG(render_vb_line_strip)( struct vertex_buffer *VB,
				       GLuint start,
				       GLuint count,
				       GLuint parity )
{
   GLuint j;
   LOCAL_VARS;
   (void) parity;
   VB->ctx->OcclusionResult = GL_TRUE;

   INIT(GL_LINES);
   for (j=start+1; j<count; j++ ) {
      RENDER_LINE( j-1, j );
   }   

   if (VB->Flag[count] & VERT_END) {
      RESET_STIPPLE;
   }
   POSTFIX;
}


static void TAG(render_vb_line_loop)( struct vertex_buffer *VB,
				      GLuint start,
				      GLuint count,
				      GLuint parity )
{
   GLuint i = start < VB->Start ? VB->Start : start + 1;	 
   LOCAL_VARS;
   (void) parity;
   VB->ctx->OcclusionResult = GL_TRUE;

   INIT(GL_LINES);
   for ( ; i < count ; i++) {
      RENDER_LINE( i-1, i );
   }

   if (VB->Flag[count] & VERT_END) {
      RENDER_LINE( i-1, start );
      RESET_STIPPLE;
   }

   POSTFIX;
}


static void TAG(render_vb_triangles)( struct vertex_buffer *VB,
				      GLuint start,
				      GLuint count,
				      GLuint parity )
{
   GLuint j;
   LOCAL_VARS;
   (void) parity;

   INIT(GL_POLYGON);
   for (j=start+2; j<count; j+=3) {
      RENDER_TRI( j-2, j-1, j, j, 0 );
      RESET_STIPPLE;
   }
   POSTFIX;
}



static void TAG(render_vb_tri_strip)( struct vertex_buffer *VB,
				      GLuint start,
				      GLuint count,
				      GLuint parity )
{
   GLuint j;
   LOCAL_VARS;
   (void) parity;
   INIT(GL_POLYGON);
   if (NEED_EDGEFLAG_SETUP) {
      for (j=start+2;j<count;j++,parity^=1) {
	 EDGEFLAG_TRI( j-2, j-1, j, j, parity );
	 RENDER_TRI( j-2, j-1, j, j, parity );
	 RESET_STIPPLE;
      }
   } else {
      for (j=start+2;j<count;j++,parity^=1) {
	 RENDER_TRI( j-2, j-1, j, j, parity );
      }
   }
   POSTFIX;
}


static void TAG(render_vb_tri_fan)( struct vertex_buffer *VB,
				    GLuint start,
				    GLuint count,
				    GLuint parity )
{
   GLuint j;
   LOCAL_VARS;
   (void) parity;
   INIT(GL_POLYGON);
   if (NEED_EDGEFLAG_SETUP) {
      for (j=start+2;j<count;j++) {
	 EDGEFLAG_TRI( start, j-1, j, j, 0 );
	 RENDER_TRI( start, j-1, j, j, 0 );
	 RESET_STIPPLE;
      }
   } else {
      for (j=start+2;j<count;j++) {
	 RENDER_TRI( start, j-1, j, j, 0 );
      }
   }

   POSTFIX;
}


static void TAG(render_vb_poly)( struct vertex_buffer *VB,
				 GLuint start,
				 GLuint count,
				 GLuint parity )
{
   GLuint j;
   LOCAL_VARS;
   (void) parity;
   INIT(GL_POLYGON);
   if (NEED_EDGEFLAG_SETUP) {
      for (j=start+2;j<count;j++) {
	 EDGEFLAG_POLY_TRI_PRE( start, j-1, j, start );
	 RENDER_TRI( start, j-1, j, start, 0 );
	 EDGEFLAG_POLY_TRI_POST( start, j-1, j, start );
      }
      if (VB->Flag[count] & VERT_END) {
	 RESET_STIPPLE;
      }
   }
   else {
      for (j=start+2;j<count;j++) {
	 RENDER_TRI( start, j-1, j, start, 0 );
      }
   }
   POSTFIX;
}


static void TAG(render_vb_quads)( struct vertex_buffer *VB,
				  GLuint start,
				  GLuint count,
				  GLuint parity )
{
   GLuint j;
   LOCAL_VARS;
   (void) parity;
   INIT(GL_POLYGON);
   for (j=start+3; j<count; j+=4) {
      RENDER_QUAD( j-3, j-2, j-1, j, j );
      RESET_STIPPLE;
   }
   POSTFIX;
}

static void TAG(render_vb_quad_strip)( struct vertex_buffer *VB,
				       GLuint start,
				       GLuint count,
				       GLuint parity )
{
   GLuint j;
   LOCAL_VARS;
   (void) parity;
   INIT(GL_POLYGON);
   if (NEED_EDGEFLAG_SETUP) {
      for (j=start+3;j<count;j+=2) {
	 EDGEFLAG_QUAD( j-3, j-2, j, j-1, j );
	 RENDER_QUAD( j-3, j-2, j, j-1, j );
	 RESET_STIPPLE;
      }
   } else {
      for (j=start+3;j<count;j+=2) {
	 RENDER_QUAD( j-3, j-2, j, j-1, j );
      }
   }
   POSTFIX;
}

static void TAG(render_vb_noop)( struct vertex_buffer *VB,
				 GLuint start,
				 GLuint count,
				 GLuint parity )
{
   (void) VB;
   (void) start;
   (void) count;
   (void) parity;
}



static render_func TAG(render_tab)[GL_POLYGON+2] = {
   TAG(render_vb_points),
   TAG(render_vb_lines),
   TAG(render_vb_line_loop),
   TAG(render_vb_line_strip),
   TAG(render_vb_triangles),
   TAG(render_vb_tri_strip),
   TAG(render_vb_tri_fan),
   TAG(render_vb_quads),
   TAG(render_vb_quad_strip),
   TAG(render_vb_poly),
   TAG(render_vb_noop)
};

static void TAG(render_init)( void )
{
}


#ifndef PRESERVE_VB_DEFS
#undef RENDER_TRI
#undef RENDER_QUAD
#undef RENDER_LINE
#undef RENDER_POINTS
#undef LOCAL_VARS
#undef INIT
#undef POSTFIX
#undef RESET_STIPPLE
#endif

#ifndef PRESERVE_TAG
#undef TAG
#endif

#undef PRESERVE_VB_DEFS
#undef PRESERVE_TAG

