
/*
 * Mesa 3-D graphics library
 * Version:  3.3
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
#include "light.h"
#include "macros.h"
#include "mmath.h"
#include "pipeline.h"
#include "shade.h"
#include "simple_list.h"
#include "types.h"
#endif

#if defined(USE_X86_ASM)
#include "X86/common_x86_asm.h"
#endif


/* Lerp between adjacent values in the f(x) lookup table, giving a
 * continuous function, with adequeate overall accuracy.  (Though
 * still pretty good compared to a straight lookup). 
 */
#define GET_SHINE_TAB_ENTRY( table, dp, result )			\
do {									\
   struct gl_shine_tab *_tab = table;					\
   if (dp>1.0)								\
      result = pow( dp, _tab->shininess );				\
   else {								\
      float f = (dp * (SHINE_TABLE_SIZE-1));				\
      int k = (int) f;							\
      result = _tab->tab[k] + (f-k)*(_tab->tab[k+1]-_tab->tab[k]);	\
   }									\
} while (0)



/* Combinatorics:  
 *     rgba_spec/rgba/rgba_fast/rgba_fast_single/ci
 *     one_side/two_side
 *     compacted_normals/ordinary_normals
 *     cull_mask/no_cull_mask
 */


/* Table of all the shading functions.
 */
gl_shade_func gl_shade_tab[0x10];
gl_shade_func gl_shade_fast_tab[0x10];
gl_shade_func gl_shade_fast_single_tab[0x10];
gl_shade_func gl_shade_spec_tab[0x10];
gl_shade_func gl_shade_ci_tab[0x10];


/* The original case where the normal for vertex[j] is normal[j],
 * both stride-aware, and every normal is present.
 */
#define NEXT_NORMAL        STRIDE_F(normal, nstride), mask++
#define NEXT_VERTEX_NORMAL STRIDE_F(normal, nstride), mask++
#define STATE_CHANGE(a,b)  1
#define COMPACTED          0

#define TAG(x)           x##_one_sided_masked
#define INVALID(x)       0
#define IDX              CULL_MASK_ACTIVE
#define LIGHT_FRONT(x)   1
#define LIGHT_REAR(x)    0
#define LIGHT_SIDE(x,y)  1
#define CULL(x)          !((x)&VERT_FACE_FLAGS)
#define NR_SIDES         1
#include "shade_tmp.h" 

#define TAG(x)           x##_one_sided
#define INVALID(x)       0
#define IDX              0
#define LIGHT_FRONT(x)   1
#define LIGHT_REAR(x)    0
#define LIGHT_SIDE(x,y)  1
#define CULL(x)          0 
#define NR_SIDES         1
#include "shade_tmp.h" 

#define TAG(x)           x##_two_sided_masked
#define INVALID(x)       ((x)&invalid)
#define IDX              SHADE_TWOSIDE|CULL_MASK_ACTIVE
#define LIGHT_FRONT(f)   ((f)&VERT_FACE_FRONT)
#define LIGHT_REAR(f)    ((f)&VERT_FACE_REAR)
#define LIGHT_SIDE(x,y)  ((x)&(y))
#define CULL(x)          !((x)&VERT_FACE_FLAGS)
#define NR_SIDES         2
#include "shade_tmp.h"

#define TAG(x)           x##_two_sided
#define INVALID(x)       0
#define IDX              SHADE_TWOSIDE
#define LIGHT_FRONT(f)   1
#define LIGHT_REAR(f)    1
#define LIGHT_SIDE(x,y)  1
#define CULL(x)          0
#define NR_SIDES         2
#include "shade_tmp.h"

#undef NEXT_NORMAL 
#undef NEXT_VERTEX_NORMAL 
#undef STATE_CHANGE
#undef COMPACTED

/* The 'compacted normal' case, where we have a sparse list of normals
 * with flags indicating a new (valid) normal, as now built by the
 * 'glVertex' API routines.   We have a small bonus in that we know
 * in advance that the normal stride must be 3 floats.
 */
#define NEXT_NORMAL         ((flags[j]&VERT_NORM) ? normal=first_normal[j],mask=&cullmask[j] : 0)
#define NEXT_VERTEX_NORMAL  ((flags[j]&VERT_NORM) ? normal=first_normal[j],mask=&cullmask[j] : 0)
#define STATE_CHANGE(a,b)   (a & b)
#define COMPACTED            1


