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


/* fxtris.c - 3Dfx VooDoo triangle functions */


#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#if defined(FX)

#include "fxdrv.h"
#include "../mmath.h"



/* Is this enough?  Do we need more triangle funcs?  
 */
static triangle_func tri_tab[0x40];  /* only 0x20 actually used */
static quad_func quad_tab[0x40];     /* only 0x20 actually used */
static line_func line_tab[0x40];     /* less than 0x20 used */
static points_func points_tab[0x40]; /* less than 0x20 used */

#define IND (0)
#define TAG(x) x
#include "fxtritmp.h"

#define IND (FX_OFFSET)
#define TAG(x) x##_offset
#include "fxtritmp.h"

#define IND (FX_TWOSIDE)
#define TAG(x) x##_twoside
#include "fxtritmp.h"

#define IND (FX_TWOSIDE|FX_OFFSET)
#define TAG(x) x##_twoside_offset
#include "fxtritmp.h"

#define IND (FX_FRONT_BACK)
#define TAG(x) x##_front_back
#include "fxtritmp.h"

#define IND (FX_FRONT_BACK|FX_OFFSET)
#define TAG(x) x##_front_back_offset
#include "fxtritmp.h"

#define IND (FX_FRONT_BACK|FX_TWOSIDE)
#define TAG(x) x##_front_back_twoside
#include "fxtritmp.h"

#define IND (FX_FRONT_BACK|FX_TWOSIDE|FX_OFFSET)
#define TAG(x) x##_front_back_twoside_offset
#include "fxtritmp.h"

#define IND (FX_FLAT)
#define TAG(x) x##_flat
#include "fxtritmp.h"

#define IND (FX_FLAT|FX_OFFSET)
#define TAG(x) x##_flat_offset
#include "fxtritmp.h"

#define IND (FX_FLAT|FX_TWOSIDE)
#define TAG(x) x##_flat_twoside
#include "fxtritmp.h"

#define IND (FX_FLAT|FX_TWOSIDE|FX_OFFSET)
#define TAG(x) x##_flat_twoside_offset
#include "fxtritmp.h"

#define IND (FX_FLAT|FX_FRONT_BACK)
#define TAG(x) x##_flat_front_back
#include "fxtritmp.h"

#define IND (FX_FLAT|FX_FRONT_BACK|FX_OFFSET)
#define TAG(x) x##_flat_front_back_offset
#include "fxtritmp.h"

#define IND (FX_FLAT|FX_FRONT_BACK|FX_TWOSIDE)
#define TAG(x) x##_flat_front_back_twoside
#include "fxtritmp.h"

#define IND (FX_FLAT|FX_FRONT_BACK|FX_TWOSIDE|FX_OFFSET)
#define TAG(x) x##_flat_front_back_twoside_offset
#include "fxtritmp.h"

/* We don't actually do antialiasing correctly. Geometry has to be
   sorted for glide's antialiasing to operate */
#if 0
#define IND (FX_ANTIALIAS)
#define TAG(x) x##_aa
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_OFFSET)
#define TAG(x) x##_aa_offset
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_TWOSIDE)
#define TAG(x) x##_aa_twoside
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_TWOSIDE|FX_OFFSET)
#define TAG(x) x##_aa_twoside_offset
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_FRONT_BACK)
#define TAG(x) x##_aa_front_back
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_FRONT_BACK|FX_OFFSET)
#define TAG(x) x##_aa_front_back_offset
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_FRONT_BACK|FX_TWOSIDE)
#define TAG(x) x##_aa_front_back_twoside
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_FRONT_BACK|FX_TWOSIDE|FX_OFFSET)
#define TAG(x) x##_aa_front_back_twoside_offset
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_FLAT)
#define TAG(x) x##_aa_flat
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_FLAT|FX_OFFSET)
#define TAG(x) x##_aa_flat_offset
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_FLAT|FX_TWOSIDE)
#define TAG(x) x##_aa_flat_twoside
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_FLAT|FX_TWOSIDE|FX_OFFSET)
#define TAG(x) x##_aa_flat_twoside_offset
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_FLAT|FX_FRONT_BACK)
#define TAG(x) x##_aa_flat_front_back
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_FLAT|FX_FRONT_BACK|FX_OFFSET)
#define TAG(x) x##_aa_flat_front_back_offset
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_FLAT|FX_FRONT_BACK|FX_TWOSIDE)
#define TAG(x) x##_aa_flat_front_back_twoside
#include "fxtritmp.h"

#define IND (FX_ANTIALIAS|FX_FLAT|FX_FRONT_BACK|FX_TWOSIDE|FX_OFFSET)
#define TAG(x) x##_aa_flat_front_back_twoside_offset
#include "fxtritmp.h"
#endif

void fxDDTrifuncInit() 
{
   init();
   init_offset();
   init_twoside();
   init_twoside_offset();
   init_front_back();
   init_front_back_offset();
   init_front_back_twoside();
   init_front_back_twoside_offset();
   init_flat();
   init_flat_offset();
   init_flat_twoside();
   init_flat_twoside_offset();
   init_flat_front_back();
   init_flat_front_back_offset();
   init_flat_front_back_twoside();
   init_flat_front_back_twoside_offset();
#if 0
   init_aa();
   init_aa_offset();
   init_aa_twoside();
   init_aa_twoside_offset();
   init_aa_front_back();
   init_aa_front_back_offset();
   init_aa_front_back_twoside();
   init_aa_front_back_twoside_offset();
   init_aa_flat();
   init_aa_flat_offset();
   init_aa_flat_twoside();
   init_aa_flat_twoside_offset();
   init_aa_flat_front_back();
   init_aa_flat_front_back_offset();
   init_aa_flat_front_back_twoside();
   init_aa_flat_front_back_twoside_offset();
#endif
}

