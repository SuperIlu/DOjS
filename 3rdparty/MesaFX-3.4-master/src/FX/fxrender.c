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


/* fxrender.c - 3Dfx VooDoo RenderVB driver function support */


#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#if defined(FX)

#include "fxdrv.h"
#include "vbcull.h"

/*
 * Render a line segment from VB[v1] to VB[v2] when either one or both
 * endpoints must be clipped.
 */
#if !defined(__MWERKS__) 
INLINE
#endif
void fxRenderClippedLine( struct vertex_buffer *VB, 
				 GLuint v1, GLuint v2 )
{
  fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;
  GLubyte mask = VB->ClipMask[v1] | VB->ClipMask[v2];

  if (!mask || (VB->ctx->line_clip_tab[VB->ClipPtr->size])(VB, &v1, &v2, mask))
    FX_grDrawLine((GrVertex *)gWin[v1].f,(GrVertex *)gWin[v2].f);
}




/* This is legal for Quads as well as triangles, hence the 'n' parameter.
 */
INLINE void fxRenderClippedTriangle( struct vertex_buffer *VB,
					 GLuint n, GLuint vlist[] )
{
  GLubyte mask = 0;
  GLuint i;

  for (i = 0 ; i < n ; i++)
    mask |= VB->ClipMask[vlist[i]];

  if (mask & CLIP_USER_BIT) {
    GLubyte *userclipmask = VB->UserClipMask;
    if (userclipmask[vlist[0]] & userclipmask[vlist[1]] & userclipmask[vlist[2]])
      return;
  }
   
  n = (VB->ctx->poly_clip_tab[VB->ClipPtr->size])( VB, n, vlist, mask );
  if (n >= 3) {
    fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;
    GrVertex *i0 = (GrVertex *)gWin[vlist[0]].f;
    GrVertex *i1 = (GrVertex *)gWin[vlist[1]].f;
    GrVertex *i2 = (GrVertex *)gWin[vlist[2]].f;
    GLuint i;
      
    for (i=2;i<n;i++, i1 = i2, i2 = (GrVertex *)gWin[vlist[i]].f) {
      FX_grDrawTriangle(i0,i1,i2);
    }
  }
}





static INLINE void fxSafeClippedLine( struct vertex_buffer *VB, 
				      GLuint v1, GLuint v2 )
{
  GLubyte mask = VB->ClipMask[v1] | VB->ClipMask[v2];

  if (!mask) {
    fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;
    FX_grDrawLine((GrVertex *)gWin[v1].f,(GrVertex *)gWin[v2].f);
  } else {
    fxMesaContext fxMesa = (fxMesaContext)VB->ctx->DriverCtx;
    fxLineClipTab[fxMesa->setupindex & 0x7]( VB, v1, v2, mask );
  }
}


static INLINE void fxSafeClippedTriangle( struct vertex_buffer *VB,
					  fxVertex *gWin,
					  tfxTriClipFunc cliptri,
					  GLuint v2, GLuint v1, GLuint v )
{
  GLubyte *clipmask = VB->ClipMask;
  GLubyte mask = clipmask[v2] | clipmask[v1] | clipmask[v];

  if (!mask) {
    FX_grDrawTriangle((GrVertex *)gWin[v2].f,
		      (GrVertex *)gWin[v1].f,
		      (GrVertex *)gWin[v].f);
    return;
  }

  if (!(clipmask[v2] & clipmask[v1] & clipmask[v] & CLIP_ALL_BITS))
  {
    GLuint vl[3];
    GLuint imask = mask;

    if (imask & CLIP_USER_BIT) {
      GLubyte *userclipmask = VB->UserClipMask;
      if (userclipmask[v2] & userclipmask[v1] & userclipmask[v])
	return;
      imask |= (userclipmask[v2] | userclipmask[v1] | userclipmask[v]) << 8;
    }

    ASSIGN_3V(vl, v2, v1, v );
    cliptri( VB, vl, imask );
  }
}


