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
 * For resampling in the x direction:
 * Assume ix input pixels become ox output pixels.
 * Imagine that ix input pixels are each divided into ox fragments, for a total
 * of ix * ox fragments.
 * Imagine also that ox output pixels are each divided into ix fragments, for a
 * total of ox * ix fragments, same as before.
 * Initialize an accumulator to 0. Add the first input pixel, multiplied by ox
 * the number of fragments per input pixel. Keep track of the number of 
 * fragments in the accumulator; when this is >= ix, (the number of fragments
 * it takes to make an output pixel), multiply the accumulator by 
 */

static void
_txResampleX(FxU32 *out, const FxU32 *in, int ox, int ix)
{
    FxU32 accr, accg, accb, acca, r, g, b, a;
    int   i, accf, o, nf;

    // printf("\n");
    accf = accr = accg = accb = acca = o = 0;

    for (i=0; i<ix; i++) {
        a = (in[i] & 0xff000000) >> 24;
        r = (in[i] & 0x00ff0000) >> 16;
        g = (in[i] & 0x0000ff00) >>  8;
        b = (in[i] & 0x000000ff)      ;

    // Each input pixel brings ox fragments
    nf = ox;

    while ((accf + nf) >= ix) {
        int          ef;
        int    oa, or, og, ob;

        // Yes, we have (possibly more than) enough to generate an output 
        // pixel. Of the nf new fragments, use up enough to generate an 
        // output pixel.

        ef = ix - accf;         // the excessive # of fragments.

            // printf("New: accf = %3d, nf = %3d, ef = %3d, ix = %3d, ox = %3d\n", 
    //  accf, nf, ef, ix, ox);

        acca += a * ef;
            accr += r * ef;
            accg += g * ef;
            accb += b * ef;

        oa = acca / ix;
        or = accr / ix;
        og = accg / ix;
        ob = accb / ix;

        if( (oa < 0) || (oa > 255) ||
            (or < 0) || (or > 255) ||
        (og < 0) || (og > 255) ||
        (ob < 0) || (ob > 255) ) {

        printf(" %d %d %d %d\n" , oa, or, og, ob);
        txPanic("ARGB: out of range\n");
        }

        *out++ = (oa << 24) | (or << 16) | (og << 8) | ob;
        // printf("Output pixel %4d: %.02x %.02x %.02x %.02x\n", 
           //  o, oa, or, og, ob);
        o++;
        acca = accr = accg = accb = accf  = 0;
        nf -= ef;
    }

    // If there's any fragments left over, accumulate them.
    if (nf) {
            acca += a * nf;
            accr += r * nf;
            accg += g * nf;
            accb += b * nf;
        accf += nf;
            // printf("i= %4d, accf = %4d, aa=%.06x, ar=%.06x, ag=%.06x, ab=%.06x\n",
        //      i, accf, acca, accr, accg, accb);
    }
    }
    if (accf != 0) {
        txPanic("Row resampling: accf != 0!\n");
    }
}

static  FxU32 AccA[MAX_TEXWIDTH];
static  FxU32 AccR[MAX_TEXWIDTH];
static  FxU32 AccG[MAX_TEXWIDTH];
static  FxU32 AccB[MAX_TEXWIDTH];
static  FxU32 argb[MAX_TEXWIDTH];

