/* $Id: xmesa2.c,v 1.32.2.5 2001/05/15 20:14:51 brianp Exp $ */

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
 * Mesa/X11 interface, part 2.
 *
 * This file contains the implementations of all the device driver functions.
 */


#include "glxheader.h"
#include "context.h"
#include "drawpix.h"
#include "mem.h"
#include "state.h"
#include "depth.h"
#include "macros.h"
#include "vb.h"
#include "types.h"
#include "xmesaP.h"
#include "extensions.h"



static void update_span_funcs( GLcontext *ctx );


/*
 * The following functions are used to trap XGetImage() calls which
 * generate BadMatch errors if the drawable isn't mapped.
 */

#ifndef XFree86Server
static int caught_xgetimage_error = 0;
static int (*old_xerror_handler)( XMesaDisplay *dpy, XErrorEvent *ev );
static unsigned long xgetimage_serial;

/*
 * This is the error handler which will be called if XGetImage fails.
 */
static int xgetimage_error_handler( XMesaDisplay *dpy, XErrorEvent *ev )
{
   if (ev->serial==xgetimage_serial && ev->error_code==BadMatch) {
      /* caught the expected error */
      caught_xgetimage_error = 0;
   }
   else {
      /* call the original X error handler, if any.  otherwise ignore */
      if (old_xerror_handler) {
         (*old_xerror_handler)( dpy, ev );
      }
   }
   return 0;
}


/*
 * Call this right before XGetImage to setup error trap.
 */
static void catch_xgetimage_errors( XMesaDisplay *dpy )
{
   xgetimage_serial = NextRequest( dpy );
   old_xerror_handler = XSetErrorHandler( xgetimage_error_handler );
   caught_xgetimage_error = 0;
}


/*
 * Call this right after XGetImage to check if an error occured.
 */
static int check_xgetimage_errors( void )
{
   /* restore old handler */
   (void) XSetErrorHandler( old_xerror_handler );
   /* return 0=no error, 1=error caught */
   return caught_xgetimage_error;
}
#endif


/*
 * Read a pixel from an X drawable.
 */
static unsigned long read_pixel( XMesaDisplay *dpy,
                                 XMesaDrawable d, int x, int y )
{
   unsigned long p;
#ifndef XFree86Server
   XMesaImage *pixel = NULL;
   int error;

   catch_xgetimage_errors( dpy );
   pixel = XGetImage( dpy, d, x, y, 1, 1, AllPlanes, ZPixmap );
   error = check_xgetimage_errors();
   if (pixel && !error) {
      p = XMesaGetPixel( pixel, 0, 0 );
   }
   else {
      p = 0;
   }
   if (pixel) {
      XMesaDestroyImage( pixel );
   }
#else
   (*dpy->GetImage)(d, x, y, 1, 1, ZPixmap, ~0L, (pointer)&p);
#endif
   return p;
}




/*
 * Return the size (width,height of the current color buffer.
 * This function should be called by the glViewport function because
 * glViewport is often called when the window gets resized.  We need to
 * update some X/Mesa stuff when that happens.
 * Output:  width - width of buffer in pixels.
 *          height - height of buffer in pixels.
 */
static void get_buffer_size( GLcontext *ctx, GLuint *width, GLuint *height )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   unsigned int winwidth, winheight;
#ifndef XFree86Server
   Window root;
   int winx, winy;
   unsigned int bw, d;

   _glthread_LOCK_MUTEX(_xmesa_lock);
   XGetGeometry( xmesa->display, xmesa->xm_buffer->frontbuffer, &root,
		 &winx, &winy, &winwidth, &winheight, &bw, &d );
   _glthread_UNLOCK_MUTEX(_xmesa_lock);
#else

   winwidth = xmesa->xm_buffer->frontbuffer->width;
   winheight = xmesa->xm_buffer->frontbuffer->height;
#endif

   *width = winwidth;
   *height = winheight;

   if (   winwidth!=xmesa->xm_buffer->width
       || winheight!=xmesa->xm_buffer->height) {
      xmesa->xm_buffer->width = winwidth;
      xmesa->xm_buffer->height = winheight;
      xmesa_alloc_back_buffer( xmesa->xm_buffer );
   }

   /* Needed by FLIP macro */
   xmesa->xm_buffer->bottom = (int) winheight - 1;

   if (xmesa->xm_buffer->backimage) {
      /* Needed by PIXELADDR1 macro */
      xmesa->xm_buffer->ximage_width1
                  = xmesa->xm_buffer->backimage->bytes_per_line;
      xmesa->xm_buffer->ximage_origin1
                  = (GLubyte *) xmesa->xm_buffer->backimage->data
                    + xmesa->xm_buffer->ximage_width1 * (winheight-1);

      /* Needed by PIXELADDR2 macro */
      xmesa->xm_buffer->ximage_width2
                  = xmesa->xm_buffer->backimage->bytes_per_line / 2;
      xmesa->xm_buffer->ximage_origin2
                  = (GLushort *) xmesa->xm_buffer->backimage->data
                    + xmesa->xm_buffer->ximage_width2 * (winheight-1);

      /* Needed by PIXELADDR3 macro */
      xmesa->xm_buffer->ximage_width3
                  = xmesa->xm_buffer->backimage->bytes_per_line;
      xmesa->xm_buffer->ximage_origin3
                  = (GLubyte *) xmesa->xm_buffer->backimage->data
                    + xmesa->xm_buffer->ximage_width3 * (winheight-1);

      /* Needed by PIXELADDR4 macro */
      xmesa->xm_buffer->ximage_width4 = xmesa->xm_buffer->backimage->width;
      xmesa->xm_buffer->ximage_origin4
                  = (GLuint *) xmesa->xm_buffer->backimage->data
                    + xmesa->xm_buffer->ximage_width4 * (winheight-1);
   }
}


static void finish( GLcontext *ctx )
{
#ifdef XFree86Server
      /* NOT_NEEDED */
#else
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (xmesa) {
      _glthread_LOCK_MUTEX(_xmesa_lock);
      XSync( xmesa->display, False );
      _glthread_UNLOCK_MUTEX(_xmesa_lock);
   }
#endif
}


static void flush( GLcontext *ctx )
{
#ifdef XFree86Server
      /* NOT_NEEDED */
#else
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (xmesa) {
      _glthread_LOCK_MUTEX(_xmesa_lock);
      XFlush( xmesa->display );
      _glthread_UNLOCK_MUTEX(_xmesa_lock);
   }
#endif
}


#if 0
static GLboolean set_buffer( GLcontext *ctx, GLenum mode )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (mode == GL_FRONT_LEFT) {
      /* read/write front buffer */
      xmesa->xm_buffer->buffer = xmesa->xm_buffer->frontbuffer;
      ctx->NewState |= NEW_RASTER_OPS;
      gl_update_state(ctx);
      return GL_TRUE;
   }
   else if (mode==GL_BACK_LEFT && xmesa->xm_buffer->db_state) {
      /* read/write back buffer */
      if (xmesa->xm_buffer->backpixmap) {
         xmesa->xm_buffer->buffer =
	     (XMesaDrawable)xmesa->xm_buffer->backpixmap;
      }
      else if (xmesa->xm_buffer->backimage) {
         xmesa->xm_buffer->buffer = None;
      }
      else {
         /* just in case there wasn't enough memory for back buffer */
         xmesa->xm_buffer->buffer = xmesa->xm_buffer->frontbuffer;
      }
      ctx->NewState |= NEW_RASTER_OPS;
      gl_update_state(ctx);
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}
#endif


static GLboolean set_draw_buffer( GLcontext *ctx, GLenum mode )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (mode == GL_FRONT_LEFT) {
      /* write to front buffer */
      xmesa->xm_buffer->buffer = xmesa->xm_buffer->frontbuffer;
      update_span_funcs(ctx);
      return GL_TRUE;
   }
   else if (mode==GL_BACK_LEFT && xmesa->xm_buffer->db_state) {
      /* write to back buffer */
      if (xmesa->xm_buffer->backpixmap) {
         xmesa->xm_buffer->buffer =
	     (XMesaDrawable)xmesa->xm_buffer->backpixmap;
      }
      else if (xmesa->xm_buffer->backimage) {
         xmesa->xm_buffer->buffer = None;
      }
      else {
         /* just in case there wasn't enough memory for back buffer */
         xmesa->xm_buffer->buffer = xmesa->xm_buffer->frontbuffer;
      }
      update_span_funcs(ctx);
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}


static void set_read_buffer( GLcontext *ctx, GLframebuffer *buffer,
                             GLenum mode )
{
   XMesaBuffer target;
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;

   if (buffer == ctx->DrawBuffer) {
      target = xmesa->xm_buffer;
      xmesa->use_read_buffer = GL_FALSE;
   }
   else {
      ASSERT(buffer == ctx->ReadBuffer);
      target = xmesa->xm_read_buffer;
      xmesa->use_read_buffer = GL_TRUE;
   }

   if (mode == GL_FRONT_LEFT) {
      target->buffer = target->frontbuffer;
      update_span_funcs(ctx);
   }
   else if (mode==GL_BACK_LEFT && xmesa->xm_read_buffer->db_state) {
      if (target->backpixmap) {
         target->buffer = (XMesaDrawable)xmesa->xm_buffer->backpixmap;
      }
      else if (target->backimage) {
         target->buffer = None;
      }
      else {
         /* just in case there wasn't enough memory for back buffer */
         target->buffer = target->frontbuffer;
      }
      update_span_funcs(ctx);
   }
   else {
      gl_problem(ctx, "invalid buffer in set_read_buffer() in xmesa2.c");
   }
}



static void clear_index( GLcontext *ctx, GLuint index )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   xmesa->clearpixel = (unsigned long) index;
   XMesaSetForeground( xmesa->display, xmesa->xm_buffer->cleargc,
                       (unsigned long) index );
}


static void clear_color( GLcontext *ctx,
                         GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   xmesa->clearcolor[0] = r;
   xmesa->clearcolor[1] = g;
   xmesa->clearcolor[2] = b;
   xmesa->clearcolor[3] = a;
   xmesa->clearpixel = xmesa_color_to_pixel( xmesa, r, g, b, a,
                                             xmesa->xm_visual->undithered_pf );
   _glthread_LOCK_MUTEX(_xmesa_lock);
   XMesaSetForeground( xmesa->display, xmesa->xm_buffer->cleargc,
                       xmesa->clearpixel );
   _glthread_UNLOCK_MUTEX(_xmesa_lock);
}


static void clear_color_HPCR_ximage( GLcontext *ctx,
                         GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   int i;
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   xmesa->clearcolor[0] = r;
   xmesa->clearcolor[1] = g;
   xmesa->clearcolor[2] = b;
   xmesa->clearcolor[3] = a;

   if (r == 0 && g == 0 && b == 0) {
      /* black is black */
      MEMSET( xmesa->xm_visual->hpcr_clear_ximage_pattern, 0x0 ,
              sizeof(xmesa->xm_visual->hpcr_clear_ximage_pattern));
   }
   else {
      /* build clear pattern */
      for (i=0; i<16; i++) {
         xmesa->xm_visual->hpcr_clear_ximage_pattern[0][i]    =
            DITHER_HPCR(i, 0, r, g, b);
         xmesa->xm_visual->hpcr_clear_ximage_pattern[1][i]    =
            DITHER_HPCR(i, 1, r, g, b);
      }
   }
}


static void clear_color_HPCR_pixmap( GLcontext *ctx,
                             GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   int i;
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   xmesa->clearcolor[0] = r;
   xmesa->clearcolor[1] = g;
   xmesa->clearcolor[2] = b;
   xmesa->clearcolor[3] = a;

 
   if (0x0==r && 0x0==g && 0x0==b) {
      /* black is black */
      for (i=0; i<16; i++) {
         XMesaPutPixel(xmesa->xm_visual->hpcr_clear_ximage, i, 0, 0);
         XMesaPutPixel(xmesa->xm_visual->hpcr_clear_ximage, i, 1, 0);
      }
   }
   else {
      for (i=0; i<16; i++) {
         XMesaPutPixel(xmesa->xm_visual->hpcr_clear_ximage, i, 0, DITHER_HPCR(i, 0, r, g, b));
         XMesaPutPixel(xmesa->xm_visual->hpcr_clear_ximage, i, 1, DITHER_HPCR(i, 1, r, g, b));
      }
   }
   /* change tile pixmap content */
   XMesaPutImage(xmesa->display, 
		 (XMesaDrawable)xmesa->xm_visual->hpcr_clear_pixmap, 
		 xmesa->xm_buffer->cleargc, 
		 xmesa->xm_visual->hpcr_clear_ximage, 0, 0, 0, 0, 16, 2);
}

/* Set current color index */
static void set_index( GLcontext *ctx, GLuint index )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   unsigned long p = (unsigned long) index;
   xmesa->pixel = p;
   XMesaSetForeground( xmesa->display, xmesa->xm_buffer->gc1, p );
}


/* Set current drawing color */
static void set_color( GLcontext *ctx,
                       GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   xmesa->red   = r;
   xmesa->green = g;
   xmesa->blue  = b;
   xmesa->alpha = a;
   xmesa->pixel = xmesa_color_to_pixel( xmesa, r, g, b, a, xmesa->pixelformat );;
   XMesaSetForeground( xmesa->display, xmesa->xm_buffer->gc1, xmesa->pixel );
}



/* Set index mask ala glIndexMask */
static GLboolean index_mask( GLcontext *ctx, GLuint mask )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (xmesa->xm_buffer->buffer==XIMAGE) {
      return GL_FALSE;
   }
   else {
      unsigned long m;
      if (mask==0xffffffff) {
	 m = ((unsigned long)~0L);
      }
      else {
         m = (unsigned long) mask;
      }
      XMesaSetPlaneMask( xmesa->display, xmesa->xm_buffer->gc1, m );
      XMesaSetPlaneMask( xmesa->display, xmesa->xm_buffer->gc2, m );
      XMesaSetPlaneMask( xmesa->display, xmesa->xm_buffer->cleargc, m );
      return GL_TRUE;
   }
}


/* Implements glColorMask() */
static GLboolean color_mask( GLcontext *ctx,
                             GLboolean rmask, GLboolean gmask,
                             GLboolean bmask, GLboolean amask )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   int xclass = GET_VISUAL_CLASS(xmesa->xm_visual);
   (void) amask;

   if (xmesa->xm_buffer->buffer!=XIMAGE
       && (xclass==TrueColor || xclass==DirectColor)) {
      unsigned long m;
      if (rmask && gmask && bmask) {
         m = ((unsigned long)~0L);
      }
      else {
         m = 0;
         if (rmask)   m |= GET_REDMASK(xmesa->xm_visual);
         if (gmask)   m |= GET_GREENMASK(xmesa->xm_visual);
         if (bmask)   m |= GET_BLUEMASK(xmesa->xm_visual);
      }
      XMesaSetPlaneMask( xmesa->display, xmesa->xm_buffer->gc1, m );
      XMesaSetPlaneMask( xmesa->display, xmesa->xm_buffer->gc2, m );
      XMesaSetPlaneMask( xmesa->display, xmesa->xm_buffer->cleargc, m );
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}


/*
 * Set the pixel logic operation.  Return GL_TRUE if the device driver
 * can perform the operation, otherwise return GL_FALSE.  GL_COPY _must_
 * be operational, obviously.
 */
static GLboolean logicop( GLcontext *ctx, GLenum op )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   int func;
   if ((xmesa->xm_buffer->buffer==XIMAGE) && op!=GL_COPY) {
      /* X can't do logic ops in Ximages, except for GL_COPY */
      return GL_FALSE;
   }
   switch (op) {
      case GL_CLEAR:		func = GXclear;		break;
      case GL_SET:		func = GXset;		break;
      case GL_COPY:		func = GXcopy;		break;
      case GL_COPY_INVERTED:	func = GXcopyInverted;	break;
      case GL_NOOP:		func = GXnoop;		break;
      case GL_INVERT:		func = GXinvert;	break;
      case GL_AND:		func = GXand;		break;
      case GL_NAND:		func = GXnand;		break;
      case GL_OR:		func = GXor;		break;
      case GL_NOR:		func = GXnor;		break;
      case GL_XOR:		func = GXxor;		break;
      case GL_EQUIV:		func = GXequiv;		break;
      case GL_AND_REVERSE:	func = GXandReverse;	break;
      case GL_AND_INVERTED:	func = GXandInverted;	break;
      case GL_OR_REVERSE:	func = GXorReverse;	break;
      case GL_OR_INVERTED:	func = GXorInverted;	break;
      default:  return GL_FALSE;
   }
   _glthread_LOCK_MUTEX(_xmesa_lock);
   XMesaSetFunction( xmesa->display, xmesa->xm_buffer->gc1, func );
   XMesaSetFunction( xmesa->display, xmesa->xm_buffer->gc2, func );
   _glthread_UNLOCK_MUTEX(_xmesa_lock);
   return GL_TRUE;
}


