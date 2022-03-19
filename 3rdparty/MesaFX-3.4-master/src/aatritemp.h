/* $Id: aatritemp.h,v 1.9.4.3 2001/05/07 16:01:11 brianp Exp $ */

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
 * Antialiased Triangle Rasterizer Template
 *
 * This file is #include'd to generate custom AA triangle rasterizers.
 * NOTE: this code hasn't been optimized yet.  That'll come after it
 * works correctly.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be copmuted across the triangle:
 *    DO_Z      - if defined, compute Z values
 *    DO_RGBA   - if defined, compute RGBA values
 *    DO_INDEX  - if defined, compute color index values
 *    DO_SPEC   - if defined, compute specular RGB values
 *    DO_STUV0  - if defined, compute unit 0 STRQ texcoords
 *    DO_STUV1  - if defined, compute unit 1 STRQ texcoords
 */

/*void triangle( GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv )*/
{
   const struct vertex_buffer *VB = ctx->VB;
   const GLfloat *p0 = VB->Win.data[v0];
   const GLfloat *p1 = VB->Win.data[v1];
   const GLfloat *p2 = VB->Win.data[v2];
   GLint vMin, vMid, vMax;
   GLint iyMin, iyMax;
   GLfloat yMin, yMax;
   GLboolean ltor;
   GLfloat majDx, majDy;
#ifdef DO_Z
   GLfloat zPlane[4];                                       /* Z (depth) */
   GLdepth z[MAX_WIDTH];
#endif
#ifdef DO_RGBA
   GLfloat rPlane[4], gPlane[4], bPlane[4], aPlane[4];      /* color */
   GLubyte rgba[MAX_WIDTH][4];
#endif
#ifdef DO_INDEX
   GLfloat iPlane[4];                                       /* color index */
   GLuint index[MAX_WIDTH];
#endif
#ifdef DO_SPEC
   GLfloat srPlane[4], sgPlane[4], sbPlane[4];              /* spec color */
   GLubyte spec[MAX_WIDTH][4];
#endif
#ifdef DO_STUV0
   GLfloat s0Plane[4], t0Plane[4], u0Plane[4], v0Plane[4];  /* texture 0 */
   GLfloat width0, height0;
   GLfloat s[MAX_TEXTURE_UNITS][MAX_WIDTH];
   GLfloat t[MAX_TEXTURE_UNITS][MAX_WIDTH];
   GLfloat u[MAX_TEXTURE_UNITS][MAX_WIDTH];
   GLfloat lambda[MAX_TEXTURE_UNITS][MAX_WIDTH];
#endif
#ifdef DO_STUV1
   GLfloat s1Plane[4], t1Plane[4], u1Plane[4], v1Plane[4];  /* texture 1 */
   GLfloat width1, height1;
#endif
   GLfloat bf = ctx->backface_sign;

   /* determine bottom to top order of vertices */
   {
      GLfloat y0 = VB->Win.data[v0][1];
      GLfloat y1 = VB->Win.data[v1][1];
      GLfloat y2 = VB->Win.data[v2][1];
      if (y0 <= y1) {
	 if (y1 <= y2) {
	    vMin = v0;   vMid = v1;   vMax = v2;   /* y0<=y1<=y2 */
	 }
	 else if (y2 <= y0) {
	    vMin = v2;   vMid = v0;   vMax = v1;   /* y2<=y0<=y1 */
	 }
	 else {
	    vMin = v0;   vMid = v2;   vMax = v1;  bf = -bf; /* y0<=y2<=y1 */
	 }
      }
      else {
	 if (y0 <= y2) {
	    vMin = v1;   vMid = v0;   vMax = v2;  bf = -bf; /* y1<=y0<=y2 */
	 }
	 else if (y2 <= y1) {
	    vMin = v2;   vMid = v1;   vMax = v0;  bf = -bf; /* y2<=y1<=y0 */
	 }
	 else {
	    vMin = v1;   vMid = v2;   vMax = v0;   /* y1<=y2<=y0 */
	 }
      }
   }

   majDx = VB->Win.data[vMax][0] - VB->Win.data[vMin][0];
   majDy = VB->Win.data[vMax][1] - VB->Win.data[vMin][1];

   {
      const GLfloat botDx = VB->Win.data[vMid][0] - VB->Win.data[vMin][0];
      const GLfloat botDy = VB->Win.data[vMid][1] - VB->Win.data[vMin][1];
      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area * area < .0025)
	 return;
   }

