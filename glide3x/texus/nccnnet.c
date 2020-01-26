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
 * This file implements the neural net quantizer, which takes an image in
 * ARGB8888 format and produces an optimal YAB table, and an 8 bit image
 * in YAB format that best represents the original image. A very detailed
 * explanation of the algorithm is available in 
 * /tdfx/engr/devel/sst1/docs/yab.doc. The summary follows.
 *
 * Neural net algorithms first determine a "representative sample" of the 
 * input image. This representative sample is repeatedly run through the net
 * during the network learning stage, and the neural net "learns" which colors
 * are important and which ones are not. It's quite possible to feed every
 * pixel in the original image repeatedly into the neural net
 * to make it learn; however, this can be extremely time consuming. 
 *
 * So, we prefer to make a representative sample of colors for the input image.
 * The original yab.doc suggests we try to derive sample colors by reducing the
 * ARGB8888 colors to RGB555. I've found that simply color quantizing to 256
 * colors (just like for the 8-bit palettized case) works quite well, and
 * so we first quantize to 8 bit palette, and use the palette as the sample
 * colors to feed the neural network. This also makes using 256-palette 
 * textures very easy.
 *
 * After the "representative colors" are determined, we train the neural net,
 * and obtain the optimal YAB table.  Each sample color in the palette, 
 * which was originally in ARGB8888 format is now replaced with IRGB8888, 
 * where the RGB is the same as before, but the alpha channel is replaced 
 * with an 8 bit number I which is the YAB index corresponding to this 
 * representative color. 
 *
 * So now it's possible to translate the original image into an 8 bit image
 * by first looking up the original pixel in the representative colors table,
 * extracting the alpha channel, and using it as the index into the YAB table
 * delivered by the algorithm.
 *
 * In the process of converting the original image to the YAB format, we could
 * optionally dither the image. Ordered dithering doesn't quite work, so we
 * use error-diffusion dithering. 
 *
 * I've found that there are three speed bottlenecks to overcome. The first
 * time consuming operation is the computation of representative image colors.
 * 256 color quantization is used for this part. The second bottleneck is the 
 * training of the neural net algorithm itself, and I've optimized this as 
 * much as possible. The third bottleneck is the translation of the original
 * image into the 8 bit YAB indexed image; this still needs work, especially
 * when error diffusion dithering is enabled.
 *
 * So, now, onto business.
 */

/******************************************************************************
 * 
 * The hardcore neural net stuff begins here.
 *
 */

#define MAX_ITERATIONS  4000
#define MAX_DRYSPELLS   2000
#define MAX_NEURONS             256

typedef struct  _weight {
    int         r,  g,  b;                             // fixed point, SUBPIXEL precision bits
    int         ir, ig, ib;                             // pure integers, maybe -256 to 255.
} Weight;

typedef struct  _vector {
    Weight      *py, *pa, *pb;
    int         r,  g,  b;                             // pure integers, 0 to 255.
} Neuron;

static  Weight                  Y[16], A[4], B[4];
static  Neuron                  N[MAX_NEURONS];
static  int                    errR, errG, errB, errMax;
static  int                    totR, totG, totB;


#define SUBPIXEL                22
#define ABS(x)                  (((x) < 0) ? (-(x)) : (x))
#define INT(x)                  ((x) >> SUBPIXEL)
#define  CLAMP_255(x)   if (x < 0) x = 0; else if (x > 255) x = 255
#define  CLAMP_PLUS(x)  if (x < 0) x = 0; else if (x > ((256 << SUBPIXEL)-1)) \
                                                x = ((256 << SUBPIXEL) -1)
#define  CLAMP_BOTH(x)  if (x < (-256 << SUBPIXEL)) x = (-256 << SUBPIXEL); \
                                                else if (x > ((256 << SUBPIXEL) - 1)) \
                                                x = ((256 << SUBPIXEL) -1)

