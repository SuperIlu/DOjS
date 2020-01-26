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
#include <string.h>
#include <memory.h>
#include "texusint.h"
#include "sst2fxt1.h"

/*************************************** TXS files ****************************/



/*************************************** Read/Write Procs ****************************/

static FxBool _txsRead16 (FILE *stream, FxU16* data);
static FxBool _txsRead32 (FILE *stream, FxU32* data);
static FxBool _readTXSPalTable (FILE* stream, FxI32* pal);
static FxBool _txsWrite16 (FILE *stream, FxU16 data);
static FxBool _txsWrite32 (FILE *stream, FxU32 data);
static FxBool _writeTXSPalTable (FILE *stream, FxU32 *pal);

// General procedure to read a TXS header.  The output_info parameter control whether the header
// information is output.

FxBool readTXSHeader(FILE *stream, TXSInfo *info, FxBool output_info)
{
  int   fileVer, curVer;
  char  cookie[TXS_COOKIE_SIZE];
  FxU32 data_offset;
  
  // Go to the start of the file
  
  if ( fseek( stream, 0, SEEK_SET ) )
    {
      return FXFALSE;
    }
  
  // Read the full header
  
  if ( fscanf ( stream, "%4s %f %hu %hu %hu %hu %8x", cookie, &info->version,
                &info->format, &info->width, &info->height, &info->mipmap_levels,
                &data_offset ) != TXS_READ_NUM_FIELDS )
    {
      return FXFALSE;
    }
  
  // Just output the header info if this is what we've been asked to do
  
  if ( output_info )
    {
      printf("cookie = %s\nversion = %f\nformat = %s (%d)\nwidth = %d\nheight = %d\n"
             "mipmap_levels = %d\ndata_offset = %#010x\n\n",
             cookie, info->version, Format_Name[info->format], info->format,
             info->width, info->height, info->mipmap_levels, data_offset);
      
      return FXTRUE;
    }
  
  // If the cookie doesn't match, then reject the file
  
  if ( strcmp (cookie, TXS_COOKIE) )
    {
      return FXFALSE;
    }
  
  // If the major version has increased, then we can't be sure we can load this
  // file so reject it.
  
  fileVer = (int) info->version;
  curVer  = (int) TXS_VERSION;
  
  if ( fileVer > curVer )
    {
      return FXFALSE;
    }
  
  // Make sure we have a known format.
  
  if (info->format > GR_TEXFMT_LAST)
    {
      return FXFALSE;
    }
  
  // Advance to the texture data in the file
  
  if ( fseek( stream, data_offset, SEEK_SET ) )
    {
      return FXFALSE;
    }
  
  return FXTRUE;
}



// Texus version of procedure to read a TXS header.  The output_info parameter controls whether
// the header information is output.

FxBool _txReadTXSHeader(FILE* stream, TxMip *txMip, FxBool output_info )
{
  int     i, w, h;
  TXSInfo info;
  
  // Read the TXS Header
  
  if (!readTXSHeader(stream, &info, output_info))
    {
      return FXFALSE;
    }
  
  // If we were just supposed to output the header info, then we're done
  
  if (output_info)
    {
      return FXTRUE;
    }
  
  // Convert the header data into Texus format
  
  txMip->format = info.format;
  txMip->width  = info.width;
  txMip->height = info.height;
  txMip->depth  = info.mipmap_levels;
  
  // Calculate size of texture data
  
  w = txMip->width;
  h = txMip->height;
  
  txMip->size = w * h;
  
  for (i = 1; i < txMip->depth; i++)
    {
      if (w > 1) w >>= 1;
      if (h > 1) h >>= 1;
      
      txMip->size += (w * h);
    }
  
  txMip->size = (txMip->size * GR_TEXFMT_SIZE(txMip->format)) / 8;
  
  return FXTRUE;
}


// General procedure to read the NCC table from a TXS file.  The format is that same as that
// used by Glide.

FxBool _readTXSNCCTable(FILE *stream, FxU32 *nccTable)
{
  unsigned int i;
  
  for (i = 0; i < sizeof(GuNccTable) >> 2; i++)
    {
      if (!_txsRead32 (stream, (FxU32 *) &nccTable[i]))
        {
          return FXFALSE;
        }
    }
  
  return FXTRUE;
}



// General procedure to read TXS texture data (including the palette table if present)

