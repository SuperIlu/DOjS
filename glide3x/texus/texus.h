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

#ifndef __TEXUS_H__
#define __TEXUS_H__

#ifdef __cplusplus 
extern "C" { 
#endif

#include <stdlib.h>
#include <stdio.h>
#include <3dfx.h>
#define FX_DLL_DEFINITION
#include <fxdll.h> 

#ifndef __3DFX_H__
/*
** basic data types
*/
typedef unsigned char   FxU8;
typedef unsigned short  FxU16;
typedef short           FxI16;
typedef unsigned int    FxU32;
typedef int             FxI32;
typedef int             FxBool;

/*
** fundamental types
*/

#define FXTRUE    1
#define FXFALSE   0
#endif  /* __3DFX_H__ */

 
#ifndef __GLIDE_H__
/* These are defined so we can use them without glide */
typedef FxU32 GrTextureFormat_t;
#define GR_TEXFMT_8BIT                  0x0
#define GR_TEXFMT_RGB_332                               GR_TEXFMT_8BIT
#define GR_TEXFMT_YIQ_422               0x1
#define GR_TEXFMT_ALPHA_8               0x2 /* (0..0xFF) alpha     */
#define GR_TEXFMT_INTENSITY_8           0x3 /* (0..0xFF) intensity */
#define GR_TEXFMT_ALPHA_INTENSITY_44    0x4
#define GR_TEXFMT_P_8                   0x5
#define GR_TEXFMT_P_8_6666              0x6
#define GR_TEXFMT_P_8_6666_EXT          0x6
#define GR_TEXFMT_RSVD2                 0x7
#define GR_TEXFMT_16BIT                 0x8
#define GR_TEXFMT_ARGB_8332                     GR_TEXFMT_16BIT
#define GR_TEXFMT_AYIQ_8422             0x9
#define GR_TEXFMT_RGB_565               0xa
#define GR_TEXFMT_ARGB_1555             0xb
#define GR_TEXFMT_ARGB_4444             0xc
#define GR_TEXFMT_ALPHA_INTENSITY_88    0xd
#define GR_TEXFMT_AP_88                 0xe
#define GR_TEXFMT_RSVD4                 0xf

/* sst2 formats */
#define GR_TEXFMT_ARGB_CMP_FXT1        0x11
#define GR_TEXFMT_ARGB_8888            0x12
#define GR_TEXFMT_YUYV_422             0x13
#define GR_TEXFMT_UYVY_422             0x14
#define GR_TEXFMT_AYUV_444             0x15

#define GR_TEXFMT_ARGB_CMP_DXT1        0x16
#define GR_TEXFMT_ARGB_CMP_DXT2        0x17
#define GR_TEXFMT_ARGB_CMP_DXT3        0x18
#define GR_TEXFMT_ARGB_CMP_DXT4        0x19
#define GR_TEXFMT_ARGB_CMP_DXT5        0x1A

#define GR_TEXFMT_LAST				   GR_TEXFMT_ARGB_CMP_DXT5

typedef FxI32 GrLOD_t;
#ifdef GLIDE3
#define GR_LOD_256              0x8
#define GR_LOD_128              0x7
#define GR_LOD_64               0x6
#define GR_LOD_32               0x5
#define GR_LOD_16               0x4
#define GR_LOD_8                0x3
#define GR_LOD_4                0x2
#define GR_LOD_2                0x1
#define GR_LOD_1                0x0
#else
#define GR_LOD_256              0x0
#define GR_LOD_128              0x1
#define GR_LOD_64               0x2
#define GR_LOD_32               0x3
#define GR_LOD_16               0x4
#define GR_LOD_8                0x5
#define GR_LOD_4                0x6
#define GR_LOD_2                0x7
#define GR_LOD_1                0x8
#endif 

typedef FxI32 GrAspectRatio_t;
#ifdef GLIDE3
#define GR_ASPECT_8x1   3       /* 8W x 1H */
#define GR_ASPECT_4x1   2       /* 4W x 1H */
#define GR_ASPECT_2x1   1       /* 2W x 1H */
#define GR_ASPECT_1x1   0       /* 1W x 1H */
#define GR_ASPECT_1x2  -1       /* 1W x 2H */
#define GR_ASPECT_1x4  -2       /* 1W x 4H */
#define GR_ASPECT_1x8  -3       /* 1W x 8H */
#else
#define GR_ASPECT_8x1 0x0       /* 8W x 1H */
#define GR_ASPECT_4x1 0x1       /* 4W x 1H */
#define GR_ASPECT_2x1 0x2       /* 2W x 1H */
#define GR_ASPECT_1x1 0x3       /* 1W x 1H */
#define GR_ASPECT_1x2 0x4       /* 1W x 2H */
#define GR_ASPECT_1x4 0x5       /* 1W x 4H */
#define GR_ASPECT_1x8 0x6       /* 1W x 8H */
#endif /* GLIDE3 */

/*
** 3DF texture file structs
*/
typedef struct
{
  FxU32               width, height;
  int                 small_lod, large_lod;
  GrAspectRatio_t     aspect_ratio;
  GrTextureFormat_t   format;
} Gu3dfHeader;

typedef struct
{
  FxU8  yRGB[16];
  FxI16 iRGB[4][3];
  FxI16 qRGB[4][3];
  FxU32 packed_data[12];
} GuNccTable;

typedef struct {
    FxU32 data[256];
} GuTexPalette;

typedef union {
    GuNccTable   nccTable;
    GuTexPalette palette;
} GuTexTable;

typedef struct
{
  Gu3dfHeader  header;
  GuTexTable   table;
  void        *data;
  FxU32        mem_required;    /* memory required for mip map in bytes. */
} Gu3dfInfo;

#endif          /* __GLIDE_H__*/

#define GR_TEXFMT_32BIT                                 0x12
#define GR_TEXFMT_RGB_888                               0xff
#define GR_TEXFMT_ANY                                   0x7fffffff

/* Save typing fingers*/
#define GR_TEXFMT_A_8                                   GR_TEXFMT_ALPHA_8
#define GR_TEXFMT_I_8                                   GR_TEXFMT_INTENSITY_8
#define GR_TEXFMT_AI_44                                 GR_TEXFMT_ALPHA_INTENSITY_44
#define GR_TEXFMT_AI_88                                 GR_TEXFMT_ALPHA_INTENSITY_88

#define TX_DITHER_NONE                                  0x00000000
#define TX_DITHER_4x4                                   0x00000001
#define TX_DITHER_ERR                                   0x00000002
#define TX_DITHER_MASK                                  0x0000000f

#define TX_COMPRESSION_STATISTICAL                      0x00000000
#define TX_COMPRESSION_HEURISTIC                        0x00000010
#define TX_COMPRESSION_MASK                             0x000000f0

#define TX_CLAMP_DISABLE                                0x00000000
#define TX_CLAMP_ENABLE                                 0x00000100
#define TX_CLAMP_MASK                                   0x00000f00

#define TX_AUTORESIZE_DISABLE                           0x00000000
#define TX_AUTORESIZE_GROW                              0x00001000
#define TX_AUTORESIZE_SHRINK                            0x00002000
#define TX_AUTORESIZE_MASK                              0x0000f000

#define TX_TARGET_PALNCC_BESTFIT                        0x00000000
#define TX_TARGET_PALNCC_SOURCE                         0x00010000
#define TX_TARGET_PALNCC_MASK                           0x000f0000

/*
 * (GaryMcT)
 * TX_FIXED_PAL_QUANT_DIST -    use color distance to map a true color
 *                              image back into a user specified  palette.
 *                              Use this when you have a lot of different
 *                              palettes.       
 * TX_FIXED_PAL_QUANT_TABLE-    use an inverse palette to map colors 
 *                              into a user specfied palette.
 *                              Use this method when you have a few
 *                              palettes.  Will run faster if you
 *                              process textures with the same 
 *                              palette sequentially since the palette
 *                              is compiled internally to a lookup table.
 */
#define TX_FIXED_PAL_QUANT_DIST                         0x00000000
#define TX_FIXED_PAL_QUANT_TABLE                        0x00100000
#define TX_FIXED_PAL_QUANT_MASK                         0x00f00000

#define TX_WRITE_3DF                                    0x00000000
#define TX_WRITE_TGA                                    0x00000001
#define TX_WRITE_TXS                                    0x00000002
#define TX_WRITE_MASK                                   0x0000000f

/*
 * Publicly accessible functions.
 */

/*
 * txInit3dfInfo:
 * Set up the target parameters for a texture conversion.
 *      info                    - Glide texture structure to set up.
 *      destFormat              - format that we will be converting to.
 *      destWidth, destHeight   - geometry of the target texture image.
 *                                Are modified if TX_AUTORESIZE is enabled.
 *      mipLevels               - number of mipmap levels in the target texture.
 *                                Either specifies the number of mipmap levels
 *                                to create, or -1 for all possible mipmap levels.
 *      flags -
 *         auto-resample mode   - TX_AUTORESIZE_*
 */
size_t txInit3dfInfo( Gu3dfInfo *info, GrTextureFormat_t destFormat,
                      int *destWidth, int *destHeight,
                      int mipLevels, FxU32 flags );

size_t txInit3dfInfoFromFile( FILE *file, 
                              Gu3dfInfo *info, GrTextureFormat_t destFormat,
                              int *destWidth, int *destHeight,
                              int mipLevels, FxU32 flags );

/*
 * txConvert: convert from an in memory texture to a Glide Gu3dfInfo.
 *      info - target texture info. . is set up with txInit3dfInfo.
 *      srcFormat - format of in-memory source texture.
 *      srcWidth, srcHeight - geometry of in memory source texture.
 *                            This is not limited in size or 
 *                            aspect ratio.
 *      flags - 
 *         dither mode                  - TX_DITHER_*
 *         compression                  - TX_COMPRESS_*
 *         clamp mode                   - TX_CLAMP_*
 *         palette conversion mode      - TX_FIXED_PAL_QUANT_*
 */
FxBool txConvert( Gu3dfInfo *info, GrTextureFormat_t srcFormat,
                  int srcWidth, int srcHeight,
                  const void *srcImage, FxU32 flags,
                  const void *palNcc );

FxBool txConvertFromFile( FILE *file, Gu3dfInfo *info, 
                          FxU32 flags, const void *palNcc );

FxBool txWrite( Gu3dfInfo *info, FILE *fp, FxU32 flags );

/*
 * Conversion of a single mip level
 */
FX_ENTRY void FX_CALL txImgQuantize( char *dst, char *src, 
                           int w, int h, 
                           FxU32 format, FxU32 dither);

/*
 * Error handling.
 */
typedef void (*TxErrorCallbackFnc_t)( const char *string, FxBool fatal );

/*
 * Set the error call back function.
 * Paramters:
 *     fnc:            If this is non-null, set the callback
 *                     function to this value.  If this pointer,
 *                     is null, set the callback function to
 *                     the default value.
 *     old_fnc:        If this pointer is non-null, fill its
 *                     contents with the callback function before
 *                     changing it.  If this pointer is null,
 *                     do nothing.
 */
extern void txErrorSetCallback( TxErrorCallbackFnc_t  fnc,
                                TxErrorCallbackFnc_t *old_fnc);

/*
 * TEXUS MEMORY ALLOCATION ROUTINES.
 * These are not to be called directly. . .they are only here
 * so that you can redefine them to your own memory management
 * routines.
 */
void    *txMalloc( size_t size );
void    txFree( void *ptr );
void    *txRealloc( void *ptr, size_t size );

FxU32 txTexCalcMapSize( int x, int y, GrTextureFormat_t format );
FxU32 txTexCalcMemRequired( GrLOD_t small_lod, GrLOD_t large_lod,
                            GrAspectRatio_t aspect, GrTextureFormat_t format );
int txBitsPerPixel(GrTextureFormat_t format);
#ifdef __cplusplus 
}
#endif

#endif  /* __TEXUS_H__ */
