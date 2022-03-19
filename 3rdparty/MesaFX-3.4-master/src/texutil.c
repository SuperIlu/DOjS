/* $Id: texutil.c,v 1.5.4.8 2001/05/18 21:44:23 brianp Exp $ */

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
 * Authors:
 *    Gareth Hughes <gareth@valinux.com>
 */

#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "context.h"
#include "enums.h"
#include "image.h"
#include "mem.h"
#include "texformat.h"
#include "texutil.h"
#include "types.h"
#endif

#define DEBUG_TEXUTIL 0


struct gl_texture_convert {
   GLint xoffset, yoffset, zoffset;	/* Subimage offset */
   GLint width, height, depth;		/* Subimage region */

   GLint imageWidth, imageHeight;	/* Full image dimensions */
   GLenum format, type;

   const struct gl_pixelstore_attrib *packing;

   const GLvoid *srcImage;
   GLvoid *dstImage;

   GLint index;
};

typedef GLboolean (*convert_func)( struct gl_texture_convert *convert );
typedef void (*unconvert_func)( struct gl_texture_convert *convert );

#define CONVERT_STRIDE_BIT	0x1
#define CONVERT_PACKING_BIT	0x2


/* ================================================================
 * RGBA8888 textures:
 */

#define DST_TYPE		GLuint
#define DST_TEXELS_PER_DWORD	1

#define CONVERT_TEXEL( src )						\
		PACK_COLOR_8888( src[3], src[2], src[1], src[0] )

#define CONVERT_DIRECT

#define SRC_TEXEL_BYTES		4

#define TAG(x) x##_rgba8888_direct
#define PRESERVE_DST_TYPE
#include "texutil_tmp.h"


#define CONVERT_TEXEL( src )						\
		PACK_COLOR_8888( src[0], src[1], src[2], src[3] )

#define CONVERT_TEXEL_DWORD( src )	CONVERT_TEXEL( src )

#define SRC_TEXEL_BYTES		4

#define TAG(x) x##_abgr8888_to_rgba8888
#define PRESERVE_DST_TYPE
#include "texutil_tmp.h"


#define CONVERT_TEXEL( src )						\
		PACK_COLOR_8888( src[0], src[1], src[2], 0xff )

#define CONVERT_TEXEL_DWORD( src )	CONVERT_TEXEL( src )

#define SRC_TEXEL_BYTES		3

#define TAG(x) x##_bgr888_to_rgba8888
#include "texutil_tmp.h"


#define CONVERT_RGBA8888( name )					\
static GLboolean							\
convert_##name##_rgba8888( struct gl_texture_convert *convert )		\
{									\
   convert_func *tab;							\
   GLint index = convert->index;					\
									\
   if ( convert->format == GL_ABGR_EXT &&				\
	convert->type == GL_UNSIGNED_INT_8_8_8_8_REV )			\
   {									\
      tab = name##_tab_rgba8888_direct;					\
   }									\
   else if ( convert->format == GL_RGBA &&				\
	     ( convert->type == GL_UNSIGNED_BYTE ||			\
	       convert->type == GL_UNSIGNED_INT_8_8_8_8 ) )		\
   {									\
      tab = name##_tab_abgr8888_to_rgba8888;				\
   }									\
   else if ( convert->format == GL_RGB &&				\
	     convert->type == GL_UNSIGNED_BYTE )			\
   {									\
      tab = name##_tab_bgr888_to_rgba8888;				\
   }									\
   else									\
   {									\
      /* Can't handle this source format/type combination */		\
      return GL_FALSE;							\
   }									\
									\
   return tab[index]( convert );					\
}

CONVERT_RGBA8888( texsubimage2d )
CONVERT_RGBA8888( texsubimage3d )


static void
unconvert_teximage_rgba8888( struct gl_texture_convert *convert )
{
   const GLubyte *src = (const GLubyte *)convert->srcImage;
   GLint texels, i;

   texels = convert->width * convert->height * convert->depth;

   switch ( convert->format ) {
   case GL_RGBA: {
      GLuint *dst = (GLuint *)convert->dstImage;
      for ( i = 0 ; i < texels ; i++ ) {
	 *dst++ = PACK_COLOR_8888( src[0], src[1], src[2], src[3] );
	 src += 4;
      }
      break;
   }
   case GL_RGB: {
      GLubyte *dst = (GLubyte *)convert->dstImage;
      for ( i = 0 ; i < texels ; i++ ) {
	 *dst++ = src[3];
	 *dst++ = src[2];
	 *dst++ = src[1];
	 src += 4;
      }
      break;
   }
   default:
      gl_problem(NULL, "texture unconvert error");
   }
}



/* ================================================================
 * ABGR8888 textures:
 */

#define DST_TYPE		GLuint
#define DST_TEXELS_PER_DWORD	1

