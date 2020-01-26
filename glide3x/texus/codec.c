#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "texusint.h"
#include "sst2fxt1.h"

#define SQUARED(x)  ((x)*(x))
#define ABS(x)      (((x) < 0) ? -(x) : (x))

int     globalX, globalY;
int     a_color_cc  = -1; // force color coding
int     a_force_cc  = -1; // force color mode
int     a_tolerance = 0;
int     a_lerp = 0; // force interpolation in alpha mode


#define static 

static int
bestColor(
    const float  *a, 
    const float  codebook[][3], 
    int          codesize)
{
    int     i;
    int     bestindex;
    float   bestdist;
    float   dist[8];

    for (i=0; i < codesize; i++) {
        dist[i] = SQUARED(a[0] - codebook[i][0]) +
                  SQUARED(a[1] - codebook[i][1]) +
                  SQUARED(a[2] - codebook[i][2]) ;
    }
    for ( i=1, bestindex=0, bestdist = dist[0]; 
          i < codesize; 
          i++) {
        if (dist[i] < bestdist) {
            bestdist  = dist[i];
            bestindex = i;
        }
    }
    return bestindex;
}

static int
bestColorError(
    const float  *a, 
    const float  codebook[][3], 
    int          codesize,
    float        *error)     // RETURN
{
    int     i;
    int     bestindex;
    float   bestdist;
    float   dist[8];

    for (i=0; i < codesize; i++) {
        dist[i] = SQUARED(a[0] - codebook[i][0]) +
                  SQUARED(a[1] - codebook[i][1]) +
                  SQUARED(a[2] - codebook[i][2]) ;
    }
    for ( i=1, bestindex=0, bestdist = dist[0]; 
          i < codesize; 
          i++) {
        if (dist[i] < bestdist) {
            bestdist  = dist[i];
            bestindex = i;
        }
    }
    *error = bestdist;
    return bestindex;
}

// Usable only for the interpolation compression modes.
// XXX Susceptible to rounding errors?
static int
bestColorRGBInterp(
    const float  a[3], 
    const float  iv[3],
    const float  b,
    const int    codesize)
{
    int t = (int)((a[0]*iv[0] + a[1]*iv[1] + a[2]*iv[2]) + b);
    if ( t <= 0)
        return 0;
    else if ( t >= codesize)
        return codesize-1;
    else
        return t;
}

static int
bestColorAlpha(
    const float  *c, 
    const float   a, 
    const float  codebook[][4], 
    const int    codesize,
    const FxU32  lerp)
{
    int     i;
    int     bestindex = -1;
    float   bestdist  = 1.0e30F;
    float   d0, d1, d2, dist;

    if (!lerp && ( c[0] == 0.0f ) && ( c[1] == 0.0f ) &&  ( c[2] == 0.0f ) && ( a == 0.0f ))
        return 3;

    for (i=0; i < codesize; i++) {
        if ( a_lerp ) {
            d0 = SQUARED(c[0]*a - codebook[i][0]*codebook[i][3]);
            d1 = SQUARED(c[1]*a - codebook[i][1]*codebook[i][3]);
            d2 = SQUARED(c[2]*a - codebook[i][2]*codebook[i][3]);

            dist = SQUARED(c[0]*a - codebook[i][0]*codebook[i][3]) +
                   SQUARED(c[1]*a - codebook[i][1]*codebook[i][3]) +
                   SQUARED(c[2]*a - codebook[i][2]*codebook[i][3]);
            dist = d0 + d1 + d2;
        } else {
            dist = SQUARED(c[0] - codebook[i][0]) +
                   SQUARED(c[1] - codebook[i][1]) +
                   SQUARED(c[2] - codebook[i][2]) +
                   SQUARED(a - codebook[i][3]);
        }
        if (dist < bestdist) {
            bestdist  = dist;
            bestindex = i;
        }
    }
    if (( c[0] < 10.f ) && ( codebook[bestindex][0] > 50.f ))
        return bestindex;
        
    return bestindex;
}

/* Given either end points for the 2 colors, generate intermediate colors */
// XXX The output fpal's should be quantized to 555, except that the newer, faster, 
//     bestColorRGBInterp() cannot easily use them.
static void
makePalette( FxU32 lo, FxU32 hi, int nlevels, 
             float fpal[][3])          // output, range [0.5f,255.5f], quantized to Z+0.5f 
{
    int rlo, glo, blo, alo, rhi, ghi, bhi, ahi, r, g, b, a, i;

    assert((nlevels == 7) || (nlevels == 4) || (nlevels == 3));

    alo = ALF(lo);
    rlo = RED(lo);
    glo = GRN(lo);
    blo = BLU(lo);

    ahi = ALF(hi);
    rhi = RED(hi);
    ghi = GRN(hi);
    bhi = BLU(hi);

    for (i=0; i < nlevels; i++) {
        a = alo + ((ahi - alo) * i) / (nlevels - 1);
        r = rlo + ((rhi - rlo) * i) / (nlevels - 1);
        g = glo + ((ghi - glo) * i) / (nlevels - 1);
        b = blo + ((bhi - blo) * i) / (nlevels - 1);

        /* make sure all values are within 0..255 */
        assert( ((a & ~0xff) == 0) && ((r & ~0xff) == 0) && 
                ((g & ~0xff) == 0) && ((b & ~0xff) == 0) );

        fpal[i][0] = (float)r + 0.5f;  // map to [0.5f,255.5f]
        fpal[i][1] = (float)g + 0.5f;
        fpal[i][2] = (float)b + 0.5f;
    }
}

// Returns a vector 'iv' that when dotted with a color and added to an offset 'b', 
// finds the index of the Cartesian-nearest color in the (linear) palette. 
//
// Imagine the set of ncolors-1 planes in color space, each normal to the line through
// the color palette, that partition color space into slab-shaped sets of points, each slab
// belonging to a particular palette entry.  This procedure implements the first phase
// of that mapping by reducing the color palette to a vector 'iv' and offset 'b' for
// later use by bestColorRGBInterp().
static void makeInterpVector( float p[8][3],  // 8 is an upper bound by the design of FXT1
                              int ncolors, 
                              float iv[3],    // RETURN
                              float *b)       // RETURN
{
    float d2 = 0.0f;
    float rd2;
    int i;

    for ( i=0; i<3; i++) {
        iv[i] = p[ncolors-1][i] - p[0][i];  // vector between extrema of palette; may be zero
        d2 += iv[i]*iv[i];                  // accumulate square of Cartesian distance
    }
    rd2 = (float)(ncolors-1) / d2;          // if all iv[] are 0, rd2 is Infinity
    *b = (float)0.0;
    for ( i=0; i<3; i++) {
        *b -= iv[i]*p[0][i];
        iv[i] *= rd2;                       // if rd2 is Infinity and iv[i] was 0, result is NaN
    }
    *b = *b * rd2 + 0.5f;
}

/* Similar to makePalette, but in addition Alpha component of fpal is computed. */
static void
makePaletteAlpha( FxU32 lo, FxU32 hi, int nlevels, 
                  float fpal[][4])          // output, range [0.5f,255.5f], quantized to Z+0.5f 
{
    int rlo, glo, blo, alo, rhi, ghi, bhi, ahi, r, g, b, a, i;

    assert((nlevels == 7) || (nlevels == 4) || (nlevels == 3));

    alo = ALF(lo);
    rlo = RED(lo);
    glo = GRN(lo);
    blo = BLU(lo);

    ahi = ALF(hi);
    rhi = RED(hi);
    ghi = GRN(hi);
    bhi = BLU(hi);

    for (i=0; i < nlevels; i++) {
        a = alo + ((ahi - alo) * i) / (nlevels - 1);
        r = rlo + ((rhi - rlo) * i) / (nlevels - 1);
        g = glo + ((ghi - glo) * i) / (nlevels - 1);
        b = blo + ((bhi - blo) * i) / (nlevels - 1);

        /* make sure all values are within 0..255 */
        assert( ((a & ~0xff) == 0) && ((r & ~0xff) == 0) && 
                ((g & ~0xff) == 0) && ((b & ~0xff) == 0) );

        fpal[i][0] = (float)r + 0.5f;    // map to [0.5f,255.5f]
        fpal[i][1] = (float)g + 0.5f;
        fpal[i][2] = (float)b + 0.5f;
        fpal[i][3] = (float)a + 0.5f;
    }
}

