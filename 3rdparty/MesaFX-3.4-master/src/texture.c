/* $Id: texture.c,v 1.22.4.8 2001/04/23 18:07:58 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.4.1
 *
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
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


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "context.h"
#include "macros.h"
#include "mmath.h"
#include "mem.h"
#include "pb.h"
#include "teximage.h"
#include "texture.h"
#include "types.h"
#include "xform.h"
#endif


/***********************************************************************
 * Automatic texture coordinate generation (texgen) code.
 */

static GLuint all_bits[5] = {
   0,
   VEC_SIZE_1,
   VEC_SIZE_2,
   VEC_SIZE_3,
   VEC_SIZE_4,
};


static texgen_func texgen_generic_tab[4];
static texgen_func texgen_reflection_map_nv_tab[4];
static texgen_func texgen_normal_map_nv_tab[4];
static texgen_func texgen_sphere_map_tab[4];


typedef void (*build_m_func)(GLfloat f[][3],
			     GLfloat m[],
			     const GLvector3f *normals,
			     const GLvector4f *coord_vec,
			     const GLuint flags[],
			     const GLubyte cullmask[] );


typedef void (*build_f_func)( GLfloat *f,
			      GLuint fstride,
			      const GLvector3f *normal_vec,
			      const GLvector4f *coord_vec,
			      const GLuint flags[],
			      const GLubyte cullmask[] );


/* KW: compacted vs. coindexed normals don't bring any performance
 *     gains to texture generation, but it is still necessary to cope
 *     with the two different formats.
 */
#define IDX              0
#define TAG(x)           x
#define FIRST_NORMAL     normals->start
#define CUR_NORMAL       (void) normal
#define NEXT_NORMAL      STRIDE_F(normal, normals->stride)
#define CHECK            
#define LOCAL_VARS
#include "texgen_tmp.h"


#define IDX              2
#define TAG(x)           x##_compacted
#define FIRST_NORMAL     normals->start
#define CUR_NORMAL       ((flags[i]&VERT_NORM) ? normal=normal_ptr : normal)
#define NEXT_NORMAL      STRIDE_F(normal_ptr, normals->stride)
#define CHECK            
#define LOCAL_VARS       const GLfloat *normal_ptr = FIRST_NORMAL;
#include "texgen_tmp.h"


#define IDX              1
#define TAG(x)           x##_masked
#define FIRST_NORMAL     normals->start
#define CUR_NORMAL       (void) normal
#define NEXT_NORMAL      STRIDE_F(normal, normals->stride)
#define CHECK            if (cullmask[i])
#define LOCAL_VARS       
#include "texgen_tmp.h"

#define IDX              3
#define TAG(x)           x##_compacted_masked
#define FIRST_NORMAL     normals->start
#define CUR_NORMAL       ((flags[i]&VERT_NORM) ? normal=normal_ptr : normal)
#define NEXT_NORMAL      STRIDE_F(normal_ptr, normals->stride)
#define CHECK            if (cullmask[i])
#define LOCAL_VARS       const GLfloat *normal_ptr = FIRST_NORMAL;
#include "texgen_tmp.h"


/*
 * End texgen code
 ***********************************************************************
 */



/*
 * One time inits for texture mapping.
 * Called by one_time_init() in context.c
 */
void gl_init_texture( void )
{
   init_texgen();
   init_texgen_compacted();
   init_texgen_masked();
   init_texgen_compacted_masked();
}


/*
 * After state changes to texturing we call this function to update
 * intermediate and derived state.
 * Called by gl_update_state().
 */
void gl_update_texture_unit( GLcontext *ctx, struct gl_texture_unit *texUnit )
{
   (void) ctx;

   if ((texUnit->Enabled & TEXTURE0_CUBE) && texUnit->CurrentCubeMap->Complete) {
      texUnit->ReallyEnabled = TEXTURE0_CUBE;
      texUnit->Current = texUnit->CurrentCubeMap;
      texUnit->CurrentDimension = 6;
   }
   else if ((texUnit->Enabled & TEXTURE0_3D) && texUnit->CurrentD[3]->Complete) {
      texUnit->ReallyEnabled = TEXTURE0_3D;
      texUnit->Current = texUnit->CurrentD[3];
      texUnit->CurrentDimension = 3;
   }
   else if ((texUnit->Enabled & TEXTURE0_2D) && texUnit->CurrentD[2]->Complete) {
      texUnit->ReallyEnabled = TEXTURE0_2D;
      texUnit->Current = texUnit->CurrentD[2];
      texUnit->CurrentDimension = 2;
   }
   else if ((texUnit->Enabled & TEXTURE0_1D) && texUnit->CurrentD[1]->Complete) {
      texUnit->ReallyEnabled = TEXTURE0_1D;
      texUnit->Current = texUnit->CurrentD[1];
      texUnit->CurrentDimension = 1;
   }
   else {
      if (MESA_VERBOSE & VERBOSE_TEXTURE) {
	 switch (texUnit->Enabled) {
	 case TEXTURE0_CUBE:
	    fprintf(stderr, "Using incomplete cube texture %u\n",
		    texUnit->CurrentCubeMap->Name);
	    break;
	 case TEXTURE0_3D:
	    fprintf(stderr, "Using incomplete 3d texture %u\n",
		    texUnit->CurrentD[3]->Name);
	    break;
	 case TEXTURE0_2D:
	    fprintf(stderr, "Using incomplete 2d texture %u\n",
		    texUnit->CurrentD[2]->Name);
	    break;
	 case TEXTURE0_1D:
	    fprintf(stderr, "Using incomplete 1d texture %u\n",
		    texUnit->CurrentD[1]->Name);
	    break;
	 default:
	    fprintf(stderr, "Bad value for texUnit->Enabled %x\n",
		    texUnit->Enabled);
	    break;
	 }
      }

      texUnit->ReallyEnabled = 0;
      texUnit->Current = NULL;
      texUnit->CurrentDimension = 0;
      return;
   }

   texUnit->GenFlags = 0;

   if (texUnit->TexGenEnabled) {
      GLuint sz = 0;

      if (texUnit->TexGenEnabled & S_BIT) {
         sz = 1;
         texUnit->GenFlags |= texUnit->GenBitS;
      }
      if (texUnit->TexGenEnabled & T_BIT) {
         sz = 2;
         texUnit->GenFlags |= texUnit->GenBitT;
      }
      if (texUnit->TexGenEnabled & Q_BIT) {
         sz = 3;
         texUnit->GenFlags |= texUnit->GenBitQ;
      }
      if (texUnit->TexGenEnabled & R_BIT) {
         sz = 4;
         texUnit->GenFlags |= texUnit->GenBitR;
      }

      texUnit->TexgenSize = sz;
      texUnit->Holes = (GLubyte) (all_bits[sz] & ~texUnit->TexGenEnabled);
      texUnit->func = texgen_generic_tab;

      if (texUnit->TexGenEnabled == (S_BIT|T_BIT|R_BIT)) {
	 if (texUnit->GenFlags == TEXGEN_REFLECTION_MAP_NV) {
	    texUnit->func = texgen_reflection_map_nv_tab;
	 }
	 else if (texUnit->GenFlags == TEXGEN_NORMAL_MAP_NV) {
	    texUnit->func = texgen_normal_map_nv_tab;
	 }
      }
      else if (texUnit->TexGenEnabled == (S_BIT|T_BIT) &&
	       texUnit->GenFlags == TEXGEN_SPHERE_MAP) {
	 texUnit->func = texgen_sphere_map_tab;
      }
   }
}



/*
 * Paletted texture sampling.
 * Input:  tObj - the texture object
 *         index - the palette index (8-bit only)
 * Output:  red, green, blue, alpha - the texel color
 */
static void palette_sample(const struct gl_texture_object *tObj,
                           GLubyte index, GLubyte rgba[4] )
{
   GLcontext *ctx = gl_get_current_context();  /* THIS IS A HACK */
   GLint i = index;
   const GLubyte *palette;
   GLenum format;

   if (ctx->Texture.SharedPalette) {
      ASSERT(ctx->Texture.Palette.TableType == GL_UNSIGNED_BYTE);
      palette = (const GLubyte *) ctx->Texture.Palette.Table;
      format = ctx->Texture.Palette.Format;
   }
   else {
      ASSERT(tObj->Palette.TableType == GL_UNSIGNED_BYTE);
      palette = (const GLubyte *) tObj->Palette.Table;
      format = tObj->Palette.Format;
   }

   switch (format) {
      case GL_ALPHA:
         rgba[ACOMP] = palette[index];
         return;
      case GL_LUMINANCE:
      case GL_INTENSITY:
         rgba[RCOMP] = palette[index];
         return;
      case GL_LUMINANCE_ALPHA:
         rgba[RCOMP] = palette[(index << 1) + 0];
         rgba[ACOMP] = palette[(index << 1) + 1];
         return;
      case GL_RGB:
         rgba[RCOMP] = palette[index * 3 + 0];
         rgba[GCOMP] = palette[index * 3 + 1];
         rgba[BCOMP] = palette[index * 3 + 2];
         return;
      case GL_RGBA:
         rgba[RCOMP] = palette[(i << 2) + 0];
         rgba[GCOMP] = palette[(i << 2) + 1];
         rgba[BCOMP] = palette[(i << 2) + 2];
         rgba[ACOMP] = palette[(i << 2) + 3];
         return;
      default:
         gl_problem(NULL, "Bad palette format in palette_sample");
   }
}



/*
 * These values are used in the fixed-point arithmetic used
 * for linear filtering.
 */
#define WEIGHT_SCALE 65536.0F
#define WEIGHT_SHIFT 16


/*
 * Used to compute texel locations for linear sampling.
 */
#define COMPUTE_LINEAR_TEXEL_LOCATIONS(wrapMode, S, U, SIZE, I0, I1)	\
{									\
   if (wrapMode == GL_REPEAT) {						\
      U = S * SIZE - 0.5F;						\
      I0 = ((GLint) myFloor(U)) & (SIZE - 1);				\
      I1 = (I0 + 1) & (SIZE - 1);					\
   }									\
   else {								\
      U = S * SIZE;							\
      if (U < 0.0F)							\
         U = 0.0F;							\
      else if (U >= SIZE)						\
         U = SIZE;							\
      U -= 0.5F;							\
      I0 = (GLint) myFloor(U);						\
      I1 = I0 + 1;							\
      if (wrapMode == GL_CLAMP_TO_EDGE) {				\
         if (I0 < 0)							\
            I0 = 0;							\
         if (I1 >= SIZE)						\
            I1 = SIZE - 1;						\
      }									\
   }									\
}


/*
 * Used to compute texel location for nearest sampling.
 */
#define COMPUTE_NEAREST_TEXEL_LOCATION(wrapMode, S, SIZE, I)		\
{									\
   if (wrapMode == GL_REPEAT) {						\
      /* s limited to [0,1) */						\
      /* i limited to [0,width-1] */					\
      I = (GLint) (S * SIZE);						\
      if (S < 0.0F)							\
         I -= 1;							\
      I &= (SIZE - 1);							\
   }									\
   else if (wrapMode == GL_CLAMP_TO_EDGE) {				\
      const GLfloat min = 1.0F / (2.0F * SIZE);				\
      const GLfloat max = 1.0F - min;					\
      if (S < min)							\
         I = 0;								\
      else if (S > max)							\
         I = SIZE - 1;							\
      else								\
         I = (GLint) (S * SIZE);					\
   }									\
   else {								\
      ASSERT(wrapMode == GL_CLAMP);					\
      /* s limited to [0,1] */						\
      /* i limited to [0,width-1] */					\
      if (S <= 0.0F)							\
         I = 0;								\
      else if (S >= 1.0F)						\
         I = SIZE - 1;							\
      else								\
         I = (GLint) (S * SIZE);					\
   }									\
}


/*
 * Compute linear mipmap levels for given lambda.
 */
#define COMPUTE_LINEAR_MIPMAP_LEVEL(tObj, lambda, level)	\
{								\
   if (lambda < 0.0F)						\
      lambda = 0.0F;						\
   else if (lambda > tObj->M)					\
      lambda = tObj->M;						\
   level = (GLint) (tObj->BaseLevel + lambda);			\
}


/*
 * Compute nearest mipmap level for given lambda.
 */
#define COMPUTE_NEAREST_MIPMAP_LEVEL(tObj, lambda, level)	\
{								\
   if (lambda <= 0.5F)						\
      lambda = 0.0F;						\
   else if (lambda > tObj->M + 0.4999F)				\
      lambda = tObj->M + 0.4999F;				\
   level = (GLint) (tObj->BaseLevel + lambda + 0.5F);		\
   if (level > tObj->P)						\
      level = tObj->P;						\
}




/*
 * Bitflags for texture border color sampling.
 */
#define I0BIT   1
#define I1BIT   2
#define J0BIT   4
#define J1BIT   8
#define K0BIT  16
#define K1BIT  32



/**********************************************************************/
/*                    1-D Texture Sampling Functions                  */
/**********************************************************************/


/*
 * Return floor of x, being careful of negative values.
 */
static GLfloat myFloor(GLfloat x)
{
   if (x < 0.0F)
      return (GLfloat) ((GLint) x - 1);
   else
      return (GLfloat) (GLint) x;
}


