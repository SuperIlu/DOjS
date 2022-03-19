/* $Id: triangle.c,v 1.17.4.3 2001/05/10 14:04:06 brianp Exp $ */

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


/*
 * Triangle rasterizers
 * When the device driver doesn't implement triangle rasterization Mesa
 * will use these functions to draw triangles.
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "aatriangle.h"
#include "context.h"
#include "depth.h"
#include "feedback.h"
#include "mem.h"
#include "mmath.h"
#include "span.h"
#include "teximage.h"
#include "texstate.h"
#include "triangle.h"
#include "types.h"
#include "vb.h"
#endif


GLboolean gl_cull_triangle( GLcontext *ctx,
			    GLuint v0, GLuint v1, GLuint v2, GLuint pv )
{
   struct vertex_buffer *VB = ctx->VB;
   GLfloat (*win)[4] = VB->Win.data;
   GLfloat ex = win[v1][0] - win[v0][0];
   GLfloat ey = win[v1][1] - win[v0][1];
   GLfloat fx = win[v2][0] - win[v0][0];
   GLfloat fy = win[v2][1] - win[v0][1];
   GLfloat c = ex*fy-ey*fx;

   if (c * ctx->backface_sign > 0)
      return 0;
   
   return 1;
}






/*
 * Render a flat-shaded color index triangle.
 */
static void flat_ci_triangle( GLcontext *ctx,
                              GLuint v0, GLuint v1, GLuint v2, GLuint pv )
{
#define INTERP_Z 1
#define SETUP_CODE				\
   GLuint index = VB->IndexPtr->data[pv];	\
   if (1) {					\
      /* set the color index */			\
      (*ctx->Driver.Index)( ctx, index );	\
   }

#define INNER_LOOP( LEFT, RIGHT, Y )				\
	{							\
	   GLint i, n = RIGHT-LEFT;				\
	   GLdepth zspan[MAX_WIDTH];				\
	   if (n>0) {						\
	      for (i=0;i<n;i++) {				\
		 zspan[i] = FixedToDepth(ffz);			\
		 ffz += fdzdx;					\
	      }							\
	      gl_write_monoindex_span( ctx, n, LEFT, Y,		\
	                            zspan, index, GL_POLYGON );	\
	   }							\
	}

#include "tritemp.h"	      
}



/*
 * Render a smooth-shaded color index triangle.
 */
static void smooth_ci_triangle( GLcontext *ctx,
                                GLuint v0, GLuint v1, GLuint v2, GLuint pv )
{
   (void) pv;
#define INTERP_Z 1
#define INTERP_INDEX 1

#define INNER_LOOP( LEFT, RIGHT, Y )				\
	{							\
	   GLint i, n = RIGHT-LEFT;				\
	   GLdepth zspan[MAX_WIDTH];				\
           GLuint index[MAX_WIDTH];				\
	   if (n>0) {						\
	      for (i=0;i<n;i++) {				\
		 zspan[i] = FixedToDepth(ffz);			\
                 index[i] = FixedToInt(ffi);			\
		 ffz += fdzdx;					\
		 ffi += fdidx;					\
	      }							\
	      gl_write_index_span( ctx, n, LEFT, Y, zspan,	\
	                           index, GL_POLYGON );		\
	   }							\
	}

#include "tritemp.h"
}



/*
 * Render a flat-shaded RGBA triangle.
 */
static void flat_rgba_triangle( GLcontext *ctx,
                                GLuint v0, GLuint v1, GLuint v2, GLuint pv )
{
#define INTERP_Z 1
#define DEPTH_TYPE DEFAULT_SOFTWARE_DEPTH_TYPE

#define SETUP_CODE				\
   if (1) {					\
      /* set the color */			\
      GLubyte r = VB->ColorPtr->data[pv][0];	\
      GLubyte g = VB->ColorPtr->data[pv][1];	\
      GLubyte b = VB->ColorPtr->data[pv][2];	\
      GLubyte a = VB->ColorPtr->data[pv][3];	\
      (*ctx->Driver.Color)( ctx, r, g, b, a );	\
   }

#define INNER_LOOP( LEFT, RIGHT, Y )				\
	{							\
	   GLint i, n = RIGHT-LEFT;				\
	   GLdepth zspan[MAX_WIDTH];				\
	   if (n>0) {						\
	      for (i=0;i<n;i++) {				\
		 zspan[i] = FixedToDepth(ffz);			\
		 ffz += fdzdx;					\
	      }							\
              gl_write_monocolor_span( ctx, n, LEFT, Y, zspan,	\
                                       VB->ColorPtr->data[pv],	\
			               GL_POLYGON );		\
	   }							\
	}

#include "tritemp.h"

   ASSERT(!ctx->Texture.ReallyEnabled);  /* texturing must be off */
   ASSERT(ctx->Light.ShadeModel==GL_FLAT);
}



/*
 * Render a smooth-shaded RGBA triangle.
 */
static void smooth_rgba_triangle( GLcontext *ctx,
                                  GLuint v0, GLuint v1, GLuint v2, GLuint pv )
{
   (void) pv;
#define INTERP_Z 1
#define DEPTH_TYPE DEFAULT_SOFTWARE_DEPTH_TYPE
#define INTERP_RGB 1
#define INTERP_ALPHA 1

#define INNER_LOOP( LEFT, RIGHT, Y )				\
	{							\
	   GLint i, n = RIGHT-LEFT;				\
	   GLdepth zspan[MAX_WIDTH];				\
	   GLubyte rgba[MAX_WIDTH][4];				\
	   if (n>0) {						\
	      for (i=0;i<n;i++) {				\
		 zspan[i] = FixedToDepth(ffz);			\
		 rgba[i][RCOMP] = FixedToInt(ffr);		\
		 rgba[i][GCOMP] = FixedToInt(ffg);		\
		 rgba[i][BCOMP] = FixedToInt(ffb);		\
		 rgba[i][ACOMP] = FixedToInt(ffa);		\
		 ffz += fdzdx;					\
		 ffr += fdrdx;					\
		 ffg += fdgdx;					\
		 ffb += fdbdx;					\
		 ffa += fdadx;					\
	      }							\
	      gl_write_rgba_span( ctx, n, LEFT, Y,		\
                                  (const GLdepth *) zspan,	\
	                          rgba, GL_POLYGON );		\
	   }							\
	}

#include "tritemp.h"

   ASSERT(!ctx->Texture.ReallyEnabled);  /* texturing must be off */
   ASSERT(ctx->Light.ShadeModel==GL_SMOOTH);
}



/*
 * Render an RGB, GL_DECAL, textured triangle.
 * Interpolate S,T only w/out mipmapping or perspective correction.
 */
static void simple_textured_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                      GLuint v2, GLuint pv )
{
#define INTERP_INT_ST 1
#define S_SCALE twidth
#define T_SCALE theight
#define SETUP_CODE							\
   struct gl_texture_object *obj = ctx->Texture.Unit[0].CurrentD[2];	\
   GLint b = obj->BaseLevel;						\
   GLfloat twidth = (GLfloat) obj->Image[b]->Width;			\
   GLfloat theight = (GLfloat) obj->Image[b]->Height;			\
   GLint twidth_log2 = obj->Image[b]->WidthLog2;			\
   GLubyte *texture = obj->Image[b]->Data;				\
   GLint smask = obj->Image[b]->Width - 1;				\
   GLint tmask = obj->Image[b]->Height - 1;				\
   (void) pv;								\
   if (!texture) {							\
      if (!_mesa_get_teximages_from_driver(ctx, obj))			\
         return;							\
      texture = obj->Image[b]->Data;					\
      ASSERT(texture);							\
   }

#define INNER_LOOP( LEFT, RIGHT, Y )				\
	{							\
	   GLint i, n = RIGHT-LEFT;				\
	   GLubyte rgb[MAX_WIDTH][3];				\
	   if (n>0) {						\
              ffs -= FIXED_HALF; /* off-by-one error? */        \
              fft -= FIXED_HALF;                                \
	      for (i=0;i<n;i++) {				\
                 GLint s = FixedToInt(ffs) & smask;		\
                 GLint t = FixedToInt(fft) & tmask;		\
                 GLint pos = (t << twidth_log2) + s;		\
                 pos = pos + pos + pos;  /* multiply by 3 */	\
                 rgb[i][RCOMP] = texture[pos];			\
                 rgb[i][GCOMP] = texture[pos+1];		\
                 rgb[i][BCOMP] = texture[pos+2];		\
		 ffs += fdsdx;					\
		 fft += fdtdx;					\
	      }							\
              (*ctx->Driver.WriteRGBSpan)( ctx, n, LEFT, Y,	\
                           (CONST GLubyte (*)[3]) rgb, NULL );	\
	   }							\
	}

#include "tritemp.h"
}


