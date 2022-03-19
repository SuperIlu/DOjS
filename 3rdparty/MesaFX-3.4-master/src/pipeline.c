/* $Id: pipeline.c,v 1.17.4.5 2000/11/26 21:10:26 brianp Exp $*/

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

/* Dynamic pipelines, support for CVA.
 * Copyright (C) 1999 Keith Whitwell.
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
#include "mem.h"
#include "mmath.h"
#include "pipeline.h"
#include "shade.h"
#include "stages.h"
#include "state.h"
#include "types.h"
#include "translate.h"
#include "vbcull.h"
#include "vbindirect.h"
#include "vbrender.h"
#include "vbxform.h"
#include "xform.h"
#endif



#ifndef MESA_VERBOSE
int MESA_VERBOSE = 0 
/*                 | VERBOSE_PIPELINE */
/*                 | VERBOSE_IMMEDIATE */
/*                 | VERBOSE_VARRAY */
/*                 | VERBOSE_TEXTURE */
/*                 | VERBOSE_API */
/*                 | VERBOSE_DRIVER */
/*                 | VERBOSE_STATE */
/*                 | VERBOSE_CULL */
/*                 | VERBOSE_DISPLAY_LIST */
;
#endif

#ifndef MESA_DEBUG_FLAGS
int MESA_DEBUG_FLAGS = 0 
/*                 | DEBUG_ALWAYS_FLUSH */
;
#endif

#ifdef MESA_DEBUG
void gl_print_pipe_ops( const char *msg, GLuint flags )
{
   fprintf(stderr, 
	   "%s: (0x%x) %s%s%s%s%s%s%s%s%s%s\n",
	   msg,
	   flags,
	   (flags & PIPE_OP_CVA_PREPARE)   ? "cva-prepare, " : "",
	   (flags & PIPE_OP_VERT_XFORM)    ? "vert-xform, " : "",
	   (flags & PIPE_OP_NORM_XFORM)    ? "norm-xform, " : "",
	   (flags & PIPE_OP_LIGHT)         ? "light, " : "",
	   (flags & PIPE_OP_FOG)           ? "fog, " : "",
	   (flags & PIPE_OP_TEX0)          ? "tex-0, " : "",
	   (flags & PIPE_OP_TEX1)          ? "tex-1, " : "",
	   (flags & PIPE_OP_RAST_SETUP_0)  ? "rast-0, " : "",
	   (flags & PIPE_OP_RAST_SETUP_1)  ? "rast-1, " : "",
	   (flags & PIPE_OP_RENDER)        ? "render, " : "");

}
#endif


/* Have to reset only those parts of the vb which are being recalculated.
 */
void gl_reset_cva_vb( struct vertex_buffer *VB, GLuint stages )
{
   GLcontext *ctx = VB->ctx;

   if (MESA_VERBOSE&VERBOSE_PIPELINE)
      gl_print_pipe_ops( "reset cva vb", stages ); 

   if (ctx->Driver.ResetCvaVB)
      ctx->Driver.ResetCvaVB( VB, stages );

   if (stages & PIPE_OP_VERT_XFORM) 
   {
      if (VB->ClipOrMask & CLIP_USER_BIT)
	 MEMSET(VB->UserClipMask, 0, VB->Count);

      VB->ClipOrMask = 0;
      VB->ClipAndMask = CLIP_ALL_BITS;
      VB->CullMode = 0;
      VB->CullFlag[0] = VB->CullFlag[1] = 0;
      VB->Culled = 0;
   }

   if (stages & PIPE_OP_NORM_XFORM) {
      VB->NormalPtr = &ctx->CVA.v.Normal;
   }

   if (stages & PIPE_OP_LIGHT) 
   {
      VB->ColorPtr = VB->Color[0] = VB->Color[1] = &ctx->CVA.v.Color;
      VB->IndexPtr = VB->Index[0] = VB->Index[1] = &ctx->CVA.v.Index;
   }
   else if (stages & PIPE_OP_FOG) 
   {
      if (ctx->Light.Enabled) {
	 VB->Color[0] = VB->LitColor[0];
	 VB->Color[1] = VB->LitColor[1];      
	 VB->Index[0] = VB->LitIndex[0];
	 VB->Index[1] = VB->LitIndex[1];      
      } else {
	 VB->Color[0] = VB->Color[1] = &ctx->CVA.v.Color;
	 VB->Index[0] = VB->Index[1] = &ctx->CVA.v.Index;
      }
      VB->ColorPtr = VB->Color[0];
      VB->IndexPtr = VB->Index[0];
   }
}




