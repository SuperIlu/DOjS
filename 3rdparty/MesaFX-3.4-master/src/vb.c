/* $Id: vb.c,v 1.15.4.1 2000/10/18 15:11:03 brianp Exp $ */

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


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "mem.h"
#include "types.h"
#include "vb.h"
#include "vbxform.h"
#include "xform.h"
#endif


/*
 * Allocate and initialize a vertex buffer.
 */
struct vertex_buffer *gl_vb_create_for_immediate( GLcontext *ctx )
{
   struct vertex_buffer *VB;
   struct immediate *IM;
   const GLuint alignment = 32;

   VB = ALIGN_CALLOC_STRUCT( vertex_buffer, alignment );
   if (!VB) 
      return 0;

   VB->ctx = ctx;
   VB->ClipAndMask = CLIP_ALL_BITS;
   VB->Size = VB_SIZE;
   VB->FirstFree = VB_MAX;
   VB->Type = VB_IMMEDIATE;
   VB->Start = VB_START;
   VB->pipeline = &ctx->CVA.elt;

   gl_vector4f_alloc( &VB->Eye, 2, VEC_WRITABLE, VB_SIZE, alignment );
   gl_vector4f_alloc( &VB->Clip, 2, VEC_WRITABLE, VB_SIZE, alignment );
   gl_vector4f_alloc( &VB->Win, 2, VEC_WRITABLE, VB_SIZE, alignment );
   gl_vector4ub_alloc( &VB->BColor, VEC_WRITABLE, VB_SIZE, alignment );      
   gl_vector1ui_alloc( &VB->BIndex, VEC_WRITABLE, VB_SIZE, alignment );

   VB->ClipMask = (GLubyte *) ALIGN_MALLOC(sizeof(GLubyte) * VB_SIZE, alignment);
   VB->UserClipMask = (GLubyte *) ALIGN_CALLOC(sizeof(GLubyte) * VB_SIZE, alignment);
   VB->CullMask = (GLubyte *) ALIGN_MALLOC(sizeof(GLubyte) * VB_SIZE, alignment);
   VB->NormCullMask = (GLubyte *) ALIGN_MALLOC(sizeof(GLubyte) * VB_SIZE, alignment);
   VB->Spec[0] = (GLubyte (*)[4]) ALIGN_MALLOC(sizeof(GLubyte) * 4 * VB_SIZE, alignment);
   VB->Spec[1] = (GLubyte (*)[4]) ALIGN_MALLOC(sizeof(GLubyte) * 4 * VB_SIZE, alignment);

   IM = VB->IM = gl_immediate_alloc(ctx);
      
   VB->store.Obj = &IM->v.Obj;
   VB->store.Normal = &IM->v.Normal;
   VB->store.Color = 0;		/* not used */
   VB->store.Index = 0;		/* not used */
   VB->store.EdgeFlag = &IM->v.EdgeFlag;
   VB->store.TexCoord[0] = &IM->v.TexCoord[0];
   VB->store.TexCoord[1] = &IM->v.TexCoord[1];
   VB->store.Elt = &IM->v.Elt;
      
   VB->LitColor[0] = VB->FoggedColor[0] = &IM->v.Color;
   VB->LitColor[1] = VB->FoggedColor[1] = &VB->BColor;

   VB->LitIndex[0] = VB->FoggedIndex[0] = &IM->v.Index;
   VB->LitIndex[1] = VB->FoggedIndex[1] = &VB->BIndex;
   

   VB->prev_buffer = IM;
   /* prev_buffer is now also a reference for the immediate buffer */
   IM->ref_count++;

   if (ctx->Driver.RegisterVB)
      ctx->Driver.RegisterVB( VB );

   return VB;
}



struct vertex_buffer *gl_vb_create_for_cva( GLcontext *ctx, GLuint size )
{
   struct vertex_buffer *VB;
   GLuint alignment = 32;

