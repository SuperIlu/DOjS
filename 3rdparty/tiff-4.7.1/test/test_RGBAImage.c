/*
 * TIFF Library
 *
 * Copyright (c) 2025, Su Laus  @Su_Laus
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * the author may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of the author.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

/* ===========  Purpose ===============================================
 *
 * Test file for the TFFRGBAImage functions.
 *
 * Test points are:
 * - Tests are performed using RGB test-images.
 * - Pixel orientation within "raster" of TIFFRGBAImageGet() and
 *   TIFFRGBAImage().
 * - Image data are always located at lower, left-hand part of the raster
 * matrix.
 * - Test for buffer overflows.
 *
 * Tests for following improvements:
 * - Raster width can now be larger than image width.
 * - Only image data are copied to raster buffer if tiles are padded.
 * - Avoid buffer overflow if col_offset > 0.
 * - If row_offset > 0 do not try to read after last row - avoiding warnings.
 * - Feature "col_offset" and "row_offset" now works as expected.
 *
 */

#include "tif_config.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "tiffio.h"

// #define DEBUG_TESTING
#ifdef DEBUG_TESTING
#define GOTOFAILURE                                                            \
    {                                                                          \
    }
#else
/*  Only for automake and CMake infrastructure the test should:
    a.) delete any written testfiles when test passed
        (otherwise autotest will fail)
    b.) goto failure, if any failure is detected, which is not
        necessary when test is initiated manually for debugging.
*/
#define GOTOFAILURE goto failure;
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* Further tweak variables for this test module. */
bool blnSpecialTest = FALSE;

/* Global parameter for writing to a logfile and/or to screen. */
FILE *stdXOut = NULL;
bool blnStdOutToLogFile = FALSE;
bool blnQuiet = FALSE;

#ifdef DEBUG_TESTING
const char *logFilename = "test_RGBAImage_log.txt";
#else
const char *logFilename = NULL;
#endif
bool blnMultipleLogFiles = FALSE;
char *arrLogFilenames[] = {
    "test_RGBAImage_log_1.txt", "test_RGBAImage_log_2.txt",
    "test_RGBAImage_log_3.txt", "test_RGBAImage_log_4.txt"};
FILE *fpLog = NULL;
bool blnPrintRasterToScreen = FALSE;

/* Global values for write_data_to_current_directory()
 * which are set only on purpose to different values. */
uint16_t photometric = PHOTOMETRIC_RGB;
uint16_t planarconfig = PLANARCONFIG_CONTIG;
uint32_t rows_per_strip = 1;

#ifndef TIFFmin
#define TIFFmax(A, B) ((A) > (B) ? (A) : (B))
#define TIFFmin(A, B) ((A) < (B) ? (A) : (B))
#endif

/* Global settings -not save to change- */
#define SPP 3 /* samples per pixel */
#define BPS 8 /* bits per sample */

const char *modeStrings[] = {"wl", "wb", "w8l", "w8b"};
const char *orientationStrings[] = {
    "none",     "TOPLEFT", /*1 row 0 top, col 0 lhs */
    "TOPRIGHT",            /*2 row 0 top, col 0 rhs */
    "BOTRIGHT",            /*3 row 0 bottom, col 0 rhs */
    "BOTLEFT",             /*4 row 0 bottom, col 0 lhs */
    "LEFTTOP ",            /*5 row 0 lhs, col 0 top */
    "RIGHTTOP",            /*6 row 0 rhs, col 0 top */
    "RIGHTBOT",            /*7 row 0 rhs, col 0 bottom */
    "LEFTBOT"              /*8 row 0 lhs, col 0 bottom */
};

/* Defines to have error checking but also clean and readable source code. */

#define TIFFSetField_M(tif, tag, value, filename, line)                        \
    if (!TIFFSetField(tif, tag, value))                                        \
    {                                                                          \
        fprintf(stdXOut, "Can't set tag %d (%s) for %s at line %d\n", tag,     \
                filename, TIFFFieldName(TIFFFieldWithTag(tif, tag)), line);    \
        goto failure;                                                          \
    }

#define TIFFGetField_M(tif, tag, value, filename, line)                        \
    if (!TIFFGetField(tif, tag, value))                                        \
    {                                                                          \
        fprintf(stdXOut, "Can't get tag %d (%s) for %s at line %d\n", tag,     \
                filename, TIFFFieldName(TIFFFieldWithTag(tif, tag)), line);    \
        goto failure;                                                          \
    }

#define TIFFOpen_M(tif, filename, modeString, line)                            \
    tif = TIFFOpen(filename, modeString);                                      \
    if (!tif)                                                                  \
    {                                                                          \
        fprintf(stdXOut, "Can't create %s. Testline %d\n", filename,           \
                __LINE__);                                                     \
        return 1;                                                              \
    }

#define TIFFWriteDirectory_M(tif, filename, line)                              \
    if (!TIFFWriteDirectory(tif))                                              \
    {                                                                          \
        fprintf(stdXOut, "Can't write directory to %s at line %d\n", filename, \
                line);                                                         \
        goto failure;                                                          \
    }

#define TIFFCheckpointDirectory_M(tif, dirnum, filename, line)                 \
    if (!TIFFCheckpointDirectory(tif))                                         \
    {                                                                          \
        fprintf(stdXOut, "Can't checkpoint directory %d of %s at line %d\n",   \
                dirnum, filename, line);                                       \
        goto failure;                                                          \
    }

#define TIFFSetDirectory_M(tif, dirnum, filename, line)                        \
    if (!TIFFSetDirectory(tif, dirnum))                                        \
    {                                                                          \
        fprintf(stdXOut, "Can't set directory %d of %s at line %d\n", dirnum,  \
                filename, line);                                               \
        goto failure;                                                          \
    }

#define TIFFWriteScanline_M(tif, buf, row, sample, filename, line)             \
    if (TIFFWriteScanline(tif, buf, row, sample) == -1)                        \
    {                                                                          \
        fprintf(                                                               \
            stdXOut,                                                           \
            "Can't write image data scanline %d sample %d of %s at line %d\n", \
            row, sample, filename, line);                                      \
        goto failure;                                                          \
    }