#define CONVERT_TEXEL( src )						\
		PACK_COLOR_8888( src[3], src[2], src[1], src[0] )

#define CONVERT_DIRECT

#define SRC_TEXEL_BYTES		4

#define TAG(x) x##_abgr8888_direct
#define PRESERVE_DST_TYPE
#include "texutil_tmp.h"


#define CONVERT_TEXEL( src )						\
		PACK_COLOR_8888( 0xff, src[2], src[1], src[0] )

#define CONVERT_TEXEL_DWORD( src )	CONVERT_TEXEL( src )

#define SRC_TEXEL_BYTES		3

#define TAG(x) x##_bgr888_to_abgr8888
#include "texutil_tmp.h"


#define CONVERT_ABGR8888( name )					\
static GLboolean							\
convert_##name##_abgr8888( struct gl_texture_convert *convert )		\
{									\
   convert_func *tab;							\
   GLint index = convert->index;					\
									\
   if ( convert->format == GL_RGBA &&					\
	convert->type == GL_UNSIGNED_BYTE )				\
   {									\
      tab = name##_tab_abgr8888_direct;					\
   }									\
   else if ( convert->format == GL_RGB &&				\
	     convert->type == GL_UNSIGNED_BYTE )			\
   {									\
      tab = name##_tab_bgr888_to_abgr8888;				\
   }									\
   else									\
   {									\
      /* Can't handle this source format/type combination */		\
      return GL_FALSE;							\
   }									\
									\
   return tab[index]( convert );					\
}

CONVERT_ABGR8888( texsubimage2d )
CONVERT_ABGR8888( texsubimage3d )


static void
unconvert_teximage_abgr8888( struct gl_texture_convert *convert )
{
   const GLubyte *src = (const GLubyte *)convert->srcImage;
   GLint texels, i;

   texels = convert->width * convert->height * convert->depth;

   switch ( convert->format ) {
   case GL_RGBA:
      MEMCPY( convert->dstImage, src, texels * 4 );
      break;
   case GL_RGB: {
      GLubyte *dst = (GLubyte *)convert->dstImage;
      for ( i = 0 ; i < texels ; i++ ) {
	 *dst++ = src[0];
	 *dst++ = src[1];
	 *dst++ = src[2];
	 src += 4;
      }
      break;
   }
   default:
      gl_problem(NULL, "texture unconvert error");
   }
}



/* ================================================================
 * ARGB8888 textures:
 */

#define DST_TYPE		GLuint
#define DST_TEXELS_PER_DWORD	1

#define CONVERT_TEXEL( src )						\
		PACK_COLOR_8888( src[3], src[2], src[1], src[0] )

#define CONVERT_DIRECT

#define SRC_TEXEL_BYTES		4

#define TAG(x) x##_argb8888_direct
#define PRESERVE_DST_TYPE
#include "texutil_tmp.h"


#define CONVERT_TEXEL( src )						\
		PACK_COLOR_8888( src[3], src[0], src[1], src[2] )

#define CONVERT_TEXEL_DWORD( src )	CONVERT_TEXEL( src )

#define SRC_TEXEL_BYTES		4

#define TAG(x) x##_abgr8888_to_argb8888
#define PRESERVE_DST_TYPE
#include "texutil_tmp.h"


#define CONVERT_TEXEL( src )						\
		PACK_COLOR_8888( 0xff, src[0], src[1], src[2] )

#define CONVERT_TEXEL_DWORD( src )	CONVERT_TEXEL( src )

#define SRC_TEXEL_BYTES		3

#define TAG(x) x##_bgr888_to_argb8888
#include "texutil_tmp.h"


#define CONVERT_ARGB8888( name )					\
static GLboolean							\
convert_##name##_argb8888( struct gl_texture_convert *convert )		\
{									\
   convert_func *tab;							\
   GLint index = convert->index;					\
									\
   if ( convert->format == GL_BGRA &&					\
	convert->type == GL_UNSIGNED_INT_8_8_8_8_REV )			\
   {									\
      tab = name##_tab_argb8888_direct;					\
   }									\
   else if ( convert->format == GL_RGBA &&				\
	     convert->type == GL_UNSIGNED_BYTE )			\
   {									\
      tab = name##_tab_abgr8888_to_argb8888;				\
   }									\
   else if ( convert->format == GL_RGB &&				\
	     convert->type == GL_UNSIGNED_BYTE )			\
   {									\
      tab = name##_tab_bgr888_to_argb8888;				\
   }									\
   else									\
   {									\
      /* Can't handle this source format/type combination */		\
      return GL_FALSE;							\
   }									\
									\
   return tab[index]( convert );					\
}

CONVERT_ARGB8888( texsubimage2d )
CONVERT_ARGB8888( texsubimage3d )