/*
 * Render an RGB, GL_DECAL, textured triangle.
 * Interpolate S,T, GL_LESS depth test, w/out mipmapping or
 * perspective correction.
 */
static void simple_z_textured_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                        GLuint v2, GLuint pv )
{
#define INTERP_Z 1
#define DEPTH_TYPE DEFAULT_SOFTWARE_DEPTH_TYPE
#define INTERP_INT_ST 1
#define S_SCALE twidth
#define T_SCALE theight
#define SETUP_CODE							\
   struct gl_texture_object *obj = ctx->Texture.Unit[0].CurrentD[2];	\
   GLint b = obj->BaseLevel;						\
   GLfloat twidth = (GLfloat) obj->Image[b]->Width;			\
   GLfloat theight = (GLfloat) obj->Image[b]->Height;			\
   GLint twidth_log2 = obj->Image[b]->WidthLog2;			\
   GLubyte *texture = obj->Image[b]->Data;				\
   GLint smask = obj->Image[b]->Width - 1;				\
   GLint tmask = obj->Image[b]->Height - 1;				\
   (void) pv;								\
   if (!texture) {							\
      if (!_mesa_get_teximages_from_driver(ctx, obj))			\
         return;							\
      texture = obj->Image[b]->Data;					\
      ASSERT(texture);							\
   }

#define INNER_LOOP( LEFT, RIGHT, Y )				\
	{							\
	   GLint i, n = RIGHT-LEFT;				\
	   GLubyte rgb[MAX_WIDTH][3];				\
           GLubyte mask[MAX_WIDTH];				\
	   if (n>0) {						\
              ffs -= FIXED_HALF; /* off-by-one error? */        \
              fft -= FIXED_HALF;                                \
	      for (i=0;i<n;i++) {				\
                 GLdepth z = FixedToDepth(ffz);			\
                 if (z < zRow[i]) {				\
                    GLint s = FixedToInt(ffs) & smask;		\
                    GLint t = FixedToInt(fft) & tmask;		\
                    GLint pos = (t << twidth_log2) + s;		\
                    pos = pos + pos + pos;  /* multiply by 3 */	\
                    rgb[i][RCOMP] = texture[pos];		\
                    rgb[i][GCOMP] = texture[pos+1];		\
                    rgb[i][BCOMP] = texture[pos+2];		\
		    zRow[i] = z;				\
                    mask[i] = 1;				\
                 }						\
                 else {						\
                    mask[i] = 0;				\
                 }						\
		 ffz += fdzdx;					\
		 ffs += fdsdx;					\
		 fft += fdtdx;					\
	      }							\
              (*ctx->Driver.WriteRGBSpan)( ctx, n, LEFT, Y,	\
                           (CONST GLubyte (*)[3]) rgb, mask );	\
	   }							\
	}

#include "tritemp.h"
}



/*
 * Render an RGB/RGBA textured triangle without perspective correction.
 */