/* rgb5555 to 8888 by msb replication */
static FxU32 
argb8888( FxU32 rgb5555 )
{
    FxU32 a = (rgb5555 >> 15) & 0x1f;
    FxU32 r = (rgb5555 >> 10) & 0x1f;
    FxU32 g = (rgb5555 >>  5) & 0x1f;
    FxU32 b = (rgb5555      ) & 0x1f;

    a = (a << 3) | (a >> 2);    
    r = (r << 3) | (r >> 2);    
    g = (g << 3) | (g >> 2);
    b = (b << 3) | (b >> 2);

    return ARGB( a, r, g, b);
}

/* rgb555 to 888 by msb replication */
static FxU32 
rgb888( FxU32 rgb555 )
{
    FxU32 r = (rgb555 >> 10) & 0x1f;
    FxU32 g = (rgb555 >>  5) & 0x1f;
    FxU32 b = (rgb555      ) & 0x1f;

    r = (r << 3) | (r >> 2);    
    g = (g << 3) | (g >> 2);
    b = (b << 3) | (b >> 2);

    return ARGB( 0xff, r, g, b);
}

/* rgb565 to 888 by msb replication */
static FxU32 
rgb565_888( FxU32 rgb565 )
{
    FxU32 r = (rgb565 >> 11) & 0x1f;
    FxU32 g = (rgb565 >>  5) & 0x3f;
    FxU32 b = (rgb565      ) & 0x1f;

    r = (r << 3) | (r >> 2);    
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);

    return ARGB( 0xff, r, g, b);
}

// Simply truncate, for symmetry with promotion by replication.
/* rgb888 to 565 */
static FxU32
rgb565( FxU32 rgb888 )
{
    FxU32 r = (RED(rgb888)) >> 3;
    FxU32 g = (GRN(rgb888)) >> 2;
    FxU32 b = (BLU(rgb888)) >> 3;

    return (r << 11) | (g << 5) | b;
}

/* argb8888 to 5555 */
static FxU32
argb5555( FxU32 argb8888 )
{
    FxU32 a = (ALF(argb8888)) >> 3;
    FxU32 r = (RED(argb8888)) >> 3;
    FxU32 g = (GRN(argb8888)) >> 3;
    FxU32 b = (BLU(argb8888)) >> 3;

    return (a << 15 ) | (r << 10) | (g << 5) | b;
}

/* rgb888 to 555 */
static FxU32
rgb555( FxU32 rgb888 )
{
    FxU32 r = (RED(rgb888)) >> 3;
    FxU32 g = (GRN(rgb888)) >> 3;
    FxU32 b = (BLU(rgb888)) >> 3;

    return (r << 10) | (g << 5) | b;
}
/*
 * The eigen vector generated may sometimes have endpoints that are outside
 * the rgb color space. We clip it along the line, and move endpoints within
 * the color space.
 */
void
clipLine(float lo[3], float hi[3])
{
    int i;
#if 0
    int j;
    int cclo, cchi;
    int swapped = 0;

again:
    cclo = 0;
    cchi = 0;

    for (i=0; i<3; i++) {
	  if (lo[i] <   0.0f) cclo |= (1 <<     i);
	  if (hi[i] <   0.0f) cchi |= (1 <<     i);

	  if (lo[i] > 255.9999f) cclo |= (1 << (3+i));
	  if (hi[i] > 255.9999f) cchi |= (1 << (3+i));
    }

    if (cclo & cchi) {
	  // trivial reject. Bad news.
#if PRINT
	  fprintf(stderr, "\nBad   : [%4.0f %4.0f %4.0f][%4.0f %4.0f %4.0f]\n",
	      lo[0], lo[1], lo[2], hi[0], hi[1], hi[2]);
#endif

	  // Try to fix it directly by clamping (Really bad, this)
	  if ((cclo & cchi) & 0x01) lo[0] = hi[0] =   0.0f;
	  if ((cclo & cchi) & 0x02) lo[1] = hi[1] =   0.0f;
	  if ((cclo & cchi) & 0x04) lo[2] = hi[2] =   0.0f;

	  if ((cclo & cchi) & 0x08) lo[0] = hi[0] = 255.9999f;
	  if ((cclo & cchi) & 0x10) lo[1] = hi[1] = 255.9999f;
	  if ((cclo & cchi) & 0x20) lo[2] = hi[2] = 255.9999f;

#if PRINT
	  fprintf(stderr, "\nFixed : [%4.0f %4.0f %4.0f][%4.0f %4.0f %4.0f]\n",
	      lo[0], lo[1], lo[2], hi[0], hi[1], hi[2]);
#endif

    } else if ((cclo | cchi) == 0){
	  // trivial accept
	  return;
    }

#if PRINT
    fprintf(stderr, "ClipIn : [%8.4f %8.4f %8.4f] - [%8.4f %8.4f %8.4f]\n",
	  lo[0], lo[1], lo[2], hi[0], hi[1], hi[2]);
#endif
    for (i=0; i<3; i++) {
	  float   t;

	  // Travel towards the center, shortening all coordinates.
	  if (lo[i] <   0.0f) {
	      t = (  0.0f - hi[i]) / (lo[i] - hi[i]);
	  } else if (lo[i] > 255.9999f) {
	      t = (255.9999f - hi[i]) / (lo[i] - hi[i]);
	  }
	  else continue;

	  // Shorten all coordinates by this amount.
	  for (j=0; j<3; j++) {
	      lo[j] = hi[j] + (lo[j] - hi[j]) * t;
	  }

	  // Account for round-off errors.
	  // if (lo[i] < 0.0f) lo[i] = 0.0f;
	  // else if (lo[i] > 255.0f) lo[i] = 255.0f;

#if PRINT
	  fprintf(stderr, "ClipMid: [%8.4f %8.4f %8.4f] - [%8.4f %8.4f %8.4f]\n",
	      lo[0], lo[1], lo[2], hi[0], hi[1], hi[2]);
#endif
    }

#if PRINT
    fprintf(stderr, "ClipOut: [%8.4f %8.4f %8.4f] - [%8.4f %8.4f %8.4f]\n",
	  lo[0], lo[1], lo[2], hi[0], hi[1], hi[2]);
    fflush(stderr);
#endif
    // There might be some roundoff errors, so we fudge.
    for (i=0; i<3; i++) {
	  if ((lo[i] <   0.0f) /* && (lo[i] >   -2.0f)*/) lo[i] = 0.0f;
	  if ((lo[i] > 255.9999f) /* && (lo[i] <  257.0f)*/) lo[i] = 255.9999f;

	  if ((lo[i] < 0.0f) || (lo[i] > 255.9999f)) {
	      fprintf(stderr, "\n Bad color: %4.0f %4.0f %4.0f\n", 
		  lo[0], lo[1], lo[2]);
	  }
    }
    if (!swapped) {
	  // reverse end points and do it again.
	  float  *tmp;
	  swapped = 1;
	  tmp = lo; lo = hi; hi = tmp;
	  goto again;
    }
#else // 1
    for (i=0; i<3; i++) {
        if ((lo[i] <   0.5f))
            lo[i] = 0.5f;
        if ((lo[i] > 255.5f)) 
            lo[i] = 255.5f;
        if ((hi[i] <   0.5f))
            hi[i] = 0.5f;
        if ((hi[i] > 255.5f)) 
            hi[i] = 255.5f;
    }
#endif // 1
}

/* 
 * Given that lo and hi differ by less than 16 on all 3 coords, encode it
 * as a midpoint color at 666 resolution, plus a 12 bit signed delta.
 */
