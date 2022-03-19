/* $Id: vbfill.c,v 1.10 2000/07/12 12:10:41 keithw Exp $ */

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


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "context.h"
#include "enums.h"
#include "light.h"
#include "macros.h"
#include "matrix.h"
#include "mmath.h"
#include "state.h"
#include "types.h"
#include "varray.h"
#include "vb.h"
#include "vbcull.h"
#include "vbfill.h"
#include "vbrender.h"
#include "vbxform.h"
#include "xform.h"
#endif



/* KW: The work of most of the functions which were in this file are
 *     now done directly by the GLVertex, GLTexCoord, GLIndex, ...,
 *     functions in api{1,2}.c.  This is made possible by the fact
 *     that display lists and vertex buffers are now built the same
 *     way, so there is no need for the indirection and overhead of a
 *     function pointer.  
 */


void
_mesa_Begin( GLenum mode )
{
   GET_CURRENT_CONTEXT(ctx);

   if (mode > GL_POLYGON) {
      gl_compile_error( ctx, GL_INVALID_ENUM, "glBegin" );
      return;		     
   }

   gl_Begin(ctx, mode);
}


void
gl_Begin( GLcontext *ctx, GLenum p )
{
   struct immediate *IM = ctx->input;
   GLuint inflags, state;

   if (MESA_VERBOSE&VERBOSE_API)
      fprintf(stderr, "glBegin(IM %d) %s\n", IM->id, gl_lookup_enum_by_nr(p));
   
   if (ctx->NewState) 
      gl_update_state( ctx );	/* should already be flushed */
       
   /* if only a very few slots left, might as well flush now
    */
   if (IM->Count > VB_MAX-4) {	      
      IM->maybe_transform_vb( IM );
      IM = ctx->input;
   }

   state = IM->BeginState;
   inflags = state & (VERT_BEGIN_0|VERT_BEGIN_1);
   state |= inflags << 2;	/* set error conditions */

   if (MESA_VERBOSE&VERBOSE_API)
      fprintf(stderr, "in gl_Begin(IM %d), BeginState is %x, errors %x", 
	      IM->id,
	      state,
	      inflags<<2);

   if (inflags != (VERT_BEGIN_0|VERT_BEGIN_1))
   {
      GLuint count = IM->Count;
      GLuint last = IM->LastPrimitive;

      state |= (VERT_BEGIN_0|VERT_BEGIN_1);
      IM->Flag[count] |= VERT_BEGIN; 
      IM->Primitive[count] = p;

      IM->NextPrimitive[IM->LastPrimitive] = count;
      IM->LastPrimitive = count;

      if (IM->FlushElt) {
	 gl_exec_array_elements( ctx, IM, last, count );
	 IM->FlushElt = 0;
      }
   }

   if (MESA_VERBOSE&VERBOSE_API)
      fprintf(stderr, "in gl_Begin final state %x\n", state);

   IM->BeginState = state;
}



/* KW: Both streams now go to the outside-begin-end state.  Raise
 *     errors for either stream if it was not in the inside state.
 */
void
_mesa_End(void)
{
   GLuint state;
   GLuint inflags;
   GET_IMMEDIATE;


   state = IM->BeginState;
   inflags = (~state) & (VERT_BEGIN_0|VERT_BEGIN_1);
   state |= inflags << 2;	/* errors */

   if (MESA_VERBOSE&VERBOSE_API)
      fprintf(stderr, "glEnd(IM %d), BeginState is %x, errors %x\n", 
	      IM->id, state,
	      inflags<<2);

   
   if (inflags != (VERT_BEGIN_0|VERT_BEGIN_1))
   {
      GLuint count = IM->Count;
      GLuint last = IM->LastPrimitive;

      state &= ~(VERT_BEGIN_0|VERT_BEGIN_1); /* update state */
      IM->Flag[count] |= VERT_END;
      IM->NextPrimitive[IM->LastPrimitive] = count;
      IM->LastPrimitive = count;
      IM->Primitive[count] = GL_POLYGON+1;

      if (IM->FlushElt) {
	 gl_exec_array_elements( IM->backref, IM, last, count );
	 IM->FlushElt = 0;
      }
   }
   
   if (MESA_VERBOSE&VERBOSE_API)
      fprintf(stderr, "in glEnd final state %x\n", state);

   IM->BeginState = state;      

   if ((MESA_DEBUG_FLAGS&DEBUG_ALWAYS_FLUSH))
      IM->maybe_transform_vb( IM );
}


