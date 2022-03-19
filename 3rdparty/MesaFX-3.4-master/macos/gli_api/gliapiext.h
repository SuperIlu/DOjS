/* $Id: gliapiext.h,v 1.1 1999/10/31 01:03:45 miklos Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.1
 * 
 * Copyright (C) 1999  Brian Paul   All Rights Reserved.
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
#include <stdio.h>
#include <stdlib.h>
#include "api.h"
#include "context.h"
#include "cva.h"
#include "types.h"
#include "varray.h"
#ifdef XFree86Server
#include "GL/xf86glx.h"
#endif
#endif



/*
 * Extension API functions
 */



/*
 * GL_EXT_blend_minmax
 */

API_PREFIX void API_NAME(BlendEquationEXT)(CTX_PREFIX  GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.BlendEquation)(CC, mode);
}




/*
 * GL_EXT_blend_color
 */

API_PREFIX void API_NAME(BlendColorEXT)(CTX_PREFIX  GLclampf red, GLclampf green,
                               GLclampf blue, GLclampf alpha )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.BlendColor)(CC, red, green, blue, alpha);
}




/*
 * GL_EXT_vertex_array
 */

API_PREFIX void API_NAME(VertexPointerEXT)(CTX_PREFIX  GLint size, GLenum type, GLsizei stride,
                                    GLsizei count, const GLvoid *ptr )
{
   glVertexPointer( size, type, stride, ptr );
   (void) count;
}


API_PREFIX void API_NAME(NormalPointerEXT)(CTX_PREFIX  GLenum type, GLsizei stride, GLsizei count,
                                    const GLvoid *ptr )
{
   glNormalPointer( type, stride, ptr);
   (void) count;
}


API_PREFIX void API_NAME(ColorPointerEXT)(CTX_PREFIX  GLint size, GLenum type, GLsizei stride,
                                   GLsizei count, const GLvoid *ptr )
{
   glColorPointer(size, type, stride, ptr);
   (void) count;
}


API_PREFIX void API_NAME(IndexPointerEXT)(CTX_PREFIX  GLenum type, GLsizei stride,
                                   GLsizei count, const GLvoid *ptr )
{
   glIndexPointer( type, stride, ptr);
   (void) count;
}


API_PREFIX void API_NAME(TexCoordPointerEXT)(CTX_PREFIX  GLint size, GLenum type, GLsizei stride,
                                      GLsizei count, const GLvoid *ptr )
{
   glTexCoordPointer( size, type, stride, ptr);
   (void) count;
}


API_PREFIX void API_NAME(EdgeFlagPointerEXT)(CTX_PREFIX  GLsizei stride, GLsizei count,
                                      const GLboolean *ptr )
{
   glEdgeFlagPointer( stride, ptr);
   (void) count;
}


API_PREFIX void API_NAME(GetPointervEXT)(CTX_PREFIX  GLenum pname, GLvoid **params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetPointerv)(CC, pname, params);
}


/* glArrayElementEXT now hiding in api1.c.
 */


API_PREFIX void API_NAME(DrawArraysEXT)(CTX_PREFIX  GLenum mode, GLint first, GLsizei count )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   gl_DrawArrays(CC, mode, first, count);
}




/*
 * GL_EXT_texture_object
 */

API_PREFIX GLboolean API_NAME(AreTexturesResidentEXT)(CTX_PREFIX  GLsizei n,
                                               const GLuint *textures,
                                               GLboolean *residences )
{
   return glAreTexturesResident( n, textures, residences );
}


API_PREFIX void API_NAME(BindTextureEXT)(CTX_PREFIX  GLenum target, GLuint texture )
{
   glBindTexture( target, texture );
}


API_PREFIX void API_NAME(DeleteTexturesEXT)(CTX_PREFIX  GLsizei n, const GLuint *textures)
{
   glDeleteTextures( n, textures );
}


API_PREFIX void API_NAME(GenTexturesEXT)(CTX_PREFIX  GLsizei n, GLuint *textures )
{
   glGenTextures( n, textures );
}


API_PREFIX GLboolean API_NAME(IsTextureEXT)(CTX_PREFIX  GLuint texture )
{
   return glIsTexture( texture );
}


API_PREFIX void API_NAME(PrioritizeTexturesEXT)(CTX_PREFIX  GLsizei n, const GLuint *textures,
                                         const GLclampf *priorities )
{
   glPrioritizeTextures( n, textures, priorities );
}




