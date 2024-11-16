/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <errno.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tiffio.h"

#include "DOjS.h"
#include "bitmap.h"

#define NUM_CHANNELS 4  //!< always use RGBA

void init_tiff(js_State *J);

/**
 * @brief load from file system
 *
 * @param filename the name of the file
 * @param pal pallette (is ignored)
 * @return BITMAP* or NULL if loading fails
 */
static BITMAP *load_tiff(AL_CONST char *filename, RGB *pal) {
    ASSERT(filename);

    TIFF *tif = TIFFOpen(filename, "r");
    DEBUGF("TIFF = %p\n", tif);
    if (tif) {
        uint32_t w, h;
        size_t npixels;
        uint32_t *raster;

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

        DEBUGF("TIFF is %ldx%ld\n", w, h);

        // create bitmap
        BITMAP *bm = create_bitmap_ex(32, w, h);
        if (!bm) {
            TIFFClose(tif);
            return NULL;
        }
        DEBUGF("bm = %p\n", bm);

        npixels = w * h;
        raster = (uint32_t *)_TIFFmalloc(npixels * sizeof(uint32_t));
        DEBUGF("raster = %p\n", raster);
        if (raster != NULL) {
            if (TIFFReadRGBAImage(tif, w, h, raster, 0)) {
                // copy RGBA data in BITMAP
                uint8_t *rgba = (uint8_t *)raster;
                for (int y = 0; y < h; y++) {
                    unsigned int yIdx = (h - y) * NUM_CHANNELS * w;  // TIFF is loaded bottom up
                    for (int x = 0; x < w; x++) {
                        unsigned int xIdx = x * NUM_CHANNELS;
                        bm->line[y][x * 4 + _rgb_r_shift_32 / 8] = rgba[0 + yIdx + xIdx];
                        bm->line[y][x * 4 + _rgb_g_shift_32 / 8] = rgba[1 + yIdx + xIdx];
                        bm->line[y][x * 4 + _rgb_b_shift_32 / 8] = rgba[2 + yIdx + xIdx];
                        bm->line[y][x * 4 + _rgb_a_shift_32 / 8] = rgba[3 + yIdx + xIdx];
                    }
                }
            } else {
                _TIFFfree(raster);
                destroy_bitmap(bm);
                TIFFClose(tif);
                return NULL;
            }
            _TIFFfree(raster);
        } else {
            destroy_bitmap(bm);
            TIFFClose(tif);
            return NULL;
        }
        TIFFClose(tif);

        return bm;
    } else {
        return NULL;
    }
}

/**
 * @brief convert BITMAP to rgba buffer and save as TIFF
 *
 * @param bm BITMAP
 * @param fname file name
 * @return true for success, else false
 */
#if LINUX == 1
bool save_tiff(BITMAP *bm, const char *fname) {
#else
static bool save_tiff(BITMAP *bm, const char *fname) {
#endif
    bool ret = false;
    TIFF *out = TIFFOpen(fname, "w");

    if (!out) {
        return ret;
    }

    // Now we need to set the tags in the new image file, and the essential ones are the following:
    TIFFSetField(out, TIFFTAG_IMAGEWIDTH, bm->w);                 // set the width of the image
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, bm->h);                // set the height of the image
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, NUM_CHANNELS);     // set number of channels per pixel
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);                  // set the size of the channels
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);  // set the origin of the image.
    //   Some other essential fields to set that you do not have to understand for now.
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);

    // We will use most basic image data storing method provided by the library to write the data into the file, this method uses strips, and we are storing a line (row) of pixel
    // at a time.  This following code writes the data from the char array image into the file:
    // size_t linebytes = NUM_CHANNELS * bm->w;  // length in memory of one row of pixel in the image.
    // DEBUGF("linebytes=%ld, scanlinesize=%ld\n", linebytes, TIFFScanlineSize(out));

    unsigned char *buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(out));  // buffer used to store the row of pixel information for writing to file

    // We set the strip size of the file to be size of one row of pixels
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, bm->w * NUM_CHANNELS));

    // Now writing image to the file one strip at a time
    for (uint32_t row = 0; row < bm->h; row++) {
        uint8_t *ptr = buf;
        // int y = (bm->h - row - 1);
        int y = row;
        for (int x = 0; x < bm->w; x++) {
            uint32_t argb = getpixel(bm, x, y);
            uint8_t a = 0xFF;
            uint8_t r = (argb >> 16) & 0xFF;
            uint8_t g = (argb >> 8) & 0xFF;
            uint8_t b = argb & 0xFF;

            *ptr++ = r;
            *ptr++ = g;
            *ptr++ = b;
            *ptr++ = a;
        }
        if (TIFFWriteScanline(out, buf, row, 0) < 0) {
            ret = false;
            break;
        } else {
            ret = true;
        }
    }

    // Finally we close the output file, and destroy the buffer
    (void)TIFFClose(out);
    _TIFFfree(buf);

    return ret;
}

/**
 * @brief save current screen to file.
 * SaveTiffImage(fname:string)
 *
 * @param J the JS context.
 */
static void f_SaveTiffImage(js_State *J) {
    BITMAP *bm = DOjS.current_bm;
    const char *fname = js_tostring(J, 1);

    if (!save_tiff(bm, fname)) {
        js_error(J, "Can't save screen to TIFF file '%s'", fname);
    }
}

#if LINUX != 1
/**
 * @brief save Bitmap to file.
 * SaveTiffImage(fname:string)
 *
 * @param J the JS context.
 */
static void Bitmap_SaveTiffImage(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    if (!save_tiff(bm, fname)) {
        js_error(J, "Can't save Bitmap to TIFF file '%s'", fname);
    }
}
#endif

/*********************
** public functions **
*********************/
/**
 * @brief initialize subsystem.
 *
 * @param J VM state.
 */
void init_tiff(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

#if LINUX == 1
    register_bitmap_file_type("tif", load_tiff, NULL);
#else
    register_bitmap_file_type("tif", load_tiff, NULL, NULL);
#endif

    NFUNCDEF(J, SaveTiffImage, 1);

#if LINUX != 1
    js_getglobal(J, TAG_BITMAP);
    js_getproperty(J, -1, "prototype");
    js_newcfunction(J, Bitmap_SaveTiffImage, "Bitmap.prototype.SaveTiffImage", 1);
    js_defproperty(J, -2, "SaveTiffImage", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    NPROTDEF(J, Bitmap, SaveTiffImage, 1);
#endif
}