#ifdef MESA_DEBUG
void fxPrintRenderState( const char *msg, GLuint state )
{
   fprintf(stderr, "%s: (%x) %s%s%s%s%s%s\n",
	   msg, state,
	   (state & FX_ANTIALIAS)  ? "antialias, "  : "",
	   (state & FX_FLAT)       ? "flat, "       : "",
	   (state & FX_TWOSIDE)    ? "twoside, "    : "",
	   (state & FX_OFFSET)     ? "offset, "     : "",
	   (state & FX_FRONT_BACK) ? "front-back, " : "",
	   (state & FX_FALLBACK)    ? "fallback"     : "");
}


void fxPrintHintState( const char *msg, GLuint state )
{
   fprintf(stderr, "%s: (%x) %s %s%s %s%s\n",
	   msg, state,
	   (state & GR_STWHINT_W_DIFF_FBI)    ? "w-fbi, "   : "",
	   (state & GR_STWHINT_W_DIFF_TMU0)   ? "w-tmu0, "  : "",
	   (state & GR_STWHINT_ST_DIFF_TMU0)  ? "st-tmu0, " : "",
	   (state & GR_STWHINT_W_DIFF_TMU1)   ? "w-tmu1, "  : "",
	   (state & GR_STWHINT_ST_DIFF_TMU1)  ? "st-tmu1, " : "");

}
#endif


void fxDDChooseRenderState( GLcontext *ctx )
{
   fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
   GLuint ind = 0;
   GLuint flags = ctx->TriangleCaps;

   ctx->IndirectTriangles &= ~DD_SW_RASTERIZE;

   if (flags) {
      if (fxMesa->render_index & FX_OFFSET) 
	 FX_grDepthBiasLevel(0);

      if (flags & (DD_SELECT|DD_FEEDBACK)) {
	 fxMesa->PointsFunc = 0;
	 fxMesa->LineFunc = 0;
	 fxMesa->TriangleFunc = 0;
	 fxMesa->QuadFunc = 0;
	 fxMesa->render_index = FX_FALLBACK;		
	 ctx->IndirectTriangles |= DD_SW_RASTERIZE;
#if 0
	 fprintf(stderr, "Fallback select|feeback\n");
#endif
	 return;
      }
   
      if (flags & DD_FLATSHADE)                    ind |= FX_FLAT;
      if (flags & DD_TRI_LIGHT_TWOSIDE)            ind |= FX_TWOSIDE;
      if (flags & DD_MULTIDRAW)                    ind |= FX_FRONT_BACK;
      if (flags & (DD_POINT_ATTEN|DD_POINT_SMOOTH))  {
	ind |= FX_FALLBACK;
#if 0
	if (flags&DD_POINT_ATTEN)
	  fprintf(stderr, "Fallback point atten\n");
	if (flags&DD_POINT_SMOOTH)
	  fprintf(stderr, "Fallback point smooth\n");
#endif
      }

      fxMesa->render_index = ind;
      fxMesa->PointsFunc = points_tab[ind];
      if (ind&FX_FALLBACK)
	 ctx->IndirectTriangles |= DD_POINT_SW_RASTERIZE;
      ind &= ~(FX_ANTIALIAS|FX_FALLBACK);

      if (flags & (DD_LINE_STIPPLE|DD_LINE_SMOOTH))  {
	ind |= FX_FALLBACK;
#if 0
	if (flags&DD_LINE_STIPPLE)
	  fprintf(stderr, "Fallback line stipple\n");
        if (flags&DD_LINE_SMOOTH)
	  fprintf(stderr, "Fallback line smooth\n");
#endif
      }

      fxMesa->render_index |= ind;
      fxMesa->LineFunc = line_tab[ind];
      if (ind&FX_FALLBACK)
	 ctx->IndirectTriangles |= DD_LINE_SW_RASTERIZE;
      ind &= ~(FX_ANTIALIAS|FX_FALLBACK);

      if (flags & DD_TRI_OFFSET)                    ind |= FX_OFFSET;
      if (flags & (DD_TRI_UNFILLED|DD_TRI_STIPPLE|DD_TRI_SMOOTH)) {
	ind |= FX_FALLBACK;	
#if 0
	if (flags&DD_TRI_UNFILLED)
	  fprintf(stderr, "Fallback tri unfilled\n");
	if (flags&DD_TRI_STIPPLE)
	  fprintf(stderr, "Fallback tri stippled\n");
	if (flags&DD_TRI_SMOOTH)
	  fprintf(stderr, "Fallback tri smooth\n");
#endif
      }

      fxMesa->render_index |= ind;
      fxMesa->TriangleFunc = tri_tab[ind];
      fxMesa->QuadFunc = quad_tab[ind];

      if (ind&FX_FALLBACK)
	 ctx->IndirectTriangles |= DD_TRI_SW_RASTERIZE | DD_QUAD_SW_RASTERIZE;
   } 
   else if (fxMesa->render_index)
   {
      if (fxMesa->render_index & FX_OFFSET) 
	 FX_grDepthBiasLevel(0);

      fxMesa->render_index = 0;
      fxMesa->PointsFunc   = points_tab[0];
      fxMesa->LineFunc     = line_tab[0];
      fxMesa->TriangleFunc = tri_tab[0];
      fxMesa->QuadFunc     = quad_tab[0];
   }

   if (MESA_VERBOSE&(VERBOSE_STATE|VERBOSE_DRIVER))
      fxPrintRenderState("fxmesa: Render state", fxMesa->render_index);
}

#else


/*
 * Need this to provide at least one external definition.
 */

extern int gl_fx_dummy_function_tris(void);
int gl_fx_dummy_function_tris(void)
{
  return 0;
}

#endif  /* FX */
