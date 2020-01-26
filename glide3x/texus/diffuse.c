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

// Error diffusion is implemented here.

static int      ErrR[MAX_TEXWIDTH], ErrG[MAX_TEXWIDTH], ErrB[MAX_TEXWIDTH];

#if FAST_DIFFUSION
int     nsorted;
FxU32   sortR[256], sortG[256], sortB[256];
FxU8    bestR[256], bestG[256], bestB[256];

static int 
_txAscendingR(const void *a, const void *b)
{
    return ((*(FxI32 *)a) & 0x00FF0000) - ((*(FxI32 *)b) & 0x00FF0000);
}


static int 
_txAscendingG(const void *a, const void *b)
{
    return ((*(FxI32 *)a) & 0x0000ff00) - ((*(FxI32 *)b) & 0x0000ff00);
}


static int 
_txAscendingB(const void *a, const void *b)
{
    return ((*(FxI32 *)a) & 0x000000ff) - ((*(FxI32 *)b) & 0x000000ff);
}

static void
_txMakeRange(const FxU32 *palette, int ncolors)
{
    int i, j, mindist, minpos, d;
    FxU32 *pal;


    for (i=0; i<ncolors; i++) {
        sortR[i] = (palette[i] & 0x00ffffff) | (i<<24);
        sortG[i] = (palette[i] & 0x00ffffff) | (i<<24);
        sortB[i] = (palette[i] & 0x00ffffff) | (i<<24);
    }

    qsort(sortR, ncolors, sizeof(FxU32), _txAscendingR);
    qsort(sortG, ncolors, sizeof(FxU32), _txAscendingG);
    qsort(sortB, ncolors, sizeof(FxU32), _txAscendingB);
    nsorted = ncolors;

#if 0
    for (i=0; i<ncolors; i++) 
        printf("[%3d] = R%.08x G%.08x B%.08x\n", 
            i, sortR[i], sortG[i], sortB[i]);
#endif

 
    for (i=0; i<256; i++) {

        /* Find index of best matching Red component given r=i */
        pal = sortR;
        minpos  = mindist = 0x7fffffff;
        for (j=0; j < ncolors; j++) {
            d = DISTANCE(i, 0, 0, (pal[j]>>16) & 0xff, 0, 0);
            if (d < mindist) { mindist = d; minpos  = j; }
        }
        bestR[i] = minpos;


        /* Find index of best matching Grn component given g=i */
        pal = sortG;
        minpos  = mindist = 0x7fffffff;
        for (j=0; j < ncolors; j++) {
            d = DISTANCE(0, i, 0, 0, (pal[j]>>8) & 0xff, 0);
            if (d < mindist) { mindist = d; minpos  = j; }
        }
        bestG[i] = minpos;


        /* Find index of best matching Blu component given b=i */
        pal = sortB;
        minpos  = mindist = 0x7fffffff;
        for (j=0; j < ncolors; j++) {
            d = DISTANCE(0, 0, i, 0, 0, (pal[j]) & 0xff);
            if (d < mindist) { mindist = d; minpos  = j; }
        }
        bestB[i] = minpos;
    }
}

static int
_txFastMatch(int r, int g, int b)
{
    int minpos, mindist, i, d, persist;
    FxU32 *pal;

    minpos = mindist = 0x7fffffff;

    /* Walk backwards from bestR, tracking best index */
    pal = sortR;
    persist = 0;
    for (i=bestR[r]; i>=0; i--) {
        d = DISTANCE(r, g, b, 
            ((pal[i] >> 16) & 0xff), ((pal[i] >> 8) & 0xff), ((pal[i]) & 0xff));
        if (d < mindist) { mindist = d; minpos = (pal[i] >> 24) & 0xff; }
        else if (++persist > 3) break;
    }

    /* Walk forwards from bestR+1, tracking best index */
    persist = 0;
    for (i=bestR[r]+1; i < nsorted; i++) {
        d = DISTANCE(r, g, b, 
            ((pal[i] >> 16) & 0xff), ((pal[i] >> 8) & 0xff), ((pal[i]) & 0xff));
        if (d < mindist) { mindist = d; minpos = (pal[i] >> 24) & 0xff; }
        else if (++persist > 3) break;
    }


    /* Walk backwards from bestG, tracking best index */
    pal = sortG;
    persist = 0;
    for (i=bestG[g]; i>=0; i--) {
        d = DISTANCE(r, g, b, 
            ((pal[i] >> 16) & 0xff), ((pal[i] >> 8) & 0xff), ((pal[i]) & 0xff));
        if (d < mindist) { mindist = d; minpos = (pal[i]>>24) & 0xff; }
        else if (++persist > 3) break;
    }

    /* Walk forwards from bestG+1, tracking best index */
    persist = 0;
    for (i=bestG[g]+1; i < nsorted; i++) {
        d = DISTANCE(r, g, b, 
            ((pal[i] >> 16) & 0xff), ((pal[i] >> 8) & 0xff), ((pal[i]) & 0xff));
        if (d < mindist) { mindist = d; minpos = (pal[i]>>24) & 0xff; }
        else if (++persist > 3) break;
    }

    /* Walk backwards from bestB, tracking best index */
    pal = sortB;
    persist = 0;
    for (i=bestB[b]; i>=0; i--) {
        d = DISTANCE(r, g, b, 
            ((pal[i] >> 16) & 0xff), ((pal[i] >> 8) & 0xff), ((pal[i]) & 0xff));
        if (d < mindist) { mindist = d; minpos = (pal[i]>>24) & 0xff; }
        else if (++persist > 3) break;
    }

    /* Walk forwards from bestB+1, tracking best index */
    persist = 0;
    for (i=bestB[b]+1; i < nsorted; i++) {
        d = DISTANCE(r, g, b, 
            ((pal[i] >> 16) & 0xff), ((pal[i] >> 8) & 0xff), ((pal[i]) & 0xff));
        if (d < mindist) { mindist = d; minpos = (pal[i]>>24) & 0xff; }
        else if (++persist > 3) break;
    }
    return minpos;
}
#endif  // FAST_DIFFUSION