static void affine_textured_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
				      GLuint v2, GLuint pv )
{
#define INTERP_Z 1
#define DEPTH_TYPE DEFAULT_SOFTWARE_DEPTH_TYPE
#define INTERP_RGB 1
#define INTERP_ALPHA 1
#define INTERP_INT_ST 1
#define S_SCALE twidth
#define T_SCALE theight
#define SETUP_CODE							\
   struct gl_texture_unit *unit = ctx->Texture.Unit+0;			\
   struct gl_texture_object *obj = unit->CurrentD[2];			\
   GLint b = obj->BaseLevel;						\
   GLfloat twidth = (GLfloat) obj->Image[b]->Width;			\
   GLfloat theight = (GLfloat) obj->Image[b]->Height;			\
   GLint twidth_log2 = obj->Image[b]->WidthLog2;			\
   GLubyte *texture = obj->Image[b]->Data;				\
   GLint smask = obj->Image[b]->Width - 1;				\
   GLint tmask = obj->Image[b]->Height - 1;                             \
   GLint format = obj->Image[b]->Format;                                \
   GLint filter = obj->MinFilter;                                       \
   GLint envmode = unit->EnvMode;                                       \
   GLint comp, tbytesline, tsize;                                       \
   GLfixed er, eg, eb, ea;                                              \
   GLint tr, tg, tb, ta;                                                \
   if (!texture) {							\
      if (!_mesa_get_teximages_from_driver(ctx, obj))			\
         return;							\
      texture = obj->Image[b]->Data;					\
      ASSERT(texture);							\
   }									\
   if (envmode == GL_BLEND) {                                           \
      /* potential off-by-one error here? (1.0f -> 2048 -> 0) */        \
      er = FloatToFixed(unit->EnvColor[0]);                             \
      eg = FloatToFixed(unit->EnvColor[1]);                             \
      eb = FloatToFixed(unit->EnvColor[2]);                             \
      ea = FloatToFixed(unit->EnvColor[3]);                             \
   }                                                                    \
   switch (format) {                                                    \
   case GL_ALPHA:                                                       \
   case GL_LUMINANCE:                                                   \
   case GL_INTENSITY:                                                   \
      comp = 1;                                                         \
      break;                                                            \
   case GL_LUMINANCE_ALPHA:                                             \
      comp = 2;                                                         \
      break;                                                            \
   case GL_RGB:                                                         \
      comp = 3;                                                         \
      break;                                                            \
   case GL_RGBA:                                                        \
      comp = 4;                                                         \
      break;                                                            \
   default:                                                             \
      gl_problem(NULL, "Bad texture format in affine_texture_triangle");\
      return;                                                           \
   }                                                                    \
   tbytesline = obj->Image[b]->Width * comp;                            \
   tsize = theight * tbytesline;
   (void) pv;

  /* Instead of defining a function for each mode, a test is done 
   * between the outer and inner loops. This is to reduce code size
   * and complexity. Observe that an optimizing compiler kills 
   * unused variables (for instance tf,sf,ti,si in case of GL_NEAREST).
   */ 

#define NEAREST_RGB    \
        tr = tex00[0]; \
        tg = tex00[1]; \
        tb = tex00[2]; \
        ta = 0xff

#define LINEAR_RGB                                                      \
	tr = (ti * (si * tex00[0] + sf * tex01[0]) +                    \
              tf * (si * tex10[0] + sf * tex11[0])) >> 2 * FIXED_SHIFT; \
	tg = (ti * (si * tex00[1] + sf * tex01[1]) +                    \
              tf * (si * tex10[1] + sf * tex11[1])) >> 2 * FIXED_SHIFT; \
	tb = (ti * (si * tex00[2] + sf * tex01[2]) +                    \
              tf * (si * tex10[2] + sf * tex11[2])) >> 2 * FIXED_SHIFT; \
	ta = 0xff

#define NEAREST_RGBA   \
        tr = tex00[0]; \
        tg = tex00[1]; \
        tb = tex00[2]; \
        ta = tex00[3]

#define LINEAR_RGBA                                                     \
	tr = (ti * (si * tex00[0] + sf * tex01[0]) +                    \
              tf * (si * tex10[0] + sf * tex11[0])) >> 2 * FIXED_SHIFT; \
	tg = (ti * (si * tex00[1] + sf * tex01[1]) +                    \
              tf * (si * tex10[1] + sf * tex11[1])) >> 2 * FIXED_SHIFT; \
	tb = (ti * (si * tex00[2] + sf * tex01[2]) +                    \
              tf * (si * tex10[2] + sf * tex11[2])) >> 2 * FIXED_SHIFT; \
	ta = (ti * (si * tex00[3] + sf * tex01[3]) +                    \
              tf * (si * tex10[3] + sf * tex11[3])) >> 2 * FIXED_SHIFT

#define MODULATE                                       \
        dest[0] = ffr * (tr + 1) >> (FIXED_SHIFT + 8); \
        dest[1] = ffg * (tg + 1) >> (FIXED_SHIFT + 8); \
        dest[2] = ffb * (tb + 1) >> (FIXED_SHIFT + 8); \
        dest[3] = ffa * (ta + 1) >> (FIXED_SHIFT + 8)

#define DECAL                                                                \
	dest[0] = ((0xff - ta) * ffr + ((ta + 1) * tr << FIXED_SHIFT)) >> (FIXED_SHIFT + 8); \
	dest[1] = ((0xff - ta) * ffg + ((ta + 1) * tg << FIXED_SHIFT)) >> (FIXED_SHIFT + 8); \
	dest[2] = ((0xff - ta) * ffb + ((ta + 1) * tb << FIXED_SHIFT)) >> (FIXED_SHIFT + 8); \
	dest[3] = FixedToInt(ffa)

#define BLEND                                                               \
        dest[0] = ((0xff - tr) * ffr + (tr + 1) * er) >> (FIXED_SHIFT + 8); \
        dest[1] = ((0xff - tg) * ffg + (tg + 1) * eg) >> (FIXED_SHIFT + 8); \
        dest[2] = ((0xff - tb) * ffb + (tb + 1) * eb) >> (FIXED_SHIFT + 8); \
	dest[3] = ffa * (ta + 1) >> (FIXED_SHIFT + 8)

#define REPLACE       \
        dest[0] = tr; \
        dest[1] = tg; \
        dest[2] = tb; \
        dest[3] = ta

/* shortcuts */

#define NEAREST_RGB_REPLACE  NEAREST_RGB;REPLACE

#define NEAREST_RGBA_REPLACE  *(GLint *)dest = *(GLint *)tex00

#define SPAN1(DO_TEX,COMP)                                 \
	for (i=0;i<n;i++) {                                \
           GLint s = FixedToInt(ffs) & smask;              \
           GLint t = FixedToInt(fft) & tmask;              \
           GLint pos = (t << twidth_log2) + s;             \
           GLubyte *tex00 = texture + COMP * pos;          \
	   zspan[i] = FixedToDepth(ffz);                   \
           DO_TEX;                                         \
	   ffz += fdzdx;                                   \
           ffr += fdrdx;                                   \
	   ffg += fdgdx;                                   \
           ffb += fdbdx;                                   \
	   ffa += fdadx;                                   \
	   ffs += fdsdx;                                   \
	   fft += fdtdx;                                   \
           dest += 4;                                      \
	}

#define SPAN2(DO_TEX,COMP)                                 \
	for (i=0;i<n;i++) {                                \
           GLint s = FixedToInt(ffs) & smask;              \
           GLint t = FixedToInt(fft) & tmask;              \
           GLint sf = ffs & FIXED_FRAC_MASK;               \
           GLint tf = fft & FIXED_FRAC_MASK;               \
           GLint si = FIXED_FRAC_MASK - sf;                \
           GLint ti = FIXED_FRAC_MASK - tf;                \
           GLint pos = (t << twidth_log2) + s;             \
           GLubyte *tex00 = texture + COMP * pos;          \
           GLubyte *tex10 = tex00 + tbytesline;            \
           GLubyte *tex01 = tex00 + COMP;                  \
           GLubyte *tex11 = tex10 + COMP;                  \
           if (t == tmask) {                               \
              tex10 -= tsize;                              \
              tex11 -= tsize;                              \
           }                                               \
           if (s == smask) {                               \
              tex01 -= tbytesline;                         \
              tex11 -= tbytesline;                         \
           }                                               \
	   zspan[i] = FixedToDepth(ffz);                   \
           DO_TEX;                                         \
	   ffz += fdzdx;                                   \
           ffr += fdrdx;                                   \
	   ffg += fdgdx;                                   \
           ffb += fdbdx;                                   \
	   ffa += fdadx;                                   \
	   ffs += fdsdx;                                   \
	   fft += fdtdx;                                   \
           dest += 4;                                      \
	}

/* here comes the heavy part.. (something for the compiler to chew on) */
#define INNER_LOOP( LEFT, RIGHT, Y )	                   \
	{				                   \
	   GLint i, n = RIGHT-LEFT;	                   \
	   GLdepth zspan[MAX_WIDTH];	                   \
	   GLubyte rgba[MAX_WIDTH][4];                     \
	   if (n>0) {                                      \
              GLubyte *dest = rgba[0];                     \
              ffs -= FIXED_HALF; /* off-by-one error? */   \
              fft -= FIXED_HALF;                           \
              switch (filter) {                            \
   	      case GL_NEAREST:                             \
		 switch (format) {                         \
                 case GL_RGB:                              \
	            switch (envmode) {                     \
	            case GL_MODULATE:                      \
                       SPAN1(NEAREST_RGB;MODULATE,3);      \
                       break;                              \
	            case GL_DECAL:                         \
                    case GL_REPLACE:                       \
                       SPAN1(NEAREST_RGB_REPLACE,3);       \
                       break;                              \
                    case GL_BLEND:                         \
                       SPAN1(NEAREST_RGB;BLEND,3);         \
                       break;                              \
                    default: /* unexpected env mode */     \
                       abort();                            \
	            }                                      \
                    break;                                 \
		 case GL_RGBA:                             \
		    switch(envmode) {                      \
		    case GL_MODULATE:                      \
                       SPAN1(NEAREST_RGBA;MODULATE,4);     \
                       break;                              \
		    case GL_DECAL:                         \
                       SPAN1(NEAREST_RGBA;DECAL,4);        \
                       break;                              \
		    case GL_BLEND:                         \
                       SPAN1(NEAREST_RGBA;BLEND,4);        \
                       break;                              \
		    case GL_REPLACE:                       \
                       SPAN1(NEAREST_RGBA_REPLACE,4);      \
                       break;                              \
                    default: /* unexpected env mode */     \
                       abort();                            \
		    }                                      \
                    break;                                 \
	         }                                         \
                 break;                                    \
	      case GL_LINEAR:                              \
                 ffs -= FIXED_HALF;                        \
                 fft -= FIXED_HALF;                        \
		 switch (format) {                         \
		 case GL_RGB:                              \
		    switch (envmode) {                     \
		    case GL_MODULATE:                      \
		       SPAN2(LINEAR_RGB;MODULATE,3);       \
                       break;                              \
		    case GL_DECAL:                         \
		    case GL_REPLACE:                       \
                       SPAN2(LINEAR_RGB;REPLACE,3);        \
                       break;                              \
		    case GL_BLEND:                         \
		       SPAN2(LINEAR_RGB;BLEND,3);          \
		       break;                              \
                    default: /* unexpected env mode */     \
                       abort();                            \
		    }                                      \
		    break;                                 \
		 case GL_RGBA:                             \
		    switch (envmode) {                     \
		    case GL_MODULATE:                      \
		       SPAN2(LINEAR_RGBA;MODULATE,4);      \
		       break;                              \
		    case GL_DECAL:                         \
		       SPAN2(LINEAR_RGBA;DECAL,4);         \
		       break;                              \
		    case GL_BLEND:                         \
		       SPAN2(LINEAR_RGBA;BLEND,4);         \
		       break;                              \
		    case GL_REPLACE:                       \
		       SPAN2(LINEAR_RGBA;REPLACE,4);       \
		       break;                              \
                    default: /* unexpected env mode */     \
                       abort();                            \
		    }                                      \
		    break;                                 \
	         }                                         \
                 break;                                    \
	      }                                            \
              gl_write_rgba_span(ctx, n, LEFT, Y, zspan,   \
                                 rgba, GL_POLYGON);        \
              /* explicit kill of variables: */            \
              ffr = ffg = ffb = ffa = 0;                   \
           }                                               \
	}

#include "tritemp.h"
#undef SPAN1
#undef SPAN2
}


/*
 * Render an perspective corrected RGB/RGBA textured triangle.
 * The Q (aka V in Mesa) coordinate must be zero such that the divide
 * by interpolated Q/W comes out right.
 */
