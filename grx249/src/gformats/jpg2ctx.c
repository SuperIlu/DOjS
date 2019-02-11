/**
 ** jpg2ctx.c ---- loads a context from a JPEG file
 **
 ** Copyright (C) 2001 Mariano Alvarez Fernandez
 ** [e-mail: malfer@teleline.es]
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <jpeglib.h>
#include "grx20.h"

/*
** GrJpegSupport - Returns true
*/

int GrJpegSupport( void )
{
  return 1;
}

static int readjpeg( FILE *f, GrContext *grc, int scale );
static int queryjpeg( FILE *f, int *w, int *h );

/*
** GrLoadContextFromJpeg - Load a context from a JPEG file
**
** If context dimensions are lesser than jpeg dimensions,
** the routine loads as much as it can
**
** If color mode is not in RGB mode, the routine allocates as
** much colors as it can
**
** Arguments:
**   grc:      Context to be loaded (NULL -> use current context)
**   jpegfn:   Name of jpeg file
**   scale:    scale the image to 1/scale, actually libjpeg support
**             1, 2, 4 and 8 noly
**
** Returns  0 on success
**         -1 on error
*/

int GrLoadContextFromJpeg( GrContext *grc, char *jpegfn, int scale )
{
  GrContext grcaux;
  FILE *f;
  int r;
  
  f = fopen( jpegfn,"rb" );
  if( f == NULL ) return -1;

  GrSaveContext( &grcaux );
  if( grc != NULL ) GrSetContext( grc );
  r = readjpeg( f,grc,scale );
  GrSetContext( &grcaux );

  fclose( f );

  return r;
}

/*
** GrQueryJpeg - Query width and height data from a JPEG file
**
** Arguments:
**   jpegfn:  Name of jpeg file
**   width:   return pnm width
**   height:  return pnm height
**
** Returns  0 on success
**         -1 on error
*/

int GrQueryJpeg( char *jpegfn, int *width, int *height )
{
  FILE *f;
  int r;
  
  f = fopen( jpegfn,"rb" );
  if( f == NULL ) return -1;

  r = queryjpeg( f,width,height );

  fclose( f );

  return r;
}

/**/

struct my_error_mgr{
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
  };

typedef struct my_error_mgr *my_error_ptr;

/**/

METHODDEF(void) my_error_exit( j_common_ptr cinfo )
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /*(*cinfo->err_>output_message)( cinfo );*/

  longjmp( myerr->setjmp_buffer,1 );
}

/**/

static int readjpeg( FILE *f, GrContext *grc, int scale )
{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  JSAMPARRAY buffer;
  int row_stride;
  int maxwidth, maxheight;
  static GrColor *pColors = NULL;
  unsigned char *pix_ptr;
  int x, y, r, g, b;

  cinfo.err = jpeg_std_error( &jerr.pub );
  jerr.pub.error_exit = my_error_exit;
  if( setjmp( jerr.setjmp_buffer ) ) {
    if( pColors) free( pColors );
    jpeg_destroy_decompress( &cinfo );
    return -1;
  }

  jpeg_create_decompress( &cinfo );
  jpeg_stdio_src( &cinfo,f );
  jpeg_read_header( &cinfo,TRUE );

  cinfo.scale_denom = scale;
  
  jpeg_start_decompress( &cinfo );

  row_stride = cinfo.output_width * cinfo.output_components;

  buffer = (*cinfo.mem->alloc_sarray)
           ( (j_common_ptr)&cinfo,JPOOL_IMAGE,row_stride,1 );

  maxwidth = (cinfo.output_width > GrSizeX()) ?
             GrSizeX() : cinfo.output_width;
  maxheight = (cinfo.output_height > GrSizeY()) ?
             GrSizeY() : cinfo.output_height;
  pColors = malloc( maxwidth * sizeof(GrColor) );
  if( pColors == NULL ) longjmp( jerr.setjmp_buffer,1 );

  for( y=0; y<maxheight; y++ ){
    jpeg_read_scanlines( &cinfo,buffer,1 );
    pix_ptr = buffer[0];
    if( cinfo.output_components == 1 ){
      for( x=0; x<maxwidth; x++ ){
        r = *pix_ptr++;
        pColors[x] = GrAllocColor( r,r,r );
        }
      }
    else{
      for( x=0; x<maxwidth; x++ ){
        r = *pix_ptr++;
        g = *pix_ptr++;
        b = *pix_ptr++;
        pColors[x] = GrAllocColor( r,g,b );
        }
      }
    GrPutScanline( 0,maxwidth-1,y,pColors,GrWRITE );
    }

  jpeg_finish_decompress( &cinfo );
  jpeg_destroy_decompress( &cinfo );
  
  return 0;
}

/**/

static int queryjpeg( FILE *f, int *w, int *h )
{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;

  cinfo.err = jpeg_std_error( &jerr.pub );
  jerr.pub.error_exit = my_error_exit;
  if( setjmp( jerr.setjmp_buffer ) ) {
    jpeg_destroy_decompress( &cinfo );
    return -1;
  }

  jpeg_create_decompress( &cinfo );
  jpeg_stdio_src( &cinfo,f );
  jpeg_read_header( &cinfo,TRUE );

  *w = cinfo.image_width;
  *h = cinfo.image_height;

  jpeg_destroy_decompress( &cinfo );
  
  return 0;
}