static const char *pipeline_name[3] = {
   0,
   "Immediate",
   "CVA Precalc",
};



static void pipeline_ctr( struct gl_pipeline *p, GLcontext *ctx, GLuint type )
{
   GLuint i;
   (void) ctx;

   p->state_change = 0;
   p->cva_state_change = 0;
   p->inputs = 0;
   p->outputs = 0;
   p->type = type;
   p->ops = 0;

   for (i = 0 ; i < gl_default_nr_stages ; i++) 
      p->state_change |= gl_default_pipeline[i].state_change;
}


void gl_pipeline_init( GLcontext *ctx )
{
   if (ctx->Driver.RegisterPipelineStages)
      ctx->NrPipelineStages = 
	 ctx->Driver.RegisterPipelineStages( ctx->PipelineStage,
					     gl_default_pipeline,
					     gl_default_nr_stages );
   else 
   {
      MEMCPY( ctx->PipelineStage, 
	      gl_default_pipeline, 
	      sizeof(*gl_default_pipeline) * gl_default_nr_stages );

      ctx->NrPipelineStages = gl_default_nr_stages;
   }

   pipeline_ctr( &ctx->CVA.elt, ctx, PIPE_IMMEDIATE);
   pipeline_ctr( &ctx->CVA.pre, ctx, PIPE_PRECALC );
}






#define MINIMAL_VERT_DATA (VERT_DATA&~(VERT_TEX0_4|VERT_TEX1_4|VERT_EVAL_ANY))

#define VERT_CURRENT_DATA (VERT_TEX0_1234|VERT_TEX1_1234|VERT_RGBA| \
			   VERT_INDEX|VERT_EDGE|VERT_NORM| \
	                   VERT_MATERIAL)

/* Called prior to every recomputation of the CVA precalc data, except where
 * the driver is able to calculate the pipeline unassisted.
 */
