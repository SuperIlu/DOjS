/* $Id: ddsample.c,v 1.6 2000/04/22 01:07:07 brianp Exp $ */

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


/*
 * This is a sample template for writing new Mesa device drivers.
 * You'll have to rewrite much of the code below.
 *
 * Let's say you're interfacing Mesa to a window/operating system
 * called FOO.  Replace all occurances of FOOMesa with the real name
 * you select for your interface  (i.e. XMesa, WMesa, AmigaMesa).
 *
 * You'll have to design an API for clients to use, defined in a
 * header called Mesa/include/GL/FooMesa.h  Use the sample as an
 * example.  The API should at least have functions for creating
 * rendering contexts, binding rendering contexts to windows/frame
 * buffers, etc.
 * 
 * Next, you'll have to write implementations for the device driver
 * functions described in dd.h
 *
 * Note that you'll usually have to flip Y coordinates since Mesa's
 * window coordinates start at the bottom and increase upward.  Most
 * window system's Y-axis increases downward
 *
 * Functions marked OPTIONAL may be completely omitted by your driver.
 *
 * Your Makefile should compile this module along with the rest of
 * the core Mesa library.
 */

/*
 * XXX XXX
 * THIS FILE IS VERY OUT OF DATE!  LOOK AT OSmesa/osmesa.c FOR A BETTER
 * EXAMPLE DRIVER!
 */



#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "GL/FooMesa.h"
#include "context.h"
#include "matrix.h"
#include "types.h"
#include "vb.h"
#endif



/*
 * In C++ terms, this class derives from the GLvisual class.
 * Add system-specific fields to it.
 */
struct foo_mesa_visual {
   GLvisual *gl_visual;
   GLboolean db_flag;		/* double buffered? */
   GLboolean rgb_flag;		/* RGB mode? */
   GLuint depth;		/* bits per pixel (1, 8, 24, etc) */
};


/*
 * In C++ terms, this class derives from the GLframebuffer class.
 * Add system-specific fields to it.
 */
struct foo_mesa_buffer {
   GLframebuffer *gl_buffer;	/* The depth, stencil, accum, etc buffers */
   void *the_window;		/* your window handle, etc */
   int Width, Height;
};



/*
 * In C++ terms, this class derives from the GLcontext class.
 * Add system-specific fields to it.
 */
struct foo_mesa_context {
   GLcontext *gl_ctx;		/* the core library context */
   FooMesaVisual visual;
   FooMesaBuffer Buffer;
   GLuint ClearIndex;
   GLubyte ClearColor[4];
   GLuint CurrentIndex;
   GLubyte CurrentColor[4];
   /* etc... */
};


static FooMesaContext CurrentContext = 0;
static FooMesaBuffer CurrentBuffer = 0;



static void setup_DD_pointers( GLcontext *ctx );


/**********************************************************************/
/*****              Miscellaneous device driver funcs             *****/
/**********************************************************************/


static void finish( GLcontext *ctx )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /* OPTIONAL FUNCTION: implements glFinish if possible */
}



static void flush( GLcontext *ctx )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /* OPTIONAL FUNCTION: implements glFlush if possible */
}



static void clear_index( GLcontext *ctx, GLuint index )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   foo->ClearIndex = index;
}



static void clear_color( GLcontext *ctx, GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   foo->ClearColor[0] = r;
   foo->ClearColor[1] = g;
   foo->ClearColor[2] = b;
   foo->ClearColor[3] = a;
}



static GLbitfield clear( GLcontext *ctx, GLbitfield mask, GLboolean all,
                        GLint x, GLint y, GLint width, GLint height )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
/*
 * Clear the specified region of the buffers indicated by 'mask'
 * using the clear color or index as specified by one of the two
 * functions above.
 * If all==GL_TRUE, clear whole buffer, else just clear region defined
 * by x,y,width,height
 */

   return mask;  /* return mask of buffers remaining to be cleared */
}



static void set_index( GLcontext *ctx, GLuint index )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   foo->CurrentIndex = index;
}