/*
 * GL_EXT_texture3D
 */

API_PREFIX void API_NAME(CopyTexSubImage3DEXT)(CTX_PREFIX  GLenum target, GLint level,
                                        GLint xoffset, GLint yoffset,
                                        GLint zoffset,
                                        GLint x, GLint y, GLsizei width,
                                        GLsizei height )
{
   glCopyTexSubImage3D( target, level, xoffset, yoffset, zoffset,
                        x, y, width, height);
}



API_PREFIX void API_NAME(TexImage3DEXT)(CTX_PREFIX  GLenum target, GLint level,
                                 GLenum internalformat,
                                 GLsizei width, GLsizei height, GLsizei depth,
                                 GLint border, GLenum format, GLenum type,
                                 const GLvoid *pixels )
{
   glTexImage3D( target, level, internalformat, width, height, depth,
                 border, format, type, pixels );
}


API_PREFIX void API_NAME(TexSubImage3DEXT)(CTX_PREFIX  GLenum target, GLint level, GLint xoffset,
                                    GLint yoffset, GLint zoffset,
                                    GLsizei width, GLsizei height,
                                    GLsizei depth, GLenum format,
                                    GLenum type, const GLvoid *pixels )
{
   glTexSubImage3D( target, level, xoffset, yoffset, zoffset,
                    width, height, depth, format, type, pixels );
}




/*
 * GL_EXT_point_parameters
 */

API_PREFIX void API_NAME(PointParameterfEXT)(CTX_PREFIX  GLenum pname, GLfloat param )
{
   GLfloat params[3];
   GET_CONTEXT;
   CHECK_CONTEXT;
   params[0] = param;
   params[1] = 0.0;
   params[2] = 0.0;
   (*CC->API.PointParameterfvEXT)(CC, pname, params);
}


API_PREFIX void API_NAME(PointParameterfvEXT)(CTX_PREFIX  GLenum pname, const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PointParameterfvEXT)(CC, pname, params);
}




#ifdef GL_MESA_window_pos
/*
 * Mesa implementation of glWindowPos*MESA()
 */
API_PREFIX void API_NAME(WindowPos4fMESA)(CTX_PREFIX  GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.WindowPos4fMESA)( CC, x, y, z, w );
}
#else
/* Implementation in winpos.c is used */
#endif


API_PREFIX void API_NAME(WindowPos2iMESA)(CTX_PREFIX  GLint x, GLint y )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}

API_PREFIX void API_NAME(WindowPos2sMESA)(CTX_PREFIX  GLshort x, GLshort y )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}

API_PREFIX void API_NAME(WindowPos2fMESA)(CTX_PREFIX  GLfloat x, GLfloat y )
{
   glWindowPos4fMESA( x, y, 0.0F, 1.0F );
}

API_PREFIX void API_NAME(WindowPos2dMESA)(CTX_PREFIX  GLdouble x, GLdouble y )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}

API_PREFIX void API_NAME(WindowPos2ivMESA)(CTX_PREFIX  const GLint *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], 0.0F, 1.0F );
}

API_PREFIX void API_NAME(WindowPos2svMESA)(CTX_PREFIX  const GLshort *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], 0.0F, 1.0F );
}

API_PREFIX void API_NAME(WindowPos2fvMESA)(CTX_PREFIX  const GLfloat *p )
{
   glWindowPos4fMESA( p[0], p[1], 0.0F, 1.0F );
}

API_PREFIX void API_NAME(WindowPos2dvMESA)(CTX_PREFIX  const GLdouble *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], 0.0F, 1.0F );
}

API_PREFIX void API_NAME(WindowPos3iMESA)(CTX_PREFIX  GLint x, GLint y, GLint z )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}

API_PREFIX void API_NAME(WindowPos3sMESA)(CTX_PREFIX  GLshort x, GLshort y, GLshort z )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}

API_PREFIX void API_NAME(WindowPos3fMESA)(CTX_PREFIX  GLfloat x, GLfloat y, GLfloat z )
{
   glWindowPos4fMESA( x, y, z, 1.0F );
}

API_PREFIX void API_NAME(WindowPos3dMESA)(CTX_PREFIX  GLdouble x, GLdouble y, GLdouble z )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}

API_PREFIX void API_NAME(WindowPos3ivMESA)(CTX_PREFIX  const GLint *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], 1.0F );
}

