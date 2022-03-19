/* $Id: stages.c,v 1.20.4.3 2000/11/26 21:10:26 brianp Exp $*/

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
#include "bbox.h"
#include "clip.h"
#include "context.h"
#include "cva.h"
#include "fog.h"
#include "light.h"
#include "mmath.h"
#include "pipeline.h"
#include "shade.h"
#include "stages.h"
#include "translate.h"
#include "types.h"
#include "vbcull.h"
#include "vbindirect.h"
#include "vbrender.h"
#include "vbxform.h"
#include "xform.h"
#endif


static GLmatrix gl_identity_mat;


/*
 * One-time init function called from one_time_init() in context.c
 */
void gl_init_vbxform( void )
{
   gl_matrix_ctr( &gl_identity_mat );
}



/* KW: Even if all the vertices are clipped or culled, still need to
 *     execute the material changes.
 *
 *     TODO: Do this backwards, from count to start.
 */
void gl_update_materials( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLuint orflag = VB->OrFlag;

   if (orflag & VERT_MATERIAL)
   {
      GLuint i;
      GLuint *flag = VB->Flag, count = VB->Count;
      struct gl_material (*new_material)[2] = VB->Material;
      GLuint *new_material_mask = VB->MaterialMask;

      /* This code never reached in cva.
       */
      for ( i = VB->Start ; i <= count ; i++ )
	 if ( flag[i] & VERT_MATERIAL )
	    gl_update_material( ctx, new_material[i], new_material_mask[i] );
   }

   if ((orflag & VERT_RGBA) && ctx->Light.ColorMaterialEnabled)
      gl_update_color_material( ctx, ctx->Current.ByteColor );
}


void gl_clean_color( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
#ifdef VAO
   struct gl_client_array *client_data = &ctx->Array.Current->Color;
#else
   struct gl_client_array *client_data = &ctx->Array.Color;
#endif
   GLvector4ub *col;

#ifdef VAO
   if (!(ctx->Array.Current->Summary & VERT_RGBA))
#else
   if (!(ctx->Array.Summary & VERT_RGBA))
#endif
      client_data = &ctx->Fallback.Color;

   if (VB->Type == VB_CVA_PRECALC) {
      col = VB->ColorPtr;
      col->data = ctx->CVA.store.Color;
   } else {
      VB->ColorPtr = VB->Color[0] = col = &VB->IM->v.Color;
   }

   gl_trans_4ub_tab[4][TYPE_IDX(GL_UNSIGNED_BYTE)]( col->data, client_data,
						    VB->Start, VB->Count );

   col->flags = VEC_WRITABLE|VEC_GOOD_STRIDE;
   col->stride = 4 * sizeof(GLubyte);
}

static void clean_index( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
#ifdef VAO
   struct gl_client_array *client_data = &ctx->Array.Current->Index;
#else
   struct gl_client_array *client_data = &ctx->Array.Index;
#endif
   GLvector1ui *index;

#ifdef VAO
   if (!(ctx->Array.Current->Summary & VERT_INDEX))
#else
   if (!(ctx->Array.Summary & VERT_INDEX))
#endif
      client_data = &ctx->Fallback.Color;

   if (VB->Type == VB_CVA_PRECALC) {
      index = VB->IndexPtr;
      index->data =  ctx->CVA.store.Index;
   } else {
      VB->IndexPtr = index = &VB->IM->v.Index;
   }

   gl_trans_1ui_tab[TYPE_IDX(GL_UNSIGNED_INT)]( index->data, client_data,
						VB->Start, VB->Count );

   index->flags = VEC_WRITABLE|VEC_GOOD_STRIDE;
   index->stride = sizeof(GLuint);
}