/* Writes some pixel data as scanline or tiles to file.
 */
int write_image_data(TIFF *tif, uint32_t width, uint32_t length, bool tiled,
                     unsigned int pixval, unsigned char *plastlinedata,
                     unsigned int lastlinebytesmax)
{
    size_t bufLen;
    unsigned char *pbufLine = NULL;
    unsigned int bpsmod = (1 << BPS);
    uint32_t tlwidth;
    uint32_t tllength;
    tmsize_t tlsize;

    uint16_t planarconfig, samples_per_pixel;
    uint32_t rows_per_strip = 0;

    (void)pixval;

    const char *filename = TIFFFileName(tif);

    TIFFGetField_M(tif, TIFFTAG_PLANARCONFIG, &planarconfig, filename,
                   __LINE__);
    TIFFGetField_M(tif, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel, filename,
                   __LINE__);

    if (tiled)
    {
        TIFFGetField_M(tif, TIFFTAG_TILEWIDTH, &tlwidth, filename, __LINE__);
        TIFFGetField_M(tif, TIFFTAG_TILELENGTH, &tllength, filename, __LINE__);
        /* For tiled mode get size of a tile in bytes */
        tlsize = TIFFTileSize(tif);
        bufLen = (size_t)tlsize;
    }
    else
    {
        TIFFGetField_M(tif, TIFFTAG_ROWSPERSTRIP, &rows_per_strip, filename,
                       __LINE__);
        /* For strip mode get size of a row in bytes */
        bufLen = (((size_t)width * SPP * BPS) + 7) / 8;
    }

    pbufLine = _TIFFmalloc(bufLen);
    if (pbufLine == NULL)
        return 1;

    /* Write dummy pixel data. */
    if (tiled)
    {
        /* With SEPARATE, the complete image of each color is written one after
         * the other. */
        uint32_t numtiles = TIFFNumberOfTiles(tif);
        uint32_t this_spp =
            (planarconfig < PLANARCONFIG_SEPARATE ? 0 : (SPP - 1));
        uint32_t last_width = width % tlwidth;
        uint32_t tiles_per_row = width / tlwidth + (last_width > 0 ? 1 : 0);
        uint32_t last_length = length % tllength;
        uint32_t tiles_per_col = length / tllength + (last_length > 0 ? 1 : 0);
        /* Correct values for following processing at special case. */
        if (last_width == 0)
            last_width = tlwidth;
        if (last_length == 0)
            last_length = tllength;

        uint32_t i, j, k, s, row, this_tlwidth, this_tllength;
        uint32_t coltile, rowtile;
        for (s = 0; s <= this_spp; s++)
        {
            for (coltile = 0; coltile < tiles_per_col; coltile++)
            {
                if (coltile < (tiles_per_col - 1))
                    this_tllength = tllength;
                else
                    this_tllength = last_length;
                for (rowtile = 0; rowtile < tiles_per_row; rowtile++)
                {
                    if (rowtile < (tiles_per_row - 1))
                        this_tlwidth = tlwidth;
                    else
                        this_tlwidth = last_width;
                    j = 0;
                    for (row = 0; row < this_tllength; row++)
                    {
                        /* Fill tile buffer. */
                        for (k = 0; (k < this_tlwidth); k++)
                        {
                            if (planarconfig < PLANARCONFIG_SEPARATE || s == 0)
                                pbufLine[j++] =
                                    (unsigned char)((row + coltile * tllength) %
                                                    bpsmod); /* row */
                            if (planarconfig < PLANARCONFIG_SEPARATE || s == 1)
                                pbufLine[j++] =
                                    (unsigned char)((k + rowtile * tlwidth) %
                                                    bpsmod); /* column */
                            if (planarconfig < PLANARCONFIG_SEPARATE || s == 2)
                                pbufLine[j++] = (unsigned char)((254) % bpsmod);
                        }
                        /* Fill rest of row in last tile with zeros. */
                        for (; (k < tlwidth); k++)
                        {
                            if (planarconfig < PLANARCONFIG_SEPARATE || s == 0)
                                pbufLine[j++] = 0; /* row */
                            if (planarconfig < PLANARCONFIG_SEPARATE || s == 1)
                                pbufLine[j++] = 0; /* column */
                            if (planarconfig < PLANARCONFIG_SEPARATE || s == 2)
                                pbufLine[j++] =
                                    (unsigned char)((0xcc) % bpsmod);
                        }
                    } /* row */
                    /* Calculate tile-number for TIFFWriteEncodedTile(). */
                    i = s * tiles_per_row * tiles_per_col +
                        coltile * tiles_per_row + rowtile;
                    if (TIFFWriteEncodedTile(tif, i, pbufLine, 0) == -1)
                    {
                        fprintf(stdXOut,
                                "Can't write image data tile. Testline %d\n",
                                __LINE__);
                        return 1;
                    }
                    if ((plastlinedata != NULL) && (i == (numtiles - 1)))
                    {
                        memcpy(plastlinedata, pbufLine,
                               TIFFmin(bufLen, (size_t)lastlinebytesmax));
                    }
                } /* rowtile */
            }     /* coltile */
        }         /* s sample in pixel */
    }
    else
    {
        /*== STRIP ==*/
        uint32_t i;
        /* Fill image buffer. */
        uint32_t k, j;
        uint16_t s;
        if (planarconfig == PLANARCONFIG_CONTIG)
        {
            for (i = 0; i < length; i++)
            {
                for (k = 0, j = 0; k < width && j <= (bufLen - SPP);
                     k++, j += SPP)
                {
                    pbufLine[j] = (unsigned char)((i) % bpsmod); /* row */
                    pbufLine[j + 1] =
                        (unsigned char)((k) % bpsmod); /* column */
                    pbufLine[j + 2] = (unsigned char)((254) % bpsmod);
                }
                TIFFWriteScanline_M(tif, pbufLine, i, 0, filename, __LINE__);
            }
        }
        else
        {
            /* SEPARATE - RRRR GGGGG BBBBB, each all lines per strip */
            for (i = 0; i < length; i += rows_per_strip)
            {
                for (s = 0; s < samples_per_pixel; s++)
                {
                    for (j = 0; j < rows_per_strip; j++)
                    {
                        for (k = 0; k < width; k++)
                        {
                            if (s == 0)
                                pbufLine[k] =
                                    (unsigned char)((i + j) % bpsmod); /* row */
                            else if (s == 1)
                                pbufLine[k] =
                                    (unsigned char)((k) % bpsmod); /* column */
                            else if (s == 2)
                                pbufLine[k] = (unsigned char)((254) % bpsmod);
                        }
                        TIFFWriteScanline_M(tif, pbufLine, (i + j), s, filename,
                                            __LINE__);
                    }
                }
            }
        }
        if ((plastlinedata != NULL) && (i == (length - 1)))
        {
            memcpy(plastlinedata, pbufLine,
                   TIFFmin(bufLen, (size_t)lastlinebytesmax));
        }
    } /* -- strip --*/
    _TIFFfree(pbufLine);
    return 0;
failure:
    return 1;
} /*-- write_image_data() --*/