/*
 * Return the fractional part of x.
 */
#define myFrac(x)  ( (x) - myFloor(x) )




/*
 * Given 1-D texture image and an (i) texel column coordinate, return the
 * texel color.
 */
static void get_1d_texel( const struct gl_texture_object *tObj,
                          const struct gl_texture_image *img, GLint i,
                          GLubyte rgba[4] )
{
   const GLubyte *texel;

#ifdef DEBUG
   GLint width = img->Width;
   assert(i >= 0);
   assert(i < width);
#endif

   switch (img->Format) {
      case GL_COLOR_INDEX:
         {
            GLubyte index = img->Data[i];
            palette_sample(tObj, index, rgba);
            return;
         }
      case GL_ALPHA:
         rgba[ACOMP] = img->Data[ i ];
         return;
      case GL_LUMINANCE:
      case GL_INTENSITY:
         rgba[RCOMP] = img->Data[ i ];
         return;
      case GL_LUMINANCE_ALPHA:
         texel = img->Data + i * 2;
         rgba[RCOMP] = texel[0];
         rgba[ACOMP] = texel[1];
         return;
      case GL_RGB:
         texel = img->Data + i * 3;
         rgba[RCOMP] = texel[0];
         rgba[GCOMP] = texel[1];
         rgba[BCOMP] = texel[2];
         return;
      case GL_RGBA:
         texel = img->Data + i * 4;
         rgba[RCOMP] = texel[0];
         rgba[GCOMP] = texel[1];
         rgba[BCOMP] = texel[2];
         rgba[ACOMP] = texel[3];
         return;
      default:
         gl_problem(NULL, "Bad format in get_1d_texel");
         return;
   }
}



/*
 * Return the texture sample for coordinate (s) using GL_NEAREST filter.
 */
static void sample_1d_nearest( const struct gl_texture_object *tObj,
                               const struct gl_texture_image *img,
                               GLfloat s, GLubyte rgba[4] )
{
   const GLint width = img->Width2;  /* without border, power of two */
   const GLubyte *texel;
   GLint i;

   COMPUTE_NEAREST_TEXEL_LOCATION(tObj->WrapS, s, width, i);

   /* skip over the border, if any */
   i += img->Border;

   /* Get the texel */
   switch (img->Format) {
      case GL_COLOR_INDEX:
         {
            GLubyte index = img->Data[i];
            palette_sample(tObj, index, rgba );
            return;
         }
      case GL_ALPHA:
         rgba[ACOMP] = img->Data[i];
         return;
      case GL_LUMINANCE:
      case GL_INTENSITY:
         rgba[RCOMP] = img->Data[i];
         return;
      case GL_LUMINANCE_ALPHA:
         texel = img->Data + i * 2;
         rgba[RCOMP] = texel[0];
         rgba[ACOMP] = texel[1];
         return;
      case GL_RGB:
         texel = img->Data + i * 3;
         rgba[RCOMP] = texel[0];
         rgba[GCOMP] = texel[1];
         rgba[BCOMP] = texel[2];
         return;
      case GL_RGBA:
         texel = img->Data + i * 4;
         rgba[RCOMP] = texel[0];
         rgba[GCOMP] = texel[1];
         rgba[BCOMP] = texel[2];
         rgba[ACOMP] = texel[3];
         return;
      default:
         gl_problem(NULL, "Bad format in sample_1d_nearest");
   }
}



/*
 * Return the texture sample for coordinate (s) using GL_LINEAR filter.
 */
static void sample_1d_linear( const struct gl_texture_object *tObj,
                              const struct gl_texture_image *img,
                              GLfloat s,
                              GLubyte rgba[4] )
{
   const GLint width = img->Width2;
   GLint i0, i1;
   GLfloat u;
   GLuint useBorderColor;

   COMPUTE_LINEAR_TEXEL_LOCATIONS(tObj->WrapS, s, u, width, i0, i1);

   useBorderColor = 0;
   if (img->Border) {
      i0 += img->Border;
      i1 += img->Border;
   }
   else {
      if (i0 < 0 || i0 >= width)   useBorderColor |= I0BIT;
      if (i1 < 0 || i1 >= width)   useBorderColor |= I1BIT;
   }

   {
      const GLfloat a = myFrac(u);
      /* compute sample weights in fixed point in [0,WEIGHT_SCALE] */
      const GLint w0 = (GLint) ((1.0F-a) * WEIGHT_SCALE + 0.5F);
      const GLint w1 = (GLint) (      a  * WEIGHT_SCALE + 0.5F);

      GLubyte t0[4], t1[4];  /* texels */

      if (useBorderColor & I0BIT) {
         COPY_4UBV(t0, tObj->BorderColor);
      }
      else {
         get_1d_texel( tObj, img, i0, t0 );
      }
      if (useBorderColor & I1BIT) {
         COPY_4UBV(t1, tObj->BorderColor);
      }
      else {
         get_1d_texel( tObj, img, i1, t1 );
      }

      rgba[0] = (GLubyte) ((w0 * t0[0] + w1 * t1[0]) >> WEIGHT_SHIFT);
      rgba[1] = (GLubyte) ((w0 * t0[1] + w1 * t1[1]) >> WEIGHT_SHIFT);
      rgba[2] = (GLubyte) ((w0 * t0[2] + w1 * t1[2]) >> WEIGHT_SHIFT);
      rgba[3] = (GLubyte) ((w0 * t0[3] + w1 * t1[3]) >> WEIGHT_SHIFT);
   }
}


static void
sample_1d_nearest_mipmap_nearest( const struct gl_texture_object *tObj,
                                  GLfloat s, GLfloat lambda,
                                  GLubyte rgba[4] )
{
   GLint level;
   COMPUTE_NEAREST_MIPMAP_LEVEL(tObj, lambda, level);
   sample_1d_nearest( tObj, tObj->Image[level], s, rgba );
}


static void
sample_1d_linear_mipmap_nearest( const struct gl_texture_object *tObj,
                                 GLfloat s, GLfloat lambda,
                                 GLubyte rgba[4] )
{
   GLint level;
   COMPUTE_NEAREST_MIPMAP_LEVEL(tObj, lambda, level);
   sample_1d_linear( tObj, tObj->Image[level], s, rgba );
}



static void
sample_1d_nearest_mipmap_linear( const struct gl_texture_object *tObj,
                                 GLfloat s, GLfloat lambda,
                                 GLubyte rgba[4] )
{
   GLint level;

   COMPUTE_LINEAR_MIPMAP_LEVEL(tObj, lambda, level);

   if (level >= tObj->P) {
      sample_1d_nearest( tObj, tObj->Image[tObj->P], s, rgba );
   }
   else {
      GLubyte t0[4], t1[4];
      const GLfloat f = myFrac(lambda);
      sample_1d_nearest( tObj, tObj->Image[level  ], s, t0 );
      sample_1d_nearest( tObj, tObj->Image[level+1], s, t1 );
      rgba[RCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[RCOMP] + f * t1[RCOMP]);
      rgba[GCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[GCOMP] + f * t1[GCOMP]);
      rgba[BCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[BCOMP] + f * t1[BCOMP]);
      rgba[ACOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[ACOMP] + f * t1[ACOMP]);
   }
}



static void
sample_1d_linear_mipmap_linear( const struct gl_texture_object *tObj,
                                GLfloat s, GLfloat lambda,
                                GLubyte rgba[4] )
{
   GLint level;

   COMPUTE_LINEAR_MIPMAP_LEVEL(tObj, lambda, level);

   if (level >= tObj->P) {
      sample_1d_linear( tObj, tObj->Image[tObj->P], s, rgba );
   }
   else {
      GLubyte t0[4], t1[4];
      const GLfloat f = myFrac(lambda);
      sample_1d_linear( tObj, tObj->Image[level  ], s, t0 );
      sample_1d_linear( tObj, tObj->Image[level+1], s, t1 );
      rgba[RCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[RCOMP] + f * t1[RCOMP]);
      rgba[GCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[GCOMP] + f * t1[GCOMP]);
      rgba[BCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[BCOMP] + f * t1[BCOMP]);
      rgba[ACOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[ACOMP] + f * t1[ACOMP]);
   }
}



static void sample_nearest_1d( const struct gl_texture_object *tObj, GLuint n,
                               const GLfloat s[], const GLfloat t[],
                               const GLfloat u[], const GLfloat lambda[],
                               GLubyte rgba[][4] )
{
   GLuint i;
   struct gl_texture_image *image = tObj->Image[tObj->BaseLevel];
   (void) t;
   (void) u;
   (void) lambda;
   for (i=0;i<n;i++) {
      sample_1d_nearest( tObj, image, s[i], rgba[i] );
   }
}



static void sample_linear_1d( const struct gl_texture_object *tObj, GLuint n,
                              const GLfloat s[], const GLfloat t[],
                              const GLfloat u[], const GLfloat lambda[],
                              GLubyte rgba[][4] )
{
   GLuint i;
   struct gl_texture_image *image = tObj->Image[tObj->BaseLevel];
   (void) t;
   (void) u;
   (void) lambda;
   for (i=0;i<n;i++) {
      sample_1d_linear( tObj, image, s[i], rgba[i] );
   }
}


/*
 * Given an (s) texture coordinate and lambda (level of detail) value,
 * return a texture sample.
 *
 */
static void sample_lambda_1d( const struct gl_texture_object *tObj, GLuint n,
                              const GLfloat s[], const GLfloat t[],
                              const GLfloat u[], const GLfloat lambda[],
                              GLubyte rgba[][4] )
{
   GLuint i;

   (void) t;
   (void) u;

   for (i=0;i<n;i++) {
      if (lambda[i] > tObj->MinMagThresh) {
         /* minification */
         switch (tObj->MinFilter) {
            case GL_NEAREST:
               sample_1d_nearest( tObj, tObj->Image[tObj->BaseLevel], s[i], rgba[i] );
               break;
            case GL_LINEAR:
               sample_1d_linear( tObj, tObj->Image[tObj->BaseLevel], s[i], rgba[i] );
               break;
            case GL_NEAREST_MIPMAP_NEAREST:
               sample_1d_nearest_mipmap_nearest( tObj, lambda[i], s[i], rgba[i] );
               break;
            case GL_LINEAR_MIPMAP_NEAREST:
               sample_1d_linear_mipmap_nearest( tObj, s[i], lambda[i], rgba[i] );
               break;
            case GL_NEAREST_MIPMAP_LINEAR:
               sample_1d_nearest_mipmap_linear( tObj, s[i], lambda[i], rgba[i] );
               break;
            case GL_LINEAR_MIPMAP_LINEAR:
               sample_1d_linear_mipmap_linear( tObj, s[i], lambda[i], rgba[i] );
               break;
            default:
               gl_problem(NULL, "Bad min filter in sample_1d_texture");
               return;
         }
      }
      else {
         /* magnification */
         switch (tObj->MagFilter) {
            case GL_NEAREST:
               sample_1d_nearest( tObj, tObj->Image[tObj->BaseLevel], s[i], rgba[i] );
               break;
            case GL_LINEAR:
               sample_1d_linear( tObj, tObj->Image[tObj->BaseLevel], s[i], rgba[i] );
               break;
            default:
               gl_problem(NULL, "Bad mag filter in sample_1d_texture");
               return;
         }
      }
   }
}




/**********************************************************************/
/*                    2-D Texture Sampling Functions                  */
/**********************************************************************/


/*
 * Given a texture image and an (i,j) integer texel coordinate, return the
 * texel color.
 */
static void get_2d_texel( const struct gl_texture_object *tObj,
                          const struct gl_texture_image *img, GLint i, GLint j,
                          GLubyte rgba[4] )
{
   const GLint width = img->Width;    /* includes border */
   const GLubyte *texel;

#ifdef DEBUG
   const GLint height = img->Height;  /* includes border */
   assert(i >= 0);
   assert(i < width);
   assert(j >= 0);
   assert(j < height);
#endif

   switch (img->Format) {
      case GL_COLOR_INDEX:
         {
            GLubyte index = img->Data[ width *j + i ];
            palette_sample(tObj, index, rgba );
            return;
         }
      case GL_ALPHA:
         rgba[ACOMP] = img->Data[ width * j + i ];
         return;
      case GL_LUMINANCE:
      case GL_INTENSITY:
         rgba[RCOMP] = img->Data[ width * j + i ];
         return;
      case GL_LUMINANCE_ALPHA:
         texel = img->Data + (width * j + i) * 2;
         rgba[RCOMP] = texel[0];
         rgba[ACOMP] = texel[1];
         return;
      case GL_RGB:
         texel = img->Data + (width * j + i) * 3;
         rgba[RCOMP] = texel[0];
         rgba[GCOMP] = texel[1];
         rgba[BCOMP] = texel[2];
         return;
      case GL_RGBA:
         texel = img->Data + (width * j + i) * 4;
         rgba[RCOMP] = texel[0];
         rgba[GCOMP] = texel[1];
         rgba[BCOMP] = texel[2];
         rgba[ACOMP] = texel[3];
         return;
      default:
         gl_problem(NULL, "Bad format in get_2d_texel");
   }
}



/*
 * Return the texture sample for coordinate (s,t) using GL_NEAREST filter.
 */
