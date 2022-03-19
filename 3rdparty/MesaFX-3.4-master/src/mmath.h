/* $Id: mmath.h,v 1.13.4.4 2000/12/13 00:57:46 brianp Exp $ */

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
 * Faster arithmetic functions.  If the FAST_MATH preprocessor symbol is
 * defined on the command line (-DFAST_MATH) then we'll use some (hopefully)
 * faster functions for sqrt(), etc.
 */


#ifndef MMATH_H
#define MMATH_H


#include "glheader.h"


/*
 * Set the x86 FPU control word to guarentee only 32 bits of presision
 * are stored in registers.  Allowing the FPU to store more introduces
 * differences between situations where numbers are pulled out of memory
 * vs. situations where the compiler is able to optimize register usage.
 * 
 * In the worst case, we force the compiler to use a memory access to
 * truncate the float, by specifying the 'volatile' keyword.
 */
#if defined(__linux__) && defined(__i386__)
#include <fpu_control.h>

#if !defined(_FPU_SETCW)
#define _FPU_SETCW __setfpucw
typedef unsigned short fpu_control_t;
#endif

#if !defined(_FPU_GETCW)
#define _FPU_GETCW(a) (a) = __fpu_control;
#endif

/* Set it up how we want it.
 */
#if !defined(NO_FAST_MATH) 
#define START_FAST_MATH(x)                  \
   {								\
      static fpu_control_t mask = _FPU_SINGLE | _FPU_MASK_IM	\
            | _FPU_MASK_DM | _FPU_MASK_ZM | _FPU_MASK_OM	\
            | _FPU_MASK_UM | _FPU_MASK_PM;			\
      _FPU_GETCW( x );						\
      _FPU_SETCW( mask );					\
   }
#else
#define START_FAST_MATH(x)			\
   {						\
      static fpu_control_t mask = _FPU_DEFAULT;	\
      _FPU_GETCW( x );				\
      _FPU_SETCW( mask );			\
   }
#endif


/* Put it back how the application had it.
 */
#define END_FAST_MATH(x)			\
   {						\
      _FPU_SETCW( x );				\
   }


#define HAVE_FAST_MATH

#elif defined(__WATCOMC__) && !defined(NO_FAST_MATH) 

/* This is the watcom specific inline assembly version of setcw and getcw */

void START_FAST_MATH2(unsigned short *x);
#pragma aux START_FAST_MATH2 =          \
    "fstcw   word ptr [esi]"            \
    "or      word ptr [esi], 0x3f"      \
    "fldcw   word ptr [esi]"            \
    parm [esi]                          \
    modify exact [];

void END_FAST_MATH2(unsigned short *x);
#pragma aux END_FAST_MATH2 =            \
    "fldcw   word ptr [esi]"            \
    parm [esi]                          \
    modify exact [];

#define START_FAST_MATH(x)  START_FAST_MATH2(& x)          
#define END_FAST_MATH(x)  END_FAST_MATH2(& x)

/*
__inline START_FAST_MATH(unsigned short x)
    {                               
    _asm {                          
        fstcw   ax                  
        mov     x , ax              
        or      ax, 0x3f            
        fldcw   ax                  
        }                           
    }

__inline END_FAST_MATH(unsigned short x)    
    {                               
    _asm {                          
        fldcw   x                   
        }                           
    }
*/
#define HAVE_FAST_MATH

#else
#define START_FAST_MATH(x) (void)(x)
#define END_FAST_MATH(x)   (void)(x)

/* The mac float really is a float, with the same precision as a
 * single precision 387 float.
 */
#if defined(macintosh)
#define HAVE_FAST_MATH
#endif

#endif



/*
 * Float -> Int conversion
 */

#if defined(USE_X86_ASM)
#if defined(__GNUC__) && defined(__i386__)
static __inline__ int FloatToInt(float f)
{
   int r;
   __asm__ ("fistpl %0" : "=m" (r) : "t" (f) : "st");
   return r;
}
#elif  defined(__MSC__) && defined(__WIN32__) && !defined(__CYGWIN__)
static __inline int FloatToInt(float f)
{
   int r;
   _asm {
	 fld f
	 fistp r
	}
   return r;
}
#elif defined(__WATCOMC__)
long FloatToInt(float f);
#pragma aux FloatToInt =                \
	"push   eax"                        \
	"fistp  dword ptr [esp]"            \
	"pop    eax"                        \
	parm [8087]                         \
	value [eax]                         \
	modify exact [eax];
float asm_sqrt (float x);
#pragma aux asm_sqrt =                  \
	"fsqrt"                             \
	parm [8087]                         \
	value [8087]                        \
	modify exact [];
#else
#define FloatToInt(F) ((int) (F))
#endif
#else
#define FloatToInt(F) ((int) (F))
#endif


/*
 * Square root
 */

extern float gl_sqrt(float x);
    
#ifdef FAST_MATH
#if defined (__WATCOMC__) && defined(USE_X86_ASM)
#  define GL_SQRT(X)  asm_sqrt(X)
#else
#  define GL_SQRT(X)  gl_sqrt(X)
#endif
#else
#  define GL_SQRT(X)  sqrt(X)
#endif


/*
 * Normalize a 3-element vector to unit length.
 */
#define NORMALIZE_3FV( V )			\
do {						\
   GLdouble len = LEN_SQUARED_3FV(V);		\
   if (len > 1e-50) {				\
      len = 1.0 / GL_SQRT(len);			\
      V[0] = (GLfloat) (V[0] * len);		\
      V[1] = (GLfloat) (V[1] * len);		\
      V[2] = (GLfloat) (V[2] * len);		\
   }						\
} while(0)

