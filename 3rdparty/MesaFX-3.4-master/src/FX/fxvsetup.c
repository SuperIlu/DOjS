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


/* fxvsetup.c - 3Dfx VooDoo vertices setup functions */


#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#if defined(FX)

#include "fxdrv.h"
#include "mmath.h"
#include "pipeline.h"
#include "fxvsetup.h"

#ifdef MESA_DEBUG
void fxPrintSetupFlags( const char *msg, GLuint flags )
{
   fprintf(stderr, "%s: %d %s%s%s%s%s%s\n",
	  msg,
	  flags,
	  (flags & SETUP_XY) ? " xy," : "", 
	  (flags & SETUP_Z)  ? " z," : "",
	  (flags & SETUP_W)  ? " w," : "",
	  (flags & SETUP_RGBA) ? " rgba," : "",
	  (flags & SETUP_TMU0)  ? " tmu0," : "",
	  (flags & SETUP_TMU1)  ? " tmu1," : "");
}
#endif

static void project_texcoords( struct vertex_buffer *VB,
			       GLuint tmu_nr, GLuint tc_nr,
			       GLuint start, GLuint count )
{			       
   fxVertex *v = FX_DRIVER_DATA(VB)->verts + start;
   GrTmuVertex *tmu = &(((GrVertex *)v->f)->tmuvtx[tmu_nr]);
   GLvector4f *vec = VB->TexCoordPtr[tc_nr];

   GLuint i;
   GLuint stride = vec->stride;
   GLfloat *data = VEC_ELT(vec, GLfloat, start);

   for (i = start ; i < count ; i++, STRIDE_F(data, stride), v++) {
      tmu->oow = v->f[OOWCOORD] * data[3];
      tmu = (GrTmuVertex *)((char *)tmu + sizeof(fxVertex));
   }      
}


static void copy_w( struct vertex_buffer *VB,
		    GLuint tmu_nr, 
		    GLuint start, GLuint count )
{			       
   fxVertex *v = FX_DRIVER_DATA(VB)->verts + start;
   GrTmuVertex *tmu = &(((GrVertex *)v->f)->tmuvtx[tmu_nr]);
   GLuint i;

   for (i = start ; i < count ; i++, v++) {
      tmu->oow = v->f[OOWCOORD];
      tmu = (GrTmuVertex *)((char *)tmu + sizeof(fxVertex));
   }      
}


static tfxSetupFunc setupfuncs[0x40];




#define IND SETUP_XY
#define NAME fxsetupXY
#include "fxvs_tmp.h"

#define IND (SETUP_XY|SETUP_Z)
#define NAME fxsetupXYZ
#include "fxvs_tmp.h"

#define IND (SETUP_XY|SETUP_W)
#define NAME fxsetupXYW
#include "fxvs_tmp.h"

#define IND (SETUP_XY|SETUP_Z|SETUP_W)
#define NAME fxsetupXYZW
#include "fxvs_tmp.h"

#define IND (SETUP_RGBA|SETUP_XY)
#define NAME fxsetupXYRGBA
#include "fxvs_tmp.h"

#define IND (SETUP_RGBA|SETUP_XY|SETUP_Z)
#define NAME fxsetupXYZRGBA
#include "fxvs_tmp.h"

#define IND (SETUP_RGBA|SETUP_XY|SETUP_W)
#define NAME fxsetupXYWRGBA
#include "fxvs_tmp.h"

#define IND (SETUP_RGBA|SETUP_XY|SETUP_Z|SETUP_W)
#define NAME fxsetupXYZWRGBA
#include "fxvs_tmp.h"

#define IND (SETUP_TMU0|SETUP_XY|SETUP_W)
#define NAME fxsetupXYWT0
#include "fxvs_tmp.h"

#define IND (SETUP_TMU0|SETUP_XY|SETUP_Z|SETUP_W)
#define NAME fxsetupXYZWT0
#include "fxvs_tmp.h"

