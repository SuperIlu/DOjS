/*
 * This software is copyrighted as noted below.  It may be freely copied,
 * modified, and redistributed, provided that the copyright notice is 
 * preserved on all copies.
 * 
 * There is no warranty or other guarantee of fitness for this software,
 * it is provided solely "as is".  Bug reports or fixes may be sent
 * to the author, who may or may not act on them as he desires.
 *
 * You may not include this software in a program or other software product
 * without supplying the source, or without informing the end-user that the 
 * source is available for no extra charge.
 *
 * If you modify this software, you should include a notice giving the
 * name of the person performing the modification, the date of modification,
 * and the reason for such modification.
 */


/*
 * Well, I hacked it anyway.... Murali.
 * Hack upon hack modified pal256 to give 6666 MWP
 *
 * pal6666.c
 *
 * Perform variance-based color quantization on a "full color" image.
 * Author:      Craig Kolb
 *              Department of Mathematics
 *              Yale University
 *              kolb@yale.edu
 * Date:        Tue Aug 22 1989
 * Copyright (C) 1989 Craig E. Kolb
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "texusint.h"

typedef unsigned int   uint;
typedef unsigned char   uchar;
typedef unsigned short  ushort;

#ifdef  HUGE
#undef  HUGE
#endif

#define HUGE    1.0e38

#define NCOMP   4  /* number of color components */

#define NBITS   4  /* LOOOK need to assign these dynamically based on the image contents */

#define MAXCOLORS       256
#define FULLINTENSITY   255

/*
 * Readability constants.
 */
#define ALPHAI          0       
#define REDI            1       
#define GREENI          2
#define BLUEI           3       

#define TRUE            1
#define FALSE           0
#ifndef bzero
#define bzero(ptr, sz)  memset(ptr, 0, sz)
#endif

#define INVERSE_PAL_A_BITS iPalBits[ALPHAI]
#define INVERSE_PAL_R_BITS iPalBits[REDI]
#define INVERSE_PAL_G_BITS iPalBits[GREENI]
#define INVERSE_PAL_B_BITS iPalBits[BLUEI]
#define INVERSE_PAL_TOTAL_BITS 16
#define INVERSE_PAL_SIZE ( 1 << INVERSE_PAL_TOTAL_BITS ) 

#define MKIDX(a, r, g, b) (( a << (NBITS*3)) | (r << (NBITS*2)) | (g << NBITS) | b)

typedef struct {
    float               weightedvar;              /* weighted variance */
    uint               mean[NCOMP];              /* centroid */
    uint               weight;                   /* # of pixels in box */
    uint               freq[NCOMP][MAXCOLORS];   /* Projected frequencies */
    int                 low[NCOMP], high[NCOMP];  /* Box extent */
} Box;

static uchar iPalBits[NCOMP] = { 4, 4, 4, 4};

static uchar inverse_pal[INVERSE_PAL_SIZE];

#define COLORMAXI ( 1 << NBITS )
#if 0
static uint    *Histogram;             /* image histogram      */
#else
static uint    Histogram[1<<INVERSE_PAL_TOTAL_BITS];
#endif
static uint    SumPixels;              /* total # of pixels    */
static uint    ColormaxI;              /* # of colors, 2^Bits */
static Box      _Boxes[MAXCOLORS];
static Box     *Boxes;                 /* Array of color boxes. */

static void     SetARGBmap(int boxnum, Box *box, uchar *argbmap);
static void     ComputeARGBMap(Box *boxes, int colors, uchar *argbmap);
static void     UpdateFrequencies(Box *box1, Box *box2);
static int      FindCutpoint(Box *box, int color, Box *newbox1, Box *newbox2);
static int      CutBox(Box *box, Box *newbox);
static void     BoxStats(Box *box);
static int      GreatestVariance(Box *boxes, int n);
static int      CutBoxes(Box *boxes, int colors);
static void     QuantHistogram(uint *pixels, int npixels, Box *box);

/*
 * Perform variance-based color quantization on a 32-bit image.
 */
