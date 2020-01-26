#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define swapValue(K, i, j)  t=K[i], K[i] = K[j], K[j] = (float) t
#define swapVector(U, i, j) swapValue(U[0], i, j); swapValue(U[1], i, j); swapValue(U[2], i, j)

#if 0 /* not used */
static void 
printMatrix(const char *title, const float m[3][3])
{
    int i, j;

    printf("%s:\n", title);

    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            printf("%12.4f ", m[i][j]);
        }
        printf("\n");
    }
}
#endif

static void 
normalizeColumns( float a[3][3])
{
    int i;

    for ( i=0; i<3; i++) {
        float rlen = 1.0f / (a[0][i] * a[0][i] +
                             a[1][i] * a[1][i] +
                             a[2][i] * a[2][i]);
        a[0][i] *= rlen;
        a[1][i] *= rlen;
        a[2][i] *= rlen;
    }
}

// OffD[] exploits the symmetry of S[][].
static void eigenVectors (const float S[3][3], float U[3][3], float K[3])
{
#define LOSSY_OPTIMIZATIONS 0  // gets about 5%, might affect results
#if LOSSY_OPTIMIZATIONS
    // XX How can we generate the single-precision fabs Intel instruction?
    float OffD[3], sm, g, h, fabsh, fabsOffDi, t, theta;
    float c, s, tau, ta, OffDq, a, b;
#else
    double OffD[3], sm, g, h, fabsh, fabsOffDi, t, theta;
    double c, s, tau, ta, OffDq, a, b;
#endif
    int sweep, i, j, p, q;
    static int mod3[]  = { 0, 1, 2, 0, 1, 2};

    for (i = 0; i < 3; i++) {
        U[i][i] = 1.0f;
        K[i] = S[i][i];
        OffD[i] = S[mod3[i + 1]][mod3[i + 2]];
        for (j = i + 1; j < 3; j++) {
            U[i][j] = U[j][i] = 0.0f;
        }
    }
    for (sweep = 25; sweep > 0; sweep--) {
        sm = fabs(OffD[0]) + fabs(OffD[1]) + fabs(OffD[2]);
        if (sm == 0.0f) break;
        for (i = 2; i >= 0; i--) {
            p = mod3[i + 1];
            q = mod3[i + 2];
     
            fabsOffDi = fabs(OffD[i]);
            g = (float) (100.0 * fabsOffDi);
            if (fabsOffDi > 0.0) {
                h = K[q] - K[p];
                fabsh = fabs(h);
                if (fabsh + g == fabsh) {
                    t = OffD[i] / h;
                } else {
                    theta = 0.5 * h / OffD[i];
                    t = 1.0 / (fabs(theta) + sqrt(theta * theta + 1.0));
                    if (theta < 0) t = -t;
                }
                c = 1.0 / sqrt(t * t + 1);
                s = t * c;
                tau = s / (c + 1);
                ta = t * OffD[i];
                OffD[i] = 0.0;
                K[p] -= (float) ta;
                K[q] += (float) ta;
                OffDq = OffD[q];
                OffD[q] -= s * (OffD[p] + tau * OffD[q]);
                OffD[p] += s * (OffDq - tau * OffD[p]);
                for (j = 2; j >= 0; j--) {
                    a = U[j][p];
                    b = U[j][q];
                    U[j][p] -= (float) (s * (b + tau * a));
                    U[j][q] += (float) (s * (a - tau * b));
                }
            }
        }
    }
    // printMatrix("\nCovariance Matrix", S);

#define TRUE_COVARIANCE 1 
#if TRUE_COVARIANCE

    /* 
     * Sort eigen values into decreasing absolute order. When this is done, make sure
     * that eigen vectors are swapped as well.
     */
    if (fabs(K[0]) < fabs(K[1])) { swapValue(K, 0, 1); swapVector(U, 0, 1); }
    if (fabs(K[0]) < fabs(K[2])) { swapValue(K, 0, 2); swapVector(U, 0, 2); }
    if (fabs(K[1]) < fabs(K[2])) { swapValue(K, 1, 2); swapVector(U, 1, 2); }

    /*
     * Normalize so that the transpose will also be the inverse.
     */
    normalizeColumns( U);

#else
    /* Eigen values can't be negative */
//    assert( K[0] >= 0.0f);
//    assert( K[1] >= 0.0f);
//    assert( K[2] >= 0.0f);
    if (K[0] < 0.0f) K[0] = 0.0f;
    if (K[1] < 0.0f) K[1] = 0.0f;
    if (K[2] < 0.0f) K[2] = 0.0f;

    /* 
     * Sort eigen values into decreasing order. When this is done, make sure
     * that eigen vectors are swapped as well.
     */
    if (K[0] < K[1]) { swapValue(K, 0, 1); swapVector(U, 0, 1); }
    if (K[0] < K[2]) { swapValue(K, 0, 2); swapVector(U, 0, 2); }
    if (K[1] < K[2]) { swapValue(K, 1, 2); swapVector(U, 1, 2); }

#endif

#if 0
    /*
     * Make the first column of eigenvectors positive.
     * XXX Shouldn't it be the first *row* made positive?
     */
    for (i=0; i<3; i++) {
        if (U[i][0] < 0.0f) 
            for (j=0; j<3; j++) 
                U[i][j] *= -1.0f;
    }
#endif

#if 0
    printf("EivenValues: %8.2f %8.2f %8.2f\n", K[0], K[1], K[2]);
    printMatrix("Eigen Vectors    ", U);
#endif

}

