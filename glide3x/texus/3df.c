
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

/*************************************** 3df files ****************************/
/* Read word, msb first */
static FxBool 
_txRead16 (FILE *stream, FxU16* data)
{
    FxU8 byte[2];

    if (fread (byte, 2, 1, stream) != 1) return FXFALSE;
    *data = (((FxU16) byte[0]) << 8) | ((FxU16) byte[1]);
    return FXTRUE;
}

/* Read long word, msb first */
static FxBool 
_txRead32 (FILE *stream, FxU32* data)
{
    FxU8 byte[4];

    if (fread (byte, 4, 1, stream) != 1) return FXFALSE;

    *data = (((FxU32) byte[0]) << 24) |
            (((FxU32) byte[1]) << 16) |
            (((FxU32) byte[2]) <<  8) |
             ((FxU32) byte[3]);

    return FXTRUE;
}


/* Read NCC table */
static FxBool 
_txRead3DFNCCTable (FILE* stream, FxI32* ncc_table)
{
    FxU32 i;
    FxI16 data;

    /* read Y */
    for (i = 0; i < 16; i ++) {
        if (FXFALSE == _txRead16 (stream, (FxU16 *)&data)) return FXFALSE;
        ncc_table[i] = (FxU8) data;
    }
    /* read A */
    for (i = 0; i < 4; i++) {
        if (!_txRead16 (stream, (FxU16 *)&data)) return FXFALSE;
        ncc_table[16 + 3 * i + 0] = (FxI32) data;
        if (!_txRead16 (stream, (FxU16 *)&data)) return FXFALSE;
        ncc_table[16 + 3 * i + 1] = (FxI32) data;
        if (!_txRead16 (stream, (FxU16 *)&data)) return FXFALSE;
        ncc_table[16 + 3 * i + 2] = (FxI32) data;
    }

    /* read B */
    for (i = 0; i < 4; i++) {
        if (!_txRead16 (stream, (FxU16 *)&data)) return FXFALSE;
        ncc_table[28 + 3 * i + 0] = (FxI32) data;
        if (!_txRead16 (stream, (FxU16 *)&data)) return FXFALSE;
        ncc_table[28 + 3 * i + 1] = (FxI32) data;
        if (!_txRead16 (stream, (FxU16 *)&data)) return FXFALSE;
        ncc_table[28 + 3 * i + 2] = (FxI32) data;
    }

    return FXTRUE;
}

static FxBool 
_txRead3DFPalTable (FILE* stream, FxI32* pal)
{
    FxU32 i;

    /* read Y */
    for (i = 0; i < 256; i ++) {
        if (FXFALSE == _txRead32 (stream, (FxU32 *)&pal[i])) return FXFALSE;
    }
    return FXTRUE;
}