/* Fills the active IFD with some default values and writes
 * an image with given number of lines as strips (scanlines) or tiles to
 * file.
 */
int write_data_to_current_directory(TIFF *tif, uint32_t width, uint32_t length,
                                    bool tiled, uint16_t orientation,
                                    bool write_data,
                                    unsigned char *plastlinedata,
                                    unsigned int lastlinebytesmax)
{
    if (!tif)
    {
        fprintf(stdXOut, "Invalid TIFF handle. Line %d\n", __LINE__);
        return 1;
    }
    const char *filename = TIFFFileName(tif);
    TIFFSetField_M(tif, TIFFTAG_IMAGEWIDTH, width, filename, __LINE__);
    TIFFSetField_M(tif, TIFFTAG_IMAGELENGTH, length, filename, __LINE__);
    TIFFSetField_M(tif, TIFFTAG_BITSPERSAMPLE, BPS, filename, __LINE__);
    TIFFSetField_M(tif, TIFFTAG_SAMPLESPERPIXEL, SPP, filename, __LINE__);

    if (tiled)
    {
        /* Tilesizes must be multiple of 16. */
        TIFFSetField_M(tif, TIFFTAG_TILEWIDTH, 16, filename, __LINE__);
        TIFFSetField_M(tif, TIFFTAG_TILELENGTH, 16, filename, __LINE__);
    }
    else
    {
        TIFFSetField_M(tif, TIFFTAG_ROWSPERSTRIP, rows_per_strip, filename,
                       __LINE__);
    }

    TIFFSetField_M(tif, TIFFTAG_PLANARCONFIG, planarconfig, filename, __LINE__);
    TIFFSetField_M(tif, TIFFTAG_PHOTOMETRIC, photometric, filename, __LINE__);
    /* Write Orientation tag. */
    TIFFSetField_M(tif, TIFFTAG_ORIENTATION, orientation, filename, __LINE__);

    /* Write dummy pixel data. */
    if (write_data)
    {
        if (write_image_data(tif, width, length, tiled, 99, plastlinedata,
                             lastlinebytesmax))
        {
            fprintf(stdXOut, "Can't write image data. Testline %d\n", __LINE__);
            return 1;
        }
    }
    return 0;

failure:
    return 1;
} /*-- write_data_to_current_directory() --*/

/* Some defines for TIFFRGBAImage raster data handling. */
#define A1 (((uint32_t)0xffL) << 24)
#define PACK(r, g, b)                                                          \
    ((uint32_t)(r) | ((uint32_t)(g) << 8) | ((uint32_t)(b) << 16) | A1)

#define RASTER_MEMSETVAL 0xba

/* Check some contents of the raster buffer. Ensure they are correctly filled.
 */