/* Create a 3x3 covariance matrix from a given nx3 data matrix */
static void
covariance(int n, float data[][3], float mean[3], float cov[3][3])
{
    int     i, j, k;

    /* Now compute cov[3][3] = Transpose(data[n][3]) * data[n][3] */
    for (i=0; i<3; i++) {
        for (j=i; j<3; j++) {
            cov[i][j] = 0.0f;
        }
    }
    for (k=0; k<n; k++) {
        cov[0][0] += data[k][0] * data[k][0];
        cov[0][1] += data[k][0] * data[k][1];
        cov[0][2] += data[k][0] * data[k][2];
        cov[1][1] += data[k][1] * data[k][1];
        cov[1][2] += data[k][1] * data[k][2];
        cov[2][2] += data[k][2] * data[k][2];
    }
#if TRUE_COVARIANCE
    cov[0][0] -= mean[0] * mean[0] * n;
    cov[0][1] -= mean[0] * mean[1] * n;
    cov[0][2] -= mean[0] * mean[2] * n;
    cov[1][1] -= mean[1] * mean[1] * n;
    cov[1][2] -= mean[1] * mean[2] * n;
    cov[2][2] -= mean[2] * mean[2] * n;
    cov[0][0] /= n-1;
    cov[0][1] /= n-1;
    cov[0][2] /= n-1;
    cov[1][1] /= n-1;
    cov[1][2] /= n-1;
    cov[2][2] /= n-1;
#endif
    for (i=0; i<3; i++) {
        for (j=i; j<3; j++) {
            cov[j][i] = cov[i][j];
        }
    }

    // printMatrix("\n\n--------Covariance matrix:", cov);
}


#if 0
static char *progname = NULL;
static char *filename = NULL;

void
usage()
{
    fprintf(stderr, "Usage: %s [filename with 3 components per row]\n");
    exit(0);
}

int
main(int argc, char **argv)
{
    int     n, maxn;
    float   *data, cov[3][3], evectors[3][3], evalues[3];
    FILE    *stream;

    progname = *argv++; argc--;
    filename = *argv++; argc--;

    if (filename) {
        printf("Filename = %s\n", filename);
        stream = fopen(filename, "r");
        if (stream == NULL) usage();
    } else {
        stream = stdin;
    }

    n    = 0;
    maxn = 10;
    data = (float *) malloc(3 * maxn * sizeof(float));

    while (1) {
        if (fscanf(stream, "%f %f %f", 
            &data[3*n + 0], &data[3*n+1], &data[3*n+2]) != 3) break;
        n++;
        if (n >= maxn) {
            maxn += 10;
            printf("Realloc'ing data, maxn = %d\n", maxn);
            data = (float *) realloc(data, 3 * maxn * sizeof(float));
            if (data == NULL) {
                fprintf(stderr, "Couldn't realloc!\n");
                exit(0);
            }
        }
    }

    for (maxn = 0; maxn < n; maxn++) {
        printf("Row %5d: %8.0f %8.0f %8.0f\n", 
            maxn, data[3*maxn+0], data[3*maxn+1], data[3*maxn+2]);
    }

    covariance(n, (float (*)[3]) data, cov); 
    eigenVectors(cov, evectors, evalues);
}
#endif

// computes idata[]*m[][]
void
eigenProject( int n, float *idata, float mean[3], float m[3][3], float *odata)
{
    int i, j;

    for (i=0; i<n; i++) {
        float   tdata[3];       // temporary space.

        // XX MSVC doesn't unroll loops!? 
        for (j=0; j<3; j++) {
            tdata[j] =  ((idata[0]-mean[0]) * m[0][j])
                      + ((idata[1]-mean[1]) * m[1][j])
                      + ((idata[2]-mean[2]) * m[2][j]);
        }
        // This is necessary in case idata == odata
        for (j=0; j<3; j++) {
            odata[i*3+j] = tdata[j];

        }
        idata += 3;
    }
}

void
eigenSpace(int n, float *data, float mean[3], float evectors[3][3], float evalues[3])
{
    float   cov[3][3];

    covariance(n, (float (*)[3]) data, mean, cov); 
    eigenVectors((const float (*)[3])cov, evectors, evalues);
}