void
gl_End( GLcontext *ctx )
{
   struct immediate *IM = ctx->input;
   GLuint state = IM->BeginState;
   GLuint inflags = (~state) & (VERT_BEGIN_0|VERT_BEGIN_1);

   state |= inflags << 2;	/* errors */

   if (inflags != (VERT_BEGIN_0|VERT_BEGIN_1))
   {
      GLuint count = IM->Count;
      GLuint last = IM->LastPrimitive;

      state &= ~(VERT_BEGIN_0|VERT_BEGIN_1); /* update state */
      IM->Flag[count] |= VERT_END;
      IM->NextPrimitive[IM->LastPrimitive] = count;
      IM->LastPrimitive = count;
      IM->Primitive[count] = GL_POLYGON+1;

      if (IM->FlushElt) {
	 gl_exec_array_elements( ctx, IM, last, count );
	 IM->FlushElt = 0;
      }
   }

   IM->BeginState = state;      

   /* You can set this flag to get the old 'flush vb on glEnd()'
    * behaviour.
    */
   if ((MESA_DEBUG_FLAGS&DEBUG_ALWAYS_FLUSH))
      IM->maybe_transform_vb( IM );
}





/* KW: Again, a stateless implementation of these functions.  The big
 * news here is the impact on color material.  This was previously
 * handled by swaping the function pointers that these API's used to
 * call.  This is no longer possible, and we have to pick up the
 * pieces later on and make them work with either color-color, or
 * color-material.
 *
 * But in truth, this is not a bad thing, because it was necessary
 * to implement that mechanism to get good performance from
 * color-material and vertex arrays.  
 */
#define COLOR( IM, r, g, b, a )			\
{						\
   GLuint count = IM->Count;			\
   IM->Flag[count] |= VERT_RGBA;		\
   IM->Color[count][0] = r;			\
   IM->Color[count][1] = g;			\
   IM->Color[count][2] = b;			\
   IM->Color[count][3] = a;			\
}

#define COLORV( IM, v )				\
{						\
   GLuint count = IM->Count;			\
   IM->Flag[count] |= VERT_RGBA;		\
   COPY_4UBV(IM->Color[count], v);		\
}


void
_mesa_Color3b( GLbyte red, GLbyte green, GLbyte blue )
{
   GET_IMMEDIATE;
   COLOR( IM, BYTE_TO_UBYTE(red), BYTE_TO_UBYTE(green),
          BYTE_TO_UBYTE(blue), 255 );
}


void
_mesa_Color3d( GLdouble red, GLdouble green, GLdouble blue )
{
   GLubyte col[4];
   GLfloat r = red;
   GLfloat g = green;
   GLfloat b = blue;
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], r);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], g);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], b);
   col[3] = 255;
   COLORV( IM, col );
}


void
_mesa_Color3f( GLfloat red, GLfloat green, GLfloat blue )
{
   GLubyte col[4];
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], red);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], green);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], blue);
   col[3] = 255;
   COLORV( IM, col );
}


void
_mesa_Color3i( GLint red, GLint green, GLint blue )
{
   GET_IMMEDIATE;
   COLOR( IM, INT_TO_UBYTE(red), INT_TO_UBYTE(green),
	  INT_TO_UBYTE(blue), 255);
}


void
_mesa_Color3s( GLshort red, GLshort green, GLshort blue )
{
   GET_IMMEDIATE;
   COLOR( IM, SHORT_TO_UBYTE(red), SHORT_TO_UBYTE(green),
	  SHORT_TO_UBYTE(blue), 255);
}


void
_mesa_Color3ub( GLubyte red, GLubyte green, GLubyte blue )
{
   GET_IMMEDIATE;
   COLOR( IM, red, green, blue, 255 );
}


void
_mesa_Color3ui( GLuint red, GLuint green, GLuint blue )
{
   GET_IMMEDIATE;
   COLOR( IM, UINT_TO_UBYTE(red), UINT_TO_UBYTE(green),
	  UINT_TO_UBYTE(blue), 255 );
}


void
_mesa_Color3us( GLushort red, GLushort green, GLushort blue )
{
   GET_IMMEDIATE;
   COLOR( IM, USHORT_TO_UBYTE(red), USHORT_TO_UBYTE(green),
	  USHORT_TO_UBYTE(blue), 255 );
}


void
_mesa_Color4b( GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha )
{
   GET_IMMEDIATE;
   COLOR( IM, BYTE_TO_UBYTE(red), BYTE_TO_UBYTE(green),
	  BYTE_TO_UBYTE(blue), BYTE_TO_UBYTE(alpha) );
}


void
_mesa_Color4d( GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha )
{
   GLubyte col[4];
   GLfloat r = red;
   GLfloat g = green;
   GLfloat b = blue;
   GLfloat a = alpha;
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], r);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], g);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], b);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[3], a);
   COLORV( IM, col );
}


void
_mesa_Color4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
   GLubyte col[4];
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], red);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], green);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], blue);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[3], alpha);
   COLORV( IM, col );
}


void
_mesa_Color4i( GLint red, GLint green, GLint blue, GLint alpha )
{
   GET_IMMEDIATE;
   COLOR( IM, INT_TO_UBYTE(red), INT_TO_UBYTE(green),
	  INT_TO_UBYTE(blue), INT_TO_UBYTE(alpha) );
}


