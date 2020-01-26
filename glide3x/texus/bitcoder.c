#include "texusint.h"
#include "sst2fxt1.h"
#include "fx64.h"

#define REMOVE(d, b, n) { d = (FxU32) (b & ((1 << (n)) - 1)); b >>= n; }
#define INSERT(b, d, n) { b <<= n; b |= ((d) & ((1 << (n)) - 1)); }

/* 
 * The following 4 procedures read and write 2 or 3 bit indices into 
 * the bit bucket 
 */
static void
encode2(int *index, FxU32 bits[4] )
{
    FxU32 lo = 0, hi = 0; 
    int   i;

    for (i=15; i >= 0; i--) {
        lo = (lo << 2) | (index[i     ] & 3);
        hi = (hi << 2) | (index[i + 16] & 3);
    }
    bits[0] = lo;
    bits[1] = hi;
}

static void
encode3(int *index, FxU32 bits[4] )
{
    FxU64 lo = 0, hi = 0;
    int   i;

    for (i=15; i >= 0; i--) {
        lo = (lo << 3) | (index[i     ] & 7);
        hi = (hi << 3) | (index[i + 16] & 7);
    }
    bits[0] = (FxU32) lo;
    bits[1] = (FxU32) ((lo >> 32) | (hi << 16));
    bits[2] = (FxU32) (hi >> 16);
}

static void
decode2(FxU32 bits[4], int *index)
{
    FxU32 lo, hi;
    int   i;

    lo = bits[0];
    hi = bits[1];

    for (i=0; i<16; i++) {
        index[i +  0] = lo & 3;  lo >>= 2;
        index[i + 16] = hi & 3;  hi >>= 2;
    }
}

static void
decode3(FxU32 bits[4], int *index)
{
    FxU64       lo, hi;
    int         i;

    lo = bits[0] | (((FxU64) bits[1]) << 32);
    hi = (bits[1] >> 16) | (((FxU64) bits[2]) << 16);

    for (i=0; i<16; i++) {
        index[i +  0] = (int) (lo & 7); lo >>= 3;
        index[i + 16] = (int) (hi & 7); hi >>= 3;
    }
}

/* decode variable length mode field */

static int
getMode(FxU32 bits3)
{
    int mode;
    
    bits3 >>= 29;

    if ( bits3 & 4 ) { // 1XX
        mode = TCC_MIXED;
    } else if (( bits3 & 6 ) == 0 ) { // 00X
        mode = TCC_HI;
    } else {
        switch ( bits3 ) {
        case TCC_CHROMA:
        case TCC_ALPHA:
            mode = bits3;
            break;
        default:
            txError("FXT1 bad mode\n");
            mode = -1;
            break;
        }
    }
    return mode;
}

int
bitDecoder( void *bits128, FxU32 c[4], int index[32], FxU32 *alpha )
{
    FxU32       *bits = (FxU32 *) bits128;
    int         mode = getMode(bits[3]);
    FxU64       d;

    d = bits[3];
    d = (d << 32) | bits[2];

    switch(mode & 3) {
    case TCC_HI:
        {
           d >>= 32;
           REMOVE(c[0], d, 15);
           REMOVE(c[1], d, 15);
           c[2] = 0;
           c[3] = 0;
           decode3( bits, index);
           *alpha = 0;
        }
        break;

    case TCC_MIXED:
        {
            REMOVE(c[0], d, 15);
            REMOVE(c[1], d, 15);
            REMOVE(c[2], d, 15);
            REMOVE(c[3], d, 15);
            REMOVE(*alpha, d, 3);
            decode2( bits, index);
        }
        break;

    case TCC_CHROMA:
        {
            REMOVE(c[0], d, 15);
            REMOVE(c[1], d, 15);
            REMOVE(c[2], d, 15);
            REMOVE(c[3], d, 15);
            *alpha = 0;
            decode2( bits, index);
        }
        break;

    case TCC_ALPHA:
        {
            FxU32 a;
            REMOVE(c[0], d, 15);
            REMOVE(c[1], d, 15);
            REMOVE(c[2], d, 15);
            REMOVE(a, d, 5); c[0] |= a << 15;
            REMOVE(a, d, 5); c[1] |= a << 15;
            REMOVE(a, d, 5); c[2] |= a << 15;
            c[3] = 0;
            REMOVE(*alpha, d, 1);      // interpolate
            decode2( bits, index);
        }
        break;
    }
    return mode;
}