static void clean_edgeflag( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
#ifdef VAO
   struct gl_client_array *client_data = &ctx->Array.Current->EdgeFlag;
#else
   struct gl_client_array *client_data = &ctx->Array.EdgeFlag;
#endif
   GLvector1ub *edge;

#ifdef VAO
   if (!(ctx->Array.Current->Summary & VERT_EDGE))
#else
   if (!(ctx->Array.Summary & VERT_EDGE))
#endif
      client_data = &ctx->Fallback.EdgeFlag;

   if (VB->Type == VB_CVA_PRECALC) {
      edge = VB->EdgeFlagPtr;
      edge->data = ctx->CVA.store.EdgeFlag;
   } else {
      VB->EdgeFlagPtr = edge = &VB->IM->v.EdgeFlag;
   }

   gl_trans_1ub_tab[TYPE_IDX(GL_UNSIGNED_BYTE)]( edge->data, client_data,
						 VB->Start, VB->Count );

   edge->flags = VEC_WRITABLE|VEC_GOOD_STRIDE;
   edge->stride = sizeof(GLubyte);
}


static void clean_texcoord( struct vertex_buffer *VB, GLuint i )
{
   GLcontext *ctx = VB->ctx;
#ifdef VAO
   struct gl_client_array *client_data = &ctx->Array.Current->TexCoord[i];
#else
   struct gl_client_array *client_data = &ctx->Array.TexCoord[i];
#endif
   GLvector4f *tc;
   GLuint flag = PIPE_TEX(i);

#ifdef VAO
   if (!(ctx->Array.Current->Summary & flag))
#else
   if (!(ctx->Array.Summary & flag))
#endif
      client_data = &ctx->Fallback.TexCoord[i];

   if (VB->Type == VB_CVA_PRECALC) {
      tc = VB->TexCoordPtr[i];
      tc->data = ctx->CVA.store.TexCoord[i];
   } else {
      VB->TexCoordPtr[i] = tc = &VB->IM->v.TexCoord[i];
   }

   gl_trans_4f_tab[tc->size][TYPE_IDX(GL_FLOAT)]( tc->data, client_data,
						  VB->Start, VB->Count );

   tc->flags = VEC_WRITABLE|VEC_GOOD_STRIDE;
   tc->stride = 4 * sizeof(GLfloat);
}

static void clean_unprojected( struct vertex_buffer *VB )
{
   (void) Transform( &VB->Eye,
                     &gl_identity_mat,
                     VB->Unprojected,
                     0,
                     0);
   VB->Unprojected = &VB->Eye;
}

static void clean_clip( struct vertex_buffer *VB )
{
   (void) Transform( &VB->Clip,
                     &gl_identity_mat,
                     VB->ClipPtr,
                     VB->ClipMask + VB->Start,
                     VB->CullFlag[0]);
   VB->ClipPtr = &VB->Clip;
}



void gl_import_client_data( struct vertex_buffer *VB,
			    GLuint required,
			    GLuint vec_flags )
{
   if ((required & VERT_RGBA) && !(VB->ColorPtr->flags & vec_flags))
      gl_clean_color(VB);

   if ((required & VERT_INDEX) && !(VB->IndexPtr->flags & vec_flags))
      clean_index(VB);

   if ((required & VERT_TEX0_ANY) && !(VB->TexCoordPtr[0]->flags & vec_flags))
      clean_texcoord(VB, 0);

   if ((required & VERT_TEX1_ANY) && !(VB->TexCoordPtr[1]->flags & vec_flags))
      clean_texcoord(VB, 1);

   if ((required & VERT_EDGE) && !(VB->EdgeFlagPtr->flags & vec_flags))
      clean_edgeflag(VB);

   if (!(VB->ClipPtr->flags & vec_flags))
      clean_clip(VB);
}



#if 0
/* Figure this is too rare to update via UpdateState, so give it a
 * flag of its own.
 */
static void gl_calculate_model_project_win_matrix( GLcontext *ctx )
{
    gl_matrix_mul( &ctx->ModelProjectWinMatrix,
		   &ctx->Viewport.WindowMap,
		   &ctx->ModelProjectMatrix );

   gl_matrix_analyze( &ctx->ModelProjectWinMatrix );
   ctx->ModelProjectWinMatrixUptodate = GL_TRUE;
}
#endif