/*
 * Enable/disable dithering
 */
static void dither( GLcontext *ctx, GLboolean enable )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (enable) {
      xmesa->pixelformat = xmesa->xm_visual->dithered_pf;
   }
   else {
      xmesa->pixelformat = xmesa->xm_visual->undithered_pf;
   }
}



/**********************************************************************/
/*** glClear implementations                                        ***/
/**********************************************************************/


static void
clear_front_pixmap( GLcontext *ctx, GLboolean all,
                    GLint x, GLint y, GLint width, GLint height )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (all) {
      XMesaFillRectangle( xmesa->display, xmesa->xm_buffer->frontbuffer,
                          xmesa->xm_buffer->cleargc,
                          0, 0,
                          xmesa->xm_buffer->width+1,
                          xmesa->xm_buffer->height+1 );
   }
   else {
      XMesaFillRectangle( xmesa->display, xmesa->xm_buffer->frontbuffer,
                          xmesa->xm_buffer->cleargc,
                          x, xmesa->xm_buffer->height - y - height,
                          width, height );
   }
}


static void
clear_back_pixmap( GLcontext *ctx, GLboolean all,
                   GLint x, GLint y, GLint width, GLint height )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (all) {
      XMesaFillRectangle( xmesa->display, xmesa->xm_buffer->backpixmap,
                          xmesa->xm_buffer->cleargc,
                          0, 0,
                          xmesa->xm_buffer->width+1,
                          xmesa->xm_buffer->height+1 );
   }
   else {
      XMesaFillRectangle( xmesa->display, xmesa->xm_buffer->backpixmap,
                          xmesa->xm_buffer->cleargc,
                          x, xmesa->xm_buffer->height - y - height,
                          width, height );
   }
}


static void
clear_8bit_ximage( GLcontext *ctx, GLboolean all,
                   GLint x, GLint y, GLint width, GLint height )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (all) {
      size_t n = xmesa->xm_buffer->backimage->bytes_per_line
         * xmesa->xm_buffer->backimage->height;
      MEMSET( xmesa->xm_buffer->backimage->data, xmesa->clearpixel, n );
   }
   else {
      GLint i;
      for (i=0;i<height;i++) {
         GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer, x, y+i );
         MEMSET( ptr, xmesa->clearpixel, width );
      }
   }
}


static void
clear_HPCR_ximage( GLcontext *ctx, GLboolean all,
                   GLint x, GLint y, GLint width, GLint height )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (all) {
      GLint i, c16 = (xmesa->xm_buffer->backimage->bytes_per_line>>4)<<4;
      GLubyte *ptr  = (GLubyte *)xmesa->xm_buffer->backimage->data;
      for (i=0; i<xmesa->xm_buffer->backimage->height; i++) {
         GLint j;
         GLubyte *sptr = xmesa->xm_visual->hpcr_clear_ximage_pattern[0];
         if (i&1) {
            sptr += 16;
         }
         for (j=0; j<c16; j+=16) {
            ptr[0] = sptr[0];
            ptr[1] = sptr[1];
            ptr[2] = sptr[2];
            ptr[3] = sptr[3];
            ptr[4] = sptr[4];
            ptr[5] = sptr[5];
            ptr[6] = sptr[6];
            ptr[7] = sptr[7];
            ptr[8] = sptr[8];
            ptr[9] = sptr[9];
            ptr[10] = sptr[10];
            ptr[11] = sptr[11];
            ptr[12] = sptr[12];
            ptr[13] = sptr[13];
            ptr[14] = sptr[14];
            ptr[15] = sptr[15];
            ptr += 16;
         }
         for (; j<xmesa->xm_buffer->backimage->bytes_per_line; j++) {
            *ptr = sptr[j&15];
            ptr++;
         }
      }
   }
   else {
      GLint i;
      for (i=y; i<y+height; i++) {
         GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer, x, i );
         int j;
         GLubyte *sptr = xmesa->xm_visual->hpcr_clear_ximage_pattern[0];
         if (i&1) {
            sptr += 16;
         }
         for (j=x; j<x+width; j++) {
            *ptr = sptr[j&15];
            ptr++;
         }
      }
   }
}


static void
clear_16bit_ximage( GLcontext *ctx, GLboolean all,
                    GLint x, GLint y, GLint width, GLint height )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint pixel = (GLuint) xmesa->clearpixel;
   if (xmesa->swapbytes) {
      pixel = ((pixel >> 8) & 0x00ff) | ((pixel << 8) & 0xff00);
   }
   if (all) {
      register GLuint n;
      register GLuint *ptr4 = (GLuint *) xmesa->xm_buffer->backimage->data;
      if ((pixel & 0xff) == ((pixel >> 8) & 0xff)) {
         /* low and high bytes are equal so use memset() */
         n = xmesa->xm_buffer->backimage->bytes_per_line
            * xmesa->xm_buffer->height;
         MEMSET( ptr4, pixel & 0xff, n );
      }
      else {
         pixel = pixel | (pixel<<16);
         n = xmesa->xm_buffer->backimage->bytes_per_line 
            * xmesa->xm_buffer->height / 4;
         do {
            *ptr4++ = pixel;
               n--;
         } while (n!=0);
         
         if ((xmesa->xm_buffer->backimage->bytes_per_line * 
              xmesa->xm_buffer->height) & 0x2)
            *(GLushort *)ptr4 = pixel & 0xffff;
      }
   }
   else {
      register int i, j;
      for (j=0;j<height;j++) {
         register GLushort *ptr2 = PIXELADDR2( xmesa->xm_buffer, x, y+j );
         for (i=0;i<width;i++) {
            *ptr2++ = pixel;
         }
      }
   }
}


/* Optimized code provided by Nozomi Ytow <noz@xfree86.org> */
static void
clear_24bit_ximage( GLcontext *ctx, GLboolean all,
                    GLint x, GLint y, GLint width, GLint height )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   const GLubyte r = xmesa->clearcolor[0];
   const GLubyte g = xmesa->clearcolor[1];
   const GLubyte b = xmesa->clearcolor[2];
   register GLuint clearPixel;
   if (xmesa->swapbytes) {
      clearPixel = (b << 16) | (g << 8) | r;
   }
   else {
      clearPixel = (r << 16) | (g << 8) | b;
   }

   if (all) {
      if (r==g && g==b) {
         /* same value for all three components (gray) */
         const GLint w3 = xmesa->xm_buffer->width * 3;
         const GLint h = xmesa->xm_buffer->height;
         GLint i;
         for (i = 0; i < h; i++) {
            bgr_t *ptr3 = PIXELADDR3(xmesa->xm_buffer, 0, i);
            MEMSET(ptr3, r, w3);
         }
      }
      else {
         /* the usual case */
         const GLint w = xmesa->xm_buffer->width;
         const GLint h = xmesa->xm_buffer->height;
         GLint i, j;
         for (i = 0; i < h; i++) {
            bgr_t *ptr3 = PIXELADDR3(xmesa->xm_buffer, 0, i);
            for (j = 0; j < w; j++) {
               ptr3->r = r;
               ptr3->g = g;
               ptr3->b = b;
               ptr3++;
            }
         }
#if 0 /* this code doesn't work for all window widths */
         register GLuint *ptr4 = (GLuint *) ptr3;
         register GLuint px;
         GLuint pixel4[3];
         register GLuint *p = pixel4;
         pixel4[0] = clearPixel | (clearPixel << 24);
         pixel4[1] = (clearPixel << 16) | (clearPixel >> 8);
         pixel4[2] = (clearPixel << 8) | (clearPixel >>  16);
         switch (3 & (int)(ptr3 - (bgr_t*) ptr4)){
            case 0:
               break;
            case 1:
               px = *ptr4 & 0x00ffffff;
               px |= pixel4[0] & 0xff000000;
               *ptr4++ = px;
               px = *ptr4 & 0xffff0000;
               px |= pixel4[2] & 0x0000ffff;
               *ptr4 = px;
               if (0 == --n)
                  break;
            case 2:
               px = *ptr4 & 0x0000fffff;
               px |= pixel4[1] & 0xffff0000;
               *ptr4++ = px;
               px = *ptr4 & 0xffffff00;
               px |= pixel4[2] & 0x000000ff;
               *ptr4 = px;
               if (0 == --n)
                  break;
            case 3:
               px = *ptr4 & 0x000000ff;
               px |= pixel4[2] & 0xffffff00;
               *ptr4++ = px;
               --n;
               break;
         }
         while (n > 3) {
            p = pixel4;
            *ptr4++ = *p++;
            *ptr4++ = *p++;
            *ptr4++ = *p++;
            n -= 4;
         }
         switch (n) {
            case 3:
               p = pixel4;
               *ptr4++ = *p++;
               *ptr4++ = *p++;
               px = *ptr4 & 0xffffff00;
               px |= clearPixel & 0xff;
               *ptr4 = px;
               break;
            case 2:
               p = pixel4;
               *ptr4++ = *p++;
               px = *ptr4 & 0xffff0000;
               px |= *p & 0xffff;
               *ptr4 = px;
               break;
            case 1:
               px = *ptr4 & 0xff000000;
               px |= *p & 0xffffff;
               *ptr4 = px;
               break;
            case 0:
               break;
         }
#endif
      }
   }
   else {
      /* only clear subrect of color buffer */
      if (r==g && g==b) {
         /* same value for all three components (gray) */
         GLint j;
         for (j=0;j<height;j++) {
            bgr_t *ptr3 = PIXELADDR3( xmesa->xm_buffer, x, y+j );
            MEMSET(ptr3, r, 3 * width);
         }
      }
      else {
         /* non-gray clear color */
         GLint i, j;
         for (j = 0; j < height; j++) {
            bgr_t *ptr3 = PIXELADDR3( xmesa->xm_buffer, x, y+j );
            for (i = 0; i < width; i++) {
               ptr3->r = r;
               ptr3->g = g;
               ptr3->b = b;
               ptr3++;
            }
         }
#if 0 /* this code might not always (seems ptr3 always == ptr4) */
         GLint j;
         GLuint pixel4[3];
         pixel4[0] = clearPixel | (clearPixel << 24);
         pixel4[1] = (clearPixel << 16) | (clearPixel >> 8);
         pixel4[2] = (clearPixel << 8) | (clearPixel >>  16);
         for (j=0;j<height;j++) {
            bgr_t *ptr3 = PIXELADDR3( xmesa->xm_buffer, x, y+j );
            register GLuint *ptr4 = (GLuint *)ptr3;
            register GLuint *p, px;
            GLuint w = width;
            switch (3 & (int)(ptr3 - (bgr_t*) ptr4)){
               case 0:
                  break;
               case 1:
                  px = *ptr4 & 0x00ffffff;
                  px |= pixel4[0] & 0xff000000;
                  *ptr4++ = px;
                  px = *ptr4 & 0xffff0000;
                  px |= pixel4[2] & 0x0000ffff;
                  *ptr4 = px;
                  if (0 == --w)
                     break;
               case 2:
                  px = *ptr4 & 0x0000fffff;
                  px |= pixel4[1] & 0xffff0000;
                  *ptr4++ = px;
                  px = *ptr4 & 0xffffff00;
                  px |= pixel4[2] & 0x000000ff;
                  *ptr4 = px;
                  if (0 == --w)
                     break;
               case 3:
                  px = *ptr4 & 0x000000ff;
                  px |= pixel4[2] & 0xffffff00;
                  *ptr4++ = px;
                  --w;
                  break;
            }
            while (w > 3){
               p = pixel4;
               *ptr4++ = *p++;
               *ptr4++ = *p++;
               *ptr4++ = *p++;
               w -= 4;
            }
            switch (w) {
               case 3:
                  p = pixel4;
                  *ptr4++ = *p++;
                  *ptr4++ = *p++;
                  px = *ptr4 & 0xffffff00;
                  px |= *p & 0xff;
                  *ptr4 = px;
                  break;
               case 2:
                  p = pixel4;
                  *ptr4++ = *p++;
                  px = *ptr4 & 0xffff0000;
                  px |= *p & 0xffff;
                  *ptr4 = px;
                  break;
               case 1:
                  px = *ptr4 & 0xff000000;
                  px |= pixel4[0] & 0xffffff;
                  *ptr4 = px;
                  break;
               case 0:
                  break;
            }
         }
#endif
      }
   }
}


static void
clear_32bit_ximage( GLcontext *ctx, GLboolean all,
                    GLint x, GLint y, GLint width, GLint height )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint pixel = (GLuint) xmesa->clearpixel;
   if (xmesa->swapbytes) {
      pixel = ((pixel >> 24) & 0x000000ff)
            | ((pixel >> 8)  & 0x0000ff00)
            | ((pixel << 8)  & 0x00ff0000)
            | ((pixel << 24) & 0xff000000);
   }
   if (all) {
      register GLint n = xmesa->xm_buffer->width * xmesa->xm_buffer->height;
      register GLuint *ptr4 = (GLuint *) xmesa->xm_buffer->backimage->data;
      if (pixel==0) {
         MEMSET( ptr4, pixel, 4*n );
      }
      else {
         do {
            *ptr4++ = pixel;
            n--;
         } while (n!=0);
      }
   }
   else {
      register int i, j;
      for (j=0;j<height;j++) {
         register GLuint *ptr4 = PIXELADDR4( xmesa->xm_buffer, x, y+j );
         for (i=0;i<width;i++) {
            *ptr4++ = pixel;
         }
      }
   }
}


static void
clear_nbit_ximage( GLcontext *ctx, GLboolean all,
                   GLint x, GLint y, GLint width, GLint height )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   if (all) {
      register int i, j;
      width = xmesa->xm_buffer->width;
      height = xmesa->xm_buffer->height;
      for (j=0;j<height;j++) {
         for (i=0;i<width;i++) {
            XMesaPutPixel( img, i, j, xmesa->clearpixel );
         }
      }
   }
   else {
      /* TODO: optimize this */
      register int i, j;
      y = FLIP(xmesa->xm_buffer, y);
      for (j=0;j<height;j++) {
         for (i=0;i<width;i++) {
            XMesaPutPixel( img, x+i, y-j, xmesa->clearpixel );
         }
      }
   }
}



static GLbitfield
clear_buffers( GLcontext *ctx, GLbitfield mask,
               GLboolean all, GLint x, GLint y, GLint width, GLint height )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   const GLuint *colorMask = (GLuint *) &ctx->Color.ColorMask;

   /* we can't handle color or index masking */
   if (mask & (DD_FRONT_LEFT_BIT | DD_BACK_LEFT_BIT)) {
      if (*colorMask != 0xffffffff || ctx->Color.IndexMask != 0xffffffff)
         return mask;
   }

   if (mask & DD_FRONT_LEFT_BIT) {
      ASSERT(xmesa->xm_buffer->front_clear_func);
      (*xmesa->xm_buffer->front_clear_func)( ctx, all, x, y, width, height );
   }
   if (mask & DD_BACK_LEFT_BIT) {
      ASSERT(xmesa->xm_buffer->back_clear_func);
      (*xmesa->xm_buffer->back_clear_func)( ctx, all, x, y, width, height );
   }
   return mask & (~(DD_FRONT_LEFT_BIT | DD_BACK_LEFT_BIT));
}



/*
 * The Mesa library needs to be able to draw pixels in a number of ways:
 *   1. RGB vs Color Index
 *   2. as horizontal spans (polygons, images) vs random locations (points,
 *      lines)
 *   3. different color per-pixel or same color for all pixels
 *
 * Furthermore, the X driver needs to support rendering to 3 possible
 * "buffers", usually one, but sometimes two at a time:
 *   1. The front buffer as an X window
 *   2. The back buffer as a Pixmap
 *   3. The back buffer as an XImage
 *
 * Finally, if the back buffer is an XImage, we can avoid using XPutPixel and
 * optimize common cases such as 24-bit and 8-bit modes.
 *
 * By multiplication, there's at least 48 possible combinations of the above.
 *
 * Below are implementations of the most commonly used combinations.  They are
 * accessed through function pointers which get initialized here and are used
 * directly from the Mesa library.  The 8 function pointers directly correspond
 * to the first 3 cases listed above.
 *
 *
 * The function naming convention is:
 *
 *   write_[span|pixels]_[mono]_[format]_[pixmap|ximage]
 *
 * New functions optimized for specific cases can be added without too much
 * trouble.  An example might be the 24-bit TrueColor mode 8A8R8G8B which is
 * found on IBM RS/6000 X servers.
 */




/**********************************************************************/
/*** Write COLOR SPAN functions                                     ***/
/**********************************************************************/


#define RGBA_SPAN_ARGS	const GLcontext *ctx,				\
			GLuint n, GLint x, GLint y,			\
			CONST GLubyte rgba[][4], const GLubyte mask[]

#define RGB_SPAN_ARGS	const GLcontext *ctx,				\
			GLuint n, GLint x, GLint y,			\
			CONST GLubyte rgb[][3], const GLubyte mask[]