int checkRasterContents(char *txt, TIFFRGBAImage *img, uint32_t *raster,
                        uint32_t rw, uint32_t rh, uint16_t orientation)
{
    /* For this test, the image pixel samples are set for R= row, G= column, B=
     * 0xfe and the last raster component is 0xff. The raster is preset with
     * 0xbe. In general, according to documentation:
     *  - If the raster height is greater than that of the image, then the image
     * data are placed in the lower part of the raster.
     *  - If the raster width is greater than that of the image, then the image
     * data are placed in the left part of the raster.
     */
    typedef union
    {
        uint8_t u8[4];
        uint32_t u32;
    } RasterPixel;
    typedef struct
    {
        uint32_t x;
        uint32_t y;
    } Coordinate;

    (void)txt;

    /* Exit for special test case */
    if (rh == 0 || rw == 0)
        return 0;

    /* When image is cropped, or raster is larger than image, determine crop
     * lengths for image within raster. */
    uint32_t rwmin = TIFFmin(rw, img->width - img->col_offset);
    uint32_t rhmin = TIFFmin(rh, img->height - img->row_offset);
    /* When there is a col_offset or row_offset, the image pixels of the corners
     * are differently. */
    uint32_t xcol_start = img->col_offset;
    uint32_t xrow_start = img->row_offset;

    /* Pixel values of image corner pixels according to image dimensions written
     * in write_image_data(). Pixel values are set to (rowIdx, colIdx, 0xfe).
     * The first pixel stored in the file is P0 with I0=(0,0,0xfe);
     * the last pixel of the first data row is P1 with I1=(0,imgWidth-1,0xfe);
     * the first pixel of the last row is P2 with I2=(imgLength-1,0,0xfe) and
     * the last pixel of the last row is P3 with
     * I3=(imgLength-1,imgWidth-1,0xfe). P/I are arranged in Z-order: P0---P1
     *    P2---P3
     * I01 is value of row-0 and col-1 or in image coordinates x=1, y=0.
     */
    RasterPixel I[8];
    /* Without offsets: */
    // I[0].u32 = PACK(0, 0, 0xfe);
    // I[1].u32 = PACK(0, rwmin - 1, 0xfe);
    // I[2].u32 = PACK(rhmin - 1, 0, 0xfe);
    // I[3].u32 = PACK(rhmin - 1, rwmin - 1, 0xfe);
    /* With offsets: */
    I[0].u32 = PACK(xrow_start, xcol_start, 0xfe);
    I[1].u32 = PACK(xrow_start, xcol_start + rwmin - 1, 0xfe);
    I[2].u32 = PACK(xrow_start + rhmin - 1, xcol_start, 0xfe);
    I[3].u32 = PACK(xrow_start + rhmin - 1, xcol_start + rwmin - 1, 0xfe);
    /* The value of the next pixels in the row (or behind) to the pixels P0 th
     * P3 is: P0, P4 --- P1, P5 P2, P6 --- P3, P7
     */
    I[4].u32 = PACK(I[0].u8[0], I[0].u8[1] + 1, 0xfe);
    I[5].u32 = PACK(RASTER_MEMSETVAL, RASTER_MEMSETVAL, RASTER_MEMSETVAL);
    I[6].u32 = PACK(I[2].u8[2], I[2].u8[1] + 1, 0xfe);
    I[7].u32 = PACK(RASTER_MEMSETVAL, RASTER_MEMSETVAL, RASTER_MEMSETVAL);

    /* Depending on orientation (and requested orientation) the input data are
     * fliped horizontally and / or vertically. Without rotation, this leads to
     * four possible orientations (locations of the first pixel from the image
     * file). See tif_getimage.c setorientation(). If the raster matrix is
     * larger than the image, the image data is stored in the lower-left part of
     * the raster matrix.
     *
     * "firstPixelAt" gives the orientation of the data from file, stored in the
     * raster matrix; i.e. the location of the first pixel from the file.
     */

    int firstPixelAt =
        ORIENTATION_TOPLEFT; /* if data from file is not flipped */
    switch (orientation)
    {
        case ORIENTATION_TOPLEFT:
        case ORIENTATION_LEFTTOP:
            if (img->req_orientation == ORIENTATION_TOPRIGHT ||
                img->req_orientation == ORIENTATION_RIGHTTOP)
                firstPixelAt = ORIENTATION_TOPRIGHT; // FLIP_HORIZONTALLY
            else if (img->req_orientation == ORIENTATION_BOTRIGHT ||
                     img->req_orientation == ORIENTATION_RIGHTBOT)
                firstPixelAt = ORIENTATION_BOTRIGHT; // FLIP_HORIZONTALLY |
                                                     // FLIP_VERTICALLY;
            else if (img->req_orientation == ORIENTATION_BOTLEFT ||
                     img->req_orientation == ORIENTATION_LEFTBOT)
                firstPixelAt = ORIENTATION_BOTLEFT; // FLIP_VERTICALLY;
            break;
        case ORIENTATION_TOPRIGHT:
        case ORIENTATION_RIGHTTOP:
            if (img->req_orientation == ORIENTATION_TOPLEFT ||
                img->req_orientation == ORIENTATION_LEFTTOP)
                firstPixelAt = ORIENTATION_TOPRIGHT; // FLIP_HORIZONTALLY;
            else if (img->req_orientation == ORIENTATION_BOTRIGHT ||
                     img->req_orientation == ORIENTATION_RIGHTBOT)
                firstPixelAt = ORIENTATION_BOTLEFT; // FLIP_VERTICALLY;
            else if (img->req_orientation == ORIENTATION_BOTLEFT ||
                     img->req_orientation == ORIENTATION_LEFTBOT)
                firstPixelAt = ORIENTATION_BOTRIGHT; // FLIP_HORIZONTALLY |
                                                     // FLIP_VERTICALLY;
            break;
        case ORIENTATION_BOTRIGHT:
        case ORIENTATION_RIGHTBOT:
            if (img->req_orientation == ORIENTATION_TOPLEFT ||
                img->req_orientation == ORIENTATION_LEFTTOP)
                firstPixelAt = ORIENTATION_BOTRIGHT; // FLIP_HORIZONTALLY |
                                                     // FLIP_VERTICALLY;
            else if (img->req_orientation == ORIENTATION_TOPRIGHT ||
                     img->req_orientation == ORIENTATION_RIGHTTOP)
                firstPixelAt = ORIENTATION_BOTLEFT; // FLIP_VERTICALLY;
            else if (img->req_orientation == ORIENTATION_BOTLEFT ||
                     img->req_orientation == ORIENTATION_LEFTBOT)
                firstPixelAt = ORIENTATION_TOPRIGHT; // FLIP_HORIZONTALLY;
            break;
        case ORIENTATION_BOTLEFT:
        case ORIENTATION_LEFTBOT:
            if (img->req_orientation == ORIENTATION_TOPLEFT ||
                img->req_orientation == ORIENTATION_LEFTTOP)
                firstPixelAt = ORIENTATION_BOTLEFT; // FLIP_VERTICALLY;
            else if (img->req_orientation == ORIENTATION_TOPRIGHT ||
                     img->req_orientation == ORIENTATION_RIGHTTOP)
                firstPixelAt = ORIENTATION_BOTRIGHT; // FLIP_HORIZONTALLY |
                                                     // FLIP_VERTICALLY;
            else if (img->req_orientation == ORIENTATION_BOTRIGHT ||
                     img->req_orientation == ORIENTATION_RIGHTBOT)
                firstPixelAt = ORIENTATION_TOPRIGHT; // FLIP_HORIZONTALLY;
            break;
        default: /* NOTREACHED */
            break;
    }

    /* Determine position in raster-matrix (0..rw-1 / 0..rh-1)
     * If the raster matrix is larger than the image, the image data is stored
     * in the lower-left part of the raster matrix.
     * P0 to P3 are image corner in Z-order sequence within raster matrix:
     *    P0---P1
     *    P2---P3
     */
    Coordinate P[8];
    switch (firstPixelAt)
    {
        case ORIENTATION_TOPLEFT:
            P[0].x = 0;
            P[0].y = TIFFmax(0, (tmsize_t)rh - rhmin);
            P[1].x = rwmin - 1;
            P[1].y = TIFFmax(0, (tmsize_t)rh - rhmin);
            P[2].x = 0;
            P[2].y = rh - 1;
            P[3].x = rwmin - 1;
            P[3].y = rh - 1;
            break;
        case ORIENTATION_TOPRIGHT:
            P[0].x = rwmin - 1;
            P[0].y = TIFFmax(0, (tmsize_t)rh - rhmin);
            P[1].x = 0;
            P[1].y = TIFFmax(0, (tmsize_t)rh - rhmin);
            P[2].x = rwmin - 1;
            P[2].y = rh - 1;
            P[3].x = 0;
            P[3].y = rh - 1;
            break;
        case ORIENTATION_BOTRIGHT:
            P[0].x = rwmin - 1;
            P[0].y = rh - 1;
            P[1].x = 0;
            P[1].y = rh - 1;
            P[2].x = rwmin - 1;
            P[2].y = TIFFmax(0, (tmsize_t)rh - rhmin);
            P[3].x = 0;
            P[3].y = TIFFmax(0, (tmsize_t)rh - rhmin);
            break;
        case ORIENTATION_BOTLEFT:
            P[0].x = 0;
            P[0].y = rh - 1;
            P[1].x = rwmin - 1;
            P[1].y = rh - 1;
            P[2].x = 0;
            P[2].y = TIFFmax(0, (tmsize_t)rh - rhmin);
            P[3].x = rwmin - 1;
            P[3].y = TIFFmax(0, (tmsize_t)rh - rhmin);
            break;
        default: /* NOTREACHED */
            fprintf(stdXOut, "Error in SWITCH statement at line %d", __LINE__);
            goto failure;
            break;
    }

    /* Determine offset of P[i] into the raster of uint32_t: */
    uint32_t roff[8];
#define NUMP 4
    for (int i = 0; i < NUMP; i++)
    {
        roff[i] = P[i].y * rw + P[i].x;
    }

    /* Check value of corner pixels at expected location in raster. */
    RasterPixel a;
    for (int i = 0; i < NUMP; i++)
    {
        a.u32 = raster[roff[i]];
        if (a.u32 != I[i].u32)
        {
            fprintf(stdXOut,
                    "\nPixel value of P%d = (%d, %d, %d)/(%02x, %02x, %02x) in "
                    "raster at offset %d does not match expected value "
                    "(%d, "
                    "%d, %d)/(%02x, %02x, %02x)",
                    i, a.u8[0], a.u8[1], a.u8[2], a.u8[0], a.u8[1], a.u8[2],
                    roff[i], I[i].u8[0], I[i].u8[1], I[i].u8[2], I[i].u8[0],
                    I[i].u8[1], I[i].u8[2]);
            goto failure;
        }
    }
    return 0;

failure:
    return 1;
} /*-- checkRasterContents() --*/

