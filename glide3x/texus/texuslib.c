/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/
#include <stdlib.h>
#include <string.h>
#include "texus.h"

#include "texusint.h"

void _txDefaultErrorCallback( const char *string, FxBool fatal );

TxErrorCallbackFnc_t _txErrorCallback = _txDefaultErrorCallback;

void _txDefaultErrorCallback( const char *string, FxBool fatal )
{
  fprintf(stderr, "Texus: %s", string );
  if( fatal )
    exit( -1 );
}

int txBitsPerPixel(GrTextureFormat_t format)
{
    switch ( format ) {
    case GR_TEXFMT_ARGB_CMP_FXT1:
    case GR_TEXFMT_ARGB_CMP_DXT1:
        return 4;
    case GR_TEXFMT_RGB_332:
    case GR_TEXFMT_YIQ_422:
    case GR_TEXFMT_ALPHA_8:
    case GR_TEXFMT_INTENSITY_8:
    case GR_TEXFMT_ALPHA_INTENSITY_44:
    case GR_TEXFMT_P_8:
    case GR_TEXFMT_P_8_6666_EXT:
    case GR_TEXFMT_ARGB_CMP_DXT2:
    case GR_TEXFMT_ARGB_CMP_DXT3:
    case GR_TEXFMT_ARGB_CMP_DXT4:
    case GR_TEXFMT_ARGB_CMP_DXT5:
        return 8;
    case GR_TEXFMT_ARGB_8332:
    case GR_TEXFMT_AYIQ_8422:
    case GR_TEXFMT_RGB_565:
    case GR_TEXFMT_ARGB_1555:
    case GR_TEXFMT_ARGB_4444:
    case GR_TEXFMT_ALPHA_INTENSITY_88:
    case GR_TEXFMT_AP_88:
    case GR_TEXFMT_YUYV_422:
    case GR_TEXFMT_UYVY_422:
        return 16;
    case GR_TEXFMT_RGB_888:
        return 24;
    case GR_TEXFMT_ARGB_8888:
    case GR_TEXFMT_AYUV_444:
        return 32;

    default:
        txPanic( "invalid texel format" );
        return 0;
    }
}

FxU32
txTexCalcMapSize( int x, int y, GrTextureFormat_t format ) 
{
    size_t memRequired =0;
    int bpp = txBitsPerPixel(format);

    switch ( format ) {
    case GR_TEXFMT_ARGB_CMP_FXT1:
        x = (x + 0x7 ) & ~0x7;
        y = (y + 0x3 ) & ~0x3;
        break;
    case GR_TEXFMT_ARGB_CMP_DXT1:
    case GR_TEXFMT_ARGB_CMP_DXT2:
    case GR_TEXFMT_ARGB_CMP_DXT3:
    case GR_TEXFMT_ARGB_CMP_DXT4:
    case GR_TEXFMT_ARGB_CMP_DXT5:
        x = (x + 0x3 ) & ~0x3;
        y = (y + 0x3 ) & ~0x3;
        break;
    case GR_TEXFMT_YUYV_422:
    case GR_TEXFMT_UYVY_422:
        x = (x + 0x1 ) & ~0x1;
        break;
    default:
        break;
    }

    memRequired += (x*y*bpp)>>3;

    return memRequired ;
}

FxU32
txTexCalcMemRequired( GrLOD_t small_lod, GrLOD_t large_lod,
                      GrAspectRatio_t aspect, GrTextureFormat_t format ) {
    size_t memRequired =0;
    FxI32 lod;
    int x, y;

    for( lod = small_lod; lod <= large_lod; lod++ ) {
        if ( aspect >= 0 ) {
            x = ( 1 << lod );
            y = ( 1 << MAX(lod-aspect, 0));
        } else {
            x = ( 1 << MAX(lod+aspect, 0));
            y = ( 1 << lod);
        }

        memRequired += txTexCalcMapSize( x, y, format );
    }
    return memRequired ;
}

static GrLOD_t _txLengthToLOD( int len )
{
  GrLOD_t lod = 0;
  int l = len;

  while ( l > 1 ) {
      l >>= 1;
      lod++;
  }
  if ( ( 1 << lod ) != len )
      lod++;
  return lod;
}