static INLINE void fxSafeClippedTriangle2( struct vertex_buffer *VB,
					   fxVertex *gWin,
					   tfxTriViewClipFunc cliptri,
					   GLuint v2, GLuint v1, GLuint v )
{
  GLubyte *clipmask = VB->ClipMask;
  GLubyte mask = clipmask[v2] | clipmask[v1] | clipmask[v];

  if (!mask) {
    FX_grDrawTriangle((GrVertex *)gWin[v2].f,(GrVertex *)gWin[v1].f,
		      (GrVertex *)gWin[v].f);
  } else if (!(clipmask[v2] & clipmask[v1] & clipmask[v])) {
    GLuint vl[3];
    ASSIGN_3V(vl, v2, v1, v );
    cliptri( VB, vl, mask );
  }
}


static INLINE void fxSafeClippedTriangle3( struct vertex_buffer *VB,
					   fxVertex *gWin,
					   tfxTriClipFunc cliptri,
					   GLuint v2, GLuint v1, GLuint v )
{
  GLubyte *clipmask = VB->ClipMask;
  GLubyte mask = clipmask[v2] | clipmask[v1] | clipmask[v];
  GLuint imask = mask;

  if (imask & CLIP_USER_BIT) {
    GLubyte *userclipmask = VB->UserClipMask;
    if (userclipmask[v2] & userclipmask[v1] & userclipmask[v])
      return;
    imask |= (userclipmask[v2] | userclipmask[v1] | userclipmask[v]) << 8;
  }

  {
    GLuint vl[3];
    ASSIGN_3V(vl, v2, v1, v );
    cliptri( VB, vl, imask );
  }
}





/************************************************************************/
/************************ RenderVB functions ****************************/
/************************************************************************/


/* Render front-facing, non-clipped primitives.
 */

#define RENDER_POINTS( start, count )				\
   (void) gWin;							\
   (void) VB;							\
   (VB->ctx->Driver.PointsFunc)( VB->ctx, start, count )

#define RENDER_LINE( i1, i )			\
  do {						\
     RVB_COLOR(i);				\
     FX_grDrawLine((GrVertex *)gWin[i1].f,	\
		(GrVertex *)gWin[i].f);		\
  } while (0)

#define RENDER_TRI( i2, i1, i, pv, parity )	\
  do {						\
    RVB_COLOR(pv);				\
    if (parity) {				\
      FX_grDrawTriangle((GrVertex *)gWin[i1].f,	\
		      (GrVertex *)gWin[i2].f,	\
		      (GrVertex *)gWin[i].f);	\
    } else {					\
      FX_grDrawTriangle((GrVertex *)gWin[i2].f,	\
		      (GrVertex *)gWin[i1].f,	\
		      (GrVertex *)gWin[i].f);	\
    }						\
  } while (0)

#define RENDER_QUAD( i3, i2, i1, i, pv )	\
  do {						\
    RVB_COLOR(pv);				\
    FX_grDrawTriangle((GrVertex *)gWin[i3].f,	\
		    (GrVertex *)gWin[i2].f,	\
		    (GrVertex *)gWin[i].f);	\
    FX_grDrawTriangle((GrVertex *)gWin[i2].f,	\
		    (GrVertex *)gWin[i1].f,	\
		    (GrVertex *)gWin[i].f);	\
  } while (0)



#define LOCAL_VARS						\
   fxMesaContext fxMesa=(fxMesaContext)VB->ctx->DriverCtx;	\
   fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;			\
   (void) fxMesa;

#define INIT(x)  

#define TAG(x) x##_fx_flat_raw
#undef  RVB_COLOR
#define RVB_COLOR(pv)  FX_VB_COLOR(fxMesa, VB->ColorPtr->data[pv])
#define PRESERVE_VB_DEFS

#include "render_tmp.h"

#define TAG(x) x##_fx_smooth_raw
#undef  RVB_COLOR
#define RVB_COLOR(x)

#include "render_tmp.h"



/* Render with clipped and/or culled primitives with cullmask information.
 */
#define RENDER_POINTS( start, count )				\
   (void) gWin;							\
   (void) cullmask;						\
   (VB->ctx->Driver.PointsFunc)( VB->ctx, start, count )


