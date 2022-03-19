/* $Id: general_clip.h,v 1.4 2000/04/17 18:18:00 keithw Exp $ */

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

/*
 * New (3.1) transformation code written by Keith Whitwell.
 */


#define PLANE CLIP_RIGHT_BIT
#define CLIP_DOTPROD(K) (- X(K) + W(K))

   GENERAL_CLIP

#undef CLIP_DOTPROD
#undef PLANE


#define PLANE CLIP_LEFT_BIT
#define CLIP_DOTPROD(K) (X(K) + W(K))

   GENERAL_CLIP

#undef CLIP_DOTPROD
#undef PLANE

#define PLANE CLIP_TOP_BIT
#define CLIP_DOTPROD(K) (- Y(K) + W(K))

   GENERAL_CLIP

#undef CLIP_DOTPROD
#undef PLANE

#define PLANE CLIP_BOTTOM_BIT
#define CLIP_DOTPROD(K) (Y(K) + W(K))

   GENERAL_CLIP

#undef CLIP_DOTPROD
#undef PLANE

#define PLANE CLIP_FAR_BIT
#define CLIP_DOTPROD(K) (- Z(K) + W(K))

   if (SIZE >= 3) { 
      GENERAL_CLIP
   }

#undef CLIP_DOTPROD
#undef PLANE

#define PLANE CLIP_NEAR_BIT
#define CLIP_DOTPROD(K) (Z(K) + W(K))

   if (SIZE >=3 ) { 
      GENERAL_CLIP
   }

#undef CLIP_DOTPROD
#undef PLANE
#undef GENERAL_CLIP
