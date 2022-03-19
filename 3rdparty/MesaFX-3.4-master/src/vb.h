/* $Id: vb.h,v 1.6.4.1 2000/10/18 15:12:07 brianp Exp $ */

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



/* OVERVIEW: 
 *
 * The vertices between glBegin() and glEnd() are accumulated in the
 * vertex buffer.  When either the vertex buffer becomes filled or a
 * state change outside the glBegin()/glEnd() is made, we must flush
 * the buffer.
 * 
 * That is, we apply the vertex transformations, compute lighting,
 * fog, texture coordinates etc.  Then, we can render the vertices as
 * points, lines or polygons by calling the gl_render_vb() function in
 * render.c
 *
 * When we're outside of a glBegin/glEnd pair the information in this
 * structure is retained pending either of the flushing events
 * described above.  
 */


#ifndef VB_H
#define VB_H


#include "glheader.h"
#include "vector.h"
#include "matrix.h"
#include "config.h"


enum {
   VB_IMMEDIATE,
   VB_CVA_PRECALC
};



/* 
 */
struct vertex_data
{
   GLfloat (*Obj)[4];
   GLfloat (*Normal)[3];
   GLubyte (*Color)[4];
   GLuint   *Index;
   GLubyte  *EdgeFlag;
   GLfloat (*TexCoord[MAX_TEXTURE_UNITS])[4];
   GLuint   *Elt;
};

struct vertex_arrays
{
   GLvector4f  Obj;
   GLvector3f  Normal;
   GLvector4ub Color;
   GLvector1ui Index;
   GLvector1ub EdgeFlag;
   GLvector4f  TexCoord[MAX_TEXTURE_UNITS];
   GLvector1ui Elt;     
};

struct vertex_array_pointers
{
   GLvector4f  *Obj;
   GLvector3f  *Normal;
   GLvector4ub *Color;
   GLvector1ui *Index;
   GLvector1ub *EdgeFlag;
   GLvector4f  *TexCoord[MAX_TEXTURE_UNITS];
   GLvector1ui *Elt;     
};


/* Move to using pointers to this struct in the immediate structs -
 * this is too big to keep 94 unused copies (7K) lying around in
 * display lists.  
 */
struct gl_material 
{
   GLfloat Ambient[4];
   GLfloat Diffuse[4];
   GLfloat Specular[4];
   GLfloat Emission[4];
   GLfloat Shininess;
   GLfloat AmbientIndex;	/* for color index lighting */
   GLfloat DiffuseIndex;	/* for color index lighting */
   GLfloat SpecularIndex;	/* for color index lighting */
};


/* KW: Represents everything that can take place between a begin and
 * end, and can represent multiple begin/end pairs.  This plus *any*
 * state variable (GLcontext) should be all you need to replay the
 * represented begin/end pairs as if they took place in that state.  
 *
 * Thus this is sufficient for both immediate and compiled modes, but
 * we could/should throw some elements away for compiled mode if we
 * know they were empty. 
 */
struct immediate 
{ 
   struct immediate *next;	/* for cache of free IM's */
   GLuint id, ref_count;

   /* This must be saved when immediates are shared in display lists.
    */
   GLuint Start, Count;
   GLuint LastData;		/* count or count+1 */
   GLuint AndFlag, OrFlag, BeginState;
   GLuint LastPrimitive;	

   GLuint ArrayAndFlags;	/* precalc'ed for glArrayElt */
   GLuint ArrayIncr;
   GLuint ArrayEltFlush;
   GLuint FlushElt;

   GLuint TF1[MAX_TEXTURE_UNITS];	/* precalc'ed for glTexCoord */
   GLuint TF2[MAX_TEXTURE_UNITS];
   GLuint TF3[MAX_TEXTURE_UNITS];
   GLuint TF4[MAX_TEXTURE_UNITS];

   GLuint  Primitive[VB_SIZE];	/* GLubyte would do... */
   GLuint  NextPrimitive[VB_SIZE]; 

   /* allocate storage for these on demand:
    */
   struct  gl_material (*Material)[2]; 
   GLuint  *MaterialMask;       

   GLfloat (*TexCoordPtr[MAX_TEXTURE_UNITS])[4]; 

   struct vertex_arrays v;
   
   struct gl_context *backref;		    
   void (*maybe_transform_vb)( struct immediate * );

   /* Normal lengths, zero if not available.
    */
   GLfloat   *NormalLengths;
   GLuint     LastCalcedLength;

   GLuint  Flag[VB_SIZE];
   GLubyte Color[VB_SIZE][4];
   GLfloat Obj[VB_SIZE][4];
   GLfloat Normal[VB_SIZE][3];
   GLfloat TexCoord[MAX_TEXTURE_UNITS][VB_SIZE][4];
   GLuint  Elt[VB_SIZE];
   GLubyte EdgeFlag[VB_SIZE];
   GLuint  Index[VB_SIZE];
};