int     
txMipPal6666(TxMip *pxMip, TxMip *txMip, int format, FxU32 dither, FxU32 compression)
{
    int         w, h;
    int         i;                      /* Counter */
    int         OutColors;              /* # of entries computed */
    int         Colormax;               /* quantized full-intensity */ 
    float       Cfactor;                /* Conversion factor */
#if 0
    uchar       *argbmap;                /* how to map colors to palette indices */
#else
    static uchar argbmap[INVERSE_PAL_SIZE]; /* how to map colors to palette indices */
#endif
    int         pixsize;


    ColormaxI = 1 << NBITS;     /* 2 ^ NBITS */
    Colormax = ColormaxI - 1;
    Cfactor = (float)FULLINTENSITY / Colormax;

    Boxes = _Boxes;     
#if 0
    Histogram = (uint *) txMalloc(INVERSE_PAL_SIZE * sizeof(int));
    argbmap = txMalloc(INVERSE_PAL_SIZE);
#endif

    /*
     * Zero-out the projected frequency arrays of the largest box.
     */

    bzero(Boxes->freq[ALPHAI], ColormaxI * sizeof(uint));
    bzero(Boxes->freq[REDI],   ColormaxI * sizeof(uint));
    bzero(Boxes->freq[GREENI], ColormaxI * sizeof(uint));
    bzero(Boxes->freq[BLUEI],  ColormaxI * sizeof(uint));

    bzero(Histogram, INVERSE_PAL_SIZE * sizeof(int));

    /* Feed all bitmaps & generate histogram */
    SumPixels = 0;
    w = txMip->width;
    h = txMip->height;
    for (i=0; i< txMip->depth; i++) {
        SumPixels += w * h;
        QuantHistogram((uint *)txMip->data[i], w * h, &Boxes[0]);
        if (w > 1) w >>= 1;
        if (h > 1) h >>= 1;
    }

    OutColors = CutBoxes(Boxes, MAXCOLORS);
    
    /*
     * We now know the set of representative colors.  We now
     * must fill in the colormap and convert the representatives
     * from their 'prequantized' range to 0-FULLINTENSITY.
     */

    for (i = 0; i < OutColors; i++) {
        uint  a, r, g, b;

        a = (uint)(Boxes[i].mean[ALPHAI] * Cfactor + 0.5);
        r = (uint)(Boxes[i].mean[REDI]   * Cfactor + 0.5);
        g = (uint)(Boxes[i].mean[GREENI] * Cfactor + 0.5);
        b = (uint)(Boxes[i].mean[BLUEI]  * Cfactor + 0.5);

        if (a > 255) a = 255;
        if (r > 255) r = 255;
        if (g > 255) g = 255;
        if (b > 255) b = 255;

        pxMip->pal[i] = (a<<24)|(r<<16) | (g << 8) | b;
    }
    ComputeARGBMap(Boxes, OutColors, argbmap); 

    /*
     * Now translate the colors to palette indices.
     */
    pixsize = (format == GR_TEXFMT_AP_88) ? 2 : 1;

    if ((dither&TX_DITHER_MASK) != TX_DITHER_NONE) {
        /* support only error diffusion, no 4x4 dithering */
        txDiffuseIndex(pxMip, txMip, pixsize, pxMip->pal, OutColors);
    } else {

        w = txMip->width;
        h = txMip->height;

        for (i=0; i< txMip->depth; i++) {
                uint   *src;
                uchar   *dst;
                int             n;

                src = (uint *) txMip->data[i];
                dst = (uchar *) pxMip->data[i];
                n   = w * h;
                while (n--) {
                        int     a, r, g, b, argb, index;

                        argb = *src++;
                        a = (argb & 0xFF000000) >> (24 + 8 - NBITS);
                        r = (argb & 0x00FF0000) >> (16 + 8 - NBITS);
                        g = (argb & 0x0000FF00) >> ( 8 + 8 - NBITS);
                        b = (argb & 0x000000FF) >> ( 0 + 8 - NBITS);

                        index = MKIDX(a, r, g, b);
                        if ((index < 0) || (index >= INVERSE_PAL_SIZE)) {
                                printf("Bad index: %d (%d %d %d %d)\n", index, a, r, g, b);
                        }
                        if (pixsize == 1) {
                                *dst++ = argbmap[index];
                        } else {
                                *(FxU16 *)dst = (argbmap[index]) | 
                                                ((argb >> 16) & 0xFF00);
                                dst+= 2;
                        }
                }
                if (w > 1) w >>= 1;
                if (h > 1) h >>= 1;
        }
    }

#if 0
    txFree((char *)Histogram);
    txFree((char *)argbmap);
#endif
    return OutColors;
}

/*
 * Compute the histogram of the image as well as the projected frequency
 * arrays for the first world-encompassing box.
 */