static void sample_2d_nearest( const struct gl_texture_object *tObj,
                               const struct gl_texture_image *img,
                               GLfloat s, GLfloat t,
                               GLubyte rgba[] )
{
   const GLint imgWidth = img->Width;  /* includes border */
   const GLint width = img->Width2;    /* without border, power of two */
   const GLint height = img->Height2;  /* without border, power of two */
   const GLubyte *texel;
   GLint i, j;

   COMPUTE_NEAREST_TEXEL_LOCATION(tObj->WrapS, s, width,  i);
   COMPUTE_NEAREST_TEXEL_LOCATION(tObj->WrapT, t, height, j);

   /* skip over the border, if any */
   i += img->Border;
   j += img->Border;

   switch (img->Format) {
      case GL_COLOR_INDEX:
         {
            GLubyte index = img->Data[ j * imgWidth + i ];
            palette_sample(tObj, index, rgba);
            return;
         }
      case GL_ALPHA:
         rgba[ACOMP] = img->Data[ j * imgWidth + i ];
         return;
      case GL_LUMINANCE:
      case GL_INTENSITY:
         rgba[RCOMP] = img->Data[ j * imgWidth + i ];
         return;
      case GL_LUMINANCE_ALPHA:
         texel = img->Data + ((j * imgWidth + i) << 1);
         rgba[RCOMP] = texel[0];
         rgba[ACOMP] = texel[1];
         return;
      case GL_RGB:
         texel = img->Data + (j * imgWidth + i) * 3;
         rgba[RCOMP] = texel[0];
         rgba[GCOMP] = texel[1];
         rgba[BCOMP] = texel[2];
         return;
      case GL_RGBA:
         texel = img->Data + ((j * imgWidth + i) << 2);
         rgba[RCOMP] = texel[0];
         rgba[GCOMP] = texel[1];
         rgba[BCOMP] = texel[2];
         rgba[ACOMP] = texel[3];
         return;
      default:
         gl_problem(NULL, "Bad format in sample_2d_nearest");
   }
}



/*
 * Return the texture sample for coordinate (s,t) using GL_LINEAR filter.
 * New sampling code contributed by Lynn Quam <quam@ai.sri.com>.
 */
static void sample_2d_linear( const struct gl_texture_object *tObj,
                              const struct gl_texture_image *img,
                              GLfloat s, GLfloat t,
                              GLubyte rgba[] )
{
   const GLint width = img->Width2;
   const GLint height = img->Height2;
   GLint i0, j0, i1, j1;
   GLuint useBorderColor;
   GLfloat u, v;

   COMPUTE_LINEAR_TEXEL_LOCATIONS(tObj->WrapS, s, u, width,  i0, i1);
   COMPUTE_LINEAR_TEXEL_LOCATIONS(tObj->WrapT, t, v, height, j0, j1);

   useBorderColor = 0;
   if (img->Border) {
      i0 += img->Border;
      i1 += img->Border;
      j0 += img->Border;
      j1 += img->Border;
   }
   else {
      if (i0 < 0 || i0 >= width)   useBorderColor |= I0BIT;
      if (i1 < 0 || i1 >= width)   useBorderColor |= I1BIT;
      if (j0 < 0 || j0 >= height)  useBorderColor |= J0BIT;
      if (j1 < 0 || j1 >= height)  useBorderColor |= J1BIT;
   }

   {
      const GLfloat a = myFrac(u);
      const GLfloat b = myFrac(v);
      /* compute sample weights in fixed point in [0,WEIGHT_SCALE] */
      const GLint w00 = (GLint) ((1.0F-a)*(1.0F-b) * WEIGHT_SCALE + 0.5F);
      const GLint w10 = (GLint) (      a *(1.0F-b) * WEIGHT_SCALE + 0.5F);
      const GLint w01 = (GLint) ((1.0F-a)*      b  * WEIGHT_SCALE + 0.5F);
      const GLint w11 = (GLint) (      a *      b  * WEIGHT_SCALE + 0.5F);
      GLubyte t00[4];
      GLubyte t10[4];
      GLubyte t01[4];
      GLubyte t11[4];

      if (useBorderColor & (I0BIT | J0BIT)) {
         COPY_4UBV(t00, tObj->BorderColor);
      }
      else {
         get_2d_texel( tObj, img, i0, j0, t00 );
      }
      if (useBorderColor & (I1BIT | J0BIT)) {
         COPY_4UBV(t10, tObj->BorderColor);
      }
      else {
         get_2d_texel( tObj, img, i1, j0, t10 );
      }
      if (useBorderColor & (I0BIT | J1BIT)) {
         COPY_4UBV(t01, tObj->BorderColor);
      }
      else {
         get_2d_texel( tObj, img, i0, j1, t01 );
      }
      if (useBorderColor & (I1BIT | J1BIT)) {
         COPY_4UBV(t11, tObj->BorderColor);
      }
      else {
         get_2d_texel( tObj, img, i1, j1, t11 );
      }

      rgba[0] = (GLubyte) ((w00 * t00[0] + w10 * t10[0] + w01 * t01[0] + w11 * t11[0]) >> WEIGHT_SHIFT);
      rgba[1] = (GLubyte) ((w00 * t00[1] + w10 * t10[1] + w01 * t01[1] + w11 * t11[1]) >> WEIGHT_SHIFT);
      rgba[2] = (GLubyte) ((w00 * t00[2] + w10 * t10[2] + w01 * t01[2] + w11 * t11[2]) >> WEIGHT_SHIFT);
      rgba[3] = (GLubyte) ((w00 * t00[3] + w10 * t10[3] + w01 * t01[3] + w11 * t11[3]) >> WEIGHT_SHIFT);
   }

}



static void
sample_2d_nearest_mipmap_nearest( const struct gl_texture_object *tObj,
                                  GLfloat s, GLfloat t, GLfloat lambda,
                                  GLubyte rgba[4] )
{
   GLint level;
   COMPUTE_NEAREST_MIPMAP_LEVEL(tObj, lambda, level);
   sample_2d_nearest( tObj, tObj->Image[level], s, t, rgba );
}



static void
sample_2d_linear_mipmap_nearest( const struct gl_texture_object *tObj,
                                 GLfloat s, GLfloat t, GLfloat lambda,
                                 GLubyte rgba[4] )
{
   GLint level;
   COMPUTE_NEAREST_MIPMAP_LEVEL(tObj, lambda, level);
   sample_2d_linear( tObj, tObj->Image[level], s, t, rgba );
}



static void
sample_2d_nearest_mipmap_linear( const struct gl_texture_object *tObj,
                                 GLfloat s, GLfloat t, GLfloat lambda,
                                 GLubyte rgba[4] )
{
   GLint level;

   COMPUTE_LINEAR_MIPMAP_LEVEL(tObj, lambda, level);

   if (level >= tObj->P) {
      sample_2d_nearest( tObj, tObj->Image[tObj->P], s, t, rgba );
   }
   else {
      GLubyte t0[4], t1[4];  /* texels */
      const GLfloat f = myFrac(lambda);
      sample_2d_nearest( tObj, tObj->Image[level  ], s, t, t0 );
      sample_2d_nearest( tObj, tObj->Image[level+1], s, t, t1 );
      rgba[RCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[RCOMP] + f * t1[RCOMP]);
      rgba[GCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[GCOMP] + f * t1[GCOMP]);
      rgba[BCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[BCOMP] + f * t1[BCOMP]);
      rgba[ACOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[ACOMP] + f * t1[ACOMP]);
   }
}



static void
sample_2d_linear_mipmap_linear( const struct gl_texture_object *tObj,
                                GLfloat s, GLfloat t, GLfloat lambda,
                                GLubyte rgba[4] )
{
   GLint level;

   COMPUTE_LINEAR_MIPMAP_LEVEL(tObj, lambda, level);

   if (level >= tObj->P) {
      sample_2d_linear( tObj, tObj->Image[tObj->P], s, t, rgba );
   }
   else {
      GLubyte t0[4], t1[4];  /* texels */
      const GLfloat f = myFrac(lambda);
      sample_2d_linear( tObj, tObj->Image[level  ], s, t, t0 );
      sample_2d_linear( tObj, tObj->Image[level+1], s, t, t1 );
      rgba[RCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[RCOMP] + f * t1[RCOMP]);
      rgba[GCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[GCOMP] + f * t1[GCOMP]);
      rgba[BCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[BCOMP] + f * t1[BCOMP]);
      rgba[ACOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[ACOMP] + f * t1[ACOMP]);
   }
}



static void sample_nearest_2d( const struct gl_texture_object *tObj, GLuint n,
                               const GLfloat s[], const GLfloat t[],
                               const GLfloat u[], const GLfloat lambda[],
                               GLubyte rgba[][4] )
{
   GLuint i;
   struct gl_texture_image *image = tObj->Image[tObj->BaseLevel];
   (void) u;
   (void) lambda;
   for (i=0;i<n;i++) {
      sample_2d_nearest( tObj, image, s[i], t[i], rgba[i] );
   }
}



static void sample_linear_2d( const struct gl_texture_object *tObj, GLuint n,
                              const GLfloat s[], const GLfloat t[],
                              const GLfloat u[], const GLfloat lambda[],
                              GLubyte rgba[][4] )
{
   GLuint i;
   struct gl_texture_image *image = tObj->Image[tObj->BaseLevel];
   (void) u;
   (void) lambda;
   for (i=0;i<n;i++) {
      sample_2d_linear( tObj, image, s[i], t[i], rgba[i] );
   }
}


/*
 * Given an (s,t) texture coordinate and lambda (level of detail) value,
 * return a texture sample.
 */
static void sample_lambda_2d( const struct gl_texture_object *tObj,
                              GLuint n,
                              const GLfloat s[], const GLfloat t[],
                              const GLfloat u[], const GLfloat lambda[],
                              GLubyte rgba[][4] )
{
   GLuint i;
   (void) u;
   for (i=0;i<n;i++) {
      if (lambda[i] > tObj->MinMagThresh) {
         /* minification */
         switch (tObj->MinFilter) {
            case GL_NEAREST:
               sample_2d_nearest( tObj, tObj->Image[tObj->BaseLevel], s[i], t[i], rgba[i] );
               break;
            case GL_LINEAR:
               sample_2d_linear( tObj, tObj->Image[tObj->BaseLevel], s[i], t[i], rgba[i] );
               break;
            case GL_NEAREST_MIPMAP_NEAREST:
               sample_2d_nearest_mipmap_nearest( tObj, s[i], t[i], lambda[i], rgba[i] );
               break;
            case GL_LINEAR_MIPMAP_NEAREST:
               sample_2d_linear_mipmap_nearest( tObj, s[i], t[i], lambda[i], rgba[i] );
               break;
            case GL_NEAREST_MIPMAP_LINEAR:
               sample_2d_nearest_mipmap_linear( tObj, s[i], t[i], lambda[i], rgba[i] );
               break;
            case GL_LINEAR_MIPMAP_LINEAR:
               sample_2d_linear_mipmap_linear( tObj, s[i], t[i], lambda[i], rgba[i] );
               break;
            default:
               gl_problem(NULL, "Bad min filter in sample_2d_texture");
               return;
         }
      }
      else {
         /* magnification */
         switch (tObj->MagFilter) {
            case GL_NEAREST:
               sample_2d_nearest( tObj, tObj->Image[tObj->BaseLevel], s[i], t[i], rgba[i] );
               break;
            case GL_LINEAR:
               sample_2d_linear( tObj, tObj->Image[tObj->BaseLevel], s[i], t[i], rgba[i] );
               break;
            default:
               gl_problem(NULL, "Bad mag filter in sample_2d_texture");
         }
      }
   }
}


/*
 * Optimized 2-D texture sampling:
 *    S and T wrap mode == GL_REPEAT
 *    GL_NEAREST min/mag filter
 *    No border
 *    Format = GL_RGB
 */
static void opt_sample_rgb_2d( const struct gl_texture_object *tObj,
                               GLuint n, const GLfloat s[], const GLfloat t[],
                               const GLfloat u[], const GLfloat lambda[],
                               GLubyte rgba[][4] )
{
   const struct gl_texture_image *img = tObj->Image[tObj->BaseLevel];
   const GLfloat width = (GLfloat) img->Width;
   const GLfloat height = (GLfloat) img->Height;
   const GLint colMask = img->Width - 1;
   const GLint rowMask = img->Height - 1;
   const GLint shift = img->WidthLog2;
   GLuint k;
   (void) u;
   (void) lambda;
   ASSERT(tObj->WrapS==GL_REPEAT);
   ASSERT(tObj->WrapT==GL_REPEAT);
   ASSERT(tObj->MinFilter==GL_NEAREST);
   ASSERT(tObj->MagFilter==GL_NEAREST);
   ASSERT(img->Border==0);
   ASSERT(img->Format==GL_RGB);

   /* NOTE: negative float->int doesn't floor, add 10000 as to work-around */
   for (k=0;k<n;k++) {
      GLint i = (GLint) ((s[k] + 10000.0) * width) & colMask;
      GLint j = (GLint) ((t[k] + 10000.0) * height) & rowMask;
      GLint pos = (j << shift) | i;
      GLubyte *texel = img->Data + pos + pos + pos;  /* pos*3 */
      rgba[k][RCOMP] = texel[0];
      rgba[k][GCOMP] = texel[1];
      rgba[k][BCOMP] = texel[2];
   }
}


