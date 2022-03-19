/* $Id: texgen_tmp.h,v 1.5.4.1 2001/02/12 20:39:34 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.4.1
 * 
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
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


/* Need to transform the eye plane equations into object space, and
 * use ctx->EyeZDir in spheregen.
 */

#define CULL IDX&1

/* 
 */
static void TAG(build_m3)(GLfloat      f[][3],
			  GLfloat      m[],
			  const GLvector3f  *normals, 
			  const GLvector4f  *coord_vec, 
			  const GLuint       flags[],
			  const GLubyte      cullmask[] )
{
   const GLuint stride = coord_vec->stride;
   const GLfloat *coord = (const GLfloat *) coord_vec->start;
   const GLuint count = coord_vec->count;
   const GLfloat *normal = FIRST_NORMAL;
   GLuint i;

   LOCAL_VARS;

   (void) flags;
   (void) cullmask;

   for (i=0; i<count; i++, STRIDE_F(coord,stride), NEXT_NORMAL) {
      CHECK {
	 GLfloat u[3], two_nu, fx, fy, fz;
         CUR_NORMAL;
	 COPY_3V( u, coord ); 
	 NORMALIZE_3FV( u );
	 two_nu = 2.0F * DOT3(normal,u);
	 fx = f[i][0] = u[0] - normal[0] * two_nu;
	 fy = f[i][1] = u[1] - normal[1] * two_nu;
	 fz = f[i][2] = u[2] - normal[2] * two_nu;
	 m[i] = fx * fx + fy * fy + (fz + 1.0F) * (fz + 1.0F);
	 if (m[i] != 0.0F) {
	    m[i] = 0.5F / (GLfloat) GL_SQRT(m[i]);
	 }
      }
   }
}



static void TAG(build_m2)(GLfloat f[][3],
			  GLfloat m[],
			  const GLvector3f  *normals, 
			  const GLvector4f  *coord_vec, 
			  const GLuint       flags[],
			  const GLubyte      cullmask[] )
{
   const GLuint stride = coord_vec->stride;
   const GLfloat *coord = coord_vec->start;
   const GLuint count = coord_vec->count;
   const GLfloat *normal = FIRST_NORMAL;
   GLuint i;

   LOCAL_VARS;

   (void) flags;
   (void) cullmask;

   for (i=0; i<count; i++, STRIDE_F(coord,stride), NEXT_NORMAL) {
      CHECK {
	 GLfloat u[3], two_nu, fx, fy, fz;
         CUR_NORMAL;
	 COPY_2V( u, coord ); u[2] = 0;
	 NORMALIZE_3FV( u );
	 two_nu = 2.0F * DOT3(normal,u);
	 fx = f[i][0] = u[0] - normal[0] * two_nu;
	 fy = f[i][1] = u[1] - normal[1] * two_nu;
	 fz = f[i][2] = u[2] - normal[2] * two_nu;
	 m[i] = fx * fx + fy * fy + (fz + 1.0F) * (fz + 1.0F);
	 if (m[i] != 0.0F) {
	    m[i] = 0.5F / (GLfloat) GL_SQRT(m[i]);
	 }
      }
   }
}



static build_m_func TAG(build_m_tab)[5] = {
   0,
   0,
   TAG(build_m2),
   TAG(build_m3),
   TAG(build_m3)
};


/* This is unusual in that we respect the stride of the output vector
 * (f).  This allows us to pass in either a texcoord vector4f, or a
 * temporary vector3f.  
 */
static void TAG(build_f3)( GLfloat *f, 
			   GLuint fstride,
			   const GLvector3f *normals,
			   const GLvector4f *coord_vec, 
			   const GLuint       flags[],
			   const GLubyte cullmask[] )
{
   const GLuint stride = coord_vec->stride;
   const GLfloat *coord = coord_vec->start;
   const GLuint count = coord_vec->count;
   const GLfloat *normal = FIRST_NORMAL;
   GLuint i;

   LOCAL_VARS;

   (void) flags;
   (void) cullmask;

   for (i=0; i<count; i++, STRIDE_F(coord,stride), STRIDE_F(f,fstride), NEXT_NORMAL) {
      CHECK {
	 GLfloat u[3], two_nu;
         CUR_NORMAL;
	 COPY_3V( u, coord ); 
	 NORMALIZE_3FV( u );
	 two_nu = 2.0F * DOT3(normal,u);
	 f[0] = u[0] - normal[0] * two_nu;
	 f[1] = u[1] - normal[1] * two_nu;
	 f[2] = u[2] - normal[2] * two_nu;
      }
   }
}