static void build_full_precalc_pipeline( GLcontext *ctx )
{
   struct gl_pipeline_stage *pipeline = ctx->PipelineStage;
   struct gl_cva *cva = &ctx->CVA;
   struct gl_pipeline *pre = &cva->pre;   
   struct gl_pipeline_stage **stages = pre->stages;
   GLuint i;
   GLuint newstate = pre->new_state;
   GLuint changed_ops = 0;
   GLuint oldoutputs = pre->outputs;
   GLuint oldinputs = pre->inputs;
#ifdef VAO
   GLuint fallback = (VERT_CURRENT_DATA & ctx->Current.Flag & 
		      ~ctx->Array.Current->Summary);
   GLuint changed_outputs = (ctx->Array.NewArrayState | 
			     (fallback & cva->orflag));
   GLuint available = fallback | ctx->Array.Current->Flags;
#else
   GLuint fallback = (VERT_CURRENT_DATA & ctx->Current.Flag & 
		      ~ctx->Array.Summary);
   GLuint changed_outputs = (ctx->Array.NewArrayState | 
			     (fallback & cva->orflag));
   GLuint available = fallback | ctx->Array.Flags;
#endif

   pre->cva_state_change = 0;
   pre->ops = 0;
   pre->outputs = 0;
   pre->inputs = 0;
   pre->forbidden_inputs = 0;
   pre->fallback = 0;
   
#ifdef VAO
   if (ctx->Array.Current->Summary & VERT_ELT) 
#else
   if (ctx->Array.Summary & VERT_ELT) 
#endif
      cva->orflag &= VERT_MATERIAL;
  
#ifdef VAO
   cva->orflag &= ~(ctx->Array.Current->Summary & ~VERT_OBJ_ANY);
#else
   cva->orflag &= ~(ctx->Array.Summary & ~VERT_OBJ_ANY);
#endif
   available &= ~cva->orflag;
   
   pre->outputs = available;
   pre->inputs = available;

   if (MESA_VERBOSE & VERBOSE_PIPELINE) {
      fprintf(stderr, ": Rebuild pipeline\n");
      gl_print_vert_flags("orflag", cva->orflag);
   }
      
   

   /* If something changes in the pipeline, tag all subsequent stages
    * using this value for recalcuation.  Also used to build the full
    * pipeline by setting newstate and newinputs to ~0.
    *
    * Because all intermediate values are buffered, the new inputs
    * are enough to fully specify what needs to be calculated, and a
    * single pass identifies all stages requiring recalculation.
    */
   for (i = 0 ; i < ctx->NrPipelineStages ; i++) 
   {
      pipeline[i].check(ctx, &pipeline[i]);

      if (pipeline[i].type & PIPE_PRECALC) 
      {
	 if ((newstate & pipeline[i].cva_state_change) ||
	     (changed_outputs & pipeline[i].inputs) ||
	     !pipeline[i].inputs)
	 {	    
	    changed_ops |= pipeline[i].ops;
	    changed_outputs |= pipeline[i].outputs;
	    pipeline[i].active &= ~PIPE_PRECALC;

	    if ((pipeline[i].inputs & ~available) == 0 &&
		(pipeline[i].ops & pre->ops) == 0)
	    {
	       pipeline[i].active |= PIPE_PRECALC;
	       *stages++ = &pipeline[i];
	    } 
	 }
      
	 /* Incompatible with multiple stages structs implementing
	  * the same stage.
	  */
	 available &= ~pipeline[i].outputs;
	 pre->outputs &= ~pipeline[i].outputs;

	 if (pipeline[i].active & PIPE_PRECALC) {
	    pre->ops |= pipeline[i].ops;
	    pre->outputs |= pipeline[i].outputs;
	    available |= pipeline[i].outputs;
	    pre->forbidden_inputs |= pipeline[i].pre_forbidden_inputs;
	 }
      } 
      else if (pipeline[i].active & PIPE_PRECALC) 
      {
	 pipeline[i].active &= ~PIPE_PRECALC;
	 changed_outputs |= pipeline[i].outputs;
	 changed_ops |= pipeline[i].ops;
      }
   }

   *stages = 0;

   pre->new_outputs = pre->outputs & (changed_outputs | ~oldoutputs);
   pre->new_inputs = pre->inputs & ~oldinputs;
   pre->fallback = pre->inputs & fallback;
   pre->forbidden_inputs |= pre->inputs & fallback;

   pre->changed_ops = changed_ops;

   if (ctx->Driver.OptimizePrecalcPipeline)
      ctx->Driver.OptimizePrecalcPipeline( ctx, pre );
}

void gl_build_precalc_pipeline( GLcontext *ctx )
{
   struct gl_pipeline *pre = &ctx->CVA.pre;   
   struct gl_pipeline *elt = &ctx->CVA.elt;   

   if (!ctx->Driver.BuildPrecalcPipeline ||
       !ctx->Driver.BuildPrecalcPipeline( ctx ))
      build_full_precalc_pipeline( ctx );

   pre->data_valid = 0;
   pre->pipeline_valid = 1;
   elt->pipeline_valid = 0;
   
   ctx->CVA.orflag = 0;
   
   if (MESA_VERBOSE&VERBOSE_PIPELINE)
      gl_print_pipeline( ctx, pre ); 
}