static int
_nn_modifyNeurons(int ir, int ig, int ib)
{
    int         i; 
    int         d0, d1;                         // closest & next closest distance to input
    int         p0, p1;                         // index into the 256 color table.
    int        d, dr, dg, db;
    Weight      *py, *pa, *pb;
    Neuron      *n;

    d0 = d1 = 0x7fffffff;
    p0 = p1 = 0;

    /* Find 2 neurons with smallest distances to r, g, b */
    for (i=0, n=N; i<256; i++, n++) {
        py = n->py;
        pa = n->pa;
        pb = n->pb;

        n->r = py->ir + pa->ir + pb->ir; CLAMP_255(n->r);
        n->g = py->ir + pa->ig + pb->ig; CLAMP_255(n->g);
        n->b = py->ir + pa->ib + pb->ib; CLAMP_255(n->b);

        d = DISTANCE(n->r, n->g, n->b, ir, ig, ib);
        if (d < d0) { 
                d1 = d0; d0 = d;
                p1 = p0; p0 = i;

        } else if (d < d1) {
                d1 = d;
                p1 = i;
        }
    }

    /* track errors */
    dr = ABS(N[p0].r - ir);
    dg = ABS(N[p0].g - ig);
    db = ABS(N[p0].b - ib);

    totR += dr;
    totG += dg;
    totB += db;

    if (errMax < d0) {
        errMax = d0;
        errR   = dr;
        errG   = dg;
        errB   = db;
    }


    // Modify weights for d0 and d1 at positions p0 and p1.

    // The 16 comes from 8.16 format, 4 comes from alpha, the learing rate, set
    // at 1/16 for now.

    // py->r += (ar * 0.25f + ag * 0.5f + ab * 0.25f) * alpha;
    // pa->r += ar * 0.25f * alpha;
    // pb->r += ar * 0.25f * alpha;


    // For the closest neuron, apply maximal learning rate.
    dr = (ir - N[p0].r) << (SUBPIXEL - 1);
    dg = (ig - N[p0].g) << (SUBPIXEL - 1);
    db = (ib - N[p0].b) << (SUBPIXEL - 1);
    py = N[p0].py;
    pa = N[p0].pa;
    pb = N[p0].pb;
    py->r += (dr >> 2) + (dg >> 1) + (db >> 2); CLAMP_PLUS(py->r);
    pa->r += (dr >> 2) ; CLAMP_BOTH(pa->r); 
    pa->g += (dg >> 2) ; CLAMP_BOTH(pa->g);
    pa->b += (db >> 2) ; CLAMP_BOTH(pa->b);
    pb->r += (dr >> 2) ; CLAMP_BOTH(pb->r);
    pb->g += (dg >> 2) ; CLAMP_BOTH(pb->g);
    pb->b += (db >> 2) ; CLAMP_BOTH(pb->b);

    py->ir = INT(py->r);
    pa->ir = INT(pa->r); pa->ig = INT(pa->g); pa->ib = INT(pa->b);
    pb->ir = INT(pb->r); pb->ig = INT(pb->g); pb->ib = INT(pb->b);

    // For the next closest neuron, apply 1/4 the learning rate.
    dr = (ir - N[p1].r) << (SUBPIXEL - 2);
    dg = (ig - N[p1].g) << (SUBPIXEL - 2);
    db = (ib - N[p1].b) << (SUBPIXEL - 2);
    py = N[p1].py;
    pa = N[p1].pa;
    pb = N[p1].pb;
    py->r += (dr >> 2) + (dg >> 1) + (db >> 2); CLAMP_PLUS(py->r);
    pa->r += (dr >> 2) ; CLAMP_BOTH(pa->r);
    pa->g += (dg >> 2) ; CLAMP_BOTH(pa->g);
    pa->b += (db >> 2) ; CLAMP_BOTH(pa->b);
    pb->r += (dr >> 2) ; CLAMP_BOTH(pb->r);
    pb->g += (dg >> 2) ; CLAMP_BOTH(pb->g);
    pb->b += (db >> 2) ; CLAMP_BOTH(pb->b);

    py->ir = INT(py->r);
    pa->ir = INT(pa->r); pa->ig = INT(pa->g); pa->ib = INT(pa->b);
    pb->ir = INT(pb->r); pb->ig = INT(pb->g); pb->ib = INT(pb->b);

    return p0;          // best fit neuron is returned.
}