static void TAG(build_f2)( GLfloat *f, 
			   GLuint fstride,
			   const GLvector3f *normals,
			   const GLvector4f *coord_vec, 
			   const GLuint      flags[],
			   const GLubyte     cullmask[] )
{
   const GLuint stride = coord_vec->stride;
   const GLfloat *coord = coord_vec->start;
   const GLuint count = coord_vec->count;
   const GLfloat *normal = FIRST_NORMAL;
   GLuint i;

   LOCAL_VARS;

   (void) flags;
   (void) cullmask;

   for (i=0; i<count; i++, STRIDE_F(coord,stride), STRIDE_F(f,fstride), NEXT_NORMAL) {
      CHECK {
	 GLfloat u[3], two_nu;
         CUR_NORMAL;
	 COPY_2V( u, coord ); u[2] = 0;
	 NORMALIZE_3FV( u );
	 two_nu = 2.0F * DOT3(normal,u);
	 f[0] = u[0] - normal[0] * two_nu;
	 f[1] = u[1] - normal[1] * two_nu;
	 f[2] = u[2] - normal[2] * two_nu;
      }
   }
}

/* Just treat 4-vectors as 3-vectors. 
 */
static build_f_func TAG(build_f_tab)[5] = {
   0,
   0,
   TAG(build_f2),
   TAG(build_f3),
   TAG(build_f3)		
};


/* Special case texgen functions.
 */
static void TAG(texgen_reflection_map_nv)( struct vertex_buffer *VB, 
					   GLuint textureUnit )
{
   GLvector4f *in = VB->TexCoordPtr[textureUnit];
   GLvector4f *out = VB->store.TexCoord[textureUnit];
   GLubyte *cullmask = VB->CullMask + VB->Start;

   TAG(build_f_tab)[VB->Unprojected->size]( out->start,
					    out->stride,
					    VB->NormalPtr, 
					    VB->Unprojected, 
					    VB->Flag + VB->Start,
					    cullmask ); 

   if (!in) in = out;

   if (in != out && in->size == 4) {
      gl_copy_tab[CULL][0x8](out, in, cullmask);
   }

   VB->TexCoordPtr[textureUnit] = out;
   out->size = MAX2(in->size, 3);
   out->flags |= in->flags | VEC_SIZE_3;
}



static void TAG(texgen_normal_map_nv)( struct vertex_buffer *VB, 
				       GLuint textureUnit )
{
   GLvector4f *in = VB->TexCoordPtr[textureUnit];
   GLvector4f *out = VB->store.TexCoord[textureUnit];
   GLvector3f *normals = VB->NormalPtr;   
   GLfloat (*texcoord)[4] = (GLfloat (*)[4])out->start;
   GLubyte *cullmask = VB->CullMask + VB->Start;
   GLuint *flags = VB->Flag + VB->Start;
   GLuint count = VB->Count;

   const GLfloat *normal = FIRST_NORMAL;
   GLuint i;

   LOCAL_VARS;

   (void) flags;
   (void) cullmask;

   for (i=0; i<count; i++,NEXT_NORMAL) {
      CHECK {
         CUR_NORMAL;
	 texcoord[i][0] = normal[0];
	 texcoord[i][1] = normal[1];
	 texcoord[i][2] = normal[2];
      }
   }

   if (!in) in = out;

   if (in != out && in->size == 4) {
      gl_copy_tab[CULL][0x8](out, in, cullmask);
   }

   VB->TexCoordPtr[textureUnit] = out;
   out->size = MAX2(in->size, 3);
   out->flags |= in->flags | VEC_SIZE_3;
}


