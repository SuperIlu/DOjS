/* $Id: varray.c,v 1.22.4.4 2000/12/13 00:57:24 brianp Exp $ */

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
#include "cva.h"
#include "dlist.h"
#include "enable.h"
#include "enums.h"
#include "hash.h"
#include "light.h"
#include "macros.h"
#include "mem.h"
#include "mmath.h"
#include "pipeline.h"
#include "state.h"
#include "texstate.h"
#include "translate.h"
#include "types.h"
#include "varray.h"
#include "vb.h"
#include "vbfill.h"
#include "vbrender.h"
#include "vbindirect.h"
#include "vbxform.h"
#include "xform.h"
#endif



void
_mesa_VertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
   GET_CURRENT_CONTEXT(ctx);
   
   if (size<2 || size>4) {
      gl_error( ctx, GL_INVALID_VALUE, "glVertexPointer(size)" );
      return;
   }
   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glVertexPointer(stride)" );
      return;
   }
   
   if (MESA_VERBOSE&(VERBOSE_VARRAY|VERBOSE_API))
      fprintf(stderr, "glVertexPointer( sz %d type %s stride %d )\n", size, 
	      gl_lookup_enum_by_nr( type ),
	      stride);

#ifdef VAO
   ctx->Array.Current->Vertex.StrideB = stride;
#else
   ctx->Array.Vertex.StrideB = stride;
#endif
   if (!stride) {
      switch (type) {
      case GL_SHORT:
#ifdef VAO
         ctx->Array.Current->Vertex.StrideB =  size*sizeof(GLshort);
#else
         ctx->Array.Vertex.StrideB =  size*sizeof(GLshort);
#endif
         break;
      case GL_INT:
#ifdef VAO
         ctx->Array.Current->Vertex.StrideB =  size*sizeof(GLint);
#else
         ctx->Array.Vertex.StrideB =  size*sizeof(GLint);
#endif
         break;
      case GL_FLOAT:
#ifdef VAO
         ctx->Array.Current->Vertex.StrideB =  size*sizeof(GLfloat);
#else
         ctx->Array.Vertex.StrideB =  size*sizeof(GLfloat);
#endif
         break;
      case GL_DOUBLE:
#ifdef VAO
         ctx->Array.Current->Vertex.StrideB =  size*sizeof(GLdouble);
#else
         ctx->Array.Vertex.StrideB =  size*sizeof(GLdouble);
#endif
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glVertexPointer(type)" );
         return;
      }
   }
#ifdef VAO
   ctx->Array.Current->Vertex.Size = size;
   ctx->Array.Current->Vertex.Type = type;
   ctx->Array.Current->Vertex.Stride = stride;
   ctx->Array.Current->Vertex.Ptr = (void *) ptr;
   ctx->Array.Current->VertexFunc = gl_trans_4f_tab[size][TYPE_IDX(type)];
   ctx->Array.Current->VertexEltFunc = gl_trans_elt_4f_tab[size][TYPE_IDX(type)];
   ctx->Array.NewArrayState |= VERT_OBJ_ANY;
#else
   ctx->Array.Vertex.Size = size;
   ctx->Array.Vertex.Type = type;
   ctx->Array.Vertex.Stride = stride;
   ctx->Array.Vertex.Ptr = (void *) ptr;
   ctx->Array.VertexFunc = gl_trans_4f_tab[size][TYPE_IDX(type)];
   ctx->Array.VertexEltFunc = gl_trans_elt_4f_tab[size][TYPE_IDX(type)];
   ctx->Array.NewArrayState |= VERT_OBJ_ANY;
#endif
   ctx->NewState |= NEW_CLIENT_STATE;
}




void
_mesa_NormalPointer(GLenum type, GLsizei stride, const GLvoid *ptr )
{
   GET_CURRENT_CONTEXT(ctx);
   
   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glNormalPointer(stride)" );
      return;
   }

   if (MESA_VERBOSE&(VERBOSE_VARRAY|VERBOSE_API))
      fprintf(stderr, "glNormalPointer( type %s stride %d )\n", 
	      gl_lookup_enum_by_nr( type ),
	      stride);

#ifdef VAO
   ctx->Array.Current->Normal.StrideB = stride;
#else
   ctx->Array.Normal.StrideB = stride;
#endif
   if (!stride) {
      switch (type) {
      case GL_BYTE:
#ifdef VAO
         ctx->Array.Current->Normal.StrideB =  3*sizeof(GLbyte);
#else
         ctx->Array.Normal.StrideB =  3*sizeof(GLbyte);
#endif
         break;
      case GL_SHORT:
#ifdef VAO
         ctx->Array.Current->Normal.StrideB =  3*sizeof(GLshort);
#else
         ctx->Array.Normal.StrideB =  3*sizeof(GLshort);
#endif
         break;
      case GL_INT:
#ifdef VAO
         ctx->Array.Current->Normal.StrideB =  3*sizeof(GLint);
#else
         ctx->Array.Normal.StrideB =  3*sizeof(GLint);
#endif
         break;
      case GL_FLOAT:
#ifdef VAO
         ctx->Array.Current->Normal.StrideB =  3*sizeof(GLfloat);
#else
         ctx->Array.Normal.StrideB =  3*sizeof(GLfloat);
#endif
         break;
      case GL_DOUBLE:
#ifdef VAO
         ctx->Array.Current->Normal.StrideB =  3*sizeof(GLdouble);
#else
         ctx->Array.Normal.StrideB =  3*sizeof(GLdouble);
#endif
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glNormalPointer(type)" );
         return;
      }
   }
#ifdef VAO
   ctx->Array.Current->Normal.Type = type;
   ctx->Array.Current->Normal.Stride = stride;
   ctx->Array.Current->Normal.Ptr = (void *) ptr;
   ctx->Array.Current->NormalFunc = gl_trans_3f_tab[TYPE_IDX(type)];
   ctx->Array.Current->NormalEltFunc = gl_trans_elt_3f_tab[TYPE_IDX(type)];
#else
   ctx->Array.Normal.Type = type;
   ctx->Array.Normal.Stride = stride;
   ctx->Array.Normal.Ptr = (void *) ptr;
   ctx->Array.NormalFunc = gl_trans_3f_tab[TYPE_IDX(type)];
   ctx->Array.NormalEltFunc = gl_trans_elt_3f_tab[TYPE_IDX(type)];
#endif
   ctx->Array.NewArrayState |= VERT_NORM;
   ctx->NewState |= NEW_CLIENT_STATE;
}



void
_mesa_ColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
   GET_CURRENT_CONTEXT(ctx);

   if (size<3 || size>4) {
      gl_error( ctx, GL_INVALID_VALUE, "glColorPointer(size)" );
      return;
   }
   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glColorPointer(stride)" );
      return;
   }

   if (MESA_VERBOSE&(VERBOSE_VARRAY|VERBOSE_API))
      fprintf(stderr, "glColorPointer( sz %d type %s stride %d )\n", size, 
	  gl_lookup_enum_by_nr( type ),
	  stride);

#ifdef VAO
   ctx->Array.Current->Color.StrideB = stride;
#else
   ctx->Array.Color.StrideB = stride;
#endif
   if (!stride) {
      switch (type) {
      case GL_BYTE:
#ifdef VAO
         ctx->Array.Current->Color.StrideB =  size*sizeof(GLbyte);
#else
         ctx->Array.Color.StrideB =  size*sizeof(GLbyte);
#endif
         break;
      case GL_UNSIGNED_BYTE:
#ifdef VAO
         ctx->Array.Current->Color.StrideB =  size*sizeof(GLubyte);
#else
         ctx->Array.Color.StrideB =  size*sizeof(GLubyte);
#endif
         break;
      case GL_SHORT:
#ifdef VAO
         ctx->Array.Current->Color.StrideB =  size*sizeof(GLshort);
#else
         ctx->Array.Color.StrideB =  size*sizeof(GLshort);
#endif
         break;
      case GL_UNSIGNED_SHORT:
#ifdef VAO
         ctx->Array.Current->Color.StrideB =  size*sizeof(GLushort);
#else
         ctx->Array.Color.StrideB =  size*sizeof(GLushort);
#endif
         break;
      case GL_INT:
#ifdef VAO
         ctx->Array.Current->Color.StrideB =  size*sizeof(GLint);
#else
         ctx->Array.Color.StrideB =  size*sizeof(GLint);
#endif
         break;
      case GL_UNSIGNED_INT:
#ifdef VAO
         ctx->Array.Current->Color.StrideB =  size*sizeof(GLuint);
#else
         ctx->Array.Color.StrideB =  size*sizeof(GLuint);
#endif
         break;
      case GL_FLOAT:
#ifdef VAO
         ctx->Array.Current->Color.StrideB =  size*sizeof(GLfloat);
#else
         ctx->Array.Color.StrideB =  size*sizeof(GLfloat);
#endif
         break;
      case GL_DOUBLE:
#ifdef VAO
         ctx->Array.Current->Color.StrideB =  size*sizeof(GLdouble);
#else
         ctx->Array.Color.StrideB =  size*sizeof(GLdouble);
#endif
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glColorPointer(type)" );
         return;
      }
   }
