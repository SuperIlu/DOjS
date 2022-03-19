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


/* fxcva.c - the CVA related code */


#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#if defined(FX)

#include "fxdrv.h"
#include "mmath.h"
#include "vbindirect.h"
#include "pb.h"
#include "fxvsetup.h"
#include "pipeline.h"
#include "stages.h"

#define PRIM_POINTS 0
#define PRIM_LINES  1
#define PRIM_TRIS   2
#define PRIM_CULLED 3


static const GLuint reduce_prim[GL_POLYGON+2] = {
   PRIM_POINTS,
   PRIM_LINES,
   PRIM_LINES,
   PRIM_LINES,
   PRIM_TRIS,
   PRIM_TRIS,
   PRIM_TRIS,
   PRIM_TRIS,
   PRIM_TRIS,
   PRIM_TRIS,
   PRIM_CULLED
};

typedef void (*mergefunc)( struct vertex_buffer *cvaVB,
			   struct vertex_buffer *VB,
			   const struct gl_prim_state *state,
			   GLuint start,
			   GLuint count );

static void fxCvaRenderNoop( struct vertex_buffer *cvaVB,
			     struct vertex_buffer *VB,
			     const struct gl_prim_state *state,
			     GLuint start,
			     GLuint count )
{
}

static INLINE void fxRenderClippedTriangle2( struct vertex_buffer *VB,
					     GLuint v1, GLuint v2, GLuint v3 )
{
  fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;
  GLubyte *clipmask = VB->ClipMask;
  GLubyte mask = clipmask[v1] | clipmask[v2] | clipmask[v3];

  if (!mask) {
     FX_grDrawTriangle((GrVertex *)gWin[v1].f, 
		    (GrVertex *)gWin[v2].f, 
		    (GrVertex *)gWin[v3].f);
  } else if (!(clipmask[v1]&clipmask[v2]&clipmask[v3]&CLIP_ALL_BITS)) {    
    GLuint n;
    GLuint vlist[VB_MAX_CLIPPED_VERTS];
    ASSIGN_3V(vlist, v1, v2, v3);

    n = (VB->ctx->poly_clip_tab[VB->ClipPtr->size])( VB, 3, vlist, mask );
    if (n >= 3) {
      GLuint i, j0 = vlist[0];
      for (i=2;i<n;i++) {
	FX_grDrawTriangle((GrVertex *)gWin[j0].f,
		       (GrVertex *)gWin[vlist[i-1]].f,
		       (GrVertex *)gWin[vlist[i]].f);
      }
    }
  }
}


static mergefunc merge_and_render_tab[2][MAX_MERGABLE][PRIM_CULLED+1];


/*
#define CVA_VARS_RGBA							\
   GLubyte (*color)[4] = VB->ColorPtr->data;			\
   GLubyte (*cva_color)[4] = (cvaVB->ColorPtr = cvaVB->LitColor[0])->data;
*/

#define CVA_VARS_RGBA							\
   GLubyte (*color)[4] = VB->ColorPtr->data;			\
   GLubyte (*cva_color)[4] = cvaVB->ColorPtr->data;



#undef DO_SETUP_RGBA
#if FX_USE_PARGB
#define DO_SETUP_RGBA				\
{									\
  GLubyte *col = color[i];			\
  GET_PARGB(v)=	((col[3] << 24) |	\
  			    (col[0] << 16)  |	\
  			    (col[1] << 8)   |   \
  			    (col[2]));			\
}
#else
#define DO_SETUP_RGBA				\
{							\
  GLubyte *col = color[i];					\
  v[GR_VERTEX_R_OFFSET]=UBYTE_COLOR_TO_FLOAT_255_COLOR(col[0]);	\
  v[GR_VERTEX_G_OFFSET]=UBYTE_COLOR_TO_FLOAT_255_COLOR(col[1]);	\
  v[GR_VERTEX_B_OFFSET]=UBYTE_COLOR_TO_FLOAT_255_COLOR(col[2]);	\
  v[GR_VERTEX_A_OFFSET]=UBYTE_COLOR_TO_FLOAT_255_COLOR(col[3]);	\
}
#endif /* FX_USE_PARGB */


#define CVA_VARS_TMU0							\
   VARS_TMU0								\
   GLfloat (*cva_tex0)[4] = (cvaVB->TexCoordPtr[tmu0_source] = cvaVB->store.TexCoord[tmu0_source])->data;

#define CVA_VARS_TMU1							\
   VARS_TMU1								\
   GLfloat (*cva_tex1)[4] = (cvaVB->TexCoordPtr[tmu1_source] = cvaVB->store.TexCoord[tmu1_source])->data;


#define INIT_RGBA (void) cva_color;
#define INIT_TMU0 (void) cva_tex0; (void) tmu0_stride; (void) tmu0_sz;
#define INIT_TMU1 (void) cva_tex1; (void) tmu1_stride; (void) tmu1_sz;