#define LEN_3FV( V ) (GL_SQRT(V[0]*V[0]+V[1]*V[1]+V[2]*V[2]))

#define LEN_SQUARED_3FV( V ) (V[0]*V[0]+V[1]*V[1]+V[2]*V[2])

/*
 * Optimization for:
 * GLfloat f;
 * GLubyte b = FloatToInt(CLAMP(f, 0, 1) * 255)
 */

#if defined(__i386__) || defined(__sparc__) || ( defined(__alpha__) && \
	      ( defined( __IEEE_FLOAT ) || !defined( VMS ) ) )
#define USE_IEEE
#define IEEE_ONE 0x3f7f0000
#endif

#if defined(__GNUC__) && (__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ > 3))
typedef GLint  __attribute__((__may_alias__)) __GLint_a;
typedef GLuint __attribute__((__may_alias__)) __GLuint_a;
#else
typedef GLint  __GLint_a;
typedef GLuint __GLuint_a;
#endif

#if defined(USE_IEEE) && !defined(DEBUG)

#define CLAMP_FLOAT_COLOR(f)			\
	do {					\
	   if (*(__GLuint_a *)&f >= IEEE_ONE)	\
	      f = (*(__GLint_a *)&f < 0) ? 0 : 1;	\
	} while(0)

#define CLAMP_FLOAT_COLOR_VALUE(f)		\
    ( (*(__GLuint_a *)&f >= IEEE_ONE)		\
      ? ((*(__GLint_a *)&f < 0) ? 0 : 1)		\
      : f )

/* 
 * This function/macro is sensitive to precision.  Test carefully
 * if you change it.
 */
#define FLOAT_COLOR_TO_UBYTE_COLOR(b, f)                        \
        do {                                                    \
           union { GLfloat r; GLuint i; } tmp;                  \
           tmp.r = f;                                           \
           b = ((tmp.i >= IEEE_ONE)                             \
               ? ((GLint)tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 \
               : (tmp.r = tmp.r*(255.0F/256.0F) + 32768.0F,     \
                  (GLubyte)tmp.i));                             \
        } while (0)


#define CLAMPED_FLOAT_COLOR_TO_UBYTE_COLOR(b,f) \
         FLOAT_COLOR_TO_UBYTE_COLOR(b, f)

#else

#define CLAMP_FLOAT_COLOR(f) \
        (void) CLAMP_SELF(f,0,1)

#define CLAMP_FLOAT_COLOR_VALUE(f) \
        CLAMP(f,0,1)
       
#define FLOAT_COLOR_TO_UBYTE_COLOR(b, f)			\
	b = ((GLubyte) FloatToInt(CLAMP(f, 0.0F, 1.0F) * 255.0F))

#define CLAMPED_FLOAT_COLOR_TO_UBYTE_COLOR(b,f) \
	b = ((GLubyte) FloatToInt(f * 255.0F))

#endif


extern float gl_ubyte_to_float_color_tab[256];
extern float gl_ubyte_to_float_255_color_tab[256];
#define UBYTE_COLOR_TO_FLOAT_COLOR(c) gl_ubyte_to_float_color_tab[c]

#define UBYTE_COLOR_TO_FLOAT_255_COLOR(c) gl_ubyte_to_float_255_color_tab[c]

#define UBYTE_COLOR_TO_FLOAT_255_COLOR2(f,c) \
    (*(int *)&(f)) = ((int *)gl_ubyte_to_float_255_color_tab)[c]


#define UBYTE_RGBA_TO_FLOAT_RGBA(f,b) 		\
do {						\
   f[0] = UBYTE_COLOR_TO_FLOAT_COLOR(b[0]);	\
   f[1] = UBYTE_COLOR_TO_FLOAT_COLOR(b[1]);	\
   f[2] = UBYTE_COLOR_TO_FLOAT_COLOR(b[2]);	\
   f[3] = UBYTE_COLOR_TO_FLOAT_COLOR(b[3]);	\
} while(0)


#define UBYTE_RGBA_TO_FLOAT_255_RGBA(f,b) 		\
do {						\
   f[0] = UBYTE_COLOR_TO_FLOAT_255_COLOR(b[0]);	\
   f[1] = UBYTE_COLOR_TO_FLOAT_255_COLOR(b[1]);	\
   f[2] = UBYTE_COLOR_TO_FLOAT_255_COLOR(b[2]);	\
   f[3] = UBYTE_COLOR_TO_FLOAT_255_COLOR(b[3]);	\
} while(0)

#define FLOAT_RGBA_TO_UBYTE_RGBA(b,f) 		\
do {						\
   FLOAT_COLOR_TO_UBYTE_COLOR((b[0]),(f[0]));	\
   FLOAT_COLOR_TO_UBYTE_COLOR((b[1]),(f[1]));	\
   FLOAT_COLOR_TO_UBYTE_COLOR((b[2]),(f[2]));	\
   FLOAT_COLOR_TO_UBYTE_COLOR((b[3]),(f[3]));	\
} while(0)

#define FLOAT_RGB_TO_UBYTE_RGB(b,f) 		\
do {						\
   FLOAT_COLOR_TO_UBYTE_COLOR(b[0],f[0]);	\
   FLOAT_COLOR_TO_UBYTE_COLOR(b[1],f[1]);	\
   FLOAT_COLOR_TO_UBYTE_COLOR(b[2],f[2]);	\
} while(0)



extern void
_mesa_init_math(void);


extern GLuint
_mesa_bitcount(GLuint n);


#endif
