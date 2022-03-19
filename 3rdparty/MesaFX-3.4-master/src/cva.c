/* $Id: cva.c,v 1.12.4.4 2001/05/15 20:19:13 brianp Exp $*/

/*
 * Mesa 3-D graphics library
 * Version:  3.4
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

/* Mesa CVA implementation.  
 * Copyright (C) 1999 Keith Whitwell
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "types.h"
#include "cva.h"
#include "context.h"
#include "macros.h"
#include "mem.h"
#include "pipeline.h"
#include "varray.h"
#include "vbcull.h"
#include "vbrender.h"
#include "vbxform.h"
#include "vector.h"
#endif



static void copy_clipmask( GLubyte *dest, GLubyte *ormask, GLubyte *andmask,
			   const GLubyte *src, 
			   const GLuint *elt, GLuint nr )
{
   GLuint i;
   GLubyte o = *ormask;
   GLubyte a = *andmask;

   for (i = 0 ; i < nr ; i++) {
      GLubyte t = src[elt[i]];
      dest[i] = t;
      o |= t;
      a &= t;
   }  

   *ormask = o;
   *andmask = a;
}

static void translate_4f( GLvector4f *dest, 
			  CONST GLvector4f *src, 
			  CONST GLuint elt[], 
			  GLuint nr )
{
   GLuint i;
   GLfloat (*from)[4] = (GLfloat (*)[4])src->start;
   GLfloat (*to)[4] = (GLfloat (*)[4])dest->start;
   GLuint stride = src->stride;
   
   if (stride == 4 * sizeof(GLfloat)) {
      for (i = 0 ; i < nr ; i++) 
	 COPY_4FV( to[i], from[elt[i]] );
   } else {
      for (i = 0 ; i < nr ; i++) {
	 CONST GLubyte *f = (GLubyte *)from + elt[i] * stride;
	 COPY_4FV( to[i], (GLfloat *)f );
      }
   }      

   dest->size = src->size;
   dest->flags |= (src->flags & VEC_SIZE_4);
   dest->count = nr;
}

static void translate_3f( GLvector3f *dest, 
			  CONST GLvector3f *src, 
			  CONST GLuint elt[], 
			  GLuint nr )
{
   GLuint i;
   GLfloat (*from)[3] = (GLfloat (*)[3])src->start;
   GLfloat (*to)[3] = (GLfloat (*)[3])dest->start;
   GLuint stride = src->stride;

   if (stride == 3 * sizeof(GLfloat)) {
      for (i = 0 ; i < nr ; i++) 
	 COPY_3FV( to[i], from[elt[i]] );
   } else {
      for (i = 0 ; i < nr ; i++) {
	 CONST GLubyte *f = (GLubyte *)from + elt[i] * stride;
	 COPY_3FV( to[i], (GLfloat *)f );
      }
   }

   dest->count = nr;
}

static void translate_4ub( GLvector4ub *dest, 
			   CONST GLvector4ub *src,
			   GLuint elt[], 
			   GLuint nr )
{
   GLuint i;
   GLubyte (*from)[4] = (GLubyte (*)[4])src->start;
   GLubyte (*to)[4] = (GLubyte (*)[4])dest->start;
   GLuint stride = src->stride;

   if (stride == 4 * sizeof(GLubyte)) {
      for (i = 0 ; i < nr ; i++)
	 COPY_4UBV( to[i], from[elt[i]]);
   } else {
      for (i = 0 ; i < nr ; i++) {
	 CONST GLubyte *f = (GLubyte *)from + elt[i] * stride;
	 COPY_4UBV( to[i], f );
      }
   }

   dest->count = nr;
}

static void translate_1ui( GLvector1ui *dest, 
			   GLvector1ui *src, 
			   GLuint elt[], 
			   GLuint nr )
{
   GLuint i;
   GLuint *from = src->start;
   GLuint *to = dest->start;
   GLuint stride = src->stride;

   if (stride == sizeof(GLuint)) {
      for (i = 0 ; i < nr ; i++)
	 to[i] = from[elt[i]];
   } else {
      for (i = 0 ; i < nr ; i++) {
	 CONST GLubyte *f = (GLubyte *)from + elt[i] * stride;
	 to[i] = *(GLuint *)f;
      }
   }

   dest->count = nr;
}

static void translate_1ub( GLvector1ub *dest, 
			   GLvector1ub *src, 
			   GLuint elt[], 
			   GLuint nr )
{
   GLuint i;
   GLubyte *from = src->start;
   GLubyte *to = dest->start;
   GLuint stride = src->stride;

   if (stride == sizeof(GLubyte)) {
      for (i = 0 ; i < nr ; i++)
	 to[i] = from[elt[i]];
   } else {
      for (i = 0 ; i < nr ; i++) {
	 CONST GLubyte *f = from + elt[i] * stride;
	 to[i] = *f;
      }
   }

   dest->count = nr;
}


/* The fallback case for handling the merge of cva and immediate
 * data.  This code back-copies data from cva->immediate to build a
 * normal looking immediate struct, which is then sent off to the
 * old raster setup and render routines.
 *
 * The FX driver implements a replacement for this step which merges
 * the new data into the already prepared GrVertex structs.
 *
 * When there is no data to merge, gl_render_vb_indirect is called
 * instead.
 */