FxBool readTXSData( FILE *stream, TXSInfo *info)
{
  FxU32 i, npixels;
  
  /* First read NCC/palette tables */
  
  if ((info->format == GR_TEXFMT_YIQ_422) ||
      (info->format == GR_TEXFMT_AYIQ_8422))
    {
      if (!_readTXSNCCTable (stream, (FxU32 *) info->table.nccTable))
        {
          if (txVerbose)
            {
              txError("Bad Ncc table\n");
            }
          
          return FXFALSE;
        }
    }
  else if ((info->format == GR_TEXFMT_P_8) ||
           (info->format == GR_TEXFMT_AP_88) ||
           (info->format == GR_TEXFMT_P_8_6666))
    {
      if (!_readTXSPalTable (stream, (FxI32*) info->table.palette))
        {
          if (txVerbose)
            {
              txError("Bad Palette table\n");
            }
          
          return FXFALSE;
        }
    }
  
  /* read mipmap image data */
  
  switch (GR_TEXFMT_SIZE(info->format))
    {
    case 4:
      // We'll only hit this when writing TXS files with a FXT1 or DXT* texture format.
      // Treat this the same as the 8 BPP case.
    case 8:
      npixels = info->data_size;
      
      if ((FxU32) npixels != fread (info->data, 1, npixels, stream))
        {
          if (txVerbose)
            {
              txError("Bad 4/8 bit data");
            }
          
          return FXFALSE;
        }
      
      break;
      
    case 16:
      {
        FxU16* data = (FxU16 *) info->data;
        
        npixels = info->data_size >> 1;
        
        for (i = 0; i < npixels; i++, data++)
          {
            if (FXFALSE == _txsRead16 (stream, data))
              {
                if (txVerbose)
                  {
                    txError("Bad 16 bit data");
                  }
                
                return FXFALSE;
              }
          }
        
        break;
      }
    case 24:
      {
        //to be done
        break;
      }
    
    case 32:
      {
        FxU32* data = (FxU32*) info->data;
        
        npixels = info->data_size >> 2;
        
        for (i = 0; i < npixels; i ++, data++)
          {
            if (FXFALSE == _txsRead32 (stream, data))
              {
                if (txVerbose)
                  {
                    txError("Bad 32 bit data");
                  }
                
                return FXFALSE;
              }
          }
        
        break;
      }
    
    default:
      return FXFALSE;
    }
  
  return FXTRUE;
}



// Texus version of procedure to read TXS texture data

FxBool _txReadTXSData ( FILE *stream, TxMip *txMip )
{
  TXSInfo info;
  FxBool  ncc = FXFALSE;
  
  // Convert the Texus data into TXS format.  The data pointer refers to the same
  // memory location that txMip uses, so the txMip structure is actually filled at the
  // same time.
  
  info.format    = txMip->format;
  info.data_size = txMip->size;
  info.data      = txMip->data[0];
  
  // If this is a YIQ texture then malloc memory for the NCC table since Texus uses a
  // different format than Glide and TXS use so it'll have to be converted after its
  // been read in.
  
  switch (txMip->format)
    {
    case GR_TEXFMT_YIQ_422:
    case GR_TEXFMT_AYIQ_8422:
      
      ncc = FXTRUE;
      
      info.table.nccTable = malloc(sizeof(GuNccTable));
      
      if (info.table.nccTable == NULL)
        {
          return FXFALSE;
        }
      
      break;
      
    default:
      
      // The palette pointer refers to the same memory location that txMip uses so the
      // txMip structure is actually filled at the same time.
      
      info.table.palette = txMip->pal;
      break;
    }
  
  // Read the texture data
  
  if (readTXSData( stream, &info ))
    {
      // If this is a YIQ texture, then convert the NCC table into the format Texus uses and
      // free the memory we allocated.
      
      if (ncc)
        {
          txNccToPal( txMip->pal, (GuNccTable *) info.table.nccTable);
          
          free(info.table.nccTable);
        }
    }
  else
    {
      // If this is a YIQ texture, then free the memory we allocated.
      
      if (ncc)
        {
          free(info.table.nccTable);
        }
      
      return FXFALSE;
    }
  
  return FXTRUE;
}



/*************************************** Write Procs ****************************/



// General procedure to write a TXS header

