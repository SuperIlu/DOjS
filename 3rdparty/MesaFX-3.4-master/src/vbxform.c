/* $Id: vbxform.c,v 1.23.4.5 2001/05/15 20:21:17 brianp Exp $*/

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
#include "glheader.h"
#include "context.h"
#include "cva.h"
#include "clip.h"
#include "eval.h"
#include "enums.h"
#include "dlist.h"
#include "fog.h"
#include "light.h"
#include "macros.h"
#include "matrix.h"
#include "mem.h"
#include "mmath.h"
#include "pipeline.h"
#include "shade.h"
#include "state.h"
#include "texture.h"
#include "types.h"
#include "varray.h"
#include "vb.h"
#include "vbcull.h"
#include "vbfill.h"
#include "vbrender.h"
#include "vbxform.h"
#include "xform.h"
#endif


void gl_maybe_transform_vb( struct immediate *IM )
{
   GLcontext *ctx = IM->backref;

   if (ctx->NewState)
      gl_update_state(ctx);

   if (IM->FlushElt) {
      gl_exec_array_elements( ctx, IM, IM->LastPrimitive, IM->Count );
      IM->FlushElt = 0;
   }

   gl_compute_orflag( IM );
 
   if (ctx->ExecuteFlag) 
      gl_cva_compile_cassette( ctx, IM );
   else
      gl_fixup_input( ctx, IM );
   
   if (ctx->CompileFlag) 
      gl_compile_cassette( ctx );
   else
      gl_reset_input( ctx );
}


void gl_flush_vb( GLcontext *ctx, const char *where )
{
   struct immediate *IM = ctx->input;

   if (MESA_VERBOSE&VERBOSE_PIPELINE)
      fprintf(stderr, "gl_flush_vb: %s\n", where);

   gl_maybe_transform_vb( IM );
}

/* Drivers can call this inside their swapbuffers routines:
 */
void gl_internal_flush( GLcontext *ctx )
{
   FLUSH_VB( ctx, "internal flush" );
}


#define RESET_VEC(v, t, s, c) (v.start = t v.data[s], v.count = c)  

/* 
 *
 */
void gl_reset_vb( struct vertex_buffer *VB )
{
   GLuint copy;
   GLuint dst;
   GLuint start;
   struct immediate *IM = VB->IM;
   GLubyte clipor = VB->ClipOrMask;

   if (!VB->CullDone)
      gl_fast_copy_vb( VB );
      
   copy = VB->CopyCount;
   start = 3-copy;

   VB->CopyStart = start;
   VB->ClipOrMask = 0;
   VB->ClipAndMask = CLIP_ALL_BITS;

   if (IM) 
   {
      if (VB->Count != IM->Count && 0) {
	 fprintf(stderr, "Trying to copy vertices in the middle of an IM (%d/%d)!!\n",
		 VB->Count, IM->Count);
      }
      else if (VB->pipeline->copy_transformed_data) {

	 for (dst = start ; dst < VB_START ; dst++) 
	 {
	    GLuint src = VB->Copy[dst];
	    
	    if (MESA_VERBOSE&VERBOSE_IMMEDIATE)
	       fprintf(stderr, "copying vertex %u to %u\n", src, dst);
	    
	    COPY_4FV( VB->Clip.data[dst], VEC_ELT(VB->ClipPtr, GLfloat, src) ); 
	    COPY_4FV( VB->Win.data[dst], VB->Win.data[src] ); 
	    
	    VB->UserClipMask[dst] = VB->UserClipMask[src];
	    VB->ClipMask[dst] = (GLubyte) (VB->ClipMask[src] & ~CLIP_CULLED_BIT);
	    VB->ClipAndMask &= VB->ClipMask[dst];
	    VB->ClipOrMask  |= VB->ClipMask[dst];
	    VB->ClipMask[src] = 0;	/* hack for bounds_cull_vb */

	    COPY_4UBV( IM->Color[dst], IM->Color[src] );
	    COPY_4UBV( VB->Spec[0][dst], VB->Spec[0][src] );
	    COPY_4UBV( VB->Spec[1][dst], VB->Spec[1][src] );
	    COPY_4UBV( VB->BColor.data[dst], VB->BColor.data[src] );
	    IM->Index[dst] = IM->Index[src];
	    VB->BIndex.data[dst] = VB->BIndex.data[src];

	    if (VB->TexCoordPtr[0] == &IM->v.TexCoord[0])
	       COPY_4FV( IM->TexCoord[0][dst], IM->TexCoord[0][src] );
	    
	    if (VB->TexCoordPtr[1] == &IM->v.TexCoord[1])
	       COPY_4FV( IM->TexCoord[1][dst], IM->TexCoord[1][src] );

	    IM->Elt[dst] = IM->Elt[src];
	    VB->SavedOrFlag |= IM->Flag[src];
	 } 
      }
      
      VB->CullDone = 0;
   }

   if (clipor & CLIP_USER_BIT) 
      MEMSET(VB->UserClipMask + VB->Start, 0, VB->Count - VB->Start);

   VB->NormCullStart = 0;
   VB->Parity = (VB->LastPrimitive^VB->Count)&1;
   VB->PurgeFlags = 0;
   VB->EarlyCull = 1;
   VB->Culled = 0;
   VB->BoundsPtr = 0;
   VB->NormalLengthPtr = 0;  
   VB->Indirect = 0;
   VB->Culled = 0;
}