static void
_txImgResample(FxU32 *out, int ox, int oy, 
               const FxU32 *in, int ix, int iy)
{
    int r, g, b, a;
    int   i, j, accf, o, nf;

    for (i=0; i<ox; i++) AccA[i] = AccR[i] = AccG[i] = AccB[i] = 0;

    accf = 0;
    o = 0;
    for (i=0; i<iy; i++) {

        // Resample a row of input into temporary array.
    // printf("Resampling input row %4d\n", i);
    _txResampleX( argb, in, ox, ix);
    in += ix;

    // This row brings in oy fragments per scanline.
    nf = oy;

    while ((accf + nf) >= iy) {
        int          ef;

        // Yes, we have (possibly more than) enough to generate an output 
        // pixel. Of the nf new fragments, use up enough to generate an 
        // output pixel.

        ef = iy - accf;         // the excessive # of fragments.

        // Accumulate  input * ef + acc, and generate a line of output.
        for (j=0; j<ox; j++) {

                a = (argb[j] & 0xff000000) >> 24;
                r = (argb[j] & 0x00ff0000) >> 16;
                g = (argb[j] & 0x0000ff00) >>  8;
                b = (argb[j] & 0x000000ff)      ;

        AccA[j] += a * ef;
        AccR[j] += r * ef;
        AccG[j] += g * ef;
        AccB[j] += b * ef;

        a = AccA[j] / iy;
        r = AccR[j] / iy;
        g = AccG[j] / iy;
        b = AccB[j] / iy;

        if( (a < 0) || (a > 255) ||
            (r < 0) || (r > 255) ||
            (g < 0) || (g > 255) ||
            (b < 0) || (b > 255) ) {
                printf(" %d %d %d %d\n" , a, r, g, b);
                txPanic("ARGB: out of range\n");
            }
        out[j] = (a << 24) | (r << 16) | (g << 8) | b;
        AccA[j] = 0;
        AccR[j] = 0;
        AccG[j] = 0;
        AccB[j] = 0;

        }
        out += ox;
        accf = 0;
        nf  -= ef;
        // printf("[%4d] Generating output row %4d\n", i, o);
        o++;
    }

    // If there's any fragments left over, accumulate them.
    if (nf) {
        for (j=0; j<ox; j++) {
                a = (argb[j] & 0xff000000) >> 24;
                r = (argb[j] & 0x00ff0000) >> 16;
                g = (argb[j] & 0x0000ff00) >>  8;
                b = (argb[j] & 0x000000ff)      ;

        AccA[j] += a * nf;
        AccR[j] += r * nf;
        AccG[j] += g * nf;
        AccB[j] += b * nf;
        }
        accf += nf;
            // printf("i= %4d, accf = %4d\n", i, accf);
    }
    }
    if (accf != 0) {
        txPanic("Img resampling: accf != 0!\n");
    }
    // Ideally, accf must be 0 now.
    // printf("Finally: accf = %d\n", accf);
}

void
txMipResample(TxMip *destMip, TxMip *srcMip)
{
        int             i, sw, sh, dw, dh;


        if ((destMip->width > MAX_TEXWIDTH) || (destMip->height > MAX_TEXWIDTH)) {
                txPanic("Bad width/height in txImageResize()\n");
        }
        if ((srcMip->format != GR_TEXFMT_ARGB_8888) ||
            (destMip->format != GR_TEXFMT_ARGB_8888)) {
                txPanic("Bad image format in txMipResample.");
        }

        if ((srcMip->width == destMip->width) && (srcMip->height == destMip->height) &&
            (srcMip->data[0] == destMip->data[0])) {
                if( txVerbose )
                  printf("No Resampling necessary.\n");
                return;
        }

        if ((srcMip->data[0] == NULL) || (destMip->data[0] == NULL))
                txPanic("txImageResize: Null buffer\n");

        if( txVerbose )
          printf("Resampling to %dx%d: ", destMip->width, destMip->height);

        
        sw = srcMip->width;
        sh = srcMip->height;
        dw = destMip->width;
        dh = destMip->height;

        for (i=0; i< srcMip->depth; i++) {
                if(!destMip->data[i])
                        txPanic("txImageResize: no miplevel present\n");
                _txImgResample (destMip->data[i], dw, dh,
                                srcMip->data[i], sw, sh);
                if( txVerbose )
                  {
                    printf(" %dx%d", sw, sh); fflush(stdout);
                  }
                if (sw > 1) sw >>= 1;
                if (sh > 1) sh >>= 1;
                if (dw > 1) dw >>= 1;
                if (dh > 1) dh >>= 1;
        }
        if( txVerbose )
          printf(".\n");
}
