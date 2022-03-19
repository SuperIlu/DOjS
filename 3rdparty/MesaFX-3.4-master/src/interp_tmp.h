/* $Id: interp_tmp.h,v 1.3.4.1 2001/05/01 22:00:46 brianp Exp $ */

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

#undef INTERP_UBYTE
#define INTERP_UBYTE( out, t, a, b ) {			\
      GLfloat fa = UBYTE_COLOR_TO_FLOAT_COLOR(a);	\
      GLfloat fb = UBYTE_COLOR_TO_FLOAT_COLOR(b);	\
      GLfloat fo = LINTERP(t, fa, fb);			\
      FLOAT_COLOR_TO_UBYTE_COLOR(out, fo);		\
}

#if 1

#undef INTERP_RGBA
#define INTERP_RGBA(nr, t, out, a, b) {			\
   int i;						\
   for (i = 0; i < nr; i++) {				\
      GLfloat fa = UBYTE_COLOR_TO_FLOAT_COLOR(a[i]);	\
      GLfloat fb = UBYTE_COLOR_TO_FLOAT_COLOR(b[i]);	\
      GLfloat fo = LINTERP(t, fa, fb);			\
      FLOAT_COLOR_TO_UBYTE_COLOR(out[i], fo);		\
   }							\
}
#else

#undef INTERP_RGBA
#define INTERP_RGBA(nr, t, out, a, b) {				\
   int n;							\
   const GLuint ti = FloatToInt(t*256.0F);			\
   const GLubyte *Ib = (const GLubyte *)&a[0];			\
   const GLubyte *Jb = (const GLubyte *)&b[0];			\
   GLubyte *Ob = (GLubyte *)&out[0];				\
								\
   for (n = 0 ; n < nr ; n++)					\
      Ob[n] = (GLubyte) (Ib[n] + ((ti * (Jb[n] - Ib[n]))/256));	\
}
#endif



static void NAME( struct vertex_buffer *VB,
		  GLuint dst, GLfloat t, GLuint in, GLuint out )
{
   (void) VB;
   (void) dst;
   (void) t;
   (void) in;
   (void) out;

#if (IND & CLIP_RGBA0)
   INTERP_RGBA( 4, t, 
		VB->Color[0]->data[dst], 
		VB->Color[0]->data[in],
		VB->Color[0]->data[out] );
#endif

#if (IND & CLIP_RGBA1)
   if (VB->ctx->TriangleCaps & DD_TRI_LIGHT_TWOSIDE) {
      INTERP_RGBA( 4, t, 
		   VB->Color[1]->data[dst], 
		   VB->Color[1]->data[in],
		   VB->Color[1]->data[out] );
   }

   if (VB->ctx->TriangleCaps & DD_SEPERATE_SPECULAR) 
   {
      INTERP_RGBA( 3, t, 
		   VB->Spec[0][dst], 
		   VB->Spec[0][in],
		   VB->Spec[0][out] );

      if (VB->ctx->TriangleCaps & DD_TRI_LIGHT_TWOSIDE) {
	 INTERP_RGBA( 3, t, 
		      VB->Spec[1][dst], 
		      VB->Spec[1][in],
		      VB->Spec[1][out] );
      }
   }      
#endif

#if (IND & CLIP_FOG_COORD)
   {
      GLubyte a = VB->Spec[0][in][3], b = VB->Spec[0][out][3];
      INTERP_UBYTE( VB->Spec[0][dst][3], t, a, b );
   }
#endif


#if (IND & CLIP_INDEX0)
   VB->IndexPtr->data[dst] = (GLuint) (GLint) 
      LINTERP( t, 
	       (GLfloat) VB->Index[0]->data[in],
	       (GLfloat) VB->Index[0]->data[out] );
#endif

#if (IND & CLIP_INDEX1)
   VB->Index[1]->data[dst] = (GLuint) (GLint) 
      LINTERP( t, 
	       (GLfloat) VB->Index[1]->data[in],
	       (GLfloat) VB->Index[1]->data[out] );
#endif

#if (IND & CLIP_TEX0)
   INTERP_SZ( t, 
	      VB->TexCoordPtr[0]->data, 
	      dst, in, out, 
	      VB->TexCoordPtr[0]->size );
#endif

#if (IND & CLIP_TEX1)
   INTERP_SZ( t, 
	      VB->TexCoordPtr[1]->data, 
	      dst, in, out, 
	      VB->TexCoordPtr[1]->size );
#endif
}


#undef IND
#undef NAME