/* Copy the untransformed parts of the overlapping vertices from one
 * immediate struct to another (or possibly the same one).  
 *
 * Only copy those elements which are genuinely untransformed.  Others
 * are done in gl_reset_vb.
 */
void gl_copy_prev_vertices( struct vertex_buffer *VB,
			    struct immediate *prev,
			    struct immediate *next )
{
   GLuint dst;
   GLuint flags = VB->pipeline->inputs;

   if (MESA_VERBOSE&VERBOSE_CULL)
      fprintf(stderr, "copy prev vertices im: prev %d next %d copystart %d\n", 
	      prev->id, next->id, VB->CopyStart);

   /* VB_START is correct as vertex copying is only required when an
    * IM wraps, which means that the next VB must start at VB_START.
    */
   for (dst = VB->CopyStart ; dst < VB_START ; dst++) 
   {
      GLuint src = VB->Copy[dst];

      if (MESA_VERBOSE&VERBOSE_CULL)
	 fprintf(stderr, "copy_prev: copy %d to %d\n", src, dst );

      COPY_4FV( next->Obj[dst], prev->Obj[src] );

      if ((flags&VERT_TEX0_ANY) && VB->TexCoordPtr[0] == &prev->v.TexCoord[0])
	 COPY_4FV( next->TexCoord[0][dst], prev->TexCoord[0][src] );
	    
      if ((flags&VERT_TEX1_ANY) && VB->TexCoordPtr[1] == &prev->v.TexCoord[1])
	 COPY_4FV( next->TexCoord[1][dst], prev->TexCoord[1][src] );

      COPY_4UBV( next->Color[dst], prev->Color[src] );
      next->Index[dst] = prev->Index[src];
      next->EdgeFlag[dst] = prev->EdgeFlag[src];

      next->Elt[dst] = prev->Elt[src];
      VB->SavedOrFlag |= prev->Flag[src];
   }
}



void RESET_IMMEDIATE( GLcontext *ctx ) 
{
   if (ctx->VB->prev_buffer != ctx->VB->IM) {
      /* Should only get here if we are trying to use the internal
       * interfaces, eg gl_Vertex3f(), gl_Begin() from inside a
       * display list.  In this case, it is necessary to pull the
       * current values into the ctx->VB.store buffer, because this
       * may not have been done.
       */
      FLUSH_VB( ctx, "RESET_IMMEDIATE" );
      gl_reset_input( ctx );
   }
}


/* Called to initialize new buffers, and to recycle old ones.
 */
void gl_reset_input( GLcontext *ctx )
{
   struct immediate *IM = ctx->input;

   MEMSET(IM->Flag, 0, sizeof(GLuint) * (IM->Count+2));
   IM->Start = VB_START;
   IM->Count = VB_START;
   
   IM->Primitive[IM->Start] = ctx->Current.Primitive;
   IM->LastPrimitive = IM->Start;
   IM->BeginState = VERT_BEGIN_0;         
   IM->OrFlag = 0;
   IM->AndFlag = ~0U;

   if (0)
      fprintf(stderr, 
	   "in reset_input(IM %d), BeginState is %x, setting prim[%d] to %s\n", 
	   IM->id,
	   VERT_BEGIN_0,
	   IM->Start, gl_lookup_enum_by_nr(ctx->Current.Primitive));

#ifdef VAO
   IM->ArrayAndFlags = ~ctx->Array.Current->Flags;
   IM->ArrayIncr = ctx->Array.Current->Vertex.Enabled;
#else
   IM->ArrayAndFlags = ~ctx->Array.Flags;
   IM->ArrayIncr = ctx->Array.Vertex.Enabled;
#endif
   IM->ArrayEltFlush = !(ctx->CompileCVAFlag);
}


/* Preserves size information as well.
 */
static void 
fixup_4f( GLfloat data[][4], GLuint flag[], GLuint start, GLuint match )
{
   GLuint i = start;

   for (;;) {
      if ((flag[++i] & match) == 0) {
	 COPY_4FV(data[i], data[i-1]);
	 flag[i] |= (flag[i-1] & match);
	 if (flag[i] & VERT_END_VB) break;
      } 
   }
}



/* Only needed for buffers with eval coords.
 */