/* NOTE: if mask==NULL, draw all pixels */


/*
 * Write a span of PF_TRUECOLOR pixels to a pixmap.
 */
static void write_span_TRUECOLOR_pixmap( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUECOLOR( p, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
            XMesaSetForeground( dpy, gc, p );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         unsigned long p;
         PACK_TRUECOLOR( p, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
         XMesaPutPixel( rowimg, i, 0, p );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_TRUECOLOR pixels to a pixmap.
 */
static void write_span_rgb_TRUECOLOR_pixmap( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUECOLOR( p, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
            XMesaSetForeground( dpy, gc, p );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         unsigned long p;
         PACK_TRUECOLOR( p, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
         XMesaPutPixel( rowimg, i, 0, p );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_TRUEDITHER pixels to a pixmap.
 */
static void write_span_TRUEDITHER_pixmap( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUEDITHER(p, x, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
            XMesaSetForeground( dpy, gc, p );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         unsigned long p;
         PACK_TRUEDITHER(p, x+i, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
         XMesaPutPixel( rowimg, i, 0, p );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_TRUEDITHER pixels to a pixmap (no alpha).
 */
static void write_span_rgb_TRUEDITHER_pixmap( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUEDITHER(p, x, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]);
            XMesaSetForeground( dpy, gc, p );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         unsigned long p;
         PACK_TRUEDITHER(p, x+i, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]);
         XMesaPutPixel( rowimg, i, 0, p );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}



/*
 * Write a span of PF_8A8B8G8R pixels to a pixmap.
 */
static void write_span_8A8B8G8R_pixmap( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc,
                         PACK_8A8B8G8R(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP], rgba[i][ACOMP]) );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLuint *ptr4 = (GLuint *) rowimg->data;
      for (i=0;i<n;i++) {
         *ptr4++ = PACK_8A8B8G8R( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP], rgba[i][ACOMP] );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_8A8B8G8R pixels to a pixmap (no alpha).
 */
static void write_span_rgb_8A8B8G8R_pixmap( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc,
                   PACK_8B8G8R(rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]) );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLuint *ptr4 = (GLuint *) rowimg->data;
      for (i=0;i<n;i++) {
         *ptr4++ = PACK_8B8G8R(rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]);
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_8R8G8B pixels to a pixmap.
 */
static void write_span_8R8G8B_pixmap( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc, PACK_8R8G8B( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ));
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLuint *ptr4 = (GLuint *) rowimg->data;
      for (i=0;i<n;i++) {
         *ptr4++ = PACK_8R8G8B( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_8R8G8B24 pixels to a pixmap.
 */
static void write_span_8R8G8B24_pixmap( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      register GLuint i;
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc,
               PACK_8R8G8B( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ));
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLuint *ptr4 = (GLuint *) rowimg->data;
      register GLuint pixel;
      static const GLuint shift[4] = {0, 8, 16, 24};
      register GLuint i = 0;
      int w = n;
      while (w > 3) {
         pixel  = rgba[i][BCOMP] /* << shift[0]*/;
         pixel |= rgba[i][GCOMP]    << shift[1];
         pixel |= rgba[i++][RCOMP]  << shift[2];
         pixel |= rgba[i][BCOMP]    << shift[3];
         *ptr4++ = pixel;

         pixel  = rgba[i][GCOMP] /* << shift[0]*/;
         pixel |= rgba[i++][RCOMP]  << shift[1];
         pixel |= rgba[i][BCOMP]    << shift[2];
         pixel |= rgba[i][GCOMP]    << shift[3];
         *ptr4++ = pixel;

         pixel  = rgba[i++][RCOMP]/* << shift[0]*/;
         pixel |= rgba[i][BCOMP]     << shift[1];
         pixel |= rgba[i][GCOMP]     << shift[2];
         pixel |= rgba[i++][RCOMP]   << shift[3];
         *ptr4++ = pixel;

         w -= 4;
      }
      switch (w) {
         case 3:
            pixel = 0;
            pixel |= rgba[i][BCOMP] /*<< shift[0]*/;
            pixel |= rgba[i][GCOMP]   << shift[1];
            pixel |= rgba[i++][RCOMP] << shift[2];
            pixel |= rgba[i][BCOMP]   << shift[3];
            *ptr4++ = pixel;
            pixel = 0;
            pixel |= rgba[i][GCOMP] /*<< shift[0]*/;
            pixel |= rgba[i++][RCOMP] << shift[1];
            pixel |= rgba[i][BCOMP]   << shift[2];
            pixel |= rgba[i][GCOMP]   << shift[3];
            *ptr4++ = pixel;
            pixel = 0xffffff00 & *ptr4;
            pixel |= rgba[i][RCOMP] /*<< shift[0]*/;
            *ptr4 = pixel;
            break;
         case 2:
            pixel = 0;
            pixel |= rgba[i][BCOMP] /*<< shift[0]*/;
            pixel |= rgba[i][GCOMP]   << shift[1];
            pixel |= rgba[i++][RCOMP] << shift[2];
            pixel |= rgba[i][BCOMP]   << shift[3];
            *ptr4++ = pixel;
            pixel = 0xffff0000 & *ptr4;
            pixel |= rgba[i][GCOMP] /*<< shift[0]*/;
            pixel |= rgba[i][RCOMP]   << shift[1];
            *ptr4 = pixel;
            break;
         case 1:
            pixel = 0xff000000 & *ptr4;
            pixel |= rgba[i][BCOMP] /*<< shift[0]*/;
            pixel |= rgba[i][GCOMP] << shift[1];
            pixel |= rgba[i][RCOMP] << shift[2];
            *ptr4 = pixel;
            break;
         case 0:
            break;
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_8R8G8B pixels to a pixmap (no alpha).
 */
static void write_span_rgb_8R8G8B_pixmap( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc, PACK_8R8G8B( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] ));
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLuint *ptr4 = (GLuint *) rowimg->data;
      for (i=0;i<n;i++) {
         *ptr4++ = PACK_8R8G8B( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_8R8G8B24 pixels to a pixmap (no alpha).
 */
static void write_span_rgb_8R8G8B24_pixmap( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      register GLuint i;
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc,
                  PACK_8R8G8B( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] ));
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLuint *ptr4 = (GLuint *) rowimg->data;
      register GLuint pixel;
      static const GLuint shift[4] = {0, 8, 16, 24};
      unsigned w = n;
      register GLuint i = 0;
      while (w > 3) {
         pixel = 0;
         pixel |= rgb[i][BCOMP]/* << shift[0]*/;
         pixel |= rgb[i][GCOMP] << shift[1];
         pixel |= rgb[i++][RCOMP] << shift[2];
         pixel |= rgb[i][BCOMP] <<shift[3];
         *ptr4++ = pixel;

         pixel = 0;
         pixel |= rgb[i][GCOMP]/* << shift[0]*/;
         pixel |= rgb[i++][RCOMP] << shift[1];
         pixel |= rgb[i][BCOMP] << shift[2];
         pixel |= rgb[i][GCOMP] << shift[3];
         *ptr4++ = pixel;

         pixel = 0;
         pixel |= rgb[i++][RCOMP]/* << shift[0]*/;
         pixel |= rgb[i][BCOMP] << shift[1];
         pixel |= rgb[i][GCOMP] << shift[2];
         pixel |= rgb[i++][RCOMP] << shift[3];
         *ptr4++ = pixel;
         w -= 4;
      }
      switch (w) {
         case 3:
            pixel = 0;
            pixel |= rgb[i][BCOMP]/* << shift[0]*/;
            pixel |= rgb[i][GCOMP] << shift[1];
            pixel |= rgb[i++][RCOMP] << shift[2];
            pixel |= rgb[i][BCOMP] << shift[3];
            *ptr4++ = pixel;
            pixel = 0;
            pixel |= rgb[i][GCOMP]/* << shift[0]*/;
            pixel |= rgb[i++][RCOMP] << shift[1];
            pixel |= rgb[i][BCOMP] << shift[2];
            pixel |= rgb[i][GCOMP] << shift[3];
            *ptr4++ = pixel;
            pixel = *ptr4;
            pixel &= 0xffffff00;
            pixel |= rgb[i++][RCOMP]/* << shift[0]*/;
            *ptr4++ = pixel;
            break;
         case 2:
            pixel = 0;
            pixel |= rgb[i][BCOMP]/* << shift[0]*/;
            pixel |= rgb[i][GCOMP] << shift[1];
            pixel |= rgb[i++][RCOMP] << shift[2];
            pixel |= rgb[i][BCOMP]  << shift[3];
            *ptr4++ = pixel;
            pixel = *ptr4;
            pixel &= 0xffff0000;
            pixel |= rgb[i][GCOMP]/* << shift[0]*/;
            pixel |= rgb[i++][RCOMP] << shift[1];
            *ptr4++ = pixel;
            break;
         case 1:
            pixel = *ptr4;
            pixel &= 0xff000000;
            pixel |= rgb[i][BCOMP]/* << shift[0]*/;
            pixel |= rgb[i][GCOMP] << shift[1];
            pixel |= rgb[i++][RCOMP] << shift[2];
            *ptr4++ = pixel;
            break;
         case 0:
            break;
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_5R6G5B pixels to a pixmap.
 */
static void write_span_5R6G5B_pixmap( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc, PACK_5R6G5B( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ));
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLushort *ptr2 = (GLushort *) rowimg->data;
      for (i=0;i<n;i++) {
         ptr2[i] = PACK_5R6G5B( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_DITHER_5R6G5B pixels to a pixmap.
 */
static void write_span_DITHER_5R6G5B_pixmap( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUEDITHER(p, x, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
            XMesaSetForeground( dpy, gc, p );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLushort *ptr2 = (GLushort *) rowimg->data;
      for (i=0;i<n;i++) {
         PACK_TRUEDITHER( ptr2[i], x+i, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_5R6G5B pixels to a pixmap (no alpha).
 */
static void write_span_rgb_5R6G5B_pixmap( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc, PACK_5R6G5B( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] ));
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLushort *ptr2 = (GLushort *) rowimg->data;
      for (i=0;i<n;i++) {
         ptr2[i] = PACK_5R6G5B( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_DITHER_5R6G5B pixels to a pixmap (no alpha).
 */
static void write_span_rgb_DITHER_5R6G5B_pixmap( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUEDITHER(p, x, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]);
            XMesaSetForeground( dpy, gc, p );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLushort *ptr2 = (GLushort *) rowimg->data;
      for (i=0;i<n;i++) {
         PACK_TRUEDITHER( ptr2[i], x+i, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}



/*
 * Write a span of PF_DITHER pixels to a pixmap.
 */
static void write_span_DITHER_pixmap( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   XDITHER_SETUP(y);
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc, XDITHER(x, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]) );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         XMesaPutPixel( rowimg, i, 0, XDITHER(x+i, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]) );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_DITHER pixels to a pixmap (no alpha).
 */
static void write_span_rgb_DITHER_pixmap( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   XDITHER_SETUP(y);
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc, XDITHER(x, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]) );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         XMesaPutPixel( rowimg, i, 0, XDITHER(x+i, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]) );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_1BIT pixels to a pixmap.
 */
static void write_span_1BIT_pixmap( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   SETUP_1BIT;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc,
                            DITHER_1BIT( x, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         XMesaPutPixel( rowimg, i, 0,
                    DITHER_1BIT( x+i, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_1BIT pixels to a pixmap (no alpha).
 */
static void write_span_rgb_1BIT_pixmap( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   SETUP_1BIT;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc,
              DITHER_1BIT(x, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]) );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         XMesaPutPixel( rowimg, i, 0,
          DITHER_1BIT(x+i, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]) );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_HPCR pixels to a pixmap.
 */
static void write_span_HPCR_pixmap( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc,
                            DITHER_HPCR( x, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLubyte *ptr = (GLubyte *) xmesa->xm_buffer->rowimage->data;
      for (i=0;i<n;i++) {
         ptr[i] = DITHER_HPCR( (x+i), y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_HPCR pixels to a pixmap (no alpha).
 */
static void write_span_rgb_HPCR_pixmap( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc,
              DITHER_HPCR(x, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]) );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLubyte *ptr = (GLubyte *) xmesa->xm_buffer->rowimage->data;
      for (i=0;i<n;i++) {
         ptr[i] = DITHER_HPCR( (x+i), y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_LOOKUP pixels to a pixmap.
 */
static void write_span_LOOKUP_pixmap( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   LOOKUP_SETUP;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc, LOOKUP( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         XMesaPutPixel( rowimg, i, 0, LOOKUP(rgba[i][RCOMP],rgba[i][GCOMP],rgba[i][BCOMP]) );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_LOOKUP pixels to a pixmap (no alpha).
 */
static void write_span_rgb_LOOKUP_pixmap( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   LOOKUP_SETUP;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc, LOOKUP( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] ) );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         XMesaPutPixel( rowimg, i, 0, LOOKUP(rgb[i][RCOMP],rgb[i][GCOMP],rgb[i][BCOMP]) );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}



/*
 * Write a span of PF_GRAYSCALE pixels to a pixmap.
 */
static void write_span_GRAYSCALE_pixmap( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc, GRAY_RGB( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         XMesaPutPixel( rowimg, i, 0, GRAY_RGB(rgba[i][RCOMP],rgba[i][GCOMP],rgba[i][BCOMP]) );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_GRAYSCALE pixels to a pixmap (no alpha).
 */
static void write_span_rgb_GRAYSCALE_pixmap( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc, GRAY_RGB( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] ) );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      XMesaImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         XMesaPutPixel( rowimg, i, 0, GRAY_RGB(rgb[i][RCOMP],rgb[i][GCOMP],rgb[i][BCOMP]) );
      }
      XMesaPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_TRUECOLOR pixels to an XImage.
 */
static void write_span_TRUECOLOR_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUECOLOR( p, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
            XMesaPutPixel( img, x, y, p );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         unsigned long p;
         PACK_TRUECOLOR( p, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
         XMesaPutPixel( img, x, y, p );
      }
   }
}


/*
 * Write a span of PF_TRUECOLOR pixels to an XImage (no alpha).
 */
static void write_span_rgb_TRUECOLOR_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUECOLOR( p, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
            XMesaPutPixel( img, x, y, p );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         unsigned long p;
         PACK_TRUECOLOR( p, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
         XMesaPutPixel( img, x, y, p );
      }
   }
}


/*
 * Write a span of PF_TRUEDITHER pixels to an XImage.
 */
static void write_span_TRUEDITHER_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUEDITHER(p, x, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
            XMesaPutPixel( img, x, y, p );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         unsigned long p;
         PACK_TRUEDITHER(p, x, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
         XMesaPutPixel( img, x, y, p );
      }
   }
}


/*
 * Write a span of PF_TRUEDITHER pixels to an XImage (no alpha).
 */
static void write_span_rgb_TRUEDITHER_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUEDITHER(p, x, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]);
            XMesaPutPixel( img, x, y, p );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         unsigned long p;
         PACK_TRUEDITHER(p, x, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]);
         XMesaPutPixel( img, x, y, p );
      }
   }
}


/*
 * Write a span of PF_8A8B8G8R-format pixels to an ximage.
 */
static void write_span_8A8B8G8R_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLuint *ptr = PIXELADDR4( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            ptr[i] = PACK_8A8B8G8R( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP], rgba[i][ACOMP] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++) {
         ptr[i] = PACK_8A8B8G8R( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP], rgba[i][ACOMP] );
      }
   }
}


/*
 * Write a span of PF_8A8B8G8R-format pixels to an ximage (no alpha).
 */
static void write_span_rgb_8A8B8G8R_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLuint *ptr = PIXELADDR4( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            ptr[i] = PACK_8A8B8G8R( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP], 255 );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++) {
         ptr[i] = PACK_8A8B8G8R( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP], 255 );
      }
   }
}


/*
 * Write a span of PF_8R8G8B-format pixels to an ximage.
 */
static void write_span_8R8G8B_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLuint *ptr = PIXELADDR4( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            ptr[i] = PACK_8R8G8B(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
         }
      }
   }
   else {
      for (i=0;i<n;i++) {
         ptr[i] = PACK_8R8G8B(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
      }
   }
}


/*
 * Write a span of PF_8R8G8B24-format pixels to an ximage.
 */
static void write_span_8R8G8B24_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = (GLubyte *) PIXELADDR3( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            GLuint *ptr4 = (GLuint *) ptr;
            register GLuint pixel = *ptr4;
            switch (3 & (int)(ptr - (GLubyte*)ptr4)) {
               case 0:
                  pixel &= 0xff000000;
                  pixel |= rgba[i][BCOMP];
                  pixel |= rgba[i][GCOMP] << 8;
                  pixel |= rgba[i][RCOMP] << 16;
                  *ptr4 = pixel;
                  break;
               case 3:
                  pixel &= 0x00ffffff;
                  pixel |= rgba[i][BCOMP] << 24;
                  *ptr4++ = pixel;
                  pixel = *ptr4 & 0xffff0000;
                  pixel |= rgba[i][GCOMP];
                  pixel |= rgba[i][RCOMP] << 8;
                  *ptr4 = pixel;
                  break;
               case 2:
                  pixel &= 0x0000ffff;
                  pixel |= rgba[i][BCOMP] << 16;
                  pixel |= rgba[i][GCOMP] << 24;
                  *ptr4++ = pixel;
                  pixel = *ptr4 & 0xffffff00;
                  pixel |= rgba[i][RCOMP];
                  *ptr4 = pixel;
                  break;
               case 1:
                  pixel &= 0x000000ff;
                  pixel |= rgba[i][BCOMP] << 8;
                  pixel |= rgba[i][GCOMP] << 16;
                  pixel |= rgba[i][RCOMP] << 24;
                  *ptr4 = pixel;
                  break;
            }
         }
	 ptr += 3;
      }
   }
   else {
      /* write all pixels */
      int w = n;
      GLuint *ptr4 = (GLuint *) ptr;
      register GLuint pixel = *ptr4;
      int index = (int)(ptr - (GLubyte *)ptr4);
      register GLuint i = 0;
      switch (index) {
         case 0:
            break;
         case 1:
            pixel &= 0x00ffffff;
            pixel |= rgba[i][BCOMP] << 24;
            *ptr4++ = pixel;
            pixel = *ptr4 & 0xffff0000;
            pixel |= rgba[i][GCOMP];
            pixel |= rgba[i++][RCOMP] << 8;
            *ptr4 = pixel;
            if (0 == --w)
               break;
         case 2:
            pixel &= 0x0000ffff;
            pixel |= rgba[i][BCOMP] << 16;
            pixel |= rgba[i][GCOMP] << 24;
            *ptr4++ = pixel;
            pixel = *ptr4 & 0xffffff00;
            pixel |= rgba[i++][RCOMP];
            *ptr4 = pixel;
            if (0 == --w)
               break;
         case 3:
            pixel &= 0x000000ff;
            pixel |= rgba[i][BCOMP] << 8;
            pixel |= rgba[i][GCOMP] << 16;
            pixel |= rgba[i++][RCOMP] << 24;
            *ptr4++ = pixel;
            if (0 == --w)
               break;
            break;
      }
      while (w > 3) {
         pixel = rgba[i][BCOMP];
         pixel |= rgba[i][GCOMP] << 8;
         pixel |= rgba[i++][RCOMP] << 16;
         pixel |= rgba[i][BCOMP] << 24;
         *ptr4++ = pixel;
         pixel = rgba[i][GCOMP];
         pixel |= rgba[i++][RCOMP] << 8;
         pixel |= rgba[i][BCOMP] << 16;
         pixel |= rgba[i][GCOMP] << 24;
         *ptr4++ = pixel;
         pixel = rgba[i++][RCOMP];
         pixel |= rgba[i][BCOMP] << 8;
         pixel |= rgba[i][GCOMP] << 16;
         pixel |= rgba[i++][RCOMP] << 24;
         *ptr4++ = pixel;
         w -= 4;
      }
      switch (w) {
         case 0:
            break;
         case 1:
            pixel = *ptr4 & 0xff000000;
            pixel |= rgba[i][BCOMP];
            pixel |= rgba[i][GCOMP] << 8;
            pixel |= rgba[i][RCOMP] << 16;
            *ptr4 = pixel;
            break;
         case 2:
            pixel = rgba[i][BCOMP];
            pixel |= rgba[i][GCOMP] << 8;
            pixel |= rgba[i++][RCOMP] << 16;
            pixel |= rgba[i][BCOMP] << 24;
            *ptr4++ = pixel;
            pixel = *ptr4 & 0xffff0000;
            pixel |= rgba[i][GCOMP];
            pixel |= rgba[i][RCOMP] << 8;
            *ptr4 = pixel;
            break;
         case 3:
            pixel = rgba[i][BCOMP];
            pixel |= rgba[i][GCOMP] << 8;
            pixel |= rgba[i++][RCOMP] << 16;
            pixel |= rgba[i][BCOMP] << 24;
            *ptr4++ = pixel;
            pixel = rgba[i][GCOMP];
            pixel |= rgba[i++][RCOMP] << 8;
            pixel |= rgba[i][BCOMP] << 16;
            pixel |= rgba[i][GCOMP] << 24;
            *ptr4++ = pixel;
            pixel = *ptr4 & 0xffffff00;
            pixel |= rgba[i][RCOMP];
            *ptr4 = pixel;
            break;
      }
   }
}


/*
 * Write a span of PF_8R8G8B-format pixels to an ximage (no alpha).
 */
static void write_span_rgb_8R8G8B_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLuint *ptr = PIXELADDR4( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            ptr[i] = PACK_8R8G8B(rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]);
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++) {
         ptr[i] = PACK_8R8G8B(rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]);
      }
   }
}