   VB = ALIGN_CALLOC_STRUCT( vertex_buffer, alignment );
   if (!VB) 
      return 0;

   /* set non-zero fields */

   VB->ctx = ctx;
   VB->Type = VB_CVA_PRECALC;
   VB->FirstFree = size;
   VB->CullDone = 1;

   size += VB_MAX_CLIPPED_VERTS;
   VB->Size = size;
   VB->ClipAndMask = CLIP_ALL_BITS;
   VB->pipeline = &ctx->CVA.pre;

   VB->ClipMask = (GLubyte *)ALIGN_MALLOC( sizeof(GLubyte) * size, alignment );
   VB->UserClipMask = (GLubyte *)ALIGN_CALLOC( sizeof(GLubyte) * size,
					       alignment );
   VB->Spec[0] = (GLubyte (*)[4])ALIGN_MALLOC( sizeof(GLubyte) * 4 * size,
					       alignment );
   VB->Spec[1] = (GLubyte (*)[4])ALIGN_MALLOC( sizeof(GLubyte) * 4 * size,
					       alignment );
   VB->Flag = (GLuint *)ALIGN_MALLOC( sizeof(GLuint) * size, alignment );

   gl_vector4f_alloc( &VB->Eye, 2, VEC_WRITABLE, size, alignment );
   gl_vector4f_alloc( &VB->Clip, 2, VEC_WRITABLE, size, alignment );
   gl_vector4f_alloc( &VB->Win, 2, VEC_WRITABLE, size, alignment );

   VB->store.Obj         = (GLvector4f *)  CALLOC(sizeof(GLvector4f));
   VB->store.Normal      = (GLvector3f *)  CALLOC(sizeof(GLvector3f));
   VB->store.Color       = 0;
   VB->store.Index       = 0;
   VB->store.EdgeFlag    = (GLvector1ub *) CALLOC(sizeof(GLvector1ub));
   VB->store.TexCoord[0] = (GLvector4f *)  CALLOC(sizeof(GLvector4f));
   VB->store.TexCoord[1] = (GLvector4f *)  CALLOC(sizeof(GLvector4f));
   VB->store.Elt         = (GLvector1ui *) CALLOC(sizeof(GLvector1ui));
   VB->LitColor[0]       = (GLvector4ub *) CALLOC(sizeof(GLvector4ub));
   VB->LitColor[1]       = (GLvector4ub *) CALLOC(sizeof(GLvector4ub));
   VB->LitIndex[0]       = (GLvector1ui *) CALLOC(sizeof(GLvector1ui));
   VB->LitIndex[1]       = (GLvector1ui *) CALLOC(sizeof(GLvector1ui));
   VB->FoggedColor[0]    = (GLvector4ub *) CALLOC(sizeof(GLvector4ub));
   VB->FoggedColor[1]    = (GLvector4ub *) CALLOC(sizeof(GLvector4ub));
   VB->FoggedIndex[0]    = (GLvector1ui *) CALLOC(sizeof(GLvector1ui));
   VB->FoggedIndex[1]    = (GLvector1ui *) CALLOC(sizeof(GLvector1ui));

   VB->ColorPtr = VB->Color[0] = VB->LitColor[0];
   VB->IndexPtr = VB->Index[0] = VB->LitIndex[0];
   VB->Specular = VB->Spec[0];
   VB->TexCoordPtr[0] = VB->store.TexCoord[0];
   VB->TexCoordPtr[1] = VB->store.TexCoord[1];
   VB->EdgeFlagPtr = VB->store.EdgeFlag;
   VB->NormalPtr = VB->store.Normal;
   VB->ObjPtr = VB->store.Obj;
   VB->EltPtr = VB->store.Elt;
      
   gl_vector4f_alloc( VB->store.Obj, 2, VEC_WRITABLE, size, alignment );
   gl_vector3f_alloc( VB->store.Normal, VEC_WRITABLE, size, alignment );
   gl_vector1ub_alloc( VB->store.EdgeFlag, VEC_WRITABLE, size, alignment );
   gl_vector4f_alloc( VB->store.TexCoord[0], 2, VEC_WRITABLE, size, alignment );
   gl_vector4f_alloc( VB->store.TexCoord[1], 2, VEC_WRITABLE, size, alignment );

