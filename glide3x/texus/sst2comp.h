#include <stdio.h>
#include <assert.h>
#include <3dfx.h>

typedef unsigned _int64 FxU64;

#define ALF(x)          (((x) >> 24) & 0xff)
#define RED(x)          (((x) >> 16) & 0xff)
#define GRN(x)          (((x) >>  8) & 0xff)
#define BLU(x)          (((x)      ) & 0xff)
#define ARGB(a, r, g, b) ((((a) << 24) & 0xff) | ((r) << 16) | ((g) << 8) |((b)))
#define RGB(r, g, b)    (ARGB( (int) 255, (int) r, (int) g, (int) b) )

#define TCC_HI          0
#define TCC_MIXED       1
#define TCC_CHROMA      2
#define TCC_ALPHA       3


/* in eigen.c */
extern void eigenProject(int n, float *idata, float m[3][3], float *odata);
extern void eigenSpace(int n, float *data, float evec[3][3], float eval[3]);
extern void eigenStatistics(
    int     n, 
    const   float input[][3], 

    float   evalues[3],
    float   output[][3],        // after transformation to eigenspace
    float   lo[3][3],           // lo and hi colors for each evector
    float   hi[3][3],

    float   avg[3],             // this is the mean value of this block
    float   min[3],             // min and max values for each evector
    float   max[3], 
    float   err[3]              // max error incurred when dropping each evector
    );

void
printStatistics(
    int     n, 
    const   float input[][3], 

    float   output[][3],        // after transformation to eigenspace
    float   lo[3][3],           // lo and hi colors for each evector
    float   hi[3][3],

    float   avg[3],             // this is the mean value of this block
    float   min[3],             // min and max values for each evector
    float   max[3], 
    float   err[3],             // max error incurred when dropping each evector
    char    *title 
    );

#define PRINT       0

/* In bitcoder.c */
int bitDecoder( void *bits128, FxU32 c[4], int index[32] );
void bitEncoder( int mode, FxU32 c[4], int index[32], void *bits128 );