void gl_merge_cva( struct vertex_buffer *VB,
		   struct vertex_buffer *cvaVB )
{
   GLcontext *ctx = VB->ctx;
   GLuint *elt = VB->EltPtr->start;
   GLuint count = VB->Count - VB->Start;
#ifdef VAO
   GLuint available = ctx->CVA.pre.outputs | ctx->Array.Current->Summary;
#else
   GLuint available = ctx->CVA.pre.outputs | ctx->Array.Summary;
#endif
   GLuint required = ctx->CVA.elt.inputs;
   GLuint flags;

   /* Should attempt to build an or-flag of reduced (ie rasterization)
    * prims in the VB, so a software points function doesn't screw us
    * when we're doing hardware triangles.
    */
   if ((required & VERT_SETUP_FULL) && 
       (ctx->IndirectTriangles & DD_SW_SETUP))
   {
      if (MESA_VERBOSE & VERBOSE_PIPELINE)
	 gl_print_vert_flags("extra flags for setup", 
			     ctx->RenderFlags & available & ~required);
      required |= ctx->RenderFlags;
   }

   flags = required & available;

   if ((flags & VERT_DATA) == 0)
      return;
   
   if (MESA_VERBOSE&VERBOSE_PIPELINE)
      gl_print_vert_flags("cva merge", flags); 

   if (flags & VERT_WIN) {
      VB->ClipPtr = &VB->Clip;
      VB->Projected = &VB->Win;
      VB->CullMode = 0;
   
      if (cvaVB->ClipOrMask) {

	 /* Copy clipmask back into VB, build a new clipOrMask */
	 copy_clipmask( VB->ClipMask + VB->Start, 
			&VB->ClipOrMask, &VB->ClipAndMask,
			cvaVB->ClipMask, 
			elt, 
			VB->Count - VB->Start );
	 
	 /* overkill if !VB->ClipOrMask - should just copy 'copied' verts */
	 translate_4f( VB->ClipPtr, cvaVB->ClipPtr, elt, count);

	 if (VB->ClipOrMask & CLIP_USER_BIT) {
	    GLubyte orMask = 0, andMask = ~0;

	    copy_clipmask( VB->UserClipMask + VB->Start,
			   &orMask, &andMask,
			   cvaVB->UserClipMask,
			   elt,
			   VB->Count - VB->Start);
	    
	    if (andMask)
               VB->ClipAndMask |= CLIP_USER_BIT;
	 }

	 if (VB->ClipOrMask) 
	    VB->CullMode |= CLIP_MASK_ACTIVE;      

	 if (VB->ClipAndMask) {
	    VB->Culled = 1;
	    gl_dont_cull_vb( VB );   
	    return;
	 }
      }

      translate_4f( &VB->Win, &cvaVB->Win, elt, count );

      /* Can't be precomputed - but may be wasteful to do this now.
       */
      if (ctx->IndirectTriangles & DD_ANY_CULL)
      {
	 GLuint cullcount = gl_cull_vb( VB );      
	 if (cullcount) VB->CullMode |= CULL_MASK_ACTIVE;
	 if (cullcount == VB->Count) { VB->Culled = 2 ; return; }
      }
      else 
	 gl_dont_cull_vb( VB );   
   } else {
      VB->ClipPtr = &VB->Clip;
      VB->Projected = &VB->Win;
   }