API_PREFIX void API_NAME(WindowPos3svMESA)(CTX_PREFIX  const GLshort *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], 1.0F );
}

API_PREFIX void API_NAME(WindowPos3fvMESA)(CTX_PREFIX  const GLfloat *p )
{
   glWindowPos4fMESA( p[0], p[1], p[2], 1.0F );
}

API_PREFIX void API_NAME(WindowPos3dvMESA)(CTX_PREFIX  const GLdouble *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], 1.0F );
}

API_PREFIX void API_NAME(WindowPos4iMESA)(CTX_PREFIX  GLint x, GLint y, GLint z, GLint w )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}

API_PREFIX void API_NAME(WindowPos4sMESA)(CTX_PREFIX  GLshort x, GLshort y, GLshort z, GLshort w )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}

API_PREFIX void API_NAME(WindowPos4dMESA)(CTX_PREFIX  GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}


API_PREFIX void API_NAME(WindowPos4ivMESA)(CTX_PREFIX  const GLint *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1],
                      (GLfloat) p[2], (GLfloat) p[3] );
}

API_PREFIX void API_NAME(WindowPos4svMESA)(CTX_PREFIX  const GLshort *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1],
                      (GLfloat) p[2], (GLfloat) p[3] );
}

API_PREFIX void API_NAME(WindowPos4fvMESA)(CTX_PREFIX  const GLfloat *p )
{
   glWindowPos4fMESA( p[0], p[1], p[2], p[3] );
}

API_PREFIX void API_NAME(WindowPos4dvMESA)(CTX_PREFIX  const GLdouble *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1],
                      (GLfloat) p[2], (GLfloat) p[3] );
}




/*
 * GL_MESA_resize_buffers
 */

/*
 * Called by user application when window has been resized.
 */
API_PREFIX void API_NAME(ResizeBuffersMESA)( CTX_VOID )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ResizeBuffersMESA)( CC );
}



/*
 * GL_SGIS_multitexture (obsolete - will be removed in near future)
 */

#define TEXCOORD1(s)				\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   count = IM->Count;				\
   IM->Flag[count] |= IM->TF1[texSet];		\
   tc = IM->TexCoordPtr[texSet][count];		\
   ASSIGN_4V(tc, s,0,0,1);			\
}


#define TEXCOORD2(s,t)				\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   count = IM->Count;				\
   IM->Flag[count] |= IM->TF2[texSet];		\
   tc = IM->TexCoordPtr[texSet][count];		\
   ASSIGN_4V(tc, s,t,0,1);			\
}

#define TEXCOORD3(s,t,u)			\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   count = IM->Count;				\
   IM->Flag[count] |= IM->TF3[texSet];		\
   tc = IM->TexCoordPtr[texSet][count];		\
   ASSIGN_4V(tc, s,t,u,1);			\
}

#define TEXCOORD4(s,t,u,v)			\
{						\
   GLuint count;				\
   GLfloat *tc;					\
   count = IM->Count;				\
   IM->Flag[count] |= IM->TF4[texSet];		\
   tc = IM->TexCoordPtr[texSet][count];		\
   ASSIGN_4V(tc, s,t,u,v);			\
}


/* KW: Do the check here so that we only have to do a single range
 *     test.  The possible compliance problem with this is that 
 *     we will throw out error-producing calls when compiling
 *     display lists.  There are ways around this if need be.
 */


/*
 * GL_ARB_multitexture
 */

#define CHECK_ARB							\
   if (target >= GL_TEXTURE0_ARB && target <= GL_TEXTURE1_ARB) {	\
      texSet = target - GL_TEXTURE0_ARB;				\
   }									\
   else {								\
      gl_error(IM->backref, GL_INVALID_ENUM, "glMultiTexCoord(target)");	\
      return;								\
   }

API_PREFIX void API_NAME(ActiveTextureARB)(CTX_PREFIX GLenum texture)
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ActiveTexture)(CC, texture);
}

API_PREFIX void API_NAME(ClientActiveTextureARB)(CTX_PREFIX GLenum texture)
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ClientActiveTexture)(CC, texture);
}

API_PREFIX void API_NAME(MultiTexCoord1dARB)(CTX_PREFIX GLenum target, GLdouble s)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD1( s );
}

API_PREFIX void API_NAME(MultiTexCoord1dvARB)(CTX_PREFIX GLenum target, const GLdouble *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD1( v[0] );
}