static FxU32
encodeDelta( float c0[3], float c1[3])
{
    int         r, g, b, dr, dg, db;

#if 0
    printf("Encode Colors: [%3d %3d %3d] - [%3d %3d %3d] at %3d %3d\n",
        (int) c0[0], (int) c0[1], (int) c0[2], 
        (int) c1[0], (int) c1[1], (int) c1[2], globalX, globalY);
#endif

    r = (int) ((c0[0] + c1[0]) * 0.5f);
    g = (int) ((c0[1] + c1[1]) * 0.5f);
    b = (int) ((c0[2] + c1[2]) * 0.5f);


    /* round to rgb666 and back to 888 */
    r = (r + 2) >> 2;
    g = (g + 2) >> 2;
    b = (b + 2) >> 2;

    if (r > 0x3f) r = 0x3f;
    if (g > 0x3f) g = 0x3f;
    if (b > 0x3f) b = 0x3f;

    r <<= 2;
    g <<= 2;
    b <<= 2;

    /* Generate half the delta value */
    dr = (int) ((c0[0] - c1[0]) * 0.5f);
    dg = (int) ((c0[1] - c1[1]) * 0.5f);
    db = (int) ((c0[2] - c1[2]) * 0.5f);

    /* Ensure it's within -8 to +7 */
    if (dr < -8) dr = -8;
    if (dg < -8) dg = -8;
    if (db < -8) db = -8;

    if (dr >  7) dr = 7;
    if (dg >  7) dg = 7;
    if (db >  7) db = 7;

    if (dr < 0) {
        if ((r + dr) <   0) dr =   0 -   r; 
        if ((r - dr) > 255) dr =   r - 255;
    } else {
        if ((r - dr) <   0) dr =   r -   0; 
        if ((r + dr) > 255) dr = 255 -   r;
    }

    if (dg < 0) {
        if ((g + dg) <   0) dg =   0 -   g; 
        if ((g - dg) > 255) dg =   g - 255;
    } else {
        if ((g - dg) <   0) dg =   g -   0; 
        if ((g + dg) > 255) dg = 255 -   g;
    }

    if (db < 0) {
        if ((b + db) <   0) db =   0 -   b; 
        if ((b - db) > 255) db =   b - 255;
    } else {
        if ((b - db) <   0) db =   b -   0; 
        if ((b + db) > 255) db = 255 -   b;
    }

    // printf("Mid pts = [%3d %3d %3d]\n", r, g, b);
    // printf("deltas = %d %d %d\n", dr, dg, db);

    /* So here's the new c0 and c1 values you would use for the palette */
    c0[0] = (float) (r - dr);
    c0[1] = (float) (g - dg);
    c0[2] = (float) (b - db);

    c1[0] = (float) (r + dr);
    c1[1] = (float) (g + dg);
    c1[2] = (float) (b + db);

#if 0
    printf("New endpts: [%3d %3d %3d] - [%3d %3d %3d]\n",
        (int) c0[0], (int) c0[1], (int) c0[2], 
        (int) c1[0], (int) c1[1], (int) c1[2]);
#endif

    fflush(stderr);


    assert((dr >= -8) && (dr <= 7) && 
           (dg >= -8) && (dg <= 7) && 
           (db >= -8) && (db <= 7));

    assert((c0[0] >= 0.0f) && (c0[1] >= 0.0f) && (c0[2] >= 0.0f));
    assert((c1[0] >= 0.0f) && (c1[1] >= 0.0f) && (c1[2] >= 0.0f));

    assert((c0[0] <= 255.9999f) && (c0[1] <= 255.9999f) && (c0[2] <= 255.9999f));
    assert((c1[0] <= 255.9999f) && (c1[1] <= 255.9999f) && (c1[2] <= 255.9999f));

    /* This will be encoded as an rgb666 + drgb444 */
    r >>= 2;
    g >>= 2;
    b >>= 2;
    r = (r << 12) | (g << 6) | b;
    dr = ((dr & 0xf) << 8) | ((dg & 0xf) << 4) | ((db & 0xf));

    return (r << 12) | dr | (0x1 << 30);        // the delta mode bit  is 30 
}

static void
decodeDelta( FxU32 col, FxU32 *lo, FxU32 *hi )
{
    int r, g, b, dr, dg, db;
    int rlo, glo, blo, rhi, ghi, bhi;

    db = col & 0x0f; col >>= 4;
    dg = col & 0x0f; col >>= 4;
    dr = col & 0x0f; col >>= 4;
    b  = col & 0x3f; col >>= 6;
    g  = col & 0x3f; col >>= 6;
    r  = col & 0x3f; col >>= 6;

    /* sign extend the deltas */
    if (dr & 8) dr |= 0xfffffff0;
    if (dg & 8) dg |= 0xfffffff0;
    if (db & 8) db |= 0xfffffff0;

    /* make rgb666 to 888 */
    r <<= 2;
    g <<= 2;
    b <<= 2;

    rlo = r - dr;
    glo = g - dg;
    blo = b - db;

    rhi = r + dr;
    ghi = g + dg;
    bhi = b + db;

    // printf("Decode: mid = [%3d %3d %3d] deltas = [%3d %3d %3d] at %3d %3d\n",
    //     r, g, b, dr, dg, db, globalX, globalY );
    // printf("Decode delta: [%3d %3d %3d] - [%3d %3d %3d]\n",
    //     rlo, glo, blo, rhi, ghi, bhi );
    // fflush(stderr);

    assert((rlo >=   0) && (glo >=   0) && (blo >=   0));
    assert((rlo <= 255) && (glo <= 255) && (blo <= 255));

    assert((rhi >=   0) && (ghi >=   0) && (bhi >=   0));
    assert((rhi <= 255) && (ghi <= 255) && (bhi <= 255));

    *lo = ARGB( 255, rlo, glo, blo);
    *hi = ARGB( 255, rhi, ghi, bhi);
}

// XXXdwm  Vtune says __ftol takes some 5% of the time.  Could __ftol's below be batched somehow?
static void 
encodeColors(int mode, int mixmode, int alpha, float c0[3], float c1[3], float c2[3], float c3[3], 
             float input[][3], FxI32 ainput[], void *bits)
{
    int         i, sel, index[32];
    FxU32       lo, hi, col[4];
    float       fpal[8][3];
    float       iv[3];
    float       b;

    switch(mode) {
    case TCC_HI:
        clipLine(c0, c1);
        lo = ARGB( 255, (int) c0[0], (int) c0[1], (int) c0[2]);
        hi = ARGB( 255, (int) c1[0], (int) c1[1], (int) c1[2]);

        col[0] = rgb555( lo );
        col[1] = rgb555( hi );

        lo = rgb888( col[0] );
        hi = rgb888( col[1] );
        makePalette( lo, hi, 7, fpal);
        makeInterpVector( fpal, 7, iv, &b);

        /* Map input colors to closest entry in the palette */
        for (i=0; i<32; i++) {
            if ( alpha && ( ainput[i] == 0 ))
                 index[i] = 7;
            else 
#define RGB_INTERP 1
#if ! RGB_INTERP
                 index[i] = bestColor((float *) &input[i][0], fpal, 7);
#else
                 index[i] = bestColorRGBInterp( (float *)&input[i][0], iv, b, 7);
#endif
        }

        /* Now encode these into the 128 bits */
        bitEncoder( mode, col, alpha, index, bits);
        break;

    case TCC_MIXED:
        clipLine(c0, c1);
        clipLine(c2, c3);

        /* Deal with even block */
        lo = ARGB( 255, (int) c0[0], (int) c0[1], (int) c0[2]);
        hi = ARGB( 255, (int) c1[0], (int) c1[1], (int) c1[2]);
        if (alpha) {
            col[0] = rgb555(lo);
            col[1] = rgb555(hi);
        } else {
            col[0] = rgb565(lo);
            col[1] = rgb565(hi);
        }
        makePalette( lo, hi, alpha ? 3 : 4, fpal);
        makeInterpVector( fpal, alpha ? 3 : 4, iv, &b);

        /* Map input colors to closest entry in the palette */
        for (i=0; i<16; i++) {
            if ( alpha && ( ainput[i] == 0 ))
                 index[i] = 3;
            else 
#if ! RGB_INTERP
                 index[i] = bestColor((float *) &input[i][0], fpal, alpha ? 3 : 4);
#else
                 index[i] = bestColorRGBInterp( (float *)&input[i][0], iv, b, alpha ? 3 : 4);
#endif
        }

        sel = alpha;

        // funky encoding for lsb of green
        if (!alpha) {
           if (( (FxU32)index[0] >> 1 ) != (( (col[0]>>5) & 0x1 ) ^ ( (col[1]>>5) & 0x1 )) ) {
               FxU32 tmp = col[0];
               col[0] = col[1];
               col[1] = tmp;
               for (i=0; i<16; i++) {
                 index[i] ^= 3;
               }
           }
           sel |= ( (col[1]>>5) & 0x1 )<<1;
           // remove lsb of green
           col[0] = ((col[0] & 0xFFC0) >> 1) | (col[0] & 0x1F);
           col[1] = ((col[1] & 0xFFC0) >> 1) | (col[1] & 0x1F);
        }

        /* Now deal with odd block */
        lo = ARGB( 255, (int) c2[0], (int) c2[1], (int) c2[2]);
        hi = ARGB( 255, (int) c3[0], (int) c3[1], (int) c3[2]);
        if (alpha) {
            col[2] = rgb555(lo);
            col[3] = rgb555(hi);
        } else {
            col[2] = rgb565(lo);
            col[3] = rgb565(hi);
        }
        makePalette( lo, hi, alpha ? 3 : 4, fpal);
        makeInterpVector( fpal, alpha ? 3 : 4, iv, &b);

        /* Map input colors to closest entry in the palette */
        for (i=16; i<32; i++) {
            if ( alpha && ( ainput[i] == 0 ))
                 index[i] = 3;
            else 
#if ! RGB_INTERP
                 index[i] = bestColor((float *) &input[i][0], fpal, alpha ? 3 : 4);
#else
                 index[i] = bestColorRGBInterp( (float *)&input[i][0], iv, b, alpha ? 3 : 4);
#endif
        }

        // funky encoding for lsb of green
        if (!alpha) {
           if (( (FxU32)index[16] >> 1 ) != (( (col[2]>>5) & 0x1 ) ^ ( (col[3]>>5) & 0x1 )) ) {
               FxU32 tmp = col[2];
               col[2] = col[3];
               col[3] = tmp;
               for (i=16; i<32; i++) {
                 index[i] ^= 3;
               }
           }
           sel |= ( (col[3]>>5) & 0x1 )<<2;
           // remove lsb of green
           col[2] = ((col[2] & 0xFFC0) >> 1) | (col[2] & 0x1F);
           col[3] = ((col[3] & 0xFFC0) >> 1) | (col[3] & 0x1F);
        }

        /* Now encode these into the 128 bits */
        bitEncoder( mode, col, sel, index, bits);
        break;

    case TCC_CHROMA:
        /* map float palette colors to int by truncation */
        col[0] = ARGB( 255, (int) c0[0], (int) c0[1], (int) c0[2]);
        col[1] = ARGB( 255, (int) c1[0], (int) c1[1], (int) c1[2]);
        col[2] = ARGB( 255, (int) c2[0], (int) c2[1], (int) c2[2]);
        col[3] = ARGB( 255, (int) c3[0], (int) c3[1], (int) c3[2]);

        // produce floats quantized to 555
        for (i=0; i < 4; i++) {
            int rgb;

            col[i] = rgb555( col[i] );
            rgb = rgb888( col[i] ); 

            /* map to float [0.5,255.5], so as to agree with range of input[][] */
            fpal[i][0] = (float) RED(rgb) + 0.5f;
            fpal[i][1] = (float) GRN(rgb) + 0.5f;
            fpal[i][2] = (float) BLU(rgb) + 0.5f;
        }

        /* Map input colors to closest entry in the palette */
        for (i=0; i<32; i++) {
            index[i] = bestColor(&input[i][0], (const float (*)[3])fpal, 4);
        }

        /* Now encode these into the 128 bits */
        bitEncoder( mode, col, alpha, index, bits);
        break;


    default:
#if PRINT
        printf("NYI in encodeColors\n");
#endif
        exit(0);
    }
}