static void set_color( GLcontext *ctx, GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   foo->CurrentColor[0] = r;
   foo->CurrentColor[1] = g;
   foo->CurrentColor[2] = b;
   foo->CurrentColor[3] = a;
}



/*
 * OPTIONAL FUNCTION: implement glIndexMask if possible, else
 * return GL_FALSE
 */
static GLboolean index_mask( GLcontext *ctx, GLuint mask )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   return GL_FALSE;
}



/*
 * OPTIONAL FUNCTION: implement glColorMask if possible, else
 * return GL_FALSE
 */
static GLboolean color_mask( GLcontext *ctx, 
                             GLboolean rmask, GLboolean gmask,
                             GLboolean bmask, GLboolean amask)
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   return GL_FALSE;
}



/*
 * OPTIONAL FUNCTION: 
 * Implements glLogicOp if possible.  Return GL_TRUE if the device driver
 * can perform the operation, otherwise return GL_FALSE.  If GL_FALSE
 * is returned, the logic op will be done in software by Mesa.
 */
static GLboolean logicop( GLcontext *ctx, GLenum op )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   return GL_FALSE;
}



/*
 * OPTIONAL FUNCTION: enable/disable dithering if applicable
 */
static void dither( GLcontext *ctx, GLboolean enable )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
}



/*
 * Set the current reading buffer.
 */
static void set_read_buffer( GLcontext *ctx, GLframebuffer *bufer,
                             GLenum mode )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   setup_DD_pointers( ctx );
}


/*
 * Set the destination/draw buffer.
 */
static GLboolean set_draw_buffer( GLcontext *ctx, GLenum mode )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   setup_DD_pointers( ctx );
   return GL_TRUE;
}



/*
 * Return the width and height of the current buffer.
 * If anything special has to been done when the buffer/window is
 * resized, do it now.
 */
static void get_buffer_size( GLcontext *ctx, GLuint *width, GLuint *height )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;

   *width  = foo->Buffer->Width  = 12345;    /* XXX fix these */
   *height = foo->Buffer->Height = 12345;

}



/**********************************************************************/
/*****            Dummy functions                                 *****/
/**********************************************************************/

static void WriteCIPixel( GLint x, GLint y, GLuint index )
{
}

static void WriteRGBAPixel( GLint x, GLint y, const GLubyte color[4] )
{
}

static void WriteRGBPixel( GLint x, GLint y, const GLubyte color[3] )
{
}

static GLuint ReadCIPixel( GLint x, GLint y )
{
   return 0;
}

static void ReadRGBAPixel( GLint x, GLint y, GLubyte color[4] )
{
}

#define FLIP(y)  foo->Buffer->Height - (y) - 1;


/**********************************************************************/
/*****           Accelerated point, line, triangle rendering      *****/
/**********************************************************************/


/* There may several functions like this, for different screen modes, etc */
static void fast_points_function( GLcontext *ctx, GLuint first, GLuint last )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   struct vertex_buffer *VB = ctx->VB;
   GLint i;

   for (i=first;i<=last;i++) {
      if (VB->ClipMask[i]==0) {
         int x, y;
         x =       (GLint) VB->Win.data[i][0];
         y = FLIP( (GLint) VB->Win.data[i][1] );
         WriteRGBAPixel( x, y, VB->ColorPtr->data[i] );
      }
   }
}




/* There may several functions like this, for different screen modes, etc */
static void fast_line_function( GLcontext *ctx, GLuint v0, GLuint v1, GLuint pv )
{
   /* Render a line by some hardware/OS accerated method */
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   struct vertex_buffer *VB = ctx->VB;
   int x0, y0, x1, y1;
   GLubyte *pixel;

   pixel = VB->ColorPtr->data[pv];

   x0 =       (int) VB->Win.data[v0][0];
   y0 = FLIP( (int) VB->Win.data[v0][1] );
   x1 =       (int) VB->Win.data[v1][0];
   y1 = FLIP( (int) VB->Win.data[v1][1] );

   /* Draw line from (x0,y0) to (x1,y1) with current pixel color/index */
}




