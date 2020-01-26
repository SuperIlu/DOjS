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
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "texusint.h"

const FxU16 IMAGIC         = 0x01DA;
const FxU16 ITYPE_RLE      = 0x01;
const FxU16 ITYPE_NCC      = 0x02;
const FxU16 ITYPE_BGR      = 0x04;
const FxU16 ITYPE_RGT      = 0x08;
    
typedef struct _rgtHeader{
    FxU16 magic;
    FxU8 typeLo;
    FxU8 typeHi;
    FxU8 dimLo;
    FxU8 dimHi;
    FxU8 sizeXLo;
    FxU8 sizeXHi;
    FxU8 sizeYLo;
    FxU8 sizeYHi;
    FxU8 sizeZLo;
    FxU8 sizeZHigh;
} RgtHeader;



static void swapShorts(unsigned short *array, int length)
{
    unsigned short s;
    while (length--) {
        s = *array;
        *array++ = (s << 8) | (s>>8);
    }
}

#if 0 /* not used */
static void swapLongs(unsigned int *array, int length)
{
    unsigned int s;
    while (length--) {
        s = *array;
        *array++ = (s << 24) | ((s >> 8)&0xFF00) | 
                        ((s&0xFF00) << 8) | (s>>24);
    }
}
#endif

// just swap RGB into BGR (leave MSB undefined)
static void swapRGB(unsigned int *array, int length)
{
    unsigned int s;
    while (length--) {
        s = *array;
        *array++ = (s << 16) | (s & 0xFF00) | (s>>16);
    }
}


FxBool 
_txReadRGTHeader( FILE *stream, FxU32 cookie, TxMip *info)
{
    RgtHeader   *rgtHeader = (RgtHeader *) info->pal;

    rgtHeader->magic = (FxU16) cookie;
    if ( stream == NULL ) {
        txPanic("RGT file: Bad file handle.");
        return FXFALSE;
    }
    
    if ( fread( &(rgtHeader->typeLo), 1, sizeof(RgtHeader)-2, stream ) != 10 ) {
        txPanic("RGT file: Unexpected end of file.");
        return FXFALSE;
    }
    if (rgtHeader->magic == IMAGIC) {
        // Needs byte swapping; don't swap magic cookie.
        swapShorts((FxU16 *) &(rgtHeader->typeLo), 5);
    }


    info->format = GR_TEXFMT_ARGB_8888; 
    info->width = rgtHeader->sizeXHi << 8 | rgtHeader->sizeXLo;
    info->height = rgtHeader->sizeYHi << 8 | rgtHeader->sizeYLo;
    info->depth = 1;
    info->size = info->width * info->height * 4;
    if( txVerbose )
      {
        printf("Magic: %.04x w = %d, h = %d, z = %d, typehi = %d, typelo = %d, swap=%d\n", rgtHeader->magic, 
               info->width, info->height, rgtHeader->sizeZLo, rgtHeader->typeHi, rgtHeader->typeLo, rgtHeader->magic == IMAGIC);
      }
    return FXTRUE;
}

// RGT is RGBA in memory (low byte to high byte), or ABGR in a register

FxBool 
_txReadRGTData( FILE *stream, TxMip *info)
{
    RgtHeader   *rgtHeader = (RgtHeader *) info->pal;
    FxU16 type = (rgtHeader->typeHi);
    FxU16 swap = (rgtHeader->magic == IMAGIC); 
    int   x, y;
    
    if ( stream == NULL ) {
        txPanic("RGT file: Bad file handle.");
        return FXFALSE;
    }
    if (type & ITYPE_NCC) {
        txPanic("RGT file: RGT NCC files not supported.");
        return FXFALSE;
    }
    if (type & ITYPE_RLE) {
        txPanic("RGT file: RGT RLE files not supported.");
        return FXFALSE;
    }
    
    // load rgt, rgt's are bottom up
    for ( y = 0; y < info->height; y++ ) {
        FxU32 *data32;
        int     r, g, b, a;

        data32 = (FxU32 *) info->data[0];
        data32 += info->width * (info->height -1 - y);

        // Read scanline worth of data.
        for (x=0; x < info->width; x++) {
            /* Read b, g, r, a  from disk.*/
            r = getc( stream );
            g = getc( stream );
            b = getc( stream );
            a = getc( stream );

            if ( a == EOF ) {
                txPanic("RGT file: Unexpected End of File.");
                return FXFALSE;
            }
            data32[x] = (a << 24) | (r << 16) | (g << 8) | b;
        }

#if 1
        if (swap) {
            swapRGB((unsigned int *)data32, (int)info->width);
        }
#endif

#if 0
        if (type & ITYPE_BGR) {
            /* Swap just blue & red channels */
            swapRGB(data32, info->width);
        }
#endif
    }
    return FXTRUE;
}