static void
fixup_3f( float data[][3], GLuint flag[], GLuint start, GLuint match )
{
   GLuint i = start;

   for (;;) {
      if ((flag[++i] & match) == 0) {
	 COPY_3V(data[i], data[i-1]);
	 flag[i] |= match;
	 if (flag[i] & VERT_END_VB) break;
      } 
   }
}


static void
fixup_1ui( GLuint *data, GLuint flag[], GLuint start, GLuint match )
{
   GLuint i = start;

   for (;;) {
      if ((flag[++i] & match) == 0) {
	 data[i] = data[i-1];
	 if (flag[i] & VERT_END_VB) break;
      } 
   }
   flag[i] |= match;
}

static void
fixup_1ub( GLubyte *data, GLuint flag[], GLuint start, GLuint match )
{
   GLuint i = start;

   for (;;) {
      if ((flag[++i] & match) == 0) {
	 data[i] = data[i-1];
	 if (flag[i] & VERT_END_VB) break;
      } 
   }
   flag[i] |= match;
}


static void
fixup_4ub( GLubyte data[][4], GLuint flag[], GLuint start, GLuint match )
{
   GLuint i = start;

   for (;;) {
      if ((flag[++i] & match) == 0) {
	 COPY_4UBV(data[i], data[i-1]);
	 if (flag[i] & VERT_END_VB) break;
      } 
   }
   flag[i] |= match;
}


/* Do this instead of fixup for shared normals.
 */
static void
find_last_3f( float data[][3], GLuint flag[], GLuint match, GLuint count )
{
   int i = count;

   do {
      if ((flag[--i] & match) != 0) {
	 COPY_3V(data[count], data[i]);	 
	 return;      
      }
   } while (i >= 0);

   /* To reach this point excercises a bug that seems only to exist on
    * dec alpha installations.  I want to leave this print statement
    * enabled on the 3.3 branch so that we are reminded to track down
    * the problem.
    */
   fprintf(stderr, 
	   "didn't find VERT_NORM in find_last_3f"
	   "(Dec alpha problem?)\n");
}

static void
fixup_first_4v( GLfloat data[][4], GLuint flag[], GLuint match, 
		GLuint start, GLfloat *dflt )
{
   GLuint i = start-1;
   match |= VERT_END_VB;

   while ((flag[++i]&match) == 0)
      COPY_4FV(data[i], dflt);
}


static void
fixup_first_1ui( GLuint data[], GLuint flag[], GLuint match,
		 GLuint start, GLuint dflt )
{
   GLuint i = start-1;
   match |= VERT_END_VB;

   while ((flag[++i]&match) == 0)
      data[i] = dflt;
}


static void
fixup_first_1ub( GLubyte data[], GLuint flag[], GLuint match,
		 GLuint start, GLubyte dflt )
{
   GLuint i = start-1;
   match |= VERT_END_VB;

   while ((flag[++i]&match) == 0)
      data[i] = dflt;
}


static void
fixup_first_4ub( GLubyte data[][4], GLuint flag[], GLuint match,
		 GLuint start, GLubyte dflt[4] )
{
   GLuint i = start-1;
   match |= VERT_END_VB;

   while ((flag[++i]&match) == 0)
      COPY_4UBV(data[i], dflt);
}




static GLuint vertex_sizes[16] = { 0, 
				   1, 
				   2, 2, 
				   3, 3, 3, 3, 
				   4, 4, 4, 4, 4, 4, 4, 4 };



GLuint gl_texcoord_size( GLuint flag, GLuint unit )
{
   flag >>= VERT_TEX0_SHIFT + unit * NR_TEXSIZE_BITS;
   return vertex_sizes[flag & 0xf];
}


static void set_vec_sizes( struct immediate *IM, GLuint orflag )
{
   GLuint i;

   if (orflag & VERT_OBJ_ANY) {
      GLuint szflag = orflag & VERT_OBJ_234;
      IM->v.Obj.size = vertex_sizes[szflag<<1];
   }

   for (i = 0 ; i < MAX_TEXTURE_UNITS ; i++) {
      if (orflag & VERT_TEX_ANY(i)) {
	 GLuint szflag = ((orflag>>(VERT_TEX0_SHIFT+i*NR_TEXSIZE_BITS))
			  & 0xf);
	 IM->v.TexCoord[i].size = vertex_sizes[szflag];
      }
   }
}


void gl_compute_orflag( struct immediate *IM )
{
   GLuint count = IM->Count;
   GLuint orflag = 0;
   GLuint andflag = ~0U;
   GLuint i;

   IM->LastData = count-1;

/*     fprintf(stderr, "In gl_compute_orflag, start %d count %d\n", IM->Start, IM->Count); */
/*     gl_print_vert_flags("initial andflag", andflag); */

   /* Compute the flags for the whole buffer, even if 
    */
   for (i = IM->Start ; i < count ; i++) {
      andflag &= IM->Flag[i];
      orflag |= IM->Flag[i];
   }

   if (IM->Flag[i] & VERT_DATA) {
      IM->LastData++;
/*        andflag &= IM->Flag[i]; */
      orflag |= IM->Flag[i];
   }      

/*     gl_print_vert_flags("final andflag", andflag); */

   IM->Flag[IM->LastData+1] |= VERT_END_VB;
   IM->AndFlag = andflag;
   IM->OrFlag = orflag;
}