static void
unconvert_teximage_argb8888( struct gl_texture_convert *convert )
{
   const GLubyte *src = (const GLubyte *)convert->srcImage;
   GLint texels, i;

   texels = convert->width * convert->height * convert->depth;

   switch ( convert->format ) {
   case GL_RGBA: {
      GLuint *dst = (GLuint *)convert->dstImage;
      for ( i = 0 ; i < texels ; i++ ) {
	 *dst++ = PACK_COLOR_8888( src[3], src[0], src[1], src[2] );
	 src += 4;
      }
      break;
   }
   case GL_RGB: {
      GLubyte *dst = (GLubyte *)convert->dstImage;
      for ( i = 0 ; i < texels ; i++ ) {
	 *dst++ = src[2];
	 *dst++ = src[1];
	 *dst++ = src[0];
	 src += 4;
      }
      break;
   }
   default:
      gl_problem(NULL, "texture unconvert error");
   }
}



/* ================================================================
 * RGB888 textures:
 */

static GLboolean
convert_texsubimage2d_rgb888( struct gl_texture_convert *convert )
{
   /* This is a placeholder for now...
    */
   return GL_FALSE;
}

static GLboolean
convert_texsubimage3d_rgb888( struct gl_texture_convert *convert )
{
   /* This is a placeholder for now...
    */
   return GL_FALSE;
}


static void
unconvert_teximage_rgb888( struct gl_texture_convert *convert )
{
   gl_problem(NULL, "texture unconvert error");
}



/* ================================================================
 * BGR888 textures:
 */

static GLboolean
convert_texsubimage2d_bgr888( struct gl_texture_convert *convert )
{
   /* This is a placeholder for now...
    */
   return GL_FALSE;
}

static GLboolean
convert_texsubimage3d_bgr888( struct gl_texture_convert *convert )
{
   /* This is a placeholder for now...
    */
   return GL_FALSE;
}


static void
unconvert_teximage_bgr888( struct gl_texture_convert *convert )
{
   gl_problem(NULL, "texture unconvert error");
}



/* ================================================================
 * RGB565 textures:
 */

#define DST_TYPE		GLushort
#define DST_TEXELS_PER_DWORD	2

#define CONVERT_TEXEL( src )						\
		PACK_COLOR_565( src[0], src[1], src[2] )

#define CONVERT_DIRECT

#define SRC_TEXEL_BYTES		2

#define TAG(x) x##_rgb565_direct
#define PRESERVE_DST_TYPE
#include "texutil_tmp.h"


#define CONVERT_TEXEL( src )						\
		PACK_COLOR_565( src[0], src[1], src[2] )

#define CONVERT_TEXEL_DWORD( src )					\
		((PACK_COLOR_565( src[0], src[1], src[2] )) |		\
		 (PACK_COLOR_565( src[3], src[4], src[5] ) << 16))

#define SRC_TEXEL_BYTES		3

#define TAG(x) x##_bgr888_to_rgb565
#define PRESERVE_DST_TYPE
#include "texutil_tmp.h"


#define CONVERT_TEXEL( src )						\
		PACK_COLOR_565( src[0], src[1], src[2] )

#define CONVERT_TEXEL_DWORD( src )					\
		((PACK_COLOR_565( src[0], src[1], src[2] )) |		\
		 (PACK_COLOR_565( src[4], src[5], src[6] ) << 16))

#define SRC_TEXEL_BYTES		4

#define TAG(x) x##_abgr8888_to_rgb565
#include "texutil_tmp.h"


#define CONVERT_RGB565( name )						\
static GLboolean							\
convert_##name##_rgb565( struct gl_texture_convert *convert )		\
{									\
   convert_func *tab;							\
   GLint index = convert->index;					\
									\
   if ( convert->format == GL_RGB &&					\
	convert->type == GL_UNSIGNED_SHORT_5_6_5 )			\
   {									\
      tab = name##_tab_rgb565_direct;					\
   }									\
   else if ( convert->format == GL_RGB &&				\
	     convert->type == GL_UNSIGNED_BYTE )			\
   {									\
      tab = name##_tab_bgr888_to_rgb565;				\
   }									\
   else if ( convert->format == GL_RGBA &&				\
	     convert->type == GL_UNSIGNED_BYTE )			\
   {									\
      tab = name##_tab_abgr8888_to_rgb565;				\
   }									\
   else									\
   {									\
      /* Can't handle this source format/type combination */		\
      return GL_FALSE;							\
   }									\
									\
   return tab[index]( convert );					\
}

CONVERT_RGB565( texsubimage2d )
CONVERT_RGB565( texsubimage3d )