/*
 * Optimized 2-D texture sampling:
 *    S and T wrap mode == GL_REPEAT
 *    GL_NEAREST min/mag filter
 *    No border
 *    Format = GL_RGBA
 */
static void opt_sample_rgba_2d( const struct gl_texture_object *tObj,
                                GLuint n, const GLfloat s[], const GLfloat t[],
                                const GLfloat u[], const GLfloat lambda[],
                                GLubyte rgba[][4] )
{
   const struct gl_texture_image *img = tObj->Image[tObj->BaseLevel];
   const GLfloat width = (GLfloat) img->Width;
   const GLfloat height = (GLfloat) img->Height;
   const GLint colMask = img->Width - 1;
   const GLint rowMask = img->Height - 1;
   const GLint shift = img->WidthLog2;
   GLuint k;
   (void) u;
   (void) lambda;
   ASSERT(tObj->WrapS==GL_REPEAT);
   ASSERT(tObj->WrapT==GL_REPEAT);
   ASSERT(tObj->MinFilter==GL_NEAREST);
   ASSERT(tObj->MagFilter==GL_NEAREST);
   ASSERT(img->Border==0);
   ASSERT(img->Format==GL_RGBA);

   /* NOTE: negative float->int doesn't floor, add 10000 as to work-around */
   for (k=0;k<n;k++) {
      GLint i = (GLint) ((s[k] + 10000.0) * width) & colMask;
      GLint j = (GLint) ((t[k] + 10000.0) * height) & rowMask;
      GLint pos = (j << shift) | i;
      GLubyte *texel = img->Data + (pos << 2);    /* pos*4 */
      rgba[k][RCOMP] = texel[0];
      rgba[k][GCOMP] = texel[1];
      rgba[k][BCOMP] = texel[2];
      rgba[k][ACOMP] = texel[3];
   }
}



/**********************************************************************/
/*                    3-D Texture Sampling Functions                  */
/**********************************************************************/

/*
 * Given a texture image and an (i,j,k) integer texel coordinate, return the
 * texel color.
 */
static void get_3d_texel( const struct gl_texture_object *tObj,
                          const struct gl_texture_image *img,
                          GLint i, GLint j, GLint k,
                          GLubyte rgba[4] )
{
   const GLint width = img->Width;    /* includes border */
   const GLint height = img->Height;  /* includes border */
   const GLint rectarea = width * height;
   const GLubyte *texel;

#ifdef DEBUG
   const GLint depth = img->Depth;    /* includes border */
   assert(i >= 0);
   assert(i < width);
   assert(j >= 0);
   assert(j < height);
   assert(k >= 0);
   assert(k < depth);
#endif

   switch (img->Format) {
      case GL_COLOR_INDEX:
         {
            GLubyte index = img->Data[ rectarea * k +  width * j + i ];
            palette_sample(tObj, index, rgba );
            return;
         }
      case GL_ALPHA:
         rgba[ACOMP] = img->Data[ rectarea * k +  width * j + i ];
         return;
      case GL_LUMINANCE:
      case GL_INTENSITY:
         rgba[RCOMP] = img->Data[ rectarea * k +  width * j + i ];
         return;
      case GL_LUMINANCE_ALPHA:
         texel = img->Data + ( rectarea * k + width * j + i) * 2;
         rgba[RCOMP] = texel[0];
         rgba[ACOMP] = texel[1];
         return;
      case GL_RGB:
         texel = img->Data + (rectarea * k + width * j + i) * 3;
         rgba[RCOMP] = texel[0];
         rgba[GCOMP] = texel[1];
         rgba[BCOMP] = texel[2];
         return;
      case GL_RGBA:
         texel = img->Data + (rectarea * k + width * j + i) * 4;
         rgba[RCOMP] = texel[0];
         rgba[GCOMP] = texel[1];
         rgba[BCOMP] = texel[2];
         rgba[ACOMP] = texel[3];
         return;
      default:
         gl_problem(NULL, "Bad format in get_3d_texel");
   }
}


/*
 * Return the texture sample for coordinate (s,t,r) using GL_NEAREST filter.
 */
static void sample_3d_nearest( const struct gl_texture_object *tObj,
                               const struct gl_texture_image *img,
                               GLfloat s, GLfloat t, GLfloat r,
                               GLubyte rgba[4] )
{
   const GLint imgWidth = img->Width;   /* includes border, if any */
   const GLint imgHeight = img->Height; /* includes border, if any */
   const GLint width = img->Width2;     /* without border, power of two */
   const GLint height = img->Height2;   /* without border, power of two */
   const GLint depth = img->Depth2;     /* without border, power of two */
   const GLint rectarea = imgWidth * imgHeight;
   const GLubyte *texel;
   GLint i, j, k;

   COMPUTE_NEAREST_TEXEL_LOCATION(tObj->WrapS, s, width,  i);
   COMPUTE_NEAREST_TEXEL_LOCATION(tObj->WrapT, t, height, j);
   COMPUTE_NEAREST_TEXEL_LOCATION(tObj->WrapR, r, depth,  k);

   switch (tObj->Image[0]->Format) {
      case GL_COLOR_INDEX:
         {
            GLubyte index = img->Data[ rectarea * k + j * imgWidth + i ];
            palette_sample(tObj, index, rgba );
            return;
         }
      case GL_ALPHA:
         rgba[ACOMP] = img->Data[ rectarea * k + j * imgWidth + i ];
         return;
      case GL_LUMINANCE:
      case GL_INTENSITY:
         rgba[RCOMP] = img->Data[ rectarea * k + j * imgWidth + i ];
         return;
      case GL_LUMINANCE_ALPHA:
         texel = img->Data + ((rectarea * k + j * imgWidth + i) << 1);
         rgba[RCOMP] = texel[0];
         rgba[ACOMP] = texel[1];
         return;
      case GL_RGB:
         texel = img->Data + ( rectarea * k + j * imgWidth + i) * 3;
         rgba[RCOMP] = texel[0];
         rgba[GCOMP] = texel[1];
         rgba[BCOMP] = texel[2];
         return;
      case GL_RGBA:
         texel = img->Data + ((rectarea * k + j * imgWidth + i) << 2);
         rgba[RCOMP] = texel[0];
         rgba[GCOMP] = texel[1];
         rgba[BCOMP] = texel[2];
         rgba[ACOMP] = texel[3];
         return;
      default:
         gl_problem(NULL, "Bad format in sample_3d_nearest");
   }
}



/*
 * Return the texture sample for coordinate (s,t,r) using GL_LINEAR filter.
 */
static void sample_3d_linear( const struct gl_texture_object *tObj,
                              const struct gl_texture_image *img,
                              GLfloat s, GLfloat t, GLfloat r,
                              GLubyte rgba[4] )
{
   const GLint width = img->Width2;
   const GLint height = img->Height2;
   const GLint depth = img->Depth2;
   GLint i0, j0, k0, i1, j1, k1;
   GLuint useBorderColor;
   GLfloat u, v, w;

   COMPUTE_LINEAR_TEXEL_LOCATIONS(tObj->WrapS, s, u, width,  i0, i1);
   COMPUTE_LINEAR_TEXEL_LOCATIONS(tObj->WrapT, t, v, height, j0, j1);
   COMPUTE_LINEAR_TEXEL_LOCATIONS(tObj->WrapR, r, w, depth,  k0, k1);

   useBorderColor = 0;
   if (img->Border) {
      i0 += img->Border;
      i1 += img->Border;
      j0 += img->Border;
      j1 += img->Border;
      k0 += img->Border;
      k1 += img->Border;
   }
   else {
      /* check if sampling texture border color */
      if (i0 < 0 || i0 >= width)   useBorderColor |= I0BIT;
      if (i1 < 0 || i1 >= width)   useBorderColor |= I1BIT;
      if (j0 < 0 || j0 >= height)  useBorderColor |= J0BIT;
      if (j1 < 0 || j1 >= height)  useBorderColor |= J1BIT;
      if (k0 < 0 || k0 >= depth)   useBorderColor |= K0BIT;
      if (k1 < 0 || k1 >= depth)   useBorderColor |= K1BIT;
   }

   {
      const GLfloat a = myFrac(u);
      const GLfloat b = myFrac(v);
      const GLfloat c = myFrac(w);
      /* compute sample weights in fixed point in [0,WEIGHT_SCALE] */
      GLint w000 = (GLint) ((1.0F-a)*(1.0F-b)*(1.0F-c) * WEIGHT_SCALE + 0.5F);
      GLint w100 = (GLint) (      a *(1.0F-b)*(1.0F-c) * WEIGHT_SCALE + 0.5F);
      GLint w010 = (GLint) ((1.0F-a)*      b *(1.0F-c) * WEIGHT_SCALE + 0.5F);
      GLint w110 = (GLint) (      a *      b *(1.0F-c) * WEIGHT_SCALE + 0.5F);
      GLint w001 = (GLint) ((1.0F-a)*(1.0F-b)*      c  * WEIGHT_SCALE + 0.5F);
      GLint w101 = (GLint) (      a *(1.0F-b)*      c  * WEIGHT_SCALE + 0.5F);
      GLint w011 = (GLint) ((1.0F-a)*      b *      c  * WEIGHT_SCALE + 0.5F);
      GLint w111 = (GLint) (      a *      b *      c  * WEIGHT_SCALE + 0.5F);

      GLubyte t000[4], t010[4], t001[4], t011[4];
      GLubyte t100[4], t110[4], t101[4], t111[4];

      if (useBorderColor & (I0BIT | J0BIT | K0BIT)) {
         COPY_4UBV(t000, tObj->BorderColor);
      }
      else {
         get_3d_texel( tObj, img, i0, j0, k0, t000 );
      }
      if (useBorderColor & (I1BIT | J0BIT | K0BIT)) {
         COPY_4UBV(t100, tObj->BorderColor);
      }
      else {
         get_3d_texel( tObj, img, i1, j0, k0, t100 );
      }
      if (useBorderColor & (I0BIT | J1BIT | K0BIT)) {
         COPY_4UBV(t010, tObj->BorderColor);
      }
      else {
         get_3d_texel( tObj, img, i0, j1, k0, t010 );
      }
      if (useBorderColor & (I1BIT | J1BIT | K0BIT)) {
         COPY_4UBV(t110, tObj->BorderColor);
      }
      else {
         get_3d_texel( tObj, img, i1, j1, k0, t110 );
      }

      if (useBorderColor & (I0BIT | J0BIT | K1BIT)) {
         COPY_4UBV(t001, tObj->BorderColor);
      }
      else {
         get_3d_texel( tObj, img, i0, j0, k1, t001 );
      }
      if (useBorderColor & (I1BIT | J0BIT | K1BIT)) {
         COPY_4UBV(t101, tObj->BorderColor);
      }
      else {
         get_3d_texel( tObj, img, i1, j0, k1, t101 );
      }
      if (useBorderColor & (I0BIT | J1BIT | K1BIT)) {
         COPY_4UBV(t011, tObj->BorderColor);
      }
      else {
         get_3d_texel( tObj, img, i0, j1, k1, t011 );
      }
      if (useBorderColor & (I1BIT | J1BIT | K1BIT)) {
         COPY_4UBV(t111, tObj->BorderColor);
      }
      else {
         get_3d_texel( tObj, img, i1, j1, k1, t111 );
      }

      rgba[0] = (GLubyte) (
                 (w000*t000[0] + w010*t010[0] + w001*t001[0] + w011*t011[0] +
                  w100*t100[0] + w110*t110[0] + w101*t101[0] + w111*t111[0]  )
                 >> WEIGHT_SHIFT);
      rgba[1] = (GLubyte) (
                 (w000*t000[1] + w010*t010[1] + w001*t001[1] + w011*t011[1] +
                  w100*t100[1] + w110*t110[1] + w101*t101[1] + w111*t111[1] )
                 >> WEIGHT_SHIFT);
      rgba[2] = (GLubyte) (
                 (w000*t000[2] + w010*t010[2] + w001*t001[2] + w011*t011[2] +
                  w100*t100[2] + w110*t110[2] + w101*t101[2] + w111*t111[2] )
                 >> WEIGHT_SHIFT);
      rgba[3] = (GLubyte) (
                 (w000*t000[3] + w010*t010[3] + w001*t001[3] + w011*t011[3] +
                  w100*t100[3] + w110*t110[3] + w101*t101[3] + w111*t111[3] )
                 >> WEIGHT_SHIFT);
   }
}



static void
sample_3d_nearest_mipmap_nearest( const struct gl_texture_object *tObj,
                                  GLfloat s, GLfloat t, GLfloat r,
                                  GLfloat lambda, GLubyte rgba[4] )
{
   GLint level;
   COMPUTE_NEAREST_MIPMAP_LEVEL(tObj, lambda, level);
   sample_3d_nearest( tObj, tObj->Image[level], s, t, r, rgba );
}


static void
sample_3d_linear_mipmap_nearest( const struct gl_texture_object *tObj,
                                 GLfloat s, GLfloat t, GLfloat r,
                                 GLfloat lambda, GLubyte rgba[4] )
{
   GLint level;
   COMPUTE_NEAREST_MIPMAP_LEVEL(tObj, lambda, level);
   sample_3d_linear( tObj, tObj->Image[level], s, t, r, rgba );
}