/*
 * Write a span of PF_8R8G8B24-format pixels to an ximage (no alpha).
 */
static void write_span_rgb_8R8G8B24_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = (GLubyte *) PIXELADDR3( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            *ptr++ = rgb[i][BCOMP];
            *ptr++ = rgb[i][GCOMP];
            *ptr++ = rgb[i][RCOMP];
         }
         else {
            ptr += 3;
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++) {
         *ptr++ = rgb[i][BCOMP];
         *ptr++ = rgb[i][GCOMP];
         *ptr++ = rgb[i][RCOMP];
      }
   }
}


/*
 * Write a span of PF_5R6G5B-format pixels to an ximage.
 */
static void write_span_5R6G5B_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLushort *ptr = PIXELADDR2( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            ptr[i] = PACK_5R6G5B( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
         }
      }
   }
   else {
      /* draw all pixels */
#if defined(__i386__) /* word stores don't have to be on 4-byte boundaries */
      GLuint *ptr32 = (GLuint *) ptr;
      GLuint extraPixel = (n & 1);
      n -= extraPixel;
      for (i = 0; i < n; i += 2) {
         GLuint p0, p1;
         p0 = PACK_5R6G5B(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
         p1 = PACK_5R6G5B(rgba[i+1][RCOMP], rgba[i+1][GCOMP], rgba[i+1][BCOMP]);
         *ptr32++ = (p1 << 16) | p0;
      }
      if (extraPixel) {
         ptr[n] = PACK_5R6G5B(rgba[n][RCOMP], rgba[n][GCOMP], rgba[n][BCOMP]);
      }
#else
      for (i = 0; i < n; i++) {
         ptr[i] = PACK_5R6G5B(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
      }
#endif
   }
}


/*
 * Write a span of PF_DITHER_5R6G5B-format pixels to an ximage.
 */
static void write_span_DITHER_5R6G5B_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLushort *ptr = PIXELADDR2( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            PACK_TRUEDITHER( ptr[i], x, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
         }
      }
   }
   else {
      /* draw all pixels */
#if defined(__i386__) /* word stores don't have to be on 4-byte boundaries */
      GLuint *ptr32 = (GLuint *) ptr;
      GLuint extraPixel = (n & 1);
      n -= extraPixel;
      for (i = 0; i < n; i += 2, x += 2) {
         GLuint p0, p1;
         PACK_TRUEDITHER( p0, x, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
         PACK_TRUEDITHER( p1, x+1, y, rgba[i+1][RCOMP], rgba[i+1][GCOMP], rgba[i+1][BCOMP] );
         *ptr32++ = (p1 << 16) | p0;
      }
      if (extraPixel) {
         PACK_TRUEDITHER( ptr[n], x+n, y, rgba[n][RCOMP], rgba[n][GCOMP], rgba[n][BCOMP]);
      }
#else
      for (i = 0; i < n; i++, x++) {
         PACK_TRUEDITHER( ptr[i], x, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
      }
#endif
   }
}


/*
 * Write a span of PF_5R6G5B-format pixels to an ximage (no alpha).
 */
static void write_span_rgb_5R6G5B_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLushort *ptr = PIXELADDR2( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            ptr[i] = PACK_5R6G5B( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
         }
      }
   }
   else {
      /* draw all pixels */
#if defined(__i386__) /* word stores don't have to be on 4-byte boundaries */
      GLuint *ptr32 = (GLuint *) ptr;
      GLuint extraPixel = (n & 1);
      n -= extraPixel;
      for (i = 0; i < n; i += 2) {
         GLuint p0, p1;
         p0 = PACK_5R6G5B(rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]);
         p1 = PACK_5R6G5B(rgb[i+1][RCOMP], rgb[i+1][GCOMP], rgb[i+1][BCOMP]);
         *ptr32++ = (p1 << 16) | p0;
      }
      if (extraPixel) {
         ptr[n] = PACK_5R6G5B(rgb[n][RCOMP], rgb[n][GCOMP], rgb[n][BCOMP]);
      }
#else
      for (i=0;i<n;i++) {
         ptr[i] = PACK_5R6G5B( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
      }
#endif
   }
}


/*
 * Write a span of PF_DITHER_5R6G5B-format pixels to an ximage (no alpha).
 */
static void write_span_rgb_DITHER_5R6G5B_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLushort *ptr = PIXELADDR2( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            PACK_TRUEDITHER( ptr[i], x, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
         }
      }
   }
   else {
      /* draw all pixels */
#if defined(__i386__) /* word stores don't have to be on 4-byte boundaries */
      GLuint *ptr32 = (GLuint *) ptr;
      GLuint extraPixel = (n & 1);
      n -= extraPixel;
      for (i = 0; i < n; i += 2, x += 2) {
         GLuint p0, p1;
         PACK_TRUEDITHER( p0, x, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
         PACK_TRUEDITHER( p1, x+1, y, rgb[i+1][RCOMP], rgb[i+1][GCOMP], rgb[i+1][BCOMP] );
         *ptr32++ = (p1 << 16) | p0;
      }
      if (extraPixel) {
         PACK_TRUEDITHER( ptr[n], x+n, y, rgb[n][RCOMP], rgb[n][GCOMP], rgb[n][BCOMP]);
      }
#else
      for (i=0;i<n;i++,x++) {
         PACK_TRUEDITHER( ptr[i], x, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
      }
#endif
   }
}


/*
 * Write a span of PF_DITHER pixels to an XImage.
 */
static void write_span_DITHER_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   int yy = FLIP(xmesa->xm_buffer, y);
   XDITHER_SETUP(yy);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaPutPixel( img, x, yy, XDITHER( x, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         XMesaPutPixel( img, x, yy, XDITHER( x, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
      }
   }
}


/*
 * Write a span of PF_DITHER pixels to an XImage (no alpha).
 */
static void write_span_rgb_DITHER_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   int yy = FLIP(xmesa->xm_buffer, y);
   XDITHER_SETUP(yy);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaPutPixel( img, x, yy, XDITHER( x, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] ) );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         XMesaPutPixel( img, x, yy, XDITHER( x, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] ) );
      }
   }
}



/*
 * Write a span of 8-bit PF_DITHER pixels to an XImage.
 */
static void write_span_DITHER8_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer, x, y );
   XDITHER_SETUP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            ptr[i] = (GLubyte) XDITHER( x, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
         }
      }
   }
   else {
      for (i=0;i<n;i++,x++) {
         ptr[i] = (GLubyte) XDITHER( x, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
   }
}


static void write_span_rgb_DITHER8_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer, x, y );
   XDITHER_SETUP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            ptr[i] = (GLubyte) XDITHER( x, rgb[i][0], rgb[i][1], rgb[i][2] );
         }
      }
   }
   else {
      const GLubyte *data = (GLubyte *) rgb;
      for (i=0;i<n;i++,x++) {
         /*ptr[i] = XDITHER( x, rgb[i][0], rgb[i][1], rgb[i][2] );*/
         ptr[i] = (GLubyte) XDITHER( x, data[i+i+i], data[i+i+i+1], data[i+i+i+2] );
      }
   }
}



/*
 * Write a span of PF_1BIT pixels to an XImage.
 */
static void write_span_1BIT_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   SETUP_1BIT;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaPutPixel(img, x, y, DITHER_1BIT(x, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]));
         }
      }
   }
   else {
      for (i=0;i<n;i++,x++) {
         XMesaPutPixel( img, x, y, DITHER_1BIT(x, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]) );
      }
   }
}


/*
 * Write a span of PF_1BIT pixels to an XImage (no alpha).
 */
static void write_span_rgb_1BIT_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   SETUP_1BIT;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaPutPixel(img, x, y, DITHER_1BIT(x, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]));
         }
      }
   }
   else {
      for (i=0;i<n;i++,x++) {
         XMesaPutPixel( img, x, y, DITHER_1BIT(x, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]) );
      }
   }
}


/*
 * Write a span of PF_HPCR pixels to an XImage.
 */
static void write_span_HPCR_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            ptr[i] = DITHER_HPCR( x, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         ptr[i] = DITHER_HPCR( x, y, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
   }
}


/*
 * Write a span of PF_HPCR pixels to an XImage (no alpha).
 */
static void write_span_rgb_HPCR_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            ptr[i] = DITHER_HPCR( x, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         ptr[i] = DITHER_HPCR( x, y, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
      }
   }
}


/*
 * Write a span of PF_LOOKUP pixels to an XImage.
 */
static void write_span_LOOKUP_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   LOOKUP_SETUP;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaPutPixel( img, x, y, LOOKUP( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         XMesaPutPixel( img, x, y, LOOKUP( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
      }
   }
}


/*
 * Write a span of PF_LOOKUP pixels to an XImage (no alpha).
 */
static void write_span_rgb_LOOKUP_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   LOOKUP_SETUP;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaPutPixel( img, x, y, LOOKUP( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] ) );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         XMesaPutPixel( img, x, y, LOOKUP( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] ) );
      }
   }
}


/*
 * Write a span of 8-bit PF_LOOKUP pixels to an XImage.
 */
static void write_span_LOOKUP8_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer, x, y );
   LOOKUP_SETUP;
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            ptr[i] = (GLubyte) LOOKUP( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         ptr[i] = (GLubyte) LOOKUP( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
   }
}


static void write_rgb_LOOKUP8_ximage( const GLcontext *ctx,
                                      GLuint n, GLint x, GLint y,
                                      CONST GLubyte rgb[][3],
                                      const GLubyte mask[] )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer, x, y );
   LOOKUP_SETUP;
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            ptr[i] = (GLubyte) LOOKUP( rgb[i][0], rgb[i][1], rgb[i][2] );
         }
      }
   }
   else {
      /* draw all pixels */
      const GLubyte *data = (GLubyte *) rgb;
      for (i=0;i<n;i++,x++) {
         /*ptr[i] = LOOKUP( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );*/
         ptr[i] = (GLubyte) LOOKUP( data[i+i+i], data[i+i+i+1], data[i+i+i+2] );
      }
   }
}


/*
 * Write a span of PF_GRAYSCALE pixels to an XImage.
 */
static void write_span_GRAYSCALE_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaPutPixel( img, x, y, GRAY_RGB( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         XMesaPutPixel( img, x, y, GRAY_RGB( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
      }
   }
}


/*
 * Write a span of PF_GRAYSCALE pixels to an XImage (no alpha).
 */
static void write_span_rgb_GRAYSCALE_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaPutPixel( img, x, y, GRAY_RGB( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] ) );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         XMesaPutPixel( img, x, y, GRAY_RGB( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] ) );
      }
   }
}


/*
 * Write a span of 8-bit PF_GRAYSCALE pixels to an XImage.
 */
