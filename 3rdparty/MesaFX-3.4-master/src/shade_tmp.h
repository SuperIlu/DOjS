/* $Id: shade_tmp.h,v 1.11.4.3 2001/03/16 07:41:31 gareth Exp $ */

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
 * New (3.1) transformation code written by Keith Whitwell.
 */


static void TAG(shade_rgba_spec)( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLfloat (*base)[3] = ctx->Light.BaseColor;
   GLubyte *sumA = ctx->Light.BaseAlpha;

   GLuint j;

   GLuint  vstride = VB->Unprojected->stride;
   const GLfloat *vertex = VB->Unprojected->start;
   GLuint  nstride = VB->NormalPtr->stride;
   const GLfloat *normal = VB->NormalPtr->start;
   CONST GLfloat (*first_normal)[3] = (CONST GLfloat (*)[3]) VB->NormalPtr->start;

   /* Has stride 4 to help the drivers - and us...
    */
   GLubyte (*CMcolor)[4] = 0;

   GLubyte (*Fcolor)[4] = (GLubyte (*)[4])VB->LitColor[0]->start;
   GLubyte (*Bcolor)[4] = (GLubyte (*)[4])VB->LitColor[1]->start;
   GLubyte (*Fspec)[4] = VB->Spec[0] + VB->Start;
   GLubyte (*Bspec)[4] = VB->Spec[1] + VB->Start;
   GLubyte *mask = VB->CullMask + VB->Start;
   GLubyte *cullmask = mask;
   GLuint *flags = VB->Flag + VB->Start;

   struct gl_material (*new_material)[2] = VB->Material + VB->Start;
   GLuint *new_material_mask = VB->MaterialMask + VB->Start;
   GLuint nr = VB->Count - VB->Start;

   GLuint cm_flags = 0;

   (void) cullmask;
   (void) nstride;
   (void) first_normal;
   (void) flags;

   if (ctx->Light.ColorMaterialEnabled) {
      cm_flags = VERT_RGBA;

      if (VB->ColorPtr->flags & VEC_BAD_STRIDE)
	 gl_clean_color(VB);

      CMcolor =  (GLubyte (*)[4])VB->ColorPtr->start;
   }

   VB->Color[0] = VB->LitColor[0];
   VB->Color[1] = VB->LitColor[1];
   VB->ColorPtr = VB->LitColor[0];
   VB->Specular = VB->Spec[0];

   for ( j=0 ; j<nr ; j++,STRIDE_F(vertex,vstride),NEXT_VERTEX_NORMAL) {
      GLfloat sum[2][3], spec[2][3];
      struct gl_light *light;

      if ( flags[j] & cm_flags )
	 gl_update_color_material( ctx, CMcolor[j] );

      if ( flags[j] & VERT_MATERIAL )
	 gl_update_material( ctx, new_material[j], new_material_mask[j] );

      if ( CULL(*mask) )
	 continue;

      if (LIGHT_FRONT(*mask)) {
	 COPY_3V(sum[0], base[0]);
	 ZERO_3V(spec[0]);
      }

      if (LIGHT_REAR(*mask)) {
	 COPY_3V(sum[1], base[1]);
	 ZERO_3V(spec[1]);
      }

      /* Add contribution from each enabled light source */
      foreach (light, &ctx->Light.EnabledList) {
	 GLfloat n_dot_h;
	 GLfloat correction;
	 GLint side;
	 GLfloat contrib[3];
	 GLfloat attenuation;
	 GLfloat VP[3];  /* unit vector from vertex to light */
	 GLfloat n_dot_VP;       /* n dot VP */
	 GLfloat *h;
	 GLboolean normalized;

	 /* compute VP and attenuation */
	 if (!(light->Flags & LIGHT_POSITIONAL)) {
	    /* directional light */
	    COPY_3V(VP, light->VP_inf_norm);
	    attenuation = light->VP_inf_spot_attenuation;
	 }
	 else {
	    GLfloat d;     /* distance from vertex to light */

	    SUB_3V(VP, light->Position, vertex);

	    d = (GLfloat) LEN_3FV( VP );

	    if (d > 1e-6) {
	       GLfloat invd = 1.0F / d;
	       SELF_SCALE_SCALAR_3V(VP, invd);
	    }

	    attenuation = 1.0F / (light->ConstantAttenuation + d *
				  (light->LinearAttenuation + d *
				   light->QuadraticAttenuation));

	    /* spotlight attenuation */
	    if (light->Flags & LIGHT_SPOT) {
	       GLfloat PV_dot_dir = - DOT3(VP, light->NormDirection);

	       if (PV_dot_dir<light->CosCutoff) {
		  continue; /* this light makes no contribution */
	       }
	       else {
		  double x = PV_dot_dir * (EXP_TABLE_SIZE-1);
		  int k = (int) x;
		  GLfloat spot = (GLfloat) (light->SpotExpTable[k][0]
				  + (x-k)*light->SpotExpTable[k][1]);
		  attenuation *= spot;
	       }
	    }
	 }


	 if (attenuation < 1e-3)
	    continue;		/* this light makes no contribution */

	 /* Compute dot product or normal and vector from V to light pos */
	 n_dot_VP = DOT3( normal, VP );

	 /* Which side gets the diffuse & specular terms? */
	 if (n_dot_VP < 0.0F) {
	    if (LIGHT_FRONT(*mask)) {
	       ACC_SCALE_SCALAR_3V(sum[0], attenuation, light->MatAmbient[0]);
	    }
	    if (!LIGHT_REAR(*mask)) {
	       continue;
	    }
	    side = 1;
	    correction = -1;
	    n_dot_VP = -n_dot_VP;
	 }
         else {
	    if (LIGHT_REAR(*mask)) {
	       ACC_SCALE_SCALAR_3V( sum[1], attenuation, light->MatAmbient[1]);
	    }
	    if (!LIGHT_FRONT(*mask)) {
	       continue;
	    }
	    side = 0;
	    correction = 1;
	 }

	 /* diffuse term */
	 COPY_3V(contrib, light->MatAmbient[side]);
	 ACC_SCALE_SCALAR_3V(contrib, n_dot_VP, light->MatDiffuse[side]);
	 ACC_SCALE_SCALAR_3V(sum[side], attenuation, contrib );

	 if (!light->IsMatSpecular[side])
	    continue;

	 /* specular term - cannibalize VP... */
	 if (ctx->Light.Model.LocalViewer) {
	    GLfloat v[3];
	    COPY_3V(v, vertex);
	    NORMALIZE_3FV(v);
	    SUB_3V(VP, VP, v);                /* h = VP + VPe */
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

	 n_dot_h = correction * DOT3(normal, h);

	 if (n_dot_h > 0.0F) {
	    GLfloat spec_coef;
	    struct gl_shine_tab *tab = ctx->ShineTable[side];

	    if (!normalized) {
	       n_dot_h *= n_dot_h;
	       n_dot_h /= LEN_SQUARED_3FV( h );
	       tab = ctx->ShineTable[side+2];
	    }

	    GET_SHINE_TAB_ENTRY( tab, n_dot_h, spec_coef );

	    if (spec_coef > 1.0e-10) {
	       spec_coef *= attenuation;
	       ACC_SCALE_SCALAR_3V( spec[side], spec_coef,
				    light->MatSpecular[side]);
	    }
	 }
      } /*loop over lights*/

      if (LIGHT_FRONT(*mask)) {
	 FLOAT_RGB_TO_UBYTE_RGB( Fcolor[j], sum[0] );
	 FLOAT_RGB_TO_UBYTE_RGB( Fspec[j], spec[0] );
	 Fcolor[j][3] = sumA[0];
      }

      if (LIGHT_REAR(*mask)) {
	 FLOAT_RGB_TO_UBYTE_RGB( Bcolor[j], sum[1] );
	 FLOAT_RGB_TO_UBYTE_RGB( Bspec[j], spec[1] );
	 Bcolor[j][3] = sumA[1];
      }
   }

   if ( flags[j] & cm_flags )
      gl_update_color_material( ctx, CMcolor[j] );

   if ( flags[j] & VERT_MATERIAL )
      gl_update_material( ctx, new_material[j], new_material_mask[j] );
}


static void TAG(shade_rgba)( struct vertex_buffer *VB )
{
   GLuint j;
   GLcontext *ctx = VB->ctx;

   GLfloat (*base)[3] = ctx->Light.BaseColor;
   GLubyte *sumA = ctx->Light.BaseAlpha;

   GLuint  vstride = VB->Unprojected->stride;
   const GLfloat *vertex = (GLfloat *)VB->Unprojected->start;
   GLuint  nstride = VB->NormalPtr->stride;
   const GLfloat *normal = VB->NormalPtr->start;
   CONST GLfloat (*first_normal)[3] = (CONST GLfloat (*)[3])VB->NormalPtr->start;

   GLubyte (*CMcolor)[4] = 0;
   GLubyte (*Fcolor)[4] = (GLubyte (*)[4])VB->LitColor[0]->start;
   GLubyte (*Bcolor)[4] = (GLubyte (*)[4])VB->LitColor[1]->start;
   GLubyte *mask = VB->CullMask + VB->Start;
   GLubyte *cullmask = mask;
   GLuint *flags = VB->Flag + VB->Start;
   GLuint cm_flags = 0;

   struct gl_material (*new_material)[2] = VB->Material + VB->Start;
   GLuint *new_material_mask = VB->MaterialMask + VB->Start;
   GLuint nr = VB->Count - VB->Start;

   (void) cullmask;
   (void) nstride;
   (void) first_normal;
   (void) flags;

   if (ctx->Light.ColorMaterialEnabled) {
      cm_flags = VERT_RGBA;

      if (VB->ColorPtr->flags & VEC_BAD_STRIDE)
	 gl_clean_color(VB);

      CMcolor =  (GLubyte (*)[4])VB->ColorPtr->start;
   }

   VB->ColorPtr = VB->LitColor[0];
   VB->Color[0] = VB->LitColor[0];
   VB->Color[1] = VB->LitColor[1];
   VB->Specular = VB->Spec[0];

   for ( j=0 ; j<nr ; j++,STRIDE_F(vertex,vstride),NEXT_VERTEX_NORMAL) {
      GLfloat sum[2][3];
      struct gl_light *light;

      if ( flags[j] & cm_flags )
	 gl_update_color_material( ctx, CMcolor[j] );

      if ( flags[j] & VERT_MATERIAL )
	 gl_update_material( ctx, new_material[j], new_material_mask[j] );

      if ( CULL(*mask) )
	 continue;

      COPY_3V(sum[0], base[0]);

      if ( NR_SIDES == 2 )
	 COPY_3V(sum[1], base[1]);

      /* Add contribution from each enabled light source */
      foreach (light, &ctx->Light.EnabledList) {

	 GLfloat n_dot_h;
	 GLfloat correction;
	 GLint side;
	 GLfloat contrib[3];
	 GLfloat attenuation = 1.0;
	 GLfloat VP[3];          /* unit vector from vertex to light */
	 GLfloat n_dot_VP;       /* n dot VP */
	 GLfloat *h;
	 GLboolean normalized;

	 /* compute VP and attenuation */
	 if (!(light->Flags & LIGHT_POSITIONAL)) {
	    /* directional light */
	    COPY_3V(VP, light->VP_inf_norm);
	    attenuation = light->VP_inf_spot_attenuation;
	 }
	 else {
	    GLfloat d;     /* distance from vertex to light */


	    SUB_3V(VP, light->Position, vertex);

	    d = LEN_3FV( VP );

	    if ( d > 1e-6) {
	       GLfloat invd = 1.0F / d;
	       SELF_SCALE_SCALAR_3V(VP, invd);
	    }

            attenuation = 1.0F / (light->ConstantAttenuation + d *
                                  (light->LinearAttenuation + d *
                                   light->QuadraticAttenuation));

	    /* spotlight attenuation */
	    if (light->Flags & LIGHT_SPOT) {
	       GLfloat PV_dot_dir = - DOT3(VP, light->NormDirection);

	       if (PV_dot_dir<light->CosCutoff) {
		  continue; /* this light makes no contribution */
	       }
	       else {
		  double x = PV_dot_dir * (EXP_TABLE_SIZE-1);
		  int k = (int) x;
		  GLfloat spot = (light->SpotExpTable[k][0]
				  + (x-k)*light->SpotExpTable[k][1]);
		  attenuation *= spot;
	       }
	    }
	 }


	 if (attenuation < 1e-3)
	    continue;		/* this light makes no contribution */


	 /* Compute dot product or normal and vector from V to light pos */
	 n_dot_VP = DOT3( normal, VP );

	 /* which side are we lighting? */
	 if (n_dot_VP < 0.0F) {
	    if (LIGHT_FRONT(*mask)) {
	       ACC_SCALE_SCALAR_3V(sum[0], attenuation, light->MatAmbient[0]);
	    }
	    if (!LIGHT_REAR(*mask))
	       continue;

	    side = 1;
	    correction = -1;
	    n_dot_VP = -n_dot_VP;
	 }
         else {
	    if (LIGHT_REAR(*mask)) {
	       ACC_SCALE_SCALAR_3V( sum[1], attenuation, light->MatAmbient[1]);
	    }
	    if (!LIGHT_FRONT(*mask))
	       continue;
	    side = 0;
	    correction = 1;
	 }

	 COPY_3V(contrib, light->MatAmbient[side]);

	 /* diffuse term */
	 ACC_SCALE_SCALAR_3V(contrib, n_dot_VP, light->MatDiffuse[side]);

	 /* specular term - cannibalize VP... */
	 if (light->IsMatSpecular[side]) {
	    if (ctx->Light.Model.LocalViewer) {
	       GLfloat v[3];
	       COPY_3V(v, vertex);
	       NORMALIZE_3FV(v);
	       SUB_3V(VP, VP, v);                /* h = VP + VPe */
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

	    n_dot_h = correction * DOT3(normal, h);

	    if (n_dot_h > 0.0F)
	    {
	       GLfloat spec_coef;
	       struct gl_shine_tab *tab = ctx->ShineTable[side];

	       if (!normalized) {
		  n_dot_h *= n_dot_h;
		  n_dot_h /= LEN_SQUARED_3FV( h );
		  tab = ctx->ShineTable[side+2];
	       }

	       GET_SHINE_TAB_ENTRY( tab, n_dot_h, spec_coef );

	       ACC_SCALE_SCALAR_3V( contrib, spec_coef,
				    light->MatSpecular[side]);
	    }
	 }

	 ACC_SCALE_SCALAR_3V( sum[side], attenuation, contrib );
      }

      if (LIGHT_FRONT(*mask)) {
	 FLOAT_RGB_TO_UBYTE_RGB( Fcolor[j], sum[0] );
	 Fcolor[j][3] = sumA[0];
      }

      if (LIGHT_REAR(*mask)) {
	 FLOAT_RGB_TO_UBYTE_RGB( Bcolor[j], sum[1] );
	 Bcolor[j][3] = sumA[1];
      }
   }

   if ( flags[j] & cm_flags )
      gl_update_color_material( ctx, CMcolor[j] );

   if ( flags[j] & VERT_MATERIAL )
      gl_update_material( ctx, new_material[j], new_material_mask[j] );
}




/* As below, but with just a single light and no colormaterial.
 */
static void TAG(shade_fast_rgba_single)( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLuint  nstride = VB->NormalPtr->stride;
   const GLfloat *normal = VB->NormalPtr->start;
   CONST GLfloat (*first_normal)[3] =
      (CONST GLfloat (*)[3])VB->NormalPtr->start;
   GLubyte (*Fcolor)[4] = (GLubyte (*)[4])VB->LitColor[0]->start;
   GLubyte (*Bcolor)[4] = (GLubyte (*)[4])VB->LitColor[1]->start;
   GLubyte *mask = VB->NormCullStart;
   struct gl_light *light = ctx->Light.EnabledList.next;
   GLubyte *cullmask = mask;
   GLuint *flags = VB->Flag + VB->Start;
   GLubyte baseubyte[2][4];
   GLuint j = 0;
   struct gl_material (*new_material)[2] = VB->Material + VB->Start;
   GLuint *new_material_mask = VB->MaterialMask + VB->Start;
   GLfloat base[2][3];

   (void) cullmask;
   (void) first_normal;
   (void) nstride;

   if ( flags[j] & VERT_MATERIAL )
      gl_update_material( ctx, new_material[j], new_material_mask[j] );

   /* No attenuation, so incoporate MatAmbient into base color.
    */
   {
      COPY_3V(base[0], light->MatAmbient[0]);
      ACC_3V(base[0], ctx->Light.BaseColor[0] );
      FLOAT_RGB_TO_UBYTE_RGB( baseubyte[0], base[0] );
      baseubyte[0][3] = ctx->Light.BaseAlpha[0];

      if (NR_SIDES == 2) {
	 COPY_3V(base[1], light->MatAmbient[1]);
	 ACC_3V(base[1], ctx->Light.BaseColor[1]);
	 FLOAT_RGB_TO_UBYTE_RGB( baseubyte[1], base[1]);
	 baseubyte[1][3] = ctx->Light.BaseAlpha[1];
      }
   }

   VB->ColorPtr = VB->LitColor[0];
   VB->Color[0] = VB->LitColor[0];
   VB->Color[1] = VB->LitColor[1];
   VB->Specular = VB->Spec[0];

   do {
      do {
	 if ( !CULL(*mask) ) {
	    GLfloat n_dot_VP = DOT3(normal, light->VP_inf_norm);

	    COPY_4UBV(Fcolor[j], baseubyte[0]);
	    if (NR_SIDES == 2) COPY_4UBV(Bcolor[j], baseubyte[1]);

	    if (n_dot_VP < 0.0F) {
	       if (LIGHT_REAR(*mask)) {
		  GLfloat n_dot_h = -DOT3(normal, light->h_inf_norm);
		  if (n_dot_h > 0.0F) {
		     GLfloat spec, sum[3];
		     GET_SHINE_TAB_ENTRY( ctx->ShineTable[1], n_dot_h, spec );
		     COPY_3V(sum, base[1]);
		     ACC_SCALE_SCALAR_3V(sum, -n_dot_VP, light->MatDiffuse[1]);
		     ACC_SCALE_SCALAR_3V(sum, spec, light->MatSpecular[1]);
		     FLOAT_RGB_TO_UBYTE_RGB(Bcolor[j], sum );
		  }
	       }
	    } else {
	       if (LIGHT_FRONT(*mask)) {
		  GLfloat n_dot_h = DOT3(normal, light->h_inf_norm);
		  if (n_dot_h > 0.0F) {
		     GLfloat spec, sum[3];
		     GET_SHINE_TAB_ENTRY( ctx->ShineTable[0], n_dot_h, spec );
		     COPY_3V(sum, base[0]);
		     ACC_SCALE_SCALAR_3V(sum, n_dot_VP, light->MatDiffuse[0]);
		     ACC_SCALE_SCALAR_3V(sum, spec, light->MatSpecular[0]);
		     FLOAT_RGB_TO_UBYTE_RGB(Fcolor[j], sum );
		  }
	       }
	    }
	 }
	 j++;
	 NEXT_NORMAL;
      } while ((flags[j] & (VERT_MATERIAL|VERT_END_VB|VERT_NORM)) == VERT_NORM);


      if (COMPACTED) {
	 GLuint last = j-1;
	 for ( ; !(flags[j] & (VERT_MATERIAL|VERT_END_VB|VERT_NORM)) ; j++ ) {
	    COPY_4UBV(Fcolor[j], Fcolor[last]);
	    if (NR_SIDES==2)
	       COPY_4UBV(Bcolor[j], Bcolor[last]);
	 }
	 NEXT_NORMAL;
      }

      /* Have to recompute our base colors on material change.
       */
      if ( flags[j] & VERT_MATERIAL ) {
	 gl_update_material( ctx, new_material[j], new_material_mask[j] );

	 COPY_3V(base[0], light->MatAmbient[0]);
	 ACC_3V(base[0], ctx->Light.BaseColor[0] );
	 FLOAT_RGB_TO_UBYTE_RGB( baseubyte[0], base[0] );

	 if (NR_SIDES == 2) {
	    COPY_3V(base[1], light->MatAmbient[1]);
	    ACC_3V(base[1], ctx->Light.BaseColor[1]);
	    FLOAT_RGB_TO_UBYTE_RGB( baseubyte[1], base[1]);
	 }
      }

   } while (!(flags[j] & VERT_END_VB));
}


/* Vertex size doesn't matter - yay!
 */
static void TAG(shade_fast_rgba)( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLfloat (*base)[3] = ctx->Light.BaseColor;
   GLubyte *sumA = ctx->Light.BaseAlpha;
   GLuint  nstride = VB->NormalPtr->stride;
   const GLfloat *normal = VB->NormalPtr->start;
   CONST GLfloat (*first_normal)[3] = (CONST GLfloat (*)[3])VB->NormalPtr->start;
   GLubyte (*CMcolor)[4] = 0;
   GLubyte (*Fcolor)[4] = (GLubyte (*)[4])VB->LitColor[0]->start;
   GLubyte (*Bcolor)[4] = (GLubyte (*)[4])VB->LitColor[1]->start;
   GLubyte *mask = VB->NormCullStart;
   GLubyte *cullmask = mask;
   GLuint *flags = VB->Flag + VB->Start;
   GLuint cm_flags = 0;
   GLuint interesting;
   GLuint j = 0;
   struct gl_material (*new_material)[2] = VB->Material + VB->Start;
   GLuint *new_material_mask = VB->MaterialMask + VB->Start;
   struct gl_light *light;

   (void) cullmask;
   (void) first_normal;
   (void) flags;
   (void) nstride;

   if (ctx->Light.ColorMaterialEnabled)
   {
      cm_flags = VERT_RGBA;

      if (VB->ColorPtr->flags & VEC_BAD_STRIDE)
	 gl_clean_color(VB);

      CMcolor =  (GLubyte (*)[4])VB->ColorPtr->start;
      if ( *flags & VERT_RGBA )
	 gl_update_color_material( ctx, *CMcolor );

   }

   if ( flags[j] & VERT_MATERIAL )
      gl_update_material( ctx, new_material[j], new_material_mask[j] );

   interesting = cm_flags | VERT_MATERIAL | VERT_END_VB | VERT_NORM;
   VB->ColorPtr = VB->LitColor[0];
   VB->Color[0] = VB->LitColor[0];
   VB->Color[1] = VB->LitColor[1];
   VB->Specular = VB->Spec[0];

   do {
      do {
	 if ( !CULL(*mask) ) {
	    GLfloat sum[2][3];
	    GLfloat spec;

	    COPY_3V(sum[0], base[0]);
	    if (NR_SIDES == 2)  COPY_3V(sum[1], base[1]);

	    if (MESA_VERBOSE&VERBOSE_LIGHTING)
	       fprintf(stderr, "light normal %d/%d, %f %f %f\n",
		       j, VB->Start, normal[0], normal[1], normal[2]);


	    foreach (light, &ctx->Light.EnabledList) {

	       GLfloat n_dot_h;
	       GLint side = 0;
	       GLfloat n_dot_VP = DOT3(normal, light->VP_inf_norm);

	       ACC_3V(sum[0], light->MatAmbient[0]);
	       if (NR_SIDES == 2) ACC_3V(sum[1], light->MatAmbient[1]);

	       if (n_dot_VP < 0.0F) {
		  if ( !LIGHT_REAR(*mask) ) continue;
		  ACC_SCALE_SCALAR_3V(sum[1], -n_dot_VP, light->MatDiffuse[1]);
		  if (!light->IsMatSpecular[1]) continue;
		  n_dot_h = -DOT3(normal, light->h_inf_norm);
		  side = 1;
	       }
               else {
		  if ( !LIGHT_FRONT(*mask) ) continue;
		  ACC_SCALE_SCALAR_3V(sum[0], n_dot_VP, light->MatDiffuse[0]);
		  if (!light->IsMatSpecular[0]) continue;
		  n_dot_h = DOT3(normal, light->h_inf_norm);
	       }

	       if (n_dot_h > 0.0F) {
		  struct gl_shine_tab *tab = ctx->ShineTable[side];
		  GET_SHINE_TAB_ENTRY( tab, n_dot_h, spec );
		  ACC_SCALE_SCALAR_3V( sum[side], spec,
				       light->MatSpecular[side]);
	       }
	    }
	    if (LIGHT_FRONT(*mask)) {
	       FLOAT_RGB_TO_UBYTE_RGB( Fcolor[j], sum[0] );
	       Fcolor[j][3] = sumA[0];
	    }

	    if (LIGHT_REAR(*mask)) {
	       FLOAT_RGB_TO_UBYTE_RGB( Bcolor[j], sum[1] );
	       Bcolor[j][3] = sumA[1];
	    }
	 }
	 j++;
	 NEXT_NORMAL;
      } while ((flags[j] & interesting) == VERT_NORM);


      if (COMPACTED) {
	 GLuint last = j-1;

	 for ( ; !(flags[j] & interesting) ; j++ )
	 {
	    COPY_4UBV(Fcolor[j], Fcolor[last]);
	    if (NR_SIDES==2)
	       COPY_4UBV(Bcolor[j], Bcolor[last]);
	 }

	 NEXT_NORMAL;
      }

      if ( flags[j] & cm_flags )
	 gl_update_color_material( ctx, CMcolor[j] );

      if ( flags[j] & VERT_MATERIAL )
	 gl_update_material( ctx, new_material[j], new_material_mask[j] );

   } while (!(flags[j] & VERT_END_VB));
}





/*
 * Use current lighting/material settings to compute the color indexes
 * for an array of vertices.
 * Input:  n - number of vertices to shade
 *         side - 0=use front material, 1=use back material
 *         vertex - array of [n] vertex position in eye coordinates
 *         normal - array of [n] surface normal vector
 * Output:  indexResult - resulting array of [n] color indexes
 */
static void TAG(shade_ci)( struct vertex_buffer *VB )
{
   GLuint j;

   GLcontext *ctx = VB->ctx;
   GLuint  vstride = VB->Unprojected->stride;
   const GLfloat *vertex = (GLfloat *)VB->Unprojected->start;
   GLuint  nstride = VB->NormalPtr->stride;
   const GLfloat *normal = VB->NormalPtr->start;
   CONST GLfloat (*first_normal)[3] = (CONST GLfloat (*)[3])VB->NormalPtr->start;

   GLubyte (*CMcolor)[4] = 0;
   GLubyte *mask = VB->CullMask + VB->Start;
   GLubyte *cullmask = mask;
   GLuint *flags = VB->Flag + VB->Start;
   GLuint cm_flags = 0;
   GLuint  *indexResult[2];

   struct gl_material (*new_material)[2] = VB->Material + VB->Start;
   GLuint *new_material_mask = VB->MaterialMask + VB->Start;
   GLuint nr = VB->Count - VB->Start;

   (void) cullmask;
   (void) nstride;
   (void) first_normal;
   (void) flags;

   VB->IndexPtr = VB->LitIndex[0];
   VB->Index[0] = VB->LitIndex[0];
   VB->Index[1] = VB->LitIndex[1];

   indexResult[0] = VB->Index[0]->start;
   indexResult[1] = VB->Index[1]->start;

   /* loop over vertices */

   if (ctx->Light.ColorMaterialEnabled) {
      cm_flags = VERT_RGBA;

      if (VB->ColorPtr->flags & VEC_BAD_STRIDE)
	 gl_clean_color(VB);

      CMcolor =  (GLubyte (*)[4])VB->ColorPtr->start;
   }

   for ( j=0 ; j<nr ; j++,STRIDE_F(vertex,vstride),NEXT_VERTEX_NORMAL) {
      GLfloat diffuse[2], specular[2];
      GLuint side = 0;
      struct gl_light *light;

      if ( flags[j] & cm_flags )
	 gl_update_color_material( ctx, CMcolor[j] );

      if ( flags[j] & VERT_MATERIAL )
	 gl_update_material( ctx, new_material[j], new_material_mask[j] );

      if ( CULL(*mask) )
	 continue;

      diffuse[0] = specular[0] = 0.0F;

      if ( NR_SIDES == 2 ) {
	 diffuse[1] = specular[1] = 0.0F;
      }

      /* Accumulate diffuse and specular from each light source */
      foreach (light, &ctx->Light.EnabledList) {

	 GLfloat attenuation = 1.0F;
	 GLfloat VP[3];  /* unit vector from vertex to light */
	 GLfloat n_dot_VP;  /* dot product of l and n */
	 GLfloat *h, n_dot_h, correction = 1.0;
	 GLboolean normalized;

	 /* compute l and attenuation */
	 if (!(light->Flags & LIGHT_POSITIONAL)) {
	    /* directional light */
	    COPY_3V(VP, light->VP_inf_norm);
	 }
	 else {
	    GLfloat d;     /* distance from vertex to light */

	    SUB_3V(VP, light->Position, vertex);

	    d = LEN_3FV( VP );
	    if ( d > 1e-6) {
	       GLfloat invd = 1.0F / d;
	       SELF_SCALE_SCALAR_3V(VP, invd);
	    }

	    attenuation = 1.0F / (light->ConstantAttenuation + d *
				  (light->LinearAttenuation + d *
				   light->QuadraticAttenuation));

	    /* spotlight attenuation */
	    if (light->Flags & LIGHT_SPOT) {
	       GLfloat PV_dot_dir = - DOT3(VP, light->NormDirection);
	       if (PV_dot_dir<light->CosCutoff) {
		  continue; /* this light makes no contribution */
	       }
	       else {
		  double x = PV_dot_dir * (EXP_TABLE_SIZE-1);
		  int k = (int) x;
		  GLfloat spot = (light->SpotExpTable[k][0]
				  + (x-k)*light->SpotExpTable[k][1]);
		  attenuation *= spot;
	       }
	    }
	 }

	 if (attenuation < 1e-3)
	    continue;		/* this light makes no contribution */

	 n_dot_VP = DOT3( normal, VP );

	 /* which side are we lighting? */
	 if (n_dot_VP < 0.0F) {
	    if (!LIGHT_REAR(*mask))
	       continue;
	    side = 1;
	    correction = -1;
	    n_dot_VP = -n_dot_VP;
	 }
         else {
	    if (!LIGHT_FRONT(*mask))
	       continue;
	 }

	 /* accumulate diffuse term */
	 diffuse[side] += n_dot_VP * light->dli * attenuation;

	 /* specular term */
	 if (!(light->Flags & LIGHT_SPECULAR))
	    continue;

	 if (ctx->Light.Model.LocalViewer) {
	    GLfloat v[3];
	    COPY_3V(v, vertex);
	    NORMALIZE_3FV(v);
	    SUB_3V(VP, VP, v);                /* h = VP + VPe */
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

	 n_dot_h = correction * DOT3(normal, h);

	 if (n_dot_h > 0.0F)
	 {
	    GLfloat spec_coef;
	    struct gl_shine_tab *tab = ctx->ShineTable[side];

	    if (!normalized) {
	       n_dot_h *= n_dot_h;
	       n_dot_h /= LEN_SQUARED_3FV( h );
	       tab = ctx->ShineTable[side+2];
	    }

	    GET_SHINE_TAB_ENTRY( tab, n_dot_h, spec_coef);

	    specular[side] += spec_coef * light->sli * attenuation;
	 }
      } /*loop over lights*/

      /* Now compute final color index */
      for (side = 0 ; side < NR_SIDES ; side++) {
	 GLfloat index;
	 struct gl_material *mat;

	 if (!LIGHT_SIDE(*mask, side))
	    continue;

	 mat = &ctx->Light.Material[side];
	 if (specular[side] > 1.0F) {
	    index = mat->SpecularIndex;
	 }
	 else {
	    GLfloat d_a = mat->DiffuseIndex - mat->AmbientIndex;
	    GLfloat s_a = mat->SpecularIndex - mat->AmbientIndex;

	    index = mat->AmbientIndex
	       + diffuse[side] * (1.0F-specular[side]) * d_a
	       + specular[side] * s_a;
	    if (index > mat->SpecularIndex) {
	       index = mat->SpecularIndex;
	    }
	 }
	 indexResult[side][j] = (GLuint) (GLint) index;
      }
   } /*for vertex*/

   if ( flags[j] & cm_flags )
      gl_update_color_material( ctx, CMcolor[j] );

   if ( flags[j] & VERT_MATERIAL )
      gl_update_material( ctx, new_material[j], new_material_mask[j] );
}



static void TAG(init_shade_tab)( void )
{
   gl_shade_tab[IDX] = TAG(shade_rgba);
   gl_shade_fast_tab[IDX] = TAG(shade_fast_rgba);
   gl_shade_fast_single_tab[IDX] = TAG(shade_fast_rgba_single);
   gl_shade_spec_tab[IDX] = TAG(shade_rgba_spec);
   gl_shade_ci_tab[IDX] = TAG(shade_ci);
}


#undef TAG
#undef INVALID
#undef IDX
#undef LIGHT_FRONT
#undef LIGHT_REAR
#undef LIGHT_SIDE
#undef NR_SIDES
#undef CULL
