/**
 ** png2ctx.c ---- loads a context from a PNG file
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

/*
** GrPngSupport - Returns true
*/

int GrPngSupport( void )
{
  return 1;
}

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

static int readpng( FILE *f, GrContext *grc, int use_alpha );
static int querypng( FILE *f, int *w, int *h );

/*
** GrLoadContextFromPng - Load a context from a PNG file
**
** If context dimensions are lesser than png dimensions,
** the routine loads as much as it can
**
** If color mode is not in RGB mode, the routine allocates as
** much colors as it can
**
** Arguments:
**   grc:      Context to be loaded (NULL -> use current context)
**   pngfn:    Name of pnm file
**   use_alpha: if true, use alpha channel if available
**
** Returns  0 on success
**         -1 on error
*/

int GrLoadContextFromPng( GrContext *grc, char *pngfn, int use_alpha )
{
  GrContext grcaux;
  FILE *f;
  int r;
  
  f = fopen( pngfn,"rb" );
  if( f == NULL ) return -1;

  GrSaveContext( &grcaux );
  if( grc != NULL ) GrSetContext( grc );
  r = readpng( f,grc,use_alpha );
  GrSetContext( &grcaux );

  fclose( f );

  return r;
}

/*
** GrQueryPnm - Query width and height data from a PNG file
**
** Arguments:
**   pngfn:   Name of png file
**   width:   return pnm width
**   height:  return pnm height
**
** Returns  0 on success
**         -1 on error
*/

int GrQueryPng( char *pngfn, int *width, int *height )
{
  FILE *f;
  int r;
  
  f = fopen( pngfn,"rb" );
  if( f == NULL ) return -1;

  r = querypng( f,width,height );

  fclose( f );

  return r;
}

/**/

