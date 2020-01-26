/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "texusint.h"

/*
 * The only two file formats we can write are: 3df and tga.
 */
static  char *Version = "1.1";
static char* aspect_names[] = { "8 1", "4 1", "2 1", "1 1", 
                                "1 2", "1 4", "1 8" };

/*************************************** tga files ****************************/
/* 
 * Write a tga file from an ARGB8888 mipmap.
 */
static FxBool 
txWriteTGA( FILE *stream, TxMip *txMip)
{
  
  struct {
    FxU8 IDLength;
    FxU8 ColorMapType;
    FxU8 ImgType;
    FxU8 CMapStartLo;
    FxU8 CMapStartHi;
    FxU8 CMapLengthLo;
    FxU8 CMapLengthHi;
    FxU8 CMapDepth;
    FxU8 XOffSetLo;
    FxU8 XOffSetHi;
    FxU8 YOffSetLo;
    FxU8 YOffSetHi;
    FxU8 WidthLo;
    FxU8 WidthHi;
    FxU8 HeightLo;
    FxU8 HeightHi;
    FxU8 PixelDepth;
    FxU8 ImageDescriptor;
  } tgaHeader;
  
  FxU8        *data, *p;
  FxU32       *data32;
  int         outW, outH, w, h, i;
  
  if (txMip->format != GR_TEXFMT_ARGB_8888) {
    txPanic("TGA Image: Write: input format must be ARGB8888.");
  }
  if ( stream == NULL ) {
    txPanic("Bad file handle");
    return FXFALSE;
  }
  
  outW = txMip->width;
  outH = txMip->height;
  if (txMip->depth > 1) outW += outW/2;
  
  tgaHeader.IDLength         = 0;
  tgaHeader.ColorMapType     = 0;
  tgaHeader.ImgType          = 0x2;
  tgaHeader.CMapStartLo      = 0;
  tgaHeader.CMapStartHi      = 0;
  tgaHeader.CMapLengthLo     = 0;
  tgaHeader.CMapLengthHi     = 0;
  tgaHeader.CMapDepth        = 0;
  tgaHeader.XOffSetLo        = 0;
  tgaHeader.XOffSetHi        = 0;
  tgaHeader.YOffSetLo        = 0;
  tgaHeader.YOffSetHi        = 0;
  tgaHeader.WidthHi          = (FxU8)((outW >> 8) & 0xFF);
  tgaHeader.WidthLo          = (FxU8) (outW & 0xFF);
  tgaHeader.HeightHi         = (FxU8)((outH >> 8) & 0xFF);
  tgaHeader.HeightLo         = (FxU8) (outH & 0xFF);
  tgaHeader.PixelDepth       = 32;
  tgaHeader.ImageDescriptor  = 0x20;          // image always right side up.
  
  
  if ( fwrite( &tgaHeader, 1, 18, stream ) != 18 ) {
    txPanic("TGA Header stream write error");
    return FXFALSE;
  }
  
  /*
   * Allocate memory to hold all mipmaps, and copy the mipmaps.
   */
  p = data = txMalloc(outW * outH * 4);
  memset(data, 0, outW * outH * 4);
  
  /* Copy level 0 into malloc'd area */
  txRectCopy( data, outW * 4, txMip->data[0], txMip->width * 4, 
              txMip->width * 4, txMip->height);
  
  p += (txMip->width * 4);
  
  /* Copy the rest of the levels to the right of level 0 */
  w = txMip->width; 
  h = txMip->height; 
  for (i=1; i< txMip->depth; i++) {
    // printf("Copying: level = %d\n", i);
    if (w > 1) w >>= 1;
    if (h > 1) h >>= 1;
    txRectCopy(p, outW * 4, txMip->data[i], w * 4, w * 4, h);
    p += ( h * outW * 4);
  }
  
  /* Write out the data */
  data32 = (FxU32 *) data;
  for (i=outW*outH; i; i--) {
    putc(((*data32      ) & 0xff)    , stream);     
    putc(((*data32 >>  8) & 0xff)    , stream);     
    putc(((*data32 >> 16) & 0xff)    , stream);     
    putc(((*data32 >> 24) & 0xff)    , stream);     
    data32++;
  }
  return FXTRUE;
}


/*************************************** 3df files ****************************/
/* Write word, msb first */

static FxBool 
_txWrite16 (FILE *stream, FxU16 data)
{
  FxU8 byte[2];
  
  byte[0] = (FxU8) ((data >> 8) & 0xFF);
  byte[1] = (FxU8) ((data     ) & 0xFF);
  
  return (fwrite (byte, 2, 1, stream) != 1) ? FXFALSE : FXTRUE;
}

/* Write long word, msb first */

static FxBool 
_txWrite32 (FILE *stream, FxU32 data)
{
  FxU8 byte[4];
  
  byte[0] = (FxU8) ((data >> 24) & 0xFF);
  byte[1] = (FxU8) ((data >> 16) & 0xFF);
  byte[2] = (FxU8) ((data >>  8) & 0xFF);
  byte[3] = (FxU8) ((data      ) & 0xFF);
  
  return (fwrite (byte, 4, 1, stream) != 1) ? FXFALSE : FXTRUE;
}