#define DRAW_POINT FX_grDrawPoint( (GrVertex *)v )
#define DRAW_LINE FX_grDrawLine( (GrVertex *)v, (GrVertex *)prev_v )
#define DRAW_TRI FX_grDrawTriangle( (GrVertex *)gWin[l[0]].f, (GrVertex *)gWin[l[1]].f, (GrVertex *)v )
#define DRAW_TRI2 FX_grDrawTriangle( vl[0], vl[1], vl[2] )
#define CLIP_LINE fxRenderClippedLine( cvaVB, e, prev )
#define CLIP_OR_DRAW_TRI fxRenderClippedTriangle2( cvaVB, l[0], l[1], e )
#define DIRECT 1

#define TAG(x) x
#define IDX 0
#define VARS
#define INIT
#define INCR
#define MERGE_RAST
#define MERGE_VB
#include "fxcvatmp.h"

#define TAG(x) x##RGBA
#define IDX SETUP_RGBA
#define VARS CVA_VARS_RGBA
#define INIT INIT_RGBA
#define INCR
#define MERGE_RAST DO_SETUP_RGBA
#define MERGE_VB COPY_4UBV(cva_color[e], color[i])
#include "fxcvatmp.h"

#define TAG(x) x##T0
#define IDX SETUP_TMU0
#define VARS CVA_VARS_TMU0
#define INIT INIT_TMU0
#define INCR , tmu0_data+=4
#define MERGE_RAST DO_SETUP_TMU0
#define MERGE_VB COPY_2FV(cva_tex0[e], tmu0_data)
#include "fxcvatmp.h"

#define TAG(x) x##T1
#define IDX SETUP_TMU1
#define VARS CVA_VARS_TMU1
#define INIT INIT_TMU1
#define INCR , tmu1_data+=4
#define MERGE_RAST DO_SETUP_TMU1
#define MERGE_VB COPY_2FV(cva_tex1[e], tmu1_data)
#include "fxcvatmp.h"

#define TAG(x) x##T0T1
#define IDX SETUP_TMU0|SETUP_TMU1
#define VARS CVA_VARS_TMU0 CVA_VARS_TMU1 
#define INIT INIT_TMU0 INIT_TMU1 
#define INCR , tmu0_data+=4 , tmu1_data+=4
#define MERGE_RAST DO_SETUP_TMU0 DO_SETUP_TMU1
#define MERGE_VB COPY_2FV(cva_tex0[e], tmu0_data); \
                 COPY_2FV(cva_tex1[e], tmu1_data);
#include "fxcvatmp.h"

#define TAG(x) x##RGBAT0
#define IDX SETUP_RGBA|SETUP_TMU0
#define VARS CVA_VARS_RGBA CVA_VARS_TMU0 
#define INIT INIT_RGBA INIT_TMU0 
#define INCR , tmu0_data+=4 
#define MERGE_RAST DO_SETUP_RGBA; DO_SETUP_TMU0
#define MERGE_VB COPY_2FV(cva_tex0[e], tmu0_data); \
                 COPY_4UBV(cva_color[e], color[i]);
#include "fxcvatmp.h"

#define TAG(x) x##RGBAT1
#define IDX SETUP_RGBA|SETUP_TMU1
#define VARS CVA_VARS_RGBA CVA_VARS_TMU1
#define INIT INIT_RGBA INIT_TMU1
#define INCR , tmu1_data+=4
#define MERGE_RAST DO_SETUP_RGBA; DO_SETUP_TMU1
#define MERGE_VB COPY_2FV(cva_tex1[e], tmu1_data); \
                 COPY_4UBV(cva_color[e], color[i]);
#include "fxcvatmp.h"

#define TAG(x) x##RGBAT0T1
#define IDX SETUP_RGBA|SETUP_TMU0|SETUP_TMU1
#define VARS CVA_VARS_RGBA CVA_VARS_TMU0 CVA_VARS_TMU1
#define INIT INIT_RGBA INIT_TMU0 INIT_TMU1
#define INCR , tmu0_data+=4 , tmu1_data+=4
#define MERGE_RAST DO_SETUP_RGBA; DO_SETUP_TMU0 DO_SETUP_TMU1
#define MERGE_VB COPY_2FV(cva_tex0[e], tmu0_data); \
                 COPY_2FV(cva_tex1[e], tmu1_data); \
                 COPY_4UBV(cva_color[e], color[i]);
#include "fxcvatmp.h"



#undef DRAW_POINT 
#undef DRAW_LINE 
#undef DRAW_TRI 
#undef CLIP_LINE 
#undef CLIP_OR_DRAW_TRI 
#undef DIRECT