static void
QuantHistogram(uint *pixels, int npixels, Box *box)
{
    uint *af, *rf, *gf, *bf;
    uchar aa, rr, gg, bb;
    int         i;

    af = box->freq[ALPHAI];
    rf = box->freq[REDI];
    gf = box->freq[GREENI];
    bf = box->freq[BLUEI];

    /*
     * We compute both the histogram and the proj. frequencies of
     * the first box at the same time to save a pass through the
     * entire image. 
     */
    
    for (i = 0; i < npixels; i++) {
        aa = (uchar) (((*pixels >> 24) & 0xff) >> (8-NBITS));
        rr = (uchar) (((*pixels >> 16) & 0xff) >> (8-NBITS));
        gg = (uchar) (((*pixels >>  8) & 0xff) >> (8-NBITS));
        bb = (uchar) (((*pixels      ) & 0xff) >> (8-NBITS));
        pixels++;
        af[aa]++;
        rf[rr]++;
        gf[gg]++;
        bf[bb]++;
        Histogram[MKIDX(aa, rr, gg, bb)]++;
    }
}

/*
 * Interatively cut the boxes.
 */
static int
CutBoxes(Box *boxes, int colors) 
{
    int curbox;

    boxes[0].low[ALPHAI] = boxes[0].low[REDI] = boxes[0].low[GREENI] = boxes[0].low[BLUEI] = 0;
    boxes[0].high[ALPHAI] = boxes[0].high[REDI] = boxes[0].high[GREENI] = boxes[0].high[BLUEI] = ColormaxI;
    boxes[0].weight = SumPixels;

    BoxStats(&boxes[0]);

    for (curbox = 1; curbox < colors; curbox++) {
        if (CutBox(&boxes[GreatestVariance(boxes, curbox)],
                   &boxes[curbox]) == FALSE)
                        break;
    }

    return curbox;
}

/*
 * Return the number of the box in 'boxes' with the greatest variance.
 * Restrict the search to those boxes with indices between 0 and n-1.
 */
static int
GreatestVariance(Box *boxes, int n)
{
    int i, whichbox = 0;
    float max;

    max = -1.0f;
    for (i = 0; i < n; i++) {
        if (boxes[i].weightedvar > max) {
                max = (float) boxes[i].weightedvar;
                whichbox = i;
        }
    }
    return whichbox;
}

/*
 * Compute mean and weighted variance of the given box.
 */
static void
BoxStats(Box *box)
{
    int i, color;
    uint *freq;
    float mean, var;

    if(box->weight == 0) {
        box->weightedvar = (float) 0.0;
        return;
    }

    box->weightedvar = (float) 0.0;
    for (color = 0; color < NCOMP; color++) {
        var = mean = (float) 0.0;
        i = box->low[color];
        freq = &box->freq[color][i];
        for (; i < box->high[color]; i++, freq++) {
                mean += (float) i * *freq;
                var += (float) i*i* *freq;
        }
        box->mean[color] = (unsigned int) (mean / (float)box->weight);
        box->weightedvar += var - box->mean[color]*box->mean[color]*
                                (float)box->weight;
    }
    box->weightedvar /= SumPixels;
}

/*
 * Cut the given box.  Returns TRUE if the box could be cut, FALSE otherwise.
 */
static int
CutBox(Box *box, Box *newbox)
{
    int i;
    float totalvar[NCOMP];
    Box newboxes[NCOMP][2];

    if (box->weightedvar == 0. || box->weight == 0)
        /*
         * Can't cut this box.
         */
        return FALSE;

    /*
     * Find 'optimal' cutpoint along each of the alpha, red, green and blue
     * axes.  Sum the variances of the two boxes which would result
     * by making each cut and store the resultant boxes for 
     * (possible) later use.
     */
    for (i = 0; i < NCOMP; i++) {
        if (FindCutpoint(box, i, &newboxes[i][0], &newboxes[i][1]))
                totalvar[i] = newboxes[i][0].weightedvar + newboxes[i][1].weightedvar;
        else
                totalvar[i] = (float) HUGE;
    }

    /*
     * Find which of the four cuts minimized the total variance
     * and make that the 'real' cut.
     */
    if (totalvar[ALPHAI] <= totalvar[REDI] &&
        totalvar[ALPHAI] <= totalvar[GREENI] && 
        totalvar[ALPHAI] <= totalvar[BLUEI]) {
        *box = newboxes[ALPHAI][0];
        *newbox = newboxes[ALPHAI][1];
    } else if (totalvar[REDI] <= totalvar[ALPHAI] && 
               totalvar[REDI] <= totalvar[GREENI] &&
               totalvar[REDI] <= totalvar[BLUEI]) {
        *box = newboxes[REDI][0];
        *newbox = newboxes[REDI][1];
    } else if (totalvar[GREENI] <= totalvar[ALPHAI] && 
               totalvar[GREENI] <= totalvar[REDI] &&
               totalvar[GREENI] <= totalvar[BLUEI]) {
        *box = newboxes[GREENI][0];
        *newbox = newboxes[GREENI][1];
    } else {
        *box = newboxes[BLUEI][0];
        *newbox = newboxes[BLUEI][1];
    }

    return TRUE;
}