static void build_full_immediate_pipeline( GLcontext *ctx )
{
   struct gl_pipeline_stage *pipeline = ctx->PipelineStage;
   struct gl_cva *cva = &ctx->CVA;
   struct gl_pipeline *pre = &cva->pre;   
   struct gl_pipeline *elt = &cva->elt;
   struct gl_pipeline_stage **stages = elt->stages;
   GLuint i;
   GLuint newstate = elt->new_state;
   GLuint active_ops = 0;
   GLuint available = cva->orflag | MINIMAL_VERT_DATA;
   GLuint generated = 0;
   GLuint is_elt = 0;

   if (pre->data_valid && ctx->CompileCVAFlag) {
      is_elt = 1;
      active_ops = cva->pre.ops;
      available |= pre->outputs | VERT_PRECALC_DATA;
   }


   elt->outputs = 0;		/* not used */
   elt->inputs = 0;

   for (i = 0 ; i < ctx->NrPipelineStages ; i++) {
      pipeline[i].active &= ~PIPE_IMMEDIATE;

      if ((pipeline[i].state_change & newstate) ||
  	  (pipeline[i].elt_forbidden_inputs & available)) 
      {
	 pipeline[i].check(ctx, &pipeline[i]);
      }

      if ((pipeline[i].type & PIPE_IMMEDIATE) &&
	  (pipeline[i].ops & active_ops) == 0 && 
	  (pipeline[i].elt_forbidden_inputs & available) == 0
	 )
      {
	 if (pipeline[i].inputs & ~available) 
	    elt->forbidden_inputs |= pipeline[i].inputs & ~available;
	 else
	 {
	    elt->inputs |= pipeline[i].inputs & ~generated;
	    elt->forbidden_inputs |= pipeline[i].elt_forbidden_inputs;
	    pipeline[i].active |= PIPE_IMMEDIATE;
	    *stages++ = &pipeline[i];
	    generated |= pipeline[i].outputs;
	    available |= pipeline[i].outputs;
	    active_ops |= pipeline[i].ops;
	 }
      }
   }

   *stages = 0;
   
   elt->copy_transformed_data = 1;
   elt->replay_copied_vertices = 0;

   if (is_elt) {
      cva->merge = elt->inputs & pre->outputs;
      elt->ops = active_ops & ~pre->ops;
   }
}



void gl_build_immediate_pipeline( GLcontext *ctx )
{
   struct gl_pipeline *elt = &ctx->CVA.elt;   

   if (!ctx->Driver.BuildEltPipeline ||
       !ctx->Driver.BuildEltPipeline( ctx )) {
      build_full_immediate_pipeline( ctx );
   }

   elt->pipeline_valid = 1;
   ctx->CVA.orflag = 0;
   
   if (MESA_VERBOSE&VERBOSE_PIPELINE)
      gl_print_pipeline( ctx, elt ); 
}
   
#define INTERESTED ~(NEW_DRIVER_STATE|NEW_CLIENT_STATE)

void gl_update_pipelines( GLcontext *ctx )
{
   GLuint newstate = ctx->NewState;
   struct gl_cva *cva = &ctx->CVA;

   newstate &= INTERESTED;

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_STATE))
      gl_print_enable_flags("enabled", ctx->Enabled);

   if (newstate ||
       cva->lock_changed ||
       cva->orflag != cva->last_orflag ||
#ifdef VAO
       ctx->Array.Current->Flags != cva->last_array_flags)
#else
       ctx->Array.Flags != cva->last_array_flags)
#endif
   {   
      GLuint flags = VERT_WIN;

      if (ctx->Visual->RGBAflag) 
	 flags |= VERT_RGBA;
      else 
	 flags |= VERT_INDEX;

      if (ctx->Texture.ReallyEnabled & 0xf) {
/* XXX this should also check that the texture is RGBA.  What about Unit[1]?
	 if (ctx->Texture.Unit[0].EnvMode == GL_REPLACE)
	    flags &= ~VERT_RGBA;
*/
	 flags |= VERT_TEX0_ANY;
      }

      if (ctx->Texture.ReallyEnabled & 0xf0)
	 flags |= VERT_TEX1_ANY;
   
      if (ctx->Polygon.Unfilled) 
	 flags |= VERT_EDGE;
 
      if (ctx->RenderMode==GL_FEEDBACK) 
      {
	 flags = (VERT_WIN|VERT_RGBA|VERT_INDEX|
		  VERT_NORM|VERT_EDGE|
		  VERT_TEX0_ANY|VERT_TEX1_ANY);
      }

      ctx->RenderFlags = flags;

      cva->elt.new_state |= newstate;
      cva->elt.pipeline_valid = 0;

      cva->pre.new_state |= newstate;
      cva->pre.forbidden_inputs = 0;
      cva->pre.pipeline_valid = 0;
      cva->lock_changed = 0;
   }