#define RENDER_LINE( i1, i )						\
  do {									\
    const GLubyte flags = cullmask[i];					\
									\
    if (!(flags & PRIM_NOT_CULLED))					\
      continue;								\
									\
    RVB_COLOR(i);							\
    if (flags & PRIM_ANY_CLIP)						\
      fxRenderClippedLine( VB, i1, i );					\
    else								\
      FX_grDrawLine( (GrVertex *)gWin[i1].f, (GrVertex *)gWin[i].f );	\
 } while (0)


#define RENDER_TRI( i2, i1, i, pv, parity)		\
  do {							\
    const GLubyte flags = cullmask[i];			\
    GLuint e2,e1;					\
							\
    if (!(flags & PRIM_NOT_CULLED))			\
      continue;						\
							\
     e2=i2, e1=i1;					\
     if (parity) { e2=i1; e1=i2; }			\
							\
     RVB_COLOR(pv);					\
     if (flags & PRIM_ANY_CLIP) {			\
       fxSafeClippedTriangle3(VB,gWin,cliptri,e2,e1,i);	\
     } else {						\
       FX_grDrawTriangle((GrVertex *)gWin[e2].f,	\
		    (GrVertex *)gWin[e1].f,		\
		    (GrVertex *)gWin[i].f);		\
     }							\
  } while (0)


#define RENDER_QUAD(i3, i2, i1, i, pv)			\
  do {							\
    const GLubyte flags = cullmask[i];			\
							\
    if (!(flags & PRIM_NOT_CULLED))			\
      continue;						\
							\
    RVB_COLOR(pv);					\
    if (flags&PRIM_ANY_CLIP) {				\
      fxSafeClippedTriangle3(VB,gWin,cliptri,i3,i2,i);	\
      fxSafeClippedTriangle3(VB,gWin,cliptri,i2,i1,i);	\
    } else {						\
      FX_grDrawTriangle((GrVertex *)gWin[i3].f,		\
		     (GrVertex *)gWin[i2].f,		\
		     (GrVertex *)gWin[i].f);		\
      FX_grDrawTriangle((GrVertex *)gWin[i2].f,		\
		     (GrVertex *)gWin[i1].f,		\
		     (GrVertex *)gWin[i].f);		\
    }							\
  } while (0)





#define LOCAL_VARS						\
   fxMesaContext fxMesa=(fxMesaContext)VB->ctx->DriverCtx;	\
   fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;			\
   const GLubyte *cullmask = VB->CullMask;			\
   tfxTriClipFunc cliptri = fxMesa->clip_tri_stride;




#define INIT(x)     (void) cliptri; (void) fxMesa;

#define TAG(x) x##_fx_smooth_culled
#undef  RVB_COLOR
#define RVB_COLOR(x)
#define PRESERVE_VB_DEFS
#include "render_tmp.h"

#define TAG(x) x##_fx_flat_culled
#undef  RVB_COLOR
#define RVB_COLOR(pv)  FX_VB_COLOR(fxMesa, VB->ColorPtr->data[pv])

#include "render_tmp.h"




/* Direct, with the possibility of clipping.
 */ 
#define RENDER_POINTS( start, count )		\
  do {						\
    fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;	\
    GLubyte *clipmask = VB->ClipMask;		\
    GLuint i;					\
    for (i = start ; i < count ; i++)		\
      if (clipmask[i] == 0) {			\
        RVB_COLOR(i);				\
        FX_grDrawPoint( (GrVertex *)gWin[i].f );\
      }						\
  } while (0)

#define RENDER_LINE( i1, i )			\
  do {						\
    RVB_COLOR(i);				\
    fxSafeClippedLine( VB, i1, i );		\
  } while (0)

#define RENDER_TRI( i2, i1, i, pv, parity)		\
  do {							\
    GLuint e2=i2, e1=i1;					\
    if (parity) { GLuint t=e2; e2=e1; e1=t; }		\
    RVB_COLOR(pv);					\
    fxSafeClippedTriangle(VB,gWin,cliptri,e2,e1,i);	\
  } while (0)

#define RENDER_QUAD( i3, i2, i1, i, pv)			\
  do {							\
    RVB_COLOR(pv);					\
    fxSafeClippedTriangle(VB,gWin,cliptri,i3,i2,i);	\
    fxSafeClippedTriangle(VB,gWin,cliptri,i2,i1,i);	\
  } while (0)