/* Write NCC table */
static FxBool 
_txWrite3dfNCCTable (FILE *stream, FxU32 *yab)
{
  int         i;
  
  for (i = 0; i < 16; i++)
    if (!_txWrite16 (stream, (FxU16) (yab[i] & 0x00ff))) return FXFALSE;
  
  for (i = 0; i < 12; i++) 
    if (!_txWrite16 (stream, (FxU16) (yab[16+i] & 0xffff))) return FXFALSE;
  
  for (i = 0; i < 12; i++) 
    if (!_txWrite16 (stream, (FxU16) (yab[28+i] & 0xffff))) return FXFALSE;
  
  return FXTRUE;
}

static FxBool
_txWrite3dfPalTable (FILE *stream, FxU32 *pal)
{
  int     i;
  
  for (i=0; i<256; i++) {
    if (!_txWrite32 (stream, pal[i])) return FXFALSE;
  }
  return FXTRUE;
}

static FxBool 
txWrite3df (FILE *stream, TxMip *txMip)
{
  FxU32       i;
  FxU32       n_pixels;
  int         small_lod, large_lod, aspect;
  
  /* Write out the header */
  large_lod = (txMip->width > txMip->height) ? txMip->width : txMip->height;
  small_lod = large_lod >> (txMip->depth - 1);
  aspect    = txAspectRatio(txMip->width, txMip->height);
  
  // printf("Format = %d\n", txMip->format);
  // printf("Format = %s\n", Format_Name[txMip->format]);
  
  // printf("Writing header...\n");
  if (EOF == fprintf (stream, 
                      "3df v%s\n%s\nlod range: %d %d\naspect ratio: %s\n",
                      Version, 
                      Format_Name[txMip->format],
                      small_lod,
                      large_lod,
                      aspect_names[aspect])) return FXFALSE;
  
  /* write out ncc table if necessary */
  // printf("Writing NCC...\n");
  if ((txMip->format == GR_TEXFMT_YIQ_422) ||
      (txMip->format == GR_TEXFMT_AYIQ_8422)) {
    if (!_txWrite3dfNCCTable (stream, txMip->pal)) return FXFALSE;
  } 
  
  else if ((txMip->format == GR_TEXFMT_P_8) ||
           (txMip->format == GR_TEXFMT_AP_88)) {
    if (!_txWrite3dfPalTable (stream, txMip->pal)) return FXFALSE;
  } 
  
  
  /* write out mipmap image data */
  // printf("Writing mipmaps (%d bytes)...\n", txMip->size);
  if (txMip->format < GR_TEXFMT_16BIT) {
    n_pixels = txMip->size;
    if (n_pixels != fwrite (txMip->data[0], 1, n_pixels, stream)) {
      printf("Bad Bad Bad!\n");
      return FXFALSE;
    }
  }
  else if (txMip->format < GR_TEXFMT_32BIT) {
    FxU16* data = (FxU16 *) txMip->data[0];
    n_pixels = txMip->size >> 1;
    
    for (i = 0; i < n_pixels; i ++)
      if (FXFALSE == _txWrite16 (stream, *data++)) return FXFALSE;
  }
  else {
    FxU32* data = (FxU32*) txMip->data[0];
    n_pixels = txMip->size >> 2;
    
    for (i = 0; i < n_pixels; i ++)
      if (FXFALSE == _txWrite32 (stream, *data++)) return FXFALSE;
  }
  return FXTRUE;
}

