/* $Id: pb.h,v 1.4 2000/05/10 22:36:05 brianp Exp $ */

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
 */


#ifndef PB_H
#define PB_H


#include "types.h"


/*
 * Pixel buffer size, must be larger than MAX_WIDTH.
 */
#define PB_SIZE (3*MAX_WIDTH)


struct pixel_buffer {
   GLenum primitive;           /* GL_POINT, GL_LINE, GL_POLYGON or GL_BITMAP */
   GLubyte currentColor[4];    /* Current color, for subsequent pixels */
   GLuint currentIndex;        /* Current index, for subsequent pixels */
   GLuint count;               /* Number of pixels in buffer */
   GLboolean mono;             /* Same color or index for all pixels? */

   GLint x[PB_SIZE];           /* X window coord in [0,MAX_WIDTH) */
   GLint y[PB_SIZE];           /* Y window coord in [0,MAX_HEIGHT) */
   GLdepth z[PB_SIZE];         /* Z window coord in [0,Visual.MaxDepth] */
   GLubyte rgba[PB_SIZE][4];   /* Colors */
   GLubyte spec[PB_SIZE][3];   /* Separate specular colors */
   GLuint index[PB_SIZE];      /* Color indexes */
   GLfloat s[MAX_TEXTURE_UNITS][PB_SIZE];	/* Texture S coordinates */
   GLfloat t[MAX_TEXTURE_UNITS][PB_SIZE];	/* Texture T coordinates */
   GLfloat u[MAX_TEXTURE_UNITS][PB_SIZE];	/* Texture R coordinates */
   GLfloat lambda[MAX_TEXTURE_UNITS][PB_SIZE];  /* Texture lambda values */
};



/*
 * Set the color used for all subsequent pixels in the buffer.
 */
#define PB_SET_COLOR( PB, R, G, B, A )		\
do {						\
   if ((PB)->count > 0)				\
      (PB)->mono = GL_FALSE;			\
   (PB)->currentColor[RCOMP] = (R);		\
   (PB)->currentColor[GCOMP] = (G);		\
   (PB)->currentColor[BCOMP] = (B);		\
   (PB)->currentColor[ACOMP] = (A);		\
} while (0)


/*
 * Set the color index used for all subsequent pixels in the buffer.
 */
#define PB_SET_INDEX( PB, I )			\
do {						\
   if ((PB)->count > 0)				\
      (PB)->mono = GL_FALSE;			\
   (PB)->currentIndex = (I);			\
} while (0)


/*
 * "write" a pixel using current color or index
 */
#define PB_WRITE_PIXEL( PB, X, Y, Z )			\
do {							\
   GLuint count = (PB)->count;				\
   (PB)->x[count] = X;					\
   (PB)->y[count] = Y;					\
   (PB)->z[count] = Z;					\
   COPY_4UBV((PB)->rgba[count], (PB)->currentColor);	\
   (PB)->index[count] = (PB)->currentIndex;		\
   (PB)->count++;					\
} while (0)


/*
   (PB)->rgba[count][0] = (PB)->currentColor[0];	\
   (PB)->rgba[count][1] = (PB)->currentColor[1];	\
   (PB)->rgba[count][2] = (PB)->currentColor[2];	\
   (PB)->rgba[count][3] = (PB)->currentColor[3];	\

*/


/*
 * "write" an RGBA pixel
 */
#define PB_WRITE_RGBA_PIXEL( PB, X, Y, Z, R, G, B, A )	\
do {							\
   GLuint count = (PB)->count;				\
   (PB)->x[count] = X;					\
   (PB)->y[count] = Y;					\
   (PB)->z[count] = Z;					\
   (PB)->rgba[count][RCOMP] = R;			\
   (PB)->rgba[count][GCOMP] = G;			\
   (PB)->rgba[count][BCOMP] = B;			\
   (PB)->rgba[count][ACOMP] = A;			\
   (PB)->mono = GL_FALSE;				\
   (PB)->count++;					\
} while (0)


/*
 * "write" a color-index pixel
 */