static void
decodeColors( void *bits, float output[][4] )
{
    int         i, mode, index[32];
    FxU32       col[4], lo, hi; 
    float       fpal[8][3];
    FxU32       alpha, glsb;

   mode = bitDecoder( bits, col, index, &alpha);
    switch(mode) {
    case TCC_HI:
        lo = rgb888(col[0]);
        hi = rgb888(col[1]);
        makePalette(lo, hi, 7, fpal);

        for (i=0; i<32; i++) {
            int j = index[i];
            if ( j == 7 ) {
                output[i][0] = 
                output[i][1] = 
                output[i][2] = 
                output[i][3] = 0.0f;
            } else {
                output[i][0] = 255.0f;
                output[i][1] = fpal[j][0];
                output[i][2] = fpal[j][1];
                output[i][3] = fpal[j][2];
           }
        }
        break;

    case TCC_MIXED:
        glsb = alpha >> 1;
        alpha &= 0x1;
        if ( alpha ) {
            lo = rgb888( col[0] );
            hi = rgb888( col[1] );
        } else {
            // compute 565 colors
            col[0] = (( col[0] & 0x7fe0 ) << 1 ) | ( col[0] & 0x1f ) |
                     (((index[0]>> 1) ^ ( glsb & 0x1)) << 5);
            col[1] = (( col[1] & 0x7fe0 ) << 1 ) | ( col[1] & 0x1f ) |
                     (( glsb & 0x1) << 5);
            lo = rgb565_888( col[0] );
            hi = rgb565_888( col[1] );
        }
        makePalette(lo, hi, alpha ? 3 : 4, fpal);
        for (i=0; i<16; i++) {
            int j = index[i];
            if ( alpha && ( j == 3 )) {
                output[i][0] = 
                output[i][1] = 
                output[i][2] = 
                output[i][3] = 0.0f;
            } else {
                output[i][0] = 255.0f;
                output[i][1] = fpal[j][0];
                output[i][2] = fpal[j][1];
                output[i][3] = fpal[j][2];
            }
        }
        if ( alpha ) {
            lo = rgb888( col[2] );
            hi = rgb888( col[3] );
        } else {
            // compute 565 colors
            col[2] = (( col[2] & 0x7fe0 ) << 1 ) | ( col[2] & 0x1f ) |
                     (((index[16]>> 1) ^ ( glsb >> 1)) << 5);
            col[3] = (( col[3] & 0x7fe0 ) << 1 ) | ( col[3] & 0x1f ) |
                     (( glsb >> 1) << 5);
            lo = rgb565_888( col[2] );
            hi = rgb565_888( col[3] );
        }
        makePalette(lo, hi, alpha ? 3 : 4, fpal);
        for (i=16; i<32; i++) {
            int j;

            j = index[i];
            if ( alpha && ( j == 3 )) {
                output[i][0] = 
                output[i][1] = 
                output[i][2] = 
                output[i][3] = 0.0f;
            } else {
                output[i][0] = 255.0f;
                output[i][1] = fpal[j][0];
                output[i][2] = fpal[j][1];
                output[i][3] = fpal[j][2];
            }
        }
        break;

    case TCC_CHROMA:
        for (i=0; i<4; i++) {
            int rgb;

            rgb = rgb888( col[i] ); 
            fpal[i][0] = (float) RED(rgb); 
            fpal[i][1] = (float) GRN(rgb);
            fpal[i][2] = (float) BLU(rgb);
        }
        for (i=0; i<32; i++) {
            int j = index[i];
            output[i][0] = 255.0f;
            output[i][1] = fpal[j][0];
            output[i][2] = fpal[j][1];
            output[i][3] = fpal[j][2];
        }
         break;

    case TCC_ALPHA:
        if ( alpha ) { // interpolate colors
            float       fpal[4][4];
     
            lo = argb8888( col[0] );
            hi = argb8888( col[1] );
            makePaletteAlpha(lo, hi, 4, fpal);
            for (i=0; i<16; i++) {
                int j;
    
                j = index[i];
                output[i][0] = fpal[j][0];
                output[i][1] = fpal[j][1];
                output[i][2] = fpal[j][2];
                output[i][3] = fpal[j][3];
            }
    
            lo = argb8888( col[2] );
            hi = argb8888( col[1] );
            makePaletteAlpha(lo, hi, 4, fpal);
            for (i=16; i<32; i++) {
                int j;
    
                j = index[i];
                output[i][0] = fpal[j][0];
                output[i][1] = fpal[j][1];
                output[i][2] = fpal[j][2];
                output[i][3] = fpal[j][3];
            }
        } else { // no interpolation use colors as they are index 3 = transparent black
            FxU32 p[4];
            p[0] = argb8888( col[0] );
            p[1] = argb8888( col[1] );
            p[2] = argb8888( col[2] );
            p[3] = 0; // transparent black
            for (i=0; i<32; i++) {
                int j = index[i];

                output[i][0] = (float)ALF(p[j]);
                output[i][1] = (float)RED(p[j]);
                output[i][2] = (float)GRN(p[j]);
                output[i][3] = (float)BLU(p[j]);
            }

        }
        break;

    default:
#if PRINT
        fprintf(stderr, "NYI in decodeColors\n");
#endif
        exit(0);
    }

    if (a_color_cc == -1) return;                          // no color coding.
    if ((a_color_cc != 4) && (a_color_cc != mode)) return; // not this block

    // Do color coding.
    {
        float r, g, b;

        if (mode == TCC_HI) { 
            r = 255.0f; g = 255.0f; b = 0.0f;           // yellow
        } else if (mode == TCC_CHROMA) {
            r = 255.0f; g = 0.0f; b = 0.0f;             // red
        } else if (mode == TCC_ALPHA) {
            r = 255.0f; g = 0.0f; b = 255.0f;           // magenta
        } else {
            // mixed.
            i = 0;
            if ((col[0] >> 30) & 1) i++;
            if ((col[1] >> 30) & 1) i++;

            if (i == 0) { 
	        r = 0.0f; g = 0.0f; b = 255.0f; }       // blue
            else if (i == 1) { 
                r = 0.0f; g = 255.0f; b = 255.0f; }     // cyan
            else {
                r = 0.0f; g = 255.0f; b = 0.0f; }       // green
        }

        output[1*8+3][1] = r;
        output[1*8+3][2] = g;
        output[1*8+3][3] = b;
        output[2*8+4][1] = r;
        output[2*8+4][2] = g;
        output[2*8+4][3] = b;
    }
}

