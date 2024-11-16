/******************************************************************************
 * tif_ovrcache.h,v 1.3 2005/05/25 09:03:16 dron Exp
 *
 * Project:  TIFF Overview Builder
 * Purpose:  Library functions to maintain two rows of tiles or two strips
 *           of data for output overviews as an output cache.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 * This code could potentially be used by other applications wanting to
 * manage a once-through write cache.
 *
 ******************************************************************************
 * Copyright (c) 2000, Frank Warmerdam
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */

#ifndef TIF_OVRCACHE_H_INCLUDED
#define TIF_OVRCACHE_H_INCLUDED

#include "tiffio.h"

#if defined(__cplusplus)
extern "C"
{
#endif

    typedef struct
    {
        uint32_t nXSize;
        uint32_t nYSize;

        uint16_t nBitsPerPixel;
        uint16_t nSamples;
        uint16_t nPlanarConfig;
        uint32_t nBlockXSize;
        uint32_t nBlockYSize;
        toff_t nBytesPerBlock;
        toff_t nBytesPerRow;

        int nBlocksPerRow;
        int nBlocksPerColumn;

        int nBlockOffset; /* what block is the first in papabyBlocks? */
        unsigned char *pabyRow1Blocks;
        unsigned char *pabyRow2Blocks;

        toff_t nDirOffset;
        TIFF *hTIFF;
        int bTiled;

    } TIFFOvrCache;

    TIFFOvrCache *TIFFCreateOvrCache(TIFF *hTIFF, toff_t nDirOffset);
    unsigned char *TIFFGetOvrBlock(TIFFOvrCache *psCache, int iTileX,
                                   int iTileY, int iSample);
    unsigned char *TIFFGetOvrBlock_Subsampled(TIFFOvrCache *psCache, int iTileX,
                                              int iTileY);
    void TIFFDestroyOvrCache(TIFFOvrCache *);

    void TIFFBuildOverviews(TIFF *, int, int *, int, const char *,
                            int (*)(double, void *), void *);

    void TIFF_ProcessFullResBlock(TIFF *, int, int, int, int, int, int *, int,
                                  int, TIFFOvrCache **, uint32_t, uint32_t,
                                  unsigned char *, uint32_t, uint32_t, int,
                                  const char *);

    uint32_t TIFF_WriteOverview(TIFF *, uint32_t, uint32_t, int, int, int, int,
                                int, int, int, int, int, unsigned short *,
                                unsigned short *, unsigned short *, int, int,
                                int);

#if defined(__cplusplus)
}
#endif

#endif /* ndef TIF_OVRCACHE_H_INCLUDED */