FxBool writeTXSHeader (FILE *stream, TXSInfo *info)
{
  FxU32 offset;
  
  // Go to the start of the file
  
  if ( fseek( stream, 0, SEEK_SET ) )
    {
      return FXFALSE;
    }
  
  // Write out the header
  
  offset = fprintf( stream, "%s %f %d %d %d %d ", TXS_COOKIE, TXS_VERSION,
                    info->format, info->width, info->height, info->mipmap_levels);
  
  if ( offset <= 0)
    {
      return FXFALSE;
    }
  
  // Account for the size of data offset being output plus the space after it
  
  offset += TXS_WRITE_OFFSET_SIZE;
  
  // Write out the data offset
  
  if ( fprintf( stream, "%08x ", offset ) != TXS_WRITE_OFFSET_SIZE)
    {
      return FXFALSE;
    }
  
  return FXTRUE;
}



// General procedure to write the NCC table to a TXS file.  The format is that same as that
// used by Glide.

FxBool _writeTXSNCCTable (FILE *stream, FxU32 *nccTable)
{
  unsigned int i;
  
  for (i = 0; i < sizeof(GuNccTable) >> 2; i++)
    {
      if (!_txsWrite32 (stream, nccTable[i]))
        {
          return FXFALSE;
        }
    }
  
  return FXTRUE;
}



// General procedure to write TXS texture data (including the palette table if present)

FxBool writeTXSData (FILE *stream, TXSInfo *info)
{
  FxU32 i, n_pixels;
  
  // Write out the NCC/palette table if necessary
  
  if ((info->format == GR_TEXFMT_YIQ_422) ||
      (info->format == GR_TEXFMT_AYIQ_8422))
    {
      if (!_writeTXSNCCTable (stream, (FxU32 *) info->table.nccTable))
        {
          return FXFALSE;
        }
    } 
  else if ((info->format == GR_TEXFMT_P_8) ||
           (info->format == GR_TEXFMT_AP_88) ||
           (info->format == GR_TEXFMT_P_8_6666))
    {
      if (!_writeTXSPalTable (stream, (FxU32 *) info->table.palette))
        {
          return FXFALSE;
        }
    } 
  
  // Write out mipmap image data
  switch (GR_TEXFMT_SIZE(info->format))
    {
    case 4:
      // We'll only hit this when writing TXS files with a FXT1 or DXT* texture format.
      // Treat this the same as the 8 BPP case.
    case 8:
      n_pixels = info->data_size;
      
      if (n_pixels != fwrite (info->data, 1, n_pixels, stream))
        {
          return FXFALSE;
        }
      
      break;
      
    case 16:
      {
        FxU16* data = (FxU16 *) info->data;
        
        n_pixels = info->data_size >> 1;
        
        for (i = 0; i < n_pixels; i++)
          {
            if (!_txsWrite16 (stream, *data++))
              {
                return FXFALSE;
              }
          }
        
        break;
      }
    case 24:
      {
        //to be done
        break;
      }
    
    case 32:
      {
        FxU32* data = (FxU32*) info->data;
        
        n_pixels = info->data_size >> 2;
        
        for (i = 0; i < n_pixels; i ++)
          {
            if (!_txsWrite32 (stream, *data++))
              {
                return FXFALSE;
              }
          }
        
        break;
      }
    
    default:
      return FXFALSE;
    }
  
  return FXTRUE;
}



// Texus version of procedure to write a TXS file

FxBool txWriteTXS( FILE *stream, TxMip *txMip)
{
  TXSInfo info;
  FxBool  ncc = FXFALSE;
  FxBool  rc;
  
  // Convert the data into TXS format.  Note that the data pointer refers to the
  // same memory location that txMip uses.
  
  info.format        = txMip->format;
  info.width         = txMip->width;
  info.height        = txMip->height;
  info.mipmap_levels = txMip->depth;
  info.data_size     = txMip->size;
  info.data          = txMip->data[0];
  
  // Write out the header
  
  if (!writeTXSHeader(stream, &info))
    {
      return FXFALSE;
    }
  
  // If this is a YIQ texture then malloc memory for the NCC table and convert the table
  // into Glide/TXS format since Texus uses a different format.
  
  switch (txMip->format)
    {
    case GR_TEXFMT_YIQ_422:
    case GR_TEXFMT_AYIQ_8422:
      
      ncc = FXTRUE;
      
      info.table.nccTable = malloc(sizeof(GuNccTable));
      
      if (info.table.nccTable == NULL)
        {
          return FXFALSE;
        }
      
      txPalToNcc( (GuNccTable *) info.table.nccTable, txMip->pal );
      break;
      
    default:
      
      // The palette pointer refers to the same memory location that txMip uses.
      
      info.table.palette = txMip->pal;
      break;
    }
  
  // Write out the texture data
  
  rc = writeTXSData(stream, &info);
  
  // If this is a YIQ texture, then free the memory we allocated.
  
  if (ncc)
    {
      free(info.table.nccTable);
    }
  
  return rc;
}



