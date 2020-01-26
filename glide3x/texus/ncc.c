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

static int 
dithmat[4][4] = { {0,  8,  2, 10},
                  {12, 4, 14,  6},
                  {3, 11,  1,  9},
                  {15, 7, 13,  5} };

static struct   {
    int yhist[256], ihist[256], qhist[256];
    int ymin, ymax, imin, imax, qmin, qmax;
    int npixels;
    int y[16], a[3*4], b[3*4];          // please leave these contiguous
} ncc;

static int
_txPixQuantize_YIQ422 (unsigned int argb, int x, int y, int w)
{
    int         r, g, b; 
    int         iy, ii, iq;

    r = (argb >> 16) & 0xFF;
    g = (argb >>  8) & 0xFF;
    b = (argb      ) & 0xFF;

    iy = (int) (( 0.30F * r + 0.59F * g + 0.11F * b) + 0.5f);
    ii = (int) (((0.60F * r - 0.28F * g - 0.32F * b) / 1.20F) + 127.5f + 0.5f);
    iq = (int) (((0.21F * r - 0.52F * g + 0.31F * b) / 1.04F) + 127.5f + 0.5f);

    // At this point, 0<=y,i,q<=255
    // Convert to 4 bits of y, 2 of i and q.

    if (iy <= ncc.ymin) iy = 0;
    else if (iy >= ncc.ymax) iy = 15;
    else iy = (int) ((iy - ncc.ymin) * 15.0f/(ncc.ymax - ncc.ymin) + 0.5f);

    if (ii <= ncc.imin) ii = 0;
    else if (ii >= ncc.imax) ii = 3;
    else ii = (int) ((ii - ncc.imin) *  3.0f/(ncc.imax - ncc.imin) + 0.5f);

    if (iq <= ncc.qmin) iq = 0;
    else if (iq >= ncc.qmax) iq = 3;
    else iq = (int) ((iq - ncc.qmin) *  3.0f/(ncc.qmax - ncc.qmin) + 0.5f);

    if ((iy < 0) || (iy > 15) || (ii < 0) || (ii > 3) || (iq < 0) || (iq > 3)) {
        printf("%d %d %d\n", iy, ii, iq);
        txPanic("Bad YIQ\n");
    }
    return ( (iy << 4) | (ii << 2) | iq);
}


static int
_txPixQuantize_YIQ422_D4x4 (unsigned int argb, int x, int y, int w)
{
    int         r, g, b;
    int         iy, ii, iq;


    r = (argb >> 16) & 0xFF;
    g = (argb >>  8) & 0xFF;
    b = (argb      ) & 0xFF;

    iy = (int) (( 0.30F * r + 0.59F * g + 0.11F * b) + 0.5f);
    ii = (int) (((0.60F * r - 0.28F * g - 0.32F * b) / 1.20F) + 127.5f + 0.5f);
    iq = (int) (((0.21F * r - 0.52F * g + 0.31F * b) / 1.04F) + 127.5f + 0.5f);

    // At this point, 0<=y,i,q<=255
    // Convert to 4 bits of y, 2 of i and q.

    if (iy <= ncc.ymin) iy = 0;
    else if (iy >= ncc.ymax) iy = 0xf0;
    else iy = (int) ((iy - ncc.ymin) * 0xF0/(ncc.ymax - ncc.ymin));

    if (ii <= ncc.imin) ii = 0;
    else if (ii >= ncc.imax) ii = 0x30;
    else ii = (int) ((ii - ncc.imin) * 0x30/(ncc.imax - ncc.imin));

    if (iq <= ncc.qmin) iq = 0;
    else if (iq >= ncc.qmax) iq = 0x30;
    else iq = (int) ((iq - ncc.qmin) * 0x30/(ncc.qmax - ncc.qmin));

    iy += dithmat[y&3][x&3];
    ii += dithmat[y&3][x&3];
    iq += dithmat[y&3][x&3];

    iy >>= 4;
    ii >>= 4;
    iq >>= 4;

    if ((iy < 0) || (iy > 15) || (ii < 0) || (ii > 3) || (iq < 0) || (iq > 3)) {
        printf("%d %d %d\n", iy, ii, iq);
        txPanic("Bad YIQ\n");
    }
    return ( (iy << 4) | (ii << 2) | iq);
}