/* Prints the raster buffer 2D matrix as hex to display and/or to file. */
void printRaster(char *txt, TIFFRGBAImage *img, uint32_t *raster, uint32_t rw,
                 uint32_t rh, uint16_t orientation, bool tiled)
{
    uint32_t h, w;
    FILE *fp = NULL;
    union
    {
        uint8_t u8[4];
        uint32_t u32;
    } u;
    char straux[2048];

    /* If fpLog is already open, don't open and close that file. */
    if (fpLog != NULL)
        fp = fpLog;
    else if (logFilename != NULL)
        fp = fopen(logFilename, "a");

    if (img->col_offset != 0 || img->row_offset != 0)
    {
        sprintf(straux,
                "\n--- (%3d /%3d) Orientation = %d (%s) %s, %s readWidth = %d, "
                "readLength = %d, col_off = %d, row_off = %d, "
                "req_orientation=%d (%s) using %s---\n",
                img->width, img->height, orientation,
                orientationStrings[orientation],
                (planarconfig == 1 ? "CONTIG" : "SEPARATE"),
                (tiled ? "TILED" : "STRIP"), rw, rh, img->col_offset,
                img->row_offset, img->req_orientation,
                orientationStrings[img->req_orientation], txt);
    }
    else
    {
        sprintf(straux,
                "\n--- (%3d /%3d) Orientation = %d (%s) %s, %s readWidth = %d, "
                "readLength = %d, req_orientation=%d (%s) using "
                "%s---\n",
                img->width, img->height, orientation,
                orientationStrings[orientation],
                (planarconfig == 1 ? "CONTIG" : "SEPARATE"),
                (tiled ? "TILED" : "STRIP"), rw, rh, img->req_orientation,
                orientationStrings[img->req_orientation], txt);
    }
    if (fp != NULL)
        fprintf(fp, "%s", straux);
    if (blnPrintRasterToScreen)
        fprintf(stdout, "%s", straux);
    for (h = 0; h < rh; h++)
    {
        for (w = 0; w < rw; w++)
        {
            u.u32 = raster[h * rw + w];
            sprintf(straux, "%02x %02x %02x %02x ", u.u8[0], u.u8[1], u.u8[2],
                    u.u8[3]);
            if (fp != NULL)
                fprintf(fp, "%s", straux);
            if (blnPrintRasterToScreen)
                fprintf(stdout, "%s", straux);
        }
        sprintf(straux, "\n");
        if (fp != NULL)
            fprintf(fp, "%s", straux);
        if (blnPrintRasterToScreen)
            fprintf(stdout, "%s", straux);
    }
    sprintf(straux, "--------------\n");
    if (fp != NULL)
    {
        fprintf(fp, "%s", straux);
        if (fpLog == NULL)
            fclose(fp);
    }
    if (blnPrintRasterToScreen)
        fprintf(stdout, "%s", straux);
    return;
} /*-- printRaster() --*/