static void persp_textured_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
				     GLuint v2, GLuint pv )
{
/* The BIAS value is used to shift negative values into positive values.
 * Without this, negative texture values don't GL_REPEAT correctly at just
 * below zero.  We're not going to worry about texture coords less than -BIAS.
 * Only seems to be a problem with GL_NEAREST filtering.
 */
#define BIAS 4096.0F

#define INTERP_Z 1
#define DEPTH_TYPE DEFAULT_SOFTWARE_DEPTH_TYPE
#define INTERP_RGB 1
#define INTERP_ALPHA 1
#define INTERP_STUV 1
#define SETUP_CODE							\
   struct gl_texture_unit *unit = ctx->Texture.Unit+0;			\
   struct gl_texture_object *obj = unit->CurrentD[2];			\
   GLint b = obj->BaseLevel;						\
   GLfloat twidth = (GLfloat) obj->Image[b]->Width;			\
   GLfloat theight = (GLfloat) obj->Image[b]->Height;			\
   GLint twidth_log2 = obj->Image[b]->WidthLog2;			\
   GLubyte *texture = obj->Image[b]->Data;				\
   GLint smask = (obj->Image[b]->Width - 1);                            \
   GLint tmask = (obj->Image[b]->Height - 1);                           \
   GLint format = obj->Image[b]->Format;                                \
   GLint filter = obj->MinFilter;                                       \
   GLint envmode = unit->EnvMode;                                       \
   GLfloat sscale, tscale;                                              \
   GLint comp, tbytesline, tsize;                                       \
   GLfixed er, eg, eb, ea;                                              \
   GLint tr, tg, tb, ta;                                                \
   if (!texture) {							\
      if (!_mesa_get_teximages_from_driver(ctx, obj))			\
         return;							\
      texture = obj->Image[b]->Data;					\
      ASSERT(texture);							\
   }									\
   if (envmode == GL_BLEND) {                                           \
      er = FloatToFixed(unit->EnvColor[0]);                             \
      eg = FloatToFixed(unit->EnvColor[1]);                             \
      eb = FloatToFixed(unit->EnvColor[2]);                             \
      ea = FloatToFixed(unit->EnvColor[3]);                             \
   }                                                                    \
   switch (format) {                                                    \
   case GL_ALPHA:                                                       \
   case GL_LUMINANCE:                                                   \
   case GL_INTENSITY:                                                   \
      comp = 1;                                                         \
      break;                                                            \
   case GL_LUMINANCE_ALPHA:                                             \
      comp = 2;                                                         \
      break;                                                            \
   case GL_RGB:                                                         \
      comp = 3;                                                         \
      break;                                                            \
   case GL_RGBA:                                                        \
      comp = 4;                                                         \
      break;                                                            \
   default:                                                             \
      gl_problem(NULL, "Bad texture format in persp_texture_triangle"); \
      return;                                                           \
   }                                                                    \
   if (filter == GL_NEAREST) {                                          \
      sscale = twidth;                                                  \
      tscale = theight;                                                 \
   }                                                                    \
   else {                                                               \
      sscale = FIXED_SCALE * twidth;                                    \
      tscale = FIXED_SCALE * theight;                                   \
   }                                                                    \
   tbytesline = obj->Image[b]->Width * comp;                            \
   tsize = theight * tbytesline;
   (void) pv;

#define SPAN1(DO_TEX,COMP)                                 \
        for (i=0;i<n;i++) {                                \
           GLfloat invQ = 1.0f / vv;                       \
           GLint s = (int)(SS * invQ + BIAS) & smask;      \
           GLint t = (int)(TT * invQ + BIAS) & tmask;      \
           GLint pos = COMP * ((t << twidth_log2) + s);    \
           GLubyte *tex00 = texture + pos;                 \
	   zspan[i] = FixedToDepth(ffz);                   \
           DO_TEX;                                         \
	   ffz += fdzdx;                                   \
           ffr += fdrdx;                                   \
	   ffg += fdgdx;                                   \
           ffb += fdbdx;                                   \
	   ffa += fdadx;                                   \
           SS += dSdx;                                     \
           TT += dTdx;                                     \
	   vv += dvdx;                                     \
           dest += 4;                                      \
	}

#define SPAN2(DO_TEX,COMP)                                 \
        for (i=0;i<n;i++) {                                \
           GLfloat invQ = 1.0f / vv;                       \
           GLfixed ffs = (int)(SS * invQ);                 \
           GLfixed fft = (int)(TT * invQ);                 \
	   GLint s = FixedToInt(ffs) & smask;              \
	   GLint t = FixedToInt(fft) & tmask;              \
           GLint sf = ffs & FIXED_FRAC_MASK;               \
           GLint tf = fft & FIXED_FRAC_MASK;               \
           GLint si = FIXED_FRAC_MASK - sf;                \
           GLint ti = FIXED_FRAC_MASK - tf;                \
           GLint pos = COMP * ((t << twidth_log2) + s);    \
           GLubyte *tex00 = texture + pos;                 \
           GLubyte *tex10 = tex00 + tbytesline;            \
           GLubyte *tex01 = tex00 + COMP;                  \
           GLubyte *tex11 = tex10 + COMP;                  \
           if (t == tmask) {                               \
              tex10 -= tsize;                              \
              tex11 -= tsize;                              \
           }                                               \
           if (s == smask) {                               \
              tex01 -= tbytesline;                         \
              tex11 -= tbytesline;                         \
           }                                               \
	   zspan[i] = FixedToDepth(ffz);                   \
           DO_TEX;                                         \
	   ffz += fdzdx;                                   \
           ffr += fdrdx;                                   \
	   ffg += fdgdx;                                   \
           ffb += fdbdx;                                   \
	   ffa += fdadx;                                   \
           SS += dSdx;                                     \
           TT += dTdx;                                     \
	   vv += dvdx;                                     \
           dest += 4;                                      \
	}

#define INNER_LOOP( LEFT, RIGHT, Y )	                   \
	{				                   \
	   GLint i, n = RIGHT-LEFT;	                   \
	   GLdepth zspan[MAX_WIDTH];	                   \
	   GLubyte rgba[MAX_WIDTH][4];	                   \
           (void)uu; /* please GCC */                      \
	   if (n>0) {                                      \
              GLfloat SS = ss * sscale;                    \
              GLfloat TT = tt * tscale;                    \
              GLfloat dSdx = dsdx * sscale;                \
              GLfloat dTdx = dtdx * tscale;                \
              GLubyte *dest = rgba[0];                     \
              switch (filter) {                            \
   	      case GL_NEAREST:                             \
		 switch (format) {                         \
                 case GL_RGB:                              \
	            switch (envmode) {                     \
	            case GL_MODULATE:                      \
                       SPAN1(NEAREST_RGB;MODULATE,3);      \
                       break;                              \
	            case GL_DECAL:                         \
                    case GL_REPLACE:                       \
                       SPAN1(NEAREST_RGB_REPLACE,3);       \
                       break;                              \
                    case GL_BLEND:                         \
                       SPAN1(NEAREST_RGB;BLEND,3);         \
                       break;                              \
                    default: /* unexpected env mode */     \
                       abort();                            \
	            }                                      \
                    break;                                 \
		 case GL_RGBA:                             \
		    switch(envmode) {                      \
		    case GL_MODULATE:                      \
                       SPAN1(NEAREST_RGBA;MODULATE,4);     \
                       break;                              \
		    case GL_DECAL:                         \
                       SPAN1(NEAREST_RGBA;DECAL,4);        \
                       break;                              \
		    case GL_BLEND:                         \
                       SPAN1(NEAREST_RGBA;BLEND,4);        \
                       break;                              \
		    case GL_REPLACE:                       \
                       SPAN1(NEAREST_RGBA_REPLACE,4);      \
                       break;                              \
                    default: /* unexpected env mode */     \
                       abort();                            \
		    }                                      \
                    break;                                 \
	         }                                         \
                 break;                                    \
	      case GL_LINEAR:                              \
	         SS -= 0.5f * FIXED_SCALE * vv;            \
		 TT -= 0.5f * FIXED_SCALE * vv;            \
		 switch (format) {                         \
		 case GL_RGB:                              \
		    switch (envmode) {                     \
		    case GL_MODULATE:                      \
		       SPAN2(LINEAR_RGB;MODULATE,3);       \
                       break;                              \
		    case GL_DECAL:                         \
		    case GL_REPLACE:                       \
                       SPAN2(LINEAR_RGB;REPLACE,3);        \
                       break;                              \
		    case GL_BLEND:                         \
		       SPAN2(LINEAR_RGB;BLEND,3);          \
		       break;                              \
                    default: /* unexpected env mode */     \
                       abort();                            \
		    }                                      \
		    break;                                 \
		 case GL_RGBA:                             \
		    switch (envmode) {                     \
		    case GL_MODULATE:                      \
		       SPAN2(LINEAR_RGBA;MODULATE,4);      \
		       break;                              \
		    case GL_DECAL:                         \
		       SPAN2(LINEAR_RGBA;DECAL,4);         \
		       break;                              \
		    case GL_BLEND:                         \
		       SPAN2(LINEAR_RGBA;BLEND,4);         \
		       break;                              \
		    case GL_REPLACE:                       \
		       SPAN2(LINEAR_RGBA;REPLACE,4);       \
		       break;                              \
                    default: /* unexpected env mode */     \
                       abort();                            \
		    }                                      \
		    break;                                 \
	         }                                         \
                 break;                                    \
	      }                                            \
	      gl_write_rgba_span( ctx, n, LEFT, Y, zspan,  \
				  rgba, GL_POLYGON );      \
              ffr = ffg = ffb = ffa = 0;                   \
	   }                                               \
	}


#include "tritemp.h"
#undef SPAN1
#undef SPAN2
#undef BIAS
}