#define IND (SETUP_TMU1|SETUP_TMU0|SETUP_XY|SETUP_W)
#define NAME fxsetupXYWT0T1
#include "fxvs_tmp.h"

#define IND (SETUP_TMU1|SETUP_TMU0|SETUP_XY|SETUP_Z|SETUP_W)
#define NAME fxsetupXYZWT0T1
#include "fxvs_tmp.h"

#define IND (SETUP_TMU0|SETUP_RGBA|SETUP_XY|SETUP_W)
#define NAME fxsetupXYWRGBAT0
#include "fxvs_tmp.h"

#define IND (SETUP_TMU0|SETUP_RGBA|SETUP_XY|SETUP_Z|SETUP_W)
#define NAME fxsetupXYZWRGBAT0
#include "fxvs_tmp.h"

#define IND (SETUP_TMU1|SETUP_TMU0|SETUP_RGBA|SETUP_XY|SETUP_W)
#define NAME fxsetupXYWRGBAT0T1
#include "fxvs_tmp.h"

#define IND (SETUP_TMU1|SETUP_TMU0|SETUP_RGBA|SETUP_XY|SETUP_Z|SETUP_W)
#define NAME fxsetupXYZWRGBAT0T1
#include "fxvs_tmp.h"

#define IND (SETUP_RGBA)
#define NAME fxsetupRGBA
#include "fxvs_tmp.h"

#define IND (SETUP_TMU0)
#define NAME fxsetupT0
#include "fxvs_tmp.h"

#define IND (SETUP_TMU1)
#define NAME fxsetupT1
#include "fxvs_tmp.h"

#define IND (SETUP_TMU1|SETUP_TMU0)
#define NAME fxsetupT0T1
#include "fxvs_tmp.h"

#define IND (SETUP_TMU0|SETUP_RGBA)
#define NAME fxsetupRGBAT0
#include "fxvs_tmp.h"

#define IND (SETUP_TMU1|SETUP_RGBA)
#define NAME fxsetupRGBAT1
#include "fxvs_tmp.h"

#define IND (SETUP_TMU1|SETUP_TMU0|SETUP_RGBA)
#define NAME fxsetupRGBAT0T1
#include "fxvs_tmp.h"

#define IND (SETUP_W|SETUP_RGBA)
#define NAME fxsetupWRGBA
#include "fxvs_tmp.h"

#define IND (SETUP_W|SETUP_TMU0)
#define NAME fxsetupWT0
#include "fxvs_tmp.h"

#define IND (SETUP_W|SETUP_TMU1)
#define NAME fxsetupWT1
#include "fxvs_tmp.h"

#define IND (SETUP_W|SETUP_TMU1|SETUP_TMU0)
#define NAME fxsetupWT0T1
#include "fxvs_tmp.h"

#define IND (SETUP_W|SETUP_TMU0|SETUP_RGBA)
#define NAME fxsetupWRGBAT0
#include "fxvs_tmp.h"

#define IND (SETUP_W|SETUP_TMU1|SETUP_RGBA)
#define NAME fxsetupWRGBAT1
#include "fxvs_tmp.h"

#define IND (SETUP_W|SETUP_TMU1|SETUP_TMU0|SETUP_RGBA)
#define NAME fxsetupWRGBAT0T1
#include "fxvs_tmp.h"



