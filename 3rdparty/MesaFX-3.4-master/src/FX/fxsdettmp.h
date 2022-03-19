/* -*- mode: C; tab-width:8; c-basic-offset:2 -*- */

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
 
 
/* 
 * Notes: the folowing code works only if count is > start.
 * Corrently we are looking for the pattern:
 * v0,v1,v2 v2,v1,v3, v2,v3,v4....
 *
 * For this:
 * #define STRIP0		((u1 == v1) && (u2 == v0))	
 * #define STRIP1		((u0 == v0) && (u2 == v1))
 *
 */


static void TAG(render_vb_triangles_smooth_indirect_sd)
	( struct vertex_buffer 	*VB,
	  GLuint 		start,
	  GLuint 		count,
	  GLuint 		parity)
{
   GLint u0,u1,u2;
   GLint v0,v1,v2;
   GLuint *elt = VB->EltPtr->data;
   
   int 	 i;
   LOCAL_VARS
   
   INIT(GL_TRIANGLES);
   
   elt = &elt[start-1];
   u0 = *(++elt);
   u1 = *(++elt);
   u2 = *(++elt);
   i = start+3;
   while (i < count)
   {
      v0 = *(++elt);
      v1 = *(++elt);
      v2 = *(++elt);
   
      if (CLIPPED(u0,u1,u2)) 
      {
      	 if (!CULLED(u0,u1,u2))  SENDCLIPTRI(u0,u1,u2);
      }
      else
      {
	 if (STRIP0(u,v))
         {
            int   is_strips = 1;
            int   parity = 0;
            STRIPSLOCAL_VAR
            
      	    FLUSHTRI();
      	    STARTSTRIPS(u0,u1,u2);
      	    while (is_strips && i < count) 
      	    {
      	       SENDSTRIPS(v2);
      	       
      	       u0 = v0; u1 = v1; u2 = v2; i+= 3;
      	       v0 = *(++elt);	
      	       v1 = *(++elt);
      	       v2 = *(++elt);
      	    
      	       if (parity) {
      	          is_strips = STRIP0(u,v);
      	          parity = 0;
      	       } else {
      	          is_strips  = STRIP1(u,v);
      	          parity = 1;
      	       }
      	     }
      	     FLUSHSTRIPS();
      	     
      	     if (i >= count)
      	     	return;
      	 } 
      	 else 
      	 {
             SENDTRI(u0,u1,u2);
      	 }
      }
      u0 = v0; u1 = v1; u2 = v2; i+= 3;
   }
   if (CLIPPED(u0,u1,u2))
   {
      if (!CULLED(u0,u1,u2))	SENDCLIPTRI(u0,u1,u2);
   }
   else
   {
      SENDTRI(u0,u1,u2);
   }
   FLUSHTRI();
   
}
 
#ifndef PRESERVE_VB_DEFS
#undef SENDTRI
#undef STRIP0
#undef STRIP1
#undef LOCAL_VARS
#undef STRIPSLOCAL_VAR
#undef INIT
#undef SENDTRI
#undef FLUSHTRI
#undef STARTSTRIPS
#undef SENDSTRIPS
#undef FLUSHSTRIPS
#undef CLIPPED
#undef CULLED
#undef SENDCLIPTRI
#endif

#ifndef PRESERVE_TAG
#undef TAG
#endif

#undef PRESERVE_VB_DEFS
#undef PRESERVE_TAG