static void
_txToDiffuseIndex (FxU8 *opixels, int pixsize, const FxU32 *pal, int ncolors,
    const FxU32 *ipixels, int width, int height)
{
    int         y, x, i;
    int         qr, qg, qb;

    for (y=0; y<height; y++) {

      if( txVerbose )
        {
          if (y == (3*height)/4) { printf("."); fflush(stdout);}
          if (y == (2*height)/4) { printf("."); fflush(stdout);}
          if (y == (1*height)/4) { printf("."); fflush(stdout);}
          if (y == (0*height)/4) { printf("."); fflush(stdout);}
        }

        qr = qg = qb = 0;
        for (i=0; i<=width; i++) ErrR[i] = ErrG[i] = ErrB[i] = 0;

        for (x=0; x<width; x++) {
            int ia, ir, ig, ib, c;

            // The input color, and add diffused errors to these.
            ia = (*ipixels >> 24) & 0xFF;
            ir = (*ipixels >> 16) & 0xFF;
            ig = (*ipixels >>  8) & 0xFF;
            ib = (*ipixels      ) & 0xFF;
            ipixels ++;

            ir += qr + ErrR[x];
            ig += qg + ErrG[x];
            ib += qb + ErrB[x];

            qr = ir;    // quantized pixel values. 
            qg = ig;    // qR is error from pixel to left, errR is
            qb = ib;    // error from pixel to the top & top left.

            if (qr < 0) qr = 0; if (qr > 255) qr = 255; // clamp.
            if (qg < 0) qg = 0; if (qg > 255) qg = 255;
            if (qb < 0) qb = 0; if (qb > 255) qb = 255;

            // Find closest color in the tables, and quantize.
            c = txNearestColor(qr, qg, qb, pal, ncolors);
#if FAST_DIFFUSION
            // 3 times faster, but not as good quality.
            c = _txFastMatch(qr, qg, qb);
#endif

            qr = (pal[c] & 0x00ff0000) >> 16;
            qg = (pal[c] & 0x0000ff00) >>  8;
            qb = (pal[c] & 0x000000ff)      ;

            // Diffuse the errors.
            qr = ir - qr;
            qg = ig - qg;
            qb = ib - qb;

            // 3/8 (=0.375) to the EAST, 3/8 to the SOUTH, 
            // 1/4 (0.25) to the SOUTH-EAST.
            ErrR[x]  = ((x == 0) ? 0 : ErrR[x]) + ((int) (qr * 0.375f));
            ErrG[x]  = ((x == 0) ? 0 : ErrG[x]) + ((int) (qg * 0.375f));
            ErrB[x]  = ((x == 0) ? 0 : ErrB[x]) + ((int) (qb * 0.375f));

            ErrR[x+1] = (int) (qr * 0.250f);
            ErrG[x+1] = (int) (qg * 0.250f);
            ErrB[x+1] = (int) (qb * 0.250f);

            qr = (int) (qr * 0.375f);           // Carried to the right.
            qg = (int) (qg * 0.375f);
            qb = (int) (qb * 0.375f);

            if (pixsize == 2) {
                *(FxU16 *) opixels = (ia << 8) | c;
                opixels += pixsize;
            } else {
                *opixels++ = (FxU8) c;
            }
        }
    }
}

void
txDiffuseIndex(TxMip *pxMip, TxMip *txMip, int pixsize, const FxU32 *palette,
    int ncolors)
{
    int i, w, h;

    if( txVerbose )
      {
        printf("EDiffusion:..."); fflush(stdout);
      }
        
#if     FAST_DIFFUSION
    _txMakeRange(palette, ncolors);
#endif

    /* Translate image to an indexed image using error diffusion */
    w = txMip->width;
    h = txMip->height;

    for (i=0; i<txMip->depth; i++) {
        _txToDiffuseIndex(pxMip->data[i], pixsize, palette, ncolors, 
            txMip->data[i], w, h);
            if (w > 1) w >>= 1;
            if (h > 1) h >>= 1;
    }
    if( txVerbose )
      {
        printf("done\n");
      }
}