static  void
_nn_initTables()
{
    int         i;

    // Intensity ramp
    for (i=0; i<16; i++) {
        Y[i].r = ((int) ((255.0f * i)/15.0f))  << SUBPIXEL;
        Y[i].ir = INT(Y[i].r);
    }

    for (i=0; i< 4; i++) {

        A[i].r = A[i].g = A[i].b = 0;
        A[i].ir = INT(A[i].r);
        A[i].ig = INT(A[i].g);
        A[i].ib = INT(A[i].b);

        B[i].r = B[i].g = B[i].b = 0;
        B[i].ir = INT(B[i].r);
        B[i].ig = INT(B[i].g);
        B[i].ib = INT(B[i].b);
    }

    for (i=0; i<MAX_NEURONS; i++) {
        int     iy, ia, ib;

        iy = (i >> 4) & 0x0f;
        ia = (i >> 2) & 0x03;
        ib = (i >> 0) & 0x03;

        N[i].py = &Y[iy];
        N[i].pa = &A[ia];
        N[i].pb = &B[ib];

        N[i].r = INT(Y[iy].r)+INT(A[ia].r)+INT(B[ib].r);CLAMP_255(N[i].r);
        N[i].g = INT(Y[iy].r)+INT(A[ia].g)+INT(B[ib].g);CLAMP_255(N[i].g);
        N[i].b = INT(Y[iy].r)+INT(A[ia].b)+INT(B[ib].b);CLAMP_255(N[i].b);
    }
}

static  int order[256];
static int 
_nn_randomOrder(const void *a, const void *b)
{
    a = b;              // no compiler warnings
    return (rand() % 3) - 1;
}

static void
txMapPal256toYAB(FxU32 *YAB, FxU8 *map, int nsamples, FxU32 *samples)
{
    int         i;
    int        bstR, bstG, bstB, bstMax;
    int         iterations;                     // track how many inputs have been fed to NN
    int         drySpells;                      // how many inputs since last best case.
    int        yab2pal[256];

    _nn_initTables();
    /* 
     * Select a number which is relatively prime to nsamples.
     */
    for (i=0; i<nsamples; i++) order[i] = i;
    qsort(order, nsamples, sizeof(int), _nn_randomOrder);

    iterations = drySpells = 0;
    bstMax = 0x7fffffff;

    while ((iterations < MAX_ITERATIONS) && (drySpells < MAX_DRYSPELLS)) {

        // An epoch:
        // A whole cycle of inputs is presented to the network in an epoch.

        errR   = errG = errB = errMax = 0;
        totR   = totG = totB = 0;

        for (i = 0; i< nsamples; i++) {
                FxU32   *pRGB;

                // We present the samples randomly to the network. 
                // _nn_modify_neurons() makes the neurons learn
                // errR, errG, errB, errMax are computed in _nn_modifyNeurons(), as
                // are totR, totG, totB (accumulative errors).

                pRGB = (FxU32 *) (&samples[order[i]]);
                (void) _nn_modifyNeurons(((*pRGB >> 16) & 0xFF),
                    ((*pRGB >> 8) & 0xFF), ((*pRGB) & 0xFF));
        }

        iterations += nsamples;

        if (errMax < bstMax) {
                /* 
                 * A lower total error than before, take a Snapshot
                 *
                 * YAB[] has 16 entries for Y, 12 entries each for A & B.
                 */
                for (i=0; i<16; i++) {
                        YAB[i] = Y[i].ir;
                        if ((Y[i].ir < 0) || (Y[i].ir > 255)) {
                                txPanic("Bad Y!\n");
                        }
                }

                for (i=0; i<4; i++) {
                        YAB[16 + 3*i + 0] = A[i].ir;
                        YAB[16 + 3*i + 1] = A[i].ig;
                        YAB[16 + 3*i + 2] = A[i].ib;

                        if ((A[i].ir < -256) || (A[i].ir > 255) ||
                            (A[i].ig < -256) || (A[i].ig > 255) ||
                            (A[i].ib < -256) || (A[i].ib > 255)) txPanic("Bad A!\n");
                }
                for (i=0; i<4; i++) {
                        YAB[28 + 3*i + 0] = B[i].ir;
                        YAB[28 + 3*i + 1] = B[i].ig;
                        YAB[28 + 3*i + 2] = B[i].ib;

                        if ((B[i].ir < -256) || (B[i].ir > 255) ||
                            (B[i].ig < -256) || (B[i].ig > 255) ||
                            (B[i].ib < -256) || (B[i].ib > 255)) txPanic("Bad B!\n");
                }

                bstMax = errMax;
                bstR   = errR;
                bstG   = errG;
                bstB   = errB;
#if 0
                printf("%8d%, dry=%8d, eMax=%8x eMax=%3d%3d%3d eAvg=%3d%3d%3d\n", 
                   iterations, drySpells, errMax, 
                   errG, errR, errB, 
                   totG/nsamples, totR/nsamples, totB/nsamples
                );
#endif
                drySpells = 0;
        }
        else    {
                drySpells += nsamples;
        }

        if (errMax == 0) { 
                // printf("******Exact Solution in %d iterations\n", iterations); 
                // _nn_Dump(); 
                break;
        }
    }

    /* 
     * At this point YAB has the YAB table, samples has input palette,
     * Replace MSB of samples with index to be used with YAB table.
     */

    txYABtoPal256((int*)yab2pal, (int*)YAB);

    for (i=0; i<nsamples; i++) {
        int             ir, ig, ib;

        // The input color, and add diffused errors to these.
        ir = (samples[i] >> 16) & 0xFF;
        ig = (samples[i] >>  8) & 0xFF;
        ib = (samples[i]      ) & 0xFF;

        // Find closest color in the yab2pal tables
        map[i] = (FxU8) txNearestColor(ir, ig, ib, (const FxU32 *)yab2pal, 256);
    }
}

