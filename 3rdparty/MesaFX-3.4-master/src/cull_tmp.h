/* $Id: cull_tmp.h,v 1.1.1.1.6.1 2000/11/05 21:24:00 brianp Exp $ */

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


static GLuint TAG(gl_cull_triangles)( struct vertex_buffer *VB,
				      GLuint start, 
				      GLuint count, 
				      GLuint parity, 
				      CONST GLfloat (*proj)[4])
{
   const GLcontext *ctx = VB->ctx;
   const GLubyte face_bit = ctx->Polygon.FrontBit;
   const GLubyte cull_faces = ctx->Polygon.CullBits;
   GLubyte *cullmask = VB->CullMask;
   GLint i, cullcount = 0;
   GLint last = count - 3;
   
   (void) parity;
   for (i=start; i <= last ; i+=3 ) {
      CULL_TRI(DO_CLIP, DO_AREA, i, i+1, i+2, face_bit, 3);
   }

   if (i != (GLint) count)
      cullcount += count - i;

   return cullcount;
}

			  




static GLuint TAG(gl_cull_triangle_fan)( struct vertex_buffer *VB,
					 GLuint start, 
					 GLuint count, 
					 GLuint parity, 
					 CONST GLfloat (*proj)[4])
{
   const GLcontext *ctx = VB->ctx;
   const GLubyte face_bit = ctx->Polygon.FrontBit;
   const GLubyte cull_faces = ctx->Polygon.CullBits;
   GLubyte *cullmask = VB->CullMask;
   GLint cullcount = 0;
   GLint i;
   GLint last = count - 3;
   GLint nr = 3;

   (void) parity;
   for (i=start; i <= last ; i++, nr = 1) {
      CULL_TRI(DO_CLIP, DO_AREA, start, i+1, i+2, face_bit, nr);
   }

   if (i != (GLint) (last + 1))
      cullcount += count - i;

   return cullcount;
}



static GLuint TAG(gl_cull_triangle_strip)( struct vertex_buffer *VB,
					   GLuint start, 
					   GLuint count, 
					   GLuint parity, 
					   CONST GLfloat (*proj)[4])
{
   const GLcontext *ctx = VB->ctx;
   const GLubyte face_bit = ctx->Polygon.FrontBit;
   const GLubyte cull_faces = ctx->Polygon.CullBits;
   GLubyte *cullmask = VB->CullMask;
   GLint i;
   GLint cullcount = 0;
   GLint last = count - 3;
   GLint nr = 2;

   parity ^= face_bit;

   for (i = start; i <= last ; i++, parity ^= 1, nr = 1 ) {
      CULL_TRI(DO_CLIP, DO_AREA, i, i+1, i+2, parity, nr);
   }

   if (i != last + 1)
      cullcount += count - i;

   return cullcount;
}





static GLuint TAG(gl_cull_quads)( struct vertex_buffer *VB,
				  GLuint start, 
				  GLuint count, 
				  GLuint parity, 
				  CONST GLfloat (*proj)[4])
{
   const GLcontext *ctx = VB->ctx;
   const GLubyte face_bit = ctx->Polygon.FrontBit;
   const GLubyte cull_faces = ctx->Polygon.CullBits;
   GLubyte *cullmask = VB->CullMask;
   GLint i,cullcount = 0;
   GLint last = count - 4;

   (void) parity;

   for (i = start; i <= last ; i += 4) {
      CULL_QUAD(DO_CLIP, DO_AREA, i, i+1, i+2, i+3, 4);
   }

   if (i != (GLint) count)
      cullcount += count - i;

   return cullcount;
}


static GLuint TAG(gl_cull_quad_strip)( struct vertex_buffer *VB,
				       GLuint start, 
				       GLuint count, 
				       GLuint parity, 
				       CONST GLfloat (*proj)[4])
{
   const GLcontext *ctx = VB->ctx;
   const GLubyte face_bit = ctx->Polygon.FrontBit;
   const GLubyte cull_faces = ctx->Polygon.CullBits;
   GLubyte *cullmask = VB->CullMask;
   GLint i,cullcount = 0;
   GLint last = count - 4;
   GLint nr = 4;

   (void) parity;

   for (i = start ; i <= last ; i += 2, nr = 2) {
      CULL_QUAD(DO_CLIP, DO_AREA, i, i+1, i+3, i+2, nr);
   }

   if (i != ((GLint) last + 2))
      cullcount += count - i;

   return cullcount;
}




#undef DO_CLIP
#undef DO_AREA
#undef TAG
