
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
#include <assert.h>

#include "texusint.h"

/* 
 * Pn_8 = convert n bits (n <= 6) to 8 bits by replicating the msb's of input
 * into the lsb's of the output.
 */
static  FxU8    P1_8[] = {0x00,0xff};
static  FxU8    P2_8[] = {0x00,0x55,0xaa,0xff};
static  FxU8    P3_8[] = {0x00,0x24,0x49,0x6d,0x92,0xb6,0xdb,0xff};
static  FxU8    P4_8[] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
                                          0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
static  FxU8    P5_8[] = {0x00,0x08,0x10,0x18,0x21,0x29,0x31,0x39,
                                          0x42,0x4a,0x52,0x5a,0x63,0x6b,0x73,0x7b,
                                          0x84,0x8c,0x94,0x9c,0xa5,0xad,0xb5,0xbd,
                                          0xc6,0xce,0xd6,0xde,0xe7,0xef,0xf7,0xff};
static  FxU8    P6_8[] = {0x00,0x04,0x08,0x0c,0x10,0x14,0x18,0x1c,
                                          0x20,0x24,0x28,0x2c,0x30,0x34,0x38,0x3c,
                                          0x41,0x45,0x49,0x4d,0x51,0x55,0x59,0x5d,
                                          0x61,0x65,0x69,0x6d,0x71,0x75,0x79,0x7d,
                                          0x82,0x86,0x8a,0x8e,0x92,0x96,0x9a,0x9e,
                                          0xa2,0xa6,0xaa,0xae,0xb2,0xb6,0xba,0xbe,
                                          0xc3,0xc7,0xcb,0xcf,0xd3,0xd7,0xdb,0xdf,
                                          0xe3,0xe7,0xeb,0xef,0xf3,0xf7,0xfb,0xff};
static FxU32
_txPixRgb332to8888 (FxU8 c332)
{
    FxU32       a, r, g, b;
    a = 0xff;
    r = P3_8[(c332>>5)&7];
    g = P3_8[(c332>>2)&7];
    b = P2_8[(c332   )&3];
    return (a << 24) | (r << 16) | (g << 8) | b;
}

/* YIQ treated at the image level */

static FxU32
_txPixA8to8888 (FxU8 a8)
{
    FxU32       p = a8;

    return (p << 24) | (0xffffff);
}

static FxU32
_txPixI8to8888 (FxU8 i8)
{
    FxU32       p = i8;

    return (0xff << 24) | (p << 16) | (p << 8) | p;
}

static FxU32
_txPixAi44to8888 (FxU8 c44)
{
    FxU32       a, i;

    a = P4_8[(c44 & 0xF0) >> 4];
    i = P4_8[(c44 & 0x0F)     ];

    return (a << 24) | (i << 16) | (i << 8) | i;
}

/* P8 treated at the image level */

/* 16 bit pixels */
static FxU32 
_txPixArgb8332to8888(FxU16 c8332)
{
    FxU32       a, r, g, b;
    a =      (c8332 >>  8);      
    r = P3_8[(c8332 >>  5) & 0x7];
    g = P3_8[(c8332 >>  2) & 0x7];
    b = P2_8[(c8332      ) & 0x3];
    return (a << 24) | (r << 16) | (g << 8) | b;
}

/* AYIQ8422 treated at image level */

static FxU32
_txPixRgb565to8888 (FxU16 c565)
{
    FxU32       a, r, g, b;
    a = 0xFF;
    r = P5_8[(c565 >> 11)       ];
    g = P6_8[(c565 >>  5) & 0x3f];
    b = P5_8[(c565      ) & 0x1f];
    return (a << 24) | (r << 16) | (g << 8) | b;
}