#ifndef DO_OCCLUSION_TEST
   ctx->OcclusionResult = GL_TRUE;
#endif

   /* plane setup */
#ifdef DO_Z
   compute_plane(p0, p1, p2, p0[2], p1[2], p2[2], zPlane);
#endif
#ifdef DO_RGBA
   if (ctx->Light.ShadeModel == GL_SMOOTH) {
      GLubyte (*rgba)[4] = VB->ColorPtr->data;
      compute_plane(p0, p1, p2, rgba[v0][0], rgba[v1][0], rgba[v2][0], rPlane);
      compute_plane(p0, p1, p2, rgba[v0][1], rgba[v1][1], rgba[v2][1], gPlane);
      compute_plane(p0, p1, p2, rgba[v0][2], rgba[v1][2], rgba[v2][2], bPlane);
      compute_plane(p0, p1, p2, rgba[v0][3], rgba[v1][3], rgba[v2][3], aPlane);
   }
   else {
      constant_plane(VB->ColorPtr->data[pv][RCOMP], rPlane);
      constant_plane(VB->ColorPtr->data[pv][GCOMP], gPlane);
      constant_plane(VB->ColorPtr->data[pv][BCOMP], bPlane);
      constant_plane(VB->ColorPtr->data[pv][ACOMP], aPlane);
   }
#endif
#ifdef DO_INDEX
   if (ctx->Light.ShadeModel == GL_SMOOTH) {
      compute_plane(p0, p1, p2, VB->IndexPtr->data[v0],
                    VB->IndexPtr->data[v1], VB->IndexPtr->data[v2], iPlane);
   }
   else {
      constant_plane(VB->IndexPtr->data[pv], iPlane);
   }
#endif
#ifdef DO_SPEC
   {
      GLubyte (*spec)[4] = VB->Specular;
      compute_plane(p0, p1, p2, spec[v0][0], spec[v1][0], spec[v2][0],srPlane);
      compute_plane(p0, p1, p2, spec[v0][1], spec[v1][1], spec[v2][1],sgPlane);
      compute_plane(p0, p1, p2, spec[v0][2], spec[v1][2], spec[v2][2],sbPlane);
   }
#endif
#ifdef DO_STUV0
   {
      const struct gl_texture_object *obj = ctx->Texture.Unit[0].Current;
      const struct gl_texture_image *texImage = obj->Image[obj->BaseLevel];
      const GLint tSize = 3;
      const GLfloat invW0 = VB->Win.data[v0][3];
      const GLfloat invW1 = VB->Win.data[v1][3];
      const GLfloat invW2 = VB->Win.data[v2][3];
      GLfloat (*texCoord)[4] = VB->TexCoordPtr[0]->data;
      const GLfloat s0 = texCoord[v0][0] * invW0;
      const GLfloat s1 = texCoord[v1][0] * invW1;
      const GLfloat s2 = texCoord[v2][0] * invW2;
      const GLfloat t0 = (tSize > 1) ? texCoord[v0][1] * invW0 : 0.0F;
      const GLfloat t1 = (tSize > 1) ? texCoord[v1][1] * invW1 : 0.0F;
      const GLfloat t2 = (tSize > 1) ? texCoord[v2][1] * invW2 : 0.0F;
      const GLfloat r0 = (tSize > 2) ? texCoord[v0][2] * invW0 : 0.0F;
      const GLfloat r1 = (tSize > 2) ? texCoord[v1][2] * invW1 : 0.0F;
      const GLfloat r2 = (tSize > 2) ? texCoord[v2][2] * invW2 : 0.0F;
      const GLfloat q0 = (tSize > 3) ? texCoord[v0][3] * invW0 : invW0;
      const GLfloat q1 = (tSize > 3) ? texCoord[v1][3] * invW1 : invW1;
      const GLfloat q2 = (tSize > 3) ? texCoord[v2][3] * invW2 : invW2;
      compute_plane(p0, p1, p2, s0, s1, s2, s0Plane);
      compute_plane(p0, p1, p2, t0, t1, t2, t0Plane);
      compute_plane(p0, p1, p2, r0, r1, r2, u0Plane);
      compute_plane(p0, p1, p2, q0, q1, q2, v0Plane);
      width0 = (GLfloat) texImage->Width;
      height0 = (GLfloat) texImage->Height;
   }
