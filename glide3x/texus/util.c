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

int txVerbose = 0;

/* These are all the formats listed in TexUS.h */ 
char *Format_Name[] = {
  "rgb332",                              // 0x0 TEXFMT_RGB_332 
  "yiq",                                 // 0x1 TEXFMT_YIQ_422
  "a8",                                  // 0x2 TEXFMT_A_8
  "i8",                                  // 0x3 TEXFMT_I_8
  "ai44",                                // 0x4 TEXFMT_A_44
  "p8",                                  // 0x5 TEXFMT_P_8
  "p6666",                               // 0x6 TEXFMT_P_8_6666
  "rsvd2",                               // 0x7 TEXFMT_RSVD2
  "argb8332",                            // 0x8 TEXFMT_ARGB_8332
  "ayiq8422",                            // 0x9 TEXFMT_AYIQ_8422
  "rgb565",                              // 0xa TEXFMT_RGB_565
  "argb1555",                            // 0xb TEXFMT_ARGB_1555
  "argb4444",                            // 0xc TEXFMT_ARGB_4444
  "ai88",                                // 0xd TEXFMT_AI_88
  "ap88",                                // 0xe TEXFMT_AP_88
  "rsvd4",                               // 0xf TEXFMT_RSVD4
  "unknown",                             // skipped 0x10 for some reason in texus.h
  "fxt1",                                // 0x11 TEXFMT_ARGB_CMP_FXT1
  "argb8888",                            // 0x12 TEXFMT_ARGB_8888
  "yuyv422",                             // 0x13 TEXFMT_YUYV_422
  "uyvy422",                             // 0x14 TEXFMT_UYVY_422
  "ayuv444",                             // 0x15 TEXFMT_AYUV_444
  "dxt1",                                // 0x16 TEXFMT_ARGB_CMP_DXT1
  "dxt2",                                // 0x17 TEXFMT_ARGB_CMP_DXT2
  "dxt3",                                // 0x18 TEXFMT_ARGB_CMP_DXT3
  "dxt4",                                // 0x19 TEXFMT_ARGB_CMP_DXT4
  "dxt5",                                // 0x1a TEXFMT_ARGB_CMP_DXT5
};

int
txLog2(int n)
{
        switch (n) {
        case    1:              return 0;
        case    2:              return 1;
        case    4:              return 2;
        case    8:              return 3;
        case    16:             return 4;
        case    32:             return 5;
        case    64:             return 6;
        case    128:    return 7;
        case    256:    return 8;
        }

        txPanic("Bad arg to Log2\n");
        return 0;               // to keep compiler quiet.
}

int
txFloorPow2(int n)
{
        // Find next smallest integer which is also a power of 2.

        int             i;

        if ((n & (n -1)) == 0) {
                return n;               // already a power of 2.
        }

        for (i=1; i<= n; i+=i);
        return i>>1;
}

int
txCeilPow2(int n)
{
        // Find next smallest integer which is also a power of 2.

        int             i;

        if ((n & (n -1)) == 0) return n;                // already a power of 2.

        for (i=1; i<= n; i+=i);
        return i;
}

extern TxErrorCallbackFnc_t _txErrorCallback;

void
txPanic(char *message)
{
        _txErrorCallback( message, FXTRUE );
}

void
txError(char *message)
{
        _txErrorCallback( message, FXFALSE );
}

extern void _txDefaultErrorCallback( const char *string, FxBool fatal );

void
txErrorSetCallback( TxErrorCallbackFnc_t  fnc,
                    TxErrorCallbackFnc_t *old_fnc)
{
    if (old_fnc) {
        *old_fnc = _txErrorCallback;
    }
    if (fnc) {
        _txErrorCallback = fnc;
    } else {
        _txErrorCallback = _txDefaultErrorCallback;
    }
}

int
txGCD(int a, int b)
{
        // Greatest common divisor, used in resampling.

        if (b > a) { int t;     t = a; a = b; b = t;}

        // a>b here.

        while (b > 0) {
                int             t;

                t = a % b;
                a = b;
                b = t;
        }
        return a;
}

void
txYABtoPal256(int *palette, const int* yabTable)
{
        // Convert YAB table to a 256 color palette 
        // Assume yabTable[] has first 16Y's, 12 A's, 12 B's

        const   int    *Y = yabTable;
        const   int    *A = yabTable + 16;
        const   int    *B = yabTable + 16 + 12;
        int             i;

        for (i=0; i<256; i++) {
                int             iy, ia, ib, r, g, b;

                iy = (i >> 4) & 0xF;
                ia = (i >> 2) & 0x3;
                ib = (i >> 0) & 0x3;

                r = Y[iy] + A[3*ia + 0] + B[3*ib + 0];
                g = Y[iy] + A[3*ia + 1] + B[3*ib + 1];
                b = Y[iy] + A[3*ia + 2] + B[3*ib + 2];

                if (r < 0) r = 0; if (r > 255) r = 255;
                if (g < 0) g = 0; if (g > 255) g = 255;
                if (b < 0) b = 0; if (b > 255) b = 255;

                palette[i] = (r << 16) | (g << 8) | b;
        }
}