#define TAG(x)           x##_one_sided_masked_compacted
#define INVALID(x)       0
#define IDX              COMPACTED_NORMALS|CULL_MASK_ACTIVE
#define LIGHT_FRONT(x)   1
#define LIGHT_REAR(x)    0
#define LIGHT_SIDE(x,y)  1
#define CULL(x)          !((x)&VERT_FACE_FLAGS)
#define NR_SIDES         1
#include "shade_tmp.h" 

#define TAG(x)           x##_one_sided_compacted
#define INVALID(x)       0
#define IDX              COMPACTED_NORMALS
#define LIGHT_FRONT(x)   1
#define LIGHT_REAR(x)    0
#define LIGHT_SIDE(x,y)  1
#define CULL(x)          0
#define NR_SIDES         1
#include "shade_tmp.h" 

#define TAG(x)           x##_two_sided_masked_compacted
#define INVALID(x)       ((x)&invalid)
#define IDX              COMPACTED_NORMALS|SHADE_TWOSIDE
#define LIGHT_FRONT(f)   ((f)&VERT_FACE_FRONT)
#define LIGHT_REAR(f)    ((f)&VERT_FACE_REAR)
#define LIGHT_SIDE(x,y)  ((x)&(y))
#define CULL(x)          !((x)&VERT_FACE_FLAGS)
#define NR_SIDES         2
#include "shade_tmp.h"

#define TAG(x)           x##_two_sided_compacted
#define INVALID(x)       0
#define IDX              COMPACTED_NORMALS|CULL_MASK_ACTIVE|SHADE_TWOSIDE
#define LIGHT_FRONT(f)   1
#define LIGHT_REAR(f)    1
#define LIGHT_SIDE(x,y)  1
#define CULL(x)          0
#define NR_SIDES         2
#include "shade_tmp.h"

#undef COMPACTED
#undef NEXT_NORMAL 
#undef NEXT_VERTEX_NORMAL 
#undef STATE_CHANGE



void gl_init_shade( void )
{
   init_shade_tab_one_sided();
   init_shade_tab_one_sided_masked();
   init_shade_tab_one_sided_compacted();
   init_shade_tab_one_sided_masked_compacted();

   init_shade_tab_two_sided();
   init_shade_tab_two_sided_masked();
   init_shade_tab_two_sided_compacted();
   init_shade_tab_two_sided_masked_compacted();

#ifdef USE_X86_ASM
   gl_init_all_x86_shade_asm();
#endif
}

void gl_update_lighting_function( GLcontext *ctx )
{
   gl_shade_func *tab;

   if (ctx->Visual->RGBAflag) {
      if (ctx->Light.NeedVertices) {
	 if (ctx->Texture.ReallyEnabled && 
	     ctx->Light.Model.ColorControl==GL_SEPARATE_SPECULAR_COLOR) 
	    tab = gl_shade_spec_tab;
	 else
	    tab = gl_shade_tab;	 
      }  
      else {
	 if (ctx->Light.EnabledList.next == ctx->Light.EnabledList.prev &&
	     !ctx->Light.ColorMaterialEnabled) 
	    tab = gl_shade_fast_single_tab;
	 else
	    tab = gl_shade_fast_tab;
      }
   }
   else
      tab = gl_shade_ci_tab;

   if (ctx->TriangleCaps & DD_TRI_LIGHT_TWOSIDE) {
      tab += SHADE_TWOSIDE;
   }

   ctx->shade_func_tab = tab;
}



/* This has been split off to allow the normal shade routines to
 * get a little closer to the vertex buffer, and to use the 
 * GLvector objects directly.
 */