static void
sample_3d_nearest_mipmap_linear( const struct gl_texture_object *tObj,
                                 GLfloat s, GLfloat t, GLfloat r,
                                 GLfloat lambda, GLubyte rgba[4] )
{
   GLint level;

   COMPUTE_LINEAR_MIPMAP_LEVEL(tObj, lambda, level);

   if (level >= tObj->P) {
      sample_3d_nearest( tObj, tObj->Image[tObj->P], s, t, r, rgba );
   }
   else {
      GLubyte t0[4], t1[4];  /* texels */
      const GLfloat f = myFrac(lambda);
      sample_3d_nearest( tObj, tObj->Image[level  ], s, t, r, t0 );
      sample_3d_nearest( tObj, tObj->Image[level+1], s, t, r, t1 );
      rgba[RCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[RCOMP] + f * t1[RCOMP]);
      rgba[GCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[GCOMP] + f * t1[GCOMP]);
      rgba[BCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[BCOMP] + f * t1[BCOMP]);
      rgba[ACOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[ACOMP] + f * t1[ACOMP]);
   }
}


static void
sample_3d_linear_mipmap_linear( const struct gl_texture_object *tObj,
                                GLfloat s, GLfloat t, GLfloat r,
                                GLfloat lambda, GLubyte rgba[4] )
{
   GLint level;

   COMPUTE_LINEAR_MIPMAP_LEVEL(tObj, lambda, level);

   if (level >= tObj->P) {
      sample_3d_linear( tObj, tObj->Image[tObj->P], s, t, r, rgba );
   }
   else {
      GLubyte t0[4], t1[4];  /* texels */
      const GLfloat f = myFrac(lambda);
      sample_3d_linear( tObj, tObj->Image[level  ], s, t, r, t0 );
      sample_3d_linear( tObj, tObj->Image[level+1], s, t, r, t1 );
      rgba[RCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[RCOMP] + f * t1[RCOMP]);
      rgba[GCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[GCOMP] + f * t1[GCOMP]);
      rgba[BCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[BCOMP] + f * t1[BCOMP]);
      rgba[ACOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[ACOMP] + f * t1[ACOMP]);
   }
}


static void sample_nearest_3d( const struct gl_texture_object *tObj, GLuint n,
                               const GLfloat s[], const GLfloat t[],
                               const GLfloat u[], const GLfloat lambda[],
                               GLubyte rgba[][4] )
{
   GLuint i;
   struct gl_texture_image *image = tObj->Image[tObj->BaseLevel];
   (void) lambda;
   for (i=0;i<n;i++) {
      sample_3d_nearest( tObj, image, s[i], t[i], u[i], rgba[i] );
   }
}



static void sample_linear_3d( const struct gl_texture_object *tObj, GLuint n,
                              const GLfloat s[], const GLfloat t[],
                              const GLfloat u[], const GLfloat lambda[],
                              GLubyte rgba[][4] )
{
   GLuint i;
   struct gl_texture_image *image = tObj->Image[tObj->BaseLevel];
   (void) lambda;
   for (i=0;i<n;i++) {
      sample_3d_linear( tObj, image, s[i], t[i], u[i], rgba[i] );
   }
}


/*
 * Given an (s,t,r) texture coordinate and lambda (level of detail) value,
 * return a texture sample.
 */
static void sample_lambda_3d( const struct gl_texture_object *tObj, GLuint n,
                              const GLfloat s[], const GLfloat t[],
                              const GLfloat u[], const GLfloat lambda[],
                              GLubyte rgba[][4] )
{
   GLuint i;

   for (i=0;i<n;i++) {

      if (lambda[i] > tObj->MinMagThresh) {
         /* minification */
         switch (tObj->MinFilter) {
            case GL_NEAREST:
               sample_3d_nearest( tObj, tObj->Image[tObj->BaseLevel], s[i], t[i], u[i], rgba[i] );
               break;
            case GL_LINEAR:
               sample_3d_linear( tObj, tObj->Image[tObj->BaseLevel], s[i], t[i], u[i], rgba[i] );
               break;
            case GL_NEAREST_MIPMAP_NEAREST:
               sample_3d_nearest_mipmap_nearest( tObj, s[i], t[i], u[i], lambda[i], rgba[i] );
               break;
            case GL_LINEAR_MIPMAP_NEAREST:
               sample_3d_linear_mipmap_nearest( tObj, s[i], t[i], u[i], lambda[i], rgba[i] );
               break;
            case GL_NEAREST_MIPMAP_LINEAR:
               sample_3d_nearest_mipmap_linear( tObj, s[i], t[i], u[i], lambda[i], rgba[i] );
               break;
            case GL_LINEAR_MIPMAP_LINEAR:
               sample_3d_linear_mipmap_linear( tObj, s[i], t[i], u[i], lambda[i], rgba[i] );
               break;
            default:
               gl_problem(NULL, "Bad min filterin sample_3d_texture");
         }
      }
      else {
         /* magnification */
         switch (tObj->MagFilter) {
            case GL_NEAREST:
               sample_3d_nearest( tObj, tObj->Image[tObj->BaseLevel], s[i], t[i], u[i], rgba[i] );
               break;
            case GL_LINEAR:
               sample_3d_linear( tObj, tObj->Image[tObj->BaseLevel], s[i], t[i], u[i], rgba[i] );
               break;
            default:
               gl_problem(NULL, "Bad mag filter in sample_3d_texture");
         }
      }
   }
}


/**********************************************************************/
/*                Texture Cube Map Sampling Functions                 */
/**********************************************************************/

/*
 * Choose one of six sides of a texture cube map given the texture
 * coord (rx,ry,rz).  Return pointer to corresponding array of texture
 * images.
 */
static const struct gl_texture_image **
choose_cube_face(const struct gl_texture_object *texObj,
                 GLfloat rx, GLfloat ry, GLfloat rz,
                 GLfloat *newS, GLfloat *newT)
{
/*
      major axis
      direction     target                             sc     tc    ma
      ----------    -------------------------------    ---    ---   ---
       +rx          TEXTURE_CUBE_MAP_POSITIVE_X_EXT    -rz    -ry   rx
       -rx          TEXTURE_CUBE_MAP_NEGATIVE_X_EXT    +rz    -ry   rx
       +ry          TEXTURE_CUBE_MAP_POSITIVE_Y_EXT    +rx    +rz   ry
       -ry          TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT    +rx    -rz   ry
       +rz          TEXTURE_CUBE_MAP_POSITIVE_Z_EXT    +rx    -ry   rz
       -rz          TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT    -rx    -ry   rz
*/
   const struct gl_texture_image **imgArray;
   const GLfloat arx = ABSF(rx),   ary = ABSF(ry),   arz = ABSF(rz);
   GLfloat sc, tc, ma;

   if (arx > ary && arx > arz) {
      if (rx >= 0.0F) {
         imgArray = (const struct gl_texture_image **) texObj->Image;
         sc = -rz;
         tc = -ry;
         ma = arx;
      }
      else {
         imgArray = (const struct gl_texture_image **) texObj->NegX;
         sc = rz;
         tc = -ry;
         ma = arx;
      }
   }
   else if (ary > arx && ary > arz) {
      if (ry >= 0.0F) {
         imgArray = (const struct gl_texture_image **) texObj->PosY;
         sc = rx;
         tc = rz;
         ma = ary;
      }
      else {
         imgArray = (const struct gl_texture_image **) texObj->NegY;
         sc = rx;
         tc = -rz;
         ma = ary;
      }
   }
   else {
      if (rz > 0.0F) {
         imgArray = (const struct gl_texture_image **) texObj->PosZ;
         sc = rx;
         tc = -ry;
         ma = arz;
      }
      else {
         imgArray = (const struct gl_texture_image **) texObj->NegZ;
         sc = -rx;
         tc = -ry;
         ma = arz;
      }
   }

   *newS = ( sc / ma + 1.0F ) * 0.5F;
   *newT = ( tc / ma + 1.0F ) * 0.5F;
   return imgArray;
}


static void
sample_nearest_cube(const struct gl_texture_object *tObj, GLuint n,
                    const GLfloat s[], const GLfloat t[],
                    const GLfloat u[], const GLfloat lambda[],
                    GLubyte rgba[][4])
{
   GLuint i;
   (void) lambda;
   for (i = 0; i < n; i++) {
      const struct gl_texture_image **images;
      GLfloat newS, newT;
      images = choose_cube_face(tObj, s[i], t[i], u[i], &newS, &newT);
      sample_2d_nearest( tObj, images[tObj->BaseLevel], newS, newT, rgba[i] );
   }
}


static void
sample_linear_cube(const struct gl_texture_object *tObj, GLuint n,
                   const GLfloat s[], const GLfloat t[],
                   const GLfloat u[], const GLfloat lambda[],
                   GLubyte rgba[][4])
{
   GLuint i;
   (void) lambda;
   for (i = 0; i < n; i++) {
      const struct gl_texture_image **images;
      GLfloat newS, newT;
      images = choose_cube_face(tObj, s[i], t[i], u[i], &newS, &newT);
      sample_2d_linear( tObj, images[tObj->BaseLevel], newS, newT, rgba[i] );
   }
}


static void
sample_cube_nearest_mipmap_nearest( const struct gl_texture_object *tObj,
                                    GLfloat s, GLfloat t, GLfloat u,
                                    GLfloat lambda, GLubyte rgba[4] )
{
   const struct gl_texture_image **images;
   GLfloat newS, newT;
   GLint level;

   COMPUTE_NEAREST_MIPMAP_LEVEL(tObj, lambda, level);

   images = choose_cube_face(tObj, s, t, u, &newS, &newT);
   sample_2d_nearest( tObj, images[level], newS, newT, rgba );
}


static void
sample_cube_linear_mipmap_nearest( const struct gl_texture_object *tObj,
                                   GLfloat s, GLfloat t, GLfloat u,
                                   GLfloat lambda, GLubyte rgba[4] )
{
   const struct gl_texture_image **images;
   GLfloat newS, newT;
   GLint level;

   COMPUTE_NEAREST_MIPMAP_LEVEL(tObj, lambda, level);

   images = choose_cube_face(tObj, s, t, u, &newS, &newT);
   sample_2d_linear( tObj, images[level], newS, newT, rgba );
}


static void
sample_cube_nearest_mipmap_linear( const struct gl_texture_object *tObj,
                                   GLfloat s, GLfloat t, GLfloat u,
                                   GLfloat lambda, GLubyte rgba[4] )
{
   const struct gl_texture_image **images;
   GLfloat newS, newT;
   GLint level;

   COMPUTE_LINEAR_MIPMAP_LEVEL(tObj, lambda, level);

   images = choose_cube_face(tObj, s, t, u, &newS, &newT);

   if (level >= tObj->P) {
      sample_2d_nearest( tObj, images[tObj->P], newS, newT, rgba );
   }
   else {
      GLubyte t0[4], t1[4];  /* texels */
      const GLfloat f = myFrac(lambda);
      sample_2d_nearest( tObj, images[level  ], newS, newT, t0 );
      sample_2d_nearest( tObj, images[level+1], newS, newT, t1 );
      rgba[RCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[RCOMP] + f * t1[RCOMP]);
      rgba[GCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[GCOMP] + f * t1[GCOMP]);
      rgba[BCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[BCOMP] + f * t1[BCOMP]);
      rgba[ACOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[ACOMP] + f * t1[ACOMP]);
   }
}


static void
sample_cube_linear_mipmap_linear( const struct gl_texture_object *tObj,
                                  GLfloat s, GLfloat t, GLfloat u,
                                  GLfloat lambda, GLubyte rgba[4] )
{
   const struct gl_texture_image **images;
   GLfloat newS, newT;
   GLint level;

   COMPUTE_LINEAR_MIPMAP_LEVEL(tObj, lambda, level);

   images = choose_cube_face(tObj, s, t, u, &newS, &newT);

   if (level >= tObj->P) {
      sample_2d_linear( tObj, images[tObj->P], newS, newT, rgba );
   }
   else {
      GLubyte t0[4], t1[4];
      const GLfloat f = myFrac(lambda);
      sample_2d_linear( tObj, images[level  ], newS, newT, t0 );
      sample_2d_linear( tObj, images[level+1], newS, newT, t1 );
      rgba[RCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[RCOMP] + f * t1[RCOMP]);
      rgba[GCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[GCOMP] + f * t1[GCOMP]);
      rgba[BCOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[BCOMP] + f * t1[BCOMP]);
      rgba[ACOMP] = (GLubyte) (GLint) ((1.0F-f) * t0[ACOMP] + f * t1[ACOMP]);
   }
}