/*
 * Given an input vector input[n][3], this routine computes various
 * statistics using the eigen Transform.
 */
#define NCOMPONENTS 3
void
eigenStatistics(
    int     n, 
    const   float input[][3], 

    float   evalues[3],
    float   output[][3],        // after transformation to eigenspace
    float   lo[3][3],           // lo and hi colors for each evector
    float   hi[3][3],

    float   avg[3],             // this is the mean value of this block
    float   min[3],             // min and max values for each evector
    float   max[3], 
    float   errs[3]             // max error incurred when dropping each evector
    )
{
    float   evectors[NCOMPONENTS][3]; // each eigenvector occupies a *column*.
    int     i, j;

    if (n < 1) {
        fprintf(stderr, "Bad n: %d (File %s)\n", n, __FILE__);
        exit(0);
    }

#if TRUE_COVARIANCE
    // Compute averages of each component
    for (j = 0; j < NCOMPONENTS; j++) 
        avg[j] = 0.0f;

    for (i=0; i<n; i++) {
        for (j=0; j < NCOMPONENTS; j++) {
            avg[j] += input[i][j];
        }
    }
    for (j = 0; j < NCOMPONENTS; j++) 
        avg[j] = avg[j] / n;
#endif

    // output[i][j] = mean removed input[i][j]
    for (i=0; i<n; i++) {
        for (j = 0; j < NCOMPONENTS; j++) {
            output[i][j] = input[i][j]  ;       //  - avg[j];
        }
    }

    // Compute eigenValues, eigenVectors
    eigenSpace(n, (float*) output, avg, evectors, (float*) evalues);

    // Project to eigenSpace
    eigenProject(n, (float*) output, avg, evectors, (float*) output);


    // Find min and max values over the projections of all colors into eigen space
    // Since scale of each eigenvector is arbitrary, so is scale of each min/max pair,
    // unless our particular eigenSpace() routine offers some special guarantee.
    for (j = 0; j < NCOMPONENTS; j++) {
        min[j] = max[j] = output[0][j];
    }
    for (i = 1; i < n; i++) {
        if (min[0] > output[i][0]) min[0] = output[i][0];
        if (max[0] < output[i][0]) max[0] = output[i][0];
        if (min[1] > output[i][1]) min[1] = output[i][1];
        if (max[1] < output[i][1]) max[1] = output[i][1];
        if (min[2] > output[i][2]) min[2] = output[i][2];
        if (max[2] < output[i][2]) max[2] = output[i][2];
    }

    // Find lo and hi values for each eigenVector
    // XX  This apparently accomplishes some sort of back-projection of the axis-aligned 
    //     eigen space bounding box to color space, where it will not in general be axis-aligned.
    //     (lo[0] and hi[0] are used as end-point colors in interpolation modes)
    for (i = 0; i < 3; i++) {                 // selects an eigen vector, and an axis in eigen space
        for (j = 0; j < NCOMPONENTS; j++) {   // selects an axis of the eigen vector
            lo[i][j] = evectors[j][i] * min[i] + avg[j];
            hi[i][j] = evectors[j][i] * max[i] + avg[j];
        }
    }

    // Compute MAX abs(spread) over all colors for each eigen vector
    // For the second and third eigen vectors, in an interpolation mode,
    // this really does bound the color space error.
    for (i=0; i<3; i++) {                      // selects an eigen vector

        errs[i] = 0.0f;
        for (j = 0; j < NCOMPONENTS; j++) {    // selects a color  
            float   e;

            e = lo[i][j] - hi[i][j];
            if (e < 0.0f) 
                e = -e;
            if (errs[i] < e) 
                errs[i] = e;   // MAX
        }
    }
}

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
    )
{
    int i;

    if (title) {
        fprintf(stdout, title);
    }

    if (input) {
        fprintf(stdout, "Input  Vector:\n");
        for (i=0; i<n; i++) {
            fprintf(stdout, "[%4.0f %4.0f %4.0f] ",
                input[i][0], input[i][1], input[i][2]);
            if ((i % 4) == 3) fprintf(stdout, "\n");
        }
    }

    if (output) {
        fprintf(stdout, "Output Vector:\n");
        for (i=0; i<n; i++) {
            fprintf(stdout, "[%4.0f %4.0f %4.0f] ", 
                output[i][0], output[i][1], output[i][2]);
            if ((i % 4) == 3) fprintf(stdout, "\n");
        }
    }

#ifdef notdef
    for (i=0; i<3; i++) {
        fprintf(stdout, 
            "V%d: [%4.0f %4.0f %4.0f] - [%4.0f %4.0f %4.0f] [%4.0f %4.0f %6.2f]\n",
            i, 
            lo[i][0] + avg[0], lo[i][1] + avg[1], lo[i][2] + avg[2],
            hi[i][0] + avg[0], hi[i][1] + avg[1], hi[i][2] + avg[2],
            min[i], max[i], err[i]);
    }
#endif
}

