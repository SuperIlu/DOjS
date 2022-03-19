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


#ifndef _FXVSETUP_H_
#define _FXVSETUP_H_


#define VARS_W  

#define VARS_Z

#define VARS_TMU0							\
  GLuint tmu0_source = fxMesa->tmu_source[0];				\
  GLfloat *tmu0_data = VEC_ELT(VB->TexCoordPtr[tmu0_source],		\
			       GLfloat, start);				\
  GLuint tmu0_stride = VB->TexCoordPtr[tmu0_source]->stride;		\
  GLuint tmu0_sz = VB->TexCoordPtr[tmu0_source]->size;			\
  struct gl_texture_unit *t0 = &ctx->Texture.Unit[tmu0_source];		\
  GLfloat sscale0 = FX_TEXTURE_DATA(t0)->sScale;	\
  GLfloat tscale0 = FX_TEXTURE_DATA(t0)->tScale;

#define VARS_TMU1							\
  GLuint tmu1_source = fxMesa->tmu_source[1];				\
  GLfloat *tmu1_data = VEC_ELT(VB->TexCoordPtr[tmu1_source],		\
			       GLfloat, start);				\
  GLuint tmu1_stride = VB->TexCoordPtr[tmu1_source]->stride;		\
  GLuint tmu1_sz = VB->TexCoordPtr[tmu1_source]->size;			\
  struct gl_texture_unit *t1 = &ctx->Texture.Unit[tmu1_source];		\
  GLfloat sscale1 = FX_TEXTURE_DATA(t1)->sScale;	\
  GLfloat tscale1 = FX_TEXTURE_DATA(t1)->tScale;

#define VARS_RGBA 						\
  GLubyte *color = VEC_ELT(VB->ColorPtr, GLubyte, start); 	\
  GLuint col_stride = VB->ColorPtr->stride; 

#define VARS_XY GLfloat *win = VB->Win.data[start]; 

#define INCR_XY win += 4


#ifdef FX_V2
#  define DO_SETUP_XY 				\
  v[XCOORD]=win[0];				\
  v[YCOORD]=win[1]; 
#else
#ifdef DRIVERTS
#  define DO_SETUP_XY 				\
  v[XCOORD]=win[0]+fxMesa->x_offset;		\
  v[YCOORD]=win[1]+fxMesa->y_delta;		
#else
#  if (defined(__linux__) && defined(__i386__)) || defined(macintosh) ||defined(__DJGPP__)
#    define DO_SETUP_XY {			\
       GLfloat t1 = win[0] + snapper;	\
       GLfloat t2 = win[1] + snapper;	\
       v[XCOORD] = t1 - snapper;			\
       v[YCOORD] = t2 - snapper;			\
     }
#  else
#    define DO_SETUP_XY {				\
        /* trunc (x,y) to multiple of 1/16 */		\
        v[XCOORD]=((int)(win[0]*16.0f))*(1.0f/16.0f);	\
        v[YCOORD]=((int)(win[1]*16.0f))*(1.0f/16.0f);	\
     }
#  endif
#endif
#endif


#define DO_SETUP_W {				\
   v[OOWCOORD]=win[3];			\
}

#define DO_SETUP_Z v[ZCOORD]=win[2];

#define DO_SETUP_TMU0					\
{							\
  v[S0COORD]=sscale0*tmu0_data[0]*v[OOWCOORD];		\
  v[T0COORD]=tscale0*tmu0_data[1]*v[OOWCOORD];		\
} 

#define INCR_TMU0 STRIDE_F(tmu0_data, tmu0_stride)

#define DO_SETUP_TMU1				\
{						\
  v[S1COORD]=sscale1*tmu1_data[0]*v[OOWCOORD];	\
  v[T1COORD]=tscale1*tmu1_data[1]*v[OOWCOORD];	\
}

#define INCR_TMU1 STRIDE_F(tmu1_data, tmu1_stride)

#if FX_USE_PARGB
#define DO_SETUP_RGBA	\
 { GET_PARGB(v) = color[ACOMP] << 24 | color[RCOMP] << 16 | color[GCOMP] << 8 | color[BCOMP];}
  
#else
#define DO_SETUP_RGBA					\
{							\
  UBYTE_COLOR_TO_FLOAT_255_COLOR2(v[RCOORD], color[0]);	\
  UBYTE_COLOR_TO_FLOAT_255_COLOR2(v[GCOORD], color[1]);	\
  UBYTE_COLOR_TO_FLOAT_255_COLOR2(v[BCOORD], color[2]);	\
  UBYTE_COLOR_TO_FLOAT_255_COLOR2(v[ACOORD], color[3]);	\
}
#endif

#define INCR_RGBA color += col_stride


#define _FIXUP_PRE							\
   GLuint hs = fxMesa->stw_hint_state & ~(GR_STWHINT_W_DIFF_TMU0 |	\
					  GR_STWHINT_W_DIFF_TMU1);

#define _FIXUP_TMU0							\
   if (tmu0_sz == 4) {							\
      project_texcoords( VB, 0, tmu0_source, start, end );		\
      hs |= GR_STWHINT_W_DIFF_TMU0;			\
   } 


#define _FIXUP_TMU1							\
   if (tmu1_sz == 4) {							\
      project_texcoords( VB, 1, tmu1_source, start, end );		\
      hs |= GR_STWHINT_W_DIFF_TMU1;			\
   } 


#define _FIXUP_TMU01						\
   if (tmu0_sz == 4) {						\
      project_texcoords( VB, 0, tmu0_source, start, end );	\
      if (tmu1_sz == 4) 					\
	 project_texcoords( VB, 1, tmu1_source, start, end );	\
      else 							\
	 copy_w( VB, 1, start, end );				\
      hs |= (GR_STWHINT_W_DIFF_TMU0|GR_STWHINT_W_DIFF_TMU1);	\
   } else if (tmu1_sz == 4) {					\
      project_texcoords( VB, 1, tmu1_source, start, end );	\
      hs |= GR_STWHINT_W_DIFF_TMU1;				\
   }

#define _FIXUP_POST					\
   if (hs != fxMesa->stw_hint_state) {			\
      fxMesa->stw_hint_state = hs;			\
      FX_grHints(GR_HINT_STWHINT, hs);			\
   }


#define FIXUP_TMU0 { _FIXUP_PRE _FIXUP_TMU0 _FIXUP_POST }
#define FIXUP_TMU1 { _FIXUP_PRE _FIXUP_TMU1 _FIXUP_POST }
#define FIXUP_TMU01 { _FIXUP_PRE _FIXUP_TMU01 _FIXUP_POST }


/* v - pointer to destination GrVertex
 * VB - source of data
 * i - index into vb for data
 */


#endif