/*
 * Compute the 'optimal' cutpoint for the given box along the axis
 * indcated by 'color'.  Store the boxes which result from the cut
 * in newbox1 and newbox2.
 */
static int
FindCutpoint(Box *box, int color, Box *newbox1, Box *newbox2)
{
    float u, v, max;
    int i, maxindex, minindex, cutpoint;
    uint optweight, curweight;

    if (box->low[color] + 1 == box->high[color])
        return FALSE;   /* Cannot be cut. */
    minindex = (int)((box->low[color] + box->mean[color]) * 0.5);
    maxindex = (int)((box->mean[color] + box->high[color]) * 0.5);

    cutpoint = minindex;
    optweight = box->weight;

    curweight = 0;
    for (i = box->low[color] ; i < minindex ; i++)
        curweight += box->freq[color][i];
    u = 0.0f;
    max = -1.0f;
    for (i = minindex; i <= maxindex ; i++) {
        curweight += box->freq[color][i];
        if (curweight == box->weight)
                break;
        u += (float)(i * box->freq[color][i]) /
                                (float)box->weight;
        v = ((float)curweight / (float)(box->weight-curweight)) *
                        (box->mean[color]-u)*(box->mean[color]-u);
        if (v > max) {
                max = v;
                cutpoint = i;
                optweight = curweight;
        }
    }
    cutpoint++;
    *newbox1 = *newbox2 = *box;
    newbox1->weight = optweight;
    newbox2->weight -= optweight;
    newbox1->high[color] = cutpoint;
    newbox2->low[color] = cutpoint;
    UpdateFrequencies(newbox1, newbox2);
    BoxStats(newbox1);
    BoxStats(newbox2);

    return TRUE;        /* Found cutpoint. */
}

/*
 * Update projected frequency arrays for two boxes which used to be
 * a single box.
 */

static void
UpdateFrequencies(Box *box1, Box *box2)
{
    uint myfreq, *h;
    int b, g, r, a;

    bzero(box1->freq[ALPHAI], ColormaxI * sizeof(uint));
    bzero(box1->freq[REDI],   ColormaxI * sizeof(uint));
    bzero(box1->freq[GREENI], ColormaxI * sizeof(uint));
    bzero(box1->freq[BLUEI],  ColormaxI * sizeof(uint)); 

    for (a = box1->low[ALPHAI]; a < box1->high[ALPHAI]; a++) {
        for (r = box1->low[REDI]; r < box1->high[REDI]; r++) {
            for (g = box1->low[GREENI];g < box1->high[GREENI]; g++) {
                    b = box1->low[BLUEI];
                    h = Histogram + MKIDX(a, r, g, b);
                    for (; b < box1->high[BLUEI]; b++) {
                            if ((myfreq = *h++) == 0)
                                    continue;
                            box1->freq[ALPHAI][a] += myfreq;
                            box1->freq[REDI  ][r] += myfreq;
                            box1->freq[GREENI][g] += myfreq;
                            box1->freq[BLUEI ][b] += myfreq;
                            box2->freq[ALPHAI][a] -= myfreq;
                            box2->freq[REDI  ][r] -= myfreq;
                            box2->freq[GREENI][g] -= myfreq;
                            box2->freq[BLUEI ][b] -= myfreq;
                    }
            }
        }
    }
}

/*
 * Compute ARGB to colormap index map.
 */

static void
ComputeARGBMap(Box *boxes, int colors, uchar *argbmap)
{
    int i;

    /*
     * The centroid of each box serves as the representative
     * for each color in the box.
     */
    for (i = 0; i < colors; i++)
        SetARGBmap(i, &boxes[i], argbmap);
}

/*
 * Make the centroid of "boxnum" serve as the representative for
 * each color in the box.
 */
static void
SetARGBmap(int boxnum, Box *box, uchar *argbmap)
{
    int a, r, g, b;
    
    for (a = box->low[ALPHAI]; a < box->high[ALPHAI]; a++) {
        for (r = box->low[REDI]; r < box->high[REDI]; r++) {
            for (g = box->low[GREENI]; g < box->high[GREENI]; g++) {
                    for (b = box->low[BLUEI]; b < box->high[BLUEI]; b++) {
                            int     index;
                            
                            index = MKIDX(a, r, g, b);
                            argbmap[index]=(char)boxnum;
                    }
            }
        }
    }
}

/* ---------------------------------------------------------------------- */