#define STATISTICS 0
#if STATISTICS
static int nvqChroma;
static int nvqChroma_outer[MAX_REPEAT+1];
static int nvqChroma_inner[50+1];
#endif

#define NCOLORS  4
#define VQCHROMA_ERR_TARGET 256.0f  // Greater == faster.
                                    // Set by gathering statistics from Q3.
#define MAX_REPEAT 10

static void
vqChroma(const float in[][3], int ncolors, float colors[][3])
{
    float   input[32][3];
    float   sums[NCOLORS][3];
    float   errors[NCOLORS];  // XXX never read!
    float   counts[NCOLORS];
    float   best[NCOLORS][3];
    float   besterr = 1.0e20f;  // infinity
    float   lasterr = 1.0e20f;

    float   oo8 = 1.0f/8.0f;
    float   err = 0.0f;
    int     i, j, k;
    int     repeat = MAX_REPEAT;        // iteration limit on outermost loop

#if STATISTICS
    nvqChroma++;
#endif

    if ( ncolors > NCOLORS ) // check we have enough space
        txError("FXT1 vqChroma: invalid number of colors\n");

    // Copy input colors, chopping down to 555
    // XXXdwm  ... but they're all floats!?!  Looks like the only effect of 
    //         this is on the magnitudes of the constants that are tested against.
    for (i=0; i<32; i++) {
        input[i][0] = in[i][0] * oo8;
        input[i][1] = in[i][1] * oo8;
        input[i][2] = in[i][2] * oo8;
    }

    // Select ncolors initial colors from a grid
    colors[0][0] = input[ 0][0];
    colors[0][1] = input[ 0][1];
    colors[0][2] = input[ 0][2];
    colors[1][0] = input[10][0];
    colors[1][1] = input[10][1];
    colors[1][2] = input[10][2];
    colors[2][0] = input[16][0];
    colors[2][1] = input[16][1];
    colors[2][2] = input[16][2];
    colors[3][0] = input[26][0];  /* wasted if ncolors < 4 */
    colors[3][1] = input[26][1];  /* wasted if ncolors < 4 */
    colors[3][2] = input[26][2];  /* wasted if ncolors < 4 */

again:

    // Here's the vector quantizer:
    for (k=0; k<50; k++) {

        // Find closest color, and track sums.
        for (i=0; i<ncolors; i++) {
            counts[i] = 0.0f;
            sums[i][0] = sums[i][1] = sums[i][2] = 0.0f;
            errors[i]    = 0.0f;
        }
        err = 0.0f;

        for (i=0; i<32; i++) {  // for each input point
            float   e;

            j = bestColorError(&input[i][0], (const float (*)[3])colors, ncolors, &e);
            counts[j] += 1.0f;
            sums[j][0] += (input[i][0]);
            sums[j][1] += (input[i][1]);
            sums[j][2] += (input[i][2]);

            err += e;
            errors[j] += e;
        }

        // Move each palette color to the barycenter of the set of input points that 
        // were closest to its previous location.
        for (j=0; j<ncolors; j++) {
            float   rc;

            rc = (counts[j] == 0.0f) ? 1.0f : 1/counts[j];
            // XXXdwm  Shouldn't colors[] be snapped to the 555 grid points?
            colors[j][0] = (sums[j][0] * rc);
            colors[j][1] = (sums[j][1] * rc);
            colors[j][2] = (sums[j][2] * rc);
        }

#if 0
        printf("It: %3d err = %f\n", k, err);
#endif

        if ((err < 1.0f) || (ABS(lasterr - err) < 1.0f))
            break;
        lasterr = err;
    }
#if STATISTICS
        nvqChroma_inner[k+1]++;
#endif

#if 0
    printf("Alt VQ results: rep: %d, (%8.2f) %s\n", repeat, err, err != 0.0f ? "Bad" : "");
    for (i=0; i<ncolors; i++) {
        printf("[%3.0f %3.0f %3.0f] %2d (Error: %8.2f)\n", colors[i][0], colors[i][1], colors[i][2], (int) counts[i], errors[i]);
    }
    printf("Repeat = %2d, err = %8.2f, besterr = %8.2g\n", repeat, err, besterr);

#endif
    /*
     * Find worst fitting color and replace any item in the palette.
     */
    if (err < besterr) {
        besterr = err; 
#if 0
        printf("%g,", besterr);
#endif
        memcpy( best, colors, ncolors * 3 * sizeof(float));
    } else {
    }

    if ((err < VQCHROMA_ERR_TARGET) || (--repeat <= 0))
        goto done;

    {
        float worsterr = -1.0f;
        int   worsti = 0;

        for (i=0; i<32; i++) {
            float   dr, dg, db;
            float   e;           /* distance according to the L-infinity metric */

            j = bestColor(&input[i][0], (const float (*)[3])colors, ncolors); /* distance according to the L-squared metric */
            dr = ABS( input[i][0] - colors[j][0] );
            dg = ABS( input[i][1] - colors[j][1] );
            db = ABS( input[i][2] - colors[j][2] );
            e = dr;
            if (dg > e) e = dg;
            if (db > e) e = db;
            if (e > worsterr) {
                worsterr = e;
                worsti   = i;
            }
        }

        /* If some palette entry is unused, use it; otherwise, gamble */
        /* XX What about an entry that has very few users, e.g. only one?  */
        for (i=0; i<ncolors; i++) 
            if (counts[i] == 0.0f) break;

        if (i >= ncolors) i = rand() % ncolors;

#if 0
        printf("Repeat %d: repl %d [%3.0f %3.0f %3.0f] with [%3.0f %3.0f %3.0f]\n",
            repeat, colors[i][0], colors[i][1], colors[i][2],
            input[worsti][0], input[worsti][1], input[worsti][2]);
#endif

        /* Replace palette entry, and retry. */
        // XXXdwm  Shouldn't colors[] be snapped to the 555 grid points?
        colors[i][0] = input[worsti][0];
        colors[i][1] = input[worsti][1];
        colors[i][2] = input[worsti][2];
    }
    goto again;

done:
#if STATISTICS
    nvqChroma_outer[ MAX_REPEAT - repeat]++;
#endif

#if 0
    printf("\n");
#endif
    /* Scale colors back to 888 */
    for (i=0; i<ncolors; i++) {
        colors[i][0] = best[i][0] * 8.0f;
        colors[i][1] = best[i][1] * 8.0f;
        colors[i][2] = best[i][2] * 8.0f;
    }
}