void
_mesa_Color4s( GLshort red, GLshort green, GLshort blue, GLshort alpha )
{
   GET_IMMEDIATE;
   COLOR( IM, SHORT_TO_UBYTE(red), SHORT_TO_UBYTE(green),
	  SHORT_TO_UBYTE(blue), SHORT_TO_UBYTE(alpha) );
}

void
_mesa_Color4ub( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha )
{
   GET_IMMEDIATE;
   COLOR( IM, red, green, blue, alpha );
}

void
_mesa_Color4ui( GLuint red, GLuint green, GLuint blue, GLuint alpha )
{
   GET_IMMEDIATE;
   COLOR( IM, UINT_TO_UBYTE(red), UINT_TO_UBYTE(green),
	  UINT_TO_UBYTE(blue), UINT_TO_UBYTE(alpha) );
}

void
_mesa_Color4us( GLushort red, GLushort green, GLushort blue, GLushort alpha )
{
   GET_IMMEDIATE;
   COLOR( IM, USHORT_TO_UBYTE(red), USHORT_TO_UBYTE(green),
	  USHORT_TO_UBYTE(blue), USHORT_TO_UBYTE(alpha) );
}


void
_mesa_Color3bv( const GLbyte *v )
{
   GET_IMMEDIATE;
   COLOR( IM, BYTE_TO_UBYTE(v[0]), BYTE_TO_UBYTE(v[1]),
	  BYTE_TO_UBYTE(v[2]), 255 );
}


void
_mesa_Color3dv( const GLdouble *v )
{
   GLubyte col[4];
   GLfloat r = v[0];
   GLfloat g = v[1];
   GLfloat b = v[2];
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], r);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], g);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], b);
   col[3]= 255;
   COLORV( IM, col );
}


void
_mesa_Color3fv( const GLfloat *v )
{
   GLubyte col[4];
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], v[0]);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], v[1]);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], v[2]);
   col[3] = 255;
   COLORV( IM, col );
}


void
_mesa_Color3iv( const GLint *v )
{
   GET_IMMEDIATE;
   COLOR( IM, INT_TO_UBYTE(v[0]), INT_TO_UBYTE(v[1]),
	  INT_TO_UBYTE(v[2]), 255 );
}


void
_mesa_Color3sv( const GLshort *v )
{
   GET_IMMEDIATE;
   COLOR( IM, SHORT_TO_UBYTE(v[0]), SHORT_TO_UBYTE(v[1]),
	  SHORT_TO_UBYTE(v[2]), 255 );
}


void
_mesa_Color3ubv( const GLubyte *v )
{
   GET_IMMEDIATE;
   COLOR( IM, v[0], v[1], v[2], 255 );
}


void
_mesa_Color3uiv( const GLuint *v )
{
   GET_IMMEDIATE;
   COLOR( IM, UINT_TO_UBYTE(v[0]), UINT_TO_UBYTE(v[1]),
	  UINT_TO_UBYTE(v[2]), 255 );
}


void
_mesa_Color3usv( const GLushort *v )
{
   GET_IMMEDIATE;
   COLOR( IM, USHORT_TO_UBYTE(v[0]), USHORT_TO_UBYTE(v[1]),
	  USHORT_TO_UBYTE(v[2]), 255 );

}


void
_mesa_Color4bv( const GLbyte *v )
{
   GET_IMMEDIATE;
   COLOR( IM, BYTE_TO_UBYTE(v[0]), BYTE_TO_UBYTE(v[1]),
	  BYTE_TO_UBYTE(v[2]), BYTE_TO_UBYTE(v[3]) );
}


void
_mesa_Color4dv( const GLdouble *v )
{
   GLubyte col[4];
   GLfloat r = v[0];
   GLfloat g = v[1];
   GLfloat b = v[2];
   GLfloat a = v[3];
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], r);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], g);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], b);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[3], a);
   COLORV( IM, col );
}


void
_mesa_Color4fv( const GLfloat *v )
{
   GLubyte col[4];
   GET_IMMEDIATE;
   FLOAT_COLOR_TO_UBYTE_COLOR(col[0], v[0]);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[1], v[1]);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[2], v[2]);
   FLOAT_COLOR_TO_UBYTE_COLOR(col[3], v[3]);
   COLORV( IM, col );
}


void
_mesa_Color4iv( const GLint *v )
{
   GET_IMMEDIATE;
   COLOR( IM, INT_TO_UBYTE(v[0]), INT_TO_UBYTE(v[1]),
	  INT_TO_UBYTE(v[2]), INT_TO_UBYTE(v[3]) );
}


void
_mesa_Color4sv( const GLshort *v)
{
   GET_IMMEDIATE;
   COLOR( IM, SHORT_TO_UBYTE(v[0]), SHORT_TO_UBYTE(v[1]),
	  SHORT_TO_UBYTE(v[2]), SHORT_TO_UBYTE(v[3]) );
}