API_PREFIX void API_NAME(MultiTexCoord1fARB)(CTX_PREFIX GLenum target, GLfloat s)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD1( s );
}

API_PREFIX void API_NAME(MultiTexCoord1fvARB)(CTX_PREFIX GLenum target, const GLfloat *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD1( v[0] );
}

API_PREFIX void API_NAME(MultiTexCoord1iARB)(CTX_PREFIX GLenum target, GLint s)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD1( s );
}

API_PREFIX void API_NAME(MultiTexCoord1ivARB)(CTX_PREFIX GLenum target, const GLint *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD1( v[0] );
}

API_PREFIX void API_NAME(MultiTexCoord1sARB)(CTX_PREFIX GLenum target, GLshort s)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD1( s );
}

API_PREFIX void API_NAME(MultiTexCoord1svARB)(CTX_PREFIX GLenum target, const GLshort *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD1( v[0] );
}

API_PREFIX void API_NAME(MultiTexCoord2dARB)(CTX_PREFIX GLenum target, GLdouble s, GLdouble t)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD2( s, t );
}

API_PREFIX void API_NAME(MultiTexCoord2dvARB)(CTX_PREFIX GLenum target, const GLdouble *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD2( v[0], v[1] );
}

API_PREFIX void API_NAME(MultiTexCoord2fARB)(CTX_PREFIX GLenum target, GLfloat s, GLfloat t)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD2( s, t );
}

API_PREFIX void API_NAME(MultiTexCoord2fvARB)(CTX_PREFIX GLenum target, const GLfloat *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD2( v[0], v[1] );
}

API_PREFIX void API_NAME(MultiTexCoord2iARB)(CTX_PREFIX GLenum target, GLint s, GLint t)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD2( s, t );
}

API_PREFIX void API_NAME(MultiTexCoord2ivARB)(CTX_PREFIX GLenum target, const GLint *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD2( v[0], v[1] );
}

API_PREFIX void API_NAME(MultiTexCoord2sARB)(CTX_PREFIX GLenum target, GLshort s, GLshort t)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD2( s, t );
}

API_PREFIX void API_NAME(MultiTexCoord2svARB)(CTX_PREFIX GLenum target, const GLshort *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD2( v[0], v[1] );
}

API_PREFIX void API_NAME(MultiTexCoord3dARB)(CTX_PREFIX GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD3( s, t, r );
}

API_PREFIX void API_NAME(MultiTexCoord3dvARB)(CTX_PREFIX GLenum target, const GLdouble *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD3( v[0], v[1], v[2] );
}

API_PREFIX void API_NAME(MultiTexCoord3fARB)(CTX_PREFIX GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD3( s, t, r );
}

API_PREFIX void API_NAME(MultiTexCoord3fvARB)(CTX_PREFIX GLenum target, const GLfloat *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD3( v[0], v[1], v[2] );
}

API_PREFIX void API_NAME(MultiTexCoord3iARB)(CTX_PREFIX GLenum target, GLint s, GLint t, GLint r)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD3( s, t, r );
}

API_PREFIX void API_NAME(MultiTexCoord3ivARB)(CTX_PREFIX GLenum target, const GLint *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD3( v[0], v[1], v[2] );
}

API_PREFIX void API_NAME(MultiTexCoord3sARB)(CTX_PREFIX GLenum target, GLshort s, GLshort t, GLshort r)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD3( s, t, r );
}

API_PREFIX void API_NAME(MultiTexCoord3svARB)(CTX_PREFIX GLenum target, const GLshort *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD3( v[0], v[1], v[2] );
}

API_PREFIX void API_NAME(MultiTexCoord4dARB)(CTX_PREFIX GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD4( s, t, r, q );
}

API_PREFIX void API_NAME(MultiTexCoord4dvARB)(CTX_PREFIX GLenum target, const GLdouble *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD4( v[0], v[1], v[2], v[3] );
}

API_PREFIX void API_NAME(MultiTexCoord4fARB)(CTX_PREFIX GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD4( s, t, r, q );
}


API_PREFIX void API_NAME(MultiTexCoord4fvARB)(CTX_PREFIX GLenum target, const GLfloat *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD4( v[0], v[1], v[2], v[3] );
}

API_PREFIX void API_NAME(MultiTexCoord4iARB)(CTX_PREFIX GLenum target, GLint s, GLint t, GLint r, GLint q)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD4( s, t, r, q );
}