static void
vqChromaAlpha(const float in[][3], FxI32 ain[], int ncolors, float colors[][4], FxU32 lerp)
{
    float   input[32][4]; // make alpha 4th comp to minimize code delta
    float   deltas[NCOLORS][4];
    float   errors[NCOLORS];    // XXX never read
    float   counts[NCOLORS];
    float   best[NCOLORS][4];
    float   besterr = 1.0e20f;  // infinity
    float   lasterr = 1.0e20f;

    float   alpha = 1.0f;       // XX no other writes to this !?
    float   oo8 = 1.0f/8.0f;
    float   err = 0.0f;
    int     i, j, k;
    int     repeat = 10;

    if ( ncolors > NCOLORS ) // check we have enough space
        txError("FXT1 vqChromaAlpha: invalid number of colors\n");

    // Copy input colors, chopping down to 555
    for (i=0; i<32; i++) {
        input[i][0] = in[i][0] * oo8;
        input[i][1] = in[i][1] * oo8;
        input[i][2] = in[i][2] * oo8;
        input[i][3] = ain[i] * oo8;
    }

    // Select ncolors initial colors from a grid
    colors[0][0] = input[ 0][0];
    colors[0][1] = input[ 0][1];
    colors[0][2] = input[ 0][2];
    colors[0][3] = input[ 0][3];
    colors[1][0] = input[10][0];
    colors[1][1] = input[10][1];
    colors[1][2] = input[10][2];
    colors[1][3] = input[10][3];
    colors[2][0] = input[16][0];
    colors[2][1] = input[16][1];
    colors[2][2] = input[16][2];
    colors[2][3] = input[16][3];
    colors[3][0] = input[26][0];  /* wasted if ncolors == 3 */
    colors[3][1] = input[26][1];  /* wasted if ncolors == 3 */
    colors[3][2] = input[26][2];  /* wasted if ncolors == 3 */
    colors[3][3] = input[26][3];  /* wasted if ncolors == 3 */

again:
    // Here's the vector quantizer:
    for (k=0; k<50; k++) {

        // Find closest color, and track deltas.
        for (i=0; i<ncolors; i++) {
            counts[i] = 0.0f;
            deltas[i][0] = deltas[i][1] = deltas[i][2] = deltas[i][3] = 0.0f;
            errors[i]    = 0.0f;
        }
        err = 0.0f;

        for (i=0; i<32; i++) {
            float   e0, e1, e2, e;

            j = bestColorAlpha(&input[i][0], input[i][3], (const float (*)[4])colors, ncolors, lerp);
            if ( !lerp && ( j == 3 )) continue; // transparent black handled specially
            counts[j] += 1.0f;
            deltas[j][0] += (input[i][0] - colors[j][0]) * alpha;
            deltas[j][1] += (input[i][1] - colors[j][1]) * alpha;
            deltas[j][2] += (input[i][2] - colors[j][2]) * alpha;
            deltas[j][3] += (input[i][3] - colors[j][3]) * alpha;

            if ( a_lerp ) {
                e0 = SQUARED(colors[j][0]*colors[j][3] - input[i][0]*input[i][3]);
                e1 = SQUARED(colors[j][1]*colors[j][3] - input[i][1]*input[i][3]);
                e2 = SQUARED(colors[j][2]*colors[j][3] - input[i][2]*input[i][3]);

                e0 = SQUARED(colors[j][0] - input[i][0]);
                e1 = SQUARED(colors[j][1] - input[i][1]);
                e2 = SQUARED(colors[j][2] - input[i][2]);

                e = ( SQUARED(colors[j][0]*colors[j][3] - input[i][0]*input[i][3]) +
                      SQUARED(colors[j][1]*colors[j][3] - input[i][1]*input[i][3]) +
                      SQUARED(colors[j][2]*colors[j][3] - input[i][2]*input[i][3]) );
                e = e0 + e1 + e2;                // XXXdwm  This overwrites the result of the previous line!?
            } else {
                e = ( SQUARED(colors[j][0] - input[i][0]) +
                      SQUARED(colors[j][1] - input[i][1]) +
                      SQUARED(colors[j][2] - input[i][2]) +
                      SQUARED(colors[j][3] - input[i][3]) );
            }
            err += e;
            errors[j] += e;
        }

        // Update colors.
        for (i=0; i<ncolors; i++) {
            float   c;

            c = (counts[i] == 0.0f) ? 1.0f : counts[i];
            colors[i][0] += (deltas[i][0] / c);
            colors[i][1] += (deltas[i][1] / c);
            colors[i][2] += (deltas[i][2] / c);
            colors[i][3] += (deltas[i][3] / c);
        }

#if 0
        printf("It: %3d err = %f\n", k, err);
#endif

        if ((err < 1.0f) || (ABS(lasterr - err) < 1.0f)) 
            break;
        lasterr = err;
    }

#if 0
    printf("Alt VQ results: rep: %d, (%8.2f) %s\n", repeat, err, err != 0.0f ? "Bad" : "");
    for (i=0; i<ncolors; i++) {
        printf("[%3.0f %3.0f %3.0f %3.0f] %2d (Error: %8.2f)\n", 
               colors[i][0], colors[i][1], colors[i][2], colors[i][3], (int) counts[i], errors[i]);
    }
    printf("Repeat = %2d, err = %8.2f, besterr = %8.2g\n", repeat, err, besterr);

#endif
    /*
     * Find worst fitting color and replace any item in the palette.
     * in palette 
     */
    if (err < besterr) {
        besterr = err; 
#if 0
        printf("%g,", besterr);
#endif
        memcpy( best, colors, ncolors * 4 * sizeof(float));
    } else {
    }

    if ((err < VQCHROMA_ERR_TARGET) || (--repeat <= 0)) 
        goto done;

    {
        float worsterr = -1.0f;
        int   worsti = 0;

        for (i=0; i<32; i++) {
            float   dr, dg, db, da, e;

            j = bestColorAlpha(&input[i][0], input[i][3], (const float (*)[4])colors, ncolors, lerp);
            if ( !lerp && ( j == 3 )) continue;
            dr = ABS( input[i][0] - colors[j][0] );
            dg = ABS( input[i][1] - colors[j][1] );
            db = ABS( input[i][2] - colors[j][2] );
            da = ABS( input[i][3] - colors[j][3] );
            e = dr;
            if (dg > e) e = dg;
            if (db > e) e = db;
            if (da > e) e = da;
            if (e > worsterr) {
                worsterr = e;
                worsti   = i;
            }
        }

        /* If some palette entry is unused, use it; otherwise, gamble */
        for (i=0; i<ncolors; i++) 
            if (counts[i] == 0.0f) break;

        if (i >= ncolors) i = rand() % ncolors;

#if 0
        printf("Repeat %d: repl %d [%3.0f %3.0f %3.0f] with [%3.0f %3.0f %3.0f]\n",
            repeat, colors[i][0], colors[i][1], colors[i][2], colors[i][3],
            input[worsti][0], input[worsti][1], input[worsti][2], input[worsti][3]);
#endif

        /* Replace palette entry, and retry. */
        colors[i][0] = input[worsti][0];
        colors[i][1] = input[worsti][1];
        colors[i][2] = input[worsti][2];
        colors[i][3] = input[worsti][3];
    }
    goto again;

done:

#if 0
    printf("\n");
#endif
    /* Scale colors back to 888 */
    for (i=0; i<ncolors; i++) {
        colors[i][0] = best[i][0] * 8.0f;
        colors[i][1] = best[i][1] * 8.0f;
        colors[i][2] = best[i][2] * 8.0f;
        colors[i][3] = best[i][3] * 8.0f;
    }
}

static int _cc_hi = 0;
static int _cc_mixed_3 = 0;
static int _cc_mixed_12 = 0;
static int _cc_mixed_0 = 0;
static int _cc_chroma = 0;
static int _cc_alpha = 0;

static void
encodeAlpha( float input[][3], FxI32  ainput[], void *bits, FxU32 lerp)
{
    FxU32   lo, hi, p[3], icol[3];
    float   col[4][4];
    float   fpal[4][4];
    int     i, index[32];

    vqChromaAlpha( (const float (*)[3])input, ainput, 3, col, lerp);

    if ( lerp ) {
        /* Deal with even block */
        lo = ARGB( (int)col[0][3], (int) col[0][0], (int) col[0][1], (int) col[0][2]);
        hi = ARGB( (int)col[1][3], (int) col[1][0], (int) col[1][1], (int) col[1][2]);
        makePaletteAlpha( lo, hi, 4, fpal);
        icol[0] = argb5555( lo );
        icol[1] = argb5555( hi );

        /* Map input colors to closest entry in the palette */
        for (i=0; i<16; i++) {
            index[i] = bestColorAlpha(&input[i][0], (float)ainput[i], (const float (*)[4])fpal, 4, lerp);
        }

        /* Now deal with odd block */
        lo = ARGB( (int)col[2][3], (int) col[2][0], (int) col[2][1], (int) col[2][2]);
        hi = ARGB( (int)col[1][3], (int) col[1][0], (int) col[1][1], (int) col[1][2]);
        makePaletteAlpha( lo, hi, 4, fpal);
        icol[2] = argb5555( hi );

        /* Map input colors to closest entry in the palette */
        for (i=16; i<32; i++) {
            index[i] = bestColorAlpha(&input[i][0], (float)ainput[i], (const float (*)[4])fpal, 4, lerp);
        }
    } else { // no interpolation
        p[0] = ARGB( (int)col[0][3], (int) col[0][0], (int) col[0][1], (int) col[0][2]);
        p[1] = ARGB( (int)col[1][3], (int) col[1][0], (int) col[1][1], (int) col[1][2]);
        p[2] = ARGB( (int)col[2][3], (int) col[2][0], (int) col[2][1], (int) col[2][2]);
        icol[0] = argb5555( p[0] );
        icol[1] = argb5555( p[1] );
        icol[2] = argb5555( p[2] );

        /* Map input colors to closest entry in the palette */
        for (i=0; i<32; i++) {
            index[i] = bestColorAlpha(&input[i][0], (float)ainput[i], (const float (*)[4])col, 3, lerp);
        }
    } 

    /* Now encode these into the 128 bits */
    bitEncoder( TCC_ALPHA, icol, lerp, index, bits);
    _cc_alpha++;
}