/*
 * Render a smooth-shaded, textured, RGBA triangle.
 * Interpolate S,T,U with perspective correction, w/out mipmapping.
 * Note: we use texture coordinates S,T,U,V instead of S,T,R,Q because
 * R is already used for red.
 */
static void general_textured_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                       GLuint v2, GLuint pv )
{
#define INTERP_Z 1
#define DEPTH_TYPE DEFAULT_SOFTWARE_DEPTH_TYPE
#define INTERP_RGB 1
#define INTERP_ALPHA 1
#define INTERP_STUV 1
#define SETUP_CODE						\
   GLboolean flat_shade = (ctx->Light.ShadeModel==GL_FLAT);	\
   GLint r, g, b, a;						\
   if (flat_shade) {						\
      r = VB->ColorPtr->data[pv][0];				\
      g = VB->ColorPtr->data[pv][1];				\
      b = VB->ColorPtr->data[pv][2];				\
      a = VB->ColorPtr->data[pv][3];				\
   }
#define INNER_LOOP( LEFT, RIGHT, Y )				\
	{							\
	   GLint i, n = RIGHT-LEFT;				\
	   GLdepth zspan[MAX_WIDTH];				\
	   GLubyte rgba[MAX_WIDTH][4];				\
           GLfloat s[MAX_WIDTH], t[MAX_WIDTH], u[MAX_WIDTH];	\
	   if (n>0) {						\
              if (flat_shade) {					\
                 for (i=0;i<n;i++) {				\
		    GLdouble invQ = 1.0 / vv;			\
		    zspan[i] = FixedToDepth(ffz);		\
		    rgba[i][RCOMP] = r;				\
		    rgba[i][GCOMP] = g;				\
		    rgba[i][BCOMP] = b;				\
		    rgba[i][ACOMP] = a;				\
		    s[i] = ss*invQ;				\
		    t[i] = tt*invQ;				\
		    u[i] = uu*invQ;				\
		    ffz += fdzdx;				\
		    ss += dsdx;					\
		    tt += dtdx;					\
		    uu += dudx;					\
		    vv += dvdx;					\
		 }						\
              }							\
              else {						\
                 for (i=0;i<n;i++) {				\
		    GLdouble invQ = 1.0 / vv;			\
		    zspan[i] = FixedToDepth(ffz);		\
		    rgba[i][RCOMP] = FixedToInt(ffr);		\
		    rgba[i][GCOMP] = FixedToInt(ffg);		\
		    rgba[i][BCOMP] = FixedToInt(ffb);		\
		    rgba[i][ACOMP] = FixedToInt(ffa);		\
		    s[i] = ss*invQ;				\
		    t[i] = tt*invQ;				\
		    u[i] = uu*invQ;				\
		    ffz += fdzdx;				\
		    ffr += fdrdx;				\
		    ffg += fdgdx;				\
		    ffb += fdbdx;				\
		    ffa += fdadx;				\
		    ss += dsdx;					\
		    tt += dtdx;					\
		    uu += dudx;					\
		    vv += dvdx;					\
		 }						\
              }							\
	      gl_write_texture_span( ctx, n, LEFT, Y, zspan,	\
                                     s, t, u, NULL, 		\
	                             rgba, \
                                     NULL, GL_POLYGON );	\
	   }							\
	}

#include "tritemp.h"
}


/*
 * Render a smooth-shaded, textured, RGBA triangle with separate specular
 * color interpolation.
 * Interpolate S,T,U with perspective correction, w/out mipmapping.
 * Note: we use texture coordinates S,T,U,V instead of S,T,R,Q because
 * R is already used for red.
 */
static void general_textured_spec_triangle1( GLcontext *ctx, GLuint v0,
                                             GLuint v1, GLuint v2, GLuint pv,
                                             GLdepth zspan[MAX_WIDTH],
                                             GLubyte rgba[MAX_WIDTH][4],
                                             GLubyte spec[MAX_WIDTH][4] )
{
#define INTERP_Z 1
#define DEPTH_TYPE DEFAULT_SOFTWARE_DEPTH_TYPE
#define INTERP_RGB 1
#define INTERP_SPEC 1
#define INTERP_ALPHA 1
#define INTERP_STUV 1
#define SETUP_CODE						\
   GLboolean flat_shade = (ctx->Light.ShadeModel==GL_FLAT);	\
   GLint r, g, b, a, sr, sg, sb;				\
   if (flat_shade) {						\
      r = VB->ColorPtr->data[pv][0];				\
      g = VB->ColorPtr->data[pv][1];				\
      b = VB->ColorPtr->data[pv][2];				\
      a = VB->ColorPtr->data[pv][3];				\
      sr = VB->Specular[pv][0]; 				\
      sg = VB->Specular[pv][1]; 				\
      sb = VB->Specular[pv][2]; 				\
   }
#define INNER_LOOP( LEFT, RIGHT, Y )				\
	{							\
	   GLint i, n = RIGHT-LEFT;				\
           GLfloat s[MAX_WIDTH], t[MAX_WIDTH], u[MAX_WIDTH];	\
	   if (n>0) {						\
              if (flat_shade) {					\
                 for (i=0;i<n;i++) {				\
		    GLdouble invQ = 1.0 / vv;			\
		    zspan[i] = FixedToDepth(ffz);		\
		    rgba[i][RCOMP] = r;				\
		    rgba[i][GCOMP] = g;				\
		    rgba[i][BCOMP] = b;				\
		    rgba[i][ACOMP] = a;				\
		    spec[i][RCOMP] = sr;			\
		    spec[i][GCOMP] = sg;			\
		    spec[i][BCOMP] = sb;			\
		    s[i] = ss*invQ;				\
		    t[i] = tt*invQ;				\
		    u[i] = uu*invQ;				\
		    ffz += fdzdx;				\
		    ss += dsdx;					\
		    tt += dtdx;					\
		    uu += dudx;					\
		    vv += dvdx;					\
		 }						\
              }							\
              else {						\
                 for (i=0;i<n;i++) {				\
		    GLdouble invQ = 1.0 / vv;			\
		    zspan[i] = FixedToDepth(ffz);		\
		    rgba[i][RCOMP] = FixedToInt(ffr);		\
		    rgba[i][GCOMP] = FixedToInt(ffg);		\
		    rgba[i][BCOMP] = FixedToInt(ffb);		\
		    rgba[i][ACOMP] = FixedToInt(ffa);		\
		    spec[i][RCOMP] = FixedToInt(ffsr);		\
		    spec[i][GCOMP] = FixedToInt(ffsg);		\
		    spec[i][BCOMP] = FixedToInt(ffsb);		\
		    s[i] = ss*invQ;				\
		    t[i] = tt*invQ;				\
		    u[i] = uu*invQ;				\
		    ffz += fdzdx;				\
		    ffr += fdrdx;				\
		    ffg += fdgdx;				\
		    ffb += fdbdx;				\
		    ffa += fdadx;				\
		    ffsr += fdsrdx;				\
		    ffsg += fdsgdx;				\
		    ffsb += fdsbdx;				\
		    ss += dsdx;					\
		    tt += dtdx;					\
		    uu += dudx;					\
		    vv += dvdx;					\
		 }						\
              }							\
	      gl_write_texture_span( ctx, n, LEFT, Y, zspan,	\
                                   s, t, u, NULL, rgba,		\
                                   (CONST GLubyte (*)[4]) spec,	\
	                           GL_POLYGON );		\
	   }							\
	}

#include "tritemp.h"
}



/*
 * Compute the lambda value for a fragment. (texture level of detail)
 */
static INLINE GLfloat
compute_lambda( GLfloat dsdx, GLfloat dsdy, GLfloat dtdx, GLfloat dtdy,
                GLfloat invQ, GLfloat width, GLfloat height ) 
{
   GLfloat dudx = dsdx * invQ * width;
   GLfloat dudy = dsdy * invQ * width;
   GLfloat dvdx = dtdx * invQ * height;
   GLfloat dvdy = dtdy * invQ * height;
   GLfloat r1 = dudx * dudx + dudy * dudy;
   GLfloat r2 = dvdx * dvdx + dvdy * dvdy;
   GLfloat rho2 = r1 + r2;     /* used to be:  rho2 = MAX2(r1,r2); */
   /* return log base 2 of rho */
   return log(rho2) * 1.442695 * 0.5;       /* 1.442695 = 1/log(2) */
}