/*
 * The following table was generated from this piece of code:

main()
{
        int     i;

        printf("static int      _explode3[256+256+4] = {\n");

        for (i=-255; i<256; i++) {
                int             k;
                int             bits;

                k = (i < 0) ? -i : i;
                bits = 0;

                if (k & 0x01) bits += 0x00000001 << 0;
                if (k & 0x02) bits += 0x00000001 << 3;
                if (k & 0x04) bits += 0x00000001 << 6;
                if (k & 0x08) bits += 0x00000001 << 9;
                if (k & 0x10) bits += 0x00000001 << 12;
                if (k & 0x20) bits += 0x00000001 << 15;
                if (k & 0x40) bits += 0x00000001 << 18;
                if (k & 0x80) bits += 0x00000001 << 21;

                // explode3[i] = bits;
                printf("0x%.06x,", bits);
                if (((i + 255) % 8) == 7) printf("\n");
        }
        printf("\n};\n");
        printf("static int  *explode3 = &_explode3[255];\n");
}
 */

int     _explode3[255+256] = {
0x249249,0x249248,0x249241,0x249240,0x249209,0x249208,0x249201,0x249200,
0x249049,0x249048,0x249041,0x249040,0x249009,0x249008,0x249001,0x249000,
0x248249,0x248248,0x248241,0x248240,0x248209,0x248208,0x248201,0x248200,
0x248049,0x248048,0x248041,0x248040,0x248009,0x248008,0x248001,0x248000,
0x241249,0x241248,0x241241,0x241240,0x241209,0x241208,0x241201,0x241200,
0x241049,0x241048,0x241041,0x241040,0x241009,0x241008,0x241001,0x241000,
0x240249,0x240248,0x240241,0x240240,0x240209,0x240208,0x240201,0x240200,
0x240049,0x240048,0x240041,0x240040,0x240009,0x240008,0x240001,0x240000,
0x209249,0x209248,0x209241,0x209240,0x209209,0x209208,0x209201,0x209200,
0x209049,0x209048,0x209041,0x209040,0x209009,0x209008,0x209001,0x209000,
0x208249,0x208248,0x208241,0x208240,0x208209,0x208208,0x208201,0x208200,
0x208049,0x208048,0x208041,0x208040,0x208009,0x208008,0x208001,0x208000,
0x201249,0x201248,0x201241,0x201240,0x201209,0x201208,0x201201,0x201200,
0x201049,0x201048,0x201041,0x201040,0x201009,0x201008,0x201001,0x201000,
0x200249,0x200248,0x200241,0x200240,0x200209,0x200208,0x200201,0x200200,
0x200049,0x200048,0x200041,0x200040,0x200009,0x200008,0x200001,0x200000,
0x049249,0x049248,0x049241,0x049240,0x049209,0x049208,0x049201,0x049200,
0x049049,0x049048,0x049041,0x049040,0x049009,0x049008,0x049001,0x049000,
0x048249,0x048248,0x048241,0x048240,0x048209,0x048208,0x048201,0x048200,
0x048049,0x048048,0x048041,0x048040,0x048009,0x048008,0x048001,0x048000,
0x041249,0x041248,0x041241,0x041240,0x041209,0x041208,0x041201,0x041200,
0x041049,0x041048,0x041041,0x041040,0x041009,0x041008,0x041001,0x041000,
0x040249,0x040248,0x040241,0x040240,0x040209,0x040208,0x040201,0x040200,
0x040049,0x040048,0x040041,0x040040,0x040009,0x040008,0x040001,0x040000,
0x009249,0x009248,0x009241,0x009240,0x009209,0x009208,0x009201,0x009200,
0x009049,0x009048,0x009041,0x009040,0x009009,0x009008,0x009001,0x009000,
0x008249,0x008248,0x008241,0x008240,0x008209,0x008208,0x008201,0x008200,
0x008049,0x008048,0x008041,0x008040,0x008009,0x008008,0x008001,0x008000,
0x001249,0x001248,0x001241,0x001240,0x001209,0x001208,0x001201,0x001200,
0x001049,0x001048,0x001041,0x001040,0x001009,0x001008,0x001001,0x001000,
0x000249,0x000248,0x000241,0x000240,0x000209,0x000208,0x000201,0x000200,
0x000049,0x000048,0x000041,0x000040,0x000009,0x000008,0x000001,0x000000,
0x000001,0x000008,0x000009,0x000040,0x000041,0x000048,0x000049,0x000200,
0x000201,0x000208,0x000209,0x000240,0x000241,0x000248,0x000249,0x001000,
0x001001,0x001008,0x001009,0x001040,0x001041,0x001048,0x001049,0x001200,
0x001201,0x001208,0x001209,0x001240,0x001241,0x001248,0x001249,0x008000,
0x008001,0x008008,0x008009,0x008040,0x008041,0x008048,0x008049,0x008200,
0x008201,0x008208,0x008209,0x008240,0x008241,0x008248,0x008249,0x009000,
0x009001,0x009008,0x009009,0x009040,0x009041,0x009048,0x009049,0x009200,
0x009201,0x009208,0x009209,0x009240,0x009241,0x009248,0x009249,0x040000,
0x040001,0x040008,0x040009,0x040040,0x040041,0x040048,0x040049,0x040200,
0x040201,0x040208,0x040209,0x040240,0x040241,0x040248,0x040249,0x041000,
0x041001,0x041008,0x041009,0x041040,0x041041,0x041048,0x041049,0x041200,
0x041201,0x041208,0x041209,0x041240,0x041241,0x041248,0x041249,0x048000,
0x048001,0x048008,0x048009,0x048040,0x048041,0x048048,0x048049,0x048200,
0x048201,0x048208,0x048209,0x048240,0x048241,0x048248,0x048249,0x049000,
0x049001,0x049008,0x049009,0x049040,0x049041,0x049048,0x049049,0x049200,
0x049201,0x049208,0x049209,0x049240,0x049241,0x049248,0x049249,0x200000,
0x200001,0x200008,0x200009,0x200040,0x200041,0x200048,0x200049,0x200200,
0x200201,0x200208,0x200209,0x200240,0x200241,0x200248,0x200249,0x201000,
0x201001,0x201008,0x201009,0x201040,0x201041,0x201048,0x201049,0x201200,
0x201201,0x201208,0x201209,0x201240,0x201241,0x201248,0x201249,0x208000,
0x208001,0x208008,0x208009,0x208040,0x208041,0x208048,0x208049,0x208200,
0x208201,0x208208,0x208209,0x208240,0x208241,0x208248,0x208249,0x209000,
0x209001,0x209008,0x209009,0x209040,0x209041,0x209048,0x209049,0x209200,
0x209201,0x209208,0x209209,0x209240,0x209241,0x209248,0x209249,0x240000,
0x240001,0x240008,0x240009,0x240040,0x240041,0x240048,0x240049,0x240200,
0x240201,0x240208,0x240209,0x240240,0x240241,0x240248,0x240249,0x241000,
0x241001,0x241008,0x241009,0x241040,0x241041,0x241048,0x241049,0x241200,
0x241201,0x241208,0x241209,0x241240,0x241241,0x241248,0x241249,0x248000,
0x248001,0x248008,0x248009,0x248040,0x248041,0x248048,0x248049,0x248200,
0x248201,0x248208,0x248209,0x248240,0x248241,0x248248,0x248249,0x249000,
0x249001,0x249008,0x249009,0x249040,0x249041,0x249048,0x249049,0x249200,
0x249201,0x249208,0x249209,0x249240,0x249241,0x249248,0x249249,
};
int  *explode3 = &_explode3[255];