static void
unconvert_teximage_rgb565( struct gl_texture_convert *convert )
{
   const GLushort *src = (const GLushort *)convert->srcImage;
   GLubyte *dst = (GLubyte *)convert->dstImage;
   GLint texels, i;

   texels = convert->width * convert->height * convert->depth;

   switch ( convert->format ) {
   case GL_RGBA:
      for ( i = 0 ; i < texels ; i++ ) {
	 GLushort s = *src++;
	 *dst++ = ((s >> 8) & 0xf8) * 255 / 0xf8;
	 *dst++ = ((s >> 3) & 0xfc) * 255 / 0xfc;
	 *dst++ = ((s << 3) & 0xf8) * 255 / 0xf8;
	 *dst++ = 0xff;
      }
      break;
   case GL_RGB:
      for ( i = 0 ; i < texels ; i++ ) {
	 GLushort s = *src++;
	 *dst++ = ((s >> 8) & 0xf8) * 255 / 0xf8;
	 *dst++ = ((s >> 3) & 0xfc) * 255 / 0xfc;
	 *dst++ = ((s << 3) & 0xf8) * 255 / 0xf8;
      }
      break;
   default:
      gl_problem(NULL, "texture unconvert error");
   }
}



/* ================================================================
 * ARGB4444 textures:
 */

#define DST_TYPE		GLushort
#define DST_TEXELS_PER_DWORD	2

#define CONVERT_TEXEL( src )						\
		PACK_COLOR_4444( src[3], src[0], src[1], src[2] )

#define CONVERT_DIRECT

#define SRC_TEXEL_BYTES		2

#define TAG(x) x##_argb4444_direct
#define PRESERVE_DST_TYPE
#include "texutil_tmp.h"


#define CONVERT_TEXEL( src )						\
		PACK_COLOR_4444( src[3], src[0], src[1], src[2] )

#define CONVERT_TEXEL_DWORD( src )					\
		((PACK_COLOR_4444( src[3], src[0], src[1], src[2] )) |	\
		 (PACK_COLOR_4444( src[7], src[4], src[5], src[6] ) << 16))

#define SRC_TEXEL_BYTES		4

#define TAG(x) x##_rgba8888_to_argb4444
#include "texutil_tmp.h"


#define CONVERT_ARGB4444( name )					\
static GLboolean							\
convert_##name##_argb4444( struct gl_texture_convert *convert )		\
{									\
   convert_func *tab;							\
   GLint index = convert->index;					\
									\
   if ( convert->format == GL_BGRA &&					\
	convert->type == GL_UNSIGNED_SHORT_4_4_4_4_REV )		\
   {									\
      tab = name##_tab_argb4444_direct;					\
   }									\
   else if ( convert->format == GL_RGBA &&				\
	     convert->type == GL_UNSIGNED_BYTE )			\
   {									\
      tab = name##_tab_rgba8888_to_argb4444;				\
   }									\
   else									\
   {									\
      /* Can't handle this source format/type combination */		\
      return GL_FALSE;							\
   }									\
									\
   return tab[index]( convert );					\
}

CONVERT_ARGB4444( texsubimage2d )
CONVERT_ARGB4444( texsubimage3d )


static void
unconvert_teximage_argb4444( struct gl_texture_convert *convert )
{
   const GLushort *src = (const GLushort *)convert->srcImage;
   GLubyte *dst = (GLubyte *)convert->dstImage;
   GLint texels, i;

   texels = convert->width * convert->height * convert->depth;

   switch ( convert->format ) {
   case GL_RGBA:
      for ( i = 0 ; i < texels ; i++ ) {
	 GLushort s = *src++;
	 *dst++ = ((s >>  8) & 0xf) * 255 / 0xf;
	 *dst++ = ((s >>  4) & 0xf) * 255 / 0xf;
	 *dst++ = ((s      ) & 0xf) * 255 / 0xf;
	 *dst++ = ((s >> 12) & 0xf) * 255 / 0xf;
      }
      break;
   default:
      gl_problem(NULL, "texture unconvert error");
   }
}



/* ================================================================
 * ARGB1555 textures:
 */

#define DST_TYPE		GLushort
#define DST_TEXELS_PER_DWORD	2

#define CONVERT_TEXEL( src )						\
		PACK_COLOR_1555( src[3], src[0], src[1], src[2] )

#define CONVERT_DIRECT

#define SRC_TEXEL_BYTES		2

#define TAG(x) x##_argb1555_direct
#define PRESERVE_DST_TYPE
#include "texutil_tmp.h"


#define CONVERT_TEXEL( src )						\
		PACK_COLOR_1555( src[3], src[0], src[1], src[2] )

#define CONVERT_TEXEL_DWORD( src )					\
		((PACK_COLOR_1555( src[3], src[0], src[1], src[2] )) |	\
		 (PACK_COLOR_1555( src[7], src[4], src[5], src[6] ) << 16))

#define SRC_TEXEL_BYTES		4