void gl_fixup_input( GLcontext *ctx, struct immediate *IM )
{
   GLuint count = IM->Count;
   GLuint start = IM->Start;
   GLuint fixup, diff;
   GLuint andflag = IM->AndFlag;
   GLuint orflag = IM->OrFlag;

   IM->Primitive[count] = IM->Primitive[IM->LastPrimitive];
   IM->NextPrimitive[IM->LastPrimitive] = count;
   IM->NextPrimitive[count] = count+1;

   if (MESA_VERBOSE&VERBOSE_IMMEDIATE) 
   {
      fprintf(stderr, "Start: %u Count: %u LastData: %u\n", 
	      IM->Start, IM->Count, IM->LastData);
      gl_print_vert_flags("Orflag", orflag);
      gl_print_vert_flags("Andflag", andflag);
   }

   /* Array elements modify the current state - must do this before
    * fixup.
    */
   if (ctx->CompileCVAFlag && !(andflag & VERT_ELT)) 
      gl_rescue_cva( ctx, IM );

   if (orflag & VERT_ELT) 
   {
      orflag = IM->OrFlag;
      andflag = IM->AndFlag;
      start = IM->Start;
   }
   
   fixup = ~andflag & VERT_FIXUP;

   if (!ctx->CompileFlag) 
      fixup &= ctx->CVA.elt.inputs; 
   
   if (!ctx->ExecuteFlag)
      fixup &= orflag;

   if (ctx->CompileCVAFlag)
      fixup &= ~ctx->CVA.pre.outputs;

   if ((orflag & (VERT_OBJ_ANY|VERT_EVAL_ANY)) == 0)
      fixup = 0;

   if (fixup) 
   {

      if (ctx->ExecuteFlag && (fixup & ~IM->Flag[start])) {
	 GLuint copy = fixup & ~IM->Flag[start];

	 if (copy & VERT_NORM) 	 
	    COPY_3V( IM->Normal[start], ctx->Current.Normal );
	 
	 if (copy & VERT_RGBA)
	    COPY_4UBV( IM->Color[start], ctx->Current.ByteColor);
      
	 if (copy & VERT_INDEX)
	    IM->Index[start] = ctx->Current.Index;
	 
	 if (copy & VERT_EDGE)
	    IM->EdgeFlag[start] = ctx->Current.EdgeFlag;

	 if (copy & VERT_TEX0_ANY)
	    COPY_4FV( IM->TexCoord[0][start], ctx->Current.Texcoord[0] ); 
	 
	 if (copy & VERT_TEX1_ANY)
	    COPY_4FV( IM->TexCoord[1][start], ctx->Current.Texcoord[1] ); 
      }


      if (MESA_VERBOSE&VERBOSE_IMMEDIATE)
	 gl_print_vert_flags("fixup", fixup);
      
      if (fixup & VERT_TEX0_ANY) {
	 if (orflag & VERT_TEX0_ANY)
	    fixup_4f( IM->TexCoord[0], IM->Flag, start, VERT_TEX0_1234 );
	 else 
	    fixup_first_4v( IM->TexCoord[0], IM->Flag, 0, start, 
			    IM->TexCoord[0][start]);
      }

      if (fixup & VERT_TEX1_ANY) {
	 if (orflag & VERT_TEX1_ANY)
	    fixup_4f( IM->TexCoord[1], IM->Flag, start, VERT_TEX1_1234 );
	 else 
	    fixup_first_4v( IM->TexCoord[1], IM->Flag, 0, start, 
			    IM->TexCoord[1][start] );
      }

      if (fixup & VERT_EDGE) {
	 if (orflag & VERT_EDGE)
	    fixup_1ub( IM->EdgeFlag, IM->Flag, start, VERT_EDGE );
	 else 
	    fixup_first_1ub( IM->EdgeFlag, IM->Flag, 0, start, 
			     IM->EdgeFlag[start] );
      }

      if (fixup & VERT_INDEX) {
	 if (orflag & VERT_INDEX)
	    fixup_1ui( IM->Index, IM->Flag, start, VERT_INDEX );
	 else 
	    fixup_first_1ui( IM->Index, IM->Flag, 0, start, IM->Index[start] );
      }

      if (fixup & VERT_RGBA) {
	 if (orflag & VERT_RGBA) 
	    fixup_4ub( IM->Color, IM->Flag, start, VERT_RGBA );      
	 else 
	    fixup_first_4ub( IM->Color, IM->Flag, 0, start, IM->Color[start] );
      }

      if (fixup & VERT_NORM) {
	 /* Only eval cannot use the Flag member to find valid normals:
	  */
	 if (IM->OrFlag & VERT_EVAL_ANY)
	    fixup_3f( IM->Normal, IM->Flag, start, VERT_NORM );
	 else {
	    /* Copy-to-current requires a valid normal in the last slot:
	     */
	    if ((IM->OrFlag & VERT_NORM) &&
		!(IM->Flag[IM->LastData] & VERT_NORM)) 
	       find_last_3f( IM->Normal, IM->Flag, VERT_NORM, IM->LastData );
	 }
      }
   }

   diff = count - start;
   IM->v.Obj.count = diff;
   IM->v.Normal.count = diff;
   IM->v.TexCoord[0].count = diff;
   IM->v.TexCoord[1].count = diff;
   IM->v.EdgeFlag.count = diff;
   IM->v.Color.count = diff;
   IM->v.Index.count = diff;
 
   /* Prune possible half-filled slot.
    */
   IM->Flag[IM->LastData+1] &= ~VERT_END_VB;
   IM->Flag[IM->Count] |= VERT_END_VB;
}