int
txNearestColor(int ir, int ig, int ib, const FxU32 *pal, int ncolors)
{
        int             i, d; 
        int             mindist, minpos;                // closest distance to input

        if (&explode3[-255] != &_explode3[0])
                txPanic("Bad explode\n");

        mindist = DISTANCE((*pal>>16)&0xff, (*pal>>8)&0xff, (*pal)&0xff, 
                ir, ig, ib);
        minpos  = 0;
        pal ++;

        /* Find closest color */
        for (i=1; i<ncolors; i++, pal ++) {
                d = DISTANCE((*pal>>16)&0xff, (*pal>>8)&0xff, (*pal)&0xff, 
                        ir, ig, ib);
                if (d < mindist) { mindist = d; minpos = i; }
        }
        return minpos;          // best fit color is returned.
}

#ifdef GLIDE3
int
txAspectRatio(int w, int h)
{
        int             ar;

        ar = (w >= h) ? (((w/h) << 4) | 1) : ((1 << 4) | (h/w));
        switch (ar) {
        case    0x81:   return 6;
        case    0x41:   return 5;
        case    0x21:   return 4;
        case    0x11:   return 3;
        case    0x12:   return 2;
        case    0x14:   return 1;
        case    0x18:   return 0;
        }
        return 0;
}
#else
int
txAspectRatio(int w, int h)
{
        int             ar;

        ar = (w >= h) ? (((w/h) << 4) | 1) : ((1 << 4) | (h/w));
        switch (ar) {
        case    0x81:   return 0;
        case    0x41:   return 1;
        case    0x21:   return 2;
        case    0x11:   return 3;
        case    0x12:   return 4;
        case    0x14:   return 5;
        case    0x18:   return 6;
        }
        return 0;
}
#endif /* GLIDE3 */