/* Read 3df header */
FxBool
_txRead3DFHeader(FILE* stream, FxU32 cookie, TxMip *txMip)
{
    int         c, i;
    char        version[6];
    char        color_format[10];
    int         lod_min, lod_max;
    int         aspect_width, aspect_height;
    int         w, h;

    /* magic cookie starts with 3df v%6...., of which we've eaten 3d */
    // printf("3df file header\n");
    if (fscanf (stream, "f v%6s", version) != 1) return FXFALSE;

    /* 
     * skip comments
     */
    while (((c = getc (stream)) != EOF) && (c == '#')) {
        while (((c = getc (stream)) != EOF) && (c != '\n'));
        if (c == EOF) return FXFALSE;
    }
    if (c == EOF) return FXFALSE;
    if (ungetc (c, stream) == EOF) return FXFALSE;

    /* color format, lod range, aspect ratio */
    if (5 != fscanf (stream, "%10s lod range: %i %i aspect ratio: %i %i",
        color_format, &lod_min, &lod_max, &aspect_width, &aspect_height))
                return FXFALSE;

    // printf("%s %d %d (lods) %d %d (aspect)\n", color_format, lod_min, lod_max, aspect_width, aspect_height);

    /* eat final nl */
    if (((c = getc (stream)) == EOF) && (c != '\n'))
        return FXFALSE;

    /* make sure null terminated */
    color_format[9] = 0;

    /* lookup name */
    for (i = 0; i <= GR_TEXFMT_ARGB_8888; i++)
        if (strcmp (Format_Name[i], color_format) == 0) break;
    if (i > GR_TEXFMT_ARGB_8888) return FXFALSE;
    txMip->format = i;

    /* Validate lod */
    if ((lod_max & (lod_max-1)) || (lod_min & (lod_min-1))) return FXFALSE;
    if ((lod_max > 256) || (lod_max < 1)) return FXFALSE;
    if ((lod_min > 256) || (lod_min < 1)) return FXFALSE;
    if (lod_min > lod_max) return FXFALSE;

    /* validate aspect ratio */
    w = h = lod_max;
    switch ((aspect_width << 4) | (aspect_height)) {
        case    0x81:   h = h/8; break;
        case    0x41:   h = h/4; break;
        case    0x21:   h = h/2; break;
        case    0x11:   h = h/1; break;
        case    0x12:   w = w/2; break;
        case    0x14:   w = w/4; break;
        case    0x18:   w = w/8; break;
        default:                return FXFALSE;
    }

    txMip->width  = w;
    txMip->height = h;
    txMip->size   = w * h;

    for (txMip->depth=1; lod_max > lod_min; lod_max >>= 1) {
        // printf("w = %d, h = %d, lod_max = %d\n", w, h, lod_max);
        txMip->depth++;
        if (w > 1) w >>= 1;
        if (h > 1) h >>= 1;
        txMip->size += (w * h);
    }
    txMip->size *= GR_TEXFMT_SIZE(txMip->format); // in bits per texel
    txMip->size >>= 3; // convert to bytes

    // printf("3df file: %dx%dx%d:%d\n", txMip->width, txMip->height, txMip->depth, txMip->size);
    return FXTRUE;
}

FxBool
_txRead3DFData( FILE *stream, TxMip *txMip)
{
    int i, npixels;

    // printf("3df file data\n");
    /* First read NCC tables */
    if ((txMip->format == GR_TEXFMT_YIQ_422) ||
        (txMip->format == GR_TEXFMT_AYIQ_8422)) {
        if (!_txRead3DFNCCTable (stream, (FxI32 *)txMip->pal)) {
                txError("Bad Ncc table\n");
                return FXFALSE;
        }
    }

    if ((txMip->format == GR_TEXFMT_P_8) ||
        (txMip->format == GR_TEXFMT_AP_88)) {
        if (!_txRead3DFPalTable (stream, (FxI32*)txMip->pal)) {
                txError("Bad Palette table\n");
                return FXFALSE;
        }
    }

    /* read mipmap image data */
    if (txMip->format < GR_TEXFMT_16BIT) {
        npixels = txMip->size;

        if ((FxU32) npixels != fread (txMip->data[0], 1, npixels, stream)) {
                txError("Bad 8 bit data");
                return FXFALSE;
        }
    }
    else if (txMip->format < GR_TEXFMT_32BIT) {
        FxU16* data = (FxU16 *) txMip->data[0];
        npixels = txMip->size >> 1;

        for (i = 0; i < npixels; i++, data++)
                if (FXFALSE == _txRead16 (stream, data)) {
                        txError("Bad 16 bit data");
                        return FXFALSE;
                }
    }
    else {
        FxU32* data = (FxU32*) txMip->data[0];
        npixels = txMip->size >> 2;

        for (i = 0; i < npixels; i ++, data++)
                if (FXFALSE == _txRead32 (stream, data)) {
                        txError("Bad 32 bit data");
                        return FXFALSE;
                }
    }
    return FXTRUE;
}