/*************************** General & Utility Procs ****************************/



// Initialize all the fields of the TXSInfo structure to 0/NULL.

void initTXSInfo (TXSInfo *info)
{
  memset(info, 0, sizeof (TXSInfo));
  
  info->table.palette = NULL;
  info->data    = NULL;
}



// Calculate and return the number of bytes required by the TXS texture

FxU32 calcTXSMemRequired (TXSInfo *info)
{
  int   i, w, h;
  FxU32 size;
  
  // Calculate size of texture data
  
  w = info->width;
  h = info->height;
  
  size = w * h;
  
  for (i = 1; i < info->mipmap_levels; i++)
    {
      if (w > 1) w >>= 1;
      if (h > 1) h >>= 1;
      
      size += (w * h);
    }
  
  size = (size * txBitsPerPixel(info->format)) >> 3;
  
  return size;
}



// Free any memory malloc'd for the TXS texture.
//
// NOTE:  This should generally only be used if you are allowing loadTXS to malloc texture data
//        and palette/NCC table for you.  If you are malloc'ing memory for either of these
//        yourself, then you may want to call free() yourself.

void freeTXS (TXSInfo *info)
{
  // Free the data if it's been malloc'd
  
  if (info->data != NULL)
    {
      free (info->data);
    }
  
  // Free the palette/NCC table if it's been malloc'd
  
  if (info->table.palette != NULL)
    {
      free (info->table.palette);
    }
}



// Internal free routine used by _mallocTXS.  This only frees data that _mallocTXS explicitly
// malloc'd itself.  free_data and free_table indicate which structures were malloc'd by
// _mallocTXS and so should be freed

void _freeTXS(TXSInfo *info, FxBool free_data, FxBool free_table)
{
  // Free data if it's been malloc'd and were told to free it
  
  if (free_data & (info->data != NULL))
    {
      free (info->data);
    }
  
  // Free the palette/NCC table if it's been malloc'd and were told to free it
  
  if (free_table & (info->table.palette != NULL))
    {
      free (info->table.palette);
    }
}



// _mallocTXS allocates any memory needed for the TXS texture described by info.
// It also sets the data_size attribute for the TXS texture.
//
// malloc_data  - set to FXTRUE if memory is allocated for the texture data
// malloc_table - set to FXTRUE if memory is allocated for the palette/NCC table

FxBool _mallocTXS (TXSInfo *info, FxBool *malloc_data, FxBool *malloc_table)
{
  *malloc_data  = FXFALSE;
  *malloc_table = FXFALSE;
  
  // Calculate the size of the texture data
  
  info->data_size = calcTXSMemRequired (info);
  
  // If the app didn't already allocate store for the texture data, then we must malloc it.
  
  if (info->data == NULL)
    {
      info->data = malloc( info->data_size );
      
      if (info->data == NULL)
        {
          return FXFALSE;
        }
      
      *malloc_data = FXTRUE;
    }
  
  // If the app didn't already allocate store for the palette table and the texture is
  // palettized (or NCC), then we must malloc it.
  
  if (info->table.palette == NULL)
    {
      switch (info->format)
        {
        case GR_TEXFMT_P_8:
        case GR_TEXFMT_AP_88:
        case GR_TEXFMT_P_8_6666:
          
          info->table.palette = malloc ( sizeof(FxU32) * 256 );
          
          if (info->table.palette == NULL)
            {
              _freeTXS(info, *malloc_data, *malloc_table);
              *malloc_data = FXFALSE;
              return FXFALSE;
            }
          
          *malloc_table = FXTRUE;
          break;
          
        case GR_TEXFMT_YIQ_422:
        case GR_TEXFMT_AYIQ_8422:
          
          info->table.nccTable = malloc ( sizeof(GuNccTable) );
          
          if (info->table.nccTable == NULL)
            {
              _freeTXS(info, *malloc_data, *malloc_table);
              *malloc_data = FXFALSE;
              return FXFALSE;
            }
          
          *malloc_table = FXTRUE;
          break;
        }
    }
  
  return FXTRUE;
}