static void TAG(texgen_sphere_map)( struct vertex_buffer *VB, 
				    GLuint textureUnit )
{
   GLvector4f *in = VB->TexCoordPtr[textureUnit];
   GLvector4f *out = VB->store.TexCoord[textureUnit];
   GLfloat (*texcoord)[4] = (GLfloat (*)[4]) out->start;
   GLubyte *cullmask = VB->CullMask + VB->Start;
   GLuint count = VB->Count;
   GLuint i;
   GLfloat (*f)[3], *m;

   if (!VB->tmp_f) 
      VB->tmp_f = (GLfloat (*)[3]) MALLOC(VB->Size * sizeof(GLfloat) * 3);

   if (!VB->tmp_m) 
      VB->tmp_m = (GLfloat *) MALLOC(VB->Size * sizeof(GLfloat));
   
   f = VB->tmp_f;
   m = VB->tmp_m;

   (TAG(build_m_tab)[VB->Unprojected->size])( f, m, 
					      VB->NormalPtr, 
					      VB->Unprojected, 
					      VB->Flag + VB->Start,
					      cullmask ); 

   for (i=0;i<count;i++) {
      CHECK {
	 texcoord[i][0] = f[i][0] * m[i] + 0.5F;
	 texcoord[i][1] = f[i][1] * m[i] + 0.5F;
      }
   }


   if (!in) in = out;

   if (in != out) {
      struct gl_texture_unit *texUnit = &VB->ctx->Texture.Unit[textureUnit];
      GLuint copy = (all_bits[in->size] & ~texUnit->TexGenEnabled);
      if (copy)
	 gl_copy_tab[CULL][copy](out, in, cullmask);
   }

   VB->TexCoordPtr[textureUnit] = out;
   out->size = MAX2(in->size,2);
   out->flags |= in->flags | VEC_SIZE_2;
}