static void calc_normal_lengths( GLfloat *dest, 
				 CONST GLfloat (*data)[3],
				 const GLuint *flags,
				 GLuint count )
{
   GLuint i;

   for (i = 0 ; i < count ; i++ )
      if (flags[i] & VERT_NORM) {
	 GLfloat tmp = (GLfloat) LEN_3FV( data[i] );
	 dest[i] = 0;
	 if (tmp > 0)
	    dest[i] = 1.0F / tmp;
      }      
}


/* Revive a compiled immediate struct - propogate new 'Current'
 * values.  Often this is redundant because the current values were
 * known and fixed up at compile time.
 */
void gl_fixup_cassette( GLcontext *ctx, struct immediate *IM )
{
   GLuint fixup;
   GLuint count = IM->Count;
   GLuint start = IM->Start;
   
   if (count == start) 
      return;

   if (ctx->NewState)
      gl_update_state( ctx );


   if (ctx->Transform.Normalize && IM->LastCalcedLength < IM->Count) {
      GLuint start = IM->LastCalcedLength;

      if (!IM->NormalLengths)
	 IM->NormalLengths = (GLfloat *)MALLOC(sizeof(GLfloat) * VB_SIZE);

      calc_normal_lengths( IM->NormalLengths + start, 
			   (CONST GLfloat (*)[3])(IM->Normal + start), 
			   IM->Flag + start, 
			   IM->Count - start);
      
      IM->LastCalcedLength = IM->Count;
   }

   fixup = ctx->CVA.elt.inputs & ~IM->AndFlag & VERT_FIXUP;

   if (fixup) {

      if (MESA_VERBOSE & VERBOSE_IMMEDIATE) {
	 fprintf(stderr, "start: %d count: %d\n", start, count);
	 gl_print_vert_flags("fixup_cassette", fixup);
      }

      if (fixup & VERT_TEX0_ANY) 
	 fixup_first_4v( IM->TexCoord[0], IM->Flag, VERT_TEX0_ANY, start,
			 ctx->Current.Texcoord[0] );

      if (fixup & VERT_TEX1_ANY) 
	 fixup_first_4v( IM->TexCoord[1], IM->Flag, VERT_TEX1_ANY, start,
			 ctx->Current.Texcoord[1] );

      if (fixup & VERT_EDGE)
	 fixup_first_1ub(IM->EdgeFlag, IM->Flag, VERT_EDGE, start,
			 ctx->Current.EdgeFlag );

      if (fixup & VERT_INDEX)
	 fixup_first_1ui(IM->Index, IM->Flag, VERT_INDEX, start,
			 ctx->Current.Index );
   
      if (fixup & VERT_RGBA) 
	 fixup_first_4ub(IM->Color, IM->Flag, VERT_RGBA, start,
			 ctx->Current.ByteColor );      

      if ((fixup & VERT_NORM) && !(IM->Flag[start] & VERT_NORM)) {
	 COPY_3V(IM->Normal[start], ctx->Current.Normal);
	 if (ctx->Transform.Normalize)
	    IM->NormalLengths[start] = 1.0F / (GLfloat) LEN_3FV(ctx->Current.Normal);
      }
   }  
}