#ifdef VAO
   if (ctx->Array.NewArrayState != cva->last_array_new_state)
#else
   if (ctx->Array.NewArrayState != cva->last_array_new_state)
#endif
      cva->pre.pipeline_valid = 0;

   cva->pre.data_valid = 0;
#ifdef VAO
   cva->last_array_new_state = ctx->Array.NewArrayState;
#else
   cva->last_array_new_state = ctx->Array.NewArrayState;
#endif
   cva->last_orflag = cva->orflag;
#ifdef VAO
   cva->last_array_flags = ctx->Array.Current->Flags;
#else
   cva->last_array_flags = ctx->Array.Flags;
#endif
}

void gl_run_pipeline( struct vertex_buffer *VB )
{
   struct gl_pipeline *pipe = VB->pipeline;
   struct gl_pipeline_stage **stages = pipe->stages;
   unsigned short x;

   pipe->data_valid = 1;	/* optimized stages might want to reset this. */

   START_FAST_MATH(x);
   
   for ( VB->Culled = 0; *stages && !VB->Culled ; stages++ )
      (*stages)->run( VB );
      
   END_FAST_MATH(x);

   pipe->new_state = 0;
}

#ifdef MESA_DEBUG
void gl_print_vert_flags( const char *name, GLuint flags ) 
{
   fprintf(stderr, 
	   "%s: (0x%x) %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
	   name,
	   flags,
	   (flags & VERT_OBJ_ANY)    ? "vertices (obj), " : "",
	   (flags & VERT_ELT)        ? "array-elt, " : "",
	   (flags & VERT_RGBA)       ? "colors, " : "",
	   (flags & VERT_NORM)       ? "normals, " : "",
	   (flags & VERT_INDEX)      ? "index, " : "",
	   (flags & VERT_EDGE)       ? "edgeflag, " : "",
	   (flags & VERT_MATERIAL)   ? "material, " : "",
	   (flags & VERT_TEX0_ANY)   ? "texcoord0, " : "",
	   (flags & VERT_TEX1_ANY)   ? "texcoord1, " : "",
	   (flags & VERT_EVAL_ANY)   ? "eval-coord, " : "",
	   (flags & VERT_EYE)        ? "eye, " : "",
	   (flags & VERT_WIN)        ? "win, " : "",
	   (flags & VERT_PRECALC_DATA) ? "precalc data, " : "",
	   (flags & VERT_SETUP_FULL) ? "driver-data, " : "", 
	   (flags & VERT_SETUP_PART) ? "partial-driver-data, " : ""
      );
}

void gl_print_tri_caps( const char *name, GLuint flags ) 
{
   fprintf(stderr, 
	   "%s: (0x%x) %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
	   name,
	   flags,
	   (flags & DD_FEEDBACK)            ? "feedback, " : "",
	   (flags & DD_SELECT)              ? "select, " : "",
	   (flags & DD_FLATSHADE)           ? "flat-shade, " : "",
	   (flags & DD_MULTIDRAW)           ? "multidraw, " : "",
	   (flags & DD_SEPERATE_SPECULAR)   ? "seperate-specular, " : "",
	   (flags & DD_TRI_LIGHT_TWOSIDE)   ? "tri-light-twoside, " : "",
	   (flags & DD_TRI_UNFILLED)        ? "tri-unfilled, " : "",
	   (flags & DD_TRI_STIPPLE)         ? "tri-stipple, " : "",
	   (flags & DD_TRI_OFFSET)          ? "tri-offset, " : "",
	   (flags & DD_TRI_CULL)            ? "tri-bf-cull, " : "",
	   (flags & DD_LINE_SMOOTH)         ? "line-smooth, " : "",
	   (flags & DD_LINE_STIPPLE)        ? "line-stipple, " : "",
	   (flags & DD_LINE_WIDTH)          ? "line-wide, " : "",
	   (flags & DD_POINT_SMOOTH)        ? "point-smooth, " : "", 
	   (flags & DD_POINT_SIZE)          ? "point-size, " : "", 
	   (flags & DD_POINT_ATTEN)         ? "point-atten, " : "", 
	   (flags & DD_LIGHTING_CULL)       ? "lighting-cull, " : "", 
	   (flags & DD_POINT_SW_RASTERIZE)  ? "sw-points, " : "", 
	   (flags & DD_LINE_SW_RASTERIZE)   ? "sw-lines, " : "", 
	   (flags & DD_TRI_SW_RASTERIZE)    ? "sw-tris, " : "", 
	   (flags & DD_QUAD_SW_RASTERIZE)   ? "sw-quads, " : "",
	   (flags & DD_TRI_CULL_FRONT_BACK) ? "cull-all, " : "",
	   (flags & DD_STENCIL)             ? "stencil, " : "",
	   (flags & DD_CLIP_FOG_COORD)      ? "clip-fog-coord, " : ""
      );
}