/* Not so big on storage these days, although still has pointers to
 * arrays used for temporary results.
 */
struct vertex_buffer
{
   /* Pointers to enable multiple vertex_buffers - required for
    * CVA, should also be useful for the PMesa people.
    *
    * Driver_data is alloc'ed in Driver.RegisterVB(), if required.
    */
   struct gl_context *ctx;
   struct gl_pipeline *pipeline;
   void *driver_data;


   /* Data easily accessible by immediate mode fuctions: There is no
    * guarentee on the driver side that processed data will end up or
    * even pass through here.  Use the GLvector pointers below.
    *
    * If we are not compiling, ctx->input points to this struct, in which
    * case the values will be scribbled during transform_vb.
    */
   struct immediate *IM;	        
   struct vertex_array_pointers store;	

   /* Where to find outstanding untransformed vertices.
    */
   struct immediate *prev_buffer;


   GLuint     Type;
   GLuint     Size, Start, Count;
   GLuint     Free, FirstFree;
   GLuint     CopyStart;
   GLuint     Parity, Ovf;
   GLuint     PurgeFlags;
   GLuint     IndirectCount;	/* defaults to count */
   GLuint     OrFlag, SavedOrFlag;
   GLuint     EarlyCull;
   GLuint     Culled, CullDone;

   /* Pointers to input data - default to buffers in 'im' above.
    */
   GLvector4f  *ObjPtr;
   GLvector3f  *NormalPtr;
   GLvector4ub *ColorPtr;
   GLvector1ui *IndexPtr;
   GLvector1ub *EdgeFlagPtr;
   GLvector4f  *TexCoordPtr[MAX_TEXTURE_UNITS];
   GLvector1ui *EltPtr;
   GLuint      *Flag, FlagMax;
   struct      gl_material (*Material)[2];
   GLuint      *MaterialMask;       

   GLuint      *NextPrimitive; 
   GLuint      *Primitive;     
   GLuint      LastPrimitive;

   GLfloat (*BoundsPtr)[3];	/* Bounds for cull check */
   GLfloat  *NormalLengthPtr;	/* Array of precomputed inv. normal lengths */
   

   /* Holds malloced storage for pipeline data not supplied by 
    * the immediate struct.
    */
   GLvector4f Eye;
   GLvector4f Clip;
   GLvector4f Win;
   GLvector4ub BColor;		/* not used in cva */
   GLvector1ui BIndex;		/* not used in cva */
   GLubyte (*Specular)[4];
   GLubyte (*Spec[2])[4];

   /* Temporary storage - may point into IM, or be dynamically
    * allocated (for cva).  
    */
   GLubyte *ClipMask;
   GLubyte *UserClipMask;
   
   /* Internal values.  Where these point depends on whether
    * there were any identity matrices defined as transformations
    * in the pipeline.
    */
   GLvector4f *EyePtr;
   GLvector4f *ClipPtr;
   GLvector4f *Unprojected;
   GLvector4f *Projected;
   GLvector4f *CurrentTexCoord;
   GLuint     *Indirect;           /* For eval rescue and cva render */


   /* Currently active colors
    */
   GLvector4ub *Color[2];
   GLvector1ui *Index[2];

   /* Storage for colors which have been lit but not yet fogged.  
    * Required for CVA, just point into store for normal VB's.
    */
   GLvector4ub *LitColor[2];
   GLvector1ui *LitIndex[2];
   GLvector4ub *FoggedColor[2];
   GLvector1ui *FoggedIndex[2];
   

   /* Temporary values used in texgen.
    */
   GLfloat (*tmp_f)[3];
   GLfloat *tmp_m;

   /* Temporary values used in eval.
    */
   GLuint *EvaluatedFlags;

   /* Not used for cva: 
    */
   GLubyte *NormCullStart;
   GLubyte *CullMask;	        /* Results of vertex culling */
   GLubyte *NormCullMask;       /* Compressed onto shared normals */


   GLubyte ClipOrMask;		/* bitwise-OR of all ClipMask[] values */
   GLubyte ClipAndMask;		/* bitwise-AND of all ClipMask[] values */
   GLubyte CullFlag[2];
   GLubyte CullMode;		/* see flags below */

   GLuint CopyCount;		/* max 3 vertices to copy after transform */
   GLuint Copy[3];
   GLfloat CopyProj[3][4];	/* temporary store for projected clip coords */
};


extern struct vertex_buffer *gl_alloc_vb( GLcontext *ctx );
extern struct immediate *gl_alloc_immediate( GLcontext *ctx );
extern void gl_free_immediate( struct immediate *im );

extern struct vertex_buffer *gl_vb_create_for_immediate( GLcontext *ctx );
extern struct vertex_buffer *gl_vb_create_for_cva( GLcontext *ctx, 
						   GLuint size );
extern void gl_vb_free( struct vertex_buffer * );
extern struct immediate *gl_immediate_alloc( GLcontext *ctx );
extern void gl_immediate_free( struct immediate *im );


#endif