void
txRectCopy(FxU8 *dst, int dstStride, const FxU8 *src, int srcStride, 
        int width, int height)
{
        // Copy a rectangular region from src to dst, each with different strides.

        while (height--) {
                int             i;
                for (i=0; i<width; i++) {
                        dst[i] = src[i];
                }
                dst += dstStride;
                src += srcStride;
        }
}

int
txMemRequired(TxMip *txMip)
{
        /* Tell me how much memory is need to hold this mipmap */
        int             w, h, memsize, i;

        w = txMip->width;
        h = txMip->height;
        memsize = 0;

        for (i=0; i<txMip->depth; i++) {
                memsize += txTexCalcMapSize( w, h, txMip->format );
                if (w > 1) w >>= 1;
                if (h > 1) h >>= 1;
        }
        return memsize;
}

FxBool
txMipAlloc(TxMip *txMip)
{
        int             i, w, h;
        FxU8    *data;

        txMip->size = txMemRequired(txMip);

        data = (FxU8 *) txMalloc(txMip->size);
        if (data == NULL) return FXFALSE;
        w = txMip->width;
        h = txMip->height;

        for (i=0; i<TX_MAX_LEVEL; i++) {
                if (i >= txMip->depth) {
                        txMip->data[i] = NULL;
                        continue;
                }
                txMip->data[i] = data;
                data += txTexCalcMapSize( w, h, txMip->format );
                if (w > 1) w >>= 1;
                if (h > 1) h >>= 1;
        }
        return FXTRUE;
}

FxBool
txMipSetMipPointers(TxMip *txMip)
{
        int             i, w, h;
        FxU8    *data = txMip->data[0];

        txMip->size = txMemRequired(txMip);

        w = txMip->width;
        h = txMip->height;

        for (i=0; i<TX_MAX_LEVEL; i++) {
                if (i >= txMip->depth) {
                        txMip->data[i] = NULL;
                        continue;
                }
                txMip->data[i] = data;
                data += txTexCalcMapSize( w, h, txMip->format );
                if (w > 1) w >>= 1;
                if (h > 1) h >>= 1;
        }
        return FXTRUE;
}

void
txBasename(const char *name, char* basename)
{
        /* Strip the pathname and leave us with filename.ext */
        char    *s; 
        const   char *p, *slash;

        /* Find the last slash */
        for (p = slash = name; *p;  p++) {
                if ((*p == '/') || (*p == '\\')) slash = p + 1;
        }

        /* Copy everything after the last slash to output */
        strcpy(basename, slash);

        // Walk to end of string */
        for (s = basename; *s; s++);

        // Walk backwards; replace any . with 0 
        while (--s >= basename) {
                if (*s == '.') {*s = 0; break;}
        }
}

void
txPathAndBasename(const char *name, char* basename)
{
        /* Strip the extension and leave us with path,basename */
        char    *s;

        strcpy(basename, name);

        /* Walk to the end of the string */
        for (s = basename; *s; s++);

        /* Walk backwards; stop when you hit a slash; replace any . with 0. */
        while (--s >= basename) {
                if ((*s == '/') || (*s == '\\')) break;
                if (*s == '.') {*s = 0; break;}

        }
}

void
txExtension(const char *name, char *extname)
{
        const char      *p, *ext;

        ext = NULL;
        for (p = name; *p;  p++) {
                if (*p == '.')  ext = p;
        }

        if (ext) 
            while (*ext)  *extname++ = *ext++;
        
        *extname = 0;
}

void
txMipFree( TxMip *mip )
{
  int i;

  txFree( mip->data[0] );
  for( i = 0; i < TX_MAX_LEVEL; i++ )
    mip->data[i] = NULL;
}

void *txMalloc( size_t size )
{
  return malloc( size );
}

void txFree( void *ptr )
{
  free( ptr );
}

void *txRealloc( void *ptr, size_t size )
{
  return realloc( ptr, size );
}