static void fixup_primitives( struct vertex_buffer *VB, struct immediate *IM )
{
   static GLuint increment[GL_POLYGON+2] = { 1,2,1,1,3,1,1,4,2,1,1 };
   static GLuint intro[GL_POLYGON+2]     = { 0,0,2,2,0,2,2,0,2,2,0 };
   GLcontext *ctx = VB->ctx;
   const GLuint *flags = IM->Flag;
   const GLuint *in_prim = IM->Primitive;
   const GLuint *in_nextprim = IM->NextPrimitive;
   GLuint *out_prim = VB->IM->Primitive;
   GLuint *out_nextprim = VB->IM->NextPrimitive;
   GLuint count = VB->Count;
   GLuint start = VB->Start;
   GLuint in, out, last;
   GLuint incr, prim;   
   GLuint transition;
   GLuint interesting;
   GLuint err;
   
/*     printf("IM: %d VB->Count %d VB->Start %d current prim: %d\n", IM->id, VB->Count, VB->Start, ctx->Current.Primitive);  */

   if (ctx->Current.Primitive == GL_POLYGON+1) {
      transition = VERT_BEGIN;
      err = IM->BeginState & VERT_ERROR_1;
   } else {
      transition = VERT_END;
      err = IM->BeginState & VERT_ERROR_0;
   }

   if (err) {
      /* Occurred somewhere inside the vb.  Don't know/care where/why.
       */
      gl_error( ctx, GL_INVALID_OPERATION, "glBegin/glEnd");
   }

   interesting = transition | VERT_END_VB;


   for (in = start ; in <= count ; in = in_nextprim[in]) 
      if (flags[in] & interesting) 
	 break;

   out = VB->CopyStart;

   if (in == out) {
      out_nextprim[out] = in_nextprim[in];
      out_prim[out] = in_prim[in];
      last = IM->LastPrimitive;
   } else if (flags[in] & transition) {
      out_nextprim[out] = in;
      out_prim[out] = ctx->Current.Primitive;
      out = in;
      last = IM->LastPrimitive;
   } else {
      out_nextprim[out] = in;
      out_prim[out] = ctx->Current.Primitive;
      in++;
      last = out;
   }

   for ( ; in <= count ; in = in_nextprim[in] ) {
      out_prim[in] = in_prim[in];
      out_nextprim[in] = in_nextprim[in];
   }


   VB->Primitive = out_prim;
   VB->NextPrimitive = out_nextprim;
   VB->LastPrimitive = last;
   prim = ctx->Current.Primitive = (GLenum) out_prim[last];


   /* Calculate whether the primitive finished on a 'good number' of
    * vertices, or whether there are overflowing vertices we have to
    * copy to the next buffer (in addition to the normal ones required
    * by a continuing primitive).
    *
    * Note that GL_POLYGON+1, ie outside begin/end, has increment 1.
    */
   incr = increment[prim];
  
   if (incr != 1 && (count - last - intro[prim])) 
      VB->Ovf = (count - last - intro[prim]) % incr; 
   else
      VB->Ovf = 0;

#ifdef MESA_DEBUG
  fprintf(stderr, "prim: %s count %u last %u incr %u ovf: %u\n",
	  gl_prim_name[prim], count, last, incr, VB->Ovf);
#endif
}


void gl_copy_to_current( GLcontext *ctx, struct immediate *IM )
{
   GLuint count = IM->LastData;
   GLuint flag = IM->OrFlag;
   GLuint mask = 0;

   if (MESA_VERBOSE&VERBOSE_IMMEDIATE)
      gl_print_vert_flags("copy to current", flag);

   if (flag & VERT_NORM) 
      COPY_3FV( ctx->Current.Normal, IM->Normal[count]);   
   
   if (flag & VERT_INDEX)
      ctx->Current.Index = IM->Index[count];

   if (flag & VERT_EDGE)
      ctx->Current.EdgeFlag = IM->EdgeFlag[count];

   if (flag & VERT_RGBA) 
      COPY_4UBV(ctx->Current.ByteColor, IM->Color[count]);

   if (flag & VERT_TEX0_ANY) {
      mask |= VERT_TEX0_1234;
      COPY_4FV( ctx->Current.Texcoord[0], IM->TexCoord[0][count]);
   }
      
   if (flag & VERT_TEX1_ANY) {
      mask |= VERT_TEX1_1234;
      COPY_4FV( ctx->Current.Texcoord[1], IM->TexCoord[1][count]);
   }

   /* Save the texcoord size information as well.
    */
   ctx->Current.Flag &= ~mask;
   ctx->Current.Flag |= IM->Flag[count] & mask;
}