#define TAG(x) x##_rgba8888_to_argb1555
#include "texutil_tmp.h"


#define CONVERT_ARGB1555( name )					\
static GLboolean							\
convert_##name##_argb1555( struct gl_texture_convert *convert )		\
{									\
   convert_func *tab;							\
   GLint index = convert->index;					\
									\
   if ( convert->format == GL_BGRA &&					\
	convert->type == GL_UNSIGNED_SHORT_1_5_5_5_REV )		\
   {									\
      tab = name##_tab_argb1555_direct;					\
   }									\
   else if ( convert->format == GL_RGBA &&				\
	     convert->type == GL_UNSIGNED_BYTE )			\
   {									\
      tab = name##_tab_rgba8888_to_argb1555;				\
   }									\
   else									\
   {									\
      /* Can't handle this source format/type combination */		\
      return GL_FALSE;							\
   }									\
									\
   return tab[index]( convert );					\
}

CONVERT_ARGB1555( texsubimage2d )
CONVERT_ARGB1555( texsubimage3d )


static void
unconvert_teximage_argb1555( struct gl_texture_convert *convert )
{
   const GLushort *src = (const GLushort *)convert->srcImage;
   GLubyte *dst = (GLubyte *)convert->dstImage;
   GLint texels, i;

   texels = convert->width * convert->height * convert->depth;

   switch ( convert->format ) {
   case GL_RGBA:
      for ( i = 0 ; i < texels ; i++ ) {
	 GLushort s = *src++;
	 *dst++ = ((s >> 10) & 0xf8) * 255 / 0xf8;
	 *dst++ = ((s >>  5) & 0xf8) * 255 / 0xf8;
	 *dst++ = ((s      ) & 0xf8) * 255 / 0xf8;
	 *dst++ = ((s >> 15) & 0x01) * 255;
      }
      break;
   default:
      gl_problem(NULL, "texture unconvert error");
   }
}



/* ================================================================
 * AL88 textures:
 */

#define DST_TYPE		GLushort
#define DST_TEXELS_PER_DWORD	2

#define CONVERT_TEXEL( src )						\
		PACK_COLOR_88( src[0], src[1] )

#define CONVERT_DIRECT

#define SRC_TEXEL_BYTES		2

#define TAG(x) x##_al88_direct
#define PRESERVE_DST_TYPE
#include "texutil_tmp.h"


#define CONVERT_TEXEL( src )						\
		PACK_COLOR_88( src[0], 0x00 )

#define CONVERT_TEXEL_DWORD( src )					\
		((PACK_COLOR_88( src[0], 0x00 )) |			\
		 (PACK_COLOR_88( src[1], 0x00 ) << 16))

#define SRC_TEXEL_BYTES		1

#define TAG(x) x##_a8_to_al88
#define PRESERVE_DST_TYPE
#include "texutil_tmp.h"


#define CONVERT_TEXEL( src )						\
		PACK_COLOR_88( 0xff, src[0] )

#define CONVERT_TEXEL_DWORD( src )					\
		((PACK_COLOR_88( 0xff, src[0] )) |			\
		 (PACK_COLOR_88( 0xff, src[1] ) << 16))

#define SRC_TEXEL_BYTES		1

#define TAG(x) x##_l8_to_al88
#include "texutil_tmp.h"


#define CONVERT_AL88( name )						\
static GLboolean							\
convert_##name##_al88( struct gl_texture_convert *convert )		\
{									\
   convert_func *tab;							\
   GLint index = convert->index;					\
									\
   if ( convert->format == GL_LUMINANCE_ALPHA &&			\
	convert->type == GL_UNSIGNED_BYTE )				\
   {									\
      tab = name##_tab_al88_direct;					\
   }									\
   else if ( convert->format == GL_ALPHA &&				\
	     convert->type == GL_UNSIGNED_BYTE )			\
   {									\
      tab = name##_tab_a8_to_al88;					\
   }									\
   else if ( convert->format == GL_LUMINANCE &&				\
	     convert->type == GL_UNSIGNED_BYTE )			\
   {									\
      tab = name##_tab_l8_to_al88;					\
   }									\
   else									\
   {									\
      /* Can't handle this source format/type combination */		\
      return GL_FALSE;							\
   }									\
									\
   return tab[index]( convert );					\
}

CONVERT_AL88( texsubimage2d )
CONVERT_AL88( texsubimage3d )