/* There may several functions like this, for different screen modes, etc */
static void fast_triangle_function( GLcontext *ctx, GLuint v0,
                                    GLuint v1, GLuint v2, GLuint pv )
{
   /* Render a triangle by some hardware/OS accerated method */
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   struct vertex_buffer *VB = ctx->VB;
   GLubyte *pixel;

   pixel = VB->ColorPtr->data[pv];

   /* Draw triangle with current pixel color/index */
}



/**********************************************************************/
/*****            Write spans of pixels                           *****/
/**********************************************************************/


static void write_index8_span( const GLcontext *ctx,
                               GLuint n, GLint x, GLint y,
                               const GLubyte index[],
                               const GLubyte mask[] )
{
   GLint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         WriteCIPixel(x+i, y, index[i]);
      }
   }
}


static void write_index32_span( const GLcontext *ctx,
                                GLuint n, GLint x, GLint y,
                                const GLuint index[],
                                const GLubyte mask[] )
{
   GLint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         WriteCIPixel(x+i, y, index[i]);
      }
   }
}



static void write_mono_index_span( const GLcontext *ctx,
                                   GLuint n, GLint x, GLint y,
                                   const GLubyte mask[] )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         WriteCIPixel(x+i, y, foo->CurrentIndex);
      }
   }
}



static void write_rgba_span( const GLcontext *ctx, GLuint n, GLint x, GLint y,
                             const GLubyte rgba[][4], const GLubyte mask[] )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   GLuint i;
   y = FLIP(y);
   if (mask) {
      /* draw some pixels */
      for (i=0; i<n; i++) {
         if (mask[i]) {
            WriteRGBAPixel( x+i, y, foo->CurrentColor );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0; i<n; i++) {
         WriteRGBAPixel( x+i, y, foo->CurrentColor );
      }
   }
}


static void write_rgb_span( const GLcontext *ctx, GLuint n, GLint x, GLint y,
                            const GLubyte rgb[][3], const GLubyte mask[] )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   GLuint i;
   y = FLIP(y);
   if (mask) {
      /* draw some pixels */
      for (i=0; i<n; i++) {
         if (mask[i]) {
            WriteRGBPixel( x+i, y, rgb[i] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0; i<n; i++) {
         WriteRGBPixel( x+i, y, rgb[i] );
      }
   }
}



static void write_mono_rgba_span( const GLcontext *ctx,
                                  GLuint n, GLint x, GLint y,
                                  const GLubyte mask[])
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   GLint i;
   y = FLIP(y);
   for (i=0; i<n; i++) {
      if (mask[i]) {
         WriteRGBAPixel( x+i, y, foo->CurrentColor );
      }
   }
}



/**********************************************************************/
/*****                 Read spans of pixels                       *****/
/**********************************************************************/


static void read_index_span( const GLcontext *ctx,
                             GLuint n, GLint x, GLint y, GLuint index[])
{
   GLint i;
   for (i=0; i<n; i++) {
      index[i] = ReadCIPixel( x+i, y );
   }
}



static void read_rgba_span( const GLcontext *ctx, GLuint n, GLint x, GLint y,
                            GLubyte rgba[][4] )
{
   int i;
   for (i=0; i<n; i++) {
      ReadRGBAPixel( x+i, y, rgba[i] );
   }
}



/**********************************************************************/
/*****              Write arrays of pixels                        *****/
/**********************************************************************/


static void write_index_pixels( const GLcontext *ctx,
                                GLuint n, const GLint x[], const GLint y[],
                                const GLuint index[], const GLubyte mask[] )
{
   GLint i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         WriteCIPixel( x[i], y[i], index[i] );
      }
   }
}



static void write_mono_index_pixels( const GLcontext *ctx,
                                     GLuint n,
                                     const GLint x[], const GLint y[],
                                     const GLubyte mask[] )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   GLint i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         WriteCIPixel( x[i], y[i], foo->CurrentIndex );
      }
   }
}