/*
 * Render a smooth-shaded, textured, RGBA triangle.
 * Interpolate S,T,U with perspective correction and compute lambda for
 * each fragment.  Lambda is used to determine whether to use the
 * minification or magnification filter.  If minification and using
 * mipmaps, lambda is also used to select the texture level of detail.
 */
static void lambda_textured_triangle1( GLcontext *ctx, GLuint v0, GLuint v1,
                                       GLuint v2, GLuint pv,
                                       GLfloat s[MAX_WIDTH],
                                       GLfloat t[MAX_WIDTH],
                                       GLfloat u[MAX_WIDTH] )
{
#define INTERP_Z 1
#define DEPTH_TYPE DEFAULT_SOFTWARE_DEPTH_TYPE
#define INTERP_RGB 1
#define INTERP_ALPHA 1
#define INTERP_STUV 1

#define SETUP_CODE							\
   const struct gl_texture_object *obj = ctx->Texture.Unit[0].Current;	\
   const GLint baseLevel = obj->BaseLevel;				\
   const struct gl_texture_image *texImage = obj->Image[baseLevel];	\
   const GLfloat twidth = (GLfloat) texImage->Width;			\
   const GLfloat theight = (GLfloat) texImage->Height;			\
   const GLboolean flat_shade = (ctx->Light.ShadeModel==GL_FLAT);	\
   GLint r, g, b, a;							\
   if (flat_shade) {							\
      r = VB->ColorPtr->data[pv][0];					\
      g = VB->ColorPtr->data[pv][1];					\
      b = VB->ColorPtr->data[pv][2];					\
      a = VB->ColorPtr->data[pv][3];					\
   }

#define INNER_LOOP( LEFT, RIGHT, Y )					\
	{								\
	   GLint i, n = RIGHT-LEFT;					\
	   GLdepth zspan[MAX_WIDTH];					\
	   GLubyte rgba[MAX_WIDTH][4];					\
	   GLfloat lambda[MAX_WIDTH];					\
	   if (n>0) {							\
	      if (flat_shade) {						\
		 for (i=0;i<n;i++) {					\
		    GLdouble invQ = 1.0 / vv;				\
		    zspan[i] = FixedToDepth(ffz);			\
		    rgba[i][RCOMP] = r;					\
		    rgba[i][GCOMP] = g;					\
		    rgba[i][BCOMP] = b;					\
		    rgba[i][ACOMP] = a;					\
		    s[i] = ss*invQ;					\
		    t[i] = tt*invQ;					\
		    u[i] = uu*invQ;					\
		    lambda[i] = compute_lambda( dsdx, dsdy, dtdx, dtdy,	\
						invQ, twidth, theight );\
		    ffz += fdzdx;					\
		    ss += dsdx;						\
		    tt += dtdx;						\
		    uu += dudx;						\
		    vv += dvdx;						\
		 }							\
              }								\
              else {							\
		 for (i=0;i<n;i++) {					\
		    GLdouble invQ = 1.0 / vv;				\
		    zspan[i] = FixedToDepth(ffz);			\
		    rgba[i][RCOMP] = FixedToInt(ffr);			\
		    rgba[i][GCOMP] = FixedToInt(ffg);			\
		    rgba[i][BCOMP] = FixedToInt(ffb);			\
		    rgba[i][ACOMP] = FixedToInt(ffa);			\
		    s[i] = ss*invQ;					\
		    t[i] = tt*invQ;					\
		    u[i] = uu*invQ;					\
		    lambda[i] = compute_lambda( dsdx, dsdy, dtdx, dtdy,	\
						invQ, twidth, theight );\
		    ffz += fdzdx;					\
		    ffr += fdrdx;					\
		    ffg += fdgdx;					\
		    ffb += fdbdx;					\
		    ffa += fdadx;					\
		    ss += dsdx;						\
		    tt += dtdx;						\
		    uu += dudx;						\
		    vv += dvdx;						\
		 }							\
              }								\
	      gl_write_texture_span( ctx, n, LEFT, Y, zspan,		\
                                     s, t, u, lambda,	 		\
	                             rgba, NULL, GL_POLYGON );		\
	   }								\
	}

#include "tritemp.h"
}


/*
 * Render a smooth-shaded, textured, RGBA triangle with separate specular
 * interpolation.
 * Interpolate S,T,U with perspective correction and compute lambda for
 * each fragment.  Lambda is used to determine whether to use the
 * minification or magnification filter.  If minification and using
 * mipmaps, lambda is also used to select the texture level of detail.
 */
static void lambda_textured_spec_triangle1( GLcontext *ctx, GLuint v0,
                                            GLuint v1, GLuint v2, GLuint pv,
                                            GLfloat s[MAX_WIDTH],
                                            GLfloat t[MAX_WIDTH],
                                            GLfloat u[MAX_WIDTH] )
{
#define INTERP_Z 1
#define DEPTH_TYPE DEFAULT_SOFTWARE_DEPTH_TYPE
#define INTERP_RGB 1
#define INTERP_SPEC 1
#define INTERP_ALPHA 1
#define INTERP_STUV 1

#define SETUP_CODE							\
   const struct gl_texture_object *obj = ctx->Texture.Unit[0].Current;	\
   const GLint baseLevel = obj->BaseLevel;				\
   const struct gl_texture_image *texImage = obj->Image[baseLevel];	\
   const GLfloat twidth = (GLfloat) texImage->Width;			\
   const GLfloat theight = (GLfloat) texImage->Height;			\
   const GLboolean flat_shade = (ctx->Light.ShadeModel==GL_FLAT);	\
   GLint r, g, b, a, sr, sg, sb;					\
   if (flat_shade) {							\
      r = VB->ColorPtr->data[pv][0];					\
      g = VB->ColorPtr->data[pv][1];					\
      b = VB->ColorPtr->data[pv][2];					\
      a = VB->ColorPtr->data[pv][3];					\
      sr = VB->Specular[pv][0];						\
      sg = VB->Specular[pv][1];						\
      sb = VB->Specular[pv][2];						\
   }

#define INNER_LOOP( LEFT, RIGHT, Y )					\
	{								\
	   GLint i, n = RIGHT-LEFT;					\
	   GLdepth zspan[MAX_WIDTH];					\
	   GLubyte spec[MAX_WIDTH][4];					\
           GLubyte rgba[MAX_WIDTH][4];					\
	   GLfloat lambda[MAX_WIDTH];					\
	   if (n>0) {							\
	      if (flat_shade) {						\
		 for (i=0;i<n;i++) {					\
		    GLdouble invQ = 1.0 / vv;				\
		    zspan[i] = FixedToDepth(ffz);			\
		    rgba[i][RCOMP] = r;					\
		    rgba[i][GCOMP] = g;					\
		    rgba[i][BCOMP] = b;					\
		    rgba[i][ACOMP] = a;					\
		    spec[i][RCOMP] = sr;				\
		    spec[i][GCOMP] = sg;				\
		    spec[i][BCOMP] = sb;				\
		    s[i] = ss*invQ;					\
		    t[i] = tt*invQ;					\
		    u[i] = uu*invQ;					\
		    lambda[i] = compute_lambda( dsdx, dsdy, dtdx, dtdy,	\
						invQ, twidth, theight );\
		    ffz += fdzdx;					\
		    ss += dsdx;						\
		    tt += dtdx;						\
		    uu += dudx;						\
		    vv += dvdx;						\
		 }							\
              }								\
              else {							\
		 for (i=0;i<n;i++) {					\
		    GLdouble invQ = 1.0 / vv;				\
		    zspan[i] = FixedToDepth(ffz);			\
		    rgba[i][RCOMP] = FixedToInt(ffr);			\
		    rgba[i][GCOMP] = FixedToInt(ffg);			\
		    rgba[i][BCOMP] = FixedToInt(ffb);			\
		    rgba[i][ACOMP] = FixedToInt(ffa);			\
		    spec[i][RCOMP] = FixedToInt(ffsr);			\
		    spec[i][GCOMP] = FixedToInt(ffsg);			\
		    spec[i][BCOMP] = FixedToInt(ffsb);			\
		    s[i] = ss*invQ;					\
		    t[i] = tt*invQ;					\
		    u[i] = uu*invQ;					\
		    lambda[i] = compute_lambda( dsdx, dsdy, dtdx, dtdy,	\
						invQ, twidth, theight );\
		    ffz += fdzdx;					\
		    ffr += fdrdx;					\
		    ffg += fdgdx;					\
		    ffb += fdbdx;					\
		    ffa += fdadx;					\
		    ffsr += fdsrdx;					\
		    ffsg += fdsgdx;					\
		    ffsb += fdsbdx;					\
		    ss += dsdx;						\
		    tt += dtdx;						\
		    uu += dudx;						\
		    vv += dvdx;						\
		 }							\
              }								\
	      gl_write_texture_span( ctx, n, LEFT, Y, zspan,		\
                                     s, t, u, lambda,	 		\
	                             rgba, (CONST GLubyte (*)[4]) spec, \
                                     GL_POLYGON );			\
	   }								\
	}

#include "tritemp.h"
}