static GrAspectRatio_t _txDimensionsToAspectRatio( int width, int height )
{
  int aspect;
  FxBool sIsLarger;
  int x, y;

  if ( width >= height ) {
      x = width;
      y = height;
      sIsLarger = FXTRUE;
  } else {
      x = height;
      y = width;
      sIsLarger = FXFALSE;
  }

  aspect = 0;
  while ( x > y ) {
    x >>= 1;
    aspect++;
  }

  if (!sIsLarger)
      aspect = -aspect; 

  return aspect;
}

void txNccToPal( FxU32 *pal, const GuNccTable *ncc_table ) 
{
  int i, j;

  for( i = 0; i < 16; i++ )
    {
      pal[i] = ncc_table->yRGB[i];
    }
      
  for( i = 0; i < 4; i++ )
    {
      for( j = 0; j < 3; j++ )
        {
            pal[16 + 3 * i + j] = ncc_table->iRGB[i][j];
            pal[28 + 3 * i + j] = ncc_table->qRGB[i][j];
        }
    }
}

void FX_CSTYLE txPalToNcc( GuNccTable *ncc_table, const FxU32 *pal ) {
  int i, j;

  for( i = 0; i < 16; i++ )
    {
      ncc_table->yRGB[i] = ( FxU8 )pal[i];
    }
      
  for( i = 0; i < 4; i++ )
    {
      for( j = 0; j < 3; j++ )
        {
          ncc_table->iRGB[i][j] = ( FxI16 )( pal[16 + 3 * i + j] );
          ncc_table->qRGB[i][j] = ( FxI16 )( pal[28 + 3 * i + j] );
        }
    }
  
  /*
  ** pack the table Y entries
  */
  for ( i = 0; i < 4; i++ )
    {
      FxU32 packedvalue;
      
      packedvalue  = ( ( FxU32 )( ncc_table->yRGB[i*4+0] & 0xff ) );
      packedvalue |= ( ( FxU32 )( ncc_table->yRGB[i*4+1] & 0xff ) ) << 8;
      packedvalue |= ( ( FxU32 )( ncc_table->yRGB[i*4+2] & 0xff ) ) << 16;
      packedvalue |= ( ( FxU32 )( ncc_table->yRGB[i*4+3] & 0xff ) ) << 24;

      ncc_table->packed_data[i] = packedvalue;
    }
  
  /*
  ** pack the table I entries
  */
  for ( i = 0; i < 4; i++ )
    {
      FxU32 packedvalue;
      
      packedvalue  = ( ( FxU32 )( ncc_table->iRGB[i][0] & 0x1ff ) ) << 18;
      packedvalue |= ( ( FxU32 )( ncc_table->iRGB[i][1] & 0x1ff ) ) << 9;

      packedvalue |= ( ( FxU32 )( ncc_table->iRGB[i][2] & 0x1ff ) ) << 0;
      
      ncc_table->packed_data[i+4] = packedvalue;
    }
  
  /*
  ** pack the table Q entries
  */
  for ( i = 0; i < 4; i++ )
    {
      FxU32 packedvalue;
      
      packedvalue  = ( ( FxU32 )( ncc_table->qRGB[i][0] & 0x1ff ) ) << 18;
      packedvalue |= ( ( FxU32 )( ncc_table->qRGB[i][1] & 0x1ff ) ) << 9;;
      packedvalue |= ( ( FxU32 )( ncc_table->qRGB[i][2] & 0x1ff ) ) << 0;
      
      ncc_table->packed_data[i+8] = packedvalue;
    }
}

size_t txInit3dfInfoFromFile( FILE *file, 
                              Gu3dfInfo *info, GrTextureFormat_t destFormat,
                              int *destWidth, int *destHeight,
                              int mipLevels, FxU32 flags )
{
  int file_start_position;
  size_t size_retval;
  int input_format;
  TxMip txMip;

#if 0
  printf( "enter: txInit3dfInfoFromFile\n" );
  fflush( stdout );
#endif

  /*
   * Save the current position of the input file so that we can
   * later recent it.
   */
  file_start_position = ftell( file );

  /*
   * Read the header of the input file to find out some information
   * about it.
   */
  if( ( input_format = _txReadHeader( file, &txMip ) ) == 0 )
    {
      return 0;
    }

  *destWidth = txMip.width;
  *destHeight = txMip.height;

  size_retval = txInit3dfInfo( info, destFormat, destWidth, destHeight,
                               mipLevels, flags );


  /*
   * Set the file offset back to where it was when we entered this 
   * function.
   */
  fseek( file, file_start_position, SEEK_SET );
  
  /*
   * Return the memory required for this texture.
   */
#if 0
  printf( "txInit3dfInfoFromFile returning %ld\n", ( long )size_retval );
  fflush( stdout );
#endif
  return size_retval;
}