   if (flags & VERT_EYE) 
   {
      VB->Unprojected = VB->EyePtr = &VB->Eye;
      translate_4f( VB->EyePtr, cvaVB->EyePtr, elt, count);
   }

   if (flags & VERT_OBJ_ANY) 
   {
      VB->ObjPtr = &VB->IM->v.Obj;
      if (!ctx->NeedEyeCoords) VB->Unprojected = VB->ObjPtr;
      translate_4f( VB->ObjPtr, cvaVB->ObjPtr, elt, count);
   }

   if (flags & VERT_NORM) 
   {
      VB->NormalPtr = &VB->IM->v.Normal;      
      translate_3f( VB->NormalPtr, cvaVB->NormalPtr, elt, count );
      VB->CullMode &= ~COMPACTED_NORMALS;
   }

   if (flags & VERT_RGBA) 
   {
      VB->ColorPtr = VB->Color[0] = VB->LitColor[0];
      translate_4ub( VB->Color[0], cvaVB->Color[0], elt, count );

      if (ctx->TriangleCaps & DD_TRI_LIGHT_TWOSIDE) {
	 VB->Color[1] = VB->LitColor[1];
	 translate_4ub( VB->Color[1], cvaVB->Color[1], elt, count );
      }
   }
   
   if (flags & VERT_INDEX) 
   {
      VB->IndexPtr = VB->Index[0] = VB->LitIndex[0];
      translate_1ui( VB->Index[0], cvaVB->Index[0], elt, count );

      if (ctx->TriangleCaps & DD_TRI_LIGHT_TWOSIDE) {
	 VB->Index[1] = VB->LitIndex[1];
	 translate_1ui( VB->Index[1], cvaVB->Index[1], elt, count );
      }
   }

   if (flags & VERT_EDGE) 
   {
      VB->EdgeFlagPtr = &VB->IM->v.EdgeFlag;      
      translate_1ub( VB->EdgeFlagPtr, cvaVB->EdgeFlagPtr, elt, count );
   }

   if (flags & VERT_TEX0_ANY) 
   {
      VB->TexCoordPtr[0] = &VB->IM->v.TexCoord[0];
      translate_4f( VB->TexCoordPtr[0], cvaVB->TexCoordPtr[0], elt, count);
   }

   if (flags & VERT_TEX1_ANY) 
   {
      VB->TexCoordPtr[1] = &VB->IM->v.TexCoord[1];      
      translate_4f( VB->TexCoordPtr[1], cvaVB->TexCoordPtr[1], elt, count);
   }
}




/* We don't have a good mechanism for dealing with the situation where
 * cva is 'abandoned' midway through a vertex buffer, or indeed any mixing
 * of cva & standard data in a single immediate struct.
 *
 * Basically just abandon CVA even though the precalced data
 * is already there.
 */
void gl_rescue_cva( GLcontext *ctx, struct immediate *IM )
{
   struct vertex_buffer *VB = ctx->VB;

   IM->Start = VB->CopyStart;

   ctx->CompileCVAFlag = 0;
   gl_build_immediate_pipeline( ctx );
   gl_exec_array_elements( ctx, IM, IM->Start, IM->Count );
}