   /* TODO: allocate these on demand.
    */
   gl_vector4ub_alloc( VB->LitColor[0], VEC_WRITABLE, size, alignment );
   gl_vector4ub_alloc( VB->LitColor[1], VEC_WRITABLE, size, alignment );
   gl_vector1ui_alloc( VB->LitIndex[0], VEC_WRITABLE, size, alignment );
   gl_vector1ui_alloc( VB->LitIndex[1], VEC_WRITABLE, size, alignment );
   gl_vector4ub_alloc( VB->FoggedColor[0], VEC_WRITABLE, size, alignment );
   gl_vector4ub_alloc( VB->FoggedColor[1], VEC_WRITABLE, size, alignment );
   gl_vector1ui_alloc( VB->FoggedIndex[0], VEC_WRITABLE, size, alignment );
   gl_vector1ui_alloc( VB->FoggedIndex[1], VEC_WRITABLE, size, alignment );



   VB->prev_buffer = 0;
   VB->Start = 0;

   if (ctx->Driver.RegisterVB)
      ctx->Driver.RegisterVB( VB );

   return VB;
}

void gl_vb_free( struct vertex_buffer *VB )
{
   gl_vector4f_free( &VB->Eye );
   gl_vector4f_free( &VB->Clip );
   gl_vector4f_free( &VB->Win );
   gl_vector4ub_free( &VB->BColor );
   gl_vector1ui_free( &VB->BIndex );

   if ( VB->prev_buffer && ! --VB->prev_buffer->ref_count )
      gl_immediate_free( VB->prev_buffer );

   if (VB->IM) {
      if ( ! --VB->IM->ref_count )
	 gl_immediate_free( VB->IM );

      ALIGN_FREE( VB->CullMask );
      ALIGN_FREE( VB->NormCullMask );
   } else {
      gl_vector4f_free( VB->store.Obj );          FREE( VB->store.Obj );
      gl_vector3f_free( VB->store.Normal );       FREE( VB->store.Normal );
      gl_vector1ub_free( VB->store.EdgeFlag );    FREE( VB->store.EdgeFlag );
      gl_vector4f_free( VB->store.TexCoord[0] );  FREE( VB->store.TexCoord[0]);
      gl_vector4f_free( VB->store.TexCoord[1] );  FREE( VB->store.TexCoord[1]);

      gl_vector4ub_free( VB->LitColor[0] );       FREE( VB->LitColor[0] );
      gl_vector4ub_free( VB->LitColor[1] );       FREE( VB->LitColor[1] );
      gl_vector1ui_free( VB->LitIndex[0] );       FREE( VB->LitIndex[0] );
      gl_vector1ui_free( VB->LitIndex[1] );       FREE( VB->LitIndex[1] );

      gl_vector4ub_free( VB->FoggedColor[0] );    FREE( VB->FoggedColor[0] );
      gl_vector4ub_free( VB->FoggedColor[1] );    FREE( VB->FoggedColor[1] );
      gl_vector1ui_free( VB->FoggedIndex[0] );    FREE( VB->FoggedIndex[0] );
      gl_vector1ui_free( VB->FoggedIndex[1] );    FREE( VB->FoggedIndex[1] );

      ALIGN_FREE( VB->Flag );
   }

   if (VB->tmp_f) FREE( VB->tmp_f );
   if (VB->tmp_m) FREE( VB->tmp_m );
   if (VB->EvaluatedFlags) FREE( VB->EvaluatedFlags );

   ALIGN_FREE( VB->Spec[0] );
   ALIGN_FREE( VB->Spec[1] );
   ALIGN_FREE( VB->ClipMask );
   ALIGN_FREE( VB->UserClipMask );

   if (VB->ctx->Driver.UnregisterVB)
      VB->ctx->Driver.UnregisterVB( VB );

   ALIGN_FREE( VB );
}