/*
 * This is the big one!
 * Interpolate Z, RGB, Alpha, and two sets of texture coordinates.
 * Yup, it's slow.
 */
static void lambda_multitextured_triangle1( GLcontext *ctx, GLuint v0,
                                      GLuint v1, GLuint v2, GLuint pv,
                                      GLfloat s[MAX_TEXTURE_UNITS][MAX_WIDTH],
                                      GLfloat t[MAX_TEXTURE_UNITS][MAX_WIDTH],
                                      GLfloat u[MAX_TEXTURE_UNITS][MAX_WIDTH]
                                      )
{
   GLubyte rgba[MAX_WIDTH][4];
#define INTERP_Z 1
#define DEPTH_TYPE DEFAULT_SOFTWARE_DEPTH_TYPE
#define INTERP_RGB 1
#define INTERP_ALPHA 1
#define INTERP_STUV 1
#define INTERP_STUV1 1

#define SETUP_CODE							\
   const struct gl_texture_object *obj0 = ctx->Texture.Unit[0].Current;	\
   const struct gl_texture_object *obj1 = ctx->Texture.Unit[1].Current;	\
   const struct gl_texture_image *texImage0, *texImage1;		\
   GLfloat twidth0, theight0, twidth1, theight1;			\
   const GLboolean flat_shade = (ctx->Light.ShadeModel==GL_FLAT);	\
   GLint r, g, b, a;							\
   if (obj0) {								\
      texImage0 = obj0->Image[obj0->BaseLevel];				\
      twidth0 = texImage0->Width;					\
      theight0 = texImage0->Height;					\
   }									\
   else {								\
      twidth0 = theight0 = 1;						\
   }									\
   if (obj1) {								\
      texImage1 = obj1->Image[obj1->BaseLevel];				\
      twidth1 = texImage1->Width;					\
      theight1 = texImage1->Height;					\
   }									\
   else {								\
      twidth1 = theight1 = 1;						\
   }									\
   if (flat_shade) {							\
      r = VB->ColorPtr->data[pv][0];					\
      g = VB->ColorPtr->data[pv][1];					\
      b = VB->ColorPtr->data[pv][2];					\
      a = VB->ColorPtr->data[pv][3];					\
   }

#define INNER_LOOP( LEFT, RIGHT, Y )					\
	{								\
	   GLint i, n = RIGHT-LEFT;					\
	   GLdepth zspan[MAX_WIDTH];					\
           GLfloat lambda[MAX_TEXTURE_UNITS][MAX_WIDTH];		\
	   if (n>0) {							\
	      if (flat_shade) {						\
		 for (i=0;i<n;i++) {					\
		    GLdouble invQ = 1.0 / vv;				\
		    GLdouble invQ1 = 1.0 / vv1;				\
		    zspan[i] = FixedToDepth(ffz);			\
		    rgba[i][RCOMP] = r;					\
		    rgba[i][GCOMP] = g;					\
		    rgba[i][BCOMP] = b;					\
		    rgba[i][ACOMP] = a;					\
		    s[0][i] = ss*invQ;					\
		    t[0][i] = tt*invQ;					\
		    u[0][i] = uu*invQ;					\
		    lambda[0][i] = compute_lambda( dsdx, dsdy,		\
						   dtdx, dtdy,		\
						   invQ,		\
						   twidth0, theight0 );	\
		    s[1][i] = ss1*invQ1;				\
		    t[1][i] = tt1*invQ1;				\
		    u[1][i] = uu1*invQ1;				\
		    lambda[1][i] = compute_lambda( ds1dx, ds1dy,	\
						   dt1dx, dt1dy,	\
						   invQ1,		\
						   twidth1, theight1 );	\
		    ffz += fdzdx;					\
		    ss += dsdx;						\
		    tt += dtdx;						\
		    uu += dudx;						\
		    vv += dvdx;						\
		    ss1 += ds1dx;					\
		    tt1 += dt1dx;					\
		    uu1 += du1dx;					\
		    vv1 += dv1dx;					\
		 }							\
              }								\
              else {							\
		 for (i=0;i<n;i++) {					\
		    GLdouble invQ = 1.0 / vv;				\
		    GLdouble invQ1 = 1.0 / vv1;				\
		    zspan[i] = FixedToDepth(ffz);			\
		    rgba[i][RCOMP] = FixedToInt(ffr);			\
		    rgba[i][GCOMP] = FixedToInt(ffg);			\
		    rgba[i][BCOMP] = FixedToInt(ffb);			\
		    rgba[i][ACOMP] = FixedToInt(ffa);			\
		    s[0][i] = ss*invQ;					\
		    t[0][i] = tt*invQ;					\
		    u[0][i] = uu*invQ;					\
		    lambda[0][i] = compute_lambda( dsdx, dsdy,		\
						   dtdx, dtdy,		\
						   invQ,		\
						   twidth0, theight0 );	\
		    s[1][i] = ss1*invQ1;				\
		    t[1][i] = tt1*invQ1;				\
		    u[1][i] = uu1*invQ1;				\
		    lambda[1][i] = compute_lambda( ds1dx, ds1dy,	\
						   dt1dx, dt1dy,	\
						   invQ1,		\
						   twidth1, theight1 );	\
		    ffz += fdzdx;					\
		    ffr += fdrdx;					\
		    ffg += fdgdx;					\
		    ffb += fdbdx;					\
		    ffa += fdadx;					\
		    ss += dsdx;						\
		    tt += dtdx;						\
		    uu += dudx;						\
		    vv += dvdx;						\
		    ss1 += ds1dx;					\
		    tt1 += dt1dx;					\
		    uu1 += du1dx;					\
		    vv1 += dv1dx;					\
		 }							\
              }								\
	      gl_write_multitexture_span( ctx, 2, n, LEFT, Y, zspan,	\
                                      (CONST GLfloat (*)[MAX_WIDTH]) s,	\
                                      (CONST GLfloat (*)[MAX_WIDTH]) t,	\
                                      (CONST GLfloat (*)[MAX_WIDTH]) u,	\
                                      (GLfloat (*)[MAX_WIDTH]) lambda,	\
	                              rgba, NULL, GL_POLYGON );		\
	   }								\
	}

#include "tritemp.h"
}


/*
 * These wrappers are needed to deal with the 32KB / stack frame limit
 * on Mac / PowerPC systems.
 */

static void general_textured_spec_triangle(GLcontext *ctx, GLuint v0,
                                           GLuint v1, GLuint v2, GLuint pv)
{
   GLdepth zspan[MAX_WIDTH];
   GLubyte rgba[MAX_WIDTH][4], spec[MAX_WIDTH][4];
   general_textured_spec_triangle1(ctx,v0,v1,v2,pv,zspan,rgba,spec);
}

static void lambda_textured_triangle( GLcontext *ctx, GLuint v0,
                                      GLuint v1, GLuint v2, GLuint pv )
{
   GLfloat s[MAX_WIDTH], t[MAX_WIDTH], u[MAX_WIDTH];
   lambda_textured_triangle1(ctx,v0,v1,v2,pv,s,t,u);
}

static void lambda_textured_spec_triangle( GLcontext *ctx, GLuint v0,
                                           GLuint v1, GLuint v2, GLuint pv )
{
   GLfloat s[MAX_WIDTH];
   GLfloat t[MAX_WIDTH];
   GLfloat u[MAX_WIDTH];
   lambda_textured_spec_triangle1(ctx,v0,v1,v2,pv,s,t,u);
}