void
txMipWrite(TxMip *txMip, char *file, char *ext, int split)
{
  FILE        *stream;
  char        filename[128];
  int         i, w, h;
  TxMip       splitImg;
  int                   filetype = 0;
  
  if ((txMip->width  & (txMip->width  - 1)) ||
      (txMip->height & (txMip->height - 1))) {
    txPanic("txMipWrite: size not power of 2!");
  }
  /*
    if (strcmp(ext, ".tga") && strcmp(ext, ".3df") && strcmp(ext, ".txs")) {
    txPanic("txMipWrite: Bad output format");
    }
    */
  if((strcmp(ext, ".txs") == 0))
    filetype = TX_WRITE_TXS;
  else if((strcmp(ext, ".3df")) == 0)
    filetype = TX_WRITE_3DF;
  else if((strcmp(ext, ".tga")) == 0)
    filetype = TX_WRITE_TGA;
  
  switch (filetype)
    {
    case TX_WRITE_3DF:
      if ((txMip->width  & (txMip->width - 1)) ||
          (txMip->height & (txMip->height - 1)))
        {
          txPanic("txMipWrite: 3DF size not power of 2!");
        }
      
      if ((txMip->width > MAX_TEXWIDTH_3DF) ||
          (txMip->height > MAX_TEXWIDTH_3DF))
        {
          txPanic("txMipWrite: 3DF size greater than 256");
        }
      
      if (txMip->format > GR_TEXFMT_ARGB_8888)
        {
          txPanic("txMipWrite: Invalid format for 3DF");
        }
      break;
      
    case TX_WRITE_TGA:
      if (txMip->format != GR_TEXFMT_ARGB_8888)
        {
          txPanic("txMipWrite: TGA format must be ARGB_8888");
        }
      break;
      
    case TX_WRITE_TXS:
      break;
      
    default:
      txPanic("txMipWrite: Bad output format");
      break;
    }
  
  /* If not split, write out a single file */
  if (!split) {
    strcpy(filename, file);
    strcat(filename, ext);
    if( txVerbose )
      {
        printf("Writing file \"%s\" (format: %s)\n", 
               filename, Format_Name[txMip->format]);
      }
    stream = fopen(filename, "wb");
    if (stream == NULL) {
      txPanic("Unable to open output file.");
    }
    switch (filetype)
      {
      case TX_WRITE_3DF:
        if (!txWrite3df( stream, txMip ))
          {
            txPanic("txMipWrite: 3DF Write failed.");
          }
        break;
        
      case TX_WRITE_TGA:
        if (!txWriteTGA( stream, txMip ))
          {
            txPanic("txMipWrite: TGA Write failed.");
          }
        break;
        
      case TX_WRITE_TXS:
        if (!txWriteTXS( stream, txMip ))
          {
            txPanic("txMipWrite: TXS Write failed.");
          }
        break;
        
      default:
        fclose(stream);
        txPanic("txMipWrite: Bad output format");
        break;
      }
    fclose(stream);
    return;
  }
  
  /* Otherwise, we need to write out separate output files */
  w = txMip->width;
  h = txMip->height;
  
  for (i=0; i<txMip->depth; i++) {
    char    suffix[2];
    
    splitImg = *txMip;              // Copy everything first, including palette.
    
    // Then change stuff.
    splitImg.format = txMip->format;
    splitImg.width  = w;
    splitImg.height = h;
    splitImg.size   = ((w * h * GR_TEXFMT_SIZE( txMip->format ))>>3);
    splitImg.depth  = 1;
    splitImg.data[0] = txMip->data[i];
    
    // manufacture a new name.
    suffix[0] = '0' + i;
    suffix[1] = 0;
    strcpy(filename, file);
    strcat(filename, suffix);
    strcat(filename, ext);
    stream = fopen(filename, "wb");
    if (stream == NULL) {
      txPanic("Unable to open output file.");
    }
    switch (filetype)
      {
      case TX_WRITE_3DF:
        if (!txWrite3df( stream, &splitImg ))
          {
            txPanic("txMipWrite: 3DF Write failed.");
          }
        break;
        
      case TX_WRITE_TGA:
        if (!txWriteTGA( stream, &splitImg ))
          {
            txPanic("txMipWrite: TGA Write failed.");
          }
        break;
        
      case TX_WRITE_TXS:
        if (!txWriteTXS( stream, &splitImg ))
          {
            txPanic("txMipWrite: TXS Write failed.");
          }
        break;
        
      default:
        fclose(stream);
        txPanic("txMipWrite: Bad output format");
        break;
      }
    fclose(stream);
    if (w > 1) w >>= 1;
    if (h > 1) h >>= 1;
  }
}

FxBool txWrite( Gu3dfInfo *info, FILE *fp, FxU32 flags )
{
  TxMip mip;
  
  mip.format    = info->header.format;
  mip.width     = info->header.width;
  mip.height    = info->header.height;
#ifdef GLIDE3
  mip.depth     = info->header.large_lod - info->header.small_lod + 1;
#else
  mip.depth     = info->header.small_lod - info->header.large_lod + 1;
#endif
  mip.size      = info->mem_required;
  mip.data[0]   = info->data;
  if( mip.format == GR_TEXFMT_P_8 || mip.format == GR_TEXFMT_AP_88 )
    {
      memcpy( mip.pal, info->table.palette.data, sizeof( FxU32 ) * 256 );
    }
  if( mip.format == GR_TEXFMT_YIQ_422 || mip.format == GR_TEXFMT_AYIQ_8422 )
    {
      txNccToPal( mip.pal, &info->table.nccTable);
    }
  switch( flags & TX_WRITE_MASK )
    {
    case TX_WRITE_3DF:
      if( !txWrite3df( fp, &mip ) )
        return FXFALSE;
      break;
    case TX_WRITE_TGA:
      if( mip.format == GR_TEXFMT_YIQ_422 || 
          mip.format == GR_TEXFMT_AYIQ_8422 )
        {
          txPanic( "Don't know how to write NCC textures\n" );
        }
      if( !txWriteTGA( fp, &mip ) )
        return FXFALSE;
      break;
    case TX_WRITE_TXS:
      if( !txWriteTXS( fp, &mip ) )
        return FXFALSE;
      break;
    default:
      txPanic( "Unknown texture write format" );
      break;
    }
  return FXTRUE;
}