#define LOCAL_VARS						\
   fxMesaContext fxMesa=(fxMesaContext)VB->ctx->DriverCtx;	\
   fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;			\
   tfxTriClipFunc cliptri = fxMesa->clip_tri_stride;

#define INIT(x)  (void) cliptri; (void) gWin;

#define TAG(x) x##_fx_smooth_clipped
#undef  RVB_COLOR
#define RVB_COLOR(x)
#define PRESERVE_VB_DEFS
#include "render_tmp.h"


#define TAG(x) x##_fx_flat_clipped
#undef  RVB_COLOR
#define RVB_COLOR(pv)  FX_VB_COLOR(fxMesa, VB->ColorPtr->data[pv])
#include "render_tmp.h"






/* Indirect, with the possibility of clipping.
 */ 
#define RENDER_POINTS( start, count )			\
  do {							\
    fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;		\
    GLuint e;						\
    GLubyte *clipmask = VB->ClipMask;			\
    for(e=start;e<count;e++)				\
      if(clipmask[elt[e]]==0) {				\
        FX_grDrawPoint((GrVertex *)gWin[elt[e]].f);	\
      }							\
  } while (0)

#define RENDER_LINE( i1, i )			\
  do {						\
    GLuint e1 = elt[i1], e = elt[i];		\
    RVB_COLOR(e);				\
    fxSafeClippedLine( VB, e1, e );		\
  } while (0)

#define RENDER_TRI( i2, i1, i, pv, parity)		\
  do {							\
    GLuint e2 = elt[i2], e1 = elt[i1], e = elt[i];	\
    if (parity) { GLuint t=e2; e2=e1; e1=t; }		\
    fxSafeClippedTriangle(VB,gWin,cliptri,e2,e1,e);	\
  } while (0)

#define RENDER_QUAD( i3, i2, i1, i, pv)				\
  do {								\
    GLuint e3 = elt[i3], e2 = elt[i2], e1 = elt[i1], e = elt[i];\
    fxSafeClippedTriangle(VB,gWin,cliptri,e3,e2,e);		\
    fxSafeClippedTriangle(VB,gWin,cliptri,e2,e1,e);		\
  } while (0)

#define LOCAL_VARS const GLuint *elt = VB->EltPtr->data;	\
   fxMesaContext fxMesa = (fxMesaContext)VB->ctx->DriverCtx;	\
   fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;			\
   tfxTriClipFunc cliptri = fxMesa->clip_tri_stride;

#define INIT(x)  (void) cliptri; (void) gWin;

#define TAG(x) x##_fx_smooth_indirect_clipped
#undef  RVB_COLOR
#define RVB_COLOR(x)
#include "render_tmp.h"


/* Indirect, clipped, but no user clip.
 */
#define RENDER_POINTS( start, count )			\
  do {							\
    fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;		\
    GLuint e;						\
    GLubyte *clipmask = VB->ClipMask;			\
    for(e=start;e<count;e++)				\
      if(clipmask[elt[e]]==0) {				\
        FX_grDrawPoint((GrVertex *)gWin[elt[e]].f);	\
      }							\
  } while (0)

#define RENDER_LINE( i1, i )			\
  do {						\
    GLuint e1 = elt[i1], e = elt[i];		\
    RVB_COLOR(e);				\
    fxSafeClippedLine( VB, e1, e );		\
  } while (0)

#define RENDER_TRI( i2, i1, i, pv, parity)		\
  do {							\
    GLuint e2 = elt[i2], e1 = elt[i1], e = elt[i];	\
    if (parity) { GLuint t=e2; e2=e1; e1=t; }		\
    fxSafeClippedTriangle2(VB,gWin,cliptri,e2,e1,e);	\
  } while (0)

#define RENDER_QUAD( i3, i2, i1, i, pv)				\
  do {								\
    GLuint e3 = elt[i3], e2 = elt[i2], e1 = elt[i1], e = elt[i];\
    fxSafeClippedTriangle2(VB,gWin,cliptri,e3,e2,e);		\
    fxSafeClippedTriangle2(VB,gWin,cliptri,e2,e1,e);		\
  } while (0)