#if 0
static void bound_cull_vb( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   VB->Projected = &VB->Win;
   VB->ClipPtr = &VB->Clip;
   VB->Culled = 1;

   gl_dont_cull_vb( VB );

   {
      GLuint start = 3 - VB->CopyCount;
      GLuint dst;
      GLuint *copy = VB->Copy;
      GLmatrix *mat = &ctx->ModelProjectMatrix;

      for (dst = start ; dst < VB->Start ; dst++) {
	 GLfloat *src = VEC_ELT(VB->ObjPtr, GLfloat, copy[dst]);
	 GLfloat *clip = VB->Clip.data[copy[dst]];
	 gl_transform_point_sz( clip, mat->m, src, VB->ObjPtr->size );
	 {
	    const GLfloat cw = clip[3];
	    const GLfloat cx = clip[0];
	    const GLfloat cy = clip[1];
	    const GLfloat cz = clip[2];
	    GLuint mask = 0;
	    if (cx >  cw) mask |= CLIP_RIGHT_BIT;
	    if (cx < -cw) mask |= CLIP_LEFT_BIT;
	    if (cy >  cw) mask |= CLIP_TOP_BIT;
	    if (cy < -cw) mask |= CLIP_BOTTOM_BIT;
	    if (cz >  cw) mask |= CLIP_FAR_BIT;
	    if (cz < -cw) mask |= CLIP_NEAR_BIT;
	    VB->ClipMask[copy[dst]] = mask;
	 }
      }
   }
}
#endif




static void do_vertex_pipeline( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLmatrix *proj_mat = ctx->vb_proj_matrix;
   GLvector4f *proj_dest = &VB->Clip;
   GLboolean need_window_transform = (1 || ctx->DoViewportMapping);
   GLuint copycount = VB->CopyCount;

   /* Eye transform
    */
   VB->Unprojected = VB->ObjPtr;

   if (ctx->NeedEyeCoords && ctx->ModelView.type != MATRIX_IDENTITY) {
      VB->Unprojected = TransformRaw( &VB->Eye, &ctx->ModelView, VB->ObjPtr );
   }

   VB->EyePtr = VB->Unprojected;


   /* Clip transform.
    */
   VB->ClipPtr = VB->Unprojected;

   if (proj_mat->type != MATRIX_IDENTITY ||
       ((ctx->IndirectTriangles & DD_ANY_CULL) &&
	VB->Unprojected->stride != 4*sizeof(GLfloat)))
   {
      VB->ClipPtr = TransformRaw(proj_dest, proj_mat, VB->Unprojected );
   }


   /* Cliptest and/or perspective divide.
    */

   VB->Projected = gl_clip_tab[VB->ClipPtr->size]( VB->ClipPtr,
                                                   &VB->Win,
                                                   VB->ClipMask + VB->Start,
                                                   &VB->ClipOrMask,
                                                   &VB->ClipAndMask );
   if (VB->ClipOrMask) {
      VB->CullFlag[1] = (GLubyte) (VB->ClipOrMask & ctx->AllowVertexCull);
      VB->CullMode |= CLIP_MASK_ACTIVE;
   }

   if (VB->ClipAndMask) {
      if (MESA_VERBOSE&VERBOSE_CULL)
	 fprintf(stderr, "Culled in clip\n");

      VB->Culled = 1;
      gl_dont_cull_vb( VB );
      gl_update_materials(VB);
      return;
   }


   /* User cliptest.
    */
   if (ctx->Transform.AnyClip) {
      gl_user_cliptest( VB );
      if (VB->Culled) {

	 if (MESA_VERBOSE&VERBOSE_CULL)
	    fprintf(stderr, "Culled in userclip\n");

	 gl_dont_cull_vb( VB );
	 gl_update_materials(VB);
	 return;
      }
   }


   /* Vertex culling - not for cva precalc.
    */
   if (VB->IM) {
      if (ctx->IndirectTriangles & DD_ANY_CULL)
      {
	 GLuint cullcount = gl_cull_vb( VB );
	 
	 if (MESA_VERBOSE&VERBOSE_CULL)
	    fprintf(stderr, "culled %u of %u vertices\n", cullcount, VB->Count);

	 if (cullcount == VB->Count) {
	    VB->Culled = 1;
	    gl_update_materials(VB);
	    return;
	 }
	 if (cullcount || (ctx->IndirectTriangles & DD_LIGHTING_CULL))
	    VB->CullMode |= CULL_MASK_ACTIVE;
      }
      else 
      {
	 if (MESA_VERBOSE&VERBOSE_CULL)
	    fprintf(stderr, "culling not active\n");

	 gl_dont_cull_vb( VB );
      }
   }


   /* Window transform.
    */
   if (need_window_transform) {

      if (VB->Start != VB->CopyStart) {
         VB->Projected->start = (GLfloat *)( (GLubyte *)VB->Projected->start 
                        - (copycount*VB->Projected->stride) );
         VB->Projected->count += copycount;

	 VB->Win.start = (GLfloat *)VB->Win.data[VB->CopyStart];
      }

      (void) Transform( &VB->Win,
			&ctx->Viewport.WindowMap,
			VB->Projected,
			VB->ClipMask + VB->CopyStart,
			VB->CullFlag[1]);

      if (VB->Win.size == 2) {
	 gl_vector4f_clean_elem(&VB->Win, VB->Count, 2);
      }
   }
}