#ifdef VAO
   ctx->Array.Current->Color.Size = size;
   ctx->Array.Current->Color.Type = type;
   ctx->Array.Current->Color.Stride = stride;
   ctx->Array.Current->Color.Ptr = (void *) ptr;
   ctx->Array.Current->ColorFunc = gl_trans_4ub_tab[size][TYPE_IDX(type)];
   ctx->Array.Current->ColorEltFunc = gl_trans_elt_4ub_tab[size][TYPE_IDX(type)];
#else
   ctx->Array.Color.Size = size;
   ctx->Array.Color.Type = type;
   ctx->Array.Color.Stride = stride;
   ctx->Array.Color.Ptr = (void *) ptr;
   ctx->Array.ColorFunc = gl_trans_4ub_tab[size][TYPE_IDX(type)];
   ctx->Array.ColorEltFunc = gl_trans_elt_4ub_tab[size][TYPE_IDX(type)];
#endif
   ctx->Array.NewArrayState |= VERT_RGBA;
   ctx->NewState |= NEW_CLIENT_STATE;
}



void
_mesa_IndexPointer(GLenum type, GLsizei stride, const GLvoid *ptr)
{
   GET_CURRENT_CONTEXT(ctx);
   
   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glIndexPointer(stride)" );
      return;
   }

#ifdef VAO
   ctx->Array.Current->Index.StrideB = stride;
#else
   ctx->Array.Index.StrideB = stride;
#endif
   if (!stride) {
      switch (type) {
      case GL_UNSIGNED_BYTE:
#ifdef VAO
         ctx->Array.Current->Index.StrideB =  sizeof(GLubyte);
#else
         ctx->Array.Index.StrideB =  sizeof(GLubyte);
#endif
         break;
      case GL_SHORT:
#ifdef VAO
         ctx->Array.Current->Index.StrideB =  sizeof(GLshort);
#else
         ctx->Array.Index.StrideB =  sizeof(GLshort);
#endif
         break;
      case GL_INT:
#ifdef VAO
         ctx->Array.Current->Index.StrideB =  sizeof(GLint);
#else
         ctx->Array.Index.StrideB =  sizeof(GLint);
#endif
         break;
      case GL_FLOAT:
#ifdef VAO
         ctx->Array.Current->Index.StrideB =  sizeof(GLfloat);
#else
         ctx->Array.Index.StrideB =  sizeof(GLfloat);
#endif
         break;
      case GL_DOUBLE:
#ifdef VAO
         ctx->Array.Current->Index.StrideB =  sizeof(GLdouble);
#else
         ctx->Array.Index.StrideB =  sizeof(GLdouble);
#endif
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glIndexPointer(type)" );
         return;
      }
   }
#ifdef VAO
   ctx->Array.Current->Index.Type = type;
   ctx->Array.Current->Index.Stride = stride;
   ctx->Array.Current->Index.Ptr = (void *) ptr;
   ctx->Array.Current->IndexFunc = gl_trans_1ui_tab[TYPE_IDX(type)];
   ctx->Array.Current->IndexEltFunc = gl_trans_elt_1ui_tab[TYPE_IDX(type)];
#else
   ctx->Array.Index.Type = type;
   ctx->Array.Index.Stride = stride;
   ctx->Array.Index.Ptr = (void *) ptr;
   ctx->Array.IndexFunc = gl_trans_1ui_tab[TYPE_IDX(type)];
   ctx->Array.IndexEltFunc = gl_trans_elt_1ui_tab[TYPE_IDX(type)];
#endif
   ctx->Array.NewArrayState |= VERT_INDEX;
   ctx->NewState |= NEW_CLIENT_STATE;
}



void
_mesa_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint texUnit;
   
   texUnit = ctx->Array.ActiveTexture;

   if (size<1 || size>4) {
      gl_error( ctx, GL_INVALID_VALUE, "glTexCoordPointer(size)" );
      return;
   }
   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glTexCoordPointer(stride)" );
      return;
   }

   if (MESA_VERBOSE&(VERBOSE_VARRAY|VERBOSE_API))
      fprintf(stderr, "glTexCoordPointer( unit %u sz %d type %s stride %d )\n", 
	  texUnit,
	  size, 
	  gl_lookup_enum_by_nr( type ),
	  stride);

#ifdef VAO
   ctx->Array.Current->TexCoord[texUnit].StrideB = stride;
#else
   ctx->Array.TexCoord[texUnit].StrideB = stride;
#endif
   if (!stride) {
      switch (type) {
      case GL_SHORT:
#ifdef VAO
         ctx->Array.Current->TexCoord[texUnit].StrideB =  size*sizeof(GLshort);
#else
         ctx->Array.TexCoord[texUnit].StrideB =  size*sizeof(GLshort);
#endif
         break;
      case GL_INT:
#ifdef VAO
         ctx->Array.Current->TexCoord[texUnit].StrideB =  size*sizeof(GLint);
#else
         ctx->Array.TexCoord[texUnit].StrideB =  size*sizeof(GLint);
#endif
         break;
      case GL_FLOAT:
#ifdef VAO
         ctx->Array.Current->TexCoord[texUnit].StrideB =  size*sizeof(GLfloat);
#else
         ctx->Array.TexCoord[texUnit].StrideB =  size*sizeof(GLfloat);
#endif
         break;
      case GL_DOUBLE:
#ifdef VAO
         ctx->Array.Current->TexCoord[texUnit].StrideB =  size*sizeof(GLdouble);
#else
         ctx->Array.TexCoord[texUnit].StrideB =  size*sizeof(GLdouble);
#endif
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glTexCoordPointer(type)" );
         return;
      }
   }
#ifdef VAO
   ctx->Array.Current->TexCoord[texUnit].Size = size;
   ctx->Array.Current->TexCoord[texUnit].Type = type;
   ctx->Array.Current->TexCoord[texUnit].Stride = stride;
   ctx->Array.Current->TexCoord[texUnit].Ptr = (void *) ptr;

   ctx->Array.Current->TexCoordFunc[texUnit] = gl_trans_4f_tab[size][TYPE_IDX(type)];
   ctx->Array.Current->TexCoordEltFunc[texUnit] = gl_trans_elt_4f_tab[size][TYPE_IDX(type)];
#else
   ctx->Array.TexCoord[texUnit].Size = size;
   ctx->Array.TexCoord[texUnit].Type = type;
   ctx->Array.TexCoord[texUnit].Stride = stride;
   ctx->Array.TexCoord[texUnit].Ptr = (void *) ptr;

   ctx->Array.TexCoordFunc[texUnit] = gl_trans_4f_tab[size][TYPE_IDX(type)];
   ctx->Array.TexCoordEltFunc[texUnit] = gl_trans_elt_4f_tab[size][TYPE_IDX(type)];
#endif
   ctx->Array.NewArrayState |= PIPE_TEX(texUnit);
   ctx->NewState |= NEW_CLIENT_STATE;
}




void
_mesa_EdgeFlagPointer(GLsizei stride, const void *vptr)
{
   GET_CURRENT_CONTEXT(ctx);
   const GLboolean *ptr = (GLboolean *)vptr;

   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glEdgeFlagPointer(stride)" );
      return;
   }
