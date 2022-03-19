/* -*- mode: C; tab-width:8; c-basic-offset:2 -*- */

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
 *
 *
 * Original Mesa / 3Dfx device driver (C) 1999 David Bucciarelli, by the
 * terms stated above.
 *
 * Thank you for your contribution, David!
 *
 * Please make note of the above copyright/license statement.  If you
 * contributed code or bug fixes to this code under the previous (GNU
 * Library) license and object to the new license, your code will be
 * removed at your request.  Please see the Mesa docs/COPYRIGHT file
 * for more information.
 *
 * Additional Mesa/3Dfx driver developers:
 *   Daryll Strauss <daryll@precisioninsight.com>
 *   Keith Whitwell <keith@precisioninsight.com>
 *
 * See fxapi.h for more revision/author details.
 */


#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif


#if defined(FX)

#include "fxdrv.h"
#include "vbindirect.h"


/* We don't handle texcoord-4 in the safe clip routines - maybe we should.
 */
static void fxDDRenderElements( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;

   if (fxMesa->render_index != 0 ||
       ((ctx->Texture.ReallyEnabled & 0xf) && VB->TexCoordPtr[0]->size>2) ||
       ((ctx->Texture.ReallyEnabled & 0xf0) && VB->TexCoordPtr[1]->size>2) ||
	(VB->ClipPtr->size != 4)) /* Brokes clipping otherwise */
      gl_render_elts( VB );
   else 
      fxDDRenderElementsDirect( VB );
}

static void fxDDCheckRenderVBIndirect( GLcontext *ctx, 
				       struct gl_pipeline_stage *d )
{   
   d->type = 0;

   if ((ctx->IndirectTriangles & DD_SW_SETUP) == 0 &&
       ctx->Driver.MultipassFunc == 0) 
   {
      d->type = PIPE_IMMEDIATE;
      d->inputs = VERT_SETUP_FULL | VERT_ELT | VERT_PRECALC_DATA;
   }      
}

static void fxDDRenderVBIndirect( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
   struct vertex_buffer *cvaVB = ctx->CVA.VB;

   if (fxMesa->render_index != 0 ||
       ((ctx->Texture.ReallyEnabled & 0xf) && cvaVB->TexCoordPtr[0]->size>2) ||
       ((ctx->Texture.ReallyEnabled & 0xf0) && cvaVB->TexCoordPtr[1]->size>2) ||
       (VB->ClipPtr->size != 4)) /* Brokes clipping otherwise */
      gl_render_vb_indirect( VB );
   else
      fxDDRenderVBIndirectDirect( VB );
}


static void fxDDRenderVB( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;

   if ((fxMesa->render_index != 0) ||
       ((ctx->Texture.ReallyEnabled & 0xf) && VB->TexCoordPtr[0]->size>2) ||
       ((ctx->Texture.ReallyEnabled & 0xf0) && VB->TexCoordPtr[1]->size>2))
      gl_render_vb( VB );
   else
      fxDDDoRenderVB( VB );
}




/* This sort of driver-based reconfiguration of the pipeline could be
 * used to support accelerated transformation and lighting on capable
 * hardware.
 *
 */
GLuint fxDDRegisterPipelineStages( struct gl_pipeline_stage *out,
				   const struct gl_pipeline_stage *in,
				   GLuint nr )
{
   GLuint i, o;

   for (i = o = 0 ; i < nr ; i++) {
      switch (in[i].ops) {
      case PIPE_OP_RAST_SETUP_1|PIPE_OP_RENDER:
	 out[o] = in[i];
	 out[o].state_change = NEW_CLIENT_STATE;
	 out[o].check = fxDDCheckMergeAndRender;
	 out[o].run = fxDDMergeAndRender;	 
	 o++;
	 break;
      case PIPE_OP_RAST_SETUP_0:
	 out[o] = in[i];
	 out[o].cva_state_change = NEW_LIGHTING|NEW_TEXTURING|NEW_RASTER_OPS;
	 out[o].state_change = ~0;
	 out[o].check = fxDDCheckPartialRasterSetup;
	 out[o].run = fxDDPartialRasterSetup;
	 o++;
	 break;
      case PIPE_OP_RAST_SETUP_0|PIPE_OP_RAST_SETUP_1:
	 out[o] = in[i];
	 out[o].run = fxDDDoRasterSetup;
	 o++;
	 break;
      case PIPE_OP_RENDER:
	 out[o] = in[i];
	 if (in[i].run == gl_render_elts) {
  	    out[o].run = fxDDRenderElements; 
	 } else if (in[i].run == gl_render_vb_indirect) {
	    out[o].check = fxDDCheckRenderVBIndirect;
	    out[o].run = fxDDRenderVBIndirect;
	 } else if (in[i].run == gl_render_vb) {
  	    out[o].run = fxDDRenderVB; 
	 }

	 o++;
	 break;
      default:
	 out[o++] = in[i];
	 break;
      }
   }

   return o;
}