void
_mesa_Color4ubv( const GLubyte *v)
{
   GET_IMMEDIATE;
   COLORV( IM, v );
}


void
_mesa_Color4uiv( const GLuint *v)
{
   GET_IMMEDIATE;
   COLOR( IM, UINT_TO_UBYTE(v[0]), UINT_TO_UBYTE(v[1]),
	  UINT_TO_UBYTE(v[2]), UINT_TO_UBYTE(v[3]) );
}


void
_mesa_Color4usv( const GLushort *v)
{
   GET_IMMEDIATE;
   COLOR( IM, USHORT_TO_UBYTE(v[0]), USHORT_TO_UBYTE(v[1]),
	  USHORT_TO_UBYTE(v[2]), USHORT_TO_UBYTE(v[3]) );
}




void
_mesa_EdgeFlag( GLboolean flag )
{
   GLuint count;
   GET_IMMEDIATE;
   count = IM->Count;
   IM->EdgeFlag[count] = flag;
   IM->Flag[count] |= VERT_EDGE;
}


void
_mesa_EdgeFlagv( const GLboolean *flag )
{
   GLuint count;
   GET_IMMEDIATE;
   count = IM->Count;
   IM->EdgeFlag[count] = *flag;
   IM->Flag[count] |= VERT_EDGE;
}



#define INDEX( c )				\
{						\
   GLuint count;				\
   GET_IMMEDIATE;				\
   count = IM->Count;				\
   IM->Index[count] = c;			\
   IM->Flag[count] |= VERT_INDEX;		\
}


void
_mesa_Indexd( GLdouble c )
{
   INDEX( ( GLuint) (GLint) c );
}


void
_mesa_Indexf( GLfloat c )
{
   INDEX( (GLuint) (GLint) c );
}


void
_mesa_Indexi( GLint c )
{
   INDEX( (GLuint) c );
}


void
_mesa_Indexs( GLshort c )
{
   INDEX( (GLuint) c );
}


void
_mesa_Indexub( GLubyte c )
{
   INDEX( (GLuint) c );
}


void
_mesa_Indexdv( const GLdouble *c )
{
   INDEX( (GLuint) (GLint) *c );
}


void
_mesa_Indexfv( const GLfloat *c )
{
   INDEX( (GLuint) (GLint) *c );
}


void
_mesa_Indexiv( const GLint *c )
{
   INDEX( *c );
}


void
_mesa_Indexsv( const GLshort *c )
{
   INDEX( (GLuint) (GLint) *c );
}


void
_mesa_Indexubv( const GLubyte *c )
{
   INDEX( (GLuint) *c );
}





/* KW: Now that we build buffers for display lists the same way we
 *     fill the vb, we can do the work here without a second function
 *     call.  The Flag member allows the identification of missing
 *     (ie shared) normals.
 */
#define NORMAL( x, y, z )			\
{						\
   GLuint count;				\
   GLfloat *normal;				\
   GET_IMMEDIATE;				\
   count = IM->Count;				\
   IM->Flag[count] |= VERT_NORM;		\
   normal = IM->Normal[count];			\
   ASSIGN_3V(normal, x,y,z);			\
}

#if defined(USE_IEEE)
#define NORMALF( x, y, z )					\
{								\
   GLuint count;						\
   GLint *normal;						\
   GET_IMMEDIATE;						\
   count = IM->Count;						\
   IM->Flag[count] |= VERT_NORM;				\
   normal = (__GLint_a *)IM->Normal[count];				\
   ASSIGN_3V(normal, *(__GLint_a*)&(x), *(__GLint_a*)&(y), *(__GLint_a*)&(z));	\
}
#else
#define NORMALF NORMAL
#endif

void
_mesa_Normal3b( GLbyte nx, GLbyte ny, GLbyte nz )
{
   NORMAL( BYTE_TO_FLOAT(nx), BYTE_TO_FLOAT(ny), BYTE_TO_FLOAT(nz) );
}


void
_mesa_Normal3d( GLdouble nx, GLdouble ny, GLdouble nz )
{
   NORMAL(nx, ny, nz);
}


void
_mesa_Normal3f( GLfloat nx, GLfloat ny, GLfloat nz )
{
   NORMALF(nx, ny, nz);
}


void
_mesa_Normal3i( GLint nx, GLint ny, GLint nz )
{
   NORMAL( INT_TO_FLOAT(nx), INT_TO_FLOAT(ny), INT_TO_FLOAT(nz) );
}


void
_mesa_Normal3s( GLshort nx, GLshort ny, GLshort nz )
{
   NORMAL( SHORT_TO_FLOAT(nx), SHORT_TO_FLOAT(ny), SHORT_TO_FLOAT(nz) );
}