#endif
#ifdef DO_STUV1
   {
      const struct gl_texture_object *obj = ctx->Texture.Unit[1].Current;
      const struct gl_texture_image *texImage = obj->Image[obj->BaseLevel];
      const GLint tSize = VB->TexCoordPtr[1]->size;
      const GLfloat invW0 = VB->Win.data[v0][3];
      const GLfloat invW1 = VB->Win.data[v1][3];
      const GLfloat invW2 = VB->Win.data[v2][3];
      GLfloat (*texCoord)[4] = VB->TexCoordPtr[1]->data;
      const GLfloat s0 = texCoord[v0][0] * invW0;
      const GLfloat s1 = texCoord[v1][0] * invW1;
      const GLfloat s2 = texCoord[v2][0] * invW2;
      const GLfloat t0 = (tSize > 1) ? texCoord[v0][1] * invW0 : 0.0F;
      const GLfloat t1 = (tSize > 1) ? texCoord[v1][1] * invW1 : 0.0F;
      const GLfloat t2 = (tSize > 1) ? texCoord[v2][1] * invW2 : 0.0F;
      const GLfloat r0 = (tSize > 2) ? texCoord[v0][2] * invW0 : 0.0F;
      const GLfloat r1 = (tSize > 2) ? texCoord[v1][2] * invW1 : 0.0F;
      const GLfloat r2 = (tSize > 2) ? texCoord[v2][2] * invW2 : 0.0F;
      const GLfloat q0 = (tSize > 3) ? texCoord[v0][3] * invW0 : invW0;
      const GLfloat q1 = (tSize > 3) ? texCoord[v1][3] * invW1 : invW1;
      const GLfloat q2 = (tSize > 3) ? texCoord[v2][3] * invW2 : invW2;
      compute_plane(p0, p1, p2, s0, s1, s2, s1Plane);
      compute_plane(p0, p1, p2, t0, t1, t2, t1Plane);
      compute_plane(p0, p1, p2, r0, r1, r2, u1Plane);
      compute_plane(p0, p1, p2, q0, q1, q2, v1Plane);
      width1 = (GLfloat) texImage->Width;
      height1 = (GLfloat) texImage->Height;
   }