// Internal general TXS load routine.  The info_only parameter controls whether only the header
// information is read or if the actual texture data is loaded.
//
// If we are loading the texture data, then memory for the data (and the palette if necessary)
// will be malloc'd if the application passed in info->data or info->palette set to NULL.
//
// If either of these fields is not NULL, then it is assumed the app has already malloc'd the
// store and sent in the pointer to the memory that should be used.

FxBool _loadTXSFile (const char *filename, TXSInfo *info, FxBool info_only)
{
  FILE *stream;
  
  stream = fopen(filename, "rb");
  
  if( stream == NULL )
    {
      return FXFALSE;
    }
  
  // Read the TXS header
  
  if (!readTXSHeader( stream, info, FXFALSE ))
    {
      fclose(stream);
      return FXFALSE;
    }
  
  // If we're only reading the header info, then just set the size.  Otherwise, malloc and
  // memory that we need (which sets the size as well).
  
  if (info_only)
    {
      // Calculate the size of the texture data
      
      info->data_size = calcTXSMemRequired (info);
    }
  else
    {
      FxBool malloc_data, malloc_table;
      
      if (!_mallocTXS(info, &malloc_data, &malloc_table))
        {
          fclose(stream);
          return FXFALSE;
        }
      
      // Read the texture data (including the palette if present)
      
      if (!readTXSData( stream, info ))
        {
          _freeTXS(info, malloc_data, malloc_table);
          fclose(stream);
          return FXFALSE;
        }
    }
  
  fclose(stream);
  
  return FXTRUE;
}



// Procedure to get the information for a TXS file.

FxBool getTXSInfo (const char *filename, TXSInfo *info)
{
  return _loadTXSFile (filename, info, FXTRUE);
}



// General procedure to load a TXS file.
//
// If you wish to malloc store for the texture data and/or palette/NCC table yourself, then malloc
// these in your app and set the info->data and/or info->table.palette/info->table.nccTable fields to these
// memory locations before you call loadTXS.
//
// If you wish to allow loadTXS to malloc the required memory for the texture data and/or
// palette/NCC table (only required for palettized/NCC textures of course), then set info->data
// and/or info->table.palette/info->table.nccTable to NULL before you call loadTXS.
//
// The initTXSInfo procedure can be used to intialize the entire TXSInfo structure (including
// setting the data and palette fields to NULL) before loadTXS is called.

FxBool loadTXS (const char *filename, TXSInfo *info)
{
  return _loadTXSFile (filename, info, FXFALSE);
}



// General procedure to write a TXS file based on the TXSInfo structure that is passed in.

FxBool writeTXS (const char *filename, TXSInfo *info)
{
  FILE   *stream;
  
  stream = fopen(filename, "wb");
  
  if( stream == NULL )
    {
      return FXFALSE;
    }
  
  // Write out the header
  
  if (!writeTXSHeader(stream, info))
    {
      fclose(stream);
      return FXFALSE;
    }
  
  // Write out the texture data (and palette if present)
  
  if (!writeTXSData(stream, info))
    {
      fclose(stream);
      return FXFALSE;
    }
  
  fclose(stream);
  
  return FXTRUE;
}



// Initializes the TXSConvertOptions structure passed in to the default values

void initTXSConvertOptions (TXSConvertOptions *opt)
{
  opt->new_mipmaps = FXTRUE;
  opt->blend_alpha = FXTRUE;
  opt->force_alpha = FXFALSE;
  opt->alpha_value = 255;
  opt->fxt1_format = TCC_MIXED;//TCC_AUTO;
  opt->dither      = TX_DITHER_ERR;
  opt->compression = TX_COMPRESSION_HEURISTIC;
}



// convertTXS converts the texture described by src into the texture format described by dest
// using the options specified by opt.
//
// Just like loadTXS, convertTXS can allocate any memory required if you want it to.
//
// If you wish to malloc store for the texture data and/or palette/NCC table yourself, then malloc
// these in your app and set the info->data and/or info->palette/info->table.nccTable fields to these
// memory locations before you call convertTXS.
//
// If you wish to allow convertTXS to malloc the required memory for the texture data and/or
// palette/NCC table (only required for palettized/NCC textures of course), then set info->data
// and/or info->palette/info->table.nccTable to NULL before you call convertTXS.
//
// The initTXSConvertOptions procedure can be used to intialize the entire TXSConvertOptions
// structure before convertTXS is called.