static void
sample_lambda_cube(const struct gl_texture_object *tObj, GLuint n,
                   const GLfloat s[], const GLfloat t[],
                   const GLfloat u[], const GLfloat lambda[],
                   GLubyte rgba[][4])
{
   GLuint i;

   for (i = 0; i < n; i++) {
      if (lambda[i] > tObj->MinMagThresh) {
         /* minification */
         switch (tObj->MinFilter) {
            case GL_NEAREST:
               {
                  const struct gl_texture_image **images;
                  GLfloat newS, newT;
                  images = choose_cube_face(tObj, s[i], t[i], u[i],
                                            &newS, &newT);
                  sample_2d_nearest( tObj, images[tObj->BaseLevel],
                                     newS, newT, rgba[i] );
               }
               break;
            case GL_LINEAR:
               {
                  const struct gl_texture_image **images;
                  GLfloat newS, newT;
                  images = choose_cube_face(tObj, s[i], t[i], u[i],
                                            &newS, &newT);
                  sample_2d_linear( tObj, images[tObj->BaseLevel],
                                    newS, newT, rgba[i] );
               }
               break;
            case GL_NEAREST_MIPMAP_NEAREST:
               sample_cube_nearest_mipmap_nearest( tObj, s[i], t[i], u[i],
                                                   lambda[i], rgba[i] );
               break;
            case GL_LINEAR_MIPMAP_NEAREST:
               sample_cube_linear_mipmap_nearest( tObj, s[i], t[i], u[i],
                                                  lambda[i], rgba[i] );
               break;
            case GL_NEAREST_MIPMAP_LINEAR:
               sample_cube_nearest_mipmap_linear( tObj, s[i], t[i], u[i],
                                                  lambda[i], rgba[i] );
               break;
            case GL_LINEAR_MIPMAP_LINEAR:
               sample_cube_linear_mipmap_linear( tObj, s[i], t[i], u[i],
                                                 lambda[i], rgba[i] );
               break;
            default:
               gl_problem(NULL, "Bad min filter in sample_lambda_cube");
         }
      }
      else {
         /* magnification */
         const struct gl_texture_image **images;
         GLfloat newS, newT;
         images = choose_cube_face(tObj, s[i], t[i], u[i],
                                   &newS, &newT);
         switch (tObj->MagFilter) {
            case GL_NEAREST:
               sample_2d_nearest( tObj, images[tObj->BaseLevel],
                                  newS, newT, rgba[i] );
               break;
            case GL_LINEAR:
               sample_2d_linear( tObj, images[tObj->BaseLevel],
                                 newS, newT, rgba[i] );
               break;
            default:
               gl_problem(NULL, "Bad mag filter in sample_lambda_cube");
         }
      }
   }
}


/**********************************************************************/
/*                       Texture Sampling Setup                       */
/**********************************************************************/


/*
 * Setup the texture sampling function for this texture object.
 */
void
_mesa_set_texture_sampler( struct gl_texture_object *t )
{
   if (!t->Complete) {
      t->SampleFunc = NULL;
   }
   else {
      GLboolean needLambda = (GLboolean) (t->MinFilter != t->MagFilter);

      if (needLambda) {
         /* Compute min/mag filter threshold */
         if (t->MagFilter==GL_LINEAR
             && (t->MinFilter==GL_NEAREST_MIPMAP_NEAREST ||
                 t->MinFilter==GL_LINEAR_MIPMAP_NEAREST)) {
            t->MinMagThresh = 0.5F;
         }
         else {
            t->MinMagThresh = 0.0F;
         }
      }

      switch (t->Dimensions) {
         case 1:
            if (needLambda) {
               t->SampleFunc = sample_lambda_1d;
            }
            else if (t->MinFilter==GL_LINEAR) {
               t->SampleFunc = sample_linear_1d;
            }
            else {
               ASSERT(t->MinFilter==GL_NEAREST);
               t->SampleFunc = sample_nearest_1d;
            }
            break;
         case 2:
            if (needLambda) {
               t->SampleFunc = sample_lambda_2d;
            }
            else if (t->MinFilter==GL_LINEAR) {
               t->SampleFunc = sample_linear_2d;
            }
            else {
               ASSERT(t->MinFilter==GL_NEAREST);
               if (t->WrapS==GL_REPEAT && t->WrapT==GL_REPEAT
                   && t->Image[0]->Border==0 && t->Image[0]->Format==GL_RGB) {
                  t->SampleFunc = opt_sample_rgb_2d;
               }
               else if (t->WrapS==GL_REPEAT && t->WrapT==GL_REPEAT
                   && t->Image[0]->Border==0 && t->Image[0]->Format==GL_RGBA) {
                  t->SampleFunc = opt_sample_rgba_2d;
               }
               else
                  t->SampleFunc = sample_nearest_2d;
            }
            break;
         case 3:
            if (needLambda) {
               t->SampleFunc = sample_lambda_3d;
            }
            else if (t->MinFilter==GL_LINEAR) {
               t->SampleFunc = sample_linear_3d;
            }
            else {
               ASSERT(t->MinFilter==GL_NEAREST);
               t->SampleFunc = sample_nearest_3d;
            }
            break;
         case 6: /* cube map */
            if (needLambda) {
               t->SampleFunc = sample_lambda_cube;
            }
            else if (t->MinFilter==GL_LINEAR) {
               t->SampleFunc = sample_linear_cube;
            }
            else {
               ASSERT(t->MinFilter==GL_NEAREST);
               t->SampleFunc = sample_nearest_cube;
            }
            break;
         default:
            gl_problem(NULL, "invalid dimensions in _mesa_set_texture_sampler");
      }
   }
}


#define PROD(A,B)   ( (GLuint)(A) * ((GLuint)(B)+1) )
#define S_PROD(A,B) ( (GLint)(A) * ((GLint)(B)+1) )

static INLINE void
_mesa_texture_combine(const GLcontext *ctx,
                      const struct gl_texture_unit *textureUnit,
                      GLuint n,
                      GLubyte (*primary_rgba)[4],
                      GLubyte (*texel)[4],
                      GLubyte (*rgba)[4])
{
   GLubyte ccolor [3][3*MAX_WIDTH][4];
   GLubyte (*argRGB [3])[4];
   GLubyte (*argA [3])[4];
   GLuint i, j;
   GLuint RGBshift = textureUnit->CombineScaleShiftRGB;
   GLuint Ashift   = textureUnit->CombineScaleShiftA;

   ASSERT(ctx->Extensions.HaveTextureEnvCombine);

   for (j = 0; j < 3; j++) {
      switch (textureUnit->CombineSourceA[j]) {
         case GL_TEXTURE:
            argA[j] = texel;
            break;
         case GL_PRIMARY_COLOR_EXT:
            argA[j] = primary_rgba;
            break;
         case GL_PREVIOUS_EXT:
            argA[j] = rgba;
            break;
         case GL_CONSTANT_EXT:
            {
               GLubyte (*c)[4] = ccolor[j];
               GLubyte alpha = FLOAT_TO_UBYTE(textureUnit->EnvColor[3]);
               for (i = 0; i < n; i++)
                  c[i][ACOMP] = alpha;
               argA[j] = ccolor[j];
            }
            break;
         default:
            gl_problem(NULL, "invalid combine source");
      }

      switch (textureUnit->CombineSourceRGB[j]) {
         case GL_TEXTURE:
            argRGB[j] = texel;
            break;
         case GL_PRIMARY_COLOR_EXT:
            argRGB[j] = primary_rgba;
            break;
         case GL_PREVIOUS_EXT:
            argRGB[j] = rgba;
            break;
         case GL_CONSTANT_EXT:
            {
               GLubyte (*c)[4] = ccolor[j];
               const GLubyte red   = FLOAT_TO_UBYTE(textureUnit->EnvColor[0]);
               const GLubyte green = FLOAT_TO_UBYTE(textureUnit->EnvColor[1]);
               const GLubyte blue  = FLOAT_TO_UBYTE(textureUnit->EnvColor[2]);
               const GLubyte alpha = FLOAT_TO_UBYTE(textureUnit->EnvColor[3]);
               for (i = 0; i < n; i++) {
                  c[i][RCOMP] = red;
                  c[i][GCOMP] = green;
                  c[i][BCOMP] = blue;
                  c[i][ACOMP] = alpha;
               }
               argRGB[j] = ccolor[j];
            }
            break;
         default:
            gl_problem(NULL, "invalid combine source");
      }

      if (textureUnit->CombineOperandRGB[j] != GL_SRC_COLOR) {
         GLubyte (*src)[4] = argRGB[j];
         GLubyte (*dst)[4] = ccolor[j];

         /* point to new arg[j] storage */
         argRGB[j] = ccolor[j];

         if (textureUnit->CombineOperandRGB[j] == GL_ONE_MINUS_SRC_COLOR) {
            for (i = 0; i < n; i++) {
               dst[i][RCOMP] = 255 - src[i][RCOMP];
               dst[i][GCOMP] = 255 - src[i][GCOMP];
               dst[i][BCOMP] = 255 - src[i][BCOMP];
            }
         }
         else if (textureUnit->CombineOperandRGB[j] == GL_SRC_ALPHA) {
            for (i = 0; i < n; i++) {
               dst[i][RCOMP] = src[i][ACOMP];
               dst[i][GCOMP] = src[i][ACOMP];
               dst[i][BCOMP] = src[i][ACOMP];
            }
         }
         else {                      /*  GL_ONE_MINUS_SRC_ALPHA  */
            for (i = 0; i < n; i++) {
               dst[i][RCOMP] = 255 - src[i][ACOMP];
               dst[i][GCOMP] = 255 - src[i][ACOMP];
               dst[i][BCOMP] = 255 - src[i][ACOMP];
            }
         }
      }

      if (textureUnit->CombineOperandA[j] == GL_ONE_MINUS_SRC_ALPHA) {
         GLubyte (*src)[4] = argA[j];
         GLubyte (*dst)[4] = ccolor[j];
         argA[j] = ccolor[j];
         for (i = 0; i < n; i++) {
            dst[i][ACOMP] = 255 - src[i][ACOMP];
         }
      }

      if (textureUnit->CombineModeRGB == GL_REPLACE &&
          textureUnit->CombineModeA == GL_REPLACE) {
         break;      /*  done, we need only arg0  */
      }

      if (j == 1 &&
          textureUnit->CombineModeRGB != GL_INTERPOLATE_EXT &&
          textureUnit->CombineModeA != GL_INTERPOLATE_EXT) {
         break;      /*  arg0 and arg1 are done. we don't need arg2. */
      }
   }

   switch (textureUnit->CombineModeRGB) {
      case GL_REPLACE:
         {
            const GLubyte (*arg0)[4] = (const GLubyte (*)[4]) argRGB[0];
            if (RGBshift) {
               for (i = 0; i < n; i++) {
                  GLuint r = (GLuint) arg0[i][RCOMP] << RGBshift;
                  GLuint g = (GLuint) arg0[i][GCOMP] << RGBshift;
                  GLuint b = (GLuint) arg0[i][BCOMP] << RGBshift;
                  rgba[i][RCOMP] = MIN2(r, 255);
                  rgba[i][GCOMP] = MIN2(g, 255);
                  rgba[i][BCOMP] = MIN2(b, 255);
               }
            }
            else {
               for (i = 0; i < n; i++) {
                  rgba[i][RCOMP] = arg0[i][RCOMP];
                  rgba[i][GCOMP] = arg0[i][GCOMP];
                  rgba[i][BCOMP] = arg0[i][BCOMP];
               }
            }
         }
         break;
      case GL_MODULATE:
         {
            const GLubyte (*arg0)[4] = (const GLubyte (*)[4]) argRGB[0];
            const GLubyte (*arg1)[4] = (const GLubyte (*)[4]) argRGB[1];
            RGBshift = 8 - RGBshift;
            for (i = 0; i < n; i++) {
               GLuint r = PROD(arg0[i][0], arg1[i][RCOMP]) >> RGBshift;
               GLuint g = PROD(arg0[i][1], arg1[i][GCOMP]) >> RGBshift;
               GLuint b = PROD(arg0[i][2], arg1[i][BCOMP]) >> RGBshift;
               rgba[i][RCOMP] = (GLubyte) MIN2(r, 255);
               rgba[i][GCOMP] = (GLubyte) MIN2(g, 255);
               rgba[i][BCOMP] = (GLubyte) MIN2(b, 255);
            }
         }
         break;
      case GL_ADD:
         {
            const GLubyte (*arg0)[4] = (const GLubyte (*)[4]) argRGB[0];
            const GLubyte (*arg1)[4] = (const GLubyte (*)[4]) argRGB[1];
            for (i = 0; i < n; i++) {
               GLint r = ((GLuint) arg0[i][RCOMP] + arg1[i][RCOMP]) << RGBshift;
               GLint g = ((GLuint) arg0[i][GCOMP] + arg1[i][GCOMP]) << RGBshift;
               GLint b = ((GLuint) arg0[i][BCOMP] + arg1[i][BCOMP]) << RGBshift;
               rgba[i][RCOMP] = (GLubyte) MIN2(r, 255);
               rgba[i][GCOMP] = (GLubyte) MIN2(g, 255);
               rgba[i][BCOMP] = (GLubyte) MIN2(b, 255);
            }
         }
         break;
      case GL_ADD_SIGNED_EXT:
         {
            const GLubyte (*arg0)[4] = (const GLubyte (*)[4]) argRGB[0];
            const GLubyte (*arg1)[4] = (const GLubyte (*)[4]) argRGB[1];
            for (i = 0; i < n; i++) {
               GLint r = (GLint) arg0[i][RCOMP] + (GLint) arg1[i][RCOMP] - 128;
               GLint g = (GLint) arg0[i][GCOMP] + (GLint) arg1[i][GCOMP] - 128;
               GLint b = (GLint) arg0[i][BCOMP] + (GLint) arg1[i][BCOMP] - 128;
               r = (r < 0) ? 0 : r << RGBshift;
               b = (b < 0) ? 0 : b << RGBshift;
               g = (g < 0) ? 0 : g << RGBshift;
               rgba[i][RCOMP] = (GLubyte) MIN2(r, 255);
               rgba[i][GCOMP] = (GLubyte) MIN2(g, 255);
               rgba[i][BCOMP] = (GLubyte) MIN2(b, 255);
            }
         }
         break;
      case GL_INTERPOLATE_EXT:
         {
            const GLubyte (*arg0)[4] = (const GLubyte (*)[4]) argRGB[0];
            const GLubyte (*arg1)[4] = (const GLubyte (*)[4]) argRGB[1];
            const GLubyte (*arg2)[4] = (const GLubyte (*)[4]) argRGB[2];
            RGBshift = 8 - RGBshift;
            for (i = 0; i < n; i++) {
               GLuint r = (PROD(arg0[i][RCOMP], arg2[i][RCOMP])
                           + PROD(arg1[i][RCOMP], 255 - arg2[i][RCOMP]))
                              >> RGBshift;
               GLuint g = (PROD(arg0[i][GCOMP], arg2[i][GCOMP])
                           + PROD(arg1[i][GCOMP], 255 - arg2[i][GCOMP]))
                              >> RGBshift;
               GLuint b = (PROD(arg0[i][BCOMP], arg2[i][BCOMP])
                           + PROD(arg1[i][BCOMP], 255 - arg2[i][BCOMP]))
                              >> RGBshift;
               rgba[i][RCOMP] = (GLubyte) MIN2(r, 255);
               rgba[i][GCOMP] = (GLubyte) MIN2(g, 255);
               rgba[i][BCOMP] = (GLubyte) MIN2(b, 255);
            }
         }
	 break;
      case GL_DOT3_RGB_EXT:
      case GL_DOT3_RGBA_EXT:
         {
            const GLubyte (*arg0)[4] = (const GLubyte (*)[4]) argRGB[0];
            const GLubyte (*arg1)[4] = (const GLubyte (*)[4]) argRGB[1];
	    /* ATI's EXT extension has a constant scale by 4.  The ARB
	     * one will likely remove this restriction, and we should
	     * drop the EXT extension in favour of the ARB one.
	     */
            RGBshift = 6;
            for (i = 0; i < n; i++) {
               GLint dot = (S_PROD((GLint) arg0[i][RCOMP] - 128,
                                   (GLint) arg1[i][RCOMP] - 128) +
                            S_PROD((GLint) arg0[i][GCOMP] - 128,
                                   (GLint) arg1[i][GCOMP] - 128) +
                            S_PROD((GLint) arg0[i][BCOMP] - 128,
                                   (GLint) arg1[i][BCOMP] - 128)) >> RGBshift;
               rgba[i][RCOMP] = (GLubyte) CLAMP(dot, 0, 255);
               rgba[i][GCOMP] = (GLubyte) CLAMP(dot, 0, 255);
               rgba[i][BCOMP] = (GLubyte) CLAMP(dot, 0, 255);
            }
         }
         break;
      default:
         gl_problem(NULL, "invalid combine mode");
   }

   switch (textureUnit->CombineModeA) {
      case GL_REPLACE:
         {
            const GLubyte (*arg0)[4] = (const GLubyte (*)[4]) argA[0];
            if (Ashift) {
               for (i = 0; i < n; i++) {
                  GLuint a = (GLuint) arg0[i][ACOMP] << Ashift;
                  rgba[i][ACOMP] = (GLubyte) MIN2(a, 255);
               }
            }
            else {
               for (i = 0; i < n; i++) {
                  rgba[i][ACOMP] = arg0[i][ACOMP];
               }
            }
         }
         break;
      case GL_MODULATE:
         {
            const GLubyte (*arg0)[4] = (const GLubyte (*)[4]) argA[0];
            const GLubyte (*arg1)[4] = (const GLubyte (*)[4]) argA[1];
            Ashift = 8 - Ashift;
            for (i = 0; i < n; i++) {
               GLuint a = (PROD(arg0[i][ACOMP], arg1[i][ACOMP]) >> Ashift);
               rgba[i][ACOMP] = (GLubyte) MIN2(a, 255);
            }
         }
         break;
      case GL_ADD:
         {
            const GLubyte (*arg0)[4] = (const GLubyte (*)[4]) argA[0];
            const GLubyte  (*arg1)[4] = (const GLubyte (*)[4]) argA[1];
            for (i = 0; i < n; i++) {
               GLint a = ((GLint) arg0[i][ACOMP] + arg1[i][ACOMP]) << Ashift;
               rgba[i][ACOMP] = (GLubyte) MIN2(a, 255);
            }
         }
         break;
      case GL_ADD_SIGNED_EXT:
         {
            const GLubyte (*arg0)[4] = (const GLubyte (*)[4]) argA[0];
            const GLubyte (*arg1)[4] = (const GLubyte (*)[4]) argA[1];
            for (i = 0; i < n; i++) {
               GLint a = (GLint) arg0[i][ACOMP] + (GLint) arg1[i][ACOMP] - 128;
               a = (a < 0) ? 0 : a << Ashift;
               rgba[i][ACOMP] = (GLubyte) MIN2(a, 255);
            }
         }
         break;
      case GL_INTERPOLATE_EXT:
         {
            const GLubyte (*arg0)[4] = (const GLubyte (*)[4]) argA[0];
            const GLubyte (*arg1)[4] = (const GLubyte (*)[4]) argA[1];
            const GLubyte (*arg2)[4] = (const GLubyte (*)[4]) argA[2];
            Ashift = 8 - Ashift;
            for (i=0; i<n; i++) {
               GLuint a = (PROD(arg0[i][ACOMP], arg2[i][ACOMP])
                           + PROD(arg1[i][ACOMP], 255 - arg2[i][ACOMP]))
                              >> Ashift;
               rgba[i][ACOMP] = (GLubyte) MIN2(a, 255);
            }
         }
         break;
      default:
         gl_problem(NULL, "invalid combine mode");
   }

   /* Fix the alpha component for GL_DOT3_RGBA_EXT combining.
    */
   if (textureUnit->CombineModeRGB == GL_DOT3_RGBA_EXT) {
      for (i = 0; i < n; i++) {
	 rgba[i][ACOMP] = rgba[i][RCOMP];
      }
   }
}
#undef PROD