API_PREFIX void API_NAME(MultiTexCoord4ivARB)(CTX_PREFIX GLenum target, const GLint *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD4( v[0], v[1], v[2], v[3] );
}

API_PREFIX void API_NAME(MultiTexCoord4sARB)(CTX_PREFIX GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD4( s, t, r, q );
}

API_PREFIX void API_NAME(MultiTexCoord4svARB)(CTX_PREFIX GLenum target, const GLshort *v)
{
   GLint texSet;
   GET_IMMEDIATE;
   CHECK_ARB
   TEXCOORD4( v[0], v[1], v[2], v[3] );
}



/*
 * GL_INGR_blend_func_separate
 */

API_PREFIX void API_NAME(BlendFuncSeparateINGR)(CTX_PREFIX  GLenum sfactorRGB,
                                         GLenum dfactorRGB,
                                         GLenum sfactorAlpha, 
                                         GLenum dfactorAlpha )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.BlendFuncSeparate)( CC, sfactorRGB, dfactorRGB,
                                 sfactorAlpha, dfactorAlpha );
}



/*
 * GL_PGI_misc_hints - I think this is a bit of a dead extension, but
 * it fits well with what I want the config. file to do.
 */
/*** XXX there is no such entry point in gl.h
API_PREFIX void API_NAME(HintPGI)(CTX_PREFIX  GLenum target, GLint mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Hint)( CC, target, mode );
}
***/


/*
 * GL_EXT_compiled_vertex_array
 */



/*
 * OpenGL 1.2 imaging subset (most not implemented, just stubs)
 */

void GLAPIENTRY
glBlendEquation( GLenum mode )
{
   glBlendEquationEXT( mode );
}


void GLAPIENTRY
glBlendColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
   glBlendColorEXT( red, green, blue, alpha );
}


void GLAPIENTRY
glHistogram( GLenum target, GLsizei width, GLenum internalFormat,
	     GLboolean sink )
{
   GET_CONTEXT;
   (void) target;
   (void) width;
   (void) internalFormat;
   (void) sink;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glHistogram" );
}


void GLAPIENTRY
glResetHistogram( GLenum target )
{
   GET_CONTEXT;
   (void) target;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glResetHistogram" );
}


void GLAPIENTRY
glGetHistogram( GLenum target, GLboolean reset, GLenum format,
		GLenum type, GLvoid *values )
{
   GET_CONTEXT;
   (void) target;
   (void) reset;
   (void) format;
   (void) type;
   (void) values;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glGetHistogram" );
}


void GLAPIENTRY
glGetHistogramParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
   GET_CONTEXT;
   (void) target;
   (void) pname;
   (void) params;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glGetHistogramParameterfv" );
}


void GLAPIENTRY
glGetHistogramParameteriv( GLenum target, GLenum pname, GLint *params )
{
   GET_CONTEXT;
   (void) target;
   (void) pname;
   (void) params;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glGetHistogramParameteriv" );
}


void GLAPIENTRY
glMinmax( GLenum target, GLenum internalFormat, GLboolean sink )
{
   GET_CONTEXT;
   (void) target;
   (void) internalFormat;
   (void) sink;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glMinmax" );
}


void GLAPIENTRY
glResetMinmax( GLenum target )
{
   GET_CONTEXT;
   (void) target;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glResetMinmax" );
}


void GLAPIENTRY
glGetMinMax( GLenum target, GLboolean reset, GLenum format, GLenum types,
             GLvoid *values )
{
   GET_CONTEXT;
   (void) target;
   (void) reset;
   (void) format;
   (void) types;
   (void) values;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glGetMinmax" );
}


void GLAPIENTRY
glGetMinmaxParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
   GET_CONTEXT;
   (void) target;
   (void) pname;
   (void) params;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glGetMinmaxParameterfv" );
}


void GLAPIENTRY
glGetMinmaxParameteriv( GLenum target, GLenum pname, GLint *params )
{
   GET_CONTEXT;
   (void) target;
   (void) pname;
   (void) params;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glGetMinmaxParameteriv" );
}


void GLAPIENTRY
glConvolutionFilter1D( GLenum target, GLenum internalFormat, GLsizei width,
		       GLenum format, GLenum type, const GLvoid *image )
{
   GET_CONTEXT;
   (void) target;
   (void) internalFormat;
   (void) width;
   (void) format;
   (void) type;
   (void) image;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glConvolutionFilter1D" );
}