/* Calls TIFFRGBAImageGet() and TIFFReadRGBAImage() with given raster sizes and
 * reads the data from file into that raster. The raster content of both
 * functions is printed and checked.
 */
int testRGBAImageReadFunctions(TIFF *tif, uint32_t imgWidth, uint32_t imgLength,
                               uint32_t rWidth, uint32_t rHeight,
                               uint16_t orientation, uint16_t req_orientation,
                               int cLine)
{
    int ret;
    uint32_t *raster1 = NULL;
    uint32_t *raster2 = NULL;
    char emsg[1024] = "";
    TIFFRGBAImage img;
    int ok;

    /* Just for debugging output in printRaster() */
    bool tiledlocal = TIFFIsTiled(tif);

    tmsize_t rasterSize = sizeof(uint32_t) * rWidth * rHeight;
    if (rasterSize == 0)
    {
        rasterSize = sizeof(uint32_t) * imgWidth * imgLength;
    }
    raster1 = (uint32_t *)_TIFFmalloc(rasterSize);
    raster2 = (uint32_t *)_TIFFmalloc(rasterSize);
    if (raster1 == NULL || raster2 == NULL)
    {
        fprintf(stdXOut,
                "Can't allocate 'raster'-buffer. Testline %d called from %d\n",
                __LINE__, cLine);
        goto failure;
    }
    memset(raster1, RASTER_MEMSETVAL, rasterSize);
    memset(raster2, RASTER_MEMSETVAL, rasterSize);

    if (TIFFRGBAImageBegin(&img, tif, 0, emsg))
    {
        img.req_orientation = req_orientation;
        ok = TIFFRGBAImageGet(&img, raster1, rWidth, rHeight);
        printRaster("TIFFRGBAImageGet()", &img, raster1, rWidth, rHeight,
                    orientation, tiledlocal);
        TIFFRGBAImageEnd(&img);
        if (ok != 1)
        {
            fprintf(stdXOut,
                    "TIFFRGBAImageGet() returned failure. Testline %d called "
                    "from %d\n",
                    __LINE__, cLine);
            goto failure;
        }
        if ((ret = checkRasterContents("TIFFRGBAImageGet()", &img, raster1,
                                       rWidth, rHeight, orientation)))
            goto failure;
    }

    _TIFFfree(raster1);
    _TIFFfree(raster2);
    return 0;

failure:
    if (raster1)
        _TIFFfree(raster1);
    if (raster2)
        _TIFFfree(raster2);
    return 1;
} /*-- testRGBAImageReadFunctions() --*/

/* Calls TIFFRGBAImageGet() with col_offset and row_offset and with a given
 * raster and reads the data from file into that raster. The raster content is
 * printed and checked.
 */
int testRGBAImageReadWithOffsets(TIFF *tif, uint32_t imgWidth,
                                 uint32_t imgLength, int w_offset, int l_offset,
                                 uint32_t rWidth, uint32_t rHeight,
                                 uint16_t orientation, uint16_t req_orientation,
                                 int cLine)
{
    int ret;
    uint32_t *raster1 = NULL;
    char emsg[1024] = "";
    TIFFRGBAImage img;
    int ok;

    /* Just for debugging output in printRaster() */
    bool tiledlocal = TIFFIsTiled(tif);

    tmsize_t rasterSize = sizeof(uint32_t) * rWidth * rHeight;
    if (rasterSize == 0)
    {
        rasterSize = sizeof(uint32_t) * imgWidth * imgLength;
    }
    raster1 = (uint32_t *)_TIFFmalloc(rasterSize);
    if (raster1 == NULL)
    {
        fprintf(stdXOut,
                "Can't allocate 'raster'-buffer. Testline %d called from %d\n",
                __LINE__, cLine);
        goto failure;
    }
    memset(raster1, RASTER_MEMSETVAL, rasterSize);

    if (TIFFRGBAImageBegin(&img, tif, 0, emsg))
    {
        img.req_orientation = req_orientation;
        img.col_offset = w_offset;
        img.row_offset = l_offset;
        ok = TIFFRGBAImageGet(&img, raster1, rWidth, rHeight);
        printRaster("TIFFRGBAImageGet()", &img, raster1, rWidth, rHeight,
                    orientation, tiledlocal);
        TIFFRGBAImageEnd(&img);
        if (ok != 1)
        {
            if (!blnQuiet)
                fprintf(stdXOut,
                        "TIFFRGBAImageGet() returned failure in test with "
                        "offsets. Testline %d called from %d\n",
                        __LINE__, cLine);
            goto failure;
        }
        if ((ret = checkRasterContents("TIFFRGBAImageGet()", &img, raster1,
                                       rWidth, rHeight, orientation)))
            goto failure;
    }
    _TIFFfree(raster1);
    return 0;

failure:
    if (raster1)
        _TIFFfree(raster1);
    return 1;
} /*-- testRGBAImageReadWithOffsets() --*/

/* Tests TIFFReadRGBAImage functions with different raster sizes, col_offset,
 * row_offset and required orientations in the raster.
 */