#define PB_WRITE_CI_PIXEL( PB, X, Y, Z, I )	\
do {						\
   GLuint count = (PB)->count;			\
   (PB)->x[count] = X;				\
   (PB)->y[count] = Y;				\
   (PB)->z[count] = Z;				\
   (PB)->index[count] = I;			\
   (PB)->mono = GL_FALSE;			\
   (PB)->count++;				\
} while (0)



/*
 * "write" an RGBA pixel with texture coordinates
 */
#define PB_WRITE_TEX_PIXEL( PB, X, Y, Z, R, G, B, A, S, T, U )	\
do {							\
   GLuint count = (PB)->count;				\
   (PB)->x[count] = X;					\
   (PB)->y[count] = Y;					\
   (PB)->z[count] = Z;					\
   (PB)->rgba[count][RCOMP] = R;			\
   (PB)->rgba[count][GCOMP] = G;			\
   (PB)->rgba[count][BCOMP] = B;			\
   (PB)->rgba[count][ACOMP] = A;			\
   (PB)->s[0][count] = S;				\
   (PB)->t[0][count] = T;				\
   (PB)->u[0][count] = U;				\
   (PB)->mono = GL_FALSE;				\
   (PB)->count++;					\
} while (0)


/*
 * "write" an RGBA pixel with multiple texture coordinates
 */
#define PB_WRITE_MULTITEX_PIXEL( PB, X, Y, Z, R, G, B, A, S0, T0, U0, S1, T1, U1 )	\
do {							\
   GLuint count = (PB)->count;				\
   (PB)->x[count] = X;					\
   (PB)->y[count] = Y;					\
   (PB)->z[count] = Z;					\
   (PB)->rgba[count][RCOMP] = R;			\
   (PB)->rgba[count][GCOMP] = G;			\
   (PB)->rgba[count][BCOMP] = B;			\
   (PB)->rgba[count][ACOMP] = A;			\
   (PB)->s[0][count] = S0;				\
   (PB)->t[0][count] = T0;				\
   (PB)->u[0][count] = U0;				\
   (PB)->s[1][count] = S1;				\
   (PB)->t[1][count] = T1;				\
   (PB)->u[1][count] = U1;				\
   (PB)->mono = GL_FALSE;				\
   (PB)->count++;					\
} while (0)


/*
 * "write" an RGBA pixel with multiple texture coordinates and specular color
 */
#define PB_WRITE_MULTITEX_SPEC_PIXEL( PB, X, Y, Z, R, G, B, A,	\
			SR, SG, SB, S0, T0, U0, S1, T1, U1 )	\
do {								\
   GLuint count = (PB)->count;					\
   (PB)->x[count] = X;						\
   (PB)->y[count] = Y;						\
   (PB)->z[count] = Z;						\
   (PB)->rgba[count][RCOMP] = R;				\
   (PB)->rgba[count][GCOMP] = G;				\
   (PB)->rgba[count][BCOMP] = B;				\
   (PB)->rgba[count][ACOMP] = A;				\
   (PB)->spec[count][RCOMP] = SR;				\
   (PB)->spec[count][GCOMP] = SG;				\
   (PB)->spec[count][BCOMP] = SB;				\
   (PB)->s[0][count] = S0;					\
   (PB)->t[0][count] = T0;					\
   (PB)->u[0][count] = U0;					\
   (PB)->s[1][count] = S1;					\
   (PB)->t[1][count] = T1;					\
   (PB)->u[1][count] = U1;					\
   (PB)->mono = GL_FALSE;					\
   (PB)->count++;						\
} while (0)



/*
 * Call this function at least every MAX_WIDTH pixels:
 */
#define PB_CHECK_FLUSH( CTX, PB )			\
do {							\
   if ((PB)->count >= PB_SIZE - MAX_WIDTH) {		\
      gl_flush_pb( CTX );				\
   }							\
} while(0)


extern struct pixel_buffer *gl_alloc_pb(void);

extern void gl_flush_pb( GLcontext *ctx );


#endif