void GLAPIENTRY
glConvolutionFilter2D( GLenum target, GLenum internalFormat, GLsizei width,
		       GLsizei height, GLenum format, GLenum type,
		       const GLvoid *image )
{
   GET_CONTEXT;
   (void) target;
   (void) internalFormat;
   (void) width;
   (void) height;
   (void) format;
   (void) type;
   (void) image;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glConvolutionFilter2D" );
}


void GLAPIENTRY
glConvolutionParameterf( GLenum target, GLenum pname, GLfloat params )
{
   GET_CONTEXT;
   (void) target;
   (void) pname;
   (void) params;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glConvolutionParameterf" );
}


void GLAPIENTRY
glConvolutionParameterfv( GLenum target, GLenum pname, const GLfloat *params )
{
   GET_CONTEXT;
   (void) target;
   (void) pname;
   (void) params;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glConvolutionParameterfv" );
}


void GLAPIENTRY
glConvolutionParameteri( GLenum target, GLenum pname, GLint params )
{
   GET_CONTEXT;
   (void) target;
   (void) pname;
   (void) params;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glConvolutionParameteri" );
}


void GLAPIENTRY
glConvolutionParameteriv( GLenum target, GLenum pname, const GLint *params )
{
   GET_CONTEXT;
   (void) target;
   (void) pname;
   (void) params;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glConvolutionParameteriv" );
}


void GLAPIENTRY
glCopyConvolutionFilter1D( GLenum target, GLenum internalFormat,
			   GLint x, GLint y, GLsizei width )
{
   GET_CONTEXT;
   (void) target;
   (void) internalFormat;
   (void) x;
   (void) y;
   (void) width;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glCopyConvolutionFilter1D" );
}


void GLAPIENTRY
glCopyConvolutionFilter2D( GLenum target, GLenum internalFormat,
			   GLint x, GLint y, GLsizei width, GLsizei height)
{
   GET_CONTEXT;
   (void) target;
   (void) internalFormat;
   (void) x;
   (void) y;
   (void) width;
   (void) height;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glCopyConvolutionFilter2D" );
}


void GLAPIENTRY
glGetConvolutionFilter( GLenum target, GLenum format,
			GLenum type, GLvoid *image )
{
   GET_CONTEXT;
   (void) target;
   (void) format;
   (void) type;
   (void) image;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glGetConvolutionFilter" );
}


void GLAPIENTRY
glGetConvolutionParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
   GET_CONTEXT;
   (void) target;
   (void) pname;
   (void) params;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glGetConvolutionParameterfv" );
}


void GLAPIENTRY
glGetConvolutionParameteriv( GLenum target, GLenum pname, GLint *params )
{
   GET_CONTEXT;
   (void) target;
   (void) pname;
   (void) params;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glGetConvolutionParameteriv" );
}


void GLAPIENTRY
glSeparableFilter2D( GLenum target, GLenum internalFormat,
		     GLsizei width, GLsizei height, GLenum format,
		     GLenum type, const GLvoid *row, const GLvoid *column )
{
   GET_CONTEXT;
   (void) target;
   (void) internalFormat;
   (void) width;
   (void) height;
   (void) format;
   (void) row;
   (void) type;
   (void) row;
   (void) column;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glSeperableFilter2D" );
}


void GLAPIENTRY
glGetSeparableFilter( GLenum target, GLenum format, GLenum type,
		      GLvoid *row, GLvoid *column, GLvoid *span )
{
   GET_CONTEXT;
   (void) target;
   (void) format;
   (void) type;
   (void) row;
   (void) column;
   (void) span;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glGetSeparableFilter" );
}


void GLAPIENTRY
glCopyColorSubTable( GLenum target, GLsizei start, GLint x, GLint y,
		     GLsizei width )
{
   GET_CONTEXT;
   (void) target;
   (void) start;
   (void) x;
   (void) y;
   (void) width;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glCopyColorSubTable" );
}


void GLAPIENTRY
glCopyColorTable( GLenum target, GLenum internalFormat,
		  GLint x, GLint y, GLsizei width )
{
   GET_CONTEXT;
   (void) target;
   (void) internalFormat;
   (void) x;
   (void) y;
   (void) width;
   CHECK_CONTEXT;
   gl_error( CC, GL_INVALID_OPERATION, "glCopyColorTable" );
}