static void write_span_GRAYSCALE8_ximage( RGBA_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            ptr[i] = (GLubyte) GRAY_RGB( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++) {
         ptr[i] = (GLubyte) GRAY_RGB( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
   }
}


/*
 * Write a span of 8-bit PF_GRAYSCALE pixels to an XImage (no alpha).
 */
static void write_span_rgb_GRAYSCALE8_ximage( RGB_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer, x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            ptr[i] = (GLubyte) GRAY_RGB( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++) {
         ptr[i] = (GLubyte) GRAY_RGB( rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
      }
   }
}




/**********************************************************************/
/*** Write COLOR PIXEL functions                                    ***/
/**********************************************************************/


#define RGBA_PIXEL_ARGS   const GLcontext *ctx,				\
			  GLuint n, const GLint x[], const GLint y[],	\
			  CONST GLubyte rgba[][4], const GLubyte mask[]


/*
 * Write an array of PF_TRUECOLOR pixels to a pixmap.
 */
static void write_pixels_TRUECOLOR_pixmap( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUECOLOR( p, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
	 XMesaSetForeground( dpy, gc, p );
	 XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_TRUEDITHER pixels to a pixmap.
 */
static void write_pixels_TRUEDITHER_pixmap( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUEDITHER(p, x[i], y[i], rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
         XMesaSetForeground( dpy, gc, p );
         XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_8A8B8G8R pixels to a pixmap.
 */
static void write_pixels_8A8B8G8R_pixmap( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaSetForeground( dpy, gc,
                         PACK_8A8B8G8R( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP], rgba[i][ACOMP] ));
	 XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_8R8G8B pixels to a pixmap.
 */
static void write_pixels_8R8G8B_pixmap( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaSetForeground( dpy, gc, PACK_8R8G8B( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
	 XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_8R8G8B24 pixels to a pixmap.
 */
static void write_pixels_8R8G8B24_pixmap( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaSetForeground( dpy, gc, PACK_8R8G8B( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
	 XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_5R6G5B pixels to a pixmap.
 */
static void write_pixels_5R6G5B_pixmap( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaSetForeground( dpy, gc, PACK_5R6G5B( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
	 XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_DITHER_5R6G5B pixels to a pixmap.
 */
static void write_pixels_DITHER_5R6G5B_pixmap( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUEDITHER(p, x[i], y[i], rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
	 XMesaSetForeground( dpy, gc, p );
	 XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_DITHER pixels to a pixmap.
 */
static void write_pixels_DITHER_pixmap( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   DITHER_SETUP;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaSetForeground( dpy, gc,
                         DITHER(x[i], y[i], rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]) );
	 XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_1BIT pixels to a pixmap.
 */
static void write_pixels_1BIT_pixmap( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   SETUP_1BIT;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaSetForeground( dpy, gc,
                         DITHER_1BIT( x[i], y[i], rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ));
	 XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_HPCR pixels to a pixmap.
 */
static void write_pixels_HPCR_pixmap( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         XMesaSetForeground( dpy, gc,
                         DITHER_HPCR( x[i], y[i], rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ));
         XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_LOOKUP pixels to a pixmap.
 */
static void write_pixels_LOOKUP_pixmap( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   LOOKUP_SETUP;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         XMesaSetForeground( dpy, gc, LOOKUP( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
         XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_GRAYSCALE pixels to a pixmap.
 */
static void write_pixels_GRAYSCALE_pixmap( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         XMesaSetForeground( dpy, gc, GRAY_RGB( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
         XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_TRUECOLOR pixels to an ximage.
 */
static void write_pixels_TRUECOLOR_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUECOLOR( p, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
	 XMesaPutPixel( img, x[i], FLIP(xmesa->xm_buffer, y[i]), p );
      }
   }
}


/*
 * Write an array of PF_TRUEDITHER pixels to an XImage.
 */
static void write_pixels_TRUEDITHER_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUEDITHER(p, x[i], FLIP(xmesa->xm_buffer, y[i]), rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
	 XMesaPutPixel( img, x[i], FLIP(xmesa->xm_buffer, y[i]), p );
      }
   }
}


/*
 * Write an array of PF_8A8B8G8R pixels to an ximage.
 */
static void write_pixels_8A8B8G8R_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLuint *ptr = PIXELADDR4( xmesa->xm_buffer, x[i], y[i] );
         *ptr = PACK_8A8B8G8R( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP], rgba[i][ACOMP] );
      }
   }
}


/*
 * Write an array of PF_8R8G8B pixels to an ximage.
 */
static void write_pixels_8R8G8B_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLuint *ptr = PIXELADDR4( xmesa->xm_buffer, x[i], y[i] );
         *ptr = PACK_8R8G8B( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
   }
}


/*
 * Write an array of PF_8R8G8B24 pixels to an ximage.
 */
static void write_pixels_8R8G8B24_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 bgr_t *ptr = PIXELADDR3( xmesa->xm_buffer, x[i], y[i] );
         ptr->r = rgba[i][RCOMP];
         ptr->g = rgba[i][GCOMP];
         ptr->b = rgba[i][BCOMP];
      }
   }
}


/*
 * Write an array of PF_5R6G5B pixels to an ximage.
 */
static void write_pixels_5R6G5B_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLushort *ptr = PIXELADDR2( xmesa->xm_buffer, x[i], y[i] );
         *ptr = PACK_5R6G5B( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
   }
}


/*
 * Write an array of PF_DITHER_5R6G5B pixels to an ximage.
 */
static void write_pixels_DITHER_5R6G5B_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLushort *ptr = PIXELADDR2( xmesa->xm_buffer, x[i], y[i] );
         PACK_TRUEDITHER( *ptr, x[i], y[i], rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
   }
}


/*
 * Write an array of PF_DITHER pixels to an XImage.
 */
static void write_pixels_DITHER_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   DITHER_SETUP;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaPutPixel( img, x[i], FLIP(xmesa->xm_buffer, y[i]),
                    DITHER( x[i], y[i], rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
      }
   }
}


/*
 * Write an array of 8-bit PF_DITHER pixels to an XImage.
 */
static void write_pixels_DITHER8_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   DITHER_SETUP;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer,x[i],y[i]);
	 *ptr = (GLubyte) DITHER( x[i], y[i], rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
   }
}


/*
 * Write an array of PF_1BIT pixels to an XImage.
 */
static void write_pixels_1BIT_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   SETUP_1BIT;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaPutPixel( img, x[i], FLIP(xmesa->xm_buffer, y[i]),
                    DITHER_1BIT( x[i], y[i], rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ));
      }
   }
}


/*
 * Write an array of PF_HPCR pixels to an XImage.
 */
static void write_pixels_HPCR_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer,x[i],y[i]);
         *ptr = (GLubyte) DITHER_HPCR( x[i], y[i], rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
   }
}


/*
 * Write an array of PF_LOOKUP pixels to an XImage.
 */
static void write_pixels_LOOKUP_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   LOOKUP_SETUP;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaPutPixel( img, x[i], FLIP(xmesa->xm_buffer, y[i]), LOOKUP(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]) );
      }
   }
}


/*
 * Write an array of 8-bit PF_LOOKUP pixels to an XImage.
 */
static void write_pixels_LOOKUP8_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   LOOKUP_SETUP;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer,x[i],y[i]);
	 *ptr = (GLubyte) LOOKUP( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
   }
}


/*
 * Write an array of PF_GRAYSCALE pixels to an XImage.
 */
static void write_pixels_GRAYSCALE_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaPutPixel( img, x[i], FLIP(xmesa->xm_buffer, y[i]),
                    GRAY_RGB( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] ) );
      }
   }
}


/*
 * Write an array of 8-bit PF_GRAYSCALE pixels to an XImage.
 */
static void write_pixels_GRAYSCALE8_ximage( RGBA_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer, x[i], y[i] );
	 *ptr = (GLubyte) GRAY_RGB( rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
      }
   }
}




/**********************************************************************/
/*** Write MONO COLOR SPAN functions                                ***/
/**********************************************************************/

#define MONO_SPAN_ARGS	const GLcontext *ctx,	\
		 	GLuint n, GLint x, GLint y, const GLubyte mask[]


/*
 * Write a span of identical pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_pixmap( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc1;
   register GLuint i;
   register GLboolean write_all;
   y = FLIP(xmesa->xm_buffer, y);
   write_all = GL_TRUE;
   for (i=0;i<n;i++) {
      if (!mask[i]) {
	 write_all = GL_FALSE;
	 break;
      }
   }
   if (write_all) {
      XMesaFillRectangle( dpy, buffer, gc, (int) x, (int) y, n, 1 );
   }
   else {
#if 0
      for (i=0;i<n;i++,x++) {
	 if (mask[i]) {
	    XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
	 }
      }
#else
      /* This version is usually faster.  Contributed by Jeff Epler
       * and cleaned up by Keith Whitwell.
       */
      for (i = 0; i < n; ) {
         GLuint start = i;
         /* Identify and emit contiguous rendered pixels
          */
         for( ; i < n && mask[i]; i++)
            /* Nothing */;
         if (start < i) 
            XMesaFillRectangle( dpy, buffer, gc,
                                (int)(x+start), (int) y,
                                (int)(i-start), 1);
         /* Eat up non-rendered pixels
          */
         for(; i < n && !mask[i]; i++)
            /* Nothing */;
      }
#endif
   }
}


/*
 * Write a span of PF_TRUEDITHER pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_TRUEDITHER_pixmap( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   register GLubyte r, g, b;
   int yy = FLIP(xmesa->xm_buffer, y);
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUEDITHER(p, x, yy, r, g, b);
         XMesaSetForeground( dpy, gc, p );
         XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) yy );
      }
   }
}


/*
 * Write a span of PF_DITHER pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_DITHER_pixmap( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   register GLubyte r, g, b;
   int yy = FLIP(xmesa->xm_buffer, y);
   XDITHER_SETUP(yy);
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
         XMesaSetForeground( dpy, gc, XDITHER( x, r, g, b ) );
         XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) yy );
      }
   }
}


/*
 * Write a span of PF_1BIT pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_1BIT_pixmap( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   register GLubyte r, g, b;
   SETUP_1BIT;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   y = FLIP(xmesa->xm_buffer, y);
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
         XMesaSetForeground( dpy, gc, DITHER_1BIT( x, y, r, g, b ) );
         XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
      }
   }
}


/*
 * Write a span of identical pixels to an XImage.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_ximage( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   register unsigned long p = xmesa->pixel;
   y = FLIP(xmesa->xm_buffer, y);
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 XMesaPutPixel( img, x, y, p );
      }
   }
}


/*
 * Write a span of identical PF_TRUEDITHER pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_TRUEDITHER_ximage( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   GLuint i;
   GLint r = xmesa->red;
   GLint g = xmesa->green;
   GLint b = xmesa->blue;
   y = FLIP(xmesa->xm_buffer, y);
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUEDITHER( p, x+i, y, r, g, b);
	 XMesaPutPixel( img, x+i, y, p );
      }
   }
}


/*
 * Write a span of identical 8A8B8G8R pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_8A8B8G8R_ximage( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLuint i, p, *ptr;
   p = (GLuint) xmesa->pixel;
   ptr = PIXELADDR4( xmesa->xm_buffer, x, y );
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 ptr[i] = p;
      }
   }
}


/*
 * Write a span of identical 8R8G8B pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_8R8G8B_ximage( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLuint i, p, *ptr;
   p = (GLuint) xmesa->pixel;
   ptr = PIXELADDR4( xmesa->xm_buffer, x, y );
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 ptr[i] = p;
      }
   }
}


/*
 * Write a span of identical 8R8G8B pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_8R8G8B24_ximage( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   const GLubyte r = (GLubyte) ((xmesa->pixel >> 16) & 0xff);
   const GLubyte g = (GLubyte) ((xmesa->pixel >> 8 ) & 0xff);
   const GLubyte b = (GLubyte) ((xmesa->pixel      ) & 0xff);
   GLuint i;
   bgr_t *ptr = PIXELADDR3( xmesa->xm_buffer, x, y );
   for (i=0;i<n;i++) {
      if (mask[i]) {
         ptr[i].r = r;
         ptr[i].g = g;
         ptr[i].b = b;
      }
   }
}


/*
 * Write a span of identical DITHER pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_DITHER_ximage( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   register GLubyte r, g, b;
   int yy = FLIP(xmesa->xm_buffer, y);
   XDITHER_SETUP(yy);
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 XMesaPutPixel( img, x, yy, XDITHER( x, r, g, b ) );
      }
   }
}


/*
 * Write a span of identical 8-bit DITHER pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_DITHER8_ximage( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer,x,y);
   register GLubyte r, g, b;
   XDITHER_SETUP(y);
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 ptr[i] = (GLubyte) XDITHER( x, r, g, b );
      }
   }
}


/*
 * Write a span of identical 8-bit LOOKUP pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_LOOKUP8_ximage( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer,x,y);
   register GLubyte pixel = (GLubyte) xmesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 ptr[i] = pixel;
      }
   }
}


/*
 * Write a span of identical PF_1BIT pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_1BIT_ximage( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   register GLubyte r, g, b;
   SETUP_1BIT;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   y = FLIP(xmesa->xm_buffer, y);
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 XMesaPutPixel( img, x, y, DITHER_1BIT( x, y, r, g, b ) );
      }
   }
}


/*
 * Write a span of identical HPCR pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_HPCR_ximage( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer,x,y);
   register GLubyte r, g, b;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
         ptr[i] = DITHER_HPCR( x, y, r, g, b );
      }
   }
}


/*
 * Write a span of identical 8-bit GRAYSCALE pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_GRAYSCALE8_ximage( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLuint i;
   GLubyte p = (GLubyte) xmesa->pixel;
   GLubyte *ptr = (GLubyte *) PIXELADDR1( xmesa->xm_buffer,x,y);
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 ptr[i] = p;
      }
   }
}



/*
 * Write a span of identical PF_DITHER_5R6G5B pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_DITHER_5R6G5B_ximage( MONO_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLushort *ptr = PIXELADDR2( xmesa->xm_buffer, x, y );
   GLuint i;
   GLint r = xmesa->red;
   GLint g = xmesa->green;
   GLint b = xmesa->blue;
   y = FLIP(xmesa->xm_buffer, y);
   for (i=0;i<n;i++) {
      if (mask[i]) {
         PACK_TRUEDITHER(ptr[i], x+i, y, r, g, b);
      }
   }
}



/**********************************************************************/
/*** Write MONO COLOR PIXELS functions                              ***/
/**********************************************************************/

#define MONO_PIXEL_ARGS	const GLcontext *ctx,				\
			GLuint n, const GLint x[], const GLint y[],	\
			const GLubyte mask[]

/*
 * Write an array of identical pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_pixmap( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc1;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_TRUEDITHER pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_TRUEDITHER_pixmap( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   register GLubyte r, g, b;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUEDITHER(p, x[i], y[i], r, g, b);
         XMesaSetForeground( dpy, gc, p );
	 XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_DITHER pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_DITHER_pixmap( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   register GLubyte r, g, b;
   DITHER_SETUP;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         XMesaSetForeground( dpy, gc, DITHER( x[i], y[i], r, g, b ) );
	 XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of PF_1BIT pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_1BIT_pixmap( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   register GLubyte r, g, b;
   SETUP_1BIT;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         XMesaSetForeground( dpy, gc, DITHER_1BIT( x[i], y[i], r, g, b ) );
	 XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of identical pixels to an XImage.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_ximage( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   register unsigned long p = xmesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaPutPixel( img, x[i], FLIP(xmesa->xm_buffer, y[i]), p );
      }
   }
}


/*
 * Write an array of identical TRUEDITHER pixels to an XImage.
 * The pixel value is the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_TRUEDITHER_ximage( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   int r = xmesa->red;
   int g = xmesa->green;
   int b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUEDITHER(p, x[i], FLIP(xmesa->xm_buffer, y[i]), r, g, b);
	 XMesaPutPixel( img, x[i], FLIP(xmesa->xm_buffer, y[i]), p );
      }
   }
}



/*
 * Write an array of identical 8A8B8G8R pixels to an XImage.  The pixel value
 * is the one set by DD.color().
 */
static void write_pixels_mono_8A8B8G8R_ximage( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLuint p = (GLuint) xmesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLuint *ptr = PIXELADDR4( xmesa->xm_buffer, x[i], y[i] );
	 *ptr = p;
      }
   }
}


/*
 * Write an array of identical 8R8G8B pixels to an XImage.  The pixel value
 * is the one set by DD.color().
 */
static void write_pixels_mono_8R8G8B_ximage( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLuint p = (GLuint) xmesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLuint *ptr = PIXELADDR4( xmesa->xm_buffer, x[i], y[i] );
	 *ptr = p;
      }
   }
}


/*
 * Write an array of identical 8R8G8B pixels to an XImage.  The pixel value
 * is the one set by DD.color().
 */
static void write_pixels_mono_8R8G8B24_ximage( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   const GLubyte r = (GLubyte) ((xmesa->pixel >> 16) & 0xff);
   const GLubyte g = (GLubyte) ((xmesa->pixel >> 8 ) & 0xff);
   const GLubyte b = (GLubyte) ((xmesa->pixel      ) & 0xff);
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 bgr_t *ptr = PIXELADDR3( xmesa->xm_buffer, x[i], y[i] );
         ptr->r = r;
         ptr->g = g;
         ptr->b = b;
      }
   }
}


/*
 * Write an array of identical PF_DITHER pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_pixels_mono_DITHER_ximage( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   register GLubyte r, g, b;
   DITHER_SETUP;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaPutPixel( img, x[i], FLIP(xmesa->xm_buffer, y[i]), DITHER( x[i], y[i], r, g, b ) );
      }
   }
}


/*
 * Write an array of identical 8-bit PF_DITHER pixels to an XImage.  The
 * pixel value is the one set by DD.color().
 */
static void write_pixels_mono_DITHER8_ximage( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte r, g, b;
   DITHER_SETUP;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer,x[i],y[i]);
	 *ptr = (GLubyte) DITHER( x[i], y[i], r, g, b );
      }
   }
}


/*
 * Write an array of identical 8-bit PF_LOOKUP pixels to an XImage.  The
 * pixel value is the one set by DD.color().
 */
static void write_pixels_mono_LOOKUP8_ximage( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte pixel = (GLubyte) xmesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer,x[i],y[i]);
	 *ptr = pixel;
      }
   }
}