void
bitEncoder( int mode, FxU32 c[4], FxU32 alpha, int index[32], void *bits128 )
{
    FxU32 *bits = (FxU32 *) bits128;
    FxU64 d = 0;

    // mode has variable length encoding to fit alpha bits
    switch(mode & 3) {
    case TCC_HI:
        {
            encode3( index, bits);
            INSERT( d, mode, 2);
            INSERT( d, c[1], 15);
            INSERT( d, c[0], 15);
            bits[3] = (FxU32) d;
        }
        break;
    case TCC_MIXED:
        {
            encode2( index, bits);
            INSERT(d, mode, 1);
            INSERT(d, alpha, 3);
            INSERT(d, c[3], 15);
            INSERT(d, c[2], 15);
            INSERT(d, c[1], 15);
            INSERT(d, c[0], 15);
            bits[2] = (FxU32) d;
            bits[3] = (FxU32) (d >> 32);
        }
        break;
    case TCC_CHROMA:
        {
            encode2( index, bits);
            INSERT(d, mode, 3);
            INSERT(d, 0, 1);             // pad
            INSERT(d, c[3], 15);
            INSERT(d, c[2], 15);
            INSERT(d, c[1], 15);
            INSERT(d, c[0], 15);
            bits[2] = (FxU32) d;
            bits[3] = (FxU32) (d >> 32);
        }
        break;
    case TCC_ALPHA:
        {
            encode2( index, bits);
            INSERT(d, mode, 3);
            INSERT(d, alpha, 1);         // interpolate
            INSERT(d, (c[2]>>15), 5);
            INSERT(d, (c[1]>>15), 5);
            INSERT(d, (c[0]>>15), 5);
            INSERT(d, (c[2]&0x7fff), 15);
            INSERT(d, (c[1]&0x7fff), 15);
            INSERT(d, (c[0]&0x7fff), 15);
            bits[2] = (FxU32) d;
            bits[3] = (FxU32) (d >> 32);
        }
        break;
    }
}

#if 0
/* For testing low level encoding */
int
rand32() { return (rand() << 16) | (rand() & 0xffff); }

main()
{
    FxU32   imode, icol[4], iindex[32], bits[4];
    FxU32   omode, ocol[4], oindex[32];
    int     n, i, mask, passed;
    int     stats[4] = {0};

    for (n=0; n<1000000; n++) {
        imode = rand() & 3;

        switch(imode) {
        case 0:
            icol[0] = rand32() & 0x7fff;
            icol[1] = rand32() & 0x7fff;
            icol[2] = 0;
            icol[3] = 0;
            mask = 7;
            break;

        case 1:
            icol[0] = rand32() & 0x7fffffff;
            icol[1] = rand32() & 0x7fffffff;
            icol[2] = 0;
            icol[3] = 0;
            mask = 3;
            break;

        case 2:
            icol[0] = rand32() & 0x7fff;
            icol[1] = rand32() & 0x7fff;
            icol[2] = rand32() & 0x7fff;
            icol[3] = rand32() & 0x7fff;
            mask = 3;
            break;

        case 3:
            icol[0] = rand32() & 0xfffff;
            icol[1] = rand32() & 0xfffff;
            icol[2] = rand32() & 0xfffff;
            icol[3] = 0;
            mask = 3;
            break;
        }
        stats[imode]++;

        for (i=0; i<32; i++) iindex[i] = rand() & mask;

        /* encode this block, decode */
        encoder( imode, icol, iindex, bits);
        omode = decoder( bits, ocol, oindex);

        /* verify */
        passed = 1;
        for (i=0; i<4; i++) passed &= (icol[i] == ocol[i]);
        if (!passed) printf("Failed colors\n");
        for (i=0; i<32; i++) passed &= (iindex[i] == oindex[i]);
        if (!passed) printf("Failed index\n");
        passed &= (imode == omode);
        if (!passed) printf("Failed mode\n");

        if (!passed) {
            for (i=0; i<32; i++) printf("%d", iindex[i]); printf("\n");
            for (i=0; i<32; i++) printf("%d", oindex[i]); printf("\n");
            for (i=0; i< 4; i++) printf("%.08x ", icol[i]); printf("\n");
            for (i=0; i< 4; i++) printf("%.08x ", ocol[i]); printf("\n");
            printf("imode = %d, omode = %d\n", imode, omode);
            exit(0);
        }

        if ((n % 100000) == 0) printf("%8d %8d %8d %8d %8d\n", n, 
            stats[0], stats[1], stats[2], stats[3]);
    }
}
#endif

