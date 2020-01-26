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
 *
 * colorquant.c
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


#define USE_INVERSE_PAL
#define INVERSE_PAL_R_BITS 5
#define INVERSE_PAL_G_BITS 5
#define INVERSE_PAL_B_BITS 5
#define INVERSE_PAL_TOTAL_BITS ( INVERSE_PAL_R_BITS +  \
                                 INVERSE_PAL_G_BITS +  \
                                 INVERSE_PAL_B_BITS )

#ifdef USE_INVERSE_PAL
unsigned char inverse_pal[1<<INVERSE_PAL_TOTAL_BITS];
#endif

typedef unsigned int   uint;
typedef unsigned char   uchar;
typedef unsigned short  ushort;

#ifdef  HUGE
#undef  HUGE
#endif

#define HUGE    1.0e38

#define NBITS   5

#define MAXCOLORS       256
#define FULLINTENSITY   255

/*
 * Readability constants.
 */
#define REDI            0       
#define GREENI          1
#define BLUEI           2       
#define TRUE            1
#define FALSE           0
#ifndef bzero
#define bzero(ptr, sz)  memset(ptr, 0, sz)
#endif

typedef struct {
    float               weightedvar;            /* weighted variance */
    uint               mean[3];                        /* centroid */
    uint               weight;                         /* # of pixels in box */
    uint               freq[3][MAXCOLORS];     /* Projected frequencies */
    int                 low[3], high[3];        /* Box extent */
} Box;

#define COLORMAXI ( 1 << NBITS )
#if 0
static uint    *Histogram;             /* image histogram      */
#else
static uint    Histogram[COLORMAXI*COLORMAXI*COLORMAXI * sizeof(int)];
#endif
static uint    SumPixels;              /* total # of pixels    */
static uint    ColormaxI;              /* # of colors, 2^Bits */
static Box              _Boxes[MAXCOLORS];
static Box              *Boxes;                 /* Array of color boxes. */

static void     SetRGBmap(int boxnum, Box *box, uchar *rgbmap);
static void     ComputeRGBMap(Box *boxes, int colors, uchar *rgbmap);
static void     UpdateFrequencies(Box *box1, Box *box2);
static int      FindCutpoint(Box *box, int color, Box *newbox1, Box *newbox2);
static int      CutBox(Box *box, Box *newbox);
static void     BoxStats(Box *box);
static int      GreatestVariance(Box *boxes, int n);
static int      CutBoxes(Box *boxes, int colors);
static void     QuantHistogram(uint *pixels, int npixels, Box *box);

/*
 * Perform variance-based color quantization on a 24-bit image.
 */