static void do_normal_transform( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLuint tmp = 0;

   if (VB->Type == VB_CVA_PRECALC)
      VB->NormalPtr = &ctx->CVA.v.Normal;


   if (VB->CullMode & (COMPACTED_NORMALS|CULL_MASK_ACTIVE)) {
      tmp = 1;
      gl_make_normal_cullmask( VB );
   }

   if (ctx->NormalTransform) {
      (ctx->NormalTransform[tmp])(&ctx->ModelView,
				  ctx->vb_rescale_factor,
				  VB->NormalPtr,
				  (VB->NormalLengthPtr ?
				   VB->NormalLengthPtr + VB->Start : 0),
				  VB->NormCullStart,
				  VB->store.Normal);

      VB->NormalPtr = VB->store.Normal;
   }
}



static void do_lighting( struct vertex_buffer *VB )
{
   GLubyte flags = (GLubyte) (VB->CullMode & 
			      (CULL_MASK_ACTIVE|COMPACTED_NORMALS));

   if ((flags&CULL_MASK_ACTIVE) && !VB->NormCullStart)
      gl_make_normal_cullmask( VB );

   /* Make sure we can talk about elements 0..2 in the vector we are
    * lighting.
    */
   if (VB->Unprojected && VB->Unprojected->size == 2) {
      if (VB->Unprojected->flags & VEC_WRITABLE)
	 gl_vector4f_clean_elem(VB->Unprojected, VB->Count, 2);
      else 
	 clean_unprojected( VB );
   }

   VB->ctx->shade_func_tab[flags]( VB );
}



static void do_update_materials( struct vertex_buffer *VB )
{
   gl_update_materials( VB );
}


static void do_texture_0( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;

   if (ctx->Enabled & ENABLE_TEXGEN0) 
      (ctx->Texture.Unit[0].func[VB->CullMode & 0x3])( VB, 0 );

   if (ctx->Enabled & ENABLE_TEXMAT0)
      VB->TexCoordPtr[0] = Transform( VB->store.TexCoord[0],
				      &ctx->TextureMatrix[0],
				      VB->TexCoordPtr[0],
				      VB->ClipMask + VB->Start,
				      VB->CullFlag[0]);
}