void gl_print_pipeline( GLcontext *ctx, struct gl_pipeline *p )
{
   struct gl_pipeline_stage *pipeline = ctx->PipelineStage;
   GLuint i;

   fprintf(stderr,"Type: %s\n", pipeline_name[p->type]);

   if (!p->pipeline_valid) {
      printf("--> Not up to date!!!\n");
      return;
   }

   gl_print_vert_flags("Inputs", p->inputs);
   gl_print_vert_flags("Forbidden", p->forbidden_inputs);
   gl_print_vert_flags("Outputs", p->outputs);

   if (0) 
      for (i = 0 ; i < ctx->NrPipelineStages ; i++) 
	 if (pipeline[i].active & p->type) {
	    fprintf(stderr,"%u: %s\n", i, pipeline[i].name);
	    
	    gl_print_vert_flags("\tinputs", pipeline[i].inputs);
	    gl_print_vert_flags("\toutputs", pipeline[i].outputs);
	    
	    if (p->type == PIPE_PRECALC && pipeline[i].pre_forbidden_inputs)
	       gl_print_vert_flags("\tforbidden", 
				   pipeline[i].pre_forbidden_inputs);

	    if (p->type == PIPE_IMMEDIATE && pipeline[i].elt_forbidden_inputs)
	       gl_print_vert_flags("\tforbidden", 
				   pipeline[i].elt_forbidden_inputs);

	 }


   if (1) {
      struct gl_pipeline_stage **stages = p->stages;
      fprintf(stderr,"\nStages requiring precalculation:\n");
      for ( i=0 ; stages[i] ; i++) {
	 fprintf(stderr, "%u: %s\n", i, stages[i]->name);
	 gl_print_vert_flags("\tinputs", stages[i]->inputs);
	 gl_print_vert_flags("\toutputs", stages[i]->outputs);
	 if (p->type == PIPE_PRECALC && pipeline[i].pre_forbidden_inputs)
	    gl_print_vert_flags("\tforbidden", 
				pipeline[i].pre_forbidden_inputs);

	 if (p->type == PIPE_IMMEDIATE && pipeline[i].elt_forbidden_inputs)
	    gl_print_vert_flags("\tforbidden", 
				pipeline[i].elt_forbidden_inputs);
      }
   }
}



void gl_print_active_pipeline( GLcontext *ctx, struct gl_pipeline *p )
{
   struct gl_pipeline_stage **stages = p->stages;
   GLuint i;

   (void) ctx;

   fprintf(stderr,"Type: %s\n", pipeline_name[p->type]);

   if (!p->pipeline_valid) {
      printf("--> Not up to date!!!\n");
      return;
   }

   gl_print_vert_flags("Inputs", p->inputs);
   gl_print_vert_flags("Forbidden", p->forbidden_inputs);
   gl_print_vert_flags("Outputs", p->outputs);

   for ( i=0 ; stages[i] ; i++) {
      fprintf(stderr, "%u: %s\n", i, stages[i]->name);

      gl_print_vert_flags("\tinputs", stages[i]->inputs);
      gl_print_vert_flags("\toutputs", stages[i]->outputs);

      if (p->type == PIPE_PRECALC && stages[i]->pre_forbidden_inputs)
	    gl_print_vert_flags("\tforbidden", 
				stages[i]->pre_forbidden_inputs);
      }
}
#endif