struct immediate *gl_immediate_alloc( GLcontext *ctx )
{
   static int id = 0;
   struct immediate *IM;
   GLuint j;

   if (ctx->freed_im_queue) {
      IM = ctx->freed_im_queue;
      ctx->freed_im_queue = IM->next;
      ctx->nr_im_queued--; 
      IM->ref_count = 1;
      return IM;
   }

   IM = ALIGN_MALLOC_STRUCT( immediate, 32 );
   if (!IM)
      return 0;

   IM->id = id++;
   IM->ref_count = 1;
   IM->backref = ctx;
   IM->maybe_transform_vb = gl_maybe_transform_vb;
/*     IM->Bounds = 0; */
   IM->NormalLengths = 0;
   IM->LastCalcedLength = 0;
   IM->FlushElt = 0;
   IM->LastPrimitive = VB_START;
   IM->Count = VB_MAX;		/* force clear of Flag. */
   IM->Start = VB_START;	
   IM->Material = 0;
   IM->MaterialMask = 0;
#ifdef VMS
   for (j = 0; j < VB_SIZE; j++)
     IM->Normal[j][0] = IM->Normal[j][1] = IM->Normal[j][2] = 0.0;
#endif
   
   if (MESA_VERBOSE&VERBOSE_IMMEDIATE)
      fprintf(stderr, "alloc immediate %d\n", id);

   gl_vector4f_init( &IM->v.Obj, VEC_WRITABLE, IM->Obj );
   gl_vector3f_init( &IM->v.Normal, VEC_WRITABLE, IM->Normal );
   gl_vector4ub_init( &IM->v.Color, VEC_WRITABLE, IM->Color );
   gl_vector1ui_init( &IM->v.Index, VEC_WRITABLE, IM->Index );
   gl_vector1ub_init( &IM->v.EdgeFlag, VEC_WRITABLE, IM->EdgeFlag );
   gl_vector1ui_init( &IM->v.Elt, VEC_WRITABLE, IM->Elt );

   for (j=0;j<MAX_TEXTURE_UNITS;j++) {
      IM->TexCoordPtr[j] = IM->TexCoord[j];
      gl_vector4f_init( &IM->v.TexCoord[j], VEC_WRITABLE, IM->TexCoord[j]);

      /* Precalculate some flags and keep them in a handy place.
       */
      IM->TF1[j] = VERT_TEX0_1 << (j*NR_TEXSIZE_BITS);
      IM->TF2[j] = VERT_TEX0_12 << (j*NR_TEXSIZE_BITS);
      IM->TF3[j] = VERT_TEX0_123 << (j*NR_TEXSIZE_BITS);
      IM->TF4[j] = VERT_TEX0_1234 << (j*NR_TEXSIZE_BITS);
   }

   return IM;
}


void gl_immediate_free( struct immediate *IM )
{
   GLcontext *ctx = IM->backref;

   if (IM->NormalLengths) {
      FREE( IM->NormalLengths );
      IM->NormalLengths = 0;
      IM->LastCalcedLength = 0;
   }

   if (IM->Material) {
      FREE( IM->Material );
      FREE( IM->MaterialMask );
      IM->Material = 0;
      IM->MaterialMask = 0;
   }

   if (ctx->nr_im_queued > 5) {
      if (MESA_VERBOSE&VERBOSE_IMMEDIATE)
	 fprintf(stderr, "really free immediate %d\n", IM->id);

      ALIGN_FREE( IM );
   }
   else {
      if (MESA_VERBOSE&VERBOSE_IMMEDIATE)
	 fprintf(stderr, "requeue immediate %d\n", IM->id);

      IM->next = ctx->freed_im_queue;
      ctx->freed_im_queue = IM;
      ctx->nr_im_queued++; 
   }
}
