/* $Id: texformat.c,v 1.1.2.1 2001/03/02 16:40:47 gareth Exp $ */

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
 *
 * Author:
 *    Gareth Hughes <gareth@valinux.com>
 */

#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "context.h"
#include "image.h"
#include "mem.h"
#include "mmath.h"
#include "span.h"
#include "texformat.h"
#include "teximage.h"
#include "texstate.h"
#include "types.h"
#endif


/* ================================================================
 * Texture internal formats:
 */

const struct gl_texture_format _mesa_texformat_rgba8888 = {
   MESA_FORMAT_RGBA8888,	/* IntFormat */
   8,				/* RedBits */
   8,				/* GreenBits */
   8,				/* BlueBits */
   8,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
   4,				/* TexelBytes */
};

const struct gl_texture_format _mesa_texformat_abgr8888 = {
   MESA_FORMAT_ABGR8888,	/* IntFormat */
   8,				/* RedBits */
   8,				/* GreenBits */
   8,				/* BlueBits */
   8,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
   4,				/* TexelBytes */
};

const struct gl_texture_format _mesa_texformat_argb8888 = {
   MESA_FORMAT_ARGB8888,	/* IntFormat */
   8,				/* RedBits */
   8,				/* GreenBits */
   8,				/* BlueBits */
   8,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
   4,				/* TexelBytes */
};

const struct gl_texture_format _mesa_texformat_rgb888 = {
   MESA_FORMAT_RGB888,		/* IntFormat */
   8,				/* RedBits */
   8,				/* GreenBits */
   8,				/* BlueBits */
   0,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
   3,				/* TexelBytes */
};

const struct gl_texture_format _mesa_texformat_bgr888 = {
   MESA_FORMAT_BGR888,		/* IntFormat */
   8,				/* RedBits */
   8,				/* GreenBits */
   8,				/* BlueBits */
   0,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
   3,				/* TexelBytes */
};

const struct gl_texture_format _mesa_texformat_rgb565 = {
   MESA_FORMAT_RGB565,		/* IntFormat */
   5,				/* RedBits */
   6,				/* GreenBits */
   5,				/* BlueBits */
   0,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
   2,				/* TexelBytes */
};

const struct gl_texture_format _mesa_texformat_argb4444 = {
   MESA_FORMAT_ARGB4444,	/* IntFormat */
   4,				/* RedBits */
   4,				/* GreenBits */
   4,				/* BlueBits */
   4,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
   2,				/* TexelBytes */
};

const struct gl_texture_format _mesa_texformat_argb1555 = {
   MESA_FORMAT_ARGB1555,	/* IntFormat */
   5,				/* RedBits */
   5,				/* GreenBits */
   5,				/* BlueBits */
   1,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
   2,				/* TexelBytes */
};

const struct gl_texture_format _mesa_texformat_al88 = {
   MESA_FORMAT_AL88,		/* IntFormat */
   0,				/* RedBits */
   0,				/* GreenBits */
   0,				/* BlueBits */
   8,				/* AlphaBits */
   8,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
   2,				/* TexelBytes */
};

const struct gl_texture_format _mesa_texformat_rgb332 = {
   MESA_FORMAT_RGB332,		/* IntFormat */
   3,				/* RedBits */
   3,				/* GreenBits */
   2,				/* BlueBits */
   0,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
   1,				/* TexelBytes */
};

const struct gl_texture_format _mesa_texformat_a8 = {
   MESA_FORMAT_A8,		/* IntFormat */
   0,				/* RedBits */
   0,				/* GreenBits */
   0,				/* BlueBits */
   8,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
   1,				/* TexelBytes */
};

const struct gl_texture_format _mesa_texformat_l8 = {
   MESA_FORMAT_L8,		/* IntFormat */
   0,				/* RedBits */
   0,				/* GreenBits */
   0,				/* BlueBits */
   0,				/* AlphaBits */
   8,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
   1,				/* TexelBytes */
};

const struct gl_texture_format _mesa_texformat_i8 = {
   MESA_FORMAT_I8,		/* IntFormat */
   0,				/* RedBits */
   0,				/* GreenBits */
   0,				/* BlueBits */
   0,				/* AlphaBits */
   0,				/* LuminanceBits */
   8,				/* IntensityBits */
   0,				/* IndexBits */
   1,				/* TexelBytes */
};

const struct gl_texture_format _mesa_texformat_ci8 = {
   MESA_FORMAT_CI8,		/* IntFormat */
   0,				/* RedBits */
   0,				/* GreenBits */
   0,				/* BlueBits */
   0,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   8,				/* IndexBits */
   1,				/* TexelBytes */
};