static void
unconvert_teximage_al88( struct gl_texture_convert *convert )
{
   const GLubyte *src = (const GLubyte *)convert->srcImage;
   GLint texels, i;

   texels = convert->width * convert->height * convert->depth;

   switch ( convert->format ) {
   case GL_LUMINANCE_ALPHA:
      MEMCPY( convert->dstImage, src, texels * 2 );
      break;
   case GL_ALPHA: {
      GLubyte *dst = (GLubyte *)convert->dstImage;
      for ( i = 0 ; i < texels ; i++ ) {
	 *dst++ = src[1];
	 src += 2;
      }
      break;
   }
   case GL_LUMINANCE: {
      GLubyte *dst = (GLubyte *)convert->dstImage;
      for ( i = 0 ; i < texels ; i++ ) {
	 *dst++ = src[0];
	 src += 2;
      }
      break;
   }
   default:
      gl_problem(NULL, "texture unconvert error");
   }
}



/* ================================================================
 * RGB332 textures:
 */

static GLboolean
convert_texsubimage2d_rgb332( struct gl_texture_convert *convert )
{
   /* This is a placeholder for now...
    */
   return GL_FALSE;
}

static GLboolean
convert_texsubimage3d_rgb332( struct gl_texture_convert *convert )
{
   /* This is a placeholder for now...
    */
   return GL_FALSE;
}


static void
unconvert_teximage_rgb332( struct gl_texture_convert *convert )
{
   gl_problem(NULL, "texture unconvert error");
}



/* ================================================================
 * CI8 (and all other single-byte texel) textures:
 */

#define DST_TYPE		GLubyte
#define DST_TEXELS_PER_DWORD	4

#define CONVERT_TEXEL( src )	src[0]

#define CONVERT_DIRECT

#define SRC_TEXEL_BYTES		1

#define TAG(x) x##_ci8_direct
#include "texutil_tmp.h"


#define CONVERT_CI8( name )						\
static GLboolean							\
convert_##name##_ci8( struct gl_texture_convert *convert )		\
{									\
   convert_func *tab;							\
   GLint index = convert->index;					\
									\
   if ( ( convert->format == GL_ALPHA ||				\
	  convert->format == GL_LUMINANCE ||				\
	  convert->format == GL_INTENSITY ||				\
	  convert->format == GL_COLOR_INDEX ) &&			\
	convert->type == GL_UNSIGNED_BYTE )				\
   {									\
      tab = name##_tab_ci8_direct;					\
   }									\
   else									\
   {									\
      /* Can't handle this source format/type combination */		\
      return GL_FALSE;							\
   }									\
									\
   return tab[index]( convert );					\
}

CONVERT_CI8( texsubimage2d )
CONVERT_CI8( texsubimage3d )


static void
unconvert_teximage_ci8( struct gl_texture_convert *convert )
{
   const GLubyte *src = (const GLubyte *)convert->srcImage;
   GLint texels;

   texels = convert->width * convert->height * convert->depth;

   switch ( convert->format ) {
   case GL_ALPHA:
   case GL_LUMINANCE:
   case GL_INTENSITY:
   case GL_COLOR_INDEX:
      MEMCPY( convert->dstImage, src, texels );
      break;
   default:
      gl_problem(NULL, "texture unconvert error");
   }
}



/* ================================================================
 * Global entry points
 */

static convert_func gl_convert_texsubimage2d_tab[] = {
   convert_texsubimage2d_rgba8888,
   convert_texsubimage2d_abgr8888,
   convert_texsubimage2d_argb8888,
   convert_texsubimage2d_rgb888,
   convert_texsubimage2d_bgr888,
   convert_texsubimage2d_rgb565,
   convert_texsubimage2d_argb4444,
   convert_texsubimage2d_argb1555,
   convert_texsubimage2d_al88,
   convert_texsubimage2d_rgb332,
   convert_texsubimage2d_ci8,		/* These are all the same... */
   convert_texsubimage2d_ci8,
   convert_texsubimage2d_ci8,
   convert_texsubimage2d_ci8,
};

static convert_func gl_convert_texsubimage3d_tab[] = {
   convert_texsubimage3d_rgba8888,
   convert_texsubimage3d_abgr8888,
   convert_texsubimage3d_argb8888,
   convert_texsubimage3d_rgb888,
   convert_texsubimage3d_bgr888,
   convert_texsubimage3d_rgb565,
   convert_texsubimage3d_argb4444,
   convert_texsubimage3d_argb1555,
   convert_texsubimage3d_al88,
   convert_texsubimage3d_rgb332,
   convert_texsubimage3d_ci8,		/* These are all the same... */
   convert_texsubimage3d_ci8,
   convert_texsubimage3d_ci8,
   convert_texsubimage3d_ci8,
};

static unconvert_func gl_unconvert_teximage_tab[] = {
   unconvert_teximage_rgba8888,
   unconvert_teximage_abgr8888,
   unconvert_teximage_argb8888,
   unconvert_teximage_rgb888,
   unconvert_teximage_bgr888,
   unconvert_teximage_rgb565,
   unconvert_teximage_argb4444,
   unconvert_teximage_argb1555,
   unconvert_teximage_al88,
   unconvert_teximage_rgb332,
   unconvert_teximage_ci8,		/* These are all the same... */
   unconvert_teximage_ci8,
   unconvert_teximage_ci8,
   unconvert_teximage_ci8,
};