int test_ReadRGBAImage(const char *filename, unsigned int openMode,
                       uint16_t orientation, uint32_t width, uint32_t length,
                       bool tiled, unsigned int req_orientation)
{

    int ret;
    TIFFErrorHandler errHandler = NULL;

    assert(openMode < (sizeof(modeStrings) / sizeof(modeStrings[0])));
    assert(orientation <
           (sizeof(orientationStrings) / sizeof(orientationStrings[0])));
    blnQuiet = FALSE;

#ifdef DEBUG_TESTING
    fprintf(stdXOut,
            "\n==== test_ReadRGBAImage() - sequence --- Orientation = %d (%s) "
            "%s, %s ====\n",
            orientation, orientationStrings[orientation],
            (planarconfig == 1 ? "CONTIG" : "SEPARATE"),
            (tiled ? "TILED" : "STRIP"));
#else
    fprintf(stdXOut, ".");
#endif
    /*-- Create a file and write numIFDs IFDs directly to it --*/
    TIFF *tif = TIFFOpen(filename, modeStrings[openMode]);
    if (!tif)
    {
        fprintf(stdXOut, "\nCan't create %s. Testline %d\n", filename,
                __LINE__);
        return 1;
    }

    /* Writing baseline images (IFDs) to file. */
    if (write_data_to_current_directory(tif, width, length, tiled, orientation,
                                        true, NULL, 0))
    {
        fprintf(stdXOut,
                "\nCan't write data to current directory in %s. Testline %d\n",
                filename, __LINE__);
        goto failure;
    }
    /* Write IFD tags to file (and setup new empty IFD). */
    TIFFWriteDirectory_M(tif, filename, __LINE__);
    TIFFClose(tif);
    TIFFOpen_M(tif, filename, "r", __LINE__);

    if (blnSpecialTest)
        goto testcase;

    /*=== Test basic function TIFFRGBAImageGet() ===*/
    /* Full image */
    if ((ret = testRGBAImageReadFunctions(tif, width, length, width, length,
                                          orientation, req_orientation,
                                          __LINE__)))
        GOTOFAILURE

    /* More lines - result will be different for NewCode - */
    if ((ret = testRGBAImageReadFunctions(tif, width, length, width, length + 2,
                                          orientation, req_orientation,
                                          __LINE__)))
        GOTOFAILURE

    /* Less lines */
    if ((ret = testRGBAImageReadFunctions(tif, width, length, width, length - 1,
                                          orientation, req_orientation,
                                          __LINE__)))
        GOTOFAILURE

    /* Less columns */
    if ((ret = testRGBAImageReadFunctions(tif, width, length, width - 3, length,
                                          orientation, req_orientation,
                                          __LINE__)))
        GOTOFAILURE

    /* Less lines and less columns */
    if ((ret = testRGBAImageReadFunctions(tif, width, length, width - 5,
                                          length - 1, orientation,
                                          req_orientation, __LINE__)))
        GOTOFAILURE

    /* More columns. */
    if ((ret = testRGBAImageReadFunctions(tif, width, length, width + 2, length,
                                          orientation, req_orientation,
                                          __LINE__)))
        GOTOFAILURE

    /* More rows and columns. */
    if ((ret = testRGBAImageReadFunctions(tif, width, length, width + 2,
                                          length + 2, orientation,
                                          req_orientation, __LINE__)))
        GOTOFAILURE

    /*-- Test an invalid raster size --*/
    if ((ret = testRGBAImageReadFunctions(tif, width, length, width, 0,
                                          orientation, req_orientation,
                                          __LINE__)))
        GOTOFAILURE
    if ((ret = testRGBAImageReadFunctions(tif, width, length, 0, length,
                                          orientation, req_orientation,
                                          __LINE__)))
        GOTOFAILURE
    if ((ret = testRGBAImageReadFunctions(tif, width, length, 0, 0, orientation,
                                          req_orientation, __LINE__)))
        GOTOFAILURE

    /*=== Testing reading with OFFSETs in the image file; raster can then also
     * be smaller ===*/
    /*-- row_offset --*/
testcase:
    if ((ret = testRGBAImageReadWithOffsets(tif, width, length, 0, length - 1,
                                            width, length, orientation,
                                            req_orientation, __LINE__)))
        GOTOFAILURE
    if ((ret = testRGBAImageReadWithOffsets(tif, width, length, 0, length - 4,
                                            width, 4, orientation,
                                            req_orientation, __LINE__)))
        GOTOFAILURE
    if ((ret = testRGBAImageReadWithOffsets(tif, width, length, 0, length - 4,
                                            width, 2, orientation,
                                            req_orientation, __LINE__)))
        GOTOFAILURE

    /*-- col_offset --*/
    if ((ret = testRGBAImageReadWithOffsets(tif, width, length, width - 2, 0,
                                            width, length, orientation,
                                            req_orientation, __LINE__)))
        GOTOFAILURE
    if ((ret = testRGBAImageReadWithOffsets(tif, width, length, width - 2, 0, 2,
                                            length, orientation,
                                            req_orientation, __LINE__)))
        GOTOFAILURE
    if ((ret = testRGBAImageReadWithOffsets(tif, width, length, width - 3, 0, 2,
                                            length, orientation,
                                            req_orientation, __LINE__)))
        GOTOFAILURE

    /*-- row_offset and col_offset --*/
    if ((ret = testRGBAImageReadWithOffsets(
             tif, width, length, width - 4, length - 2, width, length,
             orientation, req_orientation, __LINE__)))
        GOTOFAILURE

    /* Here are some tests which are expected to fail. Suppress warning messages
     * for them. */
    blnQuiet = TRUE;
    errHandler = TIFFSetErrorHandler(NULL);

    /*-- row_offset --*/
    if (!(ret = testRGBAImageReadWithOffsets(tif, width, length, 0, length,
                                             width, length, orientation,
                                             req_orientation, __LINE__)))
        GOTOFAILURE
    if (!(ret = testRGBAImageReadWithOffsets(tif, width, length, 0, length + 5,
                                             width, length, orientation,
                                             req_orientation, __LINE__)))
        GOTOFAILURE
    if (!(ret = testRGBAImageReadWithOffsets(tif, width, length, 0, -10, width,
                                             length, orientation,
                                             req_orientation, __LINE__)))
        GOTOFAILURE

    /*-- col_offset --*/
    if (!(ret = testRGBAImageReadWithOffsets(tif, width, length, width, 0,
                                             width, length, orientation,
                                             req_orientation, __LINE__)))
        GOTOFAILURE
    if (!(ret = testRGBAImageReadWithOffsets(tif, width, length, width + 5, 0,
                                             width, length, orientation,
                                             req_orientation, __LINE__)))
        GOTOFAILURE
    if (!(ret = testRGBAImageReadWithOffsets(tif, width, length, -15, 0, width,
                                             length, orientation,
                                             req_orientation, __LINE__)))
        GOTOFAILURE

    /*-- row_offset and col_offset --*/
    if (!(ret = testRGBAImageReadWithOffsets(tif, width, length, -20, -30,
                                             width, length, orientation,
                                             req_orientation, __LINE__)))
        GOTOFAILURE

    /*-- Leaving function --*/
    if (errHandler != NULL)
        TIFFSetErrorHandler(errHandler);
    TIFFClose(tif);
#ifndef DEBUG_TESTING
    unlink(filename);
#endif
    return 0;

failure:
    if (errHandler != NULL)
        TIFFSetErrorHandler(errHandler);
    if (tif)
        TIFFClose(tif);
    return 1;
} /*-- test_ReadRGBAImage() --*/