const struct gl_texture_format _mesa_null_texformat = {
   -1,				/* IntFormat */
   0,				/* RedBits */
   0,				/* GreenBits */
   0,				/* BlueBits */
   0,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
   0,				/* TexelBytes */
};



/*
 * Given an internal texture format enum or 1, 2, 3, 4 return the
 * corresponding _base_ internal format:  GL_ALPHA, GL_LUMINANCE,
 * GL_LUMANCE_ALPHA, GL_INTENSITY, GL_RGB, or GL_RGBA.
 * Return -1 if invalid enum.
 */
void _mesa_init_texture_format( GLcontext *ctx,
				struct gl_texture_image *texImage,
				GLenum internalFormat )
{
   texImage->IntFormat = internalFormat;

   /* Ask the driver for the base format, if it doesn't know, it will
    * return -1;
    */
   if ( ctx->Driver.BaseCompressedTexFormat ) {
      GLint format = 0;			/* Silence compiler warning */
      format = (*ctx->Driver.BaseCompressedTexFormat)( ctx, format );
      if ( format >= 0 ) {
	 internalFormat = format;
      }
   }

   switch ( internalFormat ) {
      /* GH: Bias towards GL_RGB, GL_RGBA texture formats.  This has
       * got to be better than sticking them way down the end of this
       * huge list.
       */
   case 4:				/* Quake3 uses this... */
   case GL_RGBA:
      texImage->Format = GL_RGBA;
      texImage->TexFormat = &_mesa_texformat_abgr8888;
      break;

   case 3:				/* ... and this. */
   case GL_RGB:
      texImage->Format = GL_RGB;
      texImage->TexFormat = &_mesa_texformat_bgr888;
      break;

      /* GH: Okay, keep checking as normal.  Still test for GL_RGB,
       * GL_RGBA formats first.
       */
   case GL_RGBA2:
   case GL_RGBA4:
   case GL_RGB5_A1:
   case GL_RGBA8:
   case GL_RGB10_A2:
   case GL_RGBA12:
   case GL_RGBA16:
      texImage->Format = GL_RGBA;
      texImage->TexFormat = &_mesa_texformat_abgr8888;
      break;

   case GL_R3_G3_B2:
   case GL_RGB4:
   case GL_RGB5:
   case GL_RGB8:
   case GL_RGB10:
   case GL_RGB12:
   case GL_RGB16:
      texImage->Format = GL_RGB;
      texImage->TexFormat = &_mesa_texformat_bgr888;
      break;

   case GL_ALPHA:
   case GL_ALPHA4:
   case GL_ALPHA8:
   case GL_ALPHA12:
   case GL_ALPHA16:
      texImage->Format = GL_ALPHA;
      texImage->TexFormat = &_mesa_texformat_a8;
      break;

   case 1:
   case GL_LUMINANCE:
   case GL_LUMINANCE4:
   case GL_LUMINANCE8:
   case GL_LUMINANCE12:
   case GL_LUMINANCE16:
      texImage->Format = GL_LUMINANCE;
      texImage->TexFormat = &_mesa_texformat_l8;
      break;

   case 2:
   case GL_LUMINANCE_ALPHA:
   case GL_LUMINANCE4_ALPHA4:
   case GL_LUMINANCE6_ALPHA2:
   case GL_LUMINANCE8_ALPHA8:
   case GL_LUMINANCE12_ALPHA4:
   case GL_LUMINANCE12_ALPHA12:
   case GL_LUMINANCE16_ALPHA16:
      texImage->Format = GL_LUMINANCE_ALPHA;
      texImage->TexFormat = &_mesa_texformat_al88;
      break;

   case GL_INTENSITY:
   case GL_INTENSITY4:
   case GL_INTENSITY8:
   case GL_INTENSITY12:
   case GL_INTENSITY16:
      texImage->Format = GL_INTENSITY;
      texImage->TexFormat = &_mesa_texformat_i8;
      break;

   case GL_COLOR_INDEX:
   case GL_COLOR_INDEX1_EXT:
   case GL_COLOR_INDEX2_EXT:
   case GL_COLOR_INDEX4_EXT:
   case GL_COLOR_INDEX8_EXT:
   case GL_COLOR_INDEX12_EXT:
   case GL_COLOR_INDEX16_EXT:
      texImage->Format = GL_COLOR_INDEX;
      texImage->TexFormat = &_mesa_texformat_ci8;
      break;

   default:
      gl_problem( ctx, "unexpected format in _mesa_init_texture_format" );
      return;
   }
}