static int readpng( FILE *f, GrContext *grc, int use_alpha )
{
  png_struct *png_ptr = NULL;
  png_info *info_ptr = NULL;
  png_byte buf[8];
  png_byte *png_pixels = NULL;
  png_byte **row_pointers = NULL;
  png_byte *pix_ptr = NULL;
  png_uint_32 row_bytes;
  png_uint_32 width;
  png_uint_32 height;
  int bit_depth;
  int color_type;
  int alpha_present;
  int i, x, y, r, g, b;
  int alpha = 0, ro, go, bo;
  int maxwidth, maxheight;
  GrColor *pColors = NULL;

  /* is it a PNG file? */
  if( fread( buf,1,8,f ) != 8 ) return -1;
  if( ! png_check_sig( buf,8 ) ) return -1;

  png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
  if( !png_ptr ){
    return -1;
    }

  info_ptr = png_create_info_struct( png_ptr );
  if( !info_ptr ){
    png_destroy_read_struct( &png_ptr,NULL,NULL );
    return -1;
    }

  if( setjmp( png_jmpbuf(png_ptr) ) ){
    png_destroy_read_struct( &png_ptr,&info_ptr,NULL );
    return -1;
    }

  png_init_io( png_ptr,f );
  png_set_sig_bytes( png_ptr,8 );
  png_read_info( png_ptr,info_ptr );

  png_get_IHDR( png_ptr,info_ptr,&width,&height,&bit_depth,
                &color_type,NULL,NULL,NULL);

  /* tell libpng to strip 16 bit/color files down to 8 bits/color */
  if( bit_depth == 16 )
    png_set_strip_16( png_ptr );
  /* expand paletted colors into true RGB triplets */
  if( color_type == PNG_COLOR_TYPE_PALETTE )
    png_set_expand( png_ptr );
  /* expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
  if( color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8 )
    png_set_expand( png_ptr );
  /* expand paletted or RGB images with transparency to full alpha channels
     so the data will be available as RGBA quartets. */
  if( png_get_valid( png_ptr,info_ptr,PNG_INFO_tRNS ) )
    png_set_expand( png_ptr );
  /* transform grayscale images into rgb */
  if( color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
    png_set_gray_to_rgb( png_ptr );

  /* we don't do gamma correction by now */

  png_read_update_info( png_ptr,info_ptr );
  png_get_IHDR( png_ptr,info_ptr,&width,&height,&bit_depth,
                &color_type,NULL,NULL,NULL);

  if( color_type == PNG_COLOR_TYPE_RGB )
    alpha_present = 0;
  else if( color_type == PNG_COLOR_TYPE_RGB_ALPHA )
    alpha_present = 1;
  else{
    png_destroy_read_struct( &png_ptr,&info_ptr,NULL );
    return -1;
    }

  row_bytes = png_get_rowbytes( png_ptr,info_ptr );

  png_pixels = (png_byte *) malloc( row_bytes * height * sizeof(png_byte) );
  if( png_pixels == NULL ){
    png_destroy_read_struct( &png_ptr,&info_ptr,NULL );
    return -1;
    }

  row_pointers = (png_byte **) malloc( height * sizeof(png_bytep) );
  if( row_pointers == NULL ){
    png_destroy_read_struct( &png_ptr,&info_ptr,NULL );
    free( png_pixels );
    png_pixels = NULL;
    return -1;
    }

  for( i=0; i<height; i++ )
    row_pointers[i] = png_pixels + i * row_bytes;

  png_read_image (png_ptr, row_pointers);

  png_read_end (png_ptr, info_ptr);

  png_destroy_read_struct( &png_ptr,&info_ptr,NULL );

  /* write data to context */
  maxwidth = (width > GrSizeX()) ? GrSizeX() : width;
  maxheight = (height > GrSizeY()) ? GrSizeY() : height;

  pColors = malloc( maxwidth * sizeof(GrColor) );
  if( pColors == NULL ){
    free( row_pointers );
    row_pointers = NULL;
    free( png_pixels );
    png_pixels = NULL;
    return -1;
    }

  for( y=0; y<maxheight; y++ ){
    pix_ptr = row_pointers[y];
    if( alpha_present && use_alpha ){
      memcpy( pColors,GrGetScanline( 0,maxwidth-1,y ),
              sizeof(GrColor)*maxwidth );
      }
    for( x=0; x<width; x++ ){
      r = *pix_ptr++;
      g = *pix_ptr++;
      b = *pix_ptr++;
      alpha = 0;
      if( alpha_present )
        alpha = *pix_ptr++;
      if( x < maxwidth ){
        if( alpha_present && use_alpha ){
          GrQueryColor( pColors[x],&ro,&go,&bo );
          r = r * (alpha/255.0) + ro * ((255-alpha)/255.0);
          g = g * (alpha/255.0) + go * ((255-alpha)/255.0);
          b = b * (alpha/255.0) + bo * ((255-alpha)/255.0);
          }
        pColors[x] = GrAllocColor( r,g,b );
        }
      }
    GrPutScanline( 0,maxwidth-1,y,pColors,GrWRITE );
    }

  if( pColors) free( pColors );
  if( row_pointers ) free( row_pointers );
  if( png_pixels ) free( png_pixels );

  return 0;
}

/**/

static int querypng( FILE *f, int *w, int *h )
{
  png_struct *png_ptr = NULL;
  png_info *info_ptr = NULL;
  png_byte buf[8];
  png_uint_32 width;
  png_uint_32 height;
  int bit_depth;
  int color_type;

  /* is it a PNG file? */
  if( fread( buf,1,8,f ) != 8 ) return -1;
  if( ! png_check_sig( buf,8 ) ) return -1;

  png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
  if( !png_ptr ) return -1;

  info_ptr = png_create_info_struct( png_ptr );
  if( !info_ptr ){
    png_destroy_read_struct( &png_ptr,NULL,NULL );
    return -1;
    }

  if( setjmp( png_jmpbuf(png_ptr) ) ){
    png_destroy_read_struct( &png_ptr,&info_ptr,NULL );
    return -1;
    }

  png_init_io( png_ptr,f );
  png_set_sig_bytes( png_ptr,8 );
  png_read_info( png_ptr,info_ptr );

  png_get_IHDR( png_ptr,info_ptr,&width,&height,&bit_depth,
                &color_type,NULL,NULL,NULL);
  *w = width;
  *h = height;

  png_destroy_read_struct( &png_ptr,&info_ptr,NULL );
  return 0;
}