#ifdef VAO
   ctx->Array.Current->EdgeFlag.Stride = stride;
   ctx->Array.Current->EdgeFlag.StrideB = stride ? stride : sizeof(GLboolean);
   ctx->Array.Current->EdgeFlag.Ptr = (GLboolean *) ptr;
   if (stride != sizeof(GLboolean)) {
      ctx->Array.Current->EdgeFlagFunc = gl_trans_1ub_tab[TYPE_IDX(GL_UNSIGNED_BYTE)];
   } else {
      ctx->Array.Current->EdgeFlagFunc = 0;
   }
   ctx->Array.Current->EdgeFlagEltFunc = gl_trans_elt_1ub_tab[TYPE_IDX(GL_UNSIGNED_BYTE)];
#else
   ctx->Array.EdgeFlag.Stride = stride;
   ctx->Array.EdgeFlag.StrideB = stride ? stride : sizeof(GLboolean);
   ctx->Array.EdgeFlag.Ptr = (GLboolean *) ptr;
   if (stride != sizeof(GLboolean)) {
      ctx->Array.EdgeFlagFunc = gl_trans_1ub_tab[TYPE_IDX(GL_UNSIGNED_BYTE)];
   } else {
      ctx->Array.EdgeFlagFunc = 0;
   }
   ctx->Array.EdgeFlagEltFunc = gl_trans_elt_1ub_tab[TYPE_IDX(GL_UNSIGNED_BYTE)];
#endif
   ctx->Array.NewArrayState |= VERT_EDGE;
   ctx->NewState |= NEW_CLIENT_STATE;
}


#if 0
/* Called only from gl_DrawElements
 */
static void gl_CVAEltPointer( GLcontext *ctx, GLenum type, const GLvoid *ptr )
{
   switch (type) {
      case GL_UNSIGNED_BYTE:
         ctx->CVA.Elt.StrideB = sizeof(GLubyte);
         break;
      case GL_UNSIGNED_SHORT:
         ctx->CVA.Elt.StrideB = sizeof(GLushort);
         break;
      case GL_UNSIGNED_INT:
         ctx->CVA.Elt.StrideB = sizeof(GLuint);
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glEltPointer(type)" );
         return;
   }
   ctx->CVA.Elt.Type = type;
   ctx->CVA.Elt.Stride = 0;
   ctx->CVA.Elt.Ptr = (void *) ptr;
   ctx->CVA.EltFunc = gl_trans_1ui_tab[TYPE_IDX(type)];
   ctx->Array.NewArrayState |= VERT_ELT; /* ??? */
}
#endif



void
_mesa_VertexPointerEXT(GLint size, GLenum type, GLsizei stride,
                       GLsizei count, const GLvoid *ptr)
{
   (void) count;
   _mesa_VertexPointer(size, type, stride, ptr);
}


void
_mesa_NormalPointerEXT(GLenum type, GLsizei stride, GLsizei count,
                       const GLvoid *ptr)
{
   (void) count;
   _mesa_NormalPointer(type, stride, ptr);
}


void
_mesa_ColorPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count,
                      const GLvoid *ptr)
{
   (void) count;
   _mesa_ColorPointer(size, type, stride, ptr);
}


void
_mesa_IndexPointerEXT(GLenum type, GLsizei stride, GLsizei count,
                      const GLvoid *ptr)
{
   (void) count;
   _mesa_IndexPointer(type, stride, ptr);
}


void
_mesa_TexCoordPointerEXT(GLint size, GLenum type, GLsizei stride,
                         GLsizei count, const GLvoid *ptr)
{
   (void) count;
   _mesa_TexCoordPointer(size, type, stride, ptr);
}


void
_mesa_EdgeFlagPointerEXT(GLsizei stride, GLsizei count, const GLboolean *ptr)
{
   (void) count;
   _mesa_EdgeFlagPointer(stride, ptr);
}





/* KW: Batch function to exec all the array elements in the input
 *     buffer prior to transform.  Done only the first time a vertex
 *     buffer is executed or compiled.
 *
 * KW: Have to do this after each glEnd if cva isn't active.  (also
 *     have to do it after each full buffer)
 */
void gl_exec_array_elements( GLcontext *ctx, struct immediate *IM,
			     GLuint start, 
			     GLuint count)
{
   GLuint *flags = IM->Flag;
   GLuint *elts = IM->Elt;
#ifdef VAO
   GLuint translate = ctx->Array.Current->Flags;
#else
   GLuint translate = ctx->Array.Flags;
#endif
   GLuint i;

   if (MESA_VERBOSE&VERBOSE_IMMEDIATE)
      fprintf(stderr, "exec_array_elements %d .. %d\n", start, count);
   
#ifdef VAO
   if (translate & VERT_OBJ_ANY) 
      (ctx->Array.Current->VertexEltFunc)( IM->Obj, 
				  &ctx->Array.Current->Vertex, 
				  flags, elts, (VERT_ELT|VERT_OBJ_ANY),
				  start, count);
   
   if (translate & VERT_NORM) 
      (ctx->Array.Current->NormalEltFunc)( IM->Normal, 
				  &ctx->Array.Current->Normal, 
				  flags, elts, (VERT_ELT|VERT_NORM),
				  start, count);

   if (translate & VERT_EDGE) 
      (ctx->Array.Current->EdgeFlagEltFunc)( IM->EdgeFlag, 
				    &ctx->Array.Current->EdgeFlag, 
				    flags, elts, (VERT_ELT|VERT_EDGE),
				    start, count);
   
   if (translate & VERT_RGBA)
      (ctx->Array.Current->ColorEltFunc)( IM->Color, 
				 &ctx->Array.Current->Color, 
				 flags, elts, (VERT_ELT|VERT_RGBA),
				 start, count);

   if (translate & VERT_INDEX)
      (ctx->Array.Current->IndexEltFunc)( IM->Index, 
				 &ctx->Array.Current->Index, 
				 flags, elts, (VERT_ELT|VERT_INDEX),
				 start, count);

   if (translate & VERT_TEX0_ANY)
      (ctx->Array.Current->TexCoordEltFunc[0])( IM->TexCoord[0], 
				       &ctx->Array.Current->TexCoord[0], 
				       flags, elts, (VERT_ELT|VERT_TEX0_ANY),
				       start, count);

   if (translate & VERT_TEX1_ANY)
      (ctx->Array.Current->TexCoordEltFunc[1])( IM->TexCoord[1], 
				       &ctx->Array.Current->TexCoord[1], 
				       flags, elts, (VERT_ELT|VERT_TEX1_ANY),
				       start, count);
#else
   if (translate & VERT_OBJ_ANY) 
      (ctx->Array.VertexEltFunc)( IM->Obj, 
				  &ctx->Array.Vertex, 
				  flags, elts, (VERT_ELT|VERT_OBJ_ANY),
				  start, count);
   
   if (translate & VERT_NORM) 
      (ctx->Array.NormalEltFunc)( IM->Normal, 
				  &ctx->Array.Normal, 
				  flags, elts, (VERT_ELT|VERT_NORM),
				  start, count);

   if (translate & VERT_EDGE) 
      (ctx->Array.EdgeFlagEltFunc)( IM->EdgeFlag, 
				    &ctx->Array.EdgeFlag, 
				    flags, elts, (VERT_ELT|VERT_EDGE),
				    start, count);
   
   if (translate & VERT_RGBA)
      (ctx->Array.ColorEltFunc)( IM->Color, 
				 &ctx->Array.Color, 
				 flags, elts, (VERT_ELT|VERT_RGBA),
				 start, count);

   if (translate & VERT_INDEX)
      (ctx->Array.IndexEltFunc)( IM->Index, 
				 &ctx->Array.Index, 
				 flags, elts, (VERT_ELT|VERT_INDEX),
				 start, count);

   if (translate & VERT_TEX0_ANY)
      (ctx->Array.TexCoordEltFunc[0])( IM->TexCoord[0], 
				       &ctx->Array.TexCoord[0], 
				       flags, elts, (VERT_ELT|VERT_TEX0_ANY),
				       start, count);

   if (translate & VERT_TEX1_ANY)
      (ctx->Array.TexCoordEltFunc[1])( IM->TexCoord[1], 
				       &ctx->Array.TexCoord[1], 
				       flags, elts, (VERT_ELT|VERT_TEX1_ANY),
				       start, count);
#endif

   for (i = start ; i < count ; i++) 
      if (flags[i] & VERT_ELT) 
	 flags[i] |= translate;

}