#if STATISTICS
static int h_nunique[32];
#endif

static void
quantize4bpp_block(float input[][3], FxI32  ainput[], void *bits)
{
    float   Evalues[3];                             // even  block Eigen values
    float   Ovalues[3];                             // odd   block Eigen values
    float   Wvalues[3];                             // whole block Eigen values

    float   Eavg[3], Emin[3], Emax[3], Eerr[3];     // even  block
    float   Oavg[3], Omin[3], Omax[3], Oerr[3];     // odd   block
    float   Wavg[3], Wmin[3], Wmax[3], Werr[3];     // whole block

    float   Eflo[3][3], Efhi[3][3];                 // even  block
    float   Oflo[3][3], Ofhi[3][3];                 // odd   block
    float   Wflo[3][3], Wfhi[3][3];                 // whole block

    float   output[32][3];
    float   col[4][3];
    int     submode = 0;
    int     i, alpha = 0;

#define ASSUME_ALPHA_EQUALS_ONE 0  // XX Set this to work around an apparent bug in PaintShop
#if ASSUME_ALPHA_EQUALS_ONE
#else
    // determine alpha properties: 
    //     alpha == 0 => opaque, 
    //     alpha == 1 => bimodal (opaque or transp)
    //     alpha == 2 => partially transparent
    for (i=0; i<32; i++) {
        // if alpha differs from 0 or 255 within tolerance it can still use non-alpha blocks.
        if ( ainput[i] >= ( 255 - a_tolerance ) )
            ainput[i] = 255;

        if ( ainput[i] <=  a_tolerance )
            ainput[i] = 0;

        if ( ainput[i] == 0 )  // XXXdwm  Also condition on: "&& alpha != 2" ?
            alpha = 1;
        else if ( ainput[i] != 255 )
            alpha = 2;
    }
#endif

    // whole block statistics
    eigenStatistics(32, (const float (*)[3])input, Wvalues, output, Wflo, Wfhi, Wavg /*not used*/, Wmin, Wmax, Werr);

#if PRINT
    fprintf(stderr, "NEW TILE----------------------(%4d %4d)\n", globalX, globalY);
    printStatistics(32, input, output, Wflo, Wfhi, Wavg, Wmin, Wmax, Werr, "Whole Block\n");
    printStatistics(16, NULL, NULL  , Eflo, Efhi, Eavg, Emin, Emax, Eerr, "Even  Block\n");
    printStatistics(16, NULL, NULL  , Oflo, Ofhi, Oavg, Omin, Omax, Oerr, "Odd   Block\n");
#endif

    if (a_force_cc != -1) {
        // int loEven, loOdd;

        switch (a_force_cc) {
        case TCC_HI:
            encodeColors( TCC_HI, 0, alpha,
               &Wflo[0][0], &Wfhi[0][0], NULL, NULL, input, ainput, bits);
            _cc_hi++;
            return;

        case TCC_MIXED:
            submode = 0;
            // Even, odd block statistics
            eigenStatistics(16, (const float(*)[3]) &input[ 0][0], Evalues, output, 
                  Eflo, Efhi, Eavg /*not used*/, Emin, Emax, Eerr /*not used*/);
            eigenStatistics(16, (const float(*)[3]) &input[16][0], Ovalues, output, 
                  Oflo, Ofhi, Oavg /*not used*/, Omin, Omax, Oerr /*not used*/);

            encodeColors( TCC_MIXED, submode, alpha,
              &Eflo[0][0], &Efhi[0][0], &Oflo[0][0], &Ofhi[0][0], input, ainput, bits);
            _cc_mixed_0++;
            return;

        case TCC_CHROMA:
            vqChroma( (const float (*)[3])input, alpha ? 3 : 4, col);
            encodeColors( TCC_CHROMA, 0, 0,
                &col[0][0], &col[1][0], &col[2][0], &col[3][0], input, ainput, bits);
            _cc_chroma++;
            return;

        case TCC_ALPHA:
            encodeAlpha( input, ainput, bits, a_lerp );
        }
        return;
    }

    if (( alpha == 2 ) || ((alpha == 1 ) && (Werr[1] >= 20))) { 
        // strong alpha component or strong color component with alpha use TCC_ALPHA
        // LOOOK need to determine whether to interpolate or not
        encodeAlpha( input, ainput, bits, a_lerp);
        return;
    }

    // Even, odd block statistics
    eigenStatistics(16, (const float(*)[3]) &input[ 0][0], Evalues, output, 
    	Eflo, Efhi, Eavg /*not used*/, Emin, Emax, Eerr /*not used*/);
    eigenStatistics(16, (const float(*)[3]) &input[16][0], Ovalues, output, 
    	Oflo, Ofhi, Oavg /*not used*/, Omin, Omax, Oerr /*not used*/);

    // If color distribution is not "sufficiently" oblong, go CHROMA
    // XXX Dither if something like the following condition is satisfied?
    //         (Werr[0] < 4*4)   // colors in a chroma block * best case color resolution (green)
    // Note that left and right halves might be each separately oblong, but not the whole.
    if ( fabs(Evalues[1])+fabs(Evalues[2]) > 8 ||
         fabs(Ovalues[1])+fabs(Evalues[2]) > 8)
    {

#if STATISTICS
    	{
    	    int nunique = 0;
    	    int iu, ii;
    	    float unique[32][3];
    	    int h_unique[32];
    
    	    for (ii=0; ii<32; ii++)
    	    {
    		for (iu=0; iu<nunique; iu++)
    		{
    		    if ( input[ii][0] == unique[iu][0] &&
    			 input[ii][1] == unique[iu][1] &&
    			 input[ii][2] == unique[iu][2])
    		    {
    			h_unique[iu]++;
    			goto next_input;
    		    }
    		}
    		unique[nunique][0] = input[ii][0];
    		unique[nunique][1] = input[ii][1];
    		unique[nunique][2] = input[ii][2];
    		nunique++;
    	      next_input:
                ;
    	    }
    
    	    h_nunique[nunique]++;
    	}    
#endif

        vqChroma( (const float (*)[3])input, alpha ? 3 : 4, col);
        encodeColors( TCC_CHROMA, 0, alpha,
            &col[0][0], &col[1][0], &col[2][0], &col[3][0], input, ainput, bits);
        _cc_chroma++;
        return;
    }

    {
#ifdef notdef
        // commented out to get rid of error on VC++ 6.0
        // int loEven, loOdd;
#endif

        int skewed;

        /* 
         * No chrominance, only intensity changes in this block, so we
         * ignore the 2nd eigenvector 
         *
         * Now we decide between coding at 7 levels, or 4 levels
         */
        // XXX dwm 'min' and 'max' are in eigen space; not sure what this expression means.
        //         Also, it's correct only if all 'min's and 'max's are positive.
        skewed = (ABS(ABS(Wmin[0]) - ABS(Wmax[0])) > 32) ||
                 (ABS(ABS(Emin[0]) - ABS(Emax[0])) > 32) ||
                 (ABS(ABS(Omin[0]) - ABS(Omax[0])) > 32) ;

#ifdef notdef
        loEven = (ABS(Eflo[0][0] - Efhi[0][0]) < 15) &&
                 (ABS(Eflo[0][1] - Efhi[0][1]) < 15) &&
                 (ABS(Eflo[0][2] - Efhi[0][2]) < 15) ;


        loOdd  = (ABS(Oflo[0][0] - Ofhi[0][0]) < 15) &&
                 (ABS(Oflo[0][1] - Ofhi[0][1]) < 15) &&
                 (ABS(Oflo[0][2] - Ofhi[0][2]) < 15) ;

        if (loEven && loOdd) {
            // Both halves have small variations only, split to 4 levels each
            encodeColors( TCC_MIXED, 3, alpha,
               &Eflo[0][0], &Efhi[0][0], &Oflo[0][0], &Ofhi[0][0], input, ainput, bits);
            _cc_mixed_3++;
            return;
        }

        if ((loEven || loOdd)){
            submode = 0;
            if (loEven) submode |= 1;
            if (loOdd ) submode |= 2;
            encodeColors( TCC_MIXED, submode, alpha,
               &Eflo[0][0], &Efhi[0][0], &Oflo[0][0], &Ofhi[0][0], input, ainput, bits);
            _cc_mixed_12++;
            return;
        }
#endif

#define DWM 1
#if DWM // Distribution is known to be oblong at this point due to failure
        // of CHROMA test above.  
//	  // Now determine whether range along major color axis is compact relative to 
//	  // total space.  This will mean that the distribution cannot
//	  // be severely 'grouped' left versus right in the block, which 
//	  // causes TCC_HI to produce bad results.
//	  // 7 is number of levels in TCC_HI; 4 is best case truncation error (green)
//	 (Werr[0] <= 7*4)    // in color units
//        if ( Werr[0] <= 7*4 &&
//             (fabs(Wvalues[1])+fabs(Wvalues[2])) < 500)
        if ( 0)
#else
        // Neither half has small variations, split whole block into 8 levels
        if (skewed)
#endif
        {
            encodeColors( TCC_HI, 0, alpha,
               &Wflo[0][0], &Wfhi[0][0], NULL, NULL, input, ainput, bits);
            _cc_hi++;
            return;
        } else {
            encodeColors( TCC_MIXED, 0, alpha,
               &Eflo[0][0], &Efhi[0][0], &Oflo[0][0], &Ofhi[0][0], input, ainput, bits);
            _cc_mixed_0++;
            return;
        }
    }
}