static void do_texture_1( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;

   if (ctx->Enabled & ENABLE_TEXGEN1)
      (ctx->Texture.Unit[1].func[VB->CullMode & 0x3])( VB, 1 );

   if (ctx->Enabled & ENABLE_TEXMAT1)
      VB->TexCoordPtr[1] = Transform( VB->store.TexCoord[1],
				      &ctx->TextureMatrix[1],
				      VB->TexCoordPtr[1],
				      VB->ClipMask + VB->Start,
				      VB->CullFlag[0]);
}



/* Done if we need to pull cva data into the immediate struct for some
 * further processing, or if there is no ctx->Driver.MergeAndRenderCVA()
 * implemented.  Inputs: ArrayElt - Outputs: ctx->Array.LockPrecalcOutputs
 *
 * Should only be done if a later stage needs LockPrecalcOutputs.  If
 */
static void do_cva_merge( struct vertex_buffer *VB )
{
   struct gl_cva *cva = &VB->ctx->CVA;
   gl_merge_cva( VB, cva->VB );
}


/* This is deprecated and should be overridden by any driver implementing CVA.
 */
static void do_full_setup( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;

   if (ctx->Driver.RasterSetup)
      ctx->Driver.RasterSetup( VB, VB->CopyStart, VB->Count );
}


static void check_fog( GLcontext *ctx, struct gl_pipeline_stage *d )
{
   d->type = 0;

   if (ctx->FogMode==FOG_VERTEX)
   {
      GLuint flags;

      if (ctx->Visual->RGBAflag)
	 flags = VERT_EYE|VERT_RGBA;
      else
	 flags = VERT_EYE|VERT_INDEX;

      d->type = PIPE_IMMEDIATE|PIPE_PRECALC;
      d->inputs = flags;
      d->outputs = VERT_RGBA;
   }
}


static void check_lighting( GLcontext *ctx, struct gl_pipeline_stage *d )
{
   d->type = 0;

   if (ctx->Light.Enabled)
   {
      GLuint flags = VERT_NORM|VERT_MATERIAL;

      if (ctx->Light.NeedVertices) {
	 if (ctx->NeedEyeCoords)
	    flags |= VERT_EYE;
	 else
	    flags |= VERT_OBJ_ANY;
      }

      if (ctx->Light.ColorMaterialEnabled) flags |= VERT_RGBA;

      d->type = PIPE_IMMEDIATE|PIPE_PRECALC;
      d->inputs = flags;
      d->outputs = VERT_RGBA;
   }
}

static void check_update_materials( GLcontext *ctx,
				    struct gl_pipeline_stage *d )
{
   d->type = 0;

   if (!ctx->Light.Enabled) {
      GLuint flags = VERT_MATERIAL;
      if (ctx->Light.ColorMaterialEnabled) flags |= VERT_RGBA;
      d->type = PIPE_IMMEDIATE;
      d->inputs = flags;
      d->outputs = 0;
   }
}


static void check_normal_transform( GLcontext *ctx,
				    struct gl_pipeline_stage *d )
{
   d->type = 0;

   if ( ctx->NormalTransform &&
	(ctx->Enabled & (ENABLE_LIGHT|ENABLE_TEXGEN0|ENABLE_TEXGEN1)) )
   {
      d->type = PIPE_IMMEDIATE|PIPE_PRECALC;
      d->inputs = VERT_NORM;
      d->outputs = VERT_NORM;
   }
}



static void check_texture( GLcontext *ctx, GLuint i,
			   struct gl_pipeline_stage *d )
{
   struct gl_texture_unit *texUnit = &ctx->Texture.Unit[i];

   d->type = 0;

   if (ctx->Enabled & ((ENABLE_TEXGEN0|ENABLE_TEXMAT0)<<i))
   {
      GLuint texflag = PIPE_TEX(i);
      GLuint flags = 0;

      /* safe, but potentially inefficient
       */
      if (texUnit->GenFlags & TEXGEN_NEED_VERTICES) {
	 if (0)
	    flags |= VERT_OBJ_ANY;
	 if (1)
	    flags |= VERT_EYE;
      }

      if (texUnit->GenFlags & TEXGEN_NEED_NORMALS)
	 flags |= VERT_NORM;

      if (texUnit->Enabled & ~texUnit->TexGenEnabled)
	 flags |= texflag;

      d->type = PIPE_IMMEDIATE|PIPE_PRECALC;
      d->inputs = flags;
      d->outputs = texflag;
   }
}