void gl_execute_cassette( GLcontext *ctx, struct immediate *IM )
{
   struct vertex_buffer *VB = ctx->VB;
   struct immediate *prev = VB->prev_buffer;
   GLuint vec_start, diff;

   IM->ref_count++;

   if (prev != IM || IM != VB->IM) {
      gl_copy_prev_vertices( VB, VB->prev_buffer, IM );
   }

   if (! --prev->ref_count ) 
      gl_immediate_free( prev );

   VB->prev_buffer = IM;
   VB->Start = IM->Start;
   VB->Count = IM->Count;
   VB->Flag = IM->Flag;
   VB->OrFlag = IM->OrFlag | VB->SavedOrFlag;
   VB->EltPtr = &IM->v.Elt;
   VB->MaterialMask = IM->MaterialMask;
   VB->Material = IM->Material;
   VB->CullMode = (GLubyte) ((IM->AndFlag & VERT_NORM) ? 0 : COMPACTED_NORMALS);
   VB->ObjPtr = &IM->v.Obj;
   VB->NormalPtr = &IM->v.Normal;
   VB->ColorPtr = &IM->v.Color;
   VB->Color[0] = VB->Color[1] = VB->ColorPtr;
   VB->IndexPtr = &IM->v.Index;
   VB->Index[0] = VB->Index[1] = VB->IndexPtr;
   VB->EdgeFlagPtr = &IM->v.EdgeFlag;
   VB->TexCoordPtr[0] = &IM->v.TexCoord[0];
   VB->TexCoordPtr[1] = &IM->v.TexCoord[1];
/*     VB->BoundsPtr = IM->Bounds; */
   VB->NormalLengthPtr = IM->NormalLengths;
   VB->IndirectCount = VB->Count;
   VB->SavedOrFlag = 0;

   if (IM->Start != VB_START)
      VB->CopyStart = IM->Start;

   vec_start = IM->Start;
   if (vec_start == VB_START && VB->pipeline->replay_copied_vertices)
      vec_start = VB->CopyStart;

   if (MESA_VERBOSE&VERBOSE_IMMEDIATE)
      fprintf(stderr, "reseting vectors to %d/%d .. %d\n", vec_start, IM->Start, IM->Count);

   VB->LastPrimitive = IM->Start; 

   diff = IM->Count - vec_start;

   RESET_VEC(IM->v.Obj, (GLfloat *), vec_start, diff);
   RESET_VEC(IM->v.Normal, (GLfloat *), vec_start, diff);
   RESET_VEC(IM->v.TexCoord[0], (GLfloat *), vec_start, diff);
   RESET_VEC(IM->v.TexCoord[1], (GLfloat *), vec_start, diff);
   RESET_VEC(IM->v.Index, &, vec_start, diff);
   RESET_VEC(IM->v.Elt, &, vec_start, diff);
   RESET_VEC(IM->v.EdgeFlag, &, vec_start, diff);
   RESET_VEC(IM->v.Color, (GLubyte *), vec_start, diff);
   RESET_VEC(VB->Clip, (GLfloat *), vec_start, diff);
   RESET_VEC(VB->Eye, (GLfloat *), vec_start, diff);
   RESET_VEC(VB->Win, (GLfloat *), vec_start, diff);
   RESET_VEC(VB->BColor, (GLubyte *), vec_start, diff); 
   RESET_VEC(VB->BIndex, &, vec_start, diff);

   if (IM != VB->IM) {
      RESET_VEC(VB->IM->v.Obj, (GLfloat *), vec_start, diff);
      RESET_VEC(VB->IM->v.Normal, (GLfloat *), vec_start, diff);
      RESET_VEC(VB->IM->v.TexCoord[0], (GLfloat *), vec_start, diff);
      RESET_VEC(VB->IM->v.TexCoord[1], (GLfloat *), vec_start, diff);
      RESET_VEC(VB->IM->v.Index, &, vec_start, diff);
      RESET_VEC(VB->IM->v.Elt, &, vec_start, diff);
      RESET_VEC(VB->IM->v.EdgeFlag, &, vec_start, diff);
      RESET_VEC(VB->IM->v.Color, (GLubyte *), vec_start, diff);
   }

   gl_copy_to_current( ctx, IM );

   set_vec_sizes( IM, VB->OrFlag );

   if (MESA_VERBOSE&VERBOSE_IMMEDIATE) 
      fprintf(stderr,
	      "executing cassette, rows %u tc0->size == %u tc1->size == %u\n",
	      VB->Count,
	      VB->TexCoordPtr[0]->size,
	      VB->TexCoordPtr[1]->size);

   if (MESA_VERBOSE&VERBOSE_IMMEDIATE) 
      gl_print_cassette( IM );

   
   if (IM->OrFlag & VERT_EVAL_ANY) 
      gl_eval_vb( VB );

   if (IM->Count > IM->Start || (IM->Flag[IM->Start] & (VERT_END|VERT_BEGIN)))
      fixup_primitives( VB, IM );

   if (VB->IndirectCount > IM->Start) 
      gl_run_pipeline( VB );      
   else 
      gl_update_materials( VB );

   /* This is unfortunate:
    */
   if (VB->pipeline->replay_copied_vertices) {
      if (!VB->CullDone)
	 gl_fast_copy_vb( VB );

      gl_copy_prev_vertices( VB, VB->prev_buffer, IM );
   }

   gl_reset_vb( VB );
}

#ifdef MESA_DEBUG
void gl_print_cassette( struct immediate *IM )
{
   gl_print_cassette_flags( IM, IM->Flag );
}

