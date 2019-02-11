/**
 ** ctx2png.c ---- saves a context to a PNG file
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
#include <png.h>
#include "grx20.h"

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

static int writepng( FILE *f, GrContext *grc );

/*
** GrSaveContextToPng - Dump a context in a PNG file
**
** This routine works both in RGB and palette modes
**
** Arguments:
**   grc:   Context to be saved (NULL -> use current context)
**   pngfn: Name of png file
**
** Returns  0 on success
**         -1 on error
*/

int GrSaveContextToPng( GrContext *grc, char *pngfn )
{
  GrContext grcaux;
  FILE *f;
  int r;
  
  f = fopen( pngfn,"wb" );
  if( f == NULL ) return -1;

  GrSaveContext( &grcaux );
  if( grc != NULL ) GrSetContext( grc );
  r = writepng( f,grc );
  GrSetContext( &grcaux );

  fclose( f );

  return r;
}

/**/

static int writepng( FILE *f, GrContext *grc )
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_uint_32 height;
  png_uint_32 width;
  png_byte *png_pixels = NULL;
  png_byte **row_pointers = NULL;
  png_byte *pix_ptr = NULL;
  int x, y, r, g, b;
  GrColor *pColors = NULL;

  /* Create and initialize the png_struct */
  png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING,NULL,NULL,NULL );
  if( png_ptr == NULL )
    return -1;

  /* Allocate/initialize the image information data */
  info_ptr = png_create_info_struct( png_ptr );
  if( info_ptr == NULL ){
    png_destroy_write_struct( &png_ptr,(png_infopp)NULL );
    return -1;
    }

  /* Set error handling */
  if( setjmp( png_jmpbuf(png_ptr) ) ){
    /* If we get here, we had a problem reading the file */
    png_destroy_write_struct( &png_ptr,&info_ptr );
    return -1;
    }

  /* set up the output control we are using standard C streams */
  png_init_io( png_ptr,f );

  /* Set the image information  */
  width = GrSizeX();
  height = GrSizeY();
  png_set_IHDR( png_ptr,info_ptr,width,height,8,PNG_COLOR_TYPE_RGB,
                PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_BASE,
                PNG_FILTER_TYPE_BASE );

  /* No gamma correction by now */
  /* png_set_gAMA(png_ptr, info_ptr, gamma); */

  /* No comments by now */
  /* text_ptr[0].key = "Title";
   text_ptr[0].text = "Mona Lisa";
   text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
   text_ptr[1].key = "Author";
   text_ptr[1].text = "Leonardo DaVinci";
   text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
   text_ptr[2].key = "Description";
   text_ptr[2].text = "<long text>";
   text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;
   png_set_text(png_ptr, info_ptr, text_ptr, 3); */

  /* Write the file header information */
  png_write_info( png_ptr,info_ptr );

  png_pixels = (png_byte *) malloc( width * 3 * sizeof(png_byte) );
  if( png_pixels == NULL ){
    png_destroy_write_struct( &png_ptr,&info_ptr );
    return -1;
    }

  row_pointers = (png_byte **) malloc( 1 * sizeof(png_bytep) );
  if( row_pointers == NULL ){
    png_destroy_write_struct( &png_ptr,&info_ptr );
    free( png_pixels );
    return -1;
    }

  pColors = malloc( width * sizeof(GrColor) );
  if( pColors == NULL ){
    png_destroy_write_struct( &png_ptr,&info_ptr );
    free( row_pointers );
    free( png_pixels );
    return -1;
    }

  row_pointers[0] = png_pixels;

  for( y=0; y<height; y++ ){
    pix_ptr = png_pixels;
    memcpy( pColors,GrGetScanline( 0,width-1,y ),sizeof(GrColor)*width );
    for( x=0; x<width; x++ ){
      GrQueryColor( pColors[x],&r,&g,&b );
      *pix_ptr++ = r;
      *pix_ptr++ = g;
      *pix_ptr++ = b;
      }
    png_write_rows( png_ptr,&row_pointers[0],1 );
    }

  /* It is REQUIRED to call this to finish writing the rest of the file */
  png_write_end( png_ptr,info_ptr );

  /* clean up after the write, and free any memory allocated */
  png_destroy_write_struct( &png_ptr,&info_ptr );
  free( pColors );
  free( row_pointers );
  free( png_pixels );

  return 0;
}