/* See if we need to care about the pixel store attributes when we're
 * converting the texture image.  This should be stored as
 * packing->_SomeBoolean and updated when the values change, to avoid
 * testing every time...
 */
static INLINE GLboolean
convert_needs_packing( const struct gl_pixelstore_attrib *packing,
		       GLenum format, GLenum type )
{
   if ( ( packing->Alignment == 1 ||
	  ( packing->Alignment == 4 &&	/* Pick up the common Q3A case... */
	    format == GL_RGBA && type == GL_UNSIGNED_BYTE ) ) &&
	packing->RowLength == 0 &&
	packing->SkipPixels == 0 &&
	packing->SkipRows == 0 &&
	packing->ImageHeight == 0 &&
	packing->SkipImages == 0 &&
	packing->SwapBytes == GL_FALSE &&
	packing->LsbFirst == GL_FALSE ) {
      return GL_FALSE;
   } else {
      return GL_TRUE;
   }
}


GLboolean
_mesa_convert_texsubimage1d( GLint mesaFormat,
			     GLint xoffset,
			     GLint width,
			     GLenum format, GLenum type,
			     const struct gl_pixelstore_attrib *packing,
			     const GLvoid *srcImage, GLvoid *dstImage )
{
   struct gl_texture_convert convert;

   ASSERT( packing );
   ASSERT( srcImage );
   ASSERT( dstImage );

   ASSERT( mesaFormat >= MESA_FORMAT_RGBA8888 );
   ASSERT( mesaFormat <= MESA_FORMAT_CI8 );

   /* Make it easier to pass all the parameters around.
    */
   convert.xoffset = xoffset;
   convert.yoffset = 0;
   convert.width = width;
   convert.height = 1;
   convert.format = format;
   convert.type = type;
   convert.packing = packing;
   convert.srcImage = srcImage;
   convert.dstImage = dstImage;

   convert.index = 0;

   if ( convert_needs_packing( packing, format, type ) )
      convert.index |= CONVERT_PACKING_BIT;

   return gl_convert_texsubimage2d_tab[mesaFormat]( &convert );
}

GLboolean
_mesa_convert_texsubimage2d( GLint mesaFormat,
			     GLint xoffset, GLint yoffset,
			     GLint width, GLint height,
			     GLint imageWidth,
			     GLenum format, GLenum type,
			     const struct gl_pixelstore_attrib *packing,
			     const GLvoid *srcImage, GLvoid *dstImage )
{
   struct gl_texture_convert convert;

   ASSERT( packing );
   ASSERT( srcImage );
   ASSERT( dstImage );

   ASSERT( mesaFormat >= MESA_FORMAT_RGBA8888 );
   ASSERT( mesaFormat <= MESA_FORMAT_CI8 );

   /* Make it easier to pass all the parameters around.
    */
   convert.xoffset = xoffset;
   convert.yoffset = yoffset;
   convert.width = width;
   convert.height = height;
   convert.imageWidth = imageWidth;
   convert.format = format;
   convert.type = type;
   convert.packing = packing;
   convert.srcImage = srcImage;
   convert.dstImage = dstImage;

   convert.index = 0;

   if ( convert_needs_packing( packing, format, type ) )
      convert.index |= CONVERT_PACKING_BIT;

   if ( width != imageWidth )
      convert.index |= CONVERT_STRIDE_BIT;

   return gl_convert_texsubimage2d_tab[mesaFormat]( &convert );
}

GLboolean
_mesa_convert_texsubimage3d( GLint mesaFormat,
			     GLint xoffset, GLint yoffset, GLint zoffset,
			     GLint width, GLint height, GLint depth,
			     GLint imageWidth, GLint imageHeight,
			     GLenum format, GLenum type,
			     const struct gl_pixelstore_attrib *packing,
			     const GLvoid *srcImage, GLvoid *dstImage )
{
   struct gl_texture_convert convert;

   ASSERT( packing );
   ASSERT( srcImage );
   ASSERT( dstImage );

   ASSERT( mesaFormat >= MESA_FORMAT_RGBA8888 );
   ASSERT( mesaFormat <= MESA_FORMAT_CI8 );

   /* Make it easier to pass all the parameters around.
    */
   convert.xoffset = xoffset;
   convert.yoffset = yoffset;
   convert.zoffset = zoffset;
   convert.width = width;
   convert.height = height;
   convert.depth = depth;
   convert.imageWidth = imageWidth;
   convert.imageHeight = imageHeight;
   convert.format = format;
   convert.type = type;
   convert.packing = packing;
   convert.srcImage = srcImage;
   convert.dstImage = dstImage;

   convert.index = 0;

   if ( convert_needs_packing( packing, format, type ) )
      convert.index |= CONVERT_PACKING_BIT;

   if ( width != imageWidth || height != imageHeight )
      convert.index |= CONVERT_STRIDE_BIT;

   return gl_convert_texsubimage3d_tab[mesaFormat]( &convert );
}