void
_mesa_Normal3bv( const GLbyte *v )
{
   NORMAL( BYTE_TO_FLOAT(v[0]), BYTE_TO_FLOAT(v[1]), BYTE_TO_FLOAT(v[2]) );
}


void
_mesa_Normal3dv( const GLdouble *v )
{
   NORMAL( v[0], v[1], v[2] );
}


void
_mesa_Normal3fv( const GLfloat *v )
{
   NORMALF( v[0], v[1], v[2] );
}


void
_mesa_Normal3iv( const GLint *v )
{
   NORMAL( INT_TO_FLOAT(v[0]), INT_TO_FLOAT(v[1]), INT_TO_FLOAT(v[2]) );
}


void
_mesa_Normal3sv( const GLshort *v )
{
   NORMAL( SHORT_TO_FLOAT(v[0]), SHORT_TO_FLOAT(v[1]), SHORT_TO_FLOAT(v[2]) );
}





#define TEXCOORD1(s)				\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   GET_IMMEDIATE;				\
   count = IM->Count;				\
   IM->Flag[count] |= VERT_TEX0_1;		\
   tc = IM->TexCoord[0][count];			\
   ASSIGN_4V(tc,s,0,0,1);			\
}

#define TEXCOORD2(s,t)				\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   GET_IMMEDIATE;				\
   count = IM->Count;				\
   IM->Flag[count] |= VERT_TEX0_12;		\
   tc = IM->TexCoord[0][count];			\
   ASSIGN_4V(tc, s,t,0,1);		        \
}

#define TEXCOORD3(s,t,u)			\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   GET_IMMEDIATE;				\
   count = IM->Count;				\
   IM->Flag[count] |= VERT_TEX0_123;		\
   tc = IM->TexCoord[0][count];			\
   ASSIGN_4V(tc, s,t,u,1);			\
}

#define TEXCOORD4(s,t,u,v)			\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   GET_IMMEDIATE;				\
   count = IM->Count;				\
   IM->Flag[count] |= VERT_TEX0_1234;		\
   tc = IM->TexCoord[0][count];			\
   ASSIGN_4V(tc, s,t,u,v);			\
}

#if defined(USE_IEEE)
#define TEXCOORD2F(s,t)				\
{						\
   GLuint count;				\
   GLint *tc;					\
   GET_IMMEDIATE;				\
   count = IM->Count;				\
   IM->Flag[count] |= VERT_TEX0_12;		\
   tc = (__GLint_a *)IM->TexCoord[0][count];	\
   *tc = *(__GLint_a *)&(s);				\
   *(tc+1) = *(__GLint_a *)&(t);			\
   *(tc+2) = 0;					\
   *(tc+3) = IEEE_ONE;				\
}
#else
#define TEXCOORD2F TEXCOORD2
#endif

void
_mesa_TexCoord1d( GLdouble s )
{
   TEXCOORD1(s);
}


void
_mesa_TexCoord1f( GLfloat s )
{
   TEXCOORD1(s);
}


void
_mesa_TexCoord1i( GLint s )
{
   TEXCOORD1(s);
}


void
_mesa_TexCoord1s( GLshort s )
{
   TEXCOORD1(s);
}


void
_mesa_TexCoord2d( GLdouble s, GLdouble t )
{
   TEXCOORD2(s,t);
}


void
_mesa_TexCoord2f( GLfloat s, GLfloat t )
{
   TEXCOORD2F(s,t); 
}


void
_mesa_TexCoord2s( GLshort s, GLshort t )
{
   TEXCOORD2(s,t);
}


void
_mesa_TexCoord2i( GLint s, GLint t )
{
   TEXCOORD2(s,t);
}


void
_mesa_TexCoord3d( GLdouble s, GLdouble t, GLdouble r )
{
   TEXCOORD3(s,t,r);
}


void
_mesa_TexCoord3f( GLfloat s, GLfloat t, GLfloat r )
{
   TEXCOORD3(s,t,r);
}


void
_mesa_TexCoord3i( GLint s, GLint t, GLint r )
{
   TEXCOORD3(s,t,r);
}


void
_mesa_TexCoord3s( GLshort s, GLshort t, GLshort r )
{
   TEXCOORD3(s,t,r);
}


void
_mesa_TexCoord4d( GLdouble s, GLdouble t, GLdouble r, GLdouble q )
{
   TEXCOORD4(s,t,r,q)
}


void
_mesa_TexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
   TEXCOORD4(s,t,r,q)
}


void
_mesa_TexCoord4i( GLint s, GLint t, GLint r, GLint q )
{
   TEXCOORD4(s,t,r,q)
}


void
_mesa_TexCoord4s( GLshort s, GLshort t, GLshort r, GLshort q )
{
   TEXCOORD4(s,t,r,q)
}


void
_mesa_TexCoord1dv( const GLdouble *v )
{
   TEXCOORD1(v[0]);
}