static void write_rgba_pixels( const GLcontext *ctx,
                               GLuint n, const GLint x[], const GLint y[],
                               const GLubyte rgba[][4], const GLubyte mask[] )
{
   GLint i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         WriteRGBAPixel( x[i], y[i], rgba[i] );
      }
   }
}



static void write_mono_rgba_pixels( const GLcontext *ctx,
                                    GLuint n, const GLint x[], const GLint y[],
                                    const GLubyte mask[] )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   GLint i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         WriteRGBAPixel( x[i], y[i], foo->CurrentColor );
      }
   }
}




/**********************************************************************/
/*****                   Read arrays of pixels                    *****/
/**********************************************************************/

/* Read an array of color index pixels. */
static void read_index_pixels( const GLcontext *ctx,
                               GLuint n, const GLint x[], const GLint y[],
                               GLuint index[], const GLubyte mask[] )
{
   GLint i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         index[i] = ReadCIPixel( x[i], y[i] );
      }
   }
}



static void read_rgba_pixels( const GLcontext *ctx,
                              GLuint n, const GLint x[], const GLint y[],
                              GLubyte rgba[][4], const GLubyte mask[] )
{
   GLint i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         ReadRGBAPixel( x[i], y[i], rgba[i] );
      }
   }
}




/**********************************************************************/
/**********************************************************************/


static void setup_DD_pointers( GLcontext *ctx )
{
   /* Initialize all the pointers in the DD struct.  Do this whenever */
   /* a new context is made current or we change buffers via set_buffer! */

   ctx->Driver.UpdateState = setup_DD_pointers;

   ctx->Driver.ClearIndex = clear_index;
   ctx->Driver.ClearColor = clear_color;
   ctx->Driver.Clear = clear;

   ctx->Driver.Index = set_index;
   ctx->Driver.Color = set_color;

   ctx->Driver.SetDrawBuffer = set_draw_buffer;
   ctx->Driver.SetReadBuffer = set_read_buffer;
   ctx->Driver.GetBufferSize = get_buffer_size;

   ctx->Driver.PointsFunc = fast_points_function;
   ctx->Driver.LineFunc = fast_line_function;
   ctx->Driver.TriangleFunc = fast_triangle_function;

   /* Pixel/span writing functions: */
   ctx->Driver.WriteRGBASpan       = write_rgba_span;
   ctx->Driver.WriteRGBSpan        = write_rgb_span;
   ctx->Driver.WriteMonoRGBASpan   = write_mono_rgba_span;
   ctx->Driver.WriteRGBAPixels     = write_rgba_pixels;
   ctx->Driver.WriteMonoRGBAPixels = write_mono_rgba_pixels;

   ctx->Driver.WriteCI32Span       = write_index32_span;
   ctx->Driver.WriteCI8Span        = write_index8_span;
   ctx->Driver.WriteMonoCISpan     = write_mono_index_span;
   ctx->Driver.WriteCI32Pixels     = write_index_pixels;
   ctx->Driver.WriteMonoCIPixels   = write_mono_index_pixels;

   /* Pixel/span reading functions: */
   ctx->Driver.ReadCI32Span        = read_index_span;
   ctx->Driver.ReadRGBASpan        = read_rgba_span;
   ctx->Driver.ReadCI32Pixels      = read_index_pixels;
   ctx->Driver.ReadRGBAPixels      = read_rgba_pixels;


   /*
    * OPTIONAL FUNCTIONS:  these may be left uninitialized if the device
    * driver can't/needn't implement them.
    */
   ctx->Driver.Finish = finish;
   ctx->Driver.Flush = flush;
   ctx->Driver.IndexMask = index_mask;
   ctx->Driver.ColorMask = color_mask;
   ctx->Driver.LogicOp = logicop;
   ctx->Driver.Dither = dither;
}



/**********************************************************************/
/*****               FOO/Mesa Public API Functions                *****/
/**********************************************************************/