static void
_txImgNcc(char *odata, unsigned int *idata, int w, int h, int format, 
    int dither)
{
    int (*quantizer)(unsigned int argb, int x, int y, int w);
    int         x, y, pixsize;

    quantizer = (dither) ? _txPixQuantize_YIQ422_D4x4 : _txPixQuantize_YIQ422;

    pixsize = (format == GR_TEXFMT_YIQ_422) ? 1 : 2;

    for (y=0; y<h; y++) {
        for (x=0; x<w; x++) {
                if (format == GR_TEXFMT_AYIQ_8422) {
                        *(FxU16 *)odata = (FxU16) ((*quantizer)(*idata,x,y,w) |
                                                   ((*idata >> 16) & 0xFF00));
                } else {
                        *odata = (*quantizer)(*idata, x, y, w);
                }
                odata += pixsize;
                idata ++;
        }
    }
}

/*
   The basics are as follows:

   RGB values can be converted to YIQ using the equation:
  
   Y = 0.30F * R + 0.59F * G + 0.11F * B;
   I = 0.60F * R - 0.28F * G - 0.32F * B;
   Q = 0.21F * R - 0.52F * G + 0.31F * B;

   Assuming that each of the RGB components are in the range 0..255, 
   Y ranges from (0 .. 255)
   I ranges from (-0.60 * 255 .. 0.60 * 255) i.e, (-153 to 153)
   Q ranges from (-0.52 * 255 .. 0.52 * 255) i.e, (-132 to 133)

*/


static void     
_txMipNccStatTable(TxMip *txMip)
{
    int i, w, h;

    for (i=0; i<256; i++) 
        ncc.yhist[i] = ncc.ihist[i] = ncc.qhist[i] = 0;
    ncc.npixels = 0;

    /* First find out the relative frequencies of Y, I, Q */
    w = txMip->width;
    h = txMip->height;

    for (i=0; i< txMip->depth; i++) {
        FxU32*  src;
        int             npixels;

        src = txMip->data[i];
        npixels = w * h;
        ncc.npixels += npixels;

        while (npixels--) {
                float   fy, fi, fq, r, g, b;
                FxU32   argb;

                argb = *src++;
                r = (float)((argb >> 16) & 0xFF);
            g = (float)((argb >>  8) & 0xFF);
            b = (float)((argb      ) & 0xFF);

            fy = 0.30F * r + 0.59F * g + 0.11F * b;
            fi = 0.60F * r - 0.28F * g - 0.32F * b;
            fq = 0.21F * r - 0.52F * g + 0.31F * b;

            // Y is now in the range [0,1] * 255, but I and Q need work
            // I is in the range [-.60, +.60] * 255, convert to [0,1]*255
            // Q is in the range [-.52, +.52] * 255, convert to [0,1]*255

            fi = fi * 0.50f / 0.60f + 127.5f;
            fq = fq * 0.50f / 0.52f + 127.5f;

                ncc.yhist[ (int)(fy + 0.5f)]++;
                ncc.ihist[ (int)(fi + 0.5f)]++;
                ncc.qhist[ (int)(fq + 0.5f)]++;
        }
        if (w > 1) w >>= 1;
        if (h > 1) h >>= 1;
    }

    /* Now discard the top 10%, bottom 10% of each channel */
    {
        int     ysum, isum, qsum, threshold;

        ncc.ymin = ncc.imin = ncc.qmin = 0;
        ncc.ymax = ncc.imax = ncc.qmax = 255;
        threshold = (int) (0.01f * ncc.npixels);

        ysum = isum = qsum = 0;
        for (i=0; i<256; i++) {
                ysum += ncc.yhist[i];
                isum += ncc.ihist[i];
                qsum += ncc.qhist[i];
                if (ysum < threshold) ncc.ymin = i;
                if (isum < threshold) ncc.imin = i;
                if (qsum < threshold) ncc.qmin = i;
        }

        ysum = isum = qsum = 0;
        for (i=255; i>=0; i--) {
                ysum += ncc.yhist[i];
                isum += ncc.ihist[i];
                qsum += ncc.qhist[i];
                if (ysum < threshold) ncc.ymax = i;
                if (isum < threshold) ncc.imax = i;
                if (qsum < threshold) ncc.qmax = i;
        }

        if (ncc.ymin > ncc.ymax) ncc.ymin = ncc.ymax;
        if (ncc.imin > ncc.imax) ncc.imin = ncc.imax;
        if (ncc.qmin > ncc.qmax) ncc.qmin = ncc.qmax;
    }
}