void
_mesa_TexCoord1fv( const GLfloat *v )
{
   TEXCOORD1(v[0]);
}


void
_mesa_TexCoord1iv( const GLint *v )
{
   TEXCOORD1(v[0]);
}


void
_mesa_TexCoord1sv( const GLshort *v )
{
   TEXCOORD1(v[0]);
}


void
_mesa_TexCoord2dv( const GLdouble *v )
{
   TEXCOORD2(v[0],v[1]);
}


void
_mesa_TexCoord2fv( const GLfloat *v )
{
   TEXCOORD2F(v[0],v[1]);
}


void
_mesa_TexCoord2iv( const GLint *v )
{
   TEXCOORD2(v[0],v[1]);
}


void
_mesa_TexCoord2sv( const GLshort *v )
{
   TEXCOORD2(v[0],v[1]);
}


void
_mesa_TexCoord3dv( const GLdouble *v )
{
   TEXCOORD2(v[0],v[1]);
}


void
_mesa_TexCoord3fv( const GLfloat *v )
{
   TEXCOORD3(v[0],v[1],v[2]);
}


void
_mesa_TexCoord3iv( const GLint *v )
{
   TEXCOORD3(v[0],v[1],v[2]);
}


void
_mesa_TexCoord3sv( const GLshort *v )
{
   TEXCOORD3(v[0],v[1],v[2]);
}


void
_mesa_TexCoord4dv( const GLdouble *v )
{
   TEXCOORD4(v[0],v[1],v[2],v[3]);
}


void
_mesa_TexCoord4fv( const GLfloat *v )
{
   TEXCOORD4(v[0],v[1],v[2],v[3]);
}


void
_mesa_TexCoord4iv( const GLint *v )
{
   TEXCOORD4(v[0],v[1],v[2],v[3]);
}


void
_mesa_TexCoord4sv( const GLshort *v )
{
   TEXCOORD4(v[0],v[1],v[2],v[3]);
}





/* KW: Run into bad problems in reset_vb/fixup_input if we don't fully pad
 *     the incoming vertices.
 */
#define VERTEX2(IM, x,y)			\
{						\
   GLuint count = IM->Count++;			\
   GLfloat *dest = IM->Obj[count];		\
   IM->Flag[count] |= VERT_OBJ_2;		\
   ASSIGN_4V(dest, x, y, 0, 1);			\
   if (dest == IM->Obj[VB_MAX-1])		\
      IM->maybe_transform_vb( IM );		\
}

#define VERTEX3(IM,x,y,z)			\
{						\
   GLuint count = IM->Count++;			\
   GLfloat *dest = IM->Obj[count];		\
   IM->Flag[count] |= VERT_OBJ_23;		\
   ASSIGN_4V(dest, x, y, z, 1);			\
   if (dest == IM->Obj[VB_MAX-1])		\
      IM->maybe_transform_vb( IM );		\
}

#define VERTEX4(IM, x,y,z,w)			\
{						\
   GLuint count = IM->Count++;			\
   GLfloat *dest = IM->Obj[count];		\
   IM->Flag[count] |= VERT_OBJ_234;		\
   ASSIGN_4V(dest, x, y, z, w);			\
   if (dest == IM->Obj[VB_MAX-1])		\
      IM->maybe_transform_vb( IM );		\
}

#if defined(USE_IEEE)
#define VERTEX3F(IM,x,y,z)			\
{						\
   GLuint count = IM->Count++;			\
   __GLint_a *dest = (__GLint_a *)IM->Obj[count];	\
   IM->Flag[count] |= VERT_OBJ_23;		\
   dest[0] = *(__GLint_a *)&(x);			\
   dest[1] = *(__GLint_a *)&(y);			\
   dest[2] = *(__GLint_a *)&(z);			\
   dest[3] = IEEE_ONE;				\
   if (dest == (__GLint_a *)IM->Obj[VB_MAX-1])	\
      IM->maybe_transform_vb( IM );		\
}
#else
#define VERTEX3F VERTEX3
#endif

void
_mesa_Vertex2d( GLdouble x, GLdouble y )
{
   GET_IMMEDIATE;
   VERTEX2( IM, (GLfloat) x, (GLfloat) y );
}


void
_mesa_Vertex2f( GLfloat x, GLfloat y )
{
   GET_IMMEDIATE;
   VERTEX2( IM, *(&x), *(&y) );
}


/* Internal use:
 */
void
gl_Vertex2f( GLcontext *ctx, GLfloat x, GLfloat y )
{
   struct immediate *im = ctx->input;
   VERTEX2( im, x, y );
}


void
_mesa_Vertex2i( GLint x, GLint y )
{
   GET_IMMEDIATE;
   VERTEX2( IM, (GLfloat) x, (GLfloat) y );
}


void
_mesa_Vertex2s( GLshort x, GLshort y )
{
   GET_IMMEDIATE;
   VERTEX2( IM, (GLfloat) x, (GLfloat) y );
}