#define ILLEGAL_ENABLES (TEXTURE0_3D|		\
			 TEXTURE1_3D|		\
			 ENABLE_TEXMAT0 |	\
			 ENABLE_TEXMAT1 |	\
			 ENABLE_TEXGEN0 |	\
			 ENABLE_TEXGEN1 |	\
			 ENABLE_USERCLIP |	\
			 ENABLE_LIGHT |		\
			 ENABLE_FOG)


			 
/* Because this is slotted in by the OptimizePipeline function, most
 * of the information here is just for gl_print_pipeline().  Only the
 * run member is required.  
 */
static struct gl_pipeline_stage fx_fast_stage = {
   "FX combined vertex transform, setup and rasterization stage",
   PIPE_OP_VERT_XFORM|PIPE_OP_RAST_SETUP_0|PIPE_OP_RAST_SETUP_1|PIPE_OP_RENDER,
   PIPE_PRECALC,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,				/* never called */
   fxDDFastPath
};




/* Better than optimizing the pipeline, we can do the whole build very
 * quickly with the aid of a new flags member.
 */
GLboolean fxDDBuildPrecalcPipeline( GLcontext *ctx )
{   
   struct gl_pipeline *pipe = &ctx->CVA.pre;
   fxMesaContext fxMesa = FX_CONTEXT(ctx);
   

   if (fxMesa->is_in_hardware &&
       fxMesa->render_index == 0 && 
       (ctx->Enabled & ILLEGAL_ENABLES) == 0 &&
#ifdef VAO
       (ctx->Array.Current->Flags & (VERT_OBJ_234|
#else
       (ctx->Array.Flags & (VERT_OBJ_234|
#endif
			    VERT_TEX0_4|
			    VERT_TEX1_4|
			    VERT_ELT)) == (VERT_OBJ_23|VERT_ELT))
   {
      if (MESA_VERBOSE & (VERBOSE_STATE|VERBOSE_DRIVER)) 
	 if (!fxMesa->using_fast_path)
	    fprintf(stderr, "fxMesa: using fast path\n");

      pipe->stages[0] = &fx_fast_stage;
      pipe->stages[1] = 0;
      pipe->new_inputs = ctx->RenderFlags & VERT_DATA;
      pipe->ops = pipe->stages[0]->ops;
      fxMesa->using_fast_path = 1;
      return 1;
   } 

   if (fxMesa->using_fast_path) 
   {
#if 0
      if (MESA_VERBOSE & (VERBOSE_STATE|VERBOSE_DRIVER)) 
 	 fprintf(stderr, "fxMesa: fall back to full pipeline %x %x %x %x %x\n",
		 fxMesa->is_in_hardware,
		 fxMesa->render_index,
		 (ctx->Enabled & ILLEGAL_ENABLES),
		 (ctx->Array.Summary & (VERT_OBJ_23)),
		 (ctx->Array.Summary & (VERT_OBJ_4|VERT_TEX0_4|VERT_TEX1_4)));
#endif

      fxMesa->using_fast_path = 0;
      ctx->CVA.VB->ClipOrMask = 0;
      ctx->CVA.VB->ClipAndMask = CLIP_ALL_BITS;
#ifdef VAO
      ctx->Array.NewArrayState |= ctx->Array.Current->Summary;
#else
      ctx->Array.NewArrayState |= ctx->Array.Summary;
#endif
      return 0;
   }
   
   return 0;
}






/* Perform global optimizations to the pipeline.  The fx driver
 * implements a single such fast path, which corresponds to the standard
 * quake3 cva pipeline.
 *
 * This is now handled by the 'build' function above.
 */
void fxDDOptimizePrecalcPipeline( GLcontext *ctx, struct gl_pipeline *pipe )
{
   fxMesaContext fxMesa = FX_CONTEXT(ctx);

   if (fxMesa->is_in_hardware &&
       fxMesa->render_index == 0 && 
       (ctx->Enabled & ILLEGAL_ENABLES) == 0 &&
#ifdef VAO
       (ctx->Array.Current->Summary & VERT_ELT))
#else
       (ctx->Array.Summary & VERT_ELT))
#endif
   {
      pipe->stages[0] = &fx_fast_stage;
      pipe->stages[1] = 0;
   }
}



/* unused?
void fxDDOptimizeEltPipeline( GLcontext *ctx, struct gl_pipeline *pipe )
{
   (void) ctx;
   (void) pipe;
}
*/

#else

/*
 * Need this to provide at least one external definition.
 */
int gl_fxpipeline_dummy(void)
{
  return 0;
}

#endif
