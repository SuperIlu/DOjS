/* -*- mode: C; tab-width:8;  -*- */

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


/* fxdos.c -- for MSDOS */


#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#if defined(FX) && defined(__MSDOS__)
#include "fxdrv.h"
#include "GL/gl.h"
#include "GL/fxmesa.h"

#if 0
/* copied from fxwgl.c */

struct __extensions__
{
  fxMesaProc  proc;
  const char *name;
};

static struct __extensions__   ext[] = {
#ifdef GL_EXT_polygon_offset
   { (fxMesaProc)glPolygonOffsetEXT,			"glPolygonOffsetEXT"		},
#endif
   { (fxMesaProc)glBlendEquationEXT,			"glBlendEquationEXT"		},
   { (fxMesaProc)glBlendColorEXT,			"glBlendColorExt"		},
   { (fxMesaProc)glVertexPointerEXT,			"glVertexPointerEXT"		},
   { (fxMesaProc)glNormalPointerEXT,			"glNormalPointerEXT"		},
   { (fxMesaProc)glColorPointerEXT,			"glColorPointerEXT"		},
   { (fxMesaProc)glIndexPointerEXT,			"glIndexPointerEXT"		},
   { (fxMesaProc)glTexCoordPointerEXT,		"glTexCoordPointer"		},
   { (fxMesaProc)glEdgeFlagPointerEXT,		"glEdgeFlagPointerEXT"		},
   { (fxMesaProc)glGetPointervEXT,			"glGetPointervEXT"		},
   { (fxMesaProc)glArrayElementEXT,			"glArrayElementEXT"		},
   { (fxMesaProc)glDrawArraysEXT,			"glDrawArrayEXT"		},
   { (fxMesaProc)glAreTexturesResidentEXT,		"glAreTexturesResidentEXT"	},
   { (fxMesaProc)glBindTextureEXT,			"glBindTextureEXT"		},
   { (fxMesaProc)glDeleteTexturesEXT,			"glDeleteTexturesEXT"		},
   { (fxMesaProc)glGenTexturesEXT,			"glGenTexturesEXT"		},
   { (fxMesaProc)glIsTextureEXT,			"glIsTextureEXT"		},
   { (fxMesaProc)glPrioritizeTexturesEXT,		"glPrioritizeTexturesEXT"	},
   { (fxMesaProc)glCopyTexSubImage3DEXT,		"glCopyTexSubImage3DEXT"	},
   { (fxMesaProc)glTexImage3DEXT,			"glTexImage3DEXT"		},
   { (fxMesaProc)glTexSubImage3DEXT,			"glTexSubImage3DEXT"		},
   { (fxMesaProc)gl3DfxSetPaletteEXT,			"3DFX_set_global_palette"	},
   { (fxMesaProc)gl3DfxSetPaletteEXT,			"gl3DfxSetPaletteEXT"		},
   { (fxMesaProc)glColorTableEXT,			"glColorTableEXT"		},
   { (fxMesaProc)glColorSubTableEXT,			"glColorSubTableEXT"		},
   { (fxMesaProc)glGetColorTableEXT,			"glGetColorTableEXT"		},
   { (fxMesaProc)glGetColorTableParameterfvEXT,	"glGetColorTableParameterfvEXT"	},
   { (fxMesaProc)glGetColorTableParameterivEXT,	"glGetColorTableParameterivEXT"	},
   { (fxMesaProc)glPointParameterfEXT,		"glPointParameterfEXT"		},
   { (fxMesaProc)glPointParameterfvEXT,		"glPointParameterfvEXT"		},
   { (fxMesaProc)glBlendFuncSeparateEXT,		"glBlendFuncSeparateINGR"	},
   { (fxMesaProc)glBlendFuncSeparateEXT,		"glBlendFuncSeparateEXT"	},
   { (fxMesaProc)glActiveTextureARB,                  "glActiveTextureARB"		},
   { (fxMesaProc)glClientActiveTextureARB,            "glClientActiveTextureARB"	},
   { (fxMesaProc)glMultiTexCoord1dARB,                "glMultiTexCoord1dARB"		},
   { (fxMesaProc)glMultiTexCoord1dvARB,               "glMultiTexCoord1dvARB"		},
   { (fxMesaProc)glMultiTexCoord1fARB,                "glMultiTexCoord1fARB"		},
   { (fxMesaProc)glMultiTexCoord1fvARB,               "glMultiTexCoord1fvARB"		},
   { (fxMesaProc)glMultiTexCoord1iARB,                "glMultiTexCoord1iARB"		},
   { (fxMesaProc)glMultiTexCoord1ivARB,               "glMultiTexCoord1ivARB"		},
   { (fxMesaProc)glMultiTexCoord1sARB,                "glMultiTexCoord1sARB"		},
   { (fxMesaProc)glMultiTexCoord1svARB,               "glMultiTexCoord1svARB"		},
   { (fxMesaProc)glMultiTexCoord2dARB,                "glMultiTexCoord2dARB"		},
   { (fxMesaProc)glMultiTexCoord2dvARB,               "glMultiTexCoord2dvARB"		},
   { (fxMesaProc)glMultiTexCoord2fARB,                "glMultiTexCoord2fARB"		},
   { (fxMesaProc)glMultiTexCoord2fvARB,               "glMultiTexCoord2fvARB"		},
   { (fxMesaProc)glMultiTexCoord2iARB,                "glMultiTexCoord2iARB"		},
   { (fxMesaProc)glMultiTexCoord2ivARB,               "glMultiTexCoord2ivARB"		},
   { (fxMesaProc)glMultiTexCoord2sARB,                "glMultiTexCoord2sARB"		},
   { (fxMesaProc)glMultiTexCoord2svARB,               "glMultiTexCoord2svARB"		},
   { (fxMesaProc)glMultiTexCoord3dARB,                "glMultiTexCoord3dARB"		},
   { (fxMesaProc)glMultiTexCoord3dvARB,               "glMultiTexCoord3dvARB"		},
   { (fxMesaProc)glMultiTexCoord3fARB,                "glMultiTexCoord3fARB"		},
   { (fxMesaProc)glMultiTexCoord3fvARB,               "glMultiTexCoord3fvARB"		},
   { (fxMesaProc)glMultiTexCoord3iARB,                "glMultiTexCoord3iARB"		},
   { (fxMesaProc)glMultiTexCoord3ivARB,               "glMultiTexCoord3ivARB"		},
   { (fxMesaProc)glMultiTexCoord3sARB,                "glMultiTexCoord3sARB"		},
   { (fxMesaProc)glMultiTexCoord3svARB,               "glMultiTexCoord3svARB"		},
   { (fxMesaProc)glMultiTexCoord4dARB,                "glMultiTexCoord4dARB"		},
   { (fxMesaProc)glMultiTexCoord4dvARB,               "glMultiTexCoord4dvARB"		},
   { (fxMesaProc)glMultiTexCoord4fARB,                "glMultiTexCoord4fARB"		},
   { (fxMesaProc)glMultiTexCoord4fvARB,               "glMultiTexCoord4fvARB"		},
   { (fxMesaProc)glMultiTexCoord4iARB,                "glMultiTexCoord4iARB"		},
   { (fxMesaProc)glMultiTexCoord4ivARB,               "glMultiTexCoord4ivARB"		},
   { (fxMesaProc)glMultiTexCoord4sARB,                "glMultiTexCoord4sARB"		},
   { (fxMesaProc)glMultiTexCoord4svARB,               "glMultiTexCoord4svARB"		},
   { (fxMesaProc)glLockArraysEXT,			"glLockArraysEXT"		},
   { (fxMesaProc)glUnlockArraysEXT,			"glUnlockArraysEXT"		}
};

static const int qt_ext = sizeof(ext) / sizeof(ext[0]);

fxMesaProc fxMesaGetProcAddress(const char * sym)
{
  int           i;

  for(i = 0;i < qt_ext;i++) {
    if(!strcmp(sym,ext[i].name))
      return(ext[i].proc);
  }
  return NULL;
}

#else
fxMesaProc fxMesaGetProcAddress(const char * sym)
{
   fxMesaProc p = (fxMesaProc)_glapi_get_proc_address(sym);
   if (p == NULL) {
     if (!strcmp(sym,"gl3DfxSetPaletteEXT")) return (fxMesaProc)gl3DfxSetPaletteEXT;
     if (!strcmp(sym,"3DFX_set_global_palette")) return (fxMesaProc)gl3DfxSetPaletteEXT;/* for compat */
   }
   return p;
}
#endif

#endif