void
_mesa_Vertex3d( GLdouble x, GLdouble y, GLdouble z )
{
   GET_IMMEDIATE;
   VERTEX3( IM, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


void
_mesa_Vertex3f( GLfloat x, GLfloat y, GLfloat z )
{
   GET_IMMEDIATE;
   VERTEX3F( IM, x, y, z ); 
}


void
_mesa_Vertex3i( GLint x, GLint y, GLint z )
{
   GET_IMMEDIATE;
   VERTEX3( IM, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


void
_mesa_Vertex3s( GLshort x, GLshort y, GLshort z )
{
   GET_IMMEDIATE;
   VERTEX3( IM, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


void
_mesa_Vertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
   GET_IMMEDIATE;
   VERTEX4( IM, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}


void
_mesa_Vertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   GET_IMMEDIATE;
   VERTEX4( IM, x, y, z, w );
}


void
_mesa_Vertex4i( GLint x, GLint y, GLint z, GLint w )
{
   GET_IMMEDIATE;
   VERTEX4( IM, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}


void
_mesa_Vertex4s( GLshort x, GLshort y, GLshort z, GLshort w )
{
   GET_IMMEDIATE;
   VERTEX4( IM, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}


void
_mesa_Vertex2dv( const GLdouble *v )
{
   GET_IMMEDIATE;
   VERTEX2( IM, (GLfloat) v[0], (GLfloat) v[1] );
}


void
_mesa_Vertex2fv( const GLfloat *v )
{
   GET_IMMEDIATE;
   VERTEX2( IM, v[0], v[1] );
}


void
_mesa_Vertex2iv( const GLint *v )
{
   GET_IMMEDIATE;
   VERTEX2( IM, (GLfloat) v[0], (GLfloat) v[1] );
}


void
_mesa_Vertex2sv( const GLshort *v )
{
   GET_IMMEDIATE;
   VERTEX2( IM, (GLfloat) v[0], (GLfloat) v[1] );
}


void
_mesa_Vertex3dv( const GLdouble *v )
{
   GET_IMMEDIATE;
   VERTEX3( IM, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2] );
}


void
_mesa_Vertex3fv( const GLfloat *v )
{
   GET_IMMEDIATE;
   VERTEX3F( IM, v[0], v[1], v[2] );
}


void
_mesa_Vertex3iv( const GLint *v )
{
   GET_IMMEDIATE;
   VERTEX3( IM, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2] );
}


void
_mesa_Vertex3sv( const GLshort *v )
{
   GET_IMMEDIATE;
   VERTEX3( IM, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2] );
}


void
_mesa_Vertex4dv( const GLdouble *v )
{
   GET_IMMEDIATE;
   VERTEX4( IM, 
	    (GLfloat) v[0], (GLfloat) v[1], 
	    (GLfloat) v[2], (GLfloat) v[3] );
}


void
_mesa_Vertex4fv( const GLfloat *v )
{
   GET_IMMEDIATE;
   VERTEX4( IM, v[0], v[1], v[2], v[3] );
}


void
_mesa_Vertex4iv( const GLint *v )
{
   GET_IMMEDIATE;
   VERTEX4( IM, 
	    (GLfloat) v[0], (GLfloat) v[1], 
	    (GLfloat) v[2], (GLfloat) v[3] );
}


void
_mesa_Vertex4sv( const GLshort *v )
{
   GET_IMMEDIATE;
   VERTEX4( IM, 
	    (GLfloat) v[0], (GLfloat) v[1], 
	    (GLfloat) v[2], (GLfloat) v[3] );
}




/* KW: Do the check here so that we only have to do a single range
 *     test.  The possible compliance problem with this is that we
 *     will throw out error-producing calls when compiling display
 *     lists.  The solution is to do dispatch on gl_error to call
 *     gl_save_error if compiling.
 */


/*
 * GL_ARB_multitexture
 */

#define CHECK_ARB							\
   if (target >= GL_TEXTURE0_ARB && target <= GL_TEXTURE1_ARB) {	\
      texSet = target - GL_TEXTURE0_ARB;				\
   }									\
   else {								\
      gl_error(IM->backref, GL_INVALID_ENUM, 				\
               "glMultiTexCoord(target)");				\
      return;								\
   }


#define MULTI_TEXCOORD1(s)			\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   count = IM->Count;				\
   IM->Flag[count] |= IM->TF1[texSet];		\
   tc = IM->TexCoordPtr[texSet][count];		\
   ASSIGN_4V(tc, s,0,0,1);			\
}


#define MULTI_TEXCOORD2(s,t)			\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   count = IM->Count;				\
   IM->Flag[count] |= IM->TF2[texSet];		\
   tc = IM->TexCoordPtr[texSet][count];		\
   ASSIGN_4V(tc, s,t,0,1);			\
}

#define MULTI_TEXCOORD3(s,t,u)			\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   count = IM->Count;				\
   IM->Flag[count] |= IM->TF3[texSet];		\
   tc = IM->TexCoordPtr[texSet][count];		\
   ASSIGN_4V(tc, s,t,u,1);			\
}

#define MULTI_TEXCOORD4(s,t,u,v)		\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   count = IM->Count;				\
   IM->Flag[count] |= IM->TF4[texSet];		\
   tc = IM->TexCoordPtr[texSet][count];		\
   ASSIGN_4V(tc, s,t,u,v);			\
}