/* Transform the array components now, upto the setup call.  When
 * actual draw commands arrive, the data will be merged prior to
 * calling render_vb.  
 */
void
_mesa_LockArraysEXT(GLint first, GLsizei count)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH( ctx, "unlock arrays" );

   if (MESA_VERBOSE & VERBOSE_API)
      fprintf(stderr, "glLockArrays %d %d\n", first, count);

   /* Can't mix locked & unlocked - if count is too large, just
    * unlock.  
    */
   if (first == 0 && 
       count > 0 && 
       count <= (GLint) ctx->Const.MaxArrayLockSize)
   {   
      struct gl_cva *cva = &ctx->CVA;

#ifdef VAO
      if (!ctx->Array.Current->LockCount) {
	 ctx->Array.NewArrayState = ~0;
	 ctx->CVA.lock_changed ^= 1;
	 ctx->NewState |= NEW_CLIENT_STATE;
      }

      ctx->Array.Current->LockFirst = first;
      ctx->Array.Current->LockCount = count;     
      ctx->CompileCVAFlag = !ctx->CompileFlag;
#else
      if (!ctx->Array.LockCount) {
	 ctx->Array.NewArrayState = ~0;
	 ctx->CVA.lock_changed ^= 1;
	 ctx->NewState |= NEW_CLIENT_STATE;
      }

      ctx->Array.LockFirst = first;
      ctx->Array.LockCount = count;     
      ctx->CompileCVAFlag = !ctx->CompileFlag;
#endif

      if (!cva->VB) {
	 cva->VB = gl_vb_create_for_cva( ctx, ctx->Const.MaxArrayLockSize );
	 gl_alloc_cva_store( cva, cva->VB->Size );
	 gl_reset_cva_vb( cva->VB, ~0 );
      }
   } 
   else
   {
#ifdef VAO
      if (ctx->Array.Current->LockCount) {
#else
      if (ctx->Array.LockCount) {
#endif
	 ctx->CVA.lock_changed ^= 1;
	 ctx->NewState |= NEW_CLIENT_STATE;
      }


#ifdef VAO
      ctx->Array.Current->LockFirst = 0;
      ctx->Array.Current->LockCount = 0;
#else
      ctx->Array.LockFirst = 0;
      ctx->Array.LockCount = 0;
#endif
      ctx->CompileCVAFlag = 0;
   }
}




void
_mesa_UnlockArraysEXT( void )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH( ctx, "unlock arrays" );

   if (MESA_VERBOSE & VERBOSE_API)
      fprintf(stderr, "glUnlockArrays\n");

#ifdef VAO
   if (ctx->Array.Current->LockCount) {
#else
   if (ctx->Array.LockCount) {
#endif
      ctx->CVA.lock_changed ^= 1;
      ctx->NewState |= NEW_CLIENT_STATE;
   }

#ifdef VAO
   ctx->Array.Current->LockFirst = 0;
   ctx->Array.Current->LockCount = 0;
#else
   ctx->Array.LockFirst = 0;
   ctx->Array.LockCount = 0;
#endif
   ctx->CompileCVAFlag = 0;
}


/* This storage used to hold translated client data if type or stride
 * need to be fixed.
 */
void gl_alloc_cva_store( struct gl_cva *cva, GLuint size )
{
   cva->store.Obj = (GLfloat (*)[4])MALLOC( sizeof(GLfloat) * 4 * size );
   cva->store.Normal = (GLfloat (*)[3])MALLOC( sizeof(GLfloat) * 3 * size );
   cva->store.Color = (GLubyte (*)[4])MALLOC( sizeof(GLubyte) * 4 * size );
   cva->store.EdgeFlag = (GLubyte *)MALLOC( sizeof(GLubyte) * size );
   cva->store.Index = (GLuint *)MALLOC( sizeof(GLuint) * size );
   cva->store.TexCoord[0] = (GLfloat (*)[4])MALLOC( sizeof(GLfloat) * 4 * size);
   cva->store.TexCoord[1] = (GLfloat (*)[4])MALLOC( sizeof(GLfloat) * 4 * size);
   cva->store.Elt = (GLuint *)MALLOC( sizeof(GLuint) * size );
   cva->elt_size = size;
}