void
txMipNccNNet(TxMip *pxMip, TxMip *txMip, int format, FxU32 dither, FxU32 comp)
{
    int         i, w, h;
    int         ncolors;
    int         pixsize = (pxMip->format == GR_TEXFMT_YIQ_422) ? 1 : 2;
    int        yabTable[16+12+12];
    FxU8        map[256];


    /* 
     * Get a 256 color palette, to be used as samples 
     * Incidentally, convert src 32 bit image to dst 8 bit indexed image,
     * with indices referring to the 256 color palette.
     * Also incidentally, pack the alpha channel if necessary.
     */
    if( txVerbose )
      {
        printf("NCC Neural nets..."); fflush(stdout);
      }
    pxMip->format = (format == GR_TEXFMT_YIQ_422) ? GR_TEXFMT_P_8 : 
        GR_TEXFMT_AP_88;
    ncolors = txMipPal256(pxMip, txMip, pxMip->format, 0, 0);
    if( txVerbose )
      {
        printf("%d samples...", ncolors); fflush(stdout);
      }
    txMapPal256toYAB((FxU32 *)yabTable, (FxU8 *)map, ncolors, (FxU32 *)pxMip->pal);
    if( txVerbose )
      {
        printf("eMax=(%3d%3d%3d)...eAvg=(%3d%3d%3d)\n",
               errG, errR, errB, 
               totG/ncolors, totR/ncolors, totB/ncolors
               );
      }


    if ((dither & TX_DITHER_MASK) != TX_DITHER_NONE) {
        /*
         * At this point, we can lose the 256 color palette, and replace it with
         * the 256 color palette generated from the YAB table. This will be 
         * useful for error diffusion dithering.
         */
        txYABtoPal256((int *)pxMip->pal, (int *)yabTable);
        txDiffuseIndex(pxMip, txMip, pixsize, pxMip->pal, 256);
    } 
    else {

        /* Translate image to YAB format */
        w = txMip->width;
        h = txMip->height;
        for (i=0; i< txMip->depth; i++) {
                int             npixels;
                FxU8    *src8;
                FxU16   *src16;

                npixels = w * h;

                if (pixsize == 2) {
                        src16 = (FxU16 *) pxMip->data[i];
                        while (npixels--) {
                                *src16 = (*src16 & 0xFF00) | (map[*src16 & 0x00FF]);
                                src16++;
                        }
                } else {
                        src8 = (FxU8 *) pxMip->data[i];
                        while (npixels--) {
                                *src8 = map[*src8];
                                src8++;
                        }
                }
                if (w > 1) w >>= 1;
                if (h > 1) h >>= 1;
        }
    }

    /* Copy the YAB table to pal. */
    pxMip->format = format;
    for (i=0; i< (16+12+12); i++)
        pxMip->pal[i] = yabTable[i];
}