void _mesa_unconvert_teximage1d( GLint mesaFormat, GLenum format, GLint width,
				 const GLvoid *srcImage, GLvoid *dstImage )
{
   struct gl_texture_convert convert;

   ASSERT( srcImage );
   ASSERT( dstImage );

   ASSERT( mesaFormat >= MESA_FORMAT_RGBA8888 );
   ASSERT( mesaFormat <= MESA_FORMAT_CI8 );

   /* Make it easier to pass all the parameters around.
    */
   convert.width = width;
   convert.height = 1;
   convert.depth = 1;
   convert.format = format;
   convert.srcImage = srcImage;
   convert.dstImage = dstImage;

   gl_unconvert_teximage_tab[mesaFormat]( &convert );
}

void _mesa_unconvert_teximage2d( GLint mesaFormat, GLenum format,
				 GLint width, GLint height,
				 const GLvoid *srcImage, GLvoid *dstImage )
{
   struct gl_texture_convert convert;

   ASSERT( srcImage );
   ASSERT( dstImage );

   ASSERT( mesaFormat >= MESA_FORMAT_RGBA8888 );
   ASSERT( mesaFormat <= MESA_FORMAT_CI8 );

   /* Make it easier to pass all the parameters around.
    */
   convert.width = width;
   convert.height = height;
   convert.depth = 1;
   convert.format = format;
   convert.srcImage = srcImage;
   convert.dstImage = dstImage;

   gl_unconvert_teximage_tab[mesaFormat]( &convert );
}

void _mesa_unconvert_teximage3d( GLint mesaFormat, GLenum format,
				 GLint width, GLint height, GLint depth,
				 const GLvoid *srcImage, GLvoid *dstImage )
{
   struct gl_texture_convert convert;

   ASSERT( srcImage );
   ASSERT( dstImage );

   ASSERT( mesaFormat >= MESA_FORMAT_RGBA8888 );
   ASSERT( mesaFormat <= MESA_FORMAT_CI8 );

   /* Make it easier to pass all the parameters around.
    */
   convert.width = width;
   convert.height = height;
   convert.depth = depth;
   convert.format = format;
   convert.srcImage = srcImage;
   convert.dstImage = dstImage;

   gl_unconvert_teximage_tab[mesaFormat]( &convert );
}



/* Nearest filtering only (for broken hardware that can't support
 * all aspect ratios).  This can be made a lot faster, but I don't
 * really care enough...
 */
void _mesa_rescale_teximage2d( GLint texelBytes,
			       GLint srcWidth, GLint srcHeight,
			       GLint dstWidth, GLint dstHeight,
			       const GLvoid *srcImage, GLvoid *dstImage )
{
   GLint row, col;

#define INNER_LOOP( HOP, WOP )						\
   for ( row = 0 ; row < dstHeight ; row++ ) {				\
      GLint srcRow = row HOP hScale;					\
      for ( col = 0 ; col < dstWidth ; col++ ) {			\
	 GLint srcCol = col WOP wScale;					\
	 *dst++ = src[srcRow * srcWidth + srcCol];			\
      }									\
   }									\

#define RESCALE_IMAGE( TYPE )						\
do {									\
   const TYPE *src = (const TYPE *)srcImage;				\
   TYPE *dst = (TYPE *)dstImage;					\
									\
   if ( srcHeight <= dstHeight ) {					\
      const GLint hScale = dstHeight / srcHeight;			\
      if ( srcWidth <= dstWidth ) {					\
	 const GLint wScale = dstWidth / srcWidth;			\
	 INNER_LOOP( /, / );						\
      }									\
      else {								\
	 const GLint wScale = srcWidth / dstWidth;			\
	 INNER_LOOP( /, * );						\
      }									\
   }									\
   else {								\
      const GLint hScale = srcHeight / dstHeight;			\
      if ( srcWidth <= dstWidth ) {					\
	 const GLint wScale = dstWidth / srcWidth;			\
	 INNER_LOOP( *, / );						\
      }									\
      else {								\
	 const GLint wScale = srcWidth / dstWidth;			\
	 INNER_LOOP( *, * );						\
      }									\
   }									\
} while (0)

   switch ( texelBytes ) {
   case 4:
      RESCALE_IMAGE( GLuint );
      break;

   case 2:
      RESCALE_IMAGE( GLushort );
      break;

   case 1:
      RESCALE_IMAGE( GLubyte );
      break;
   }
}