static void check_texture_0( GLcontext *ctx, struct gl_pipeline_stage *d )
{
   check_texture(ctx, 0, d);
}

static void check_texture_1( GLcontext *ctx, struct gl_pipeline_stage *d )
{
   check_texture(ctx, 1, d);
}

static void check_full_setup( GLcontext *ctx, struct gl_pipeline_stage *d )
{
   /* Even on drivers without a raster setup step, this is useful as a
    * token to show you've got everything you need to render.  On
    * those drivers, this stage is active, but a no-op.
    */
   d->type = PIPE_IMMEDIATE|PIPE_PRECALC;
   d->inputs = ctx->RenderFlags;
   d->outputs = VERT_SETUP_FULL;

   if (ctx->IndirectTriangles & DD_SW_SETUP)
      d->type = PIPE_IMMEDIATE;
}

static void check_indirect_render( GLcontext *ctx,
				   struct gl_pipeline_stage *d )
{
   d->type = 0;

   if ((ctx->IndirectTriangles & DD_SW_SETUP) == 0)
   {
      d->type = PIPE_IMMEDIATE;
      d->inputs = VERT_SETUP_FULL | VERT_ELT | VERT_PRECALC_DATA;
   }
}

static void check_noop( GLcontext *ctx, struct gl_pipeline_stage *d )
{
   (void) ctx; (void) d;
}


static void do_noop( struct vertex_buffer *VB )
{
   (void) VB;
}


/*
 */
#define DYN_STATE 0,0,0