/**********************************************************************/
/*                      Texture Application                           */
/**********************************************************************/


/*
 * Combine incoming fragment color with texel color to produce output color.
 * Input:  textureUnit - pointer to texture unit to apply
 *         format - base internal texture format
 *         n - number of fragments
 *         primary_rgba - primary colors (may be rgba for single texture)
 *         texels - array of texel colors
 * InOut:  rgba - incoming fragment colors modified by texel colors
 *                according to the texture environment mode.
 */
static void apply_texture( CONST GLcontext *ctx,
                           const struct gl_texture_unit *texUnit,
                           GLuint n,
                           GLubyte primary_rgba[][4], GLubyte texel[][4],
                           GLubyte rgba[][4] )
{
   GLint baseLevel;
   GLuint i;
   GLint Rc, Gc, Bc, Ac;
   GLenum format;

   ASSERT(texUnit);
   ASSERT(texUnit->Current);

   baseLevel = texUnit->Current->BaseLevel;
   ASSERT(texUnit->Current->Image[baseLevel]);

   format = texUnit->Current->Image[baseLevel]->Format;

/*
 * Use (A*(B+1)) >> 8 as a fast approximation of (A*B)/255 for A
 * and B in [0,255]
 */
#define PROD(A,B)   ( (GLubyte) (((GLint)(A) * ((GLint)(B)+1)) >> 8) )

   if (format==GL_COLOR_INDEX) {
      format = GL_RGBA;  /* XXXX a hack! */
   }

   switch (texUnit->EnvMode) {
      case GL_REPLACE:
	 switch (format) {
	    case GL_ALPHA:
	       for (i=0;i<n;i++) {
		  /* Cv = Cf */
                  /* Av = At */
                  rgba[i][ACOMP] = texel[i][ACOMP];
	       }
	       break;
	    case GL_LUMINANCE:
	       for (i=0;i<n;i++) {
		  /* Cv = Lt */
                  GLubyte Lt = texel[i][RCOMP];
                  rgba[i][RCOMP] = rgba[i][GCOMP] = rgba[i][BCOMP] = Lt;
                  /* Av = Af */
	       }
	       break;
	    case GL_LUMINANCE_ALPHA:
	       for (i=0;i<n;i++) {
                  GLubyte Lt = texel[i][RCOMP];
		  /* Cv = Lt */
		  rgba[i][RCOMP] = rgba[i][GCOMP] = rgba[i][BCOMP] = Lt;
		  /* Av = At */
		  rgba[i][ACOMP] = texel[i][ACOMP];
	       }
	       break;
	    case GL_INTENSITY:
	       for (i=0;i<n;i++) {
		  /* Cv = It */
                  GLubyte It = texel[i][RCOMP];
                  rgba[i][RCOMP] = rgba[i][GCOMP] = rgba[i][BCOMP] = It;
                  /* Av = It */
                  rgba[i][ACOMP] = It;
	       }
	       break;
	    case GL_RGB:
	       for (i=0;i<n;i++) {
		  /* Cv = Ct */
		  rgba[i][RCOMP] = texel[i][RCOMP];
		  rgba[i][GCOMP] = texel[i][GCOMP];
		  rgba[i][BCOMP] = texel[i][BCOMP];
		  /* Av = Af */
	       }
	       break;
	    case GL_RGBA:
	       for (i=0;i<n;i++) {
		  /* Cv = Ct */
		  rgba[i][RCOMP] = texel[i][RCOMP];
		  rgba[i][GCOMP] = texel[i][GCOMP];
		  rgba[i][BCOMP] = texel[i][BCOMP];
		  /* Av = At */
		  rgba[i][ACOMP] = texel[i][ACOMP];
	       }
	       break;
            default:
               gl_problem(ctx, "Bad format (GL_REPLACE) in apply_texture");
               return;
	 }
	 break;

      case GL_MODULATE:
         switch (format) {
	    case GL_ALPHA:
	       for (i=0;i<n;i++) {
		  /* Cv = Cf */
		  /* Av = AfAt */
		  rgba[i][ACOMP] = PROD( rgba[i][ACOMP], texel[i][ACOMP] );
	       }
	       break;
	    case GL_LUMINANCE:
	       for (i=0;i<n;i++) {
		  /* Cv = LtCf */
                  GLubyte Lt = texel[i][RCOMP];
		  rgba[i][RCOMP] = PROD( rgba[i][RCOMP], Lt );
		  rgba[i][GCOMP] = PROD( rgba[i][GCOMP], Lt );
		  rgba[i][BCOMP] = PROD( rgba[i][BCOMP], Lt );
		  /* Av = Af */
	       }
	       break;
	    case GL_LUMINANCE_ALPHA:
	       for (i=0;i<n;i++) {
		  /* Cv = CfLt */
                  GLubyte Lt = texel[i][RCOMP];
		  rgba[i][RCOMP] = PROD( rgba[i][RCOMP], Lt );
		  rgba[i][GCOMP] = PROD( rgba[i][GCOMP], Lt );
		  rgba[i][BCOMP] = PROD( rgba[i][BCOMP], Lt );
		  /* Av = AfAt */
		  rgba[i][ACOMP] = PROD( rgba[i][ACOMP], texel[i][ACOMP] );
	       }
	       break;
	    case GL_INTENSITY:
	       for (i=0;i<n;i++) {
		  /* Cv = CfIt */
                  GLubyte It = texel[i][RCOMP];
		  rgba[i][RCOMP] = PROD( rgba[i][RCOMP], It );
		  rgba[i][GCOMP] = PROD( rgba[i][GCOMP], It );
		  rgba[i][BCOMP] = PROD( rgba[i][BCOMP], It );
		  /* Av = AfIt */
		  rgba[i][ACOMP] = PROD( rgba[i][ACOMP], It );
	       }
	       break;
	    case GL_RGB:
	       for (i=0;i<n;i++) {
		  /* Cv = CfCt */
		  rgba[i][RCOMP] = PROD( rgba[i][RCOMP], texel[i][RCOMP] );
		  rgba[i][GCOMP] = PROD( rgba[i][GCOMP], texel[i][GCOMP] );
		  rgba[i][BCOMP] = PROD( rgba[i][BCOMP], texel[i][BCOMP] );
		  /* Av = Af */
	       }
	       break;
	    case GL_RGBA:
	       for (i=0;i<n;i++) {
		  /* Cv = CfCt */
		  rgba[i][RCOMP] = PROD( rgba[i][RCOMP], texel[i][RCOMP] );
		  rgba[i][GCOMP] = PROD( rgba[i][GCOMP], texel[i][GCOMP] );
		  rgba[i][BCOMP] = PROD( rgba[i][BCOMP], texel[i][BCOMP] );
		  /* Av = AfAt */
		  rgba[i][ACOMP] = PROD( rgba[i][ACOMP], texel[i][ACOMP] );
	       }
	       break;
            default:
               gl_problem(ctx, "Bad format (GL_MODULATE) in apply_texture");
               return;
	 }
	 break;

      case GL_DECAL:
         switch (format) {
            case GL_ALPHA:
            case GL_LUMINANCE:
            case GL_LUMINANCE_ALPHA:
            case GL_INTENSITY:
               /* undefined */
               break;
	    case GL_RGB:
	       for (i=0;i<n;i++) {
		  /* Cv = Ct */
		  rgba[i][RCOMP] = texel[i][RCOMP];
		  rgba[i][GCOMP] = texel[i][GCOMP];
		  rgba[i][BCOMP] = texel[i][BCOMP];
		  /* Av = Af */
	       }
	       break;
	    case GL_RGBA:
	       for (i=0;i<n;i++) {
		  /* Cv = Cf(1-At) + CtAt */
		  GLint t = texel[i][ACOMP], s = 255 - t;
		  rgba[i][RCOMP] = PROD(rgba[i][RCOMP], s) + PROD(texel[i][RCOMP],t);
		  rgba[i][GCOMP] = PROD(rgba[i][GCOMP], s) + PROD(texel[i][GCOMP],t);
		  rgba[i][BCOMP] = PROD(rgba[i][BCOMP], s) + PROD(texel[i][BCOMP],t);
		  /* Av = Af */
	       }
	       break;
            default:
               gl_problem(ctx, "Bad format (GL_DECAL) in apply_texture");
               return;
	 }
	 break;

      case GL_BLEND:
         Rc = (GLint) (texUnit->EnvColor[0] * 255.0F);
         Gc = (GLint) (texUnit->EnvColor[1] * 255.0F);
         Bc = (GLint) (texUnit->EnvColor[2] * 255.0F);
         Ac = (GLint) (texUnit->EnvColor[3] * 255.0F);
	 switch (format) {
	    case GL_ALPHA:
	       for (i=0;i<n;i++) {
		  /* Cv = Cf */
		  /* Av = AfAt */
                  rgba[i][ACOMP] = PROD(rgba[i][ACOMP], texel[i][ACOMP]);
	       }
	       break;
            case GL_LUMINANCE:
	       for (i=0;i<n;i++) {
		  /* Cv = Cf(1-Lt) + CcLt */
		  GLubyte Lt = texel[i][RCOMP], s = 255 - Lt;
		  rgba[i][RCOMP] = PROD(rgba[i][RCOMP], s) + PROD(Rc, Lt);
		  rgba[i][GCOMP] = PROD(rgba[i][GCOMP], s) + PROD(Gc, Lt);
		  rgba[i][BCOMP] = PROD(rgba[i][BCOMP], s) + PROD(Bc, Lt);
		  /* Av = Af */
	       }
	       break;
	    case GL_LUMINANCE_ALPHA:
	       for (i=0;i<n;i++) {
		  /* Cv = Cf(1-Lt) + CcLt */
		  GLubyte Lt = texel[i][RCOMP], s = 255 - Lt;
		  rgba[i][RCOMP] = PROD(rgba[i][RCOMP], s) + PROD(Rc, Lt);
		  rgba[i][GCOMP] = PROD(rgba[i][GCOMP], s) + PROD(Gc, Lt);
		  rgba[i][BCOMP] = PROD(rgba[i][BCOMP], s) + PROD(Bc, Lt);
		  /* Av = AfAt */
		  rgba[i][ACOMP] = PROD(rgba[i][ACOMP],texel[i][ACOMP]);
	       }
	       break;
            case GL_INTENSITY:
	       for (i=0;i<n;i++) {
		  /* Cv = Cf(1-It) + CcLt */
		  GLubyte It = texel[i][RCOMP], s = 255 - It;
		  rgba[i][RCOMP] = PROD(rgba[i][RCOMP], s) + PROD(Rc, It);
		  rgba[i][GCOMP] = PROD(rgba[i][GCOMP], s) + PROD(Gc, It);
		  rgba[i][BCOMP] = PROD(rgba[i][BCOMP], s) + PROD(Bc, It);
                  /* Av = Af(1-It) + Ac*It */
                  rgba[i][ACOMP] = PROD(rgba[i][ACOMP], s) + PROD(Ac, It);
               }
               break;
	    case GL_RGB:
	       for (i=0;i<n;i++) {
		  /* Cv = Cf(1-Ct) + CcCt */
		  rgba[i][RCOMP] = PROD(rgba[i][RCOMP], (255-texel[i][RCOMP])) + PROD(Rc,texel[i][RCOMP]);
		  rgba[i][GCOMP] = PROD(rgba[i][GCOMP], (255-texel[i][GCOMP])) + PROD(Gc,texel[i][GCOMP]);
		  rgba[i][BCOMP] = PROD(rgba[i][BCOMP], (255-texel[i][BCOMP])) + PROD(Bc,texel[i][BCOMP]);
		  /* Av = Af */
	       }
	       break;
	    case GL_RGBA:
	       for (i=0;i<n;i++) {
		  /* Cv = Cf(1-Ct) + CcCt */
		  rgba[i][RCOMP] = PROD(rgba[i][RCOMP], (255-texel[i][RCOMP])) + PROD(Rc,texel[i][RCOMP]);
		  rgba[i][GCOMP] = PROD(rgba[i][GCOMP], (255-texel[i][GCOMP])) + PROD(Gc,texel[i][GCOMP]);
		  rgba[i][BCOMP] = PROD(rgba[i][BCOMP], (255-texel[i][BCOMP])) + PROD(Bc,texel[i][BCOMP]);
		  /* Av = AfAt */
		  rgba[i][ACOMP] = PROD(rgba[i][ACOMP],texel[i][ACOMP]);
	       }
	       break;
            default:
               gl_problem(ctx, "Bad format (GL_BLEND) in apply_texture");
               return;
	 }
	 break;

      case GL_ADD:  /* GL_EXT_texture_add_env */
         switch (format) {
            case GL_ALPHA:
               for (i=0;i<n;i++) {
                  /* Rv = Rf */
                  /* Gv = Gf */
                  /* Bv = Bf */
                  rgba[i][ACOMP] = PROD(rgba[i][ACOMP], texel[i][ACOMP]);
               }
               break;
            case GL_LUMINANCE:
               for (i=0;i<n;i++) {
                  GLuint Lt = texel[i][RCOMP];
                  GLuint r = rgba[i][RCOMP] + Lt;
                  GLuint g = rgba[i][GCOMP] + Lt;
                  GLuint b = rgba[i][BCOMP] + Lt;
                  rgba[i][RCOMP] = r < 256 ? (GLubyte) r : 255;
                  rgba[i][GCOMP] = g < 256 ? (GLubyte) g : 255;
                  rgba[i][BCOMP] = b < 256 ? (GLubyte) b : 255;
                  /* Av = Af */
               }
               break;
            case GL_LUMINANCE_ALPHA:
               for (i=0;i<n;i++) {
                  GLuint Lt = texel[i][RCOMP];
                  GLuint r = rgba[i][RCOMP] + Lt;
                  GLuint g = rgba[i][GCOMP] + Lt;
                  GLuint b = rgba[i][BCOMP] + Lt;
                  rgba[i][RCOMP] = r < 256 ? (GLubyte) r : 255;
                  rgba[i][GCOMP] = g < 256 ? (GLubyte) g : 255;
                  rgba[i][BCOMP] = b < 256 ? (GLubyte) b : 255;
                  rgba[i][ACOMP] = PROD(rgba[i][ACOMP], texel[i][ACOMP]);
               }
               break;
            case GL_INTENSITY:
               for (i=0;i<n;i++) {
                  GLubyte It = texel[i][RCOMP];
                  GLuint r = rgba[i][RCOMP] + It;
                  GLuint g = rgba[i][GCOMP] + It;
                  GLuint b = rgba[i][BCOMP] + It;
                  GLuint a = rgba[i][ACOMP] + It;
                  rgba[i][RCOMP] = r < 256 ? (GLubyte) r : 255;
                  rgba[i][GCOMP] = g < 256 ? (GLubyte) g : 255;
                  rgba[i][BCOMP] = b < 256 ? (GLubyte) b : 255;
                  rgba[i][ACOMP] = a < 256 ? (GLubyte) a : 255;
               }
               break;
	    case GL_RGB:
	       for (i=0;i<n;i++) {
                  GLuint r = rgba[i][RCOMP] + texel[i][RCOMP];
                  GLuint g = rgba[i][GCOMP] + texel[i][GCOMP];
                  GLuint b = rgba[i][BCOMP] + texel[i][BCOMP];
		  rgba[i][RCOMP] = r < 256 ? (GLubyte) r : 255;
		  rgba[i][GCOMP] = g < 256 ? (GLubyte) g : 255;
		  rgba[i][BCOMP] = b < 256 ? (GLubyte) b : 255;
		  /* Av = Af */
	       }
	       break;
	    case GL_RGBA:
	       for (i=0;i<n;i++) {
                  GLuint r = rgba[i][RCOMP] + texel[i][RCOMP];
                  GLuint g = rgba[i][GCOMP] + texel[i][GCOMP];
                  GLuint b = rgba[i][BCOMP] + texel[i][BCOMP];
		  rgba[i][RCOMP] = r < 256 ? (GLubyte) r : 255;
		  rgba[i][GCOMP] = g < 256 ? (GLubyte) g : 255;
		  rgba[i][BCOMP] = b < 256 ? (GLubyte) b : 255;
                  rgba[i][ACOMP] = PROD(rgba[i][ACOMP], texel[i][ACOMP]);
               }
               break;
            default:
               gl_problem(ctx, "Bad format (GL_ADD) in apply_texture");
               return;
	 }
	 break;

      case GL_COMBINE_EXT:    /*  GL_EXT_combine_ext; we modify texel array */
         switch (format) {
            case GL_ALPHA:
               for (i=0;i<n;i++)
                  texel[i][RCOMP] = texel[i][GCOMP] = texel[i][BCOMP] = 0;
               break;
            case GL_LUMINANCE:
               for (i=0;i<n;i++) {
                  /* Cv = Lt */
                  GLubyte Lt = texel[i][RCOMP];
                  texel[i][GCOMP] = texel[i][BCOMP] = Lt;
                  /* Av = 1 */
                  texel[i][ACOMP] = 255;
               }
               break;
            case GL_LUMINANCE_ALPHA:
               for (i=0;i<n;i++) {
                  GLubyte Lt = texel[i][RCOMP];
                  /* Cv = Lt */
                  texel[i][GCOMP] = texel[i][BCOMP] = Lt;
               }
               break;
            case GL_INTENSITY:
               for (i=0;i<n;i++) {
                  /* Cv = It */
                  GLubyte It = texel[i][RCOMP];
                  texel[i][GCOMP] = texel[i][BCOMP] = It;
                  /* Av = It */
                  texel[i][ACOMP] = It;
               }
               break;
            case GL_RGB:
               for (i=0;i<n;i++) {
                  /* Av = 1 */
                  texel[i][ACOMP] = 255;
               }
               break;
            case GL_RGBA:  /* do nothing. */
               break;
            default:
               gl_problem(ctx, "Bad format in apply_texture (GL_COMBINE_EXT)");
               return;
         }
         _mesa_texture_combine(ctx, texUnit, n, primary_rgba, texel, rgba);
         break;

      default:
         gl_problem(ctx, "Bad env mode in apply_texture");
         return;
   }
#undef PROD
}