static void TAG(texgen)( struct vertex_buffer *VB, GLuint textureUnit )
{
   GLcontext *ctx = VB->ctx;
   struct gl_texture_unit *texUnit = &ctx->Texture.Unit[textureUnit];
   const GLvector4f *obj = VB->ObjPtr;
   const GLvector4f *eye = VB->EyePtr;
   const GLvector3f *normals = VB->NormalPtr;
   const GLubyte *cullmask = VB->CullMask + VB->Start;
   const GLuint *flags = VB->Flag + VB->Start;

   GLvector4f *in = VB->TexCoordPtr[textureUnit];
   GLvector4f *out = VB->store.TexCoord[textureUnit];
   GLfloat (*texcoord)[4] = (GLfloat (*)[4])out->start;
   GLfloat *indata;
   GLuint instride;
   GLuint count = VB->Count;
   GLfloat (*f)[3], *m;

   LOCAL_VARS;

   if (!VB->tmp_f) 
      VB->tmp_f = (GLfloat (*)[3]) MALLOC(VB->Size * sizeof(GLfloat) * 3);

   if (!VB->tmp_m) 
      VB->tmp_m = (GLfloat *) MALLOC(VB->Size * sizeof(GLfloat));
   
   f = VB->tmp_f;
   m = VB->tmp_m;

   if (!in) in = out;
   instride = in->stride;



   if (texUnit->GenFlags & TEXGEN_NEED_M) {
      TAG(build_m_tab)[in->size]( f, m, normals, eye, flags, cullmask ); 
   } else if (texUnit->GenFlags & TEXGEN_NEED_F) {
      TAG(build_f_tab)[in->size]( (GLfloat *)f, 3, normals, eye, 
				  flags, cullmask ); 
   }

   if (in != out) {
      GLuint copy = (all_bits[in->size] & ~texUnit->TexGenEnabled);
      if (copy)
	 gl_copy_tab[CULL][copy](out, in, cullmask);
   }

   if (texUnit->Holes)
   {
      GLubyte holes = (GLubyte) (texUnit->Holes & ~all_bits[in->size]);
      if (holes) {
	 if (holes & VEC_DIRTY_2) gl_vector4f_clean_elem(out, count, 2);
	 if (holes & VEC_DIRTY_1) gl_vector4f_clean_elem(out, count, 1);
	 if (holes & VEC_DIRTY_0) gl_vector4f_clean_elem(out, count, 0);
      }
   }

   VB->TexCoordPtr[textureUnit] = out;
   out->size = MAX2(in->size, texUnit->TexgenSize);
   out->flags |= in->flags | texUnit->TexGenEnabled;

   if (texUnit->TexGenEnabled & S_BIT) {
      GLuint i;
      switch (texUnit->GenModeS) {
      case GL_OBJECT_LINEAR:
	 (gl_dotprod_tab[CULL][obj->size])(out, 0, obj, 
					   texUnit->ObjectPlaneS, cullmask);
	 break;
      case GL_EYE_LINEAR:
	 (gl_dotprod_tab[CULL][eye->size])(out, 0, eye,
					   texUnit->EyePlaneS, cullmask);
	 break;
      case GL_SPHERE_MAP: 
	 for (indata=in->start,i=0 ; i<count ; i++, STRIDE_F(indata,instride) )
	    CHECK texcoord[i][0] = indata[0] * m[i] + 0.5F;
	 break;
      case GL_REFLECTION_MAP_NV: 
	 for (i=0;i<count;i++) 
	    CHECK texcoord[i][0] = f[i][0];
	 break;
      case GL_NORMAL_MAP_NV: {

	 const GLfloat *normal = FIRST_NORMAL;
	 for (i=0;i<count;i++, NEXT_NORMAL) {
	    CHECK {
               CUR_NORMAL;
               texcoord[i][0] = normal[0];
            }
	 }
	 break;
      }
      default:
	 gl_problem(ctx, "Bad S texgen");
      }
   } 

   if (texUnit->TexGenEnabled & T_BIT) {
      GLuint i;
      switch (texUnit->GenModeT) {
      case GL_OBJECT_LINEAR:
	 (gl_dotprod_tab[CULL][obj->size])(out, 1, obj, 
					   texUnit->ObjectPlaneT, cullmask);
	 break;
      case GL_EYE_LINEAR:
	 (gl_dotprod_tab[CULL][eye->size])(out, 1, eye, 
				       texUnit->EyePlaneT, cullmask);
	 break; 
      case GL_SPHERE_MAP: 
	 for (indata=in->start,i=0; i<count ;i++,STRIDE_F(indata,instride)) 
	    CHECK texcoord[i][1] = indata[1] * m[i] + 0.5F;
	 break;      
      case GL_REFLECTION_MAP_NV: 
	 for (i=0;i<count;i++) 
	    CHECK texcoord[i][0] = f[i][0];
	 break;
      case GL_NORMAL_MAP_NV: {
	 const GLfloat *normal = FIRST_NORMAL;
	 for (i=0;i<count;i++, NEXT_NORMAL) {
	    CHECK {
               CUR_NORMAL;
               texcoord[i][1] = normal[1];
            }
	 }
	 break;
      }
      default:
	 gl_problem(ctx, "Bad T texgen");
      }
   }

   if (texUnit->TexGenEnabled & R_BIT) {
      GLuint i;
      switch (texUnit->GenModeR) {
      case GL_OBJECT_LINEAR:
	 (gl_dotprod_tab[CULL][obj->size])(out, 2, obj, 
					  texUnit->ObjectPlaneR, cullmask);
	 break;
      case GL_EYE_LINEAR:
	 (gl_dotprod_tab[CULL][eye->size])(out, 2, eye,
					  texUnit->EyePlaneR, cullmask);
	 break;
      case GL_REFLECTION_MAP_NV: 
	 for (i=0;i<count;i++) 
	    CHECK texcoord[i][2] = f[i][2];
	 break;
      case GL_NORMAL_MAP_NV: {
	 const GLfloat *normal = FIRST_NORMAL;
	 for (i=0;i<count;i++,NEXT_NORMAL) {
	    CHECK {
               CUR_NORMAL;
               texcoord[i][2] = normal[2];
            }
	 }
	 break;
      }
      default:
	 gl_problem(ctx, "Bad R texgen");
      }
   }

   if (texUnit->TexGenEnabled & Q_BIT) {
      switch (texUnit->GenModeQ) {
      case GL_OBJECT_LINEAR:
	 (gl_dotprod_tab[CULL][obj->size])(out, 3, obj, 
					   texUnit->ObjectPlaneQ, cullmask);
	 break;
      case GL_EYE_LINEAR:
	 (gl_dotprod_tab[CULL][eye->size])(out, 3, eye,
					   texUnit->EyePlaneQ, cullmask);
	 break;
      default:
	 gl_problem(ctx, "Bad Q texgen");
      }
   }
}

static void TAG(init_texgen)( void )
{
   texgen_generic_tab[IDX] = TAG(texgen);
   texgen_reflection_map_nv_tab[IDX] = TAG(texgen_reflection_map_nv);
   texgen_normal_map_nv_tab[IDX] = TAG(texgen_normal_map_nv);
   texgen_sphere_map_tab[IDX] = TAG(texgen_sphere_map);
}

#undef TAG
#undef IDX
#undef CULL
#undef FIRST_NORMAL  
#undef NEXT_NORMAL
#undef CUR_NORMAL
#undef LOCAL_VARS
#undef CHECK


