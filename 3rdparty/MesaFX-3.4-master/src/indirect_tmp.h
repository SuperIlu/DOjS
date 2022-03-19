/* $Id: indirect_tmp.h,v 1.1.1.1 1999/08/19 00:55:41 jtg Exp $ */

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


#define DO_CULL 0x1
#define DO_OFFSET 0x2
#define DO_LIGHT_TWOSIDE 0x4
#define DO_UNFILLED 0x8
#define HAVE_CULL_MASK 0x10


static void TAG(render_triangle)( GLcontext *ctx,
				  GLuint v0, GLuint v1, GLuint v2, GLuint pv )
{
   struct vertex_buffer *VB = ctx->VB;

#if ((IND & NEED_FACING) && !(IND & HAVE_CULL_MASK)) || (IND & DO_OFFSET)
   GLfloat (*win)[4] = VB->Win.data;
   GLfloat ex = win[v1][0] - win[v0][0];
   GLfloat ey = win[v1][1] - win[v0][1];
   GLfloat fx = win[v2][0] - win[v0][0];
   GLfloat fy = win[v2][1] - win[v0][1];
   GLfloat c = ex*fy-ey*fx;
#endif

#if (IND & NEED_FACING) 
#if (IND & HAVE_CULL_MASK)
   GLuint facing = ((VB->CullMask[pv]&PRIM_FACE_REAR)?1:0);
#else
   GLuint facing = (c<0.0F) ^ (ctx->Polygon.FrontFace==GL_CW);
#endif
#endif

#if (IND & DO_CULL)
   if ((facing+1) & ctx->Polygon.CullBits)
      return;
#endif

#if (IND & DO_OFFSET)
   {
      /* finish computing plane equation of polygon, compute offset */
      GLfloat fz = win[v2][2] - win[v0][2];
      GLfloat ez = win[v1][2] - win[v0][2];
      GLfloat a = ey*fz-ez*fy;
      GLfloat b = ez*fx-ex*fz;
      offset_polygon( ctx, a, b, c );
   }
#endif

#if (IND & DO_LIGHT_TWOSIDE)
   VB->Specular = VB->Spec[facing];
   VB->ColorPtr = VB->Color[facing];
   VB->IndexPtr = VB->Index[facing];
#endif


#if (IND & DO_UNFILLED)
   {
      GLuint vlist[3];
      vlist[0] = v0;
      vlist[1] = v1;
      vlist[2] = v2;
      unfilled_polygon( ctx, 3, vlist, pv, facing );
   }
#else
   ctx->Driver.TriangleFunc( ctx, v0, v1, v2, pv );
#endif
}



/* Implements quad rendering when direct_triangles is false.
 *
 * Need to implement cull check ?
 */
static void render_quad( GLcontext *ctx, GLuint v0, GLuint v1,
                         GLuint v2, GLuint v3, GLuint pv )
{
   struct vertex_buffer *VB = ctx->VB;

   GLuint facing = ((VB->CullMask[pv]&PRIM_FACE_REAR)?1:0);


   if (ctx->IndirectTriangles & DD_TRI_OFFSET) {
      GLfloat (*win)[4] = VB->Win.data;
      GLfloat ez = win[v2][2] - win[v0][2];
      GLfloat fz = win[v3][2] - win[v1][2];
      GLfloat ex = win[v2][0] - win[v0][0];
      GLfloat ey = win[v2][1] - win[v0][1];
      GLfloat fx = win[v3][0] - win[v1][0];
      GLfloat fy = win[v3][1] - win[v1][1];
      GLfloat a = ey*fz-ez*fy;
      GLfloat b = ez*fx-ex*fz;
      GLfloat c = ex*fy-ey*fx;
      offset_polygon( ctx, a, b, c );
   }


   if (ctx->IndirectTriangles & DD_TRI_LIGHT_TWOSIDE) {
      VB->Specular = VB->Spec[facing];
      VB->ColorPtr = VB->Color[facing];
      VB->IndexPtr = VB->Index[facing];
   }


   /* Render the quad! */
   if (ctx->Polygon.Unfilled) {
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
}


void unfilled_triangle( GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, 
			GLuint pv )
{
   GLuint vlist[4];
   vlist[0] = v0;
   vlist[1] = v1;
   vlist[2] = v2;
   unfilled_polygon( ctx, 3, vlist, pv, facing );
}


void unfilled_quad( GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, 
		    GLuint v3, GLuint pv )
{
   GLuint vlist[4];
   vlist[0] = v0;
   vlist[1] = v1;
   vlist[2] = v2;
   vlist[3] = v3;
   unfilled_polygon( ctx, 4, vlist, pv, facing );
}

