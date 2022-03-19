/* $Id: texformat.h,v 1.1.2.1 2001/03/02 16:40:47 gareth Exp $ */

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

#ifndef TEXFORMAT_H
#define TEXFORMAT_H

#include "types.h"


/* The Mesa internal texture image types.  These will be set to their
 * default value, but may be changed by drivers as required.
 */
				/* msb <------ TEXEL BITS -----------> lsb */
enum _format {			/* ---- ---- ---- ---- ---- ---- ---- ---- */
   MESA_FORMAT_RGBA8888,	/* RRRR RRRR GGGG GGGG BBBB BBBB AAAA AAAA */
   MESA_FORMAT_ABGR8888,	/* AAAA AAAA BBBB BBBB GGGG GGGG RRRR RRRR */
   MESA_FORMAT_ARGB8888,	/* AAAA AAAA RRRR RRRR GGGG GGGG BBBB BBBB */
   MESA_FORMAT_RGB888,		/*           RRRR RRRR GGGG GGGG BBBB BBBB */
   MESA_FORMAT_BGR888,		/*           BBBB BBBB GGGG GGGG RRRR RRRR */
   MESA_FORMAT_RGB565,		/*                     RRRR RGGG GGGB BBBB */
   MESA_FORMAT_ARGB4444,	/*                     AAAA RRRR GGGG BBBB */
   MESA_FORMAT_ARGB1555,	/*                     ARRR RRGG GGGB BBBB */
   MESA_FORMAT_AL88,		/*                     AAAA AAAA LLLL LLLL */
   MESA_FORMAT_RGB332,		/*                               RRRG GGBB */
   MESA_FORMAT_A8,		/*                               AAAA AAAA */
   MESA_FORMAT_L8,		/*                               LLLL LLLL */
   MESA_FORMAT_I8,		/*                               IIII IIII */
   MESA_FORMAT_CI8		/*                               CCCC CCCC */
};


extern void
_mesa_init_texture_format( GLcontext *ctx,
			   struct gl_texture_image *texImage,
			   GLenum internalFormat );


extern const struct gl_texture_format _mesa_texformat_rgba8888;
extern const struct gl_texture_format _mesa_texformat_abgr8888;
extern const struct gl_texture_format _mesa_texformat_argb8888;
extern const struct gl_texture_format _mesa_texformat_rgb888;
extern const struct gl_texture_format _mesa_texformat_bgr888;
extern const struct gl_texture_format _mesa_texformat_rgb565;
extern const struct gl_texture_format _mesa_texformat_argb4444;
extern const struct gl_texture_format _mesa_texformat_argb1555;
extern const struct gl_texture_format _mesa_texformat_al88;
extern const struct gl_texture_format _mesa_texformat_rgb332;
extern const struct gl_texture_format _mesa_texformat_a8;
extern const struct gl_texture_format _mesa_texformat_l8;
extern const struct gl_texture_format _mesa_texformat_i8;
extern const struct gl_texture_format _mesa_texformat_ci8;

extern const struct gl_texture_format _mesa_null_texformat;

#endif