#if defined(USE_IEEE)
#define MULTI_TEXCOORD2F(s,t)			\
{						\
   GLuint count;				\
   GLint *tc;					\
   count = IM->Count;				\
   IM->Flag[count] |= IM->TF2[texSet];		\
   tc = (__GLint_a *)IM->TexCoord[texSet][count];	\
   tc[0] = *(__GLint_a *)&(s);			\
   tc[1] = *(__GLint_a *)&(t);			\
   tc[2] = 0;					\
   tc[3] = IEEE_ONE;				\
}
#else
#define MULTI_TEXCOORD2F MULTI_TEXCOORD2
#endif

void
_mesa_MultiTexCoord1dARB(GLenum target, GLdouble s)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD1( s );
}

void
_mesa_MultiTexCoord1dvARB(GLenum target, const GLdouble *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD1( v[0] );
}

void
_mesa_MultiTexCoord1fARB(GLenum target, GLfloat s)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD1( s );
}

void
_mesa_MultiTexCoord1fvARB(GLenum target, const GLfloat *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD1( v[0] );
}

void
_mesa_MultiTexCoord1iARB(GLenum target, GLint s)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD1( s );
}

void
_mesa_MultiTexCoord1ivARB(GLenum target, const GLint *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD1( v[0] );
}

void
_mesa_MultiTexCoord1sARB(GLenum target, GLshort s)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD1( s );
}

void
_mesa_MultiTexCoord1svARB(GLenum target, const GLshort *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD1( v[0] );
}

void
_mesa_MultiTexCoord2dARB(GLenum target, GLdouble s, GLdouble t)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD2( s, t );
}

void
_mesa_MultiTexCoord2dvARB(GLenum target, const GLdouble *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD2( v[0], v[1] );
}

void
_mesa_MultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD2F( s, t );
}

void
_mesa_MultiTexCoord2fvARB(GLenum target, const GLfloat *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD2F( v[0], v[1] );
}

void
_mesa_MultiTexCoord2iARB(GLenum target, GLint s, GLint t)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD2( s, t );
}

void
_mesa_MultiTexCoord2ivARB(GLenum target, const GLint *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD2( v[0], v[1] );
}

void
_mesa_MultiTexCoord2sARB(GLenum target, GLshort s, GLshort t)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD2( s, t );
}

void
_mesa_MultiTexCoord2svARB(GLenum target, const GLshort *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD2( v[0], v[1] );
}

void
_mesa_MultiTexCoord3dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD3( s, t, r );
}

void
_mesa_MultiTexCoord3dvARB(GLenum target, const GLdouble *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD3( v[0], v[1], v[2] );
}

void
_mesa_MultiTexCoord3fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD3( s, t, r );
}

void
_mesa_MultiTexCoord3fvARB(GLenum target, const GLfloat *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD3( v[0], v[1], v[2] );
}

void
_mesa_MultiTexCoord3iARB(GLenum target, GLint s, GLint t, GLint r)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD3( s, t, r );
}

void
_mesa_MultiTexCoord3ivARB(GLenum target, const GLint *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD3( v[0], v[1], v[2] );
}

void
_mesa_MultiTexCoord3sARB(GLenum target, GLshort s, GLshort t, GLshort r)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD3( s, t, r );
}

void
_mesa_MultiTexCoord3svARB(GLenum target, const GLshort *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD3( v[0], v[1], v[2] );
}

void
_mesa_MultiTexCoord4dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD4( s, t, r, q );
}

void
_mesa_MultiTexCoord4dvARB(GLenum target, const GLdouble *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD4( v[0], v[1], v[2], v[3] );
}

void
_mesa_MultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD4( s, t, r, q );
}


void
_mesa_MultiTexCoord4fvARB(GLenum target, const GLfloat *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD4( v[0], v[1], v[2], v[3] );
}

void
_mesa_MultiTexCoord4iARB(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD4( s, t, r, q );
}

void
_mesa_MultiTexCoord4ivARB(GLenum target, const GLint *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD4( v[0], v[1], v[2], v[3] );
}

void
_mesa_MultiTexCoord4sARB(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD4( s, t, r, q );
}

void
_mesa_MultiTexCoord4svARB(GLenum target, const GLshort *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   MULTI_TEXCOORD4( v[0], v[1], v[2], v[3] );
}