/*
 * Apply a unit of texture mapping to the incoming fragments.
 */
void gl_texture_pixels( GLcontext *ctx, GLuint texUnit, GLuint n,
                        const GLfloat s[], const GLfloat t[],
                        const GLfloat r[], GLfloat lambda[],
                        GLubyte primary_rgba[][4], GLubyte rgba[][4] )
{
   GLuint mask = (TEXTURE0_1D | TEXTURE0_2D | TEXTURE0_3D | TEXTURE0_CUBE) << (texUnit * 4);
   if (ctx->Texture.ReallyEnabled & mask) {
      const struct gl_texture_unit *textureUnit = &ctx->Texture.Unit[texUnit];
      if (textureUnit->Current && textureUnit->Current->SampleFunc) {
         GLubyte texel[PB_SIZE][4];
	 if (textureUnit->LodBias != 0.0F) {
	    /* apply LOD bias, but don't clamp yet */
            GLuint i;
	    for (i=0;i<n;i++) {
	       lambda[i] += textureUnit->LodBias;
	    }
	 }

         if (textureUnit->Current->MinLod != -1000.0
             || textureUnit->Current->MaxLod != 1000.0) {
            /* apply LOD clamping to lambda */
            GLfloat min = textureUnit->Current->MinLod;
            GLfloat max = textureUnit->Current->MaxLod;
            GLuint i;
            for (i=0;i<n;i++) {
               GLfloat l = lambda[i];
               lambda[i] = CLAMP(l, min, max);
            }
         }

         /* fetch texture images from device driver, if needed */
         if (ctx->Driver.GetTexImage) {
            if (!_mesa_get_teximages_from_driver(ctx, textureUnit->Current)) {
               return;
            }
         }

         /* Sample the texture. */
         (*textureUnit->Current->SampleFunc)( textureUnit->Current, n,
                                             s, t, r, lambda, texel );

         apply_texture( ctx, textureUnit, n, primary_rgba, texel, rgba );
      }
   }
}