int     
txMipPal256(TxMip *pxMip, TxMip *txMip, int format, FxU32 dither, FxU32 compression)
{
    int         w, h;
    int         i;                              /* Counter */
    int         OutColors;              /* # of entries computed */
    int         Colormax;               /* quantized full-intensity */ 
    float       Cfactor;                /* Conversion factor */
#if 0
    uchar       *rgbmap;                /* how to map colors to palette indices */
#else
    static uchar rgbmap[(1<<NBITS)*(1<<NBITS)*(1<<NBITS)]; /* how to map colors to palette indices */
#endif
    int         pixsize;


    ColormaxI = 1 << NBITS;     /* 2 ^ NBITS */
    Colormax = ColormaxI - 1;
    Cfactor = (float)FULLINTENSITY / Colormax;

    Boxes = _Boxes;     
#if 0
    Histogram = (uint *) txMalloc(ColormaxI*ColormaxI*ColormaxI * sizeof(int));
    rgbmap = txMalloc((1<<NBITS)*(1<<NBITS)*(1<<NBITS));
#endif

    /*
     * Zero-out the projected frequency arrays of the largest box.
     */
    bzero(Boxes->freq[0], ColormaxI * sizeof(uint));
    bzero(Boxes->freq[1], ColormaxI * sizeof(uint));
    bzero(Boxes->freq[2], ColormaxI * sizeof(uint));
    bzero(Histogram, ColormaxI * ColormaxI * ColormaxI * sizeof(int));

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
        uint   r, g, b;
        r = (uint)(Boxes[i].mean[REDI] * Cfactor + 0.5);
        g = (uint)(Boxes[i].mean[GREENI] * Cfactor + 0.5);
        b = (uint)(Boxes[i].mean[BLUEI] * Cfactor + 0.5);

        /*
        r &= 0xff;
        g &= 0xff;
        b &= 0xff;
        */
        if (r > 255) r = 255;
        if (g > 255) g = 255;
        if (b > 255) b = 255;

        pxMip->pal[i] = (r<<16) | (g << 8) | b;
    }
    ComputeRGBMap(Boxes, OutColors, rgbmap); 

    /*
     * Now translate the colors to palette indices.
     */
    pixsize = (format == GR_TEXFMT_P_8) ? 1 : 2;

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
                        int     r, g, b, argb, index;

                        argb = *src++;
                        r = (argb & 0x00FF0000) >> (16 + 8 - NBITS);
                        g = (argb & 0x0000FF00) >> ( 8 + 8 - NBITS);
                        b = (argb & 0x000000FF) >> ( 0 + 8 - NBITS);

                        index = (r << (NBITS+NBITS)) | (g << NBITS) | b;
                        if ((index < 0) || (index >= 32768)) {
                                printf("Bad index: %d (%d %d %d)\n", index, r, g, b);
                        }
                        if (pixsize == 1) {
                                *dst++ = rgbmap[index];
                        } else {
                                *(FxU16 *)dst = (rgbmap[index]) | 
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
    txFree((char *)rgbmap);
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
    uint *rf, *gf, *bf;
    uchar rr, gg, bb;
    int         i;

    rf = box->freq[0];
    gf = box->freq[1];
    bf = box->freq[2];

    /*
     * We compute both the histogram and the proj. frequencies of
     * the first box at the same time to save a pass through the
     * entire image. 
     */
    
    for (i = 0; i < npixels; i++) {
        rr = (uchar) (((*pixels >> 16) & 0xff) >> (8-NBITS));
        gg = (uchar) (((*pixels >>  8) & 0xff) >> (8-NBITS));
        bb = (uchar) (((*pixels      ) & 0xff) >> (8-NBITS));
        pixels++;
        rf[rr]++;
        gf[gg]++;
        bf[bb]++;
        Histogram[(((rr<<NBITS)|gg)<<NBITS)|bb]++;
    }
        
}

/*
 * Interatively cut the boxes.
 */
