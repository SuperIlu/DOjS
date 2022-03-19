/* $Id: mmath.c,v 1.7 2000/04/12 00:28:14 brianp Exp $ */

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


#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "mmath.h"
#endif


static int in_fast_math;

/*
 * A High Speed, Low Precision Square Root
 * by Paul Lalonde and Robert Dawson
 * from "Graphics Gems", Academic Press, 1990
 */

/*
 * SPARC implementation of a fast square root by table 
 * lookup.
 * SPARC floating point format is as follows:
 *
 * BIT 31 	30 	23 	22 	0
 *     sign	exponent	mantissa
 */
static short sqrttab[0x100];    /* declare table of square roots */

static void init_sqrt(void)
{
#ifdef FAST_MATH
   unsigned short i;
   float f;
   unsigned int *fi = (unsigned int *)&f;
                                /* to access the bits of a float in  */
                                /* C quickly we must misuse pointers */

   for(i=0; i<= 0x7f; i++) {
      *fi = 0;

      /*
       * Build a float with the bit pattern i as mantissa
       * and an exponent of 0, stored as 127
       */

      *fi = (i << 16) | (127 << 23);
      f = sqrt(f);

      /*
       * Take the square root then strip the first 7 bits of
       * the mantissa into the table
       */

      sqrttab[i] = (*fi & 0x7fffff) >> 16;

      /*
       * Repeat the process, this time with an exponent of
       * 1, stored as 128
       */

      *fi = 0;
      *fi = (i << 16) | (128 << 23);
      f = sqrt(f);
      sqrttab[i+0x80] = (*fi & 0x7fffff) >> 16;
   }
#else
   (void) sqrttab;  /* silence compiler warnings */
#endif /*FAST_MATH*/
}


float gl_sqrt( float x )
{
#ifdef FAST_MATH
   unsigned int *num = (unsigned int *)&x;
                                /* to access the bits of a float in C
                                 * we must misuse pointers */
                                                        
   short e;                     /* the exponent */
   if (x == 0.0F) return 0.0F;  /* check for square root of 0 */
   e = (*num >> 23) - 127;      /* get the exponent - on a SPARC the */
                                /* exponent is stored with 127 added */
   *num &= 0x7fffff;            /* leave only the mantissa */
   if (e & 0x01) *num |= 0x800000;
                                /* the exponent is odd so we have to */
                                /* look it up in the second half of  */
                                /* the lookup table, so we set the   */
                                /* high bit                                */
   e >>= 1;                     /* divide the exponent by two */
                                /* note that in C the shift */
                                /* operators are sign preserving */
                                /* for signed operands */
   /* Do the table lookup, based on the quaternary mantissa,
    * then reconstruct the result back into a float
    */
   *num = ((sqrttab[*num >> 16]) << 16) | ((e + 127) << 23);
   return x;
#else
   return sqrt(x);
#endif
}

float gl_ubyte_to_float_color_tab[256];
float gl_ubyte_to_float_255_color_tab[256];

static void
init_ubyte_color_tab(void)
{
   int i;
   for (i = 0 ; i < 256 ; i++) {
      gl_ubyte_to_float_color_tab[i] = (float) i * (1.0/255.0);
      gl_ubyte_to_float_255_color_tab[i] = (float) i;
   }
}

/*
 * Initialize tables, etc for fast math functions.
 */
void
_mesa_init_math(void)
{
   static GLboolean initialized = GL_FALSE;

   if (!initialized) {
      init_sqrt();
      init_ubyte_color_tab();

      initialized = GL_TRUE;
      in_fast_math = 0;

#if defined(_FPU_GETCW) && defined(_FPU_SETCW)
      {
         const char *debug = getenv("MESA_DEBUG");
         if (debug && strcmp(debug, "FP")==0) {
            /* die on FP exceptions */
            fpu_control_t mask;
            _FPU_GETCW(mask);
            mask &= ~(_FPU_MASK_IM | _FPU_MASK_DM | _FPU_MASK_ZM
                      | _FPU_MASK_OM | _FPU_MASK_UM);
            _FPU_SETCW(mask);
         }
      }
#endif
   }
}



/*
 * Return number of bits set in given GLuint.
 */
GLuint
_mesa_bitcount(GLuint n)
{
   GLuint bits;
   for (bits = 0; n > 0; n = n >> 1) {
      if (n & 1) {
         bits++;
      }
   }
   return bits;
}