void fxDDSetupInit( void )
{
   setupfuncs[SETUP_XY] = fxsetupXY;
   setupfuncs[SETUP_XY|SETUP_Z] = fxsetupXYZ;
   setupfuncs[SETUP_XY|SETUP_W] = fxsetupXYW;
   setupfuncs[SETUP_XY|SETUP_Z|SETUP_W] = fxsetupXYZW;

   setupfuncs[SETUP_RGBA|SETUP_XY] = fxsetupXYRGBA;
   setupfuncs[SETUP_RGBA|SETUP_XY|SETUP_Z] = fxsetupXYZRGBA;
   setupfuncs[SETUP_RGBA|SETUP_XY|SETUP_W] = fxsetupXYWRGBA;
   setupfuncs[SETUP_RGBA|SETUP_XY|SETUP_Z|SETUP_W] = fxsetupXYZWRGBA;

   /* If we have texture and xy then we must have w.
    * If we have texture1 and w then we must have texture 0.
    */
   setupfuncs[SETUP_TMU0|SETUP_XY|SETUP_W] = fxsetupXYWT0;
   setupfuncs[SETUP_TMU0|SETUP_XY|SETUP_Z|SETUP_W] = fxsetupXYZWT0;

   setupfuncs[SETUP_TMU1|SETUP_TMU0|SETUP_XY|SETUP_W] = fxsetupXYWT0T1;
   setupfuncs[SETUP_TMU1|SETUP_TMU0|SETUP_XY|SETUP_Z|SETUP_W] = fxsetupXYZWT0T1;

   setupfuncs[SETUP_TMU0|SETUP_RGBA|SETUP_XY|SETUP_W] = fxsetupXYWRGBAT0;
   setupfuncs[SETUP_TMU0|SETUP_RGBA|SETUP_XY|SETUP_Z|SETUP_W] = fxsetupXYZWRGBAT0;

   setupfuncs[SETUP_TMU1|SETUP_TMU0|SETUP_RGBA|SETUP_XY|SETUP_W] = fxsetupXYWRGBAT0T1;
   setupfuncs[SETUP_TMU1|SETUP_TMU0|SETUP_RGBA|SETUP_XY|SETUP_Z|SETUP_W] = fxsetupXYZWRGBAT0T1;

   /* If we don't have xy then we can't have z... w is still a possibility.
    */
   setupfuncs[SETUP_RGBA] = fxsetupRGBA;
   setupfuncs[SETUP_TMU0] = fxsetupT0;
   setupfuncs[SETUP_TMU1] = fxsetupT1;
   setupfuncs[SETUP_TMU1|SETUP_TMU0] = fxsetupT0T1;
   setupfuncs[SETUP_TMU0|SETUP_RGBA] = fxsetupRGBAT0;
   setupfuncs[SETUP_TMU1|SETUP_RGBA] = fxsetupRGBAT1;
   setupfuncs[SETUP_TMU1|SETUP_TMU0|SETUP_RGBA] = fxsetupRGBAT0T1;

   setupfuncs[SETUP_W|SETUP_RGBA] = fxsetupWRGBA;
   setupfuncs[SETUP_W|SETUP_TMU0] = fxsetupWT0;
   setupfuncs[SETUP_W|SETUP_TMU1] = fxsetupWT1;
   setupfuncs[SETUP_W|SETUP_TMU1|SETUP_TMU0] = fxsetupWT0T1;
   setupfuncs[SETUP_W|SETUP_TMU0|SETUP_RGBA] = fxsetupWRGBAT0;
   setupfuncs[SETUP_W|SETUP_TMU1|SETUP_RGBA] = fxsetupWRGBAT1;
   setupfuncs[SETUP_W|SETUP_TMU1|SETUP_TMU0|SETUP_RGBA] = fxsetupWRGBAT0T1;

}