/*
 * The exact arguments to this function will depend on your window system
 */
FooMesaVisual FooMesaCreateVisual( GLboolean rgb_mode, GLboolean dbFlag,
                                   GLint depthSize, GLint stencilSize,
                                   GLint accumSize )
{
   FooMesaVisual v;
   GLint redBits, greenBits, blueBits, alphaBits, indexBits;
   GLboolean alphaFlag = GL_FALSE;

   v = (FooMesaVisual) calloc( 1, sizeof(struct foo_mesa_visual) );
   if (!v) {
      return NULL;
   }

   if (rgb_mode) {
      /* RGB(A) mode */
      redBits = 8;         /* XXX 8 is just an example */
      greenBits = 8;
      blueBits = 8;
      alphaBits = 8;
      indexBits = 0;
   }
   else {
      /* color index mode */
      redBits = 0;
      greenBits = 0;
      blueBits = 0;
      alphaBits = 0;
      indexBits = 8;   /* XXX 8 is just an example */
   }

   /* Create core visual */
   v->gl_visual = gl_create_visual( rgb_mode, 
                                    alphaFlag,
                                    dbFlag,
                                    GL_FALSE,  /* stereo */
                                    depthSize,
                                    stencilSize,
                                    accumSize,
                                    indexBits,
                                    redBits, greenBits, blueBits, alphaBits );

   return v;
}



void FooMesaDestroyVisual( FooMesaVisual v )
{
   gl_destroy_visual( v->gl_visual );
   free( v );
}




FooMesaBuffer FooMesaCreateBuffer( FooMesaVisual visual,
                                   void *your_window_id )
{
   FooMesaBuffer b;

   b = (FooMesaBuffer) calloc( 1, sizeof(struct foo_mesa_buffer) );
   if (!b) {
      return NULL;
   }

   b->gl_buffer = gl_create_framebuffer( visual->gl_visual,
                                         visual->gl_visual->DepthBits > 0,
                                         visual->gl_visual->StencilBits > 0,
                                         visual->gl_visual->AccumBits > 0,
                                         visual->gl_visual->AlphaBits > 0 );
   b->the_window = your_window_id;

   /* other stuff */

   return b;
}



void FooMesaDestroyBuffer( FooMesaBuffer b )
{
   gl_destroy_framebuffer( b->gl_buffer );
   free( b );
}




FooMesaContext FooMesaCreateContext( FooMesaVisual visual,
                                     FooMesaContext share )
{
   FooMesaContext c;
   GLboolean direct = GL_FALSE;

   c = (FooMesaContext) calloc( 1, sizeof(struct foo_mesa_context) );
   if (!c) {
      return NULL;
   }

   c->gl_ctx = gl_create_context( visual->gl_visual,
                                  share ? share->gl_ctx : NULL,
                                  (void *) c, direct );


   /* you probably have to do a bunch of other initializations here. */


   /* and then, finally let the context examine your initializations */
   _mesa_initialize_context( c->gl_ctx );


   return c;
}



void FooMesaDestroyContext( FooMesaContext c )
{
   gl_destroy_context( c->gl_ctx );
   free( c );
}



/*
 * Make the specified context and buffer the current one.
 */
void FooMesaMakeCurrent( FooMesaContext c, FooMesaBuffer b )
{
   if (c && b) {
      gl_make_current( c->gl_ctx, b->gl_buffer );
      CurrentContext = c;
      CurrentBuffer = b;
      if (c->gl_ctx->Viewport.Width==0) {
         /* initialize viewport to window size */
         gl_Viewport( c->gl_ctx, 0, 0, c->Buffer->Width, c->Buffer->Height );
      }
   }
   else {
      /* Detach */
      gl_make_current( NULL, NULL );
      CurrentContext = 0;
      CurrentBuffer = 0;
   }
}



void FooMesaSwapBuffers( FooMesaBuffer b )
{
   /* copy/swap back buffer to front if applicable */
}



/* you may need to add other FOO/Mesa functions too... */