void    
txMipNcc(TxMip *pxMip, TxMip *txMip, int format, FxU32 dither, FxU32 compression)
{
    int         i, w, h, pixsize;

    switch (compression & TX_COMPRESSION_MASK) {
    case        TX_COMPRESSION_HEURISTIC:                               
                _txMipNccStatTable(txMip);
                for (i=0; i< 16; i++) pxMip->pal[ 0 + i] = ncc.y[i];
                for (i=0; i< 12; i++) pxMip->pal[16 + i] = ncc.a[i];
                for (i=0; i< 12; i++) pxMip->pal[28 + i] = ncc.b[i];
                txMipNccNNet(pxMip, txMip, format, dither, compression);
                return;

    case        TX_COMPRESSION_STATISTICAL:
                if( txVerbose )
                  printf("Statistical tables\n");
                _txMipNccStatTable(txMip);
                break;

#if 0
    // This disabled, because it sucks.
    case        TX_COMPRESSION_YIQ:
                if( txVerbose )
                  printf("YIQ tables\n");
                ncc.ymin = ncc.imin = ncc.qmin = 0;
                ncc.ymax = ncc.imax = ncc.qmax = 255;
                break;
#endif
    }
    pixsize = (format == GR_TEXFMT_YIQ_422) ? 1 : 2;

    /* Generate the YAB tables */
    for (i=0; i<16; i++) {
        ncc.y[i] = (int) (ncc.ymin + (ncc.ymax - ncc.ymin)*i/15.0f + 0.5f);
    }

    for (i=0; i<4; i++) {
        float   a, b;

        a = ncc.imin + (ncc.imax - ncc.imin)*i/3.0f;
        a = (a / 255.0f) * 1.20f - 0.60f;                               // a is (-0.60, +0.60)
        a *= 255.0f;

        b = ncc.qmin + (ncc.qmax - ncc.qmin)*i/3.0f;
        b = (b / 255.0f) * 1.04f - 0.52f;                               // b is (-0.52, +0.52)
        b *= 255.0f;

        ncc.a[3*i + 0] = (int) ( 0.95f * a + 0.5f);
        ncc.a[3*i + 1] = (int) (-0.28f * a + 0.5f);
        ncc.a[3*i + 2] = (int) (-1.11f * a + 0.5f);

        ncc.b[3*i + 0] = (int) ( 0.62f * b + 0.5f);
        ncc.b[3*i + 1] = (int) (-0.64f * b + 0.5f);
        ncc.b[3*i + 2] = (int) ( 1.73f * b + 0.5f);
    }


    if ((dither & TX_DITHER_MASK) == TX_DITHER_ERR) {
        txYABtoPal256((int *)pxMip->pal, (int *) &ncc.y[0]);
        txDiffuseIndex(pxMip, txMip, pixsize, pxMip->pal, 256);
    }
    else {

        /* For each mipmap, translate input RGB values to YIQ, and quantize */
        /* Optionally, dither using 4x4 dither matrix */
        /* Return the quantized YIQ values */

        w = txMip->width;
        h = txMip->height;

        for (i=0; i<txMip->depth; i++) {
                _txImgNcc(pxMip->data[i], txMip->data[i], w, h,format,dither);
                if (w > 1) w >>= 1;
                if (h > 1) h >>= 1;
        }
    }

    // Copy decompression table.
    for (i=0; i<16; i++) pxMip->pal[   i] = ncc.y[i];
    for (i=0; i<12; i++) pxMip->pal[16+i] = ncc.a[i];
    for (i=0; i<12; i++) pxMip->pal[28+i] = ncc.b[i];
}