CONST struct gl_pipeline_stage gl_default_pipeline[] = {
   { "cva merge",
     0,
     PIPE_IMMEDIATE,
     0,
     0,				/* state change (recheck) */
     0,				/* cva state change (recalc) */
     0, 0,			/* cva forbidden */
     0, VERT_PRECALC_DATA | VERT_ELT, 0,
     check_noop,
     do_cva_merge },

   { "cva prepare arrays",
     0,
     PIPE_PRECALC,
     0,
     0,
     NEW_CLIENT_STATE,		/* cva state change (recalc) */
     0, 0,
     PIPE_PRECALC, 0, 0,
     check_noop,
     gl_prepare_arrays_cva },

   { "vertex pipeline",
     PIPE_OP_VERT_XFORM,
     PIPE_PRECALC|PIPE_IMMEDIATE,
     0,
     NEW_LIGHTING|NEW_TEXTURING|NEW_MODELVIEW|NEW_PROJECTION|NEW_USER_CLIP|NEW_VIEWPORT, /* state change */
     NEW_MODELVIEW|NEW_PROJECTION|NEW_USER_CLIP|NEW_VIEWPORT, /* cva state change */
     0, 0,
     0, VERT_OBJ_ANY, VERT_EYE|VERT_WIN,
     check_noop,
     do_vertex_pipeline },

   { "normal transform",
     PIPE_OP_NORM_XFORM,
     PIPE_PRECALC|PIPE_IMMEDIATE,
     0,
     NEW_LIGHTING|NEW_FOG|NEW_TEXTURING|NEW_NORMAL_TRANSFORM,	/* state change (recheck) */
     NEW_NORMAL_TRANSFORM,	/* cva state change (recalc) */
     0, 0,
     DYN_STATE,
     check_normal_transform,
     do_normal_transform },

   { "lighting",
     PIPE_OP_LIGHT,
     PIPE_PRECALC|PIPE_IMMEDIATE,
     0,
     NEW_LIGHTING,		 /* recheck */
     NEW_LIGHTING|NEW_MODELVIEW, /* recalc */
     0, VERT_MATERIAL,  	 /* I hate glMaterial() */
     DYN_STATE,
     check_lighting,
     do_lighting },

   { "update materials (no lighting)",
     0,
     PIPE_IMMEDIATE,
     0,
     NEW_LIGHTING,		/* recheck */
     0,
     0, 0,
     DYN_STATE,
     check_update_materials,
     do_update_materials },

   { "fog",
     PIPE_OP_FOG,
     PIPE_PRECALC|PIPE_IMMEDIATE,
     0,
     NEW_FOG,
     NEW_LIGHTING|NEW_RASTER_OPS|NEW_FOG|NEW_MODELVIEW,
     0, 0,
     DYN_STATE,
     check_fog,
     _mesa_fog_vertices },

   { "texture gen/transform 0",
     PIPE_OP_TEX0,
     PIPE_PRECALC|PIPE_IMMEDIATE,
     0,
     NEW_TEXTURING|NEW_TEXTURE_MATRIX|NEW_TEXTURE_ENABLE,
     NEW_TEXTURING|NEW_TEXTURE_MATRIX|NEW_TEXTURE_ENABLE,
     0, 0,
     DYN_STATE,
     check_texture_0,
     do_texture_0 },

   { "texture gen/transform 1",
     PIPE_OP_TEX1,
     PIPE_PRECALC|PIPE_IMMEDIATE,
     0,
     NEW_TEXTURING|NEW_TEXTURE_MATRIX|NEW_TEXTURE_ENABLE,
     NEW_TEXTURING|NEW_TEXTURE_MATRIX|NEW_TEXTURE_ENABLE,
     0, 0,
     DYN_STATE,
     check_texture_1,
     do_texture_1 },


   { "cva indirect render",	/* prior to setup -> require precalc'd setup */
     PIPE_OP_RENDER,
     PIPE_IMMEDIATE,
     0,
     NEW_LIGHTING|NEW_TEXTURING|NEW_RASTER_OPS,
     0,
     0, 0,
     DYN_STATE,
     check_indirect_render,
     gl_render_vb_indirect },


   { "full raster setup",	/* prepare for indirect and normal render */
     PIPE_OP_RAST_SETUP_0|PIPE_OP_RAST_SETUP_1,
     PIPE_PRECALC|PIPE_IMMEDIATE,
     0,
     NEW_LIGHTING|NEW_TEXTURING|NEW_RASTER_OPS|NEW_POLYGON|NEW_TEXTURE_ENV,
     NEW_LIGHTING|NEW_TEXTURING|NEW_RASTER_OPS|NEW_POLYGON|NEW_TEXTURE_ENV,
     0, 0,
     DYN_STATE,
     check_full_setup,
     do_full_setup },

   { "partial raster setup",	/* prepare for merge_and_render */
     PIPE_OP_RAST_SETUP_0,
     0,
     0,
     0,
     0,
     0, 0,
     DYN_STATE,
     check_noop,
     do_noop },

   { "cva merge & render",
     PIPE_OP_RAST_SETUP_1|PIPE_OP_RENDER, /* rsetup1 if processing in elt? */
     0,
     0,
     0,				/* state change (recheck)  */
     0,				/* cva state change */
     0,	0,			/* cva forbidden */
     DYN_STATE,
     check_noop,
     do_noop },

   { "render",
     PIPE_OP_RENDER,
     PIPE_IMMEDIATE,
     0,
     0,
     0,
     0, 0,
     0, VERT_SETUP_FULL, 0,
     check_noop,
     gl_render_vb },

   { "render elements",		/* essentially the same as indirect render */
     PIPE_OP_RENDER,
     PIPE_PRECALC,
     0,
     0,				/* render always triggered by data updates */
     0,
     0, 0,
     0, VERT_SETUP_FULL|VERT_ELT, 0,
     check_noop,
     gl_render_elts },
};

CONST GLuint gl_default_nr_stages = Elements(gl_default_pipeline);