void gl_shade_rastpos( GLcontext *ctx,
		       GLfloat vertex[4],
		       GLfloat normal[3],
		       GLfloat Rcolor[4],
		       GLuint *index )
{
   GLfloat (*base)[3] = ctx->Light.BaseColor;
   GLubyte *sumA = ctx->Light.BaseAlpha;
   struct gl_light *light;
   GLfloat color[4];
   GLfloat diffuse = 0, specular = 0;

   COPY_3V(color, base[0]);
   color[3] = UBYTE_COLOR_TO_FLOAT_COLOR( sumA[0] );

   foreach (light, &ctx->Light.EnabledList) {
      GLfloat n_dot_h;
      GLfloat attenuation = 1.0;
      GLfloat VP[3];  
      GLfloat n_dot_VP;
      GLfloat *h;
      GLfloat contrib[3];
      GLboolean normalized;

      if (!(light->Flags & LIGHT_POSITIONAL)) {
	 COPY_3V(VP, light->VP_inf_norm);
	 attenuation = light->VP_inf_spot_attenuation;
      }
      else {
	 GLfloat d; 
	 
	 SUB_3V(VP, light->Position, vertex);
	 d = LEN_3FV( VP );
	 
	 if ( d > 1e-6) {
	    GLfloat invd = 1.0F / d;
	    SELF_SCALE_SCALAR_3V(VP, invd);
	 }
	 attenuation = 1.0F / (light->ConstantAttenuation + d * 
			       (light->LinearAttenuation + d * 
				light->QuadraticAttenuation));
	 
	 if (light->Flags & LIGHT_SPOT) 
	 {
	    GLfloat PV_dot_dir = - DOT3(VP, light->NormDirection);
	    
	    if (PV_dot_dir<light->CosCutoff) {
	       continue; 
	    }
	    else 
	    {
	       double x = PV_dot_dir * (EXP_TABLE_SIZE-1);
	       int k = (int) x;
	       GLfloat spot = (GLfloat) (light->SpotExpTable[k][0]
			       + (x-k)*light->SpotExpTable[k][1]);
	       attenuation *= spot;
	    }
	 }
      }

      if (attenuation < 1e-3) 
	 continue;

      n_dot_VP = DOT3( normal, VP );

      if (n_dot_VP < 0.0F) {
	 ACC_SCALE_SCALAR_3V(color, attenuation, light->MatAmbient[0]);
	 continue;
      } 

      COPY_3V(contrib, light->MatAmbient[0]);
      ACC_SCALE_SCALAR_3V(contrib, n_dot_VP, light->MatDiffuse[0]);
      diffuse += n_dot_VP * light->dli * attenuation;

      if (light->IsMatSpecular[0]) {
	 if (ctx->Light.Model.LocalViewer) {
	    GLfloat v[3];
	    COPY_3V(v, vertex);
	    NORMALIZE_3FV(v);
	    SUB_3V(VP, VP, v);
	    h = VP;
	    normalized = 0;
	 }
	 else if (light->Flags & LIGHT_POSITIONAL) {
	    h = VP;
	    ACC_3V(h, ctx->EyeZDir);
	    normalized = 0;
	 }
         else {
	    h = light->h_inf_norm;
	    normalized = 1;
	 }
	 
	 n_dot_h = DOT3(normal, h);

	 if (n_dot_h > 0.0F) {
	    struct gl_material *mat = &ctx->Light.Material[0];
	    GLfloat spec_coef;
	    GLfloat shininess = mat->Shininess;

	    if (!normalized) {
	       n_dot_h *= n_dot_h;
	       n_dot_h /= LEN_SQUARED_3FV( h );
	       shininess *= .5;
	    }
	    
	    GET_SHINE_TAB_ENTRY( ctx->ShineTable[0], n_dot_h, spec_coef );

	    if (spec_coef > 1.0e-10) {
	       ACC_SCALE_SCALAR_3V( contrib, spec_coef,
				    light->MatSpecular[0]);
	       specular += spec_coef * light->sli * attenuation;
	    }
	 }
      }

      ACC_SCALE_SCALAR_3V( color, attenuation, contrib );
   } 

   if (ctx->Visual->RGBAflag) {
      Rcolor[0] = CLAMP(color[0], 0.0F, 1.0F);
      Rcolor[1] = CLAMP(color[1], 0.0F, 1.0F);
      Rcolor[2] = CLAMP(color[2], 0.0F, 1.0F);
      Rcolor[3] = CLAMP(color[3], 0.0F, 1.0F);
   }
   else {
      struct gl_material *mat = &ctx->Light.Material[0];
      GLfloat d_a = mat->DiffuseIndex - mat->AmbientIndex;
      GLfloat s_a = mat->SpecularIndex - mat->AmbientIndex;
      GLfloat ind = mat->AmbientIndex
                  + diffuse * (1.0F-specular) * d_a
                  + specular * s_a;
      if (ind > mat->SpecularIndex) {
	 ind = mat->SpecularIndex;
      }
      *index = (GLuint) (GLint) ind;
   }

}