/* Enough funny business going on in here it might be quicker to use a
 * function pointer.
 */
#define ARRAY_ELT( IM, i )					\
{								\
   GLuint count = IM->Count;					\
   IM->Elt[count] = i;						\
   IM->Flag[count] = ((IM->Flag[count] & IM->ArrayAndFlags) |	\
		      VERT_ELT);				\
   IM->FlushElt |= IM->ArrayEltFlush;				\
   IM->Count = count += IM->ArrayIncr;				\
   if (count == VB_MAX)						\
      IM->maybe_transform_vb( IM );				\
}


void
_mesa_ArrayElement( GLint i )
{
   GET_IMMEDIATE;
   ARRAY_ELT( IM, i );
}


static void
gl_ArrayElement( GLcontext *CC, GLint i )
{
   struct immediate *im = CC->input;
   ARRAY_ELT( im, i );
}



void
_mesa_DrawArrays(GLenum mode, GLint start, GLsizei count)
{
   GET_CURRENT_CONTEXT(ctx);
   struct vertex_buffer *VB = ctx->VB;
   GLint i;

   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glDrawArrays");

   if (count<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glDrawArrays(count)" );
      return;
   }

#ifdef VAO
   if (!ctx->CompileFlag && ctx->Array.Current->Vertex.Enabled) {
#else
   if (!ctx->CompileFlag && ctx->Array.Vertex.Enabled) {
#endif
      GLint remaining = count;
      GLint i;
      struct gl_client_array *Normal = 0;
      struct gl_client_array *Color = 0;
      struct gl_client_array *Index = 0;
      struct gl_client_array *TexCoord[MAX_TEXTURE_UNITS];
      struct gl_client_array *EdgeFlag = 0;
      struct immediate *IM = VB->IM;
      struct gl_pipeline *elt = &ctx->CVA.elt;
      GLboolean relock;
      GLuint fallback, required;

      if (ctx->NewState)
	 gl_update_state( ctx );	

      /* Just turn off cva on this path.  Could be useful for multipass
       * rendering to keep it turned on.
       */
      relock = ctx->CompileCVAFlag;

      if (relock) {
	 ctx->CompileCVAFlag = 0;
	 elt->pipeline_valid = 0;
      }

      if (!elt->pipeline_valid)
	 gl_build_immediate_pipeline( ctx );

      required = elt->inputs;
#ifdef VAO
      fallback = (elt->inputs & ~ctx->Array.Current->Summary);
#else
      fallback = (elt->inputs & ~ctx->Array.Summary);
#endif

      /* The translate function doesn't do anything about size.  It
       * just ensures that type and stride come out right.
       */
#ifdef VAO
      IM->v.Obj.size = ctx->Array.Current->Vertex.Size;

      if (required & VERT_RGBA) 
      {
	 Color = &ctx->Array.Current->Color;
	 if (fallback & VERT_RGBA) {
	    Color = &ctx->Fallback.Color;
	    ctx->Array.Current->ColorFunc = 
	       gl_trans_4ub_tab[4][TYPE_IDX(GL_UNSIGNED_BYTE)];
	 }
      }
   
      if (required & VERT_INDEX) 
      {
	 Index = &ctx->Array.Current->Index;
	 if (fallback & VERT_INDEX) {
	    Index = &ctx->Fallback.Index;
	    ctx->Array.Current->IndexFunc = gl_trans_1ui_tab[TYPE_IDX(GL_UNSIGNED_INT)];
	 }
      }

      for (i = 0 ; i < MAX_TEXTURE_UNITS ; i++) 
      {
	 GLuint flag = VERT_TEX_ANY(i);

	 if (required & flag) {
	    TexCoord[i] = &ctx->Array.Current->TexCoord[i];

	    if (fallback & flag) 
	    {
	       TexCoord[i] = &ctx->Fallback.TexCoord[i];
	       TexCoord[i]->Size = gl_texcoord_size( ctx->Current.Flag, i );

	       ctx->Array.Current->TexCoordFunc[i] = 
		  gl_trans_4f_tab[TexCoord[i]->Size][TYPE_IDX(GL_FLOAT)];
	    }
	 }
      }

      if (ctx->Array.Current->Flags != ctx->Array.Current->Flag[0])
 	 for (i = 0 ; i < VB_MAX ; i++) 
	    ctx->Array.Current->Flag[i] = ctx->Array.Current->Flags;


      if (required & VERT_NORM) 
      {
	 Normal = &ctx->Array.Current->Normal;
	 if (fallback & VERT_NORM) {
	    Normal = &ctx->Fallback.Normal;
	    ctx->Array.Current->NormalFunc = gl_trans_3f_tab[TYPE_IDX(GL_FLOAT)];
	 }
      }

      if ( required & VERT_EDGE )
      {
	 if (mode == GL_TRIANGLES || 
	     mode == GL_QUADS || 
	     mode == GL_POLYGON)
	 {
	    EdgeFlag = &ctx->Array.Current->EdgeFlag;
	    if (fallback & VERT_EDGE) {
	       EdgeFlag = &ctx->Fallback.EdgeFlag;
	       ctx->Array.Current->EdgeFlagFunc = 
		  gl_trans_1ub_tab[TYPE_IDX(GL_UNSIGNED_BYTE)];
	    }
	 }
	 else
	    required &= ~VERT_EDGE;
      }
#else
      IM->v.Obj.size = ctx->Array.Vertex.Size;

      if (required & VERT_RGBA) 
      {
	 Color = &ctx->Array.Color;
	 if (fallback & VERT_RGBA) {
	    Color = &ctx->Fallback.Color;
	    ctx->Array.ColorFunc = 
	       gl_trans_4ub_tab[4][TYPE_IDX(GL_UNSIGNED_BYTE)];
	 }
      }
   
      if (required & VERT_INDEX) 
      {
	 Index = &ctx->Array.Index;
	 if (fallback & VERT_INDEX) {
	    Index = &ctx->Fallback.Index;
	    ctx->Array.IndexFunc = gl_trans_1ui_tab[TYPE_IDX(GL_UNSIGNED_INT)];
	 }
      }

      for (i = 0 ; i < MAX_TEXTURE_UNITS ; i++) 
      {
	 GLuint flag = VERT_TEX_ANY(i);

	 if (required & flag) {
	    TexCoord[i] = &ctx->Array.TexCoord[i];

	    if (fallback & flag) 
	    {
	       TexCoord[i] = &ctx->Fallback.TexCoord[i];
	       TexCoord[i]->Size = gl_texcoord_size( ctx->Current.Flag, i );

	       ctx->Array.TexCoordFunc[i] = 
		  gl_trans_4f_tab[TexCoord[i]->Size][TYPE_IDX(GL_FLOAT)];
	    }
	 }
      }

      if (ctx->Array.Flags != ctx->Array.Flag[0])
 	 for (i = 0 ; i < VB_MAX ; i++) 
	    ctx->Array.Flag[i] = ctx->Array.Flags;


      if (required & VERT_NORM) 
      {
	 Normal = &ctx->Array.Normal;
	 if (fallback & VERT_NORM) {
	    Normal = &ctx->Fallback.Normal;
	    ctx->Array.NormalFunc = gl_trans_3f_tab[TYPE_IDX(GL_FLOAT)];
	 }
      }

      if ( required & VERT_EDGE )
      {
	 if (mode == GL_TRIANGLES || 
	     mode == GL_QUADS || 
	     mode == GL_POLYGON)
	 {
	    EdgeFlag = &ctx->Array.EdgeFlag;
	    if (fallback & VERT_EDGE) {
	       EdgeFlag = &ctx->Fallback.EdgeFlag;
	       ctx->Array.EdgeFlagFunc = 
		  gl_trans_1ub_tab[TYPE_IDX(GL_UNSIGNED_BYTE)];
	    }
	 }
	 else
	    required &= ~VERT_EDGE;
      }
#endif

      VB->Primitive = IM->Primitive; 
      VB->NextPrimitive = IM->NextPrimitive; 
      VB->MaterialMask = IM->MaterialMask;
      VB->Material = IM->Material;
      VB->BoundsPtr = 0;

      while (remaining > 0) {
         GLint vbspace = VB_MAX - VB_START;
	 GLuint count, n;
	 
	 if (vbspace >= remaining) {
	    n = remaining;
	    VB->LastPrimitive = VB_START + n;
	 } else {
	    n = vbspace;
	    VB->LastPrimitive = VB_START;
	 }
	 
	 VB->CullMode = 0;
	 
#ifdef VAO
	 ctx->Array.Current->VertexFunc( IM->Obj + VB_START, 
				&ctx->Array.Current->Vertex, start, n );
	 
	 if (required & VERT_NORM) {
	    ctx->Array.Current->NormalFunc( IM->Normal + VB_START, 
				   Normal, start, n );
	 }
	 
	 if (required & VERT_EDGE) {
	    ctx->Array.Current->EdgeFlagFunc( IM->EdgeFlag + VB_START, 
				     EdgeFlag, start, n );
	 }
	 
	 if (required & VERT_RGBA) {
	    ctx->Array.Current->ColorFunc( IM->Color + VB_START, 
				  Color, start, n );
	 }
	 
	 if (required & VERT_INDEX) {
	    ctx->Array.Current->IndexFunc( IM->Index + VB_START, 
				  Index, start, n );
	 }
	 
	 if (required & VERT_TEX0_ANY) {
	    IM->v.TexCoord[0].size = TexCoord[0]->Size;
	    ctx->Array.Current->TexCoordFunc[0]( IM->TexCoord[0] + VB_START, 
					TexCoord[0], start, n );
	 }
	 
	 if (required & VERT_TEX1_ANY) {
	    IM->v.TexCoord[1].size = TexCoord[1]->Size;
	    ctx->Array.Current->TexCoordFunc[1]( IM->TexCoord[1] + VB_START, 
					TexCoord[1], start, n );
	 }
#else
	 ctx->Array.VertexFunc( IM->Obj + VB_START, 
				&ctx->Array.Vertex, start, n );
	 
	 if (required & VERT_NORM) {
	    ctx->Array.NormalFunc( IM->Normal + VB_START, 
				   Normal, start, n );
	 }
	 
	 if (required & VERT_EDGE) {
	    ctx->Array.EdgeFlagFunc( IM->EdgeFlag + VB_START, 
				     EdgeFlag, start, n );
	 }
	 
	 if (required & VERT_RGBA) {
	    ctx->Array.ColorFunc( IM->Color + VB_START, 
				  Color, start, n );
	 }
	 
	 if (required & VERT_INDEX) {
	    ctx->Array.IndexFunc( IM->Index + VB_START, 
				  Index, start, n );
	 }
	 
	 if (required & VERT_TEX0_ANY) {
	    IM->v.TexCoord[0].size = TexCoord[0]->Size;
	    ctx->Array.TexCoordFunc[0]( IM->TexCoord[0] + VB_START, 
					TexCoord[0], start, n );
	 }
	 
	 if (required & VERT_TEX1_ANY) {
	    IM->v.TexCoord[1].size = TexCoord[1]->Size;
	    ctx->Array.TexCoordFunc[1]( IM->TexCoord[1] + VB_START, 
					TexCoord[1], start, n );
	 }
#endif
	 VB->ObjPtr = &IM->v.Obj;
	 VB->NormalPtr = &IM->v.Normal;
	 VB->ColorPtr = &IM->v.Color;
	 VB->Color[0] = VB->Color[1] = VB->ColorPtr;
	 VB->IndexPtr = &IM->v.Index;
	 VB->EdgeFlagPtr = &IM->v.EdgeFlag;
	 VB->TexCoordPtr[0] = &IM->v.TexCoord[0];
	 VB->TexCoordPtr[1] = &IM->v.TexCoord[1];

#ifdef VAO
	 VB->Flag = ctx->Array.Current->Flag;
	 VB->OrFlag = ctx->Array.Current->Flags;
#else
	 VB->Flag = ctx->Array.Flag;
	 VB->OrFlag = ctx->Array.Flags;
#endif

	 VB->Start = IM->Start = VB_START;
	 count = VB->Count = IM->Count = VB_START + n;

#define RESET_VEC(v, t, s, c) (v.start = t v.data[s], v.count = c)  

	 RESET_VEC(IM->v.Obj, (GLfloat *), VB_START, count);
	 RESET_VEC(IM->v.Normal, (GLfloat *), VB_START, count);
	 RESET_VEC(IM->v.TexCoord[0], (GLfloat *), VB_START, count);
	 RESET_VEC(IM->v.TexCoord[1], (GLfloat *), VB_START, count);
	 RESET_VEC(IM->v.Index, &, VB_START, count);
	 RESET_VEC(IM->v.Elt, &, VB_START, count);
	 RESET_VEC(IM->v.EdgeFlag, &, VB_START, count);
	 RESET_VEC(IM->v.Color, (GLubyte *), VB_START, count);
	 RESET_VEC(VB->Clip, (GLfloat *), VB_START, count);
	 RESET_VEC(VB->Eye, (GLfloat *), VB_START, count);
	 RESET_VEC(VB->Win, (GLfloat *), VB_START, count);
	 RESET_VEC(VB->BColor, (GLubyte *), VB_START, count); 
	 RESET_VEC(VB->BIndex, &, VB_START, count);

	 VB->NextPrimitive[VB->CopyStart] = VB->Count;
	 VB->Primitive[VB->CopyStart] = mode;
#ifdef VAO
	 ctx->Array.Current->Flag[count] |= VERT_END_VB;
#else
	 ctx->Array.Flag[count] |= VERT_END_VB;
#endif

         /* Transform and render.
	  */
         gl_run_pipeline( VB );
	 gl_reset_vb( VB );

#ifdef VAO
	 ctx->Array.Current->Flag[count] = ctx->Array.Current->Flags;
	 ctx->Array.Current->Flag[VB_START] = ctx->Array.Current->Flags;
#else
	 ctx->Array.Flag[count] = ctx->Array.Flags;
	 ctx->Array.Flag[VB_START] = ctx->Array.Flags;
#endif

         start += n;
         remaining -= n;
      }

      gl_reset_input( ctx );

      if (relock) {
	 ctx->CompileCVAFlag = relock;
	 elt->pipeline_valid = 0;
      }
   }
#ifdef VAO
   else if (ctx->Array.Current->Vertex.Enabled)
#else
   else if (ctx->Array.Vertex.Enabled)
#endif
   {
      /* The GL_COMPILE and GL_COMPILE_AND_EXECUTE cases.  These
       * could be handled by the above code, but it gets a little
       * complex.  The generated list is still of good quality
       * this way.
       */
      gl_Begin( ctx, mode );
      for (i=0;i<count;i++) {
         gl_ArrayElement( ctx, start+i );
      }
      gl_End( ctx );
   }
   else
   {
      /* The degenerate case where vertices are not enabled - only
       * need to process the very final array element, as all of the
       * preceding ones would be overwritten anyway. 
       */
      gl_Begin( ctx, mode );
      gl_ArrayElement( ctx, start+count );
      gl_End( ctx );
   }
}



/* KW: Exactly fakes the effects of calling glArrayElement multiple times.
 *     Compilation is handled via. the IM->maybe_transform_vb() callback.
 */
#if 1
#define DRAW_ELT(FUNC, TYPE)				\
static void FUNC( GLcontext *ctx, GLenum mode,		\
		  TYPE *indices, GLuint count )		\
{							\
   GLuint i,j;						\
							\
   gl_Begin( ctx, mode );				\
							\
   for (j = 0 ; j < count ; ) {				\
      struct immediate *IM = ctx->input;		\
      GLuint start = IM->Start;				\
      GLuint nr = MIN2( VB_MAX, count - j + start );	\
      GLuint sf = IM->Flag[start];			\
      IM->FlushElt |= IM->ArrayEltFlush;		\
							\
      for (i = start ; i < nr ; i++) {			\
	 IM->Elt[i] = (GLuint) *indices++;		\
	 IM->Flag[i] = VERT_ELT;			\
      }							\
							\
      if (j == 0) IM->Flag[start] |= sf;		\
							\
      IM->Count = nr;					\
      j += nr - start;					\
							\
      if (j == count) gl_End( ctx );			\
      IM->maybe_transform_vb( IM );			\
   }							\
}
#else 
#define DRAW_ELT(FUNC, TYPE)				\
static void FUNC( GLcontext *ctx, GLenum mode,		\
		   TYPE *indices, GLuint count )	\
{							\
  int i;						\
  glBegin(mode);					\
  for (i = 0 ; i < count ; i++)				\
    glArrayElement( indices[i] );			\
  glEnd();						\
}
#endif
	

DRAW_ELT( draw_elt_ubyte, GLubyte )
DRAW_ELT( draw_elt_ushort, GLushort )
DRAW_ELT( draw_elt_uint, GLuint )


static GLuint natural_stride[0x10] = 
{
   sizeof(GLbyte),		/* 0 */
   sizeof(GLubyte),		/* 1 */
   sizeof(GLshort),		/* 2 */
   sizeof(GLushort),		/* 3 */
   sizeof(GLint),		/* 4 */
   sizeof(GLuint),		/* 5 */
   sizeof(GLfloat),		/* 6 */
   2 * sizeof(GLbyte),		/* 7 */
   3 * sizeof(GLbyte),		/* 8 */
   4 * sizeof(GLbyte),		/* 9 */
   sizeof(GLdouble),		/* a */
   0,				/* b */
   0,				/* c */
   0,				/* d */
   0,				/* e */
   0				/* f */
};


void
_mesa_DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_cva *cva;
      
   cva = &ctx->CVA;
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glDrawElements");

   if (count <= 0) {
      if (count < 0)
	 gl_error( ctx, GL_INVALID_VALUE, "glDrawElements(count)" );
      return;
   }

   if (mode > GL_POLYGON) {
      gl_error( ctx, GL_INVALID_ENUM, "glDrawArrays(mode)" );
      return;
   }
   
   if (type != GL_UNSIGNED_INT && type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT)
   {
       gl_error( ctx, GL_INVALID_ENUM, "glDrawElements(type)" );
       return;
   }

   if (ctx->NewState)
      gl_update_state(ctx);

   if (ctx->CompileCVAFlag) 
   {
#if defined(MESA_CVA_PROF)
      force_init_prof(); 
#endif

      /* Treat VERT_ELT like a special client array.
       */
      ctx->Array.NewArrayState |= VERT_ELT;
#ifdef VAO
      ctx->Array.Current->Summary |= VERT_ELT;
      ctx->Array.Current->Flags |= VERT_ELT;
#else
      ctx->Array.Summary |= VERT_ELT;
      ctx->Array.Flags |= VERT_ELT;
#endif

      cva->elt_mode = mode;
      cva->elt_count = count;
      cva->Elt.Type = type;
      cva->Elt.Ptr = (void *) indices;
      cva->Elt.StrideB = natural_stride[TYPE_IDX(type)];
      cva->EltFunc = gl_trans_1ui_tab[TYPE_IDX(type)];

      if (!cva->pre.pipeline_valid) 
	 gl_build_precalc_pipeline( ctx );
      else if (MESA_VERBOSE & VERBOSE_PIPELINE)
	 fprintf(stderr, ": dont rebuild\n");

      gl_cva_force_precalc( ctx );

      /* Did we 'precalculate' the render op?
       */
      if (ctx->CVA.pre.ops & PIPE_OP_RENDER) {
	 ctx->Array.NewArrayState |= VERT_ELT;
#ifdef VAO
	 ctx->Array.Current->Summary &= ~VERT_ELT;
	 ctx->Array.Current->Flags &= ~VERT_ELT;
#else
	 ctx->Array.Summary &= ~VERT_ELT;
	 ctx->Array.Flags &= ~VERT_ELT;
#endif
	 return;
      } 

      if ( (MESA_VERBOSE&VERBOSE_VARRAY) )
	 printf("using immediate\n");
   }


   /* Otherwise, have to use the immediate path to render.
    */
   switch (type) {
   case GL_UNSIGNED_BYTE:
   {
      GLubyte *ub_indices = (GLubyte *) indices;
#ifdef VAO
      if (ctx->Array.Current->Summary & VERT_OBJ_ANY) {
#else
      if (ctx->Array.Summary & VERT_OBJ_ANY) {
#endif
	 draw_elt_ubyte( ctx, mode, ub_indices, count );
      } else {
	 gl_ArrayElement( ctx, (GLuint) ub_indices[count-1] );
      }
   }
   break;
   case GL_UNSIGNED_SHORT:
   {
      GLushort *us_indices = (GLushort *) indices;
#ifdef VAO
      if (ctx->Array.Current->Summary & VERT_OBJ_ANY) {
#else
      if (ctx->Array.Summary & VERT_OBJ_ANY) {
#endif
	 draw_elt_ushort( ctx, mode, us_indices, count );
      } else {
	 gl_ArrayElement( ctx, (GLuint) us_indices[count-1] );
      }
   }
   break;
   case GL_UNSIGNED_INT:
   {
      GLuint *ui_indices = (GLuint *) indices;
#ifdef VAO
      if (ctx->Array.Current->Summary & VERT_OBJ_ANY) {
#else
      if (ctx->Array.Summary & VERT_OBJ_ANY) {
#endif
	 draw_elt_uint( ctx, mode, ui_indices, count );
      } else {
	 gl_ArrayElement( ctx, ui_indices[count-1] );
      }
   }
   break;
   default:
      gl_error( ctx, GL_INVALID_ENUM, "glDrawElements(type)" );
      break;
   }

   if (ctx->CompileCVAFlag) {
      ctx->Array.NewArrayState |= VERT_ELT;
#ifdef VAO
      ctx->Array.Current->Summary &= ~VERT_ELT;
#else
      ctx->Array.Summary &= ~VERT_ELT;
#endif
   }
}



void
_mesa_InterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer)
{
   GET_CURRENT_CONTEXT(ctx);
   GLboolean tflag, cflag, nflag;              /* enable/disable flags */
   GLint tcomps, ccomps, vcomps;               /* components per texcoord, color, vertex */

   GLenum ctype = 0;                           /* color type */
   GLint coffset = 0, noffset = 0, voffset = 0;/* color, normal, vertex offsets */
   GLint defstride;                            /* default stride */
   GLint c, f;
   GLint coordUnitSave;
   
   f = sizeof(GLfloat);
   c = f * ((4*sizeof(GLubyte) + (f-1)) / f);

   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glInterleavedArrays(stride)" );
      return;
   }

   switch (format) {
      case GL_V2F:
         tflag = GL_FALSE;  cflag = GL_FALSE;  nflag = GL_FALSE;
         tcomps = 0;  ccomps = 0;  vcomps = 2;
         voffset = 0;
         defstride = 2*f;
         break;
      case GL_V3F:
         tflag = GL_FALSE;  cflag = GL_FALSE;  nflag = GL_FALSE;
         tcomps = 0;  ccomps = 0;  vcomps = 3;
         voffset = 0;
         defstride = 3*f;
         break;
      case GL_C4UB_V2F:
         tflag = GL_FALSE;  cflag = GL_TRUE;  nflag = GL_FALSE;
         tcomps = 0;  ccomps = 4;  vcomps = 2;
         ctype = GL_UNSIGNED_BYTE;
         coffset = 0;
         voffset = c;
         defstride = c + 2*f;
         break;
      case GL_C4UB_V3F:
         tflag = GL_FALSE;  cflag = GL_TRUE;  nflag = GL_FALSE;
         tcomps = 0;  ccomps = 4;  vcomps = 3;
         ctype = GL_UNSIGNED_BYTE;
         coffset = 0;
         voffset = c;
         defstride = c + 3*f;
         break;
      case GL_C3F_V3F:
         tflag = GL_FALSE;  cflag = GL_TRUE;  nflag = GL_FALSE;
         tcomps = 0;  ccomps = 3;  vcomps = 3;
         ctype = GL_FLOAT;
         coffset = 0;
         voffset = 3*f;
         defstride = 6*f;
         break;
      case GL_N3F_V3F:
         tflag = GL_FALSE;  cflag = GL_FALSE;  nflag = GL_TRUE;
         tcomps = 0;  ccomps = 0;  vcomps = 3;
         noffset = 0;
         voffset = 3*f;
         defstride = 6*f;
         break;
      case GL_C4F_N3F_V3F:
         tflag = GL_FALSE;  cflag = GL_TRUE;  nflag = GL_TRUE;
         tcomps = 0;  ccomps = 4;  vcomps = 3;
         ctype = GL_FLOAT;
         coffset = 0;
         noffset = 4*f;
         voffset = 7*f;
         defstride = 10*f;
         break;
      case GL_T2F_V3F:
         tflag = GL_TRUE;  cflag = GL_FALSE;  nflag = GL_FALSE;
         tcomps = 2;  ccomps = 0;  vcomps = 3;
         voffset = 2*f;
         defstride = 5*f;
         break;
      case GL_T4F_V4F:
         tflag = GL_TRUE;  cflag = GL_FALSE;  nflag = GL_FALSE;
         tcomps = 4;  ccomps = 0;  vcomps = 4;
         voffset = 4*f;
         defstride = 8*f;
         break;
      case GL_T2F_C4UB_V3F:
         tflag = GL_TRUE;  cflag = GL_TRUE;  nflag = GL_FALSE;
         tcomps = 2;  ccomps = 4;  vcomps = 3;
         ctype = GL_UNSIGNED_BYTE;
         coffset = 2*f;
         voffset = c+2*f;
         defstride = c+5*f;
         break;
      case GL_T2F_C3F_V3F:
         tflag = GL_TRUE;  cflag = GL_TRUE;  nflag = GL_FALSE;
         tcomps = 2;  ccomps = 3;  vcomps = 3;
         ctype = GL_FLOAT;
         coffset = 2*f;
         voffset = 5*f;
         defstride = 8*f;
         break;
      case GL_T2F_N3F_V3F:
         tflag = GL_TRUE;  cflag = GL_FALSE;  nflag = GL_TRUE;
         tcomps = 2;  ccomps = 0;  vcomps = 3;
         noffset = 2*f;
         voffset = 5*f;
         defstride = 8*f;
         break;
      case GL_T2F_C4F_N3F_V3F:
         tflag = GL_TRUE;  cflag = GL_TRUE;  nflag = GL_TRUE;
         tcomps = 2;  ccomps = 4;  vcomps = 3;
         ctype = GL_FLOAT;
         coffset = 2*f;
         noffset = 6*f;
         voffset = 9*f;
         defstride = 12*f;
         break;
      case GL_T4F_C4F_N3F_V4F:
         tflag = GL_TRUE;  cflag = GL_TRUE;  nflag = GL_TRUE;
         tcomps = 4;  ccomps = 4;  vcomps = 4;
         ctype = GL_FLOAT;
         coffset = 4*f;
         noffset = 8*f;
         voffset = 11*f;
         defstride = 15*f;
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glInterleavedArrays(format)" );
         return;
   }

   if (stride==0) {
      stride = defstride;
   }

   _mesa_DisableClientState( GL_EDGE_FLAG_ARRAY );
   _mesa_DisableClientState( GL_INDEX_ARRAY );

   /* Texcoords */
   coordUnitSave = ctx->Array.ActiveTexture;
   if (tflag) {
      GLint i;
#ifdef VAO
      GLint factor = ctx->Array.Current->TexCoordInterleaveFactor;
#else
      GLint factor = ctx->Array.TexCoordInterleaveFactor;
#endif
      for (i = 0; i < factor; i++) {
         _mesa_ClientActiveTextureARB( (GLenum) (GL_TEXTURE0_ARB + i) );
         _mesa_EnableClientState( GL_TEXTURE_COORD_ARRAY );
         glTexCoordPointer( tcomps, GL_FLOAT, stride,
                             (GLubyte *) pointer + i * coffset );
      }
      for (i = factor; i < (GLint) ctx->Const.MaxTextureUnits; i++) {
         _mesa_ClientActiveTextureARB( (GLenum) (GL_TEXTURE0_ARB + i) );
         _mesa_DisableClientState( GL_TEXTURE_COORD_ARRAY );
      }
   }
   else {
      GLint i;
      for (i = 0; i < (GLint) ctx->Const.MaxTextureUnits; i++) {
         _mesa_ClientActiveTextureARB( (GLenum) (GL_TEXTURE0_ARB + i) );
         _mesa_DisableClientState( GL_TEXTURE_COORD_ARRAY );
      }
   }
   /* Restore texture coordinate unit index */
   _mesa_ClientActiveTextureARB( (GLenum) (GL_TEXTURE0_ARB + coordUnitSave) );


   /* Color */
   if (cflag) {
      _mesa_EnableClientState( GL_COLOR_ARRAY );
      glColorPointer( ccomps, ctype, stride,
                       (GLubyte*) pointer + coffset );
   }
   else {
      _mesa_DisableClientState( GL_COLOR_ARRAY );
   }


   /* Normals */
   if (nflag) {
      _mesa_EnableClientState( GL_NORMAL_ARRAY );
      glNormalPointer( GL_FLOAT, stride,
                        (GLubyte*) pointer + noffset );
   }
   else {
      _mesa_DisableClientState( GL_NORMAL_ARRAY );
   }

   _mesa_EnableClientState( GL_VERTEX_ARRAY );
   glVertexPointer( vcomps, GL_FLOAT, stride,
                     (GLubyte *) pointer + voffset );
}



void
_mesa_DrawRangeElements(GLenum mode, GLuint start,
                        GLuint end, GLsizei count,
                        GLenum type, const GLvoid *indices)
{
   GET_CURRENT_CONTEXT(ctx);

   if (end < start) {
      gl_error(ctx, GL_INVALID_VALUE, "glDrawRangeElements( end < start )");
      return;
   }

#if 0
   /*
    * XXX something in locked arrays is broken!  If start = 0,
    * end = 1 and count = 2 we'll take the LockArrays path and
    * get incorrect results.  See Scott McMillan's bug of 3 Jan 2000.
    * For now, don't use locked arrays.
    */
   if (!ctx->Array.LockCount && 2*count > (GLint) 3*(end-start)) {
      glLockArraysEXT( start, end );
      glDrawElements( mode, count, type, indices );
      glUnlockArraysEXT();
   } else {
      glDrawElements( mode, count, type, indices );
   }
#else
   glDrawElements( mode, count, type, indices );
#endif
}



void gl_update_client_state( GLcontext *ctx )
{
   static GLuint sz_flags[5] = { 0, 
				 0,
				 VERT_OBJ_2, 
				 VERT_OBJ_23, 
				 VERT_OBJ_234 };

   static GLuint tc_flags[5] = { 0, 
				 VERT_TEX0_1,
				 VERT_TEX0_12, 
				 VERT_TEX0_123, 
				 VERT_TEX0_1234 };

#ifdef VAO
   ctx->Array.Current->Flags = 0;
   ctx->Array.Current->Summary = 0;
#else
   ctx->Array.Flags = 0;
   ctx->Array.Summary = 0;
#endif
   ctx->input->ArrayIncr = 0;
   
#ifdef VAO
   if (ctx->Array.Current->Normal.Enabled)      ctx->Array.Current->Flags |= VERT_NORM;
   if (ctx->Array.Current->Color.Enabled)       ctx->Array.Current->Flags |= VERT_RGBA;
   if (ctx->Array.Current->Index.Enabled)       ctx->Array.Current->Flags |= VERT_INDEX;
   if (ctx->Array.Current->EdgeFlag.Enabled)    ctx->Array.Current->Flags |= VERT_EDGE;
   if (ctx->Array.Current->Vertex.Enabled) {
      ctx->Array.Current->Flags |= sz_flags[ctx->Array.Current->Vertex.Size];
      ctx->input->ArrayIncr = 1;
   }
   if (ctx->Array.Current->TexCoord[0].Enabled) {
      ctx->Array.Current->Flags |= tc_flags[ctx->Array.Current->TexCoord[0].Size];
   }
   if (ctx->Array.Current->TexCoord[1].Enabled) {
      ctx->Array.Current->Flags |= (tc_flags[ctx->Array.Current->TexCoord[1].Size] << NR_TEXSIZE_BITS);
   }
#else
   if (ctx->Array.Normal.Enabled)      ctx->Array.Flags |= VERT_NORM;
   if (ctx->Array.Color.Enabled)       ctx->Array.Flags |= VERT_RGBA;
   if (ctx->Array.Index.Enabled)       ctx->Array.Flags |= VERT_INDEX;
   if (ctx->Array.EdgeFlag.Enabled)    ctx->Array.Flags |= VERT_EDGE;
   if (ctx->Array.Vertex.Enabled) {
      ctx->Array.Flags |= sz_flags[ctx->Array.Vertex.Size];
      ctx->input->ArrayIncr = 1;
   }
   if (ctx->Array.TexCoord[0].Enabled) {
      ctx->Array.Flags |= tc_flags[ctx->Array.TexCoord[0].Size];
   }
   if (ctx->Array.TexCoord[1].Enabled) {
      ctx->Array.Flags |= (tc_flags[ctx->Array.TexCoord[1].Size] << NR_TEXSIZE_BITS);
   }
#endif

   /* Not really important any more:
    */
#ifdef VAO
   ctx->Array.Current->Summary = ctx->Array.Current->Flags & VERT_DATA;
   ctx->input->ArrayAndFlags = ~ctx->Array.Current->Flags;
#else
   ctx->Array.Summary = ctx->Array.Flags & VERT_DATA;
   ctx->input->ArrayAndFlags = ~ctx->Array.Flags;
#endif
   ctx->input->ArrayEltFlush = !(ctx->CompileCVAFlag);
}



/**********************************************************************/
/* Vertex Array Objects extension                                     */
/**********************************************************************/

#ifdef VAO

struct gl_array_object *
_mesa_alloc_vertex_array_object(GLcontext *ctx, GLuint name)
{
   struct gl_array_object *arrayObj;

   arrayObj = MALLOC_STRUCT(gl_array_object);
   if (arrayObj) {
      GLuint i;
      arrayObj->Name = name;
      arrayObj->RefCount = 1;
      arrayObj->Vertex.Size = 4;
      arrayObj->Vertex.Type = GL_FLOAT;
      arrayObj->Vertex.Stride = 0;
      arrayObj->Vertex.StrideB = 0;
      arrayObj->Vertex.Ptr = NULL;
      arrayObj->Vertex.Enabled = GL_FALSE;
      arrayObj->Normal.Type = GL_FLOAT;
      arrayObj->Normal.Stride = 0;
      arrayObj->Normal.StrideB = 0;
      arrayObj->Normal.Ptr = NULL;
      arrayObj->Normal.Enabled = GL_FALSE;
      arrayObj->Color.Size = 4;
      arrayObj->Color.Type = GL_FLOAT;
      arrayObj->Color.Stride = 0;
      arrayObj->Color.StrideB = 0;
      arrayObj->Color.Ptr = NULL;
      arrayObj->Color.Enabled = GL_FALSE;
      arrayObj->Index.Type = GL_FLOAT;
      arrayObj->Index.Stride = 0;
      arrayObj->Index.StrideB = 0;
      arrayObj->Index.Ptr = NULL;
      arrayObj->Index.Enabled = GL_FALSE;
      for (i = 0; i < MAX_TEXTURE_UNITS; i++) {
         arrayObj->TexCoord[i].Size = 4;
         arrayObj->TexCoord[i].Type = GL_FLOAT;
         arrayObj->TexCoord[i].Stride = 0;
         arrayObj->TexCoord[i].StrideB = 0;
         arrayObj->TexCoord[i].Ptr = NULL;
         arrayObj->TexCoord[i].Enabled = GL_FALSE;
      }
      arrayObj->TexCoordInterleaveFactor = 1;
      arrayObj->EdgeFlag.Stride = 0;
      arrayObj->EdgeFlag.StrideB = 0;
      arrayObj->EdgeFlag.Ptr = NULL;
      arrayObj->EdgeFlag.Enabled = GL_FALSE;

      /* Put into hash table */
      _mesa_HashInsert(ctx->Shared->ArrayObjects, name, arrayObj);
   }
   return arrayObj;
}


void
glGenArraySetsEXT(GLsizei n, GLuint *arrayset)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint first;
   GLint i;

   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glGenArraySetsEXT");

   if (n < 0) {
      gl_error(ctx, GL_INVALID_VALUE, "glGenArraySetsEXT");
      return;
   }

   if (!arrayset)
      return;

   /*
    * This must be atomic (generation and allocation of texture IDs)
    */
   /*XXX _glthread_LOCK_MUTEX(GenTexturesLock);*/

   first = _mesa_HashFindFreeKeyBlock(ctx->Shared->ArrayObjects, n);

   /* Return the object names */
   for (i = 0; i < n; i++) {
      arrayset[i] = first + i;
   }

   /* Allocate new array objects */
   for (i = 0; i < n; i++) {
      struct gl_array_object *arrayObj;
      GLuint name = first + i;
      arrayObj = _mesa_alloc_vertex_array_object(ctx, name);
      if (!arrayObj) {
         gl_error(ctx, GL_OUT_OF_MEMORY, "glGenArraySetsEXT");
         break;
      }
   }

   /*XXX _glthread_UNLOCK_MUTEX(GenTexturesLock);*/
}