void gl_print_cassette_flags( struct immediate *IM, GLuint *flags ) 
{
   GLuint i;
   GLuint andflag = IM->AndFlag;
   GLuint orflag = IM->OrFlag;
   GLuint state = IM->BeginState;
   GLuint req = ~0;
   static const char *tplate[5] = { "%s ", 
				    "%s: %f ",
				    "%s: %f %f ",
				    "%s: %f %f %f ",
				    "%s: %f %f %f %f " };

   fprintf(stderr, "Cassette id %d, %u rows.\n", IM->id, IM->Count - IM->Start);

   gl_print_vert_flags("Contains at least one", orflag);

   if (IM->Count != IM->Start) 
   {
      gl_print_vert_flags("Contains a full complement of", andflag);
   
      fprintf(stderr, "Final begin/end state %s/%s, errors %s/%s\n",
	     (state & VERT_BEGIN_0) ? "in" : "out", 
	     (state & VERT_BEGIN_1) ? "in" : "out", 
	     (state & VERT_ERROR_0) ? "y" : "n", 
	     (state & VERT_ERROR_1) ? "y" : "n");

      fprintf(stderr, "Obj size: %u, TexCoord0 size: %u, TexCoord1 size: %u\n",
	     IM->v.Obj.size,
	     IM->v.TexCoord[0].size,
	     IM->v.TexCoord[1].size);
   }

   for (i = IM->Start ; i <= IM->Count ; i++) {
      fprintf(stderr, "%u: ", i);
      if (req & VERT_OBJ_ANY) {
	 if (flags[i] & VERT_EVAL_C1) 
	    fprintf(stderr, "EvalCoord %f ", IM->Obj[i][0]);
	 else if (flags[i] & VERT_EVAL_P1) 
	    fprintf(stderr, "EvalPoint %.0f ", IM->Obj[i][0]);
	 else if (flags[i] & VERT_EVAL_C2) 
	    fprintf(stderr, "EvalCoord %f %f ", IM->Obj[i][0], IM->Obj[i][1]);
	 else if (flags[i] & VERT_EVAL_P2) 
	    fprintf(stderr, "EvalPoint %.0f %.0f ", IM->Obj[i][0], IM->Obj[i][1]);
	 else if (i < IM->Count && (flags[i]&VERT_OBJ_234)) {
	    fprintf(stderr, "(%x) ", flags[i] & VERT_OBJ_234);
	    fprintf(stderr, tplate[vertex_sizes[(flags[i]&VERT_OBJ_234)<<1]],
		   "Obj",
		   IM->Obj[i][0], IM->Obj[i][1], IM->Obj[i][2], IM->Obj[i][3]);
	 }
      }

      if (req & flags[i] & VERT_ELT) 
	 fprintf(stderr, " Elt %u\t", IM->Elt[i]);

      if (req & flags[i] & VERT_NORM)
	 fprintf(stderr, " Norm %f %f %f ", 
		IM->Normal[i][0], IM->Normal[i][1], IM->Normal[i][2]);

      if (req & flags[i] & VERT_TEX0_ANY)
	 fprintf(stderr, tplate[vertex_sizes[(flags[i]>>VERT_TEX0_SHIFT)&7]], 
		"TC0",
		IM->TexCoord[0][i][0], IM->TexCoord[0][i][1], 
		IM->TexCoord[0][i][2], IM->TexCoord[0][i][2]);


      if (req & flags[i] & VERT_TEX1_ANY)
	 fprintf(stderr, tplate[vertex_sizes[(flags[i]>>(VERT_TEX0_SHIFT+4))&7]], 
		"TC1",
		IM->TexCoord[1][i][0], IM->TexCoord[1][i][1], 
		IM->TexCoord[1][i][2], IM->TexCoord[1][i][2]);

      if (req & flags[i] & VERT_RGBA)
	 fprintf(stderr, " Rgba %d %d %d %d ", 
		IM->Color[i][0], IM->Color[i][1], 
		IM->Color[i][2], IM->Color[i][3]);

      if (req & flags[i] & VERT_INDEX)
	 fprintf(stderr, " Index %u ", IM->Index[i]);
      
      if (req & flags[i] & VERT_EDGE)
	 fprintf(stderr, " Edgeflag %d ", IM->EdgeFlag[i]);

      if (req & flags[i] & VERT_MATERIAL)
	 fprintf(stderr, " Material ");
	 

      /* The order of these two is not easily knowable, but this is
       * the usually correct way to look at them.
       */
      if (req & flags[i] & VERT_END) 
	 fprintf(stderr, " END ");

      if (req & flags[i] & VERT_BEGIN) 
	 fprintf(stderr, " BEGIN(%s) ", gl_prim_name[IM->Primitive[i]]);

      fprintf(stderr, "\n");  
   }      
}
#endif