static int
CutBoxes(Box *boxes, int colors) 
{
    int curbox;

    boxes[0].low[REDI] = boxes[0].low[GREENI] = boxes[0].low[BLUEI] = 0;
    boxes[0].high[REDI] = boxes[0].high[GREENI] =
                      boxes[0].high[BLUEI] = ColormaxI;
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
    for (color = 0; color < 3; color++) {
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
    float totalvar[3];
    Box newboxes[3][2];

    if (box->weightedvar == 0. || box->weight == 0)
        /*
         * Can't cut this box.
         */
        return FALSE;

    /*
     * Find 'optimal' cutpoint along each of the red, green and blue
     * axes.  Sum the variances of the two boxes which would result
     * by making each cut and store the resultant boxes for 
     * (possible) later use.
     */
    for (i = 0; i < 3; i++) {
        if (FindCutpoint(box, i, &newboxes[i][0], &newboxes[i][1]))
                totalvar[i] = newboxes[i][0].weightedvar +
                        newboxes[i][1].weightedvar;
        else
                totalvar[i] = (float) HUGE;
    }

    /*
     * Find which of the three cuts minimized the total variance
     * and make that the 'real' cut.
     */
    if (totalvar[REDI] <= totalvar[GREENI] &&
        totalvar[REDI] <= totalvar[BLUEI]) {
        *box = newboxes[REDI][0];
        *newbox = newboxes[REDI][1];
    } else if (totalvar[GREENI] <= totalvar[REDI] &&
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
    int b, g, r;
    int roff;

    bzero(box1->freq[0], ColormaxI * sizeof(uint));
    bzero(box1->freq[1], ColormaxI * sizeof(uint));
    bzero(box1->freq[2], ColormaxI * sizeof(uint)); 

    for (r = box1->low[0]; r < box1->high[0]; r++) {
        roff = r << NBITS;
        for (g = box1->low[1];g < box1->high[1]; g++) {
                b = box1->low[2];
                h = Histogram + (((roff | g) << NBITS) | b);
                for (; b < box1->high[2]; b++) {
                        if ((myfreq = *h++) == 0)
                                continue;
                        box1->freq[0][r] += myfreq;
                        box1->freq[1][g] += myfreq;
                        box1->freq[2][b] += myfreq;
                        box2->freq[0][r] -= myfreq;
                        box2->freq[1][g] -= myfreq;
                        box2->freq[2][b] -= myfreq;
                }
        }
    }
}

/*
 * Compute RGB to colormap index map.
 */

static void
ComputeRGBMap(Box *boxes, int colors, uchar *rgbmap)
{
    int i;

    /*
     * The centroid of each box serves as the representative
     * for each color in the box.
     */
    for (i = 0; i < colors; i++)
        SetRGBmap(i, &boxes[i], rgbmap);
}

/*
 * Make the centroid of "boxnum" serve as the representative for
 * each color in the box.
 */
static void
SetRGBmap(int boxnum, Box *box, uchar *rgbmap)
{
    int r, g, b;
    
    for (r = box->low[REDI]; r < box->high[REDI]; r++) {
        for (g = box->low[GREENI]; g < box->high[GREENI]; g++) {
                for (b = box->low[BLUEI]; b < box->high[BLUEI]; b++) {
                        int     index;
                        
                        index = (((r<<NBITS)|g)<<NBITS)|b;
                        rgbmap[index]=(char)boxnum;
                }
        }
    }
}

/* ---------------------------------------------------------------------- */

unsigned char _txPixTrueToFixedPal( void *pix, const FxU32 *pal )
{
  int i;
  int min_dist;
  int min_index;
  int r, g, b;
  
  min_dist = 256 * 256 + 256 * 256 + 256 * 256;
  min_index = -1;
  /* 0 1 2 */
  r = ( int )( ( unsigned char * )pix )[2];
  g = ( int )( ( unsigned char * )pix )[1];
  b = ( int )( ( unsigned char * )pix )[0];

  for( i = 0; i < 256; i++ )
    {
      int palr, palg, palb, dist;
      int dr, dg, db;

      palr = ( int )( ( pal[i] & 0x00ff0000 ) >> 16 );
      palg = ( int )( ( pal[i] & 0x0000ff00 ) >> 8 );
      palb = ( int )( pal[i] & 0x000000ff );
      dr = palr - r;
      dg = palg - g;
      db = palb - b;
      dist = dr * dr + dg * dg + db * db;
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

void _txImgTrueToFixedPal( unsigned char *dst, unsigned char *src, const FxU32 *pal,
                           int w, int h, FxU32 flags )
{
  int i;

  for( i = 0; i < w * h; i++ )
    {
      if( flags == TX_FIXED_PAL_QUANT_TABLE )
        {
          unsigned int index;
          unsigned int r_index, g_index, b_index;
          
          r_index = ( ( ( unsigned int )src[i*4+2] ) >> ( 8 - INVERSE_PAL_R_BITS ) );
          g_index = ( ( ( unsigned int )src[i*4+1] ) >> ( 8 - INVERSE_PAL_G_BITS ) );
          b_index = ( ( ( unsigned int )src[i*4+0] ) >> ( 8 - INVERSE_PAL_B_BITS ) );
          index = 
            ( r_index << ( INVERSE_PAL_G_BITS + INVERSE_PAL_B_BITS ) ) |
            ( g_index << INVERSE_PAL_B_BITS ) |
            b_index;
          dst[i] = inverse_pal[index];
        }
      else
        {
          dst[i] = _txPixTrueToFixedPal( &src[i*4], pal );
        }
    }
}

void _CreateInversePal( const FxU32 *pal )
{
  int r, g, b;
  int index = 0;
  unsigned char true_color[4];

  true_color[3] = 0;
  for( r = 0; r < ( 1 << INVERSE_PAL_R_BITS ); r++ )
    for( g = 0; g < ( 1 << INVERSE_PAL_G_BITS ); g++ )
      for( b = 0; b < ( 1 << INVERSE_PAL_B_BITS ); b++ )
        {
          true_color[2] = ( unsigned char )( r << ( 8 - INVERSE_PAL_R_BITS ) );
          true_color[1] = ( unsigned char )( g << ( 8 - INVERSE_PAL_G_BITS ) );
          true_color[0] = ( unsigned char )( b << ( 8 - INVERSE_PAL_B_BITS ) );
          inverse_pal[index] = _txPixTrueToFixedPal( ( void * )true_color, pal );
          index++;
        }
}

/*
 * Convert an image from true color to a predefined palette.
 */
void txMipTrueToFixedPal( TxMip *outputMip, TxMip *trueColorMip, const FxU32 *pal,
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