tfxSetupFunc fxDDChooseSetupFunction(GLcontext *ctx)
{
   GLuint setupindex = SETUP_XY|SETUP_Z;
   fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;

   fxMesa->setupindex = 0;

   if (ctx->RenderMode != GL_RENDER)
      return 0;

   fxMesa->tmu_source[0] = 0;
   fxMesa->tmu_source[1] = 1;

   fxMesa->tex_dest[0] = SETUP_TMU0;
   fxMesa->tex_dest[1] = SETUP_TMU1;

   if (ctx->Light.ShadeModel == GL_SMOOTH && !ctx->Light.Model.TwoSide)
      setupindex |= SETUP_RGBA;

   if (ctx->Fog.Enabled && ctx->FogMode==FOG_FRAGMENT)
      setupindex |= SETUP_RGBA|SETUP_W;

   if ((ctx->Texture.ReallyEnabled & (TEXTURE0_2D|TEXTURE0_3D)) == TEXTURE0_2D) 
   {
     /* This only works for GL_RGBA texture format.
      if (ctx->Texture.Unit[0].EnvMode == GL_REPLACE) 
	 setupindex &= ~SETUP_RGBA;
     */
      setupindex |= SETUP_TMU0|SETUP_W;
   }

   if ((ctx->Texture.ReallyEnabled & (TEXTURE1_2D|TEXTURE1_3D)) == TEXTURE1_2D)
   {
     setupindex |= SETUP_TMU1|SETUP_W;
     if (setupindex & SETUP_TMU0) { /* both TMUs in use */
       struct gl_texture_object *tObj=ctx->Texture.Unit[0].CurrentD[2];
       tfxTexInfo *ti=fxTMGetTexInfo(tObj);

       if (ti->whichTMU!=FX_TMU0) { /* TMU0 and TMU1 are swapped */
	 fxMesa->tmu_source[0] = 1; fxMesa->tex_dest[1] = SETUP_TMU0;
	 fxMesa->tmu_source[1] = 0; fxMesa->tex_dest[0] = SETUP_TMU1;
       }
     }
   }

   if (ctx->Color.BlendEnabled)
      setupindex |= SETUP_RGBA;

#ifdef MESA_DEBUG
   if (MESA_VERBOSE & (VERBOSE_DRIVER|VERBOSE_PIPELINE|VERBOSE_STATE))
      fxPrintSetupFlags("fxmesa: vertex setup function", setupindex); 
#endif

   fxMesa->setupindex = setupindex;
   fxMesa->view_clip_tri = fxTriViewClipTab[setupindex&0x7];
   fxMesa->clip_tri_stride = fxTriClipStrideTab[setupindex&0x7];
   return setupfuncs[setupindex];
}

void fxDDDoRasterSetup( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   FX_DRIVER_DATA(VB)->last_vert = FX_DRIVER_DATA(VB)->verts + VB->Count;

#if 0 /* leaving this out fixes the Heretic2 stray polygon bug */
   if ((ctx->IndirectTriangles & DD_SW_RASTERIZE) == DD_SW_RASTERIZE) {
      fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
      fxMesa->setupdone = 0;
      return;
   }
#endif

   if (VB->Type == VB_CVA_PRECALC) 
      fxDDPartialRasterSetup( VB );
   else {
      if ((ctx != NULL) && (ctx->Driver.RasterSetup != NULL))
         ctx->Driver.RasterSetup( VB, VB->CopyStart, VB->Count );
   }
}


/*
 * Need to check that merge&render will work before allowing this to
 * happen here.  Therefore - need to know that this will be fired when
 * we get a forbidden input in the elt pipeline - and therefore need to check
 * whether we have one *now*.  Similarly need to know if state changes cause
 * size4 texcoords to be introduced.  
 */
void fxDDCheckPartialRasterSetup( GLcontext *ctx, struct gl_pipeline_stage *d )
{
   fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
   GLuint tmp = fxMesa->setupdone;

   d->type = 0;
   d->pre_forbidden_inputs = 0;
   fxMesa->setupdone = 0;	/* cleared if we return */

   /* Indirect triangles must be rendered via the immediate pipeline.  
    * If all rasterization is software, no need to set up.  
    */
#ifdef VAO
   if ((ctx->Array.Current->Summary & VERT_OBJ_ANY) == 0)
#else
   if ((ctx->Array.Summary & VERT_OBJ_ANY) == 0)
#endif
      return;
   
   if ((ctx->IndirectTriangles & DD_SW_SETUP) ||
       (ctx->IndirectTriangles & DD_SW_RASTERIZE) == DD_SW_RASTERIZE) 
      return;

   if ((ctx->Texture.ReallyEnabled & 0xf) &&
#ifdef VAO
       !(ctx->Array.Current->Flags & VERT_TEX0_ANY))
#else
       !(ctx->Array.Flags & VERT_TEX0_ANY))