size_t txInit3dfInfo( Gu3dfInfo *info, GrTextureFormat_t destFormat,
                      int *destWidth, int *destHeight,
                      int mipLevels, FxU32 flags )
{
  /*
   * If auto-resize is enabled,
   */
  if( ( flags & TX_AUTORESIZE_MASK ) != TX_AUTORESIZE_DISABLE )
    {
      /*
       * Make sure that the texture dimensions are power of two.
       */
      if( ( flags & TX_AUTORESIZE_MASK ) == TX_AUTORESIZE_SHRINK )
        {
          *destWidth  = txFloorPow2( *destWidth );
          *destHeight = txFloorPow2( *destHeight );
        }
      else
        {
          *destWidth  = txCeilPow2( *destWidth );
          *destHeight = txCeilPow2( *destHeight );
        }
      
      /*
       * Make sure the dimensions are in range.
       */

      while( *destWidth > 2048 )
        *destWidth >>= 1;
      while( *destHeight > 2048 )
        *destHeight >>= 1;

#ifdef notdef
      /*
       * Make sure the aspect ratio is valid.
       */
      while( ( *destWidth / *destHeight ) > 8 )
        {
          *destWidth >>= 1;
        }

      while( ( *destHeight/ *destWidth ) > 8 )
        {
          *destHeight >>= 1;
        }
#endif
    }

  /*
   * Calculate the aspect ratio of the texture . . this assumse that
   * the desired texture size is valid!
   */
  info->header.aspect_ratio = _txDimensionsToAspectRatio( *destWidth, *destHeight );

  /*
   * Calculate the min and max LODs for the texture.
   */
  info->header.large_lod = _txLengthToLOD( ( *destHeight > *destWidth ) ? *destHeight : *destWidth );
  info->header.small_lod = 0;

  /*
   * Make sure that we have the correct number of mipmap levels given
   * the parameter "mipLevels".
   * If mipLevels is -1, then we want all levels.
   */
  if( mipLevels != -1 )
    {
      int tmpNumLevels;
      tmpNumLevels = info->header.large_lod - info->header.small_lod + 1;
      if( tmpNumLevels > mipLevels ) {
          info->header.small_lod += tmpNumLevels - mipLevels;
      }
    }

  /*
   * Store the geometry of the texture.
   */
  info->header.width    = *destWidth;
  info->header.height   = *destHeight;

  /*
   * Store the format of the texture.
   */
  info->header.format   = destFormat;
  
  info->mem_required = txTexCalcMemRequired( info->header.small_lod,
                                             info->header.large_lod,
                                             info->header.aspect_ratio,
                                             info->header.format );

  /*
   * Return the amount of texture memory required for this texture.
   * The user is responsible for allocating the space for the texture.
   */
  return info->mem_required;
}


FxBool txConvertFromFile( FILE *file, Gu3dfInfo *info, 
                          FxU32 flags, const void *palNcc )
{
  int file_start_position;
  FxBool retval;
  TxMip txMip;
  
  /*
   * Save the current position of the input file so that we can
   * later recent it.
   */
  file_start_position = ftell( file );

  txMipReadFromFP( &txMip, "(FILE*)", file, GR_TEXFMT_ANY );

  retval = txConvert( info, txMip.format, txMip.width, txMip.height,
                      txMip.data[0], flags, palNcc );
  txFree( txMip.data[0] );

  return retval;
}