void gl_free_cva_store( struct gl_cva *cva )
{
   FREE( cva->store.Obj );
   FREE( cva->store.Normal );
   FREE( cva->store.Color );
   FREE( cva->store.EdgeFlag );
   FREE( cva->store.Index );
   FREE( cva->store.TexCoord[0] );
   FREE( cva->store.TexCoord[1] );
   FREE( cva->store.Elt );
}


void gl_prepare_arrays_cva( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   struct gl_cva *cva = &ctx->CVA;
#ifdef VAO
   GLuint start = ctx->Array.Current->LockFirst;
   GLuint n = ctx->Array.Current->LockCount;
   GLuint enable = ((ctx->Array.NewArrayState & ctx->Array.Current->Summary) |
		    VB->pipeline->fallback);
   GLuint disable = ctx->Array.NewArrayState & ~enable;
#else
   GLuint start = ctx->Array.LockFirst;
   GLuint n = ctx->Array.LockCount;
   GLuint enable = ((ctx->Array.NewArrayState & ctx->Array.Summary) |
		    VB->pipeline->fallback);
   GLuint disable = ctx->Array.NewArrayState & ~enable;
#endif
   GLuint i;

   if (MESA_VERBOSE&VERBOSE_PIPELINE) {
      gl_print_vert_flags("*** ENABLE", enable);
      gl_print_vert_flags("*** DISABLE", disable);
   }

   if (enable) 
   {
      struct gl_client_array *client_data;
      GLuint fallback = VB->pipeline->fallback;
   
      if (enable & VERT_ELT) 
      {
	 GLvector1ui *elt = VB->EltPtr = &cva->v.Elt;

	 if (cva->Elt.Type == GL_UNSIGNED_INT)
	 {
	    elt->data = (GLuint *) cva->Elt.Ptr;
	    elt->stride = sizeof(GLuint);
	    elt->flags = 0;
	 } else {
	    elt->data = cva->store.Elt;
	    elt->stride = sizeof(GLuint);

	    if (cva->elt_count > cva->elt_size) 
	    {
	       while (cva->elt_count > (cva->elt_size *= 2)) {};
	       FREE(cva->store.Elt);
	       cva->store.Elt = (GLuint *) MALLOC(cva->elt_size * 
						  sizeof(GLuint));
	    }
	    cva->EltFunc( elt->data, &cva->Elt, 0, cva->elt_count );
	 }	
	 elt->start = VEC_ELT(elt, GLuint, 0); 
	 elt->count = cva->elt_count;

#ifdef VAO
	 fallback |= (cva->pre.new_inputs & ~ctx->Array.Current->Summary);
#else
	 fallback |= (cva->pre.new_inputs & ~ctx->Array.Summary);
#endif
	 enable |= fallback;
	 disable &= ~fallback;
	 if (MESA_VERBOSE&VERBOSE_PIPELINE) {
	    gl_print_vert_flags("*** NEW INPUTS", cva->pre.new_inputs);
	    gl_print_vert_flags("*** FALLBACK", fallback);
	 }
      }

      if (enable & VERT_RGBA) 
      {
	 GLvector4ub *col = &cva->v.Color;

#ifdef VAO
	 client_data = &ctx->Array.Current->Color;
#else
	 client_data = &ctx->Array.Color;
#endif
	 if (fallback & VERT_RGBA) client_data = &ctx->Fallback.Color;

	 VB->Color[0] = VB->Color[1] = VB->ColorPtr = &cva->v.Color;
      
	 if (client_data->Type != GL_UNSIGNED_BYTE ||
	     client_data->Size != 4)
	 {
	    col->data = cva->store.Color;
	    col->stride = 4 * sizeof(GLubyte);
#ifdef VAO
	    ctx->Array.Current->ColorFunc( col->data, client_data, start, n );
#else
	    ctx->Array.ColorFunc( col->data, client_data, start, n );
#endif
	    col->flags = VEC_WRITABLE|VEC_GOOD_STRIDE;
	 } else {
	    col->data = (GLubyte (*)[4]) client_data->Ptr;
	    col->stride = client_data->StrideB;
	    col->flags = VEC_NOT_WRITABLE|VEC_GOOD_STRIDE;
	    if (client_data->StrideB != 4 * sizeof(GLubyte)) 
	       col->flags ^= VEC_STRIDE_FLAGS;
	 }	
	 col->start = VEC_ELT(col, GLubyte, start); 
	 col->count = n;
      }
   
      if (enable & VERT_INDEX) 
      {
	 GLvector1ui *index = VB->IndexPtr = &cva->v.Index; 
	 VB->Index[0] = VB->Index[1] = VB->IndexPtr = &cva->v.Index;

#ifdef VAO
	 client_data = &ctx->Array.Current->Index;
#else
	 client_data = &ctx->Array.Index;
#endif
	 if (fallback & VERT_INDEX) client_data = &ctx->Fallback.Index;

	 if (client_data->Type != GL_UNSIGNED_INT)
	 {
	    index->data = cva->store.Index;
	    index->stride = sizeof(GLuint);
#ifdef VAO
	    ctx->Array.Current->IndexFunc( index->data, client_data, start, n );
#else
	    ctx->Array.IndexFunc( index->data, client_data, start, n );
#endif
	    index->flags = VEC_WRITABLE|VEC_GOOD_STRIDE;
	 } else {
	    index->data = (GLuint *) client_data->Ptr;
	    index->stride = client_data->StrideB;
	    index->flags = VEC_NOT_WRITABLE|VEC_GOOD_STRIDE; 
	    if (index->stride != sizeof(GLuint))
	       index->flags ^= VEC_STRIDE_FLAGS;
	 }	
	 index->count = n;
	 index->start = VEC_ELT(index, GLuint, start); 
      }

      for (i = 0 ; i < ctx->Const.MaxTextureUnits ; i++)
	 if (enable & PIPE_TEX(i)) {
	    GLvector4f *tc = VB->TexCoordPtr[i] = &cva->v.TexCoord[i];

#ifdef VAO
	    client_data = &ctx->Array.Current->TexCoord[i];
#else
	    client_data = &ctx->Array.TexCoord[i];
#endif

	    if (fallback & PIPE_TEX(i)) {
	       client_data = &ctx->Fallback.TexCoord[i];
	       client_data->Size = gl_texcoord_size( ctx->Current.Flag, i );
	    }

	    /* Writeability and stride handled lazily by
	     * gl_import_client_data(). 
	     */
	    if (client_data->Type == GL_FLOAT)
	    {
	       tc->data = (GLfloat (*)[4]) client_data->Ptr;
	       tc->stride = client_data->StrideB;
	       tc->flags = VEC_NOT_WRITABLE|VEC_GOOD_STRIDE;
	       if (tc->stride != 4 * sizeof(GLfloat)) 
		  tc->flags ^= VEC_STRIDE_FLAGS;
	    } else {
	       tc->data = cva->store.TexCoord[i];
	       tc->stride = 4 * sizeof(GLfloat);
#ifdef VAO
	       ctx->Array.Current->TexCoordFunc[i]( tc->data, client_data, start, n );
#else
	       ctx->Array.TexCoordFunc[i]( tc->data, client_data, start, n );
#endif
	       tc->flags = VEC_WRITABLE|VEC_GOOD_STRIDE;
	    }		 
	    tc->count = n;
	    tc->start = VEC_ELT(tc, GLfloat, start); 
	    tc->size = client_data->Size;
	 }
      
      if (enable & VERT_OBJ_ANY) 
      {
	 GLvector4f *obj = VB->ObjPtr = &cva->v.Obj;

#ifdef VAO
	 if (ctx->Array.Current->Vertex.Type == GL_FLOAT)
	 {
	    obj->data = (GLfloat (*)[4]) ctx->Array.Current->Vertex.Ptr;
	    obj->stride = ctx->Array.Current->Vertex.StrideB;
	    obj->flags = VEC_NOT_WRITABLE|VEC_GOOD_STRIDE;
	    if (obj->stride != 4 * sizeof(GLfloat)) 
	       obj->flags ^= VEC_STRIDE_FLAGS;
	 } else {
	    obj->data = cva->store.Obj;
	    obj->stride = 4 * sizeof(GLfloat);
	    ctx->Array.Current->VertexFunc( obj->data, &ctx->Array.Current->Vertex, start, n );
	    obj->flags = VEC_WRITABLE|VEC_GOOD_STRIDE;
	 }
	 obj->count = n;
	 obj->start = VEC_ELT(obj, GLfloat, start);
	 obj->size = ctx->Array.Current->Vertex.Size;
#else
	 if (ctx->Array.Vertex.Type == GL_FLOAT)
	 {
	    obj->data = (GLfloat (*)[4]) ctx->Array.Vertex.Ptr;
	    obj->stride = ctx->Array.Vertex.StrideB;
	    obj->flags = VEC_NOT_WRITABLE|VEC_GOOD_STRIDE;
	    if (obj->stride != 4 * sizeof(GLfloat)) 
	       obj->flags ^= VEC_STRIDE_FLAGS;
	 } else {
	    obj->data = cva->store.Obj;
	    obj->stride = 4 * sizeof(GLfloat);
	    ctx->Array.VertexFunc( obj->data, &ctx->Array.Vertex, start, n );
	    obj->flags = VEC_WRITABLE|VEC_GOOD_STRIDE;
	 }
	 obj->count = n;
	 obj->start = VEC_ELT(obj, GLfloat, start);
	 obj->size = ctx->Array.Vertex.Size;
#endif
      }

      if (enable & VERT_NORM) 
      {
	 GLvector3f *norm = VB->NormalPtr = &cva->v.Normal;

#ifdef VAO
	 client_data = &ctx->Array.Current->Normal;
#else
	 client_data = &ctx->Array.Normal;
#endif

	 if (fallback & VERT_NORM) 
	    client_data = &ctx->Fallback.Normal;

	 /* Never need to write to normals, and we can always cope with stride.
	  */
	 if (client_data->Type == GL_FLOAT) {
	    norm->data = (GLfloat (*)[3]) client_data->Ptr;
	    norm->stride = client_data->StrideB;
	 } else {
	    norm->data = cva->store.Normal;
	    norm->stride = 3 * sizeof(GLfloat);
#ifdef VAO
	    ctx->Array.Current->NormalFunc( norm->data, client_data, start, n );
#else
	    ctx->Array.NormalFunc( norm->data, client_data, start, n );
#endif
	 }
	 norm->flags = 0;
	 norm->count = n;
	 norm->start = VEC_ELT(norm, GLfloat, start);
      }

      if (enable & VERT_EDGE) 
      {
	 GLvector1ub *edge = VB->EdgeFlagPtr = &cva->v.EdgeFlag;

#ifdef VAO
	 client_data = &ctx->Array.Current->EdgeFlag;
#else
	 client_data = &ctx->Array.EdgeFlag;
#endif

	 if (fallback & VERT_EDGE) 
	    client_data = &ctx->Fallback.EdgeFlag;

	 edge->data = (GLboolean *) client_data->Ptr;
	 edge->stride = client_data->StrideB;
	 edge->flags = VEC_NOT_WRITABLE|VEC_GOOD_STRIDE;
	 if (edge->stride != sizeof(GLubyte))
	    edge->flags ^= VEC_STRIDE_FLAGS;

	 edge->count = n;
	 edge->start = VEC_ELT(edge, GLubyte, start);
      }
   }

   if (disable) {
      if (disable & VERT_RGBA) cva->v.Color = *VB->LitColor[0];
      if (disable & VERT_INDEX) cva->v.Index = *VB->LitIndex[0];
      if (disable & VERT_NORM) cva->v.Normal = *VB->store.Normal;
      if (disable & VERT_OBJ_ANY) cva->v.Obj = *VB->store.Obj;
      if (disable & VERT_TEX0_ANY) cva->v.TexCoord[0]= *(VB->store.TexCoord[0]);
      if (disable & VERT_TEX1_ANY) cva->v.TexCoord[1]= *(VB->store.TexCoord[1]);
      if (disable & VERT_EDGE) cva->v.EdgeFlag = *VB->store.EdgeFlag;
   }
      
   VB->Flag[VB->Count] &= ~VERT_END_VB;
   VB->Count = n;
      	 
   if (ctx->Enabled & ENABLE_LIGHT) 
   {
#ifdef VAO
      if (ctx->Array.Current->Flags != VB->Flag[0])
#else
      if (ctx->Array.Flags != VB->Flag[0])
#endif
	 VB->FlagMax = 0;
      
      if (VB->FlagMax < n) {
	 for (i = VB->FlagMax ; i < n ; i++) 
#ifdef VAO
	    VB->Flag[i] = ctx->Array.Current->Flags;
#else
	    VB->Flag[i] = ctx->Array.Flags;
#endif
	 VB->Flag[i] = 0;
	 VB->FlagMax = n;
      }

      VB->Flag[n] |= VERT_END_VB;
   }
}