static unsigned char _txPixTrueToFixedPal( void *pix, const FxU32 *pal )
{
  int i;
  int min_dist;
  int min_index;
  int a, r, g, b;
  
  min_dist = 256 * 256 + 256 * 256 + 256 * 256 + 256 * 256;
  min_index = -1;
  /* 0 1 2 3 */
  a = ( int )( ( uchar * )pix )[3];
  r = ( int )( ( uchar * )pix )[2];
  g = ( int )( ( uchar * )pix )[1];
  b = ( int )( ( uchar * )pix )[0];

  for( i = 0; i < 256; i++ )
    {
      int pala, palr, palg, palb, dist;
      int da, dr, dg, db;

      pala = ( int )( ( pal[i] & 0xff000000 ) >> 24 );
      palr = ( int )( ( pal[i] & 0x00ff0000 ) >> 16 );
      palg = ( int )( ( pal[i] & 0x0000ff00 ) >> 8 );
      palb = ( int )(   pal[i] & 0x000000ff );
      da = pala - a;
      dr = palr - r;
      dg = palg - g;
      db = palb - b;
      dist = da * da + dr * dr + dg * dg + db * db;
      if( dist < min_dist )
        {
          min_dist = dist;
          min_index = i;
        }
    }

  if( min_index < 0 )
    txPanic( "_txPixTrueToFixedPal: this shouldn't happen\n" );

  //  printf( "%d\n", ( max_index ) );
  return ( unsigned char )min_index;
}

static void _txImgTrueToFixedPal( unsigned char *dst, unsigned char *src, const FxU32 *pal,
                           int w, int h, FxU32 flags )
{
  int i;

  for( i = 0; i < w * h; i++ )
    {
      if( flags == TX_FIXED_PAL_QUANT_TABLE )
        {
          uint index;
          uint a_index, r_index, g_index, b_index;
          
          a_index = ( ( ( uint )src[i*4+3] ) >> ( 8 - INVERSE_PAL_A_BITS ) );
          r_index = ( ( ( uint )src[i*4+2] ) >> ( 8 - INVERSE_PAL_R_BITS ) );
          g_index = ( ( ( uint )src[i*4+1] ) >> ( 8 - INVERSE_PAL_G_BITS ) );
          b_index = ( ( ( uint )src[i*4+0] ) >> ( 8 - INVERSE_PAL_B_BITS ) );
          index = MKIDX(a_index, r_index, g_index, b_index);
          dst[i] = inverse_pal[index];
        }
      else
        {
          dst[i] = _txPixTrueToFixedPal( &src[i*4], pal );
        }
    }
}

static void _CreateInversePal( const FxU32 *pal )
{
  int a, r, g, b;
  int index = 0;
  uchar true_color[NCOMP];

  for( a = 0; a < ( 1 << INVERSE_PAL_A_BITS ); a++ )
    for( r = 0; r < ( 1 << INVERSE_PAL_R_BITS ); r++ )
      for( g = 0; g < ( 1 << INVERSE_PAL_G_BITS ); g++ )
        for( b = 0; b < ( 1 << INVERSE_PAL_B_BITS ); b++ )
          {
            true_color[3] = ( uchar )( a << ( 8 - INVERSE_PAL_A_BITS ) ); 
            true_color[2] = ( uchar )( r << ( 8 - INVERSE_PAL_R_BITS ) );
            true_color[1] = ( uchar )( g << ( 8 - INVERSE_PAL_G_BITS ) );
            true_color[0] = ( uchar )( b << ( 8 - INVERSE_PAL_B_BITS ) );
            inverse_pal[index] = _txPixTrueToFixedPal( ( void * )true_color, pal );
            index++;
          }
}

/*
 * Convert an image from true color to a predefined palette.
 */
void txMipTrueToFixedPal6666( TxMip *outputMip, TxMip *trueColorMip, const FxU32 *pal,
                              FxU32 flags )
{
  int             i, w, h;
  static          FxU32 last_pal[256];
  static          FxBool been_here = FXFALSE;
  
  w = outputMip->width;
  h = outputMip->height;

  if( flags == TX_FIXED_PAL_QUANT_TABLE )
    {
      if( !been_here || ( memcmp( last_pal, pal, sizeof( FxU32 ) * 256 ) != 0 ) )
        {
          memcpy( last_pal, pal, sizeof( FxU32 ) * 256 );
          _CreateInversePal( pal );
          been_here = FXTRUE;
        }
    }

  for( i = 0; i < trueColorMip->depth; i++ ) 
    {
      _txImgTrueToFixedPal( outputMip->data[i], trueColorMip->data[i], pal,
                            w, h, flags );
      if (w > 1) w >>= 1;
      if (h > 1) h >>= 1;
    }
}