FxBool txConvert( Gu3dfInfo *info, GrTextureFormat_t srcFormat,
                  int srcWidth, int srcHeight,
                  const void *srcImage, FxU32 flags,
                  const void *palNcc )
{
  TxMip srcMip;
  TxMip trueColorMip;
  TxMip outputMip;
  TxMip tmpMip;

  /*
   * Make a txMip out of the passed data.
   */
  memset( &srcMip, 0, sizeof( srcMip ) );
  srcMip.format         = srcFormat;
  srcMip.width          = srcWidth;
  srcMip.height         = srcHeight;
  srcMip.depth          = 1;

  if( palNcc )
    {
      switch( srcFormat )
        {
        case GR_TEXFMT_YIQ_422:
        case GR_TEXFMT_AYIQ_8422:
         txNccToPal( srcMip.pal, palNcc);
          break;
        case GR_TEXFMT_P_8:
        case GR_TEXFMT_AP_88:
        case GR_TEXFMT_P_8_6666_EXT:
          memcpy( srcMip.pal, palNcc, sizeof( FxU32 ) * 256 );
          break;
        }
    }
  srcMip.data[0]        = ( void * )srcImage;

  /*
   * Set up a txMip to put a true color version of the texture in.
   */
  memset( &trueColorMip, 0, sizeof( trueColorMip ) );
  trueColorMip.format         = GR_TEXFMT_ARGB_8888;
  trueColorMip.width          = srcWidth;
  trueColorMip.height         = srcHeight;
  /*
   * Set the depth to the mipmapped depth to allocate the image.
   */
  trueColorMip.depth          = info->header.large_lod - info->header.small_lod + 1;
  if( !txMipAlloc( &trueColorMip ) )
    return FXFALSE;

  /*
   * Set to one level only since we only want to dequant the first 
   * level.
   */
  trueColorMip.depth          = 1;

  /*
   * Convert from the input format to truecolor.
   */
  txMipDequantize( &trueColorMip, &srcMip );

  /*
   * We realy have more than one level, so. . . 
   */
  trueColorMip.depth          = info->header.large_lod - info->header.small_lod + 1;

  /*
   * WARNING!  I do not free srcMip.data[0] since it is passed in by the users.
   */

  /*
   * Resample the true color version of the input image to 
   * the passed in size. . . . this should be a valid
   * size for the hardware to handle.
   */
  tmpMip = trueColorMip;
  tmpMip.width = info->header.width;
  tmpMip.height = info->header.height;
  txMipAlloc( &tmpMip );

  if( ( flags & TX_CLAMP_MASK ) == TX_CLAMP_DISABLE )
    {
      txMipResample( &tmpMip, &trueColorMip );
    }
  else
    {
      txMipClamp( &tmpMip, &trueColorMip );
#if 0
      txMipView( &tmpMip, "blah", FXTRUE, 0 );
#endif
    }
    
  
#if 0
  if( _heapchk() != _HEAPOK )
    txPanic( "_heapchk failed" );
#endif
  txFree( trueColorMip.data[0] );

  trueColorMip = tmpMip;

  /*
   * Generate mipmap levels.
   */
  trueColorMip.depth          = info->header.large_lod - info->header.small_lod + 1;
  txMipMipmap( &trueColorMip );

#if 0
  txMipView( &trueColorMip, "blah", FXTRUE, 0 );
#endif

  /*
   * Convert from true color to the output color format.
   */
  memset( &outputMip, 0, sizeof( outputMip ) );
  outputMip.format         = info->header.format;
  outputMip.width          = info->header.width;
  outputMip.height         = info->header.height;
  outputMip.depth          = trueColorMip.depth;
  outputMip.data[0]        = info->data;
#if 0
  txMipAlloc( &outputMip );
#else
  txMipSetMipPointers( &outputMip );
#endif
  
  if( ( flags & TX_TARGET_PALNCC_MASK ) == TX_TARGET_PALNCC_SOURCE )
    {
      txMipTrueToFixedPal( &outputMip, &trueColorMip, palNcc, 
                           flags & TX_FIXED_PAL_QUANT_MASK );
    }
  else
    {
      txMipQuantize( &outputMip, &trueColorMip, outputMip.format, 
                     flags & TX_DITHER_MASK, flags & TX_COMPRESSION_MASK );
    }

  info->data = outputMip.data[0];

  if( ( info->header.format == GR_TEXFMT_YIQ_422 ) ||
      ( info->header.format == GR_TEXFMT_AYIQ_8422 ) )
    {
      txPalToNcc( &info->table.nccTable, outputMip.pal );
    }
  
  if ( info->header.format == GR_TEXFMT_P_8   || 
       info->header.format == GR_TEXFMT_AP_88 ||
       info->header.format == GR_TEXFMT_P_8_6666_EXT )
    {
      memcpy( info->table.palette.data, outputMip.pal, sizeof( FxU32 ) * 256 );
    }

  txFree( trueColorMip.data[0] );

  return FXTRUE;
}