/*
 * Write an array of identical PF_1BIT pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_pixels_mono_1BIT_ximage( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   register GLubyte r, g, b;
   SETUP_1BIT;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaPutPixel( img, x[i], FLIP(xmesa->xm_buffer, y[i]), DITHER_1BIT( x[i], y[i], r, g, b ));
      }
   }
}


/*
 * Write an array of identical PF_HPCR pixels to an XImage.  The
 * pixel value is the one set by DD.color().
 */
static void write_pixels_mono_HPCR_ximage( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte r, g, b;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer,x[i],y[i]);
         *ptr = DITHER_HPCR( x[i], y[i], r, g, b );
      }
   }
}


/*
 * Write an array of identical 8-bit PF_GRAYSCALE pixels to an XImage.  The
 * pixel value is the one set by DD.color().
 */
static void write_pixels_mono_GRAYSCALE8_ximage( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte p = (GLubyte) xmesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1( xmesa->xm_buffer,x[i],y[i]);
	 *ptr = p;
      }
   }
}


/*
 * Write an array of identical PF_DITHER_5R6G5B pixels to an XImage.
 * The pixel value is the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_DITHER_5R6G5B_ximage( MONO_PIXEL_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   int r = xmesa->red;
   int g = xmesa->green;
   int b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLushort *ptr = PIXELADDR2( xmesa->xm_buffer, x[i], y[i] );
         PACK_TRUEDITHER(*ptr, x[i], y[i], r, g, b);
      }
   }
}



/**********************************************************************/
/*** Write INDEX SPAN functions                                     ***/
/**********************************************************************/

#define INDEX_SPAN_ARGS	const GLcontext *ctx,				\
			GLuint n, GLint x, GLint y, const GLuint index[], \
			const GLubyte mask[]

#define INDEX8_SPAN_ARGS const GLcontext *ctx,				\
			 GLuint n, GLint x, GLint y, const GLubyte index[], \
			 const GLubyte mask[]


/*
 * Write a span of CI pixels to a Pixmap.
 */
static void write_span_index_pixmap( INDEX_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 XMesaSetForeground( dpy, gc, (unsigned long) index[i] );
	 XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
      }
   }
}


/*
 * Write a span of 8-bit CI pixels to a Pixmap.
 */
static void write_span_index8_pixmap( INDEX8_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaSetForeground( dpy, gc, (unsigned long) index[i] );
            XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      for (i=0;i<n;i++,x++) {
         XMesaSetForeground( dpy, gc, (unsigned long) index[i] );
         XMesaDrawPoint( dpy, buffer, gc, (int) x, (int) y );
      }
   }
}


/*
 * Write a span of CI pixels to an XImage.
 */
static void write_span_index_ximage( INDEX_SPAN_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   y = FLIP(xmesa->xm_buffer, y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XMesaPutPixel( img, x, y, (unsigned long) index[i] );
         }
      }
   }
   else {
      for (i=0;i<n;i++,x++) {
         XMesaPutPixel( img, x, y, (unsigned long) index[i] );
      }
   }
}


/*
 * Write a span of 8-bit CI pixels to a non 8-bit XImage.
 */
static void write_span_index8_ximage( const GLcontext *ctx, GLuint n,
                                      GLint x, GLint y, const GLubyte index[],
                                      const GLubyte mask[] )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (mask) {
      GLuint i;
      for (i=0;i<n;i++) {
         if (mask[i]) {
            XMesaPutPixel(xmesa->xm_buffer->backimage, x+i, y, index[i]);
         }
      }
   }
   else {
      GLuint i;
      for (i=0;i<n;i++) {
         XMesaPutPixel(xmesa->xm_buffer->backimage, x+i, y, index[i]);
      }
   }
}

/*
 * Write a span of 8-bit CI pixels to an 8-bit XImage.
 */
static void write_span_index8_ximage8( const GLcontext *ctx, GLuint n,
                                      GLint x, GLint y, const GLubyte index[],
                                      const GLubyte mask[] )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *dst = PIXELADDR1( xmesa->xm_buffer,x,y);
   if (mask) {
      GLuint i;
      for (i=0;i<n;i++) {
         if (mask[i]) {
            dst[i] = index[i];
         }
      }
   }
   else {
      MEMCPY( dst, index, n );
   }
}



/**********************************************************************/
/*** Write INDEX PIXELS functions                                   ***/
/**********************************************************************/

#define INDEX_PIXELS_ARGS	const GLcontext *ctx,			\
				GLuint n, const GLint x[], const GLint y[], \
				const GLuint index[], const GLubyte mask[]


/*
 * Write an array of CI pixels to a Pixmap.
 */
static void write_pixels_index_pixmap( INDEX_PIXELS_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaSetForeground( dpy, gc, (unsigned long) index[i] );
	 XMesaDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(xmesa->xm_buffer, y[i]) );
      }
   }
}


/*
 * Write an array of CI pixels to an XImage.
 */
static void write_pixels_index_ximage( INDEX_PIXELS_ARGS )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XMesaPutPixel( img, x[i], FLIP(xmesa->xm_buffer, y[i]), (unsigned long) index[i] );
      }
   }
}




/**********************************************************************/
/*****                      Pixel reading                         *****/
/**********************************************************************/



/*
 * Read a horizontal span of color-index pixels.
 */
static void read_index_span( const GLcontext *ctx,
			     GLuint n, GLint x, GLint y, GLuint index[] )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaBuffer source;
   GLuint i;

   if (xmesa->use_read_buffer)
      source = xmesa->xm_read_buffer;
   else
      source = xmesa->xm_buffer;

   y = FLIP(source, y);

   if (source->buffer) {
#ifndef XFree86Server
      XMesaImage *span = NULL;
      int error;
      catch_xgetimage_errors( xmesa->display );
      span = XGetImage( xmesa->display, source->buffer,
		        x, y, n, 1, AllPlanes, ZPixmap );
      error = check_xgetimage_errors();
      if (span && !error) {
	 for (i=0;i<n;i++) {
	    index[i] = (GLuint) XMesaGetPixel( span, i, 0 );
	 }
      }
      else {
	 /* return 0 pixels */
	 for (i=0;i<n;i++) {
	    index[i] = 0;
	 }
      }
      if (span) {
	 XMesaDestroyImage( span );
      }
#else
      (*xmesa->display->GetImage)(source->buffer,
				  x, y, n, 1, ZPixmap,
				  ~0L, (pointer)index);
#endif
   }
   else if (source->backimage) {
      XMesaImage *img = source->backimage;
      for (i=0;i<n;i++,x++) {
	 index[i] = (GLuint) XMesaGetPixel( img, x, y );
      }
   }
}



/*
 * Read a horizontal span of color pixels.
 */
static void read_color_span( const GLcontext *ctx,
			     GLuint n, GLint x, GLint y,
                             GLubyte rgba[][4] )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaBuffer source;

   if (xmesa->use_read_buffer)
      source = xmesa->xm_read_buffer;
   else
      source = xmesa->xm_buffer;

   if (source->buffer) {
      /* Read from Pixmap or Window */
      XMesaImage *span = NULL;
      int error;
#ifdef XFree86Server
      span = XMesaCreateImage(xmesa->xm_visual->BitsPerPixel, n, 1, NULL);
      span->data = (char *)MALLOC(span->height * span->bytes_per_line);
      error = (!span->data);
      (*xmesa->display->GetImage)(source->buffer,
				  x, FLIP(source, y), n, 1, ZPixmap,
				  ~0L, (pointer)span->data);
#else
      catch_xgetimage_errors( xmesa->display );
      span = XGetImage( xmesa->display, source->buffer,
		        x, FLIP(source, y), n, 1, AllPlanes, ZPixmap );
      error = check_xgetimage_errors();
#endif
      if (span && !error) {
	 switch (xmesa->pixelformat) {
	    case PF_TRUECOLOR:
	    case PF_TRUEDITHER:
               {
                  const GLubyte *pixelToR = xmesa->xm_visual->PixelToR;
                  const GLubyte *pixelToG = xmesa->xm_visual->PixelToG;
                  const GLubyte *pixelToB = xmesa->xm_visual->PixelToB;
                  unsigned long rMask = GET_REDMASK(xmesa->xm_visual);
                  unsigned long gMask = GET_GREENMASK(xmesa->xm_visual);
                  unsigned long bMask = GET_BLUEMASK(xmesa->xm_visual);
                  GLint rShift = xmesa->xm_visual->rshift;
                  GLint gShift = xmesa->xm_visual->gshift;
                  GLint bShift = xmesa->xm_visual->bshift;
                  GLuint i;
                  for (i=0;i<n;i++) {
                     unsigned long p;
                     p = XMesaGetPixel( span, i, 0 );
                     rgba[i][RCOMP] = pixelToR[(p & rMask) >> rShift];
                     rgba[i][GCOMP] = pixelToG[(p & gMask) >> gShift];
                     rgba[i][BCOMP] = pixelToB[(p & bMask) >> bShift];
                     rgba[i][ACOMP] = 255;
                  }
               }
	       break;
            case PF_5R6G5B:
            case PF_DITHER_5R6G5B:
               {
                  const GLubyte *pixelToR = xmesa->xm_visual->PixelToR;
                  const GLubyte *pixelToG = xmesa->xm_visual->PixelToG;
                  const GLubyte *pixelToB = xmesa->xm_visual->PixelToB;
                  GLuint i;
                  for (i=0;i<n;i++) {
                     unsigned long p = XMesaGetPixel( span, i, 0 );
                     /* fast, but not quite accurate
                     rgba[i][RCOMP] = ((p >> 8) & 0xf8);
                     rgba[i][GCOMP] = ((p >> 3) & 0xfc);
                     rgba[i][BCOMP] = ((p << 3) & 0xff);
                     */
                     rgba[i][RCOMP] = pixelToR[p >> 11];
                     rgba[i][GCOMP] = pixelToG[(p >> 5) & 0x3f];
                     rgba[i][BCOMP] = pixelToB[p & 0x1f];
                     rgba[i][ACOMP] = 255;
                  }
               }
	       break;
	    case PF_8A8B8G8R:
               {
                  const GLuint *ptr4 = (GLuint *) span->data;
                  GLuint i;
                  for (i=0;i<n;i++) {
                     GLuint p4 = *ptr4++;
                     rgba[i][RCOMP] = (GLubyte) ( p4        & 0xff);
                     rgba[i][GCOMP] = (GLubyte) ((p4 >> 8)  & 0xff);
                     rgba[i][BCOMP] = (GLubyte) ((p4 >> 16) & 0xff);
                     rgba[i][ACOMP] = (GLubyte) ((p4 >> 24) & 0xff);
                  }
	       }
	       break;
            case PF_8R8G8B:
               {
                  const GLuint *ptr4 = (GLuint *) span->data;
                  GLuint i;
                  for (i=0;i<n;i++) {
                     GLuint p4 = *ptr4++;
                     rgba[i][RCOMP] = (GLubyte) ((p4 >> 16) & 0xff);
                     rgba[i][GCOMP] = (GLubyte) ((p4 >> 8)  & 0xff);
                     rgba[i][BCOMP] = (GLubyte) ( p4        & 0xff);
                     rgba[i][ACOMP] = 255;
                  }
	       }
	       break;
            case PF_8R8G8B24:
               {
                  const bgr_t *ptr3 = (bgr_t *) span->data;
                  GLuint i;
                  for (i=0;i<n;i++) {
                     rgba[i][RCOMP] = ptr3[i].r;
                     rgba[i][GCOMP] = ptr3[i].g;
                     rgba[i][BCOMP] = ptr3[i].b;
                     rgba[i][ACOMP] = 255;
                  }
	       }
	       break;
            case PF_HPCR:
               {
                  GLubyte *ptr1 = (GLubyte *) span->data;
                  GLuint i;
                  for (i=0;i<n;i++) {
                     GLubyte p = *ptr1++;
                     rgba[i][RCOMP] =  p & 0xE0;
                     rgba[i][GCOMP] = (p & 0x1C) << 3;
                     rgba[i][BCOMP] = (p & 0x03) << 6;
                     rgba[i][ACOMP] = 255;
                  }
               }
               break;
	    case PF_DITHER:
	    case PF_LOOKUP:
	    case PF_GRAYSCALE:
               {
                  GLubyte *rTable = source->pixel_to_r;
                  GLubyte *gTable = source->pixel_to_g;
                  GLubyte *bTable = source->pixel_to_b;
                  if (GET_VISUAL_DEPTH(xmesa->xm_visual)==8) {
                     const GLubyte *ptr1 = (GLubyte *) span->data;
                     GLuint i;
                     for (i=0;i<n;i++) {
                        unsigned long p = *ptr1++;
                        rgba[i][RCOMP] = rTable[p];
                        rgba[i][GCOMP] = gTable[p];
                        rgba[i][BCOMP] = bTable[p];
                        rgba[i][ACOMP] = 255;
                     }
                  }
                  else {
                     GLuint i;
                     for (i=0;i<n;i++) {
                        unsigned long p = XMesaGetPixel( span, i, 0 );
                        rgba[i][RCOMP] = rTable[p];
                        rgba[i][GCOMP] = gTable[p];
                        rgba[i][BCOMP] = bTable[p];
                        rgba[i][ACOMP] = 255;
                     }
                  }
               }
	       break;
	    case PF_1BIT:
               {
                  int bitFlip = xmesa->xm_visual->bitFlip;
                  GLuint i;
                  for (i=0;i<n;i++) {
                     unsigned long p;
                     p = XMesaGetPixel( span, i, 0 ) ^ bitFlip;
                     rgba[i][RCOMP] = (GLubyte) (p * 255);
                     rgba[i][GCOMP] = (GLubyte) (p * 255);
                     rgba[i][BCOMP] = (GLubyte) (p * 255);
                     rgba[i][ACOMP] = 255;
                  }
               }
	       break;
	    default:
	       gl_problem(NULL,"Problem in DD.read_color_span (1)");
               return;
	 }
      }
      else {
	 /* return black pixels */
         GLuint i;
	 for (i=0;i<n;i++) {
	    rgba[i][RCOMP] = rgba[i][GCOMP] = rgba[i][BCOMP] = rgba[i][ACOMP] = 0;
	 }
      }
      if (span) {
	 XMesaDestroyImage( span );
      }
   }
   else if (source->backimage) {
      /* Read from XImage back buffer */
      switch (xmesa->pixelformat) {
         case PF_TRUECOLOR:
         case PF_TRUEDITHER:
            {
               const GLubyte *pixelToR = xmesa->xm_visual->PixelToR;
               const GLubyte *pixelToG = xmesa->xm_visual->PixelToG;
               const GLubyte *pixelToB = xmesa->xm_visual->PixelToB;
               unsigned long rMask = GET_REDMASK(xmesa->xm_visual);
               unsigned long gMask = GET_GREENMASK(xmesa->xm_visual);
               unsigned long bMask = GET_BLUEMASK(xmesa->xm_visual);
               GLint rShift = xmesa->xm_visual->rshift;
               GLint gShift = xmesa->xm_visual->gshift;
               GLint bShift = xmesa->xm_visual->bshift;
               XMesaImage *img = source->backimage;
               GLuint i;
               y = FLIP(source, y);
               for (i=0;i<n;i++) {
                  unsigned long p;
		  p = XMesaGetPixel( img, x+i, y );
                  rgba[i][RCOMP] = pixelToR[(p & rMask) >> rShift];
                  rgba[i][GCOMP] = pixelToG[(p & gMask) >> gShift];
                  rgba[i][BCOMP] = pixelToB[(p & bMask) >> bShift];
                  rgba[i][ACOMP] = 255;
               }
            }
            break;
         case PF_5R6G5B:
         case PF_DITHER_5R6G5B:
            {
               const GLubyte *pixelToR = xmesa->xm_visual->PixelToR;
               const GLubyte *pixelToG = xmesa->xm_visual->PixelToG;
               const GLubyte *pixelToB = xmesa->xm_visual->PixelToB;
               const GLushort *ptr2 = PIXELADDR2( source, x, y );
               const GLuint *ptr4 = (const GLuint *) ptr2;
               GLuint i;
#if defined(__i386__) /* word stores don't have to be on 4-byte boundaries */
               GLuint extraPixel = (n & 1);
               n -= extraPixel;
               for (i = 0; i < n; i += 2) {
                  const GLuint p = *ptr4++;
                  const GLuint p0 = p & 0xffff;
                  const GLuint p1 = p >> 16;
                  /* fast, but not quite accurate
                  rgba[i][RCOMP] = ((p >> 8) & 0xf8);
                  rgba[i][GCOMP] = ((p >> 3) & 0xfc);
                  rgba[i][BCOMP] = ((p << 3) & 0xff);
                  */
                  rgba[i][RCOMP] = pixelToR[p0 >> 11];
                  rgba[i][GCOMP] = pixelToG[(p0 >> 5) & 0x3f];
                  rgba[i][BCOMP] = pixelToB[p0 & 0x1f];
                  rgba[i][ACOMP] = 255;
                  rgba[i+1][RCOMP] = pixelToR[p1 >> 11];
                  rgba[i+1][GCOMP] = pixelToG[(p1 >> 5) & 0x3f];
                  rgba[i+1][BCOMP] = pixelToB[p1 & 0x1f];
                  rgba[i+1][ACOMP] = 255;
               }
               if (extraPixel) {
                  GLushort p = ptr2[n];
                  rgba[n][RCOMP] = pixelToR[p >> 11];
                  rgba[n][GCOMP] = pixelToG[(p >> 5) & 0x3f];
                  rgba[n][BCOMP] = pixelToB[p & 0x1f];
                  rgba[n][ACOMP] = 255;
               }
#else
               for (i = 0; i < n; i++) {
                  const GLushort p = ptr2[i];
                  rgba[i][RCOMP] = pixelToR[p >> 11];
                  rgba[i][GCOMP] = pixelToG[(p >> 5) & 0x3f];
                  rgba[i][BCOMP] = pixelToB[p & 0x1f];
                  rgba[i][ACOMP] = 255;
               }
#endif
            }
            break;
	 case PF_8A8B8G8R:
            {
               const GLuint *ptr4 = PIXELADDR4( source, x, y );
               GLuint i;
               for (i=0;i<n;i++) {
                  GLuint p4 = *ptr4++;
                  rgba[i][RCOMP] = (GLubyte) ( p4        & 0xff);
                  rgba[i][GCOMP] = (GLubyte) ((p4 >> 8)  & 0xff);
                  rgba[i][BCOMP] = (GLubyte) ((p4 >> 16) & 0xff);
                  rgba[i][ACOMP] = (GLint)   ((p4 >> 24) & 0xff);
               }
            }
	    break;
	 case PF_8R8G8B:
            {
               const GLuint *ptr4 = PIXELADDR4( source, x, y );
               GLuint i;
               for (i=0;i<n;i++) {
                  GLuint p4 = *ptr4++;
                  rgba[i][RCOMP] = (GLubyte) ((p4 >> 16) & 0xff);
                  rgba[i][GCOMP] = (GLubyte) ((p4 >> 8)  & 0xff);
                  rgba[i][BCOMP] = (GLubyte) ( p4        & 0xff);
                  rgba[i][ACOMP] = 255;
               }
            }
	    break;
	 case PF_8R8G8B24:
            {
               const bgr_t *ptr3 = PIXELADDR3( source, x, y );
               GLuint i;
               for (i=0;i<n;i++) {
                  rgba[i][RCOMP] = ptr3[i].r;
                  rgba[i][GCOMP] = ptr3[i].g;
                  rgba[i][BCOMP] = ptr3[i].b;
                  rgba[i][ACOMP] = 255;
               }
            }
	    break;
         case PF_HPCR:
            {
               const GLubyte *ptr1 = PIXELADDR1( source, x, y );
               GLuint i;
               for (i=0;i<n;i++) {
                  GLubyte p = *ptr1++;
                  rgba[i][RCOMP] =  p & 0xE0;
                  rgba[i][GCOMP] = (p & 0x1C) << 3;
                  rgba[i][BCOMP] = (p & 0x03) << 6;
                  rgba[i][ACOMP] = 255;
               }
            }
            break;
	 case PF_DITHER:
	 case PF_LOOKUP:
	 case PF_GRAYSCALE:
            {
               const GLubyte *rTable = source->pixel_to_r;
               const GLubyte *gTable = source->pixel_to_g;
               const GLubyte *bTable = source->pixel_to_b;
               if (GET_VISUAL_DEPTH(xmesa->xm_visual)==8) {
                  GLubyte *ptr1 = PIXELADDR1( source, x, y );
                  GLuint i;
                  for (i=0;i<n;i++) {
                     unsigned long p = *ptr1++;
                     rgba[i][RCOMP] = rTable[p];
                     rgba[i][GCOMP] = gTable[p];
                     rgba[i][BCOMP] = bTable[p];
                     rgba[i][ACOMP] = 255;
                  }
               }
               else {
                  XMesaImage *img = source->backimage;
                  GLuint i;
                  y = FLIP(source, y);
                  for (i=0;i<n;i++,x++) {
                     unsigned long p = XMesaGetPixel( img, x, y );
                     rgba[i][RCOMP] = rTable[p];
                     rgba[i][GCOMP] = gTable[p];
                     rgba[i][BCOMP] = bTable[p];
                     rgba[i][ACOMP] = 255;
                  }
               }
            }
	    break;
	 case PF_1BIT:
            {
               XMesaImage *img = source->backimage;
               int bitFlip = xmesa->xm_visual->bitFlip;
               GLuint i;
               y = FLIP(source, y);
               for (i=0;i<n;i++,x++) {
                  unsigned long p;
		  p = XMesaGetPixel( img, x, y ) ^ bitFlip;
                  rgba[i][RCOMP] = (GLubyte) (p * 255);
                  rgba[i][GCOMP] = (GLubyte) (p * 255);
                  rgba[i][BCOMP] = (GLubyte) (p * 255);
                  rgba[i][ACOMP] = 255;
               }
	    }
	    break;
	 default:
	    gl_problem(NULL,"Problem in DD.read_color_span (2)");
            return;
      }
   }
}