static void
encode4bpp_block(
    int *pp0, 
    int *pp1, 
    int *pp2, 
    int *pp3, 
    int *code)
{
    float   input[32][3];
    FxI32   ainput[32];
    int     i;

    /* This maps to [0.5, 255.5], consistent with mappings elsewhere. 
     * To understand why this mapping is best, consider that arithmetic means of these
     * values will be taken later.  The mean of two adjacent values should lie exactly
     * on the boundary between them implied by truncations to lower precision.
     */
    /* Convert input to input vectors */
    for (i=0; i<4; i++) {
        // 1st block of 4x4
        ainput[ 0 + i] = ALF(pp0[i]);
        input[ 0 + i][0] = (float) (RED(pp0[i])) + 0.5f;
        input[ 0 + i][1] = (float) (GRN(pp0[i])) + 0.5f;
        input[ 0 + i][2] = (float) (BLU(pp0[i])) + 0.5f;

        ainput[ 4 + i] = ALF(pp1[i]);
        input[ 4 + i][0] = (float) (RED(pp1[i])) + 0.5f;
        input[ 4 + i][1] = (float) (GRN(pp1[i])) + 0.5f;
        input[ 4 + i][2] = (float) (BLU(pp1[i])) + 0.5f;

        ainput[ 8 + i] = ALF(pp2[i]);
        input[ 8 + i][0] = (float) (RED(pp2[i])) + 0.5f;
        input[ 8 + i][1] = (float) (GRN(pp2[i])) + 0.5f;
        input[ 8 + i][2] = (float) (BLU(pp2[i])) + 0.5f;

        ainput[12 + i] = ALF(pp3[i]);
        input[12 + i][0] = (float) (RED(pp3[i])) + 0.5f;
        input[12 + i][1] = (float) (GRN(pp3[i])) + 0.5f;
        input[12 + i][2] = (float) (BLU(pp3[i])) + 0.5f;

        // 2nd block of 4x4
        ainput[16 + i] = ALF(pp0[i+4]);
        input[16 + i][0] = (float) (RED(pp0[i+4])) + 0.5f;
        input[16 + i][1] = (float) (GRN(pp0[i+4])) + 0.5f;
        input[16 + i][2] = (float) (BLU(pp0[i+4])) + 0.5f;

        ainput[20 + i] = ALF(pp1[i+4]);
        input[20 + i][0] = (float) (RED(pp1[i+4])) + 0.5f;
        input[20 + i][1] = (float) (GRN(pp1[i+4])) + 0.5f;
        input[20 + i][2] = (float) (BLU(pp1[i+4])) + 0.5f;

        ainput[24 + i] = ALF(pp2[i+4]);
        input[24 + i][0] = (float) (RED(pp2[i+4])) + 0.5f;
        input[24 + i][1] = (float) (GRN(pp2[i+4])) + 0.5f;
        input[24 + i][2] = (float) (BLU(pp2[i+4])) + 0.5f;

        ainput[28 + i] = ALF(pp3[i+4]);
        input[28 + i][0] = (float) (RED(pp3[i+4])) + 0.5f;
        input[28 + i][1] = (float) (GRN(pp3[i+4])) + 0.5f;
        input[28 + i][2] = (float) (BLU(pp3[i+4])) + 0.5f;
    }

    quantize4bpp_block(input, ainput, code);
}

#define FARGB(a, r, g, b)    (ARGB( (int) a, (int) r, (int) g, (int) b) )

static void
decode4bpp_block(
    int *code,
    int *pp0, 
    int *pp1, 
    int *pp2, 
    int *pp3)
{
    float       output[32][4];  // order AGBR
    int         i;

    decodeColors(code, output);

    // Decode and put it back into source array right away!
    for (i=0; i<4; i++) {
        pp0[i+0] = FARGB(output[ 0 + i][0], output[ 0+i][1], output[ 0+i][2], output[ 0+i][3]);
        pp1[i+0] = FARGB(output[ 4 + i][0], output[ 4+i][1], output[ 4+i][2], output[ 4+i][3]);
        pp2[i+0] = FARGB(output[ 8 + i][0], output[ 8+i][1], output[ 8+i][2], output[ 8+i][3]);
        pp3[i+0] = FARGB(output[12 + i][0], output[12+i][1], output[12+i][2], output[12+i][3]);

        pp0[i+4] = FARGB(output[16 + i][0], output[16+i][1], output[16+i][2], output[16+i][3]);
        pp1[i+4] = FARGB(output[20 + i][0], output[20+i][1], output[20+i][2], output[20+i][3]);
        pp2[i+4] = FARGB(output[24 + i][0], output[24+i][1], output[24+i][2], output[24+i][3]);
        pp3[i+4] = FARGB(output[28 + i][0], output[28+i][1], output[28+i][2], output[28+i][3]);
    }
}


void
sst2FXT1Encode4bpp(int *data, int width, int height, int* encoded)
{
    int x, y ;

    for (y=0; y < height; y += 4) {
        for (x=0; x < width; x += 8) {
            globalX = x;
            globalY = y;
            encode4bpp_block(
                &data[x + (y + 0) * width],
                &data[x + (y + 1) * width],
                &data[x + (y + 2) * width],
                &data[x + (y + 3) * width],
                encoded);
            encoded += 4;       // 128 bits per 8x4 block = 4bpp
        }
    }
    // fprintf(stderr, "%d alpha, %d chroma, %d mixed3, %d mixed12, %d mixed0, %d hi\n",
    //         _cc_alpha, _cc_chroma, _cc_mixed_3, _cc_mixed_12, _cc_mixed_0, _cc_hi);
}

void
sst2FXT1Decode4bpp(int *encoded, int width, int height, int *data)
{
    int x, y ;

    for (y=0; y < height; y += 4) {
        for (x=0; x < width; x += 8) {
            globalX = x;
            globalY = y;
            decode4bpp_block(
                encoded,
                &data[x + (y + 0) * width],
                &data[x + (y + 1) * width],
                &data[x + (y + 2) * width],
                &data[x + (y + 3) * width] );
            encoded += 4;       // 128 bits per 8x4 block = 4bpp
        }
    }
}