#define DRAW_POINT ctx->Driver.PointsFunc( ctx, e, e )
#define DRAW_LINE ctx->Driver.LineFunc( ctx, e, prev, e ) 
#define DRAW_TRI ctx->TriangleFunc( ctx, l[0], l[1], e, e )
#define CLIP_LINE gl_render_clipped_line( ctx, e, prev )
#define CLIP_OR_DRAW_TRI 						\
do {									\
   if (clip[l[0]] | clip[l[1]] | clip[e]) {				\
      if (!(clip[l[0]] & clip[l[1]] & clip[e] & CLIP_ALL_BITS)) {	\
	 COPY_3V(vlist, l);						\
	 gl_render_clipped_triangle( ctx, 3, vlist, e );		\
      }									\
   }									\
   else ctx->TriangleFunc( ctx, l[0], l[1], e, e );			\
} while (0)

	    
#define DIRECT 0

#define TAG(x) x##_indirect
#define IDX 0
#define VARS
#define INIT
#define INCR
#define MERGE_RAST
#define MERGE_VB
#include "fxcvatmp.h"

#define TAG(x) x##RGBA_indirect
#define IDX SETUP_RGBA
#define VARS CVA_VARS_RGBA
#define INIT INIT_RGBA
#define INCR
#define MERGE_RAST DO_SETUP_RGBA
#define MERGE_VB COPY_4UBV(cva_color[e], color[i])
#include "fxcvatmp.h"

#define TAG(x) x##T0_indirect
#define IDX SETUP_TMU0
#define VARS CVA_VARS_TMU0
#define INIT INIT_TMU0
#define INCR , tmu0_data+=4
#define MERGE_RAST DO_SETUP_TMU0
#define MERGE_VB COPY_2FV(cva_tex0[e], tmu0_data)
#include "fxcvatmp.h"

#define TAG(x) x##T1_indirect
#define IDX SETUP_TMU1
#define VARS CVA_VARS_TMU1
#define INIT INIT_TMU1
#define INCR , tmu1_data+=4
#define MERGE_RAST DO_SETUP_TMU1
#define MERGE_VB COPY_2FV(cva_tex1[e], tmu1_data)
#include "fxcvatmp.h"

#define TAG(x) x##T0T1_indirect
#define IDX SETUP_TMU0|SETUP_TMU1
#define VARS CVA_VARS_TMU0 CVA_VARS_TMU1 
#define INIT INIT_TMU0 INIT_TMU1 
#define INCR , tmu0_data+=4 , tmu1_data+=4
#define MERGE_RAST DO_SETUP_TMU0 DO_SETUP_TMU1
#define MERGE_VB COPY_2FV(cva_tex0[e], tmu0_data); \
                 COPY_2FV(cva_tex1[e], tmu1_data);
#include "fxcvatmp.h"

#define TAG(x) x##RGBAT0_indirect
#define IDX SETUP_RGBA|SETUP_TMU0
#define VARS CVA_VARS_RGBA CVA_VARS_TMU0 
#define INIT INIT_RGBA INIT_TMU0 
#define INCR , tmu0_data+=4
#define MERGE_RAST DO_SETUP_RGBA; DO_SETUP_TMU0
#define MERGE_VB COPY_2FV(cva_tex0[e], tmu0_data); \
                 COPY_4UBV(cva_color[e], color[i]);
#include "fxcvatmp.h"

#define TAG(x) x##RGBAT1_indirect
#define IDX SETUP_RGBA|SETUP_TMU1
#define VARS CVA_VARS_RGBA CVA_VARS_TMU1
#define INIT INIT_RGBA INIT_TMU1
#define INCR , tmu1_data+=4
#define MERGE_RAST DO_SETUP_RGBA; DO_SETUP_TMU1
#define MERGE_VB COPY_2FV(cva_tex1[e], tmu1_data); \
                 COPY_4UBV(cva_color[e], color[i]);
#include "fxcvatmp.h"

#define TAG(x) x##RGBAT0T1_indirect
#define IDX SETUP_RGBA|SETUP_TMU0|SETUP_TMU1
#define VARS CVA_VARS_RGBA CVA_VARS_TMU0 CVA_VARS_TMU1
#define INIT INIT_RGBA INIT_TMU0 INIT_TMU1
#define INCR , tmu0_data+=4 , tmu1_data+=4
#define MERGE_RAST DO_SETUP_RGBA DO_SETUP_TMU0 DO_SETUP_TMU1
#define MERGE_VB COPY_2FV(cva_tex0[e], tmu0_data); \
                 COPY_2FV(cva_tex1[e], tmu1_data); \
                 COPY_4UBV(cva_color[e], color[i]);
#include "fxcvatmp.h"