FxBool convertTXS (TXSInfo *src, TXSInfo *dest, TXSConvertOptions *opt)
{
  TxMip  srcMip, destMip, trueColorMip, tmpMip;
  FxBool malloc_data, malloc_table;
  int    i, j, w, h;
  
  // If we aren't generating new mipmaps then the source must have at least as many
  // levels as the destination requires.
  
  
  if (!opt->new_mipmaps)
    {
      if (src->mipmap_levels < dest->mipmap_levels)
        {
          return FXFALSE;
        }
    }
  
  // Setup the source TxMip structure pointing to the data in the src TXS structure so that
  // we don't have to copy the data.
  
  srcMip.format  = src->format;
  srcMip.width   = src->width;
  srcMip.height  = src->height;
  srcMip.depth   = src->mipmap_levels;
  srcMip.size    = src->data_size;
  srcMip.data[0] = src->data;
  
  txMipSetMipPointers(&srcMip);
  
  
  switch (src->format)
    {
    case GR_TEXFMT_YIQ_422:
    case GR_TEXFMT_AYIQ_8422:
      
      // Convert the NCC table to the format Texus uses which is different than the
      // format used by Glide and TXS.
      
      txNccToPal( srcMip.pal, (GuNccTable *) src->table.nccTable);
      break;
      
    case GR_TEXFMT_P_8:
    case GR_TEXFMT_AP_88:
    case GR_TEXFMT_P_8_6666:
      
      // Copy the palette
      
      memcpy( srcMip.pal, src->table.palette, sizeof(FxU32) * 256 );
      break;
    }
  
  // Setup the true color texture structure.  Note that the depth is set to dest->mipmap_levels
  // since at most we only need to process as many levels as the destination needs.
  //
  // Side Note - if we are generating new mipmaps but not resampling and the source has fewer
  // levels than the dest, then using srcMip.depth wouldn't allocate sufficient memory
  
  trueColorMip.format = GR_TEXFMT_ARGB_8888;
  trueColorMip.width  = srcMip.width;
  trueColorMip.height = srcMip.height;
  trueColorMip.depth  = dest->mipmap_levels;
  
  // Allocate the memory for the true color texture
  
  if( !txMipAlloc( &trueColorMip ) )
    {
      return FXFALSE;
    }
  
  // If we are generating new mipmap levels, then we only need to process the top level of
  // the source data so set the depth to 1.  Otherwise, we are guaranteed from the check at
  // the top that the source has at least as many levels as the destination so using the
  // destination depth as we are is fine.
  
  if (opt->new_mipmaps)
    {
      trueColorMip.depth = 1;
    }
  
  // Dequantize the texture from the source format into 32-bit ARGB.
  
  txMipDequantize( &trueColorMip, &srcMip );
  
  // Force all the alpha values in the texture to alpha_value if we are requested to do so.
  
  if (opt->force_alpha)
    {
      FxU32 *data, mask;
      
      mask = ((FxU32) opt->alpha_value) << 24;
      mask |= 0x00FFFFFF;
      
      w = trueColorMip.width;
      h = trueColorMip.height;
      
      for (i = 0; i < trueColorMip.depth; i++)
        {
          data = (FxU32 *) trueColorMip.data[i];
          
          for (j = w * h; j > 0; j--)
            {
              *data &= mask;
              data++;
            }
          
          if (w > 1) w >>= 1;
          if (h > 1) h >>= 1;
        }
    }
  
  // If the width or height is changing, then we need to resample the texture data.
  
  if ((src->width != dest->width) || (src->height != dest->height))
    {
      // Setup a new texture structure for the resampled size
      
      tmpMip.format = GR_TEXFMT_ARGB_8888;
      tmpMip.width  = dest->width;
      tmpMip.height = dest->height;
      tmpMip.depth  = dest->mipmap_levels;
      
      // Allocate the memory for the resample true color texture
      
      if ( !txMipAlloc( &tmpMip ) )
        {
          txFree( trueColorMip.data[0] );
          return FXFALSE;
        }
      
      // If we are generating new mipmap levels, then we only need to process the top level of
      // the source data so set the depth to 1.
      
      if (opt->new_mipmaps)
        {
          tmpMip.depth = 1;
        }
      
      // Resample the texture to its new size
      
      txMipResample( &tmpMip, &trueColorMip );
      
      // Free the old true color texture data
      
      txFree( trueColorMip.data[0] );
      
      // Set our true color texture to its new settings and data
      
      trueColorMip = tmpMip;
    }
  
  // If we are supposed to generate new mipmaps, then set the depth to the real destination
  // value and generate the mipmaps.
  
  if (opt->new_mipmaps)
    {
      trueColorMip.depth = dest->mipmap_levels;
      
      //txMipMipmap( &trueColorMip, opt->blend_alpha );
      txMipMipmap( &trueColorMip );
    }
  
  // Setup the destination TxMip structure.
  
  destMip.format = dest->format;
  destMip.width  = dest->width;
  destMip.height = dest->height;
  destMip.depth  = dest->mipmap_levels;
  
  // Allocate any memory we need to for the TXS texture
  
  if (!_mallocTXS(dest, &malloc_data, &malloc_table))
    {
      txFree( trueColorMip.data[0] );
      return FXFALSE;
    }
  
  // Point the destination TxMip to the destination TXS's data area so that we don't have to
  // copy the data.
  
  destMip.data[0] = dest->data;
  
  txMipSetMipPointers(&destMip);
  
  // Quantize the texture from 32-bit ARGB into the destination format
  
  txMipQuantize( &destMip, &trueColorMip, destMip.format, opt->dither, opt->compression );
  
  switch (dest->format)
    {
    case GR_TEXFMT_YIQ_422:
    case GR_TEXFMT_AYIQ_8422:
      
      // Convert the NCC table to the format Glide and TXS use since the format that
      // Texus uses is different.
      
      txPalToNcc( (GuNccTable *) dest->table.nccTable, destMip.pal );
      break;
      
    case GR_TEXFMT_P_8:
    case GR_TEXFMT_AP_88:
    case GR_TEXFMT_P_8_6666:
      
      // Copy the palette
      
      memcpy( dest->table.palette, destMip.pal, sizeof(FxU32) * 256 );
      break;
    }
  
  // Free the true color texture
  
  txFree( trueColorMip.data[0] );
  
  return FXTRUE;
}