#endif
   {
      if (ctx->TextureMatrix[0].type == MATRIX_GENERAL ||
	  ctx->TextureMatrix[0].type == MATRIX_PERSPECTIVE ||
	  (ctx->Texture.Unit[1].TexGenEnabled & Q_BIT))
	 return;

      d->pre_forbidden_inputs |= VERT_TEX0_4;
   }

   if ((ctx->Texture.ReallyEnabled & 0xf0) &&
#ifdef VAO
       !(ctx->Array.Current->Flags & VERT_TEX1_ANY))
#else
       !(ctx->Array.Flags & VERT_TEX1_ANY))
#endif
   {
      if (ctx->TextureMatrix[1].type == MATRIX_GENERAL ||
	  ctx->TextureMatrix[1].type == MATRIX_PERSPECTIVE ||
	  (ctx->Texture.Unit[1].TexGenEnabled & Q_BIT))
	 return;

      d->pre_forbidden_inputs |= VERT_TEX1_4;
   }

   
   fxMesa->setupdone = tmp;
   d->inputs = 0;
   d->outputs = VERT_SETUP_PART;
   d->type = PIPE_PRECALC;
}


/* Will be different every time - no point in trying to precalc the
 * function to call.
 */
void fxDDPartialRasterSetup( struct vertex_buffer *VB )
{
   GLuint newOut = VB->pipeline->new_outputs;
   fxMesaContext fxMesa = (fxMesaContext)VB->ctx->DriverCtx;
   GLuint ind = 0;

   FX_DRIVER_DATA(VB)->last_vert = FX_DRIVER_DATA(VB)->verts + VB->Count;

   if (newOut & VERT_WIN) {
      newOut = VB->pipeline->outputs;
      ind |= SETUP_XY|SETUP_W|SETUP_Z;
   }

   if (newOut & VERT_TEX0_ANY)
      ind |= SETUP_W | fxMesa->tex_dest[0];

   if (newOut & VERT_TEX1_ANY)
      ind |= SETUP_W | fxMesa->tex_dest[1];

   if (newOut & VERT_RGBA)
      ind |= SETUP_W|SETUP_RGBA;

   if ((newOut & VERT_WIN) == 0)
      ind &= ~(fxMesa->setupdone & SETUP_W);
      
   fxMesa->setupdone &= ~ind;
   ind &= fxMesa->setupindex;
   fxMesa->setupdone |= ind;

#ifdef MESA_DEBUG
   if (MESA_VERBOSE & (VERBOSE_DRIVER|VERBOSE_PIPELINE)) {
      gl_print_vert_flags("new outputs", VB->pipeline->new_outputs);
      fxPrintSetupFlags("fxmesa: partial setup function", ind); 
   }
#endif

   if (ind) 
      setupfuncs[ind]( VB, VB->Start, VB->Count );   
}

/* Almost certainly never called.
 */