static void lambda_multitextured_triangle( GLcontext *ctx, GLuint v0,
                                           GLuint v1, GLuint v2, GLuint pv)
{

   GLfloat s[MAX_TEXTURE_UNITS][MAX_WIDTH];
   GLfloat t[MAX_TEXTURE_UNITS][MAX_WIDTH];
   DEFMARRAY(GLfloat,u,MAX_TEXTURE_UNITS,MAX_WIDTH);
   CHECKARRAY(u,return);
   
   lambda_multitextured_triangle1(ctx,v0,v1,v2,pv,s,t,u);
   
   UNDEFARRAY(u);
}



static void occlusion_zless_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                      GLuint v2, GLuint pv )
{
   (void)pv;
   if (ctx->OcclusionResult) {
      return;
   }

#define DO_OCCLUSION_TEST
#define INTERP_Z 1
#define DEPTH_TYPE DEFAULT_SOFTWARE_DEPTH_TYPE
#define INNER_LOOP( LEFT, RIGHT, Y )		\
   {						\
      GLint i, len = RIGHT-LEFT;		\
      for (i=0;i<len;i++) {			\
	 GLdepth z = FixedToDepth(ffz);		\
	 if (z < zRow[i]) {			\
	    ctx->OcclusionResult = GL_TRUE;	\
	    return;				\
	 }					\
	 ffz += fdzdx;				\
      }						\
   }
#include "tritemp.h"
}



/*
 * Null rasterizer for measuring transformation speed.
 */
static void null_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                           GLuint v2, GLuint pv )
{
   (void) ctx;
   (void) v0;
   (void) v1;
   (void) v2;
   (void) pv;
}


#if 0
# define dputs(s) puts(s)
#else
# define dputs(s)
#endif



/*
 * Determine which triangle rendering function to use given the current
 * rendering context.
 */
void gl_set_triangle_function( GLcontext *ctx )
{
   GLboolean rgbmode = ctx->Visual->RGBAflag;

   if (ctx->RenderMode==GL_RENDER) {
      if (ctx->NoRaster) {
         ctx->Driver.TriangleFunc = null_triangle;
         return;
      }
      if (ctx->Driver.TriangleFunc) {
         /* Device driver will draw triangles. */
         dputs("Driver triangle");
	 return;
      }

      if (ctx->Polygon.SmoothFlag) {
         _mesa_set_aa_triangle_function(ctx);
         ASSERT(ctx->Driver.TriangleFunc);
         return;
      }

      if (ctx->Depth.OcclusionTest &&
          ctx->DrawBuffer->DepthBuffer &&
          ctx->Depth.Test &&
          ctx->Depth.Mask == GL_FALSE &&
          ctx->Depth.Func == GL_LESS &&
          !ctx->Stencil.Enabled) {
         if ((ctx->Visual->RGBAflag &&
              ctx->Color.ColorMask[0] == 0 && 
              ctx->Color.ColorMask[1] == 0 && 
              ctx->Color.ColorMask[2] == 0 &&
              ctx->Color.ColorMask[3] == 0)
             ||
             (!ctx->Visual->RGBAflag && ctx->Color.IndexMask == 0)) {
            dputs("occlusion_test_triangle");
            ctx->Driver.TriangleFunc = occlusion_zless_triangle;
            return;
         }
      }

      if (ctx->Texture.ReallyEnabled) {
         /* Ugh, we do a _lot_ of tests to pick the best textured tri func */
	 GLint format, filter;
	 const struct gl_texture_object *current2Dtex = ctx->Texture.Unit[0].CurrentD[2];
         const struct gl_texture_image *image;
         /* First see if we can used an optimized 2-D texture function */
         if (ctx->Texture.ReallyEnabled==TEXTURE0_2D
             && current2Dtex->WrapS==GL_REPEAT
	     && current2Dtex->WrapT==GL_REPEAT
             && ((image = current2Dtex->Image[current2Dtex->BaseLevel]) != 0)  /* correct! */
             && image->Border==0
             && ((format = image->Format)==GL_RGB || format==GL_RGBA)
	     && (filter = current2Dtex->MinFilter)==current2Dtex->MagFilter
	     && ctx->Light.Model.ColorControl==GL_SINGLE_COLOR
	     && ctx->Texture.Unit[0].EnvMode!=GL_COMBINE_EXT) {

	    if (ctx->Hint.PerspectiveCorrection==GL_FASTEST) {
	     
	       if (filter==GL_NEAREST
		   && format==GL_RGB
		   && (ctx->Texture.Unit[0].EnvMode==GL_REPLACE
		       || ctx->Texture.Unit[0].EnvMode==GL_DECAL)
		   && ((ctx->RasterMask==DEPTH_BIT
			&& ctx->Depth.Func==GL_LESS
			&& ctx->Depth.Mask==GL_TRUE)
		       || ctx->RasterMask==0)
		   && ctx->Polygon.StippleFlag==GL_FALSE) {

		  if (ctx->RasterMask==DEPTH_BIT) {
		     ctx->Driver.TriangleFunc = simple_z_textured_triangle;
		     dputs("simple_z_textured_triangle");
		  }
		  else {
		     ctx->Driver.TriangleFunc = simple_textured_triangle;
		     dputs("simple_textured_triangle");
		  }
	       }
	       else {
                  if (ctx->Texture.Unit[0].EnvMode==GL_ADD) {
                     ctx->Driver.TriangleFunc = general_textured_triangle;
                     dputs("general_textured_triangle");
                  }
                  else {
                     ctx->Driver.TriangleFunc = affine_textured_triangle;
                     dputs("affine_textured_triangle");
                  }
	       }
	    }
	    else {
	       ctx->Driver.TriangleFunc = persp_textured_triangle;
	       dputs("persp_textured_triangle");
	    }
	 }
         else {
            /* More complicated textures (mipmap, multi-tex, sep specular) */
            GLboolean needLambda;
            /* if mag filter != min filter we need to compute lambda */
            const struct gl_texture_object *obj0 = ctx->Texture.Unit[0].Current;
            const struct gl_texture_object *obj1 = ctx->Texture.Unit[1].Current;
            if (obj0 && obj0->MinFilter != obj0->MagFilter)
               needLambda = GL_TRUE;
            else if (obj1 && obj1->MinFilter != obj1->MagFilter)
               needLambda = GL_TRUE;
            else
               needLambda = GL_FALSE;
            if (ctx->Texture.ReallyEnabled >= TEXTURE1_1D) {
               /* multi-texture! */
               ctx->Driver.TriangleFunc = lambda_multitextured_triangle;
	       dputs("lambda_multitextured_triangle");
            }
            else if (ctx->Light.Enabled &&
               ctx->Light.Model.ColorControl==GL_SEPARATE_SPECULAR_COLOR) {
               /* separate specular color interpolation */
               if (needLambda) {
                  ctx->Driver.TriangleFunc = lambda_textured_spec_triangle;
		  dputs("lambda_textured_spec_triangle");
	       }
               else {
                  ctx->Driver.TriangleFunc = general_textured_spec_triangle;
		  dputs("general_textured_spec_triangle");
	       }
            }
            else {
               if (needLambda) {
                  ctx->Driver.TriangleFunc = lambda_textured_triangle;
		  dputs("lambda_textured_triangle");
	       }
               else {
                  ctx->Driver.TriangleFunc = general_textured_triangle;
		  dputs("general_textured_triangle");
	       }
            }
         }
      }
      else {
	 if (ctx->Light.ShadeModel==GL_SMOOTH) {
	    /* smooth shaded, no texturing, stippled or some raster ops */
            if (rgbmode) {
               dputs("smooth_rgba_triangle");
               ctx->Driver.TriangleFunc = smooth_rgba_triangle;
            }
            else {
               dputs("smooth_ci_triangle");
               ctx->Driver.TriangleFunc = smooth_ci_triangle;
            }
	 }
	 else {
	    /* flat shaded, no texturing, stippled or some raster ops */
            if (rgbmode) {
               dputs("flat_rgba_triangle");
               ctx->Driver.TriangleFunc = flat_rgba_triangle;
            }
            else {
               dputs("flat_ci_triangle");
               ctx->Driver.TriangleFunc = flat_ci_triangle;
            }
	 }
      }
   }
   else if (ctx->RenderMode==GL_FEEDBACK) {
      ctx->Driver.TriangleFunc = gl_feedback_triangle;
   }
   else {
      /* GL_SELECT mode */
      ctx->Driver.TriangleFunc = gl_select_triangle;
   }
}