#define LOCAL_VARS const GLuint *elt = VB->EltPtr->data;	\
   fxMesaContext fxMesa = (fxMesaContext)VB->ctx->DriverCtx;	\
   fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;			\
   tfxTriViewClipFunc cliptri = fxMesa->view_clip_tri;

#define INIT(x)  (void) cliptri; (void) gWin;

#define TAG(x) x##_fx_smooth_indirect_view_clipped
#undef  RVB_COLOR
#define RVB_COLOR(x)
#include "render_tmp.h"







/* Indirect, and no clipping required.
 */
#define RENDER_POINTS( start, count )			\
  do {							\
    GLuint e;						\
    for(e=start;e<count;e++) {				\
      FX_grDrawPoint((GrVertex *)gWin[elt[e]].f);	\
    }							\
  } while (0)

#define RENDER_LINE( i1, i )						\
  do {									\
    GLuint e1 = elt[i1], e = elt[i];					\
    FX_grDrawLine((GrVertex *)gWin[e1].f, (GrVertex *)gWin[e].f);	\
  } while (0)


#define RENDER_TRI( i2, i1, i, pv, parity)		\
  do {							\
    GLuint e2 = elt[i2], e1 = elt[i1], e = elt[i];	\
    if (parity) {GLuint tmp = e2; e2 = e1; e1 = tmp;}	\
    FX_grDrawTriangle((GrVertex *)gWin[e2].f,		\
		 (GrVertex *)gWin[e1].f,		\
		 (GrVertex *)gWin[e].f);		\
  } while (0)


#define RENDER_QUAD( i3, i2, i1, i, pv)				\
  do {								\
    GLuint e3 = elt[i3], e2 = elt[i2], e1 = elt[i1], e = elt[i];\
    FX_grDrawTriangle((GrVertex *)gWin[e3].f,			\
		 (GrVertex *)gWin[e2].f,			\
		 (GrVertex *)gWin[e].f);			\
    FX_grDrawTriangle((GrVertex *)gWin[e2].f,			\
		 (GrVertex *)gWin[e1].f,			\
		 (GrVertex *)gWin[e].f);			\
  } while (0)

#define LOCAL_VARS				\
   fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;	\
   const GLuint *elt = VB->EltPtr->data;

#define INIT(x)  

#define TAG(x) x##_fx_smooth_indirect
#undef  RVB_COLOR
#define RVB_COLOR(x)
#include "render_tmp.h"





/* Direct in this context means that triangles, lines, points can be
 * rendered simply by calling grDrawTriangle, etc., without any
 * additional setup (such as calling grConstantColor).  We also use a
 * 'safe' set of clipping routines which don't require write-access to
 * the arrays in the vertex buffer, and don't care about array
 * stride.  
 *
 * Thus there is no call to gl_import_arrays() in this function.  
 *
 * This safe clipping should be generalized to call driver->trianglefunc
 * under the appropriate conditions.
 *
 * We don't handle texcoord-4 in the safe clip routines - maybe we should.
 *
 */
void fxDDRenderElementsDirect( struct vertex_buffer *VB )
{
  GLcontext *ctx = VB->ctx;
  struct vertex_buffer *saved_vb = ctx->VB;
  GLenum prim = ctx->CVA.elt_mode;
  GLuint nr = VB->EltPtr->count;
  render_func func = render_tab_fx_smooth_indirect[prim];
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  GLuint p = 0;

  if (!nr) 
    return;

  if (fxMesa->new_state) 
    fxSetupFXUnits(ctx);

  if (!nr) return;

  if (VB->ClipOrMask) {
    func = render_tab_fx_smooth_indirect_view_clipped[prim];
    if (VB->ClipOrMask & CLIP_USER_BIT)
      func = render_tab_fx_smooth_indirect_clipped[prim];
  }
    
  ctx->VB = VB;			/* kludge */

  do {
    func( VB, 0, nr, 0 );
  } while (ctx->Driver.MultipassFunc &&
	   ctx->Driver.MultipassFunc( VB, ++p ));


  ctx->VB = saved_vb;
}