void fxDDResizeVB( struct vertex_buffer *VB, GLuint size )
{
   struct tfxMesaVertexBuffer *fvb = FX_DRIVER_DATA(VB);

   while (fvb->size < size)
      fvb->size *= 2;

   ALIGN_FREE( VB->ClipMask );
   VB->ClipMask = (GLubyte *) ALIGN_MALLOC(sizeof(GLubyte) * fvb->size, 4);

   FREE( fvb->vert_store );
   fvb->vert_store = MALLOC( sizeof(fxVertex) * fvb->size + 31);
   if (!fvb->vert_store || !VB->ClipMask) 
   {
     fprintf(stderr,"fx Driver: out of memory !\n");
     fxCloseHardware();
     exit(-1);
   }
   fvb->verts = (fxVertex *)(((unsigned long)fvb->vert_store + 31) & ~31);

   gl_vector1ui_free( &fvb->clipped_elements );
   gl_vector1ui_alloc( &fvb->clipped_elements, VEC_WRITABLE, fvb->size, 32 );
   
   if (!fvb->clipped_elements.start) goto memerror;
   
   return;
memerror: 
   fprintf(stderr,"fx Driver: out of memory !\n");
   fxCloseHardware();
   exit(-1);
}


void fxDDRegisterVB( struct vertex_buffer *VB )
{
   struct tfxMesaVertexBuffer *fvb;

   fvb = (struct tfxMesaVertexBuffer *)calloc( 1, sizeof(*fvb) );

   /* This looks like it allocates a lot of memory, but it basically
    * just sets an upper limit on how much can be used - nothing like
    * this amount will ever be turned into 'real' memory.
    */
   if (VB->Type == VB_CVA_PRECALC) {
      fvb->size = VB->Size * 5;
      fvb->vert_store = MALLOC( sizeof(fxVertex) * fvb->size + 31);
      if (!fvb->vert_store) goto memerror;
#if defined(FX_GLIDE3) 
      fvb->triangle_b = MALLOC( sizeof(GrVertex*) *4* fvb->size+ 31);
      if (!fvb->triangle_b)  goto memerror;
      fvb->strips_b = MALLOC( sizeof(GrVertex*) *4* fvb->size+ 31);
      if (!fvb->strips_b )  goto memerror;          
#endif     
      fvb->verts = (fxVertex *)(((unsigned long)fvb->vert_store + 31) & ~31);
      gl_vector1ui_alloc( &fvb->clipped_elements, VEC_WRITABLE, fvb->size, 32 );
      if (!fvb->clipped_elements.start) goto memerror;

      ALIGN_FREE( VB->ClipMask );
      VB->ClipMask = (GLubyte *) ALIGN_MALLOC(sizeof(GLubyte) * fvb->size, 4);
      if (!VB->ClipMask) goto memerror;

   } else {
      fvb->vert_store = MALLOC( sizeof(fxVertex) * (VB->Size + 12) + 31);
      if (!fvb->vert_store) goto memerror;
#if defined(FX_GLIDE3) 
      fvb->triangle_b = MALLOC( sizeof(GrVertex*) *4* fvb->size+ 31);
      if (!fvb->triangle_b)  goto memerror;
      fvb->strips_b = MALLOC( sizeof(GrVertex*) *4* fvb->size+ 31);
      if (!fvb->strips_b )  goto memerror;     
#endif       
      fvb->verts = (fxVertex *)(((unsigned long)fvb->vert_store + 31) & ~31);
      fvb->size = VB->Size + 12;
   }

   
   VB->driver_data = fvb;
   return;
memerror:
   fprintf(stderr,"fx Driver: out of memory !\n");
   fxCloseHardware();
   exit(-1);   
}

void fxDDUnregisterVB( struct vertex_buffer *VB )
{
   struct tfxMesaVertexBuffer *fvb = FX_DRIVER_DATA(VB);
   
   if (fvb) {
      if (fvb->vert_store) FREE(fvb->vert_store);
      gl_vector1ui_free( &fvb->clipped_elements );
      FREE(fvb);
#if defined(FX_GLIDE3)
      if (fvb->strips_b)
      	FREE(fvb->strips_b);
      if (fvb->triangle_b)
      	FREE(fvb->triangle_b);
#endif
      VB->driver_data = 0;
   }      
}



#else


/*
 * Need this to provide at least one external definition.
 */

int gl_fx_dummy_function_vsetup(void)
{
  return 0;
}

#endif  /* FX */