#endif

   yMin = VB->Win.data[vMin][1];
   yMax = VB->Win.data[vMax][1];
   iyMin = (int) yMin;
   iyMax = (int) yMax + 1;

   if (ltor) {
      /* scan left to right */
      const float *pMin = VB->Win.data[vMin];
      const float *pMid = VB->Win.data[vMid];
      const float *pMax = VB->Win.data[vMax];
      const float dxdy = majDx / majDy;
      const float xAdj = dxdy < 0.0F ? -dxdy : 0.0F;
      float x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      int iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, startX = (GLint) (x - xAdj);
         GLuint count, n;
         GLfloat coverage = 0.0F;
         /* skip over fragments with zero coverage */
         while (startX < MAX_WIDTH) {
            coverage = compute_coveragef(pMin, pMid, pMax, startX, iy);
            if (coverage > 0.0F)
               break;
            startX++;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            /* (cx,cy) = center of fragment */
            GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
#ifdef DO_Z
            z[count] = (GLdepth) solve_plane(cx, cy, zPlane);
#endif
#ifdef DO_RGBA
            rgba[count][RCOMP] = solve_plane_0_255(cx, cy, rPlane);
            rgba[count][GCOMP] = solve_plane_0_255(cx, cy, gPlane);
            rgba[count][BCOMP] = solve_plane_0_255(cx, cy, bPlane);
            rgba[count][ACOMP] = (GLubyte) (solve_plane_0_255(cx, cy, aPlane) * coverage);
#endif
#ifdef DO_INDEX
            {
               GLint frac = compute_coveragei(pMin, pMid, pMax, ix, iy);
               GLint indx = (GLint) solve_plane(cx, cy, iPlane);
               index[count] = (indx & ~0xf) | frac;
            }
#endif
#ifdef DO_SPEC
            spec[count][RCOMP] = solve_plane_0_255(cx, cy, srPlane);
            spec[count][GCOMP] = solve_plane_0_255(cx, cy, sgPlane);
            spec[count][BCOMP] = solve_plane_0_255(cx, cy, sbPlane);
#endif
#ifdef DO_STUV0
            {
               const GLfloat invQ = solve_plane_recip(cx, cy, v0Plane);
               s[0][count] = solve_plane(cx, cy, s0Plane) * invQ;
               t[0][count] = solve_plane(cx, cy, t0Plane) * invQ;
               u[0][count] = solve_plane(cx, cy, u0Plane) * invQ;
               lambda[0][count] = compute_lambda(s0Plane, t0Plane, invQ,
                                                 width0, height0);
            }
#endif
#ifdef DO_STUV1
            {
               const GLfloat invQ = solve_plane_recip(cx, cy, v1Plane);
               s[1][count] = solve_plane(cx, cy, s1Plane) * invQ;
               t[1][count] = solve_plane(cx, cy, t1Plane) * invQ;
               u[1][count] = solve_plane(cx, cy, u1Plane) * invQ;
               lambda[1][count] = compute_lambda(s1Plane, t1Plane, invQ,
                                                 width1, height1);
            }
#endif
            ix++;
            count++;
            coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
         }

         n = (GLuint) ix - (GLuint) startX;
#ifdef DO_STUV1
#  ifdef DO_SPEC
         gl_write_multitexture_span(ctx, 2, n, startX, iy, z,
                                    (CONST GLfloat (*)[MAX_WIDTH]) s,
                                    (CONST GLfloat (*)[MAX_WIDTH]) t,
                                    (CONST GLfloat (*)[MAX_WIDTH]) u,
                                    (GLfloat (*)[MAX_WIDTH]) lambda,
                                    rgba, (CONST GLubyte (*)[4]) spec,
                                    GL_POLYGON);
#  else
         gl_write_multitexture_span(ctx, 2, n, startX, iy, z,
                                    (CONST GLfloat (*)[MAX_WIDTH]) s,
                                    (CONST GLfloat (*)[MAX_WIDTH]) t,
                                    (CONST GLfloat (*)[MAX_WIDTH]) u,
                                    lambda, rgba, NULL, GL_POLYGON);
#  endif
#elif defined(DO_STUV0)
#  ifdef DO_SPEC
         gl_write_texture_span(ctx, n, startX, iy, z,
                               s[0], t[0], u[0], lambda[0], rgba,
                               (CONST GLubyte (*)[4]) spec, GL_POLYGON);
#  else
         gl_write_texture_span(ctx, n, startX, iy, z,
                               s[0], t[0], u[0], lambda[0],
                               rgba, NULL, GL_POLYGON);
#  endif
#elif defined(DO_RGBA)
         gl_write_rgba_span(ctx, n, startX, iy, z, rgba, GL_POLYGON);
#elif defined(DO_INDEX)
         gl_write_index_span(ctx, n, startX, iy, z, index, GL_POLYGON);
#endif
      }
   }
   else {
      /* scan right to left */
      const GLfloat *pMin = VB->Win.data[vMin];
      const GLfloat *pMid = VB->Win.data[vMid];
      const GLfloat *pMax = VB->Win.data[vMax];
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      GLfloat x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, left, startX = (GLint) (x + xAdj);
         GLuint count, n;
         GLfloat coverage = 0.0F;
         /* skip fragments with zero coverage */
         while (startX >= 0) {
            coverage = compute_coveragef(pMin, pMax, pMid, startX, iy);
            if (coverage > 0.0F)
               break;
            startX--;
         }

         if (startX > ctx->DrawBuffer->Xmax) {
            startX = ctx->DrawBuffer->Xmax;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            /* (cx,cy) = center of fragment */
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
#ifdef DO_Z
            z[ix] = (GLdepth) solve_plane(cx, cy, zPlane);
#endif
#ifdef DO_RGBA
            rgba[ix][RCOMP] = solve_plane_0_255(cx, cy, rPlane);
            rgba[ix][GCOMP] = solve_plane_0_255(cx, cy, gPlane);
            rgba[ix][BCOMP] = solve_plane_0_255(cx, cy, bPlane);
            rgba[ix][ACOMP] = (GLubyte) (solve_plane_0_255(cx, cy, aPlane) * coverage);
#endif
#ifdef DO_INDEX
            {
               GLint frac = compute_coveragei(pMin, pMax, pMid, ix, iy);
               GLint indx = (GLint) solve_plane(cx, cy, iPlane);
               index[ix] = (indx & ~0xf) | frac;
            }
#endif
#ifdef DO_SPEC
            spec[ix][RCOMP] = solve_plane_0_255(cx, cy, srPlane);
            spec[ix][GCOMP] = solve_plane_0_255(cx, cy, sgPlane);
            spec[ix][BCOMP] = solve_plane_0_255(cx, cy, sbPlane);
#endif
#ifdef DO_STUV0
            {
               const GLfloat invQ = solve_plane_recip(cx, cy, v0Plane);
               s[0][ix] = solve_plane(cx, cy, s0Plane) * invQ;
               t[0][ix] = solve_plane(cx, cy, t0Plane) * invQ;
               u[0][ix] = solve_plane(cx, cy, u0Plane) * invQ;
               lambda[0][ix] = compute_lambda(s0Plane, t0Plane, invQ,
                                              width0, height0);
            }
#endif
#ifdef DO_STUV1
            {
               const GLfloat invQ = solve_plane_recip(cx, cy, v1Plane);
               s[1][ix] = solve_plane(cx, cy, s1Plane) * invQ;
               t[1][ix] = solve_plane(cx, cy, t1Plane) * invQ;
               u[1][ix] = solve_plane(cx, cy, u1Plane) * invQ;
               lambda[1][ix] = compute_lambda(s1Plane, t1Plane, invQ,
                                              width1, height1);
            }
#endif
            ix--;
            count++;
            coverage = compute_coveragef(pMin, pMax, pMid, ix, iy);
         }

         n = (GLuint) startX - (GLuint) ix;
         left = ix + 1;
#ifdef DO_STUV1
         {
            GLuint j;
            for (j = 0; j < n; j++) {
               s[0][j] = s[0][j + left];
               t[0][j] = t[0][j + left];
               u[0][j] = u[0][j + left];
               s[1][j] = s[1][j + left];
               t[1][j] = t[1][j + left];
               u[1][j] = u[1][j + left];
               lambda[0][j] = lambda[0][j + left];
               lambda[1][j] = lambda[1][j + left];
            }
         }
#  ifdef DO_SPEC
         gl_write_multitexture_span(ctx, 2, n, left, iy, z + left,
                                    (CONST GLfloat (*)[MAX_WIDTH]) s,
                                    (CONST GLfloat (*)[MAX_WIDTH]) t,
                                    (CONST GLfloat (*)[MAX_WIDTH]) u,
                                    lambda, rgba + left,
                                    (CONST GLubyte (*)[4]) (spec + left),
                                    GL_POLYGON);
#  else
         gl_write_multitexture_span(ctx, 2, n, left, iy, z + left,
                                    (CONST GLfloat (*)[MAX_WIDTH]) s,
                                    (CONST GLfloat (*)[MAX_WIDTH]) t,
                                    (CONST GLfloat (*)[MAX_WIDTH]) u,
                                    lambda,
                                    rgba + left, NULL, GL_POLYGON);
#  endif
#elif defined(DO_STUV0)
#  ifdef DO_SPEC
         gl_write_texture_span(ctx, n, left, iy, z + left,
                               s[0] + left, t[0] + left, u[0] + left,
                               lambda[0] + left, rgba + left,
                               (CONST GLubyte (*)[4]) (spec + left),
                               GL_POLYGON);
#  else
         gl_write_texture_span(ctx, n, left, iy, z + left,
                               s[0] + left, t[0] + left,
                               u[0] + left, lambda[0] + left,
                               rgba + left, NULL, GL_POLYGON);
#  endif
#elif defined(DO_RGBA)
         gl_write_rgba_span(ctx, n, left, iy, z + left,
                            rgba + left, GL_POLYGON);
#elif defined(DO_INDEX)
         gl_write_index_span(ctx, n, left, iy, z + left,
                             index + left, GL_POLYGON);
#endif
      }
   }
}


#ifdef DO_Z
#undef DO_Z
#endif

#ifdef DO_RGBA
#undef DO_RGBA
#endif

#ifdef DO_INDEX
#undef DO_INDEX
#endif

#ifdef DO_SPEC
#undef DO_SPEC
#endif

#ifdef DO_STUV0
#undef DO_STUV0
#endif

#ifdef DO_STUV1
#undef DO_STUV1
#endif

#ifdef DO_OCCLUSION_TEST
#undef DO_OCCLUSION_TEST
#endif
