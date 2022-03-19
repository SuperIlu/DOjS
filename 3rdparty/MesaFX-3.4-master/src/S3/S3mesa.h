/*
 * Mesa 3-D graphics library
 * Version:  2.3
 * Copyright (C) 1995-1997  Brian Paul
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */



/*
 * S3MESA - S3 Virge driver for Mesa.
 */


#ifndef S3MESA_H
#define S3MESA_H



#ifdef __cplusplus
extern "C" {
#endif

#include "s3dtk.h"


#define S3MESA_MAJOR_VERSION 2
#define S3MESA_MINOR_VERSION 3


/*
 * 1 makes it use the DirectDraw fullscreen
 */
#define USE_FULLSCREEN 0


/*
 * Values for attribList parameter to s3MesaCreateContext():
 */
#define S3MESA_NONE		0	/* to terminate attribList */
#define S3MESA_DOUBLEBUFFER	10
#define S3MESA_ALPHA_SIZE	11      /* followed by an integer */
#define S3MESA_DEPTH_SIZE	12      /* followed by an integer */
#define S3MESA_STENCIL_SIZE	13      /* followed by an integer */
#define S3MESA_ACCUM_SIZE	14      /* followed by an integer */



typedef unsigned long dword;
typedef unsigned short word;
typedef unsigned char byte;


typedef struct s3_mesa_context *s3MesaContext;


s3MesaContext s3MesaCreateContext(  GLuint win,
									int width, int height,
									const GLint attribList[]);

s3MesaContext s3MesaCreateBestContext(  GLuint win,
										GLint width, GLint height,
										const GLint attribList[]);

void s3MesaDestroyContext(s3MesaContext s3ctx);

void s3MesaMakeCurrent(s3MesaContext s3ctx);

s3MesaContext s3MesaGetCurrentContext( void );

void s3MesaSwapBuffers(void);

void s3MesaSetNearFar(GLfloat nearVal, GLfloat farVal);

void s3MesaMoveWindow(s3MesaContext s3ctx, int x, int y);
void s3MesaResizeWindow(s3MesaContext s3ctx, int w, int h);


extern HWND hWND;


#ifdef __cplusplus
}
#endif


#endif