void fxDDRenderVBIndirectDirect( struct vertex_buffer *VB )
{
  GLcontext *ctx = VB->ctx;
  struct vertex_buffer *cvaVB = ctx->CVA.VB;
  struct vertex_buffer *saved_vb = ctx->VB;
  GLuint i, next, count = VB->Count;
  render_func *tab = render_tab_fx_smooth_indirect;
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  GLuint p = 0;

  if (cvaVB->ClipOrMask)
    tab = render_tab_fx_smooth_indirect_clipped;
    
  if (!VB->CullDone)
    gl_fast_copy_vb( VB );

  if (fxMesa->new_state) 
    fxSetupFXUnits( ctx );

  ctx->VB = cvaVB;
  cvaVB->EltPtr = VB->EltPtr;

  do {
    GLuint parity = VB->Parity;

    for (i = VB->CopyStart ; i < count ; parity = 0, i = next) 
    {
      GLuint prim = VB->Primitive[i];
      next = VB->NextPrimitive[i];
      tab[prim]( cvaVB, i, next, parity );
    }
    /* loop never taken */
  } while (ctx->Driver.MultipassFunc &&
	   ctx->Driver.MultipassFunc( cvaVB, ++p ));

  cvaVB->EltPtr = 0;
  ctx->VB = saved_vb;
}


static render_func *fxDDRenderVBSmooth_tables[3] = {
  render_tab_fx_smooth_clipped,
  render_tab_fx_smooth_culled,
  render_tab_fx_smooth_raw
};

static render_func *fxDDRenderVBFlat_tables[3] = {
  render_tab_fx_flat_clipped,
  render_tab_fx_flat_culled,
  render_tab_fx_flat_raw
};


static render_func *null_tables[3] = {
  0,
  0,
  0
};

#if defined(FX_GLIDE3)
#include "fxstripdet.c"
#endif

void fxDDRenderInit( GLcontext *ctx )
{
   render_init_fx_smooth_indirect_view_clipped();
   render_init_fx_smooth_indirect_clipped();
   render_init_fx_smooth_indirect();
   render_init_fx_smooth_raw();
   render_init_fx_smooth_culled();
   render_init_fx_smooth_clipped();
   render_init_fx_flat_raw();
   render_init_fx_flat_culled();
   render_init_fx_flat_clipped();
#if defined(FX_GLIDE3)
   fxDDRenderInitGlide3(ctx);
#endif
}


/* Now used to set an internal var in fxMesa - we hook out at the
 * level of gl_render_vb() instead.
 */
render_func **fxDDChooseRenderVBTables(GLcontext *ctx)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

  if (ctx->IndirectTriangles & DD_SW_SETUP) 
    return null_tables;  

  switch (fxMesa->render_index) {
/*    case FX_FLAT: */
/*      return fxDDRenderVBFlat_tables; */
  case 0:
    return fxDDRenderVBSmooth_tables; 
  default:
    return null_tables;    
  }
}


void fxDDDoRenderVB( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
   GLuint i, next, prim;
   GLuint parity = VB->Parity;
   render_func *tab;
   GLuint count = VB->Count;
   GLint p = 0;

   if (fxMesa->new_state) 
     fxSetupFXUnits(ctx);

   if (VB->Indirect) { 
      return; 
   } else if (VB->CullMode & CLIP_MASK_ACTIVE) {
      tab = fxMesa->RenderVBClippedTab;
   } else {
      tab = fxMesa->RenderVBRawTab;
   }

   if (!VB->CullDone)
      gl_fast_copy_vb( VB );

   do
   {      
      for ( i= VB->CopyStart ; i < count ; parity = 0, i = next ) 
      {
	 prim = VB->Primitive[i];
	 next = VB->NextPrimitive[i];
	 tab[prim]( VB, i, next, parity );
      }

   } while (ctx->Driver.MultipassFunc &&
	    ctx->Driver.MultipassFunc( VB, ++p ));
}


#else


/*
 * Need this to provide at least one external definition.
 */

int gl_fx_dummy_function_render(void)
{
  return 0;
}

#endif  /* FX */