/*
 * Read an array of color index pixels.
 */
static void read_index_pixels( const GLcontext *ctx,
			       GLuint n, const GLint x[], const GLint y[],
                               GLuint indx[], const GLubyte mask[] )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   XMesaBuffer source;

   if (xmesa->use_read_buffer)
      source = xmesa->xm_read_buffer;
   else
      source = xmesa->xm_buffer;

   if (source->buffer) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            indx[i] = (GLuint) read_pixel( xmesa->display,
                                           source->buffer,
                                           x[i], FLIP(source, y[i]) );
         }
      }
   }
   else if (source->backimage) {
      XMesaImage *img = source->backimage;
      for (i=0;i<n;i++) {
         if (mask[i]) {
            indx[i] = (GLuint) XMesaGetPixel( img, x[i], FLIP(source, y[i]) );
         }
      }
   }
}



static void read_color_pixels( const GLcontext *ctx,
			       GLuint n, const GLint x[], const GLint y[],
                               GLubyte rgba[][4], const GLubyte mask[] )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   register GLuint i;
   XMesaBuffer source;
   XMesaDrawable buffer;

   if (xmesa->use_read_buffer)
      source = xmesa->xm_read_buffer;
   else
      source = xmesa->xm_buffer;

   buffer = source->buffer;  /* the X drawable */

   if (source->buffer) {
      switch (xmesa->pixelformat) {
	 case PF_TRUECOLOR:
         case PF_TRUEDITHER:
         case PF_5R6G5B:
         case PF_DITHER_5R6G5B:
            {
               unsigned long rMask = GET_REDMASK(xmesa->xm_visual);
               unsigned long gMask = GET_GREENMASK(xmesa->xm_visual);
               unsigned long bMask = GET_BLUEMASK(xmesa->xm_visual);
               GLubyte *pixelToR = xmesa->xm_visual->PixelToR;
               GLubyte *pixelToG = xmesa->xm_visual->PixelToG;
               GLubyte *pixelToB = xmesa->xm_visual->PixelToB;
               GLint rShift = xmesa->xm_visual->rshift;
               GLint gShift = xmesa->xm_visual->gshift;
               GLint bShift = xmesa->xm_visual->bshift;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p = read_pixel( dpy, buffer,
                                                   x[i], FLIP(source, y[i]) );
                     rgba[i][RCOMP] = pixelToR[(p & rMask) >> rShift];
                     rgba[i][GCOMP] = pixelToG[(p & gMask) >> gShift];
                     rgba[i][BCOMP] = pixelToB[(p & bMask) >> bShift];
                     rgba[i][ACOMP] = 255;
                  }
               }
            }
            break;
	 case PF_8A8B8G8R:
	    for (i=0;i<n;i++) {
               if (mask[i]) {
                  unsigned long p = read_pixel( dpy, buffer,
                                                x[i], FLIP(source, y[i]) );
                  rgba[i][RCOMP] = (GLubyte) ( p        & 0xff);
                  rgba[i][GCOMP] = (GLubyte) ((p >> 8)  & 0xff);
                  rgba[i][BCOMP] = (GLubyte) ((p >> 16) & 0xff);
                  rgba[i][ACOMP] = (GLubyte) ((p >> 24) & 0xff);
               }
	    }
	    break;
	 case PF_8R8G8B:
	    for (i=0;i<n;i++) {
               if (mask[i]) {
                  unsigned long p = read_pixel( dpy, buffer,
                                                x[i], FLIP(source, y[i]) );
                  rgba[i][RCOMP] = (GLubyte) ((p >> 16) & 0xff);
                  rgba[i][GCOMP] = (GLubyte) ((p >> 8)  & 0xff);
                  rgba[i][BCOMP] = (GLubyte) ( p        & 0xff);
                  rgba[i][ACOMP] = 255;
               }
	    }
	    break;
	 case PF_8R8G8B24:
	    for (i=0;i<n;i++) {
               if (mask[i]) {
                  unsigned long p = read_pixel( dpy, buffer,
                                                x[i], FLIP(source, y[i]) );
                  rgba[i][RCOMP] = (GLubyte) ((p >> 16) & 0xff);
                  rgba[i][GCOMP] = (GLubyte) ((p >> 8)  & 0xff);
                  rgba[i][BCOMP] = (GLubyte) ( p        & 0xff);
                  rgba[i][ACOMP] = 255;
               }
	    }
	    break;
         case PF_HPCR:
            {
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p = read_pixel( dpy, buffer,
                                                   x[i], FLIP(source, y[i]) );
                     rgba[i][RCOMP] = (GLubyte) ( p & 0xE0      );
                     rgba[i][GCOMP] = (GLubyte) ((p & 0x1C) << 3);
                     rgba[i][BCOMP] = (GLubyte) ((p & 0x03) << 6);
                     rgba[i][ACOMP] = (GLubyte) 255;
                  }
               }
            }
            break;
	 case PF_DITHER:
	 case PF_LOOKUP:
	 case PF_GRAYSCALE:
            {
               GLubyte *rTable = source->pixel_to_r;
               GLubyte *gTable = source->pixel_to_g;
               GLubyte *bTable = source->pixel_to_b;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p = read_pixel( dpy, buffer,
                                                   x[i], FLIP(source, y[i]) );
                     rgba[i][RCOMP] = rTable[p];
                     rgba[i][GCOMP] = gTable[p];
                     rgba[i][BCOMP] = bTable[p];
                     rgba[i][ACOMP] = 255;
                  }
               }
	    }
	    break;
	 case PF_1BIT:
            {
               int bitFlip = xmesa->xm_visual->bitFlip;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p = read_pixel( dpy, buffer,
                                     x[i], FLIP(source, y[i])) ^ bitFlip;
                     rgba[i][RCOMP] = (GLubyte) (p * 255);
                     rgba[i][GCOMP] = (GLubyte) (p * 255);
                     rgba[i][BCOMP] = (GLubyte) (p * 255);
                     rgba[i][ACOMP] = 255;
                  }
               }
	    }
	    break;
	 default:
	    gl_problem(NULL,"Problem in DD.read_color_pixels (1)");
            return;
      }
   }
   else if (source->backimage) {
      switch (xmesa->pixelformat) {
	 case PF_TRUECOLOR:
         case PF_TRUEDITHER:
         case PF_5R6G5B:
         case PF_DITHER_5R6G5B:
            {
               unsigned long rMask = GET_REDMASK(xmesa->xm_visual);
               unsigned long gMask = GET_GREENMASK(xmesa->xm_visual);
               unsigned long bMask = GET_BLUEMASK(xmesa->xm_visual);
               GLubyte *pixelToR = xmesa->xm_visual->PixelToR;
               GLubyte *pixelToG = xmesa->xm_visual->PixelToG;
               GLubyte *pixelToB = xmesa->xm_visual->PixelToB;
               GLint rShift = xmesa->xm_visual->rshift;
               GLint gShift = xmesa->xm_visual->gshift;
               GLint bShift = xmesa->xm_visual->bshift;
               XMesaImage *img = source->backimage;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p;
                     p = XMesaGetPixel( img, x[i], FLIP(source, y[i]) );
                     rgba[i][RCOMP] = pixelToR[(p & rMask) >> rShift];
                     rgba[i][GCOMP] = pixelToG[(p & gMask) >> gShift];
                     rgba[i][BCOMP] = pixelToB[(p & bMask) >> bShift];
                     rgba[i][ACOMP] = 255;
                  }
               }
            }
            break;
	 case PF_8A8B8G8R:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLuint *ptr4 = PIXELADDR4( source, x[i], y[i] );
                  GLuint p4 = *ptr4;
                  rgba[i][RCOMP] = (GLubyte) ( p4        & 0xff);
                  rgba[i][GCOMP] = (GLubyte) ((p4 >> 8)  & 0xff);
                  rgba[i][BCOMP] = (GLubyte) ((p4 >> 16) & 0xff);
                  rgba[i][ACOMP] = (GLubyte) ((p4 >> 24) & 0xff);
               }
	    }
	    break;
	 case PF_8R8G8B:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLuint *ptr4 = PIXELADDR4( source, x[i], y[i] );
                  GLuint p4 = *ptr4;
                  rgba[i][RCOMP] = (GLubyte) ((p4 >> 16) & 0xff);
                  rgba[i][GCOMP] = (GLubyte) ((p4 >> 8)  & 0xff);
                  rgba[i][BCOMP] = (GLubyte) ( p4        & 0xff);
                  rgba[i][ACOMP] = 255;
               }
	    }
	    break;
	 case PF_8R8G8B24:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  bgr_t *ptr3 = PIXELADDR3( source, x[i], y[i] );
                  rgba[i][RCOMP] = ptr3->r;
                  rgba[i][GCOMP] = ptr3->g;
                  rgba[i][BCOMP] = ptr3->b;
                  rgba[i][ACOMP] = 255;
               }
	    }
	    break;
         case PF_HPCR:
            for (i=0;i<n;i++) {
               if (mask[i]) {
                  GLubyte *ptr1 = PIXELADDR1( source, x[i], y[i] );
                  GLubyte p = *ptr1;
                  rgba[i][RCOMP] =  p & 0xE0;
                  rgba[i][GCOMP] = (p & 0x1C) << 3;
                  rgba[i][BCOMP] = (p & 0x03) << 6;
                  rgba[i][ACOMP] = 255;
               }
            }
            break;
	 case PF_DITHER:
	 case PF_LOOKUP:
	 case PF_GRAYSCALE:
            {
               GLubyte *rTable = source->pixel_to_r;
               GLubyte *gTable = source->pixel_to_g;
               GLubyte *bTable = source->pixel_to_b;
               XMesaImage *img = source->backimage;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p;
                     p = XMesaGetPixel( img, x[i], FLIP(source, y[i]) );
                     rgba[i][RCOMP] = rTable[p];
                     rgba[i][GCOMP] = gTable[p];
                     rgba[i][BCOMP] = bTable[p];
                     rgba[i][ACOMP] = 255;
                  }
               }
	    }
	    break;
	 case PF_1BIT:
            {
               XMesaImage *img = source->backimage;
               int bitFlip = xmesa->xm_visual->bitFlip;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p;
                     p = XMesaGetPixel( img, x[i], FLIP(source, y[i]) ) ^ bitFlip;
                     rgba[i][RCOMP] = (GLubyte) (p * 255);
                     rgba[i][GCOMP] = (GLubyte) (p * 255);
                     rgba[i][BCOMP] = (GLubyte) (p * 255);
                     rgba[i][ACOMP] = 255;
                  }
               }
	    }
	    break;
	 default:
	    gl_problem(NULL,"Problem in DD.read_color_pixels (1)");
            return;
      }
   }
}



#ifndef XFree86Server
/*
 * This function implements glDrawPixels() with an XPutImage call when
 * drawing to the front buffer (X Window drawable).
 * The image format must be GL_BGRA to match the PF_8R8G8B pixel format.
 * XXX top/bottom edge clipping is broken!
 */
