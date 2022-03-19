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


#if (IND & (SETUP_XY|SETUP_W|SETUP_Z))
#define V1 VARS_XY
#define I1 , INCR_XY
#else
#define V1
#define I1
#endif

#if (IND & SETUP_XY)
#define S1 DO_SETUP_XY
#else
#define S1
#endif

#if (IND & SETUP_W)
#define S2 S1 DO_SETUP_W
#define V2 V1 VARS_W
#else
#define S2 S1
#define V2 V1
#endif

#if (IND & SETUP_Z)
#define S3 S2 DO_SETUP_Z
#else
#define S3 S2
#endif

#if (IND & SETUP_RGBA)
#define V4 V2 VARS_RGBA
#define S4 S3 DO_SETUP_RGBA
#define I4 I1 , INCR_RGBA
#else
#define V4 V2
#define S4 S3
#define I4 I1
#endif

#if (IND & SETUP_TMU0)
#define V5 V4 VARS_TMU0
#define S5 S4 DO_SETUP_TMU0
#define I5 I4 , INCR_TMU0
#define F5    FIXUP_TMU0
#else
#define V5 V4
#define S5 S4
#define I5 I4
#define F5
#endif

#if (IND & SETUP_TMU1)
#define V6 V5 VARS_TMU1
#define S6 S5 DO_SETUP_TMU1
#define I6 I5 , INCR_TMU1
#define F6 F5 FIXUP_TMU1
#else
#define V6 V5
#define S6 S5
#define I6 I5
#define F6 F5
#endif

#if (IND & SETUP_TMU0) && (IND & SETUP_TMU1)
#define F7 FIXUP_TMU01
#else
#define F7 F6
#endif

#define VARS V6
#define DO_SETUP S6
#define INCR I6
#define FIXUP F7

static void NAME(struct vertex_buffer *VB, GLuint start, GLuint end)
{
   GLcontext *ctx = VB->ctx;
   fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;

   if (fxMesa->new_state) 
      fxSetupFXUnits( ctx );

   {
      const float snapper = (3L<<18);
      fxVertex *gWin = FX_DRIVER_DATA(VB)->verts;
      GLfloat *v = gWin[start].f;
      GLfloat *vend = gWin[end].f;
      VARS;

      (void) gWin;
      (void) fxMesa;
      (void) snapper;

      if (VB->ClipOrMask) {
	 GLubyte *clipmask = &VB->ClipMask[start];
	 for (;v!=vend;v+=16,clipmask++ INCR) {
	    if (*clipmask == 0) {
	       DO_SETUP;
	    }
	 }
      }
      else {
	 for (;v!=vend;v+=16 INCR) {
	    DO_SETUP;
	 }
      }

      if (ctx->FogMode == FOG_FRAGMENT && ctx->ProjectionMatrix.m[15] != 0.0F) {
        /* need to compute W values for fogging purposes */
        const GLfloat m10 = ctx->ProjectionMatrix.m[10];
        const GLfloat m14 = ctx->ProjectionMatrix.m[14];
        const GLfloat v10 = ctx->Viewport.WindowMap.m[10];
        const GLfloat v14 = ctx->Viewport.WindowMap.m[14];
        GLfloat *v = gWin[start].f;
        GLfloat *win = VB->Win.data[start]; 
        if (VB->ClipOrMask) {
           GLubyte *clipmask = &VB->ClipMask[start];
           for (;v!=vend;v+=16,clipmask++, win+=4) {
              if (*clipmask == 0) {
                GLfloat zNDC = (win[2] - v14) / v10;
                GLfloat zEye = (zNDC - m14) / m10;
                v[OOWCOORD] = -1.0F / zEye;
              }
           }
        }
        else {
           for (;v!=vend;v+=16, win+=4) {
             GLfloat zNDC = (win[2] - v14) / v10;
             GLfloat zEye = (zNDC - m14) / m10;
             v[OOWCOORD] = -1.0F / zEye;
           }
        }
      }

      /* rare - I hope */
      FIXUP;
   }
}


#undef V1
#undef V2
#undef V3
#undef V4
#undef V5
#undef V6
#undef VARS

#undef S1
#undef S2
#undef S3
#undef S4
#undef S5
#undef S6
#undef DO_SETUP

#undef I1
#undef I4
#undef I5
#undef I6
#undef INCR

#undef F5
#undef F6
#undef F7
#undef FIXUP


#undef IND
#undef NAME