/* Dependent on the given output- and logging- flags, the logFile is opende or a
 * new one is re-opened, and the stdXOut output is redirected from stderr to
 * file.
 */
void checkOpenLogFile(int blnReOpen)
{
    /* First open logfile, if it does not exist, otherwise leave it as is except
     * blnReOpen is set. */
    if (blnReOpen && fpLog != NULL)
    {
        fclose(fpLog);
        fpLog = NULL;
    }

    if (fpLog == NULL && logFilename != NULL)
    {
        fpLog = fopen(logFilename, "a");
        if (fpLog == NULL)
        {
            fprintf(stderr, "\nError: Could not open logfile %s.\n",
                    logFilename);
        }
    }

    /* Switch stdXOut to logFile, if ...*/
    if (blnStdOutToLogFile)
    {
        if (fpLog != NULL)
        {
            stdXOut = fpLog;
        }
    }
    if (stdXOut == NULL)
    {
        fprintf(stderr, "\nError: stdXOut is NULL - exiting.\n");
        exit(10);
    }
    return;
}

/* ============  MAIN =============== */
int main()
{
    int retval = 0;
    int retvalLast = 0;
    int ntest = 0;
    char filename[128] = {0};

    if (logFilename != NULL)
    {
        unlink(logFilename);
        if (blnMultipleLogFiles)
        {
            for (unsigned int i = 0;
                 i < (sizeof(arrLogFilenames) / sizeof(arrLogFilenames[0]));
                 i++)
            {
                unlink(arrLogFilenames[i]);
            }
        }
    }

    /* Set default output to stderr. */
    stdXOut = stderr;

    /* If fprintf shall be redirected from stderr to logFile. */
    checkOpenLogFile(FALSE);

    /* Individual single testcase. */
    if (blnSpecialTest)
    {
        retval +=
            test_ReadRGBAImage("test_RGBAImage_xxx.tif", 0, 1, 8, 4, FALSE, 1);
        return 0;
    }

    fprintf(stdXOut, "==== Testing RGBAImage... ====\n");
    if (stdXOut != stderr)
        fprintf(stderr, "==== Testing RGBAImage... ====\n");

    unsigned int openMode = 0;
    for (int tiled = 0; tiled <= 1; tiled++)
    {
        fprintf(stdXOut,
                "\n---------------------------------------------"
                "\n==== Testing %s with openMode = %s ===="
                "\n---------------------------------------------\n",
                (tiled ? "TILED" : "STRIP"), modeStrings[openMode]);
        for (planarconfig = PLANARCONFIG_CONTIG;
             planarconfig <= PLANARCONFIG_SEPARATE; planarconfig++)
        {
            if (blnMultipleLogFiles)
            {
                unsigned int n = tiled * 2 + (planarconfig - 1);
                assert(n <
                       (sizeof(arrLogFilenames) / sizeof(arrLogFilenames[0])));
                logFilename = arrLogFilenames[n];
                checkOpenLogFile(TRUE);
            }
            for (unsigned int orientation = 1; orientation < 9; orientation++)
            {
                for (unsigned int req_orientation = 1; req_orientation < 9;
                     req_orientation++)
                {
                    sprintf(filename, "test_RGBAImage_%02d_%s_%s_%s_%s-%s.tif",
                            ntest, modeStrings[openMode], (tiled ? "TL" : "ST"),
                            (planarconfig == 1 ? "CONTIG" : "SEPARATE"),
                            orientationStrings[orientation],
                            orientationStrings[req_orientation]);
                    /* clang-format off */
                retval += test_ReadRGBAImage(filename, openMode, orientation,8, 4, tiled, req_orientation);  ntest++;
                if (retval != retvalLast) { fprintf(stdXOut, "    >>>> Test %d FAILED  (openMode %s; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], tiled); retvalLast = retval; }
                retval += test_ReadRGBAImage(filename, openMode, orientation,16, 16, tiled, req_orientation);  ntest++;
                if (retval != retvalLast) { fprintf(stdXOut, "    >>>> Test %d FAILED  (openMode %s; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], tiled); retvalLast = retval; }
                retval += test_ReadRGBAImage(filename, openMode, orientation,31, 18, tiled, req_orientation);  ntest++;
                if (retval != retvalLast) { fprintf(stdXOut, "    >>>> Test %d FAILED  (openMode %s; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], tiled); retvalLast = retval; }
                retval += test_ReadRGBAImage(filename, openMode, orientation,32, 32, tiled, req_orientation);  ntest++;
                if (retval != retvalLast) { fprintf(stdXOut, "    >>>> Test %d FAILED  (openMode %s; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], tiled); retvalLast = retval; }
                    /* clang-format on */
                }
            }
        }
    }

    if (!retval)
    {
        fprintf(stdXOut, "\n==== Testing RGBAImage finished OK. ====\n");
        if (stdXOut != stderr)
            fprintf(stderr, "\n==== Testing RGBAImage finished OK. ====\n");
    }
    else
    {
        fprintf(stdXOut,
                "\n==== Testing RGBAImage finished with ERROR. ====\n");
        if (stdXOut != stderr)
            fprintf(stderr,
                    "\n==== Testing RGBAImage finished with ERROR. ====\n");
    }

    return retval;
} /*-- main() -- */
