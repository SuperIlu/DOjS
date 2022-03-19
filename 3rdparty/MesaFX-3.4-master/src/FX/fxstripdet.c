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
#if defined(FX) && defined(FX_GLIDE3)

#include "fxdrv.h"
#include "vbcull.h"

 
#define STRIP0(u,v)		((u1 == v1) && (u2 == v0))	
#define STRIP1(u,v)		((u0 == v0) && (u2 == v1))

#define LOCAL_VARS		fxVertex* gWin = FX_DRIVER_DATA(VB)->verts;	\
				GrVertex** sb = FX_DRIVER_DATA(VB)->strips_b;	
				
#define STRIPSLOCAL_VAR		int sc	       = 0;

#define INIT(a)			

#define SENDTRI(u0,u1,u2)	FX_grDrawTriangle((GrVertex*)&(gWin[u0].f),(GrVertex*)&(gWin[u1].f),(GrVertex*)&(gWin[u2].f))
#define FLUSHTRI()		/* No-Op */
#define STARTSTRIPS(u0,u1,u2)	{ sb[sc++] = (GrVertex*)&(gWin[u0].f); sb[sc++] = (GrVertex*)&(gWin[u1].f); sb[sc++] = (GrVertex*)&(gWin[u2].f); }
#define SENDSTRIPS(v2)		{ sb[sc++] = (GrVertex*)&(gWin[v2].f); }
#define FLUSHSTRIPS()		FX_grDrawVertexArray(GR_TRIANGLE_STRIP,sc,sb)

#define CLIPPED(a,b,c)		0
#define CULLED(a,b,c)		0
#define SENDCLIPTRI(a,b,c)	/* NoOp */		

#define TAG(x)			x##_fx

#include "fxsdettmp.h"


/* Clipped but no userclip */
#define STRIP0(u,v)		((u1 == v1) && (u2 == v0)) && !clipmask[v2]	
#define STRIP1(u,v)		((u0 == v0) && (u2 == v1)) && !clipmask[v2]

#define LOCAL_VARS		fxVertex* gWin = FX_DRIVER_DATA(VB)->verts;			\
				GrVertex** sb = FX_DRIVER_DATA(VB)->strips_b;			\
				const GLubyte *const clipmask = VB->ClipMask;				\
				const fxMesaContext fxMesa=(fxMesaContext)VB->ctx->DriverCtx;	\
				const tfxTriClipFunc cliptri = fxMesa->clip_tri_stride;
				
#define STRIPSLOCAL_VAR		int sc	       = 0;

#define INIT(a)			

#define SENDTRI(u0,u1,u2)	FX_grDrawTriangle((GrVertex*)&(gWin[u0].f),(GrVertex*)&(gWin[u1].f),(GrVertex*)&(gWin[u2].f))
#define FLUSHTRI()		/* No-Op */
#define STARTSTRIPS(u0,u1,u2)	{ sb[sc++] = (GrVertex*)&(gWin[u0].f); sb[sc++] = (GrVertex*)&(gWin[u1].f); sb[sc++] = (GrVertex*)&(gWin[u2].f); }
#define SENDSTRIPS(v2)		{ sb[sc++] = (GrVertex*)&(gWin[v2].f); }
#define FLUSHSTRIPS()		FX_grDrawVertexArray(GR_TRIANGLE_STRIP,sc,sb)

#define CLIPPED(u0,u1,u2)	(clipmask[u0] | clipmask[u1] | clipmask[u2])
#define CULLED(u0,u1,u2)	(clipmask[u0] & clipmask[u1] & clipmask[u2])
#define SENDCLIPTRI(u0,u1,u2)	{									\
				   GLuint vl[3];							\
    				   ASSIGN_3V(vl, u0, u1, u2 );						\
				   cliptri(VB,vl,clipmask[u0] | clipmask[u1] | clipmask[u2]);		\
				}
 
#define TAG(x)			x##_fx_view_clipped

#include "fxsdettmp.h"

/* Clipped and might be userclip */
#define STRIP0(u,v)		((u1 == v1) && (u2 == v0)) && !clipmask[v2]	
#define STRIP1(u,v)		((u0 == v0) && (u2 == v1)) && !clipmask[v2]

#define LOCAL_VARS		fxVertex* gWin = FX_DRIVER_DATA(VB)->verts;			\
				GrVertex** sb = FX_DRIVER_DATA(VB)->strips_b;			\
				const GLubyte *const clipmask = VB->ClipMask;			\
				const GLubyte *userclipmask = VB->UserClipMask;			\
				const fxMesaContext fxMesa=(fxMesaContext)VB->ctx->DriverCtx;	\
				const tfxTriClipFunc cliptri = fxMesa->clip_tri_stride;
				
#define STRIPSLOCAL_VAR		int sc	       = 0;

#define INIT(a)			

#define SENDTRI(u0,u1,u2)	FX_grDrawTriangle((GrVertex*)&(gWin[u0].f),(GrVertex*)&(gWin[u1].f),(GrVertex*)&(gWin[u2].f))
#define FLUSHTRI()		/* No-Op */
#define STARTSTRIPS(u0,u1,u2)	{ sb[sc++] = (GrVertex*)&(gWin[u0].f); sb[sc++] = (GrVertex*)&(gWin[u1].f); sb[sc++] = (GrVertex*)&(gWin[u2].f); }
#define SENDSTRIPS(v2)		{ sb[sc++] = (GrVertex*)&(gWin[v2].f); }
#define FLUSHSTRIPS()		FX_grDrawVertexArray(GR_TRIANGLE_STRIP,sc,sb)

#define CLIPPED(u0,u1,u2)	(clipmask[u0] | clipmask[u1] | clipmask[u2])
#define CULLED(u0,u1,u2)	(clipmask[u0] & clipmask[u1] & clipmask[u2] & CLIP_ALL_BITS)
#define SENDCLIPTRI(u0,u1,u2)	{									                  \
    					GLuint vl[3];							                  \
    					GLuint imask = (clipmask[u0] | clipmask[u1] | clipmask[u2]);			  \
    															  \
					if (imask & CLIP_USER_BIT) {					                  \
      						if (!(userclipmask[u2] & userclipmask[u1] & userclipmask[u0]))		  \
      						{ ASSIGN_3V(vl, u2, u1, u0 );						  \
      						  imask |= (userclipmask[u2] | userclipmask[u1] | userclipmask[u0]) << 8; \
      						   cliptri( VB, vl, imask );}					          \
    					}										  \
    					else { ASSIGN_3V(vl, u2, u1, u0 );						  \
    					    cliptri( VB, vl, imask );	}						  \
  				}
 
#define TAG(x)			x##_fx_clipped

#include "fxsdettmp.h"


void fxDDRenderInitGlide3(GLcontext *ctx)
{
#if 0
  render_tab_fx_smooth_indirect[GL_TRIANGLES] = render_vb_triangles_smooth_indirect_sd_fx;
  render_tab_fx_smooth_indirect_view_clipped[GL_TRIANGLES] = render_vb_triangles_smooth_indirect_sd_fx_view_clipped;
  render_tab_fx_smooth_indirect_clipped[GL_TRIANGLES] = render_vb_triangles_smooth_indirect_sd_fx_clipped;
#endif
}


#endif /* defined(FX) && FX_GLIDE3 */