// convertTXS converts the TXS texture stored in filename into the texture format described by
// dest using the options specified by opt.
//
// Note that convertTXS performs the actual conversion, so please see the additional comments
// that go along with this procedure (such as memory allocation for dest).

FxBool convertTXSFile (const char *filename, TXSInfo *dest, TXSConvertOptions *opt)
{
  TXSInfo src;
  FxBool  rc = FXFALSE;
  
  initTXSInfo(&src);
  
  if (!loadTXS(filename, &src))
    {
      return rc;
    }
  
  rc = convertTXS(&src, dest, opt);
  
  freeTXS(&src);
  
  return rc;
}

static FxBool 
_readTXSPalTable (FILE* stream, FxI32* pal)
{
  FxU32 i;
  
  /* read palette */
  for (i = 0; i < 256; i ++) {
    if (FXFALSE == _txsRead32 (stream, (FxU32 *)&pal[i])) return FXFALSE;
  }
  return FXTRUE;
}

static FxBool 
_txsRead16 (FILE *stream, FxU16* data)
{
  
  if (fread (data, 2, 1, stream) != 1) return FXFALSE;
  
  return FXTRUE;
}

/* Read long word, little endian.*/
static FxBool 
_txsRead32 (FILE *stream, FxU32* data)
{
  
  if (fread (data, 4, 1, stream) != 1) return FXFALSE;
  
  return FXTRUE;
}

static FxBool 
_txsWrite16 (FILE *stream, FxU16 data)
{
  
  return (fwrite (&data, sizeof(FxU16), 1, stream) != 1) ? FXFALSE : FXTRUE;
  
}

/* Write long word, msb first */

static FxBool 
_txsWrite32 (FILE *stream, FxU32 data)
{
  
  return (fwrite (&data, sizeof(FxU32), 1, stream) != 1) ? FXFALSE : FXTRUE;
  
}

static FxBool
_writeTXSPalTable (FILE *stream, FxU32 *pal)
{
  int     i;
  
  for (i=0; i<256; i++) {
    if (!_txsWrite32 (stream, pal[i])) return FXFALSE;
  }
  return FXTRUE;
}

//FxBool _txsRead16 (FILE *stream, FxU16* data);
//FxBool _txsRead32 (FILE *stream, FxU32* data);

//FxBool _txWrite16 (FILE *stream, FxU16 data);
//FxBool _txWrite32 (FILE *stream, FxU32 data);