void fxDDCvaInit()
{
   /* Call grDrawTriangle et al */
   init_cva();
   init_cvaT0();
   init_cvaT1();
   init_cvaT0T1();
   init_cvaRGBA();
   init_cvaRGBAT0();
   init_cvaRGBAT1();
   init_cvaRGBAT0T1();

   /* Call ctx->TriangleFunc and friends */
   init_cva_indirect();
   init_cvaT0_indirect();
   init_cvaT1_indirect();
   init_cvaT0T1_indirect();
   init_cvaRGBA_indirect();
   init_cvaRGBAT0_indirect();
   init_cvaRGBAT1_indirect();
   init_cvaRGBAT0T1_indirect();
}


void fxDDCheckMergeAndRender( GLcontext *ctx, struct gl_pipeline_stage *d )
{
   GLuint inputs = ctx->RenderFlags & ~ctx->CVA.pre.outputs;

   if (!(ctx->TriangleCaps & DD_TRI_UNFILLED) &&
#ifdef VAO
       (ctx->Array.Current->Summary & VERT_OBJ_ANY))
#else
       (ctx->Array.Summary & VERT_OBJ_ANY))
#endif
   {
      d->inputs = (VERT_SETUP_PART | VERT_ELT | inputs);   
      d->outputs = 0;
      d->type = PIPE_IMMEDIATE;
   }

/*     gl_print_vert_flags("merge&render inputs", d->inputs); */
}


extern void fxPointSmooth(GLcontext *ctx, GLuint first, GLuint last);
extern void fxLineSmooth(GLcontext *ctx, GLuint v1, GLuint v2, GLuint pv);
extern void fxTriangleSmooth(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, 
			     GLuint pv);


/* static GLboolean edge_flag[GL_POLYGON+2] = { 0,0,0,0,1,0,0,1,0,1,0 }; */

void fxDDMergeAndRender( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   struct vertex_buffer *cvaVB = ctx->CVA.VB;
   fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
   GLuint i, next, prim;
   GLuint parity = VB->Parity;
   GLuint count = VB->Count;
   const struct gl_prim_state *state;
   mergefunc func;
   struct vertex_buffer *saved_vb = ctx->VB;
   GLuint inputs = ctx->RenderFlags & ~ctx->CVA.pre.outputs;
   GLuint flags = 0;
   GLuint direct = (fxMesa->render_index == 0);
   mergefunc (*tab)[PRIM_CULLED+1] = merge_and_render_tab[direct];
   GLuint p = 0;

   if (fxMesa->new_state) 
      fxSetupFXUnits(ctx);


   /* May actually contain elements not present in fxMesa->setupindex,
    * eg RGBA when flat shading.  These need to be copied into the cva
    * VB so that funcs like fxTriangleFlat will be able to reach them.
    *
    * This leads to some duplication of effort in the merge funcs.  
    */
   if (inputs & VERT_RGBA) {
      cvaVB->ColorPtr = cvaVB->Color[0] = cvaVB->LitColor[0];
      cvaVB->Color[1] = cvaVB->LitColor[1];
      flags |= SETUP_RGBA;
   }

   if (inputs & VERT_TEX0_ANY) {
      cvaVB->TexCoordPtr[0] = cvaVB->store.TexCoord[0];
      flags |= fxMesa->tex_dest[0];
   }

   if (inputs & VERT_TEX1_ANY) {
      cvaVB->TexCoordPtr[1] = cvaVB->store.TexCoord[1];
      flags |= fxMesa->tex_dest[1];
   }
#if defined(MESA_DEBUG) && 0
   fxPrintSetupFlags("FX cva merge & render", flags);
#endif

   if (cvaVB->ClipOrMask)
      gl_import_client_data( cvaVB, ctx->RenderFlags,
			     VEC_WRITABLE|VEC_GOOD_STRIDE );

   ctx->VB = cvaVB;

   do {
      for ( i= VB->CopyStart ; i < count ; parity = 0, i = next ) 
      {
	 prim = VB->Primitive[i];
	 next = VB->NextPrimitive[i];

	 state = gl_prim_state_machine[prim][parity];
	 func = tab[flags][reduce_prim[prim]];
	 func( cvaVB, VB, state, i, next );
      
	 if ( ctx->TriangleCaps & DD_TRI_LIGHT_TWOSIDE ) 
	 {
	    cvaVB->Specular = cvaVB->Spec[0];
	    cvaVB->ColorPtr = cvaVB->Color[0];
	    cvaVB->IndexPtr = cvaVB->Index[0];
	 }
      }
   } while (ctx->Driver.MultipassFunc &&
	    ctx->Driver.MultipassFunc( VB, ++p ));



   if (ctx->PB->count > 0) 
      gl_flush_pb(ctx);

   ctx->VB = saved_vb;
}



#else


/*
 * Need this to provide at least one external definition.
 */

int gl_fx_dummy_function_cva(void)
{
  return 0;
}


#endif   /* FX */