void gl_cva_force_precalc( GLcontext *ctx )
{
   struct gl_cva *cva = &ctx->CVA;

   if (cva->pre.changed_ops) 
      gl_reset_cva_vb( cva->VB, cva->pre.changed_ops );

   gl_run_pipeline( cva->VB );

   ctx->Array.NewArrayState = 0;
}


/* Simplified: just make sure the pipelines are valid.
 */
void gl_cva_compile_cassette( GLcontext *ctx, struct immediate *IM )
{
   struct gl_cva *cva = &ctx->CVA;
   cva->orflag |= IM->OrFlag;

   /* Allow pipeline recalculation based on inputs received from client.
    */
   if (IM->OrFlag & (cva->pre.forbidden_inputs|cva->elt.forbidden_inputs)) 
   {
      if (IM->OrFlag & cva->pre.forbidden_inputs) 
      {
	 cva->pre.pipeline_valid = 0;
	 cva->pre.data_valid = 0;
	 cva->pre.forbidden_inputs = 0;
      }

      if ((IM->OrFlag & cva->elt.forbidden_inputs))
      {
	 cva->elt.forbidden_inputs = 0;
      }

      cva->elt.pipeline_valid = 0;
   }

   /* Recalculate CVA data if necessary.
    */
   if (ctx->CompileCVAFlag && !cva->pre.data_valid) 
   {
      if (!cva->pre.pipeline_valid) 
	 gl_build_precalc_pipeline( ctx );
   
      gl_cva_force_precalc( ctx );
   }

   /* Build immediate pipeline if necessary.
    */
   if (!cva->elt.pipeline_valid)
      gl_build_immediate_pipeline( ctx );

   gl_fixup_input( ctx, IM );
   gl_execute_cassette( ctx, IM );      
}


void gl_cva_init( GLcontext *ctx )
{
   (void) ctx;
}
