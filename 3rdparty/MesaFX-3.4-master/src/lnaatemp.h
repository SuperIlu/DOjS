/* $Id: lnaatemp.h,v 1.6.4.1 2000/11/05 21:24:01 brianp Exp $ */

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
 * Antialiased Line Rasterizer Template
 *
 * This file is #include'd to generate custom line rasterizers.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be interplated along the line:
 *    INTERP_RGBA   - if defined, interpolate RGBA values
 *    INTERP_SPEC   - if defined, interpolate specular RGB values
 *    INTERP_INDEX  - if defined, interpolate color index values
 *    INTERP_STUV0  - if defined, interpolate unit 0 STU texcoords with
 *                        perspective correction
 *                        NOTE:  OpenGL STRQ = Mesa STUV (R was taken for red)
 *    INTERP_STUV1  - if defined, interpolate unit 1 STU texcoords
 *
 * Optionally, one may provide one-time setup code
 *    SETUP_CODE    - code which is to be executed once per line
 *
 * To actually "plot" each pixel the PLOT macro must be defined.
 *
 * This code was designed for the origin to be in the lower-left corner.
 */

/* void aa_line( GLcontext *ctx, GLuint vert0, GLuint vert1, GLuint pvert ) */
{
   const struct vertex_buffer *VB = ctx->VB;
   struct pixel_buffer *pb = ctx->PB;
   GLfloat halfWidth = 0.5F * ctx->Line.Width;  /* 0.5 is a bit of a hack */
   GLboolean solid = !ctx->Line.StippleFlag;
   GLint x0 = (GLint) VB->Win.data[vert0][0];
   GLint x1 = (GLint) VB->Win.data[vert1][0];
   GLint y0 = (GLint) VB->Win.data[vert0][1];
   GLint y1 = (GLint) VB->Win.data[vert1][1];
   GLint dx = x1 - x0;
   GLint dy = y1 - y0;
   GLint xStep, yStep;
   GLint z0, z1;
   const GLint depthBits = ctx->Visual->DepthBits;
   const GLint fixedToDepthShift = depthBits <= 16 ? FIXED_SHIFT : 0;
#define FixedToDepth(F)  ((F) >> fixedToDepthShift)
#ifdef INTERP_RGBA
   GLfixed fr, fg, fb, fa;      /* fixed-pt RGBA */
   GLfixed dfr, dfg, dfb, dfa;  /* fixed-pt RGBA deltas */
#endif
#ifdef INTERP_SPEC
   GLfixed fsr, fsg, fsb;      /* fixed-pt specular RGBA */
   GLfixed dfsr, dfsg, dfsb;   /* fixed-pt specular RGBA deltas */
#endif
#ifdef INTERP_INDEX
   GLfixed fi, dfi;
#endif
#if defined(INTERP_STUV0) || defined(INTERP_STUV1)
   GLfloat invw0 = VB->Win.data[vert0][3];
   GLfloat invw1 = VB->Win.data[vert1][3];
#endif
#ifdef INTERP_STUV0
   /* h denotes hyperbolic */
   GLfloat hs0 = invw0 * VB->TexCoordPtr[0]->data[vert0][0];
   GLfloat dhs = invw1 * VB->TexCoordPtr[0]->data[vert1][0] - hs0;
   GLfloat ht0 = invw0 * VB->TexCoordPtr[0]->data[vert0][1];
   GLfloat dht = invw1 * VB->TexCoordPtr[0]->data[vert1][1] - ht0;
   GLfloat hu0 = 0, dhu = 0;
   GLfloat hv0 = invw0, dhv = invw1 - invw0;
#endif
#ifdef INTERP_STUV1
   GLfloat hs01 = invw0 * VB->TexCoordPtr[1]->data[vert0][0];
   GLfloat dhs1 = invw1 * VB->TexCoordPtr[1]->data[vert1][0] - hs01;
   GLfloat ht01 = invw0 * VB->TexCoordPtr[1]->data[vert0][1];
   GLfloat dht1 = invw1 * VB->TexCoordPtr[1]->data[vert1][1] - ht01;
   GLfloat hu01 = 0, dhu1 = 0;
   GLfloat hv01 = invw0, dhv1 = invw1 - invw0;
#endif

   if (dx == 0 && dy == 0)
      return;

   ctx->PB->mono = GL_FALSE;

   if (depthBits <= 16) {
      z0 = FloatToFixed(VB->Win.data[vert0][2] + ctx->LineZoffset);
      z1 = FloatToFixed(VB->Win.data[vert1][2] + ctx->LineZoffset);
   }
   else {
      z0 = (GLint) (VB->Win.data[vert0][2] + ctx->LineZoffset);
      z1 = (GLint) (VB->Win.data[vert1][2] + ctx->LineZoffset);
   }

#ifdef INTERP_STUV0
   if (VB->TexCoordPtr[0]->size > 2) {
      hu0 = invw0 * VB->TexCoordPtr[0]->data[vert0][2];
      dhu = invw1 * VB->TexCoordPtr[0]->data[vert1][2] - hu0;
      if (VB->TexCoordPtr[0]->size > 3) {
         hv0 = invw0 * VB->TexCoordPtr[0]->data[vert0][3];
         dhv = invw1 * VB->TexCoordPtr[0]->data[vert1][3] - hv0;
      }
   }
#endif

#ifdef INTERP_STUV1
   if (VB->TexCoordPtr[1]->size > 2) {
      hu01 = invw0 * VB->TexCoordPtr[1]->data[vert0][2];
      dhu1 = invw1 * VB->TexCoordPtr[1]->data[vert1][2] - hu01;
      if (VB->TexCoordPtr[1]->size > 3) {
         hv01 = invw0 * VB->TexCoordPtr[1]->data[vert0][3];
         dhv1 = invw1 * VB->TexCoordPtr[1]->data[vert1][3] - hv01;
      }
   }
#endif

#ifdef INTERP_RGBA
   if (ctx->Light.ShadeModel == GL_SMOOTH) {
      fr = IntToFixed(VB->ColorPtr->data[vert0][0]);
      fg = IntToFixed(VB->ColorPtr->data[vert0][1]);
      fb = IntToFixed(VB->ColorPtr->data[vert0][2]);
      fa = IntToFixed(VB->ColorPtr->data[vert0][3]);
   }
   else {
      fr = IntToFixed(VB->ColorPtr->data[pvert][0]);
      fg = IntToFixed(VB->ColorPtr->data[pvert][1]);
      fb = IntToFixed(VB->ColorPtr->data[pvert][2]);
      fa = IntToFixed(VB->ColorPtr->data[pvert][3]);
      dfr = dfg = dfb = dfa = 0;
   }
#endif
#ifdef INTERP_SPEC
   if (ctx->Light.ShadeModel == GL_SMOOTH) {
      fsr = IntToFixed(VB->Specular[vert0][0]);
      fsg = IntToFixed(VB->Specular[vert0][1]);
      fsb = IntToFixed(VB->Specular[vert0][2]);
   }
   else {
      fsr = IntToFixed(VB->Specular[pvert][0]);
      fsg = IntToFixed(VB->Specular[pvert][1]);
      fsb = IntToFixed(VB->Specular[pvert][2]);
      dfsr = dfsg = dfsb = 0;
   }
#endif
#ifdef INTERP_INDEX
   if (ctx->Light.ShadeModel == GL_SMOOTH) {
      fi = IntToFixed(VB->IndexPtr->data[vert0]);
   }
   else {
      fi = IntToFixed(VB->IndexPtr->data[pvert]);
      dfi = 0;
   }
#endif

   /*
    * Setup
    */
#ifdef SETUP_CODE
   SETUP_CODE
#endif

   if (dx < 0) {
      xStep = -1;
      dx = -dx;
   }
   else {
      xStep = 1;
   }

   if (dy < 0) {
      yStep = -1;
      dy = -dy;
   }
   else {
      yStep = 1;
   }

   if (dx > dy) {
      /*** X-major line ***/
      GLint i;
      GLint x = x0;
      GLfloat y = VB->Win.data[vert0][1];
      const GLfloat invDx = 1.0F / dx;
      GLfloat yStep = (VB->Win.data[vert1][1] - y) * invDx;
      GLint dz = (GLint) ((z1 - z0) * invDx);
#ifdef INTERP_RGBA
      if (ctx->Light.ShadeModel == GL_SMOOTH) {
         dfr = (IntToFixed(VB->ColorPtr->data[vert1][0]) - fr) * invDx;
         dfg = (IntToFixed(VB->ColorPtr->data[vert1][1]) - fg) * invDx;
         dfb = (IntToFixed(VB->ColorPtr->data[vert1][2]) - fb) * invDx;
         dfa = (IntToFixed(VB->ColorPtr->data[vert1][3]) - fa) * invDx;
      }
#endif
#ifdef INTERP_SPEC
      if (ctx->Light.ShadeModel == GL_SMOOTH) {
         dfsr = (IntToFixed(VB->Specular[vert1][0]) - fsr) * invDx;
         dfsg = (IntToFixed(VB->Specular[vert1][1]) - fsg) * invDx;
         dfsb = (IntToFixed(VB->Specular[vert1][2]) - fsb) * invDx;
      }
#endif
#ifdef INTERP_STUV0
      dhs *= invDx;
      dht *= invDx;
      dhu *= invDx;
      dhv *= invDx;
#endif
#ifdef INTERP_STUV1
      dhs1 *= invDx;
      dht1 *= invDx;
      dhu1 *= invDx;
      dhv1 *= invDx;
#endif
#ifdef INTERP_INDEX
      if (ctx->Light.ShadeModel == GL_SMOOTH) {
         dfi = (IntToFixed(VB->IndexPtr->data[vert1]) - fi) * invDx;
      }
#endif

      for (i = 0; i < dx; i++) {
         if (solid || (ctx->Line.StipplePattern & (1 << ((ctx->StippleCounter/ctx->Line.StippleFactor) & 0xf)))) {

            GLfloat yTop = y + halfWidth;
            GLfloat yBot = y - halfWidth;
            GLint yTopi = (GLint) yTop;
            GLint yBoti = (GLint) yBot;
            GLint iy;
#ifdef INTERP_RGBA
            GLubyte red   = FixedToInt(fr);
            GLubyte green = FixedToInt(fg);
            GLubyte blue  = FixedToInt(fb);
            GLubyte alpha = FixedToInt(fa);
            GLint coverage;
#endif
#ifdef INTERP_SPEC
            GLubyte specRed   = FixedToInt(fsr);
            GLubyte specGreen = FixedToInt(fsg);
            GLubyte specBlue  = FixedToInt(fsb);
#endif
#ifdef INTERP_INDEX
            GLuint index = FixedToInt(fi) & 0xfffffff0;
            GLuint coverage;
#endif
            GLdepth z = FixedToDepth(z0);
            ASSERT(yBoti <= yTopi);

            {
#ifdef INTERP_STUV0
               GLfloat invQ = 1.0F / hv0;
               GLfloat s = hs0 * invQ;
               GLfloat t = ht0 * invQ;
               GLfloat u = hu0 * invQ;
#endif
#ifdef INTERP_STUV1
               GLfloat invQ1 = 1.0F / hv01;
               GLfloat s1 = hs01 * invQ1;
               GLfloat t1 = ht01 * invQ1;
               GLfloat u1 = hu01 * invQ1;
#endif

               /* bottom pixel of swipe */
#ifdef INTERP_RGBA
               coverage = (GLint) (alpha * (1.0F - (yBot - yBoti)));
#endif
#ifdef INTERP_INDEX
               coverage = (GLuint) (15.0F * (1.0F - (yBot - yBoti)));
#endif
               PLOT(x, yBoti);
               yBoti++;

               /* top pixel of swipe */
#ifdef INTERP_RGBA
               coverage = (GLint) (alpha * (yTop - yTopi));
#endif
#ifdef INTERP_INDEX
               coverage = (GLuint) (15.0F * (yTop - yTopi));
#endif
               PLOT(x, yTopi);
               yTopi--;

               /* pixels between top and bottom with 100% coverage */
#ifdef INTERP_RGBA
               coverage = alpha;
#endif
#ifdef INTERP_INDEX
               coverage = 15;
#endif
               for (iy = yBoti; iy <= yTopi; iy++) {
                  PLOT(x, iy);
               }
            }
            PB_CHECK_FLUSH( ctx, pb );

         } /* if stippling */

         x += xStep;
         y += yStep;
         z0 += dz;
#ifdef INTERP_RGBA
         fr += dfr;
         fg += dfg;
         fb += dfb;
         fa += dfa;
#endif
#ifdef INTERP_SPEC
         fsr += dfsr;
         fsg += dfsg;
         fsb += dfsb;
#endif
#ifdef INTERP_STUV0
         hs0 += dhs;
         ht0 += dht;
         hu0 += dhu;
         hv0 += dhv;
#endif
#ifdef INTERP_STUV1
         hs01 += dhs1;
         ht01 += dht1;
         hu01 += dhu1;
         hv01 += dhv1;
#endif
#ifdef INTERP_INDEX
         fi += dfi;
#endif

         if (!solid)
            ctx->StippleCounter++;
      }
   }
   else {
      /*** Y-major line ***/
      GLint i;
      GLint y = y0;
      GLfloat x = VB->Win.data[vert0][0];
      const GLfloat invDy = 1.0F / dy;
      GLfloat xStep = (VB->Win.data[vert1][0] - x) * invDy;
      GLint dz = (GLint) ((z1 - z0) * invDy);
#ifdef INTERP_RGBA
      if (ctx->Light.ShadeModel == GL_SMOOTH) {
         dfr = (IntToFixed(VB->ColorPtr->data[vert1][0]) - fr) * invDy;
         dfg = (IntToFixed(VB->ColorPtr->data[vert1][1]) - fg) * invDy;
         dfb = (IntToFixed(VB->ColorPtr->data[vert1][2]) - fb) * invDy;
         dfa = (IntToFixed(VB->ColorPtr->data[vert1][3]) - fa) * invDy;
      }
#endif
#ifdef INTERP_SPEC
      if (ctx->Light.ShadeModel == GL_SMOOTH) {
         dfsr = (IntToFixed(VB->Specular[vert1][0]) - fsr) * invDy;
         dfsg = (IntToFixed(VB->Specular[vert1][1]) - fsg) * invDy;
         dfsb = (IntToFixed(VB->Specular[vert1][2]) - fsb) * invDy;
      }
#endif
#ifdef INTERP_STUV0
      dhs *= invDy;
      dht *= invDy;
      dhu *= invDy;
      dhv *= invDy;
#endif
#ifdef INTERP_STUV1
      dhs1 *= invDy;
      dht1 *= invDy;
      dhu1 *= invDy;
      dhv1 *= invDy;
#endif
#ifdef INTERP_INDEX
      if (ctx->Light.ShadeModel == GL_SMOOTH) {
         dfi = (IntToFixed(VB->IndexPtr->data[vert1]) - fi) * invDy;
      }
#endif
      for (i = 0; i < dy; i++) {
         if (solid || (ctx->Line.StipplePattern & (1 << ((ctx->StippleCounter/ctx->Line.StippleFactor) & 0xf)))) {
            GLfloat xRight = x + halfWidth;
            GLfloat xLeft = x - halfWidth;
            GLint xRighti = (GLint) xRight;
            GLint xLefti = (GLint) xLeft;
            GLint ix;
#ifdef INTERP_RGBA
            GLubyte red   = FixedToInt(fr);
            GLubyte green = FixedToInt(fg);
            GLubyte blue  = FixedToInt(fb);
            GLubyte alpha = FixedToInt(fa);
            GLint coverage;
#endif
#ifdef INTERP_SPEC
            GLubyte specRed   = FixedToInt(fsr);
            GLubyte specGreen = FixedToInt(fsg);
            GLubyte specBlue  = FixedToInt(fsb);
#endif
#ifdef INTERP_INDEX
            GLuint index = FixedToInt(fi) & 0xfffffff0;
            GLuint coverage;
#endif
            GLdepth z = FixedToDepth(z0);

            ASSERT(xLefti < xRight);

            {
#ifdef INTERP_STUV0
               GLfloat invQ = 1.0F / hv0;
               GLfloat s = hs0 * invQ;
               GLfloat t = ht0 * invQ;
               GLfloat u = hu0 * invQ;
#endif
#ifdef INTERP_STUV1
               GLfloat invQ1 = 1.0F / hv01;
               GLfloat s1 = hs01 * invQ1;
               GLfloat t1 = ht01 * invQ1;
               GLfloat u1 = hu01 * invQ1;
#endif

               /* left pixel of swipe */
#ifdef INTERP_RGBA
               coverage = (GLint) (alpha * (1.0F - (xLeft - xLefti)));
#endif
#ifdef INTERP_INDEX
               coverage = (GLuint) (15.0F * (1.0F - (xLeft - xLefti)));
#endif
               PLOT(xLefti, y);
               xLefti++;

               /* right pixel of swipe */
#ifdef INTERP_RGBA
               coverage = (GLint) (alpha * (xRight - xRighti));
#endif
#ifdef INTERP_INDEX
               coverage = (GLuint) (15.0F * (xRight - xRighti));
#endif
               PLOT(xRighti, y)
               xRighti--;

               /* pixels between top and bottom with 100% coverage */
#ifdef INTERP_RGBA
               coverage = alpha;
#endif
#ifdef INTERP_INDEX
               coverage = 15;
#endif
               for (ix = xLefti; ix <= xRighti; ix++) {
                  PLOT(ix, y);
               }
            }
            PB_CHECK_FLUSH( ctx, pb );
         }

         x += xStep;
         y += yStep;
         z0 += dz;
#ifdef INTERP_RGBA
         fr += dfr;
         fg += dfg;
         fb += dfb;
         fa += dfa;
#endif
#ifdef INTERP_SPEC
         fsr += dfsr;
         fsg += dfsg;
         fsb += dfsb;
#endif
#ifdef INTERP_STUV0
         hs0 += dhs;
         ht0 += dht;
         hu0 += dhu;
         hv0 += dhv;
#endif
#ifdef INTERP_STUV1
         hs01 += dhs1;
         ht01 += dht1;
         hu01 += dhu1;
         hv01 += dhv1;
#endif
#ifdef INTERP_INDEX
         fi += dfi;
#endif
         if (!solid)
            ctx->StippleCounter++;
      }
   }
}

#undef INTERP_RGBA
#undef INTERP_SPEC
#undef INTERP_STUV0
#undef INTERP_STUV1
#undef INTERP_INDEX
#undef PLOT