static FxU32
_txPixArgb1555to8888 (FxU16 c1555)
{
    FxU32       a, r, g, b;
    a = P1_8[(c1555 >> 15)       ];
    r = P5_8[(c1555 >> 10) & 0x1f];
    g = P5_8[(c1555 >>  5) & 0x1f];
    b = P5_8[(c1555      ) & 0x1f];
    return (a << 24) | (r << 16) | (g << 8) | b;
}

static FxU32
_txPixRgba4444to8888 (FxU16 c4444)
{
    FxU32       a, r, g, b;
    a = P4_8[(c4444 >> 12) & 0xf];
    r = P4_8[(c4444 >>  8) & 0xf];
    g = P4_8[(c4444 >>  4) & 0xf];
    b = P4_8[(c4444      ) & 0xf];
    return (a << 24) | (r << 16) | (g << 8) | b;
}

static FxU32
_txPixAi88to8888(FxU16 c88)
{
    FxU32       a, r, g, b;
    a = c88 >> 8;
    r = g = b = (c88 & 0xff);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

/******************************************************************************/
static void
_txImgDequantizeRGB332(FxU32 *out, FxU8 *in, int w, int h)
{
    int         n = w * h;

    out += n;
    in  += n;
    while (n--) *--out = _txPixRgb332to8888(*--in);
}

static void
_txImgDequantizeYIQ422(FxU32 *out, FxU8 *in, int w, int h, const int *yabTable)
{
    int                         n = w * h;
    FxU32                       pal[256];

    txYABtoPal256((int *)pal, (int *)yabTable);
    out += n;
    in  += n;
    while (n--) *--out = pal[*--in] | 0xff000000;
}

static void
_txImgDequantizeA8(FxU32 *out, FxU8 *in, int w, int h)
{
    int         n = w * h;

    out += n;
    in  += n;
    while (n--) *--out = _txPixA8to8888(*--in);
}

static void
_txImgDequantizeI8(FxU32 *out, FxU8 *in, int w, int h)
{
    int         n = w * h;

    out += n;
    in  += n;
    while (n--) *--out = _txPixI8to8888(*--in);
}

static void
_txImgDequantizeAI44(FxU32 *out, FxU8 *in, int w, int h)
{
    int         n = w * h;

    out += n;
    in  += n;
    while (n--) *--out = _txPixAi44to8888(*--in);
}

static void
_txImgDequantizeP8(FxU32 *out, FxU8 *in, int w, int h, const FxU32 *pal)
{
    int         n = w * h;

    out += n;
    in  += n;
    while (n--) *--out = pal[*--in] | 0xff000000;
}


static void
_txImgDequantizeARGB8332(FxU32 *out, FxU16 *in, int w, int h)
{
    int         n = w * h;

    out += n;
    in  += n;
    while (n--) *--out = _txPixArgb8332to8888 (*--in);
}

static void
_txImgDequantizeAYIQ8422(FxU32 *out, FxU16 *in, int w, int h, const int *yab)
{
    int         n = w * h;
    int        pal[256];

    txYABtoPal256(pal, yab);
    out += n;
    in  += n;
    while (n--) {
        in--; 
        *--out = (pal[(*in) & 0xff] & 0x00ffffff) | ((*in & 0xFF00) << 16);
    }
}

static void
_txImgDequantizeRGB565(FxU32 *out, FxU16 *in, int w, int h)
{
    int         n = w * h;

    out += n;
    in  += n;
    while (n--) *--out = _txPixRgb565to8888(*--in);
}

static void
_txImgDequantizeARGB1555(FxU32 *out, FxU16 *in, int w, int h)
{
    int         n = w * h;

    out += n;
    in  += n;
    while (n--) *--out = _txPixArgb1555to8888 (*--in);
}

static void
_txImgDequantizeARGB4444(FxU32 *out, FxU16 *in, int w, int h)
{
    int         n = w * h;

    out += n;
    in  += n;
    while (n--) *--out = _txPixRgba4444to8888 (*--in);
}

static void
_txImgDequantizeAI88(FxU32 *out, FxU16 *in, int w, int h)
{
    int         n = w * h;

    out += n;
    in  += n;
    while (n--) *--out = _txPixAi88to8888(*--in);
}


static void
_txImgDequantizeAP88(FxU32 *out, FxU16 *in, int w, int h, const FxU32 *pal)
{
    int         n = w * h;

    out += n;
    in  += n;

    while (n--) {
        in--;
        *--out = (pal[(*in) & 0xff] & 0x00ffffff) | ((*in & 0xFF00) << 16);
    }
}

#if 0 /* not used */
static void
_txImgDequantizeRGB88(FxU32 *out, FxU8 *in, int w, int h)
{
    int         n = w * h;

    out += n;
    in  += 3*n;
    while (n--) {
        FxU32 a, r, g, b;
        in -= 3;
        
        a = 0xff;
        r = in[0]; 
        g = in[1]; 
        b = in[2]; 

        *--out = (a << 24) | (r << 16) | (g << 8) | b;
    }
}
#endif

static void
_txImgDequantizeARGB8888(FxU32 *out, FxU32 *in, int w, int h)
{
    int         n = w * h;

    out += n;
    in  += n;

    while (n--) *--out = *--in; 
}

static void
_txImgDequantizeRGB888(FxU32 *out, FxU32 *in_, int w, int h)
{
    int         n = w * h;
    int i;
    FxU8 *in = ( FxU8 * )in_;

    for( i = 0; i < n; i++ )
      {
        out[i] = 
          ( ( ( FxU32 )0xff ) << 24 ) | 
          ( ( ( FxU32 )in[i*3] )<< 16 ) |
          ( ( ( FxU32 )in[i*3+1] )<< 8 ) |
          ( ( ( FxU32 )in[i*3+2] ) );
      }
}

static void 
_txCalcRGBFromYUV(unsigned int y, unsigned int u, unsigned int v, FxU32 *rgb)
{
        FxI32 r, g, b;
        int  y16, u128, v128;

        y16 = y - 16;
        u128 = u - 128;
        v128 = v - 128;

        r = (FxI32) (1.164f * y16                 + 1.596f * v128 + 0.5);
        g = (FxI32) (1.164f * y16 - 0.391f * u128 - 0.813f * v128 + 0.5);
        b = (FxI32) (1.164f * y16 + 2.018f * u128 + 0.5);
        
        if (r > 255)
        {
                r = 255;
        }
        else if (r < 0)
        {
                r = 0;
        }

        if (g > 255)
        {
                g = 255;
        }
        else if (g < 0)
        {
                g = 0;
        }

        if (b > 255)
        {
                b = 255;
        }
        else if (b < 0)
        {
                b = 0;
        }

        *rgb = (r << 16) | (g << 8) | b;
}

void 
_txImgDequantizeYUV(FxU32 *out, FxU16 *in, int w, int h, FxU32 format)
{
        int i, j, k;
        unsigned int Y[2], UV[2];


        k = w * h;

        for (i = 0; i < k; i += 2)
        {
                // Process 2 texels at a time

                for (j = 0; j < 2; j++)
                {
                        if ( format == GR_TEXFMT_YUYV_422 )
                        {
                                Y[j]  = *in & 0xFF;
                                UV[j] = *in >> 8;
                        }
                        else
                        {
                                // GR_TEXFMT_UYVY_422 format

                                Y[j]  = *in >> 8;
                                UV[j] = *in & 0xFF;
                        }

                        in++;
                }

                // Convert the texels into RGB and store them.  U is stored in UV[0], and V is stored
                // in UV[1].

                _txCalcRGBFromYUV(Y[0], UV[0], UV[1], out);

                out++;

                _txCalcRGBFromYUV(Y[1], UV[0], UV[1], out);

                out++;
        }
}

void 
_txImgDequantizeAYUV(FxU32 *out, FxU32 *in, int w, int h)
{
        int           i, k;
        unsigned int y, u, v;

        k = w * h;

        for (i = 0; i < k; i++)
        {
                y = (*in >> 16) & 0xFF;
                u = (*in >> 8) & 0xFF;
                v = *in & 0xFF;

                // Get the RGB for the texel.

                _txCalcRGBFromYUV(y, u, v, out);

                // Output the ARGB texel

                *out++ |= (*in++ & 0xFF000000);
        }
}

void
sst2FXT1Decode4bpp(int *encoded, int width, int height, int *data);

void    
_txImgDequantizeFXT1(FxU32 *out, FxU32 *in, int w, int h)
{
    assert( w % 8 == 0);
    assert( h % 4 == 0);

    sst2FXT1Decode4bpp((int *)in, w, h, (int *)out);
}

void    
txMipDequantize(TxMip *txMip, TxMip *pxMip)
{
    /* Walk through all mipmap levels, and convert to ARGB8888 format */
    int         i, w, h;

    if( txVerbose )
      {
        printf("Dequant: (from %s) ..", Format_Name[pxMip->format]);
      }
    w = pxMip->width;
    h = pxMip->height;
    for (i=0; i< txMip->depth; i++) {
        void    *src, *dst;

        src = pxMip->data[i];
        dst = txMip->data[i];

        if( txVerbose )
          {
            printf(" %dx%d", w, h); fflush(stdout);
          }
            
        switch(pxMip->format) {
        case GR_TEXFMT_RGB_332:         _txImgDequantizeRGB332(dst, src, w, h); break;
        case GR_TEXFMT_YIQ_422:         _txImgDequantizeYIQ422(dst, src, w, h, (int *)pxMip->pal); break;
        case GR_TEXFMT_A_8:             _txImgDequantizeA8(dst, src, w, h); break;
        case GR_TEXFMT_I_8:             _txImgDequantizeI8(dst, src, w, h); break;
        case GR_TEXFMT_AI_44:           _txImgDequantizeAI44(dst, src, w, h); break;
        case GR_TEXFMT_P_8:             _txImgDequantizeP8(dst, src, w, h, pxMip->pal); break;
        case GR_TEXFMT_ARGB_8332:       _txImgDequantizeARGB8332(dst, src, w, h); break;
        case GR_TEXFMT_AYIQ_8422:       _txImgDequantizeAYIQ8422(dst, src, w, h, (int *)pxMip->pal); break;
        case GR_TEXFMT_RGB_565:         _txImgDequantizeRGB565(dst, src, w, h); break;
        case GR_TEXFMT_ARGB_1555:       _txImgDequantizeARGB1555(dst, src, w, h); break;
        case GR_TEXFMT_ARGB_4444:       _txImgDequantizeARGB4444(dst, src, w, h); break;
        case GR_TEXFMT_AI_88:           _txImgDequantizeAI88(dst, src, w, h); break;
        case GR_TEXFMT_AP_88:           _txImgDequantizeAP88(dst, src, w, h, pxMip->pal); break;
        case GR_TEXFMT_RGB_888:         _txImgDequantizeRGB888(dst, src, w, h); break;
        case GR_TEXFMT_ARGB_8888:       _txImgDequantizeARGB8888(dst, src, w, h); break;

        case GR_TEXFMT_YUYV_422:
        case GR_TEXFMT_UYVY_422:        _txImgDequantizeYUV(dst, src, w, h, pxMip->format); break;
        case GR_TEXFMT_AYUV_444:        _txImgDequantizeAYUV(dst, src, w, h); break;
     
        case GR_TEXFMT_ARGB_CMP_FXT1:   _txImgDequantizeFXT1(dst, src, (w + 0x7) & ~0x7, (h + 0x3) & ~0x3); break;
        default:                                                                                                        
          break;
        }
        
        if (w > 1) w >>= 1;
        if (h > 1) h >>= 1;
    }
    if( txVerbose )
      {
        printf(".\n"); fflush(stdout);
      }
}