void
glBindArraySetEXT(GLuint arraySet)
{
   struct gl_array_object *arrayObj;
   GET_CURRENT_CONTEXT(ctx);

   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glBindArraySetEXT");

   if (arraySet < 0) {
      gl_error(ctx, GL_INVALID_OPERATION, "glBindArraySetEXT");
      return;
   }

   ctx->Array.Current->RefCount--;
   if (ctx->Array.Current->RefCount == 0) {
      _mesa_HashRemove(ctx->Shared->ArrayObjects, arraySet);
      FREE(ctx->Array.Current);
   }

   arrayObj = (struct gl_array_object *) _mesa_HashLookup(ctx->Shared->ArrayObjects, arraySet);
   if (arrayObj) {
      ctx->Array.Current = arrayObj;
   }
   else {
      /* create the array object and insert into hash table */
      arrayObj = _mesa_alloc_vertex_array_object(ctx, arraySet);
      if (!arrayObj) {
         gl_error(ctx, GL_OUT_OF_MEMORY, "glBindArraySetEXT");
         return;
      }
      ctx->Array.Current = (struct gl_array_object *) _mesa_HashLookup(ctx->Shared->ArrayObjects, 0);
   }

   ctx->Array.Current->RefCount++;

   /* Signal new state */
   /*   ctx->Array.NewArrayState = ~0;*/
   ctx->NewState |= NEW_CLIENT_STATE;

#if 0
   printf("Bind %d\n", ctx->Array.Current->Name);
   printf("Vertex array size: %d\n", ctx->Array.Current->Vertex.Size);
   printf("Vertex array enable: %d\n", ctx->Array.Current->Vertex.Enabled);

   printf("Normal array size: %d\n", ctx->Array.Current->Normal.Size);
   printf("Normal array enable: %d\n", ctx->Array.Current->Normal.Enabled);
   printf("Lock first %d  count %d\n", ctx->Array.Current->LockFirst,
          ctx->Array.Current->LockCount);
#endif
}


void
glDeleteArraySetsEXT(GLsizei n, const GLuint *arraySet)
{
   GLint i;
   GET_CURRENT_CONTEXT(ctx);
   
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glDeleteArraySetsEXT");

   if (n < 0) {
      gl_error(ctx, GL_INVALID_OPERATION, "glDeleteArraySetEXT");
      return;
   }

   for (i = 0; i < n; i++) {
      if (arraySet[i] > 0) {
         struct gl_array_object *arrayObj;
         arrayObj = (struct gl_array_object *) _mesa_HashLookup(ctx->Shared->ArrayObjects, arraySet[i]);
         if (arrayObj) {
            arrayObj->RefCount--;
            if (arrayObj->RefCount == 0) {
               _mesa_HashRemove(ctx->Shared->ArrayObjects, arraySet[i]);
               FREE(arrayObj);
            }
         }
      }
   }
}


GLboolean
glIsArraySetEXT(GLuint arraySet)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH_WITH_RETVAL(ctx, "glIsArraySetEXT",
						  GL_FALSE);

   if (arraySet > 0 && _mesa_HashLookup(ctx->Shared->ArrayObjects, arraySet)) {
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}

#endif