static GLboolean
drawpixels_8R8G8B( GLcontext *ctx,
                   GLint x, GLint y, GLsizei width, GLsizei height,
                   GLenum format, GLenum type,
                   const struct gl_pixelstore_attrib *unpack,
                   const GLvoid *pixels )
{
   const XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XMesaDisplay *dpy = xmesa->xm_visual->display;
   XMesaDrawable buffer = xmesa->xm_buffer->buffer;
   XMesaGC gc = xmesa->xm_buffer->gc1;
   assert(dpy);
   assert(buffer);
   assert(gc);

   /* XXX also check for pixel scale/bias/lookup/zooming! */
   if (format == GL_BGRA && type == GL_UNSIGNED_BYTE) {
      int dstX = x;
      int dstY = y;
      int w = width;
      int h = height;
      int srcX = unpack->SkipPixels;
      int srcY = unpack->SkipRows;
      if (_mesa_clip_pixelrect(ctx, &dstX, &dstY, &w, &h, &srcX, &srcY)) {
         XMesaImage ximage;
         MEMSET(&ximage, 0, sizeof(XMesaImage));
         ximage.width = width;
         ximage.height = height;
         ximage.format = ZPixmap;
         ximage.data = (char *) pixels + (height - 1) * width * 4;
         ximage.byte_order = LSBFirst;
         ximage.bitmap_unit = 32;
         ximage.bitmap_bit_order = LSBFirst;
         ximage.bitmap_pad = 32;
         ximage.depth = 24;
         ximage.bytes_per_line = -width * 4;
         ximage.bits_per_pixel = 32;
         ximage.red_mask   = 0xff0000;
         ximage.green_mask = 0x00ff00;
         ximage.blue_mask  = 0x0000ff;
         dstY = FLIP(xmesa->xm_buffer,dstY) - height + 1;
         XPutImage(dpy, buffer, gc, &ximage, srcX, srcY, dstX, dstY, w, h);
         return GL_TRUE;
      }
   }
   return GL_FALSE;
}
#endif




static const GLubyte *get_string( GLcontext *ctx, GLenum name )
{
   (void) ctx;
   switch (name) {
      case GL_RENDERER:
#ifdef XFree86Server
         return (const GLubyte *) "Mesa GLX Indirect";
#else
         return (const GLubyte *) "Mesa X11";
#endif
      case GL_VENDOR:
#ifdef XFree86Server
         return (const GLubyte *) "VA Linux Systems, Inc.";
#else
         return NULL;
#endif
      default:
         return NULL;
   }
}


static void update_span_funcs( GLcontext *ctx )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   int depth=GET_VISUAL_DEPTH(xmesa->xm_visual);

   /*
    * These drawing functions depend on color buffer config:
    */
   if (xmesa->xm_buffer->buffer!=XIMAGE) {
      /* Writing to window or back pixmap */
      switch (xmesa->pixelformat) {
	 case PF_INDEX:
	    ctx->Driver.WriteCI32Span     = write_span_index_pixmap;
	    ctx->Driver.WriteCI8Span      = write_span_index8_pixmap;
	    ctx->Driver.WriteMonoCISpan   = write_span_mono_pixmap;
	    ctx->Driver.WriteCI32Pixels   = write_pixels_index_pixmap;
	    ctx->Driver.WriteMonoCIPixels = write_pixels_mono_pixmap;
	    break;
	 case PF_TRUECOLOR:
	    ctx->Driver.WriteRGBASpan       = write_span_TRUECOLOR_pixmap;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_TRUECOLOR_pixmap;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_pixmap;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_TRUECOLOR_pixmap;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_pixmap;
	    break;
	 case PF_TRUEDITHER:
	    ctx->Driver.WriteRGBASpan       = write_span_TRUEDITHER_pixmap;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_TRUEDITHER_pixmap;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_TRUEDITHER_pixmap;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_TRUEDITHER_pixmap;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_TRUEDITHER_pixmap;
	    break;
	 case PF_8A8B8G8R:
	    ctx->Driver.WriteRGBASpan       = write_span_8A8B8G8R_pixmap;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_8A8B8G8R_pixmap;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_pixmap;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_8A8B8G8R_pixmap;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_pixmap;
	    break;
	 case PF_8R8G8B:
	    ctx->Driver.WriteRGBASpan       = write_span_8R8G8B_pixmap;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_8R8G8B_pixmap;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_pixmap;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_8R8G8B_pixmap;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_pixmap;
            ctx->Driver.DrawPixels          = NULL; /*drawpixels_8R8G8B;*/
	    break;
	 case PF_8R8G8B24:
	    ctx->Driver.WriteRGBASpan       = write_span_8R8G8B24_pixmap;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_8R8G8B24_pixmap;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_pixmap;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_8R8G8B24_pixmap;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_pixmap;
	    break;
	 case PF_5R6G5B:
	    ctx->Driver.WriteRGBASpan       = write_span_5R6G5B_pixmap;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_5R6G5B_pixmap;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_pixmap;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_5R6G5B_pixmap;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_pixmap;
	    break;
	 case PF_DITHER_5R6G5B:
	    ctx->Driver.WriteRGBASpan       = write_span_DITHER_5R6G5B_pixmap;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_DITHER_5R6G5B_pixmap;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_TRUEDITHER_pixmap;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_DITHER_5R6G5B_pixmap;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_TRUEDITHER_pixmap;
	    break;
	 case PF_DITHER:
	    ctx->Driver.WriteRGBASpan       = write_span_DITHER_pixmap;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_DITHER_pixmap;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_DITHER_pixmap;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_DITHER_pixmap;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_DITHER_pixmap;
	    break;
	 case PF_1BIT:
	    ctx->Driver.WriteRGBASpan       = write_span_1BIT_pixmap;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_1BIT_pixmap;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_1BIT_pixmap;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_1BIT_pixmap;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_1BIT_pixmap;
	    break;
         case PF_HPCR:
            ctx->Driver.WriteRGBASpan       = write_span_HPCR_pixmap;
            ctx->Driver.WriteRGBSpan        = write_span_rgb_HPCR_pixmap;
            ctx->Driver.WriteMonoRGBASpan   = write_span_mono_pixmap;
            ctx->Driver.WriteRGBAPixels     = write_pixels_HPCR_pixmap;
            ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_pixmap;
	    if (xmesa->xm_visual->hpcr_clear_flag) {
   		ctx->Driver.ClearColor = clear_color_HPCR_pixmap;
	    }
            break;
         case PF_LOOKUP:
            ctx->Driver.WriteRGBASpan       = write_span_LOOKUP_pixmap;
            ctx->Driver.WriteRGBSpan        = write_span_rgb_LOOKUP_pixmap;
            ctx->Driver.WriteMonoRGBASpan   = write_span_mono_pixmap;
            ctx->Driver.WriteRGBAPixels     = write_pixels_LOOKUP_pixmap;
            ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_pixmap;
            break;
         case PF_GRAYSCALE:
            ctx->Driver.WriteRGBASpan       = write_span_GRAYSCALE_pixmap;
            ctx->Driver.WriteRGBSpan        = write_span_rgb_GRAYSCALE_pixmap;
            ctx->Driver.WriteMonoRGBASpan   = write_span_mono_pixmap;
            ctx->Driver.WriteRGBAPixels     = write_pixels_GRAYSCALE_pixmap;
            ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_pixmap;
            break;
	 default:
	    gl_problem(NULL,"Bad pixel format in xmesa_update_state (1)");
            return;
      }
   }
   else if (xmesa->xm_buffer->buffer==XIMAGE) {
      /* Writing to back XImage */
      switch (xmesa->pixelformat) {
	 case PF_INDEX:
	    ctx->Driver.WriteCI32Span     = write_span_index_ximage;
	    if (depth==8)
               ctx->Driver.WriteCI8Span   = write_span_index8_ximage8;
            else
               ctx->Driver.WriteCI8Span   = write_span_index8_ximage;
	    ctx->Driver.WriteMonoCISpan   = write_span_mono_ximage;
	    ctx->Driver.WriteCI32Pixels   = write_pixels_index_ximage;
	    ctx->Driver.WriteMonoCIPixels = write_pixels_mono_ximage;
	    break;
	 case PF_TRUECOLOR:
	    /* Generic RGB */
	    ctx->Driver.WriteRGBASpan       = write_span_TRUECOLOR_ximage;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_TRUECOLOR_ximage;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_ximage;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_TRUECOLOR_ximage;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_ximage;
	    break;
	 case PF_TRUEDITHER:
	    ctx->Driver.WriteRGBASpan       = write_span_TRUEDITHER_ximage;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_TRUEDITHER_ximage;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_TRUEDITHER_ximage;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_TRUEDITHER_ximage;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_TRUEDITHER_ximage;
	    break;
	 case PF_8A8B8G8R:
	    ctx->Driver.WriteRGBASpan       = write_span_8A8B8G8R_ximage;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_8A8B8G8R_ximage;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_8A8B8G8R_ximage;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_8A8B8G8R_ximage;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_8A8B8G8R_ximage;
	    break;
	 case PF_8R8G8B:
	    ctx->Driver.WriteRGBASpan       = write_span_8R8G8B_ximage;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_8R8G8B_ximage;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_8R8G8B_ximage;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_8R8G8B_ximage;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_8R8G8B_ximage;
            ctx->Driver.DrawPixels          = NULL;
	    break;
	 case PF_8R8G8B24:
	    ctx->Driver.WriteRGBASpan       = write_span_8R8G8B24_ximage;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_8R8G8B24_ximage;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_8R8G8B24_ximage;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_8R8G8B24_ximage;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_8R8G8B24_ximage;
	    break;
	 case PF_5R6G5B:
	    ctx->Driver.WriteRGBASpan       = write_span_5R6G5B_ximage;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_5R6G5B_ximage;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_ximage;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_5R6G5B_ximage;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_ximage;
	    break;
	 case PF_DITHER_5R6G5B:
	    ctx->Driver.WriteRGBASpan       = write_span_DITHER_5R6G5B_ximage;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_DITHER_5R6G5B_ximage;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_DITHER_5R6G5B_ximage;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_DITHER_5R6G5B_ximage;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_DITHER_5R6G5B_ximage;
	    break;
	 case PF_DITHER:
	    if (depth==8) {
	       ctx->Driver.WriteRGBASpan      = write_span_DITHER8_ximage;
	       ctx->Driver.WriteRGBSpan       = write_span_rgb_DITHER8_ximage;
	       ctx->Driver.WriteMonoRGBASpan  = write_span_mono_DITHER8_ximage;
	       ctx->Driver.WriteRGBAPixels    = write_pixels_DITHER8_ximage;
	       ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_DITHER8_ximage;
	    }
	    else {
	       ctx->Driver.WriteRGBASpan       = write_span_DITHER_ximage;
	       ctx->Driver.WriteRGBSpan        = write_span_rgb_DITHER_ximage;
	       ctx->Driver.WriteMonoRGBASpan   = write_span_mono_DITHER_ximage;
	       ctx->Driver.WriteRGBAPixels     = write_pixels_DITHER_ximage;
	       ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_DITHER_ximage;
	    }
	    break;
	 case PF_1BIT:
	    ctx->Driver.WriteRGBASpan       = write_span_1BIT_ximage;
	    ctx->Driver.WriteRGBSpan        = write_span_rgb_1BIT_ximage;
	    ctx->Driver.WriteMonoRGBASpan   = write_span_mono_1BIT_ximage;
	    ctx->Driver.WriteRGBAPixels     = write_pixels_1BIT_ximage;
	    ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_1BIT_ximage;
	    break;
         case PF_HPCR:
            ctx->Driver.WriteRGBASpan       = write_span_HPCR_ximage;
            ctx->Driver.WriteRGBSpan        = write_span_rgb_HPCR_ximage;
            ctx->Driver.WriteMonoRGBASpan   = write_span_mono_HPCR_ximage;
            ctx->Driver.WriteRGBAPixels     = write_pixels_HPCR_ximage;
            ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_HPCR_ximage;
	    if (xmesa->xm_visual->hpcr_clear_flag) {
               ctx->Driver.ClearColor = clear_color_HPCR_ximage;
	    }
            break;
         case PF_LOOKUP:
	    if (depth==8) {
               ctx->Driver.WriteRGBASpan       = write_span_LOOKUP8_ximage;
               ctx->Driver.WriteRGBSpan        = write_rgb_LOOKUP8_ximage;
               ctx->Driver.WriteMonoRGBASpan   = write_span_mono_LOOKUP8_ximage;
               ctx->Driver.WriteRGBAPixels     = write_pixels_LOOKUP8_ximage;
               ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_LOOKUP8_ximage;
            }
            else {
               ctx->Driver.WriteRGBASpan       = write_span_LOOKUP_ximage;
               ctx->Driver.WriteRGBSpan        = write_span_rgb_LOOKUP_ximage;
               ctx->Driver.WriteMonoRGBASpan   = write_span_mono_ximage;
               ctx->Driver.WriteRGBAPixels     = write_pixels_LOOKUP_ximage;
               ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_ximage;
            }
            break;
         case PF_GRAYSCALE:
	    if (depth==8) {
	       ctx->Driver.WriteRGBASpan       = write_span_GRAYSCALE8_ximage;
	       ctx->Driver.WriteRGBSpan        = write_span_rgb_GRAYSCALE8_ximage;
	       ctx->Driver.WriteMonoRGBASpan   = write_span_mono_GRAYSCALE8_ximage;
	       ctx->Driver.WriteRGBAPixels     = write_pixels_GRAYSCALE8_ximage;
	       ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_GRAYSCALE8_ximage;
	    }
	    else {
	       ctx->Driver.WriteRGBASpan       = write_span_GRAYSCALE_ximage;
	       ctx->Driver.WriteRGBSpan        = write_span_rgb_GRAYSCALE_ximage;
	       ctx->Driver.WriteMonoRGBASpan   = write_span_mono_ximage;
	       ctx->Driver.WriteRGBAPixels     = write_pixels_GRAYSCALE_ximage;
	       ctx->Driver.WriteMonoRGBAPixels = write_pixels_mono_ximage;
	    }
	    break;
	 default:
	    gl_problem(NULL,"Bad pixel format in xmesa_update_state (2)");
            return;
      }
   }

   /* Pixel/span reading functions: */
   ctx->Driver.ReadCI32Span = read_index_span;
   ctx->Driver.ReadRGBASpan = read_color_span;
   ctx->Driver.ReadCI32Pixels = read_index_pixels;
   ctx->Driver.ReadRGBAPixels = read_color_pixels;
}


/*
 * Initialize all the DD.* function pointers depending on the color
 * buffer configuration.  This is mainly called by XMesaMakeCurrent.
 */
void xmesa_update_state( GLcontext *ctx )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   /*int depth=GET_VISUAL_DEPTH(xmesa->xm_visual);*/

   (void) DitherValues;  /* silenced unused var warning */
#ifndef XFree86Server
   (void) drawpixels_8R8G8B;
#endif

   /*
    * Always the same:
    */
   ctx->Driver.GetString = get_string;
   ctx->Driver.UpdateState = xmesa_update_state;
   ctx->Driver.GetBufferSize = get_buffer_size;
   ctx->Driver.Flush = flush;
   ctx->Driver.Finish = finish;
   
   ctx->Driver.RenderStart = 0;
   ctx->Driver.RenderFinish = 0;

   ctx->Driver.SetDrawBuffer = set_draw_buffer;
   ctx->Driver.SetReadBuffer = set_read_buffer;

   ctx->Driver.Index = set_index;
   ctx->Driver.Color = set_color;
   ctx->Driver.ClearIndex = clear_index;
   ctx->Driver.ClearColor = clear_color;
   ctx->Driver.Clear = clear_buffers;
   ctx->Driver.IndexMask = index_mask;
   ctx->Driver.ColorMask = color_mask;
   ctx->Driver.LogicOp = logicop;
   ctx->Driver.Dither = dither;

   ctx->Driver.PointsFunc = xmesa_get_points_func( ctx );
   ctx->Driver.LineFunc = xmesa_get_line_func( ctx );
   ctx->Driver.TriangleFunc = xmesa_get_triangle_func( ctx );

/*     ctx->Driver.TriangleCaps = DD_TRI_CULL; */

   /* setup pointers to front and back buffer clear functions */
   /* XXX this bit of code could be moved to a one-time init */
   xmesa->xm_buffer->front_clear_func = clear_front_pixmap;
   if (xmesa->xm_buffer->backpixmap != XIMAGE) {
      /* back buffer is a pixmap */
      xmesa->xm_buffer->back_clear_func = clear_back_pixmap;
   }
   else if (sizeof(GLushort)!=2 || sizeof(GLuint)!=4) {
      /* Do this on Crays */
      xmesa->xm_buffer->back_clear_func = clear_nbit_ximage;
   }
   else {
      /* Do this on most machines */
      switch (xmesa->xm_visual->BitsPerPixel) {
         case 8:
	    if (xmesa->xm_visual->hpcr_clear_flag) {
               xmesa->xm_buffer->back_clear_func = clear_HPCR_ximage;
            }
            else {
               xmesa->xm_buffer->back_clear_func = clear_8bit_ximage;
            }
            break;
         case 16:
            xmesa->xm_buffer->back_clear_func = clear_16bit_ximage;
            break;
         case 24:
            xmesa->xm_buffer->back_clear_func = clear_24bit_ximage;
            break;
         case 32:
            xmesa->xm_buffer->back_clear_func = clear_32bit_ximage;
            break;
         default:
            xmesa->xm_buffer->back_clear_func = clear_nbit_ximage;
            break;
      }
   }

   update_span_funcs(ctx);
}
