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
#include <time.h>
#include <setjmp.h>

#include "DOjS.h"
#include "bitmap.h"

#define QOI_IMPLEMENTATION
#include "qoi.h"

#define QOI_BUFFER_SIZE (4096 * 16)  //!< memory allocation increment while loading file data
#define NUM_CHANNELS 4               //!< always use RGBA

void init_qoi(js_State *J);

/**
 * @brief load QOI data from PACKFILE.
 *
 * @param f the PACKFILE to load from
 * @param pal pallette (is ignored)
 * @return BITMAP* or NULL if loading fails
 */
static BITMAP *load_qoi_pf(PACKFILE *f, RGB *pal) {
    BITMAP *bm = NULL;
    uint8_t *buffer = NULL;
    unsigned int malloc_size = QOI_BUFFER_SIZE;
    unsigned int pos = 0;

    // try loading the whole JPEG to memory, Allegro4 PACKFILES have no way of determining file size, so we need this ugly hack!
    buffer = realloc(buffer, malloc_size);
    if (!buffer) {
        return NULL;
    }
    DEBUGF("realloc : mem_size = %d, file_size = %d\n", malloc_size, pos);
    while (true) {
        int ch = pack_getc(f);
        if (ch == EOF) {
            break;  // reading done
        } else {
            buffer[pos++] = ch;                                      // store byte
            if (pos >= malloc_size) {                                // check buffer bounds
                malloc_size += QOI_BUFFER_SIZE;                      // increase buffer size
                uint8_t *new_buffer = realloc(buffer, malloc_size);  // try re-alloc
                if (!new_buffer) {                                   // bail out on failure
                    free(buffer);
                    return NULL;
                } else {
                    buffer = new_buffer;
                }
                DEBUGF("realloc : mem_size = %d, file_size = %d\n", malloc_size, pos);
            }
        }
    }
    DEBUGF("final   : mem_size = %d, file_size = %d\n", malloc_size, pos);

    qoi_desc desc;
    uint8_t *rgba = qoi_decode(buffer, pos, &desc, NUM_CHANNELS);

    DEBUGF("QOI is %dx%d\n", desc.width, desc.height);

    // create bitmap
    bm = create_bitmap_ex(32, desc.width, desc.height);
    if (!bm) {
        QOI_FREE(rgba);
        return NULL;
    }

    // copy RGBA data in BITMAP
    for (int y = 0; y < desc.height; y++) {
        unsigned int yIdx = y * NUM_CHANNELS * desc.width;
        for (int x = 0; x < desc.width; x++) {
            unsigned int xIdx = x * NUM_CHANNELS;
            bm->line[y][x * 4 + _rgb_r_shift_32 / 8] = rgba[0 + yIdx + xIdx];
            bm->line[y][x * 4 + _rgb_g_shift_32 / 8] = rgba[1 + yIdx + xIdx];
            bm->line[y][x * 4 + _rgb_b_shift_32 / 8] = rgba[2 + yIdx + xIdx];
            bm->line[y][x * 4 + _rgb_a_shift_32 / 8] = rgba[3 + yIdx + xIdx];
        }
    }

    QOI_FREE(rgba);
    free(buffer);

    return bm;
}

/**
 * @brief loader for datafiles
 *
 * @param f the PACKFILE to load from
 * @param size size is ignored
 * @return BITMAP* or NULL if loading fails
 */
static void *load_from_datafile(PACKFILE *f, long size) {
    (void)size;
    return load_qoi_pf(f, 0);
}

/**
 * @brief load from file system
 *
 * @param filename the name of the file
 * @param pal pallette (is ignored)
 * @return BITMAP* or NULL if loading fails
 */
static BITMAP *load_qoi(AL_CONST char *filename, RGB *pal) {
    BITMAP *b;
    PACKFILE *f;

    ASSERT(filename);

    f = pack_fopen(filename, F_READ);
    if (!f) {
        return 0;
    }

    b = load_qoi_pf(f, pal);

    pack_fclose(f);
    return b;
}

/**
 * @brief convert BITMAP to rgba buffer and save as QOI
 *
 * @param bm BITMAP
 * @param fname file name
 * @return true for success, else false
 */
static bool save_qoi(BITMAP *bm, const char *fname) {
    bool ret = false;

    uint8_t *rgba = malloc(bm->w * bm->h * NUM_CHANNELS);
    if (!rgba) {
        return ret;
    }

    uint8_t *ptr = rgba;
    for (int y = 0; y < bm->h; y++) {
        for (int x = 0; x < bm->w; x++) {
            uint32_t argb = getpixel(bm, x, y);
            uint8_t a = (argb >> 24) & 0xFF;
            uint8_t r = (argb >> 16) & 0xFF;
            uint8_t g = (argb >> 8) & 0xFF;
            uint8_t b = argb & 0xFF;

            *ptr++ = r;
            *ptr++ = g;
            *ptr++ = b;
            *ptr++ = a;
        }
    }

    ret = qoi_write(fname, rgba, &(qoi_desc){.width = bm->w, .height = bm->h, .channels = NUM_CHANNELS, .colorspace = QOI_SRGB}) != 0;

    free(rgba);
    return ret;
}

/**
 * @brief save current screen to file.
 * SavePngImage(fname:string)
 *
 * @param J the JS context.
 */
static void f_SaveQoiImage(js_State *J) {
    BITMAP *bm = DOjS.current_bm;
    const char *fname = js_tostring(J, 1);

    if (!save_qoi(bm, fname)) {
        js_error(J, "Can't save screen to QOI file '%s'", fname);
    }
}

/**
 * @brief save Bitmap to file.
 * SavePngImage(fname:string)
 *
 * @param J the JS context.
 */
static void Bitmap_SaveQoiImage(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    if (!save_qoi(bm, fname)) {
        js_error(J, "Can't save Bitmap to QOI file '%s'", fname);
    }
}

/*********************
** public functions **
*********************/
/**
 * @brief initialize subsystem.
 *
 * @param J VM state.
 */
void init_qoi(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    register_bitmap_file_type("qoi", load_qoi, NULL, load_qoi_pf);
    register_datafile_object(DAT_ID('Q', 'O', 'I', ' '), load_from_datafile, (void (*)(void *))destroy_bitmap);

    NFUNCDEF(J, SaveQoiImage, 1);

    js_getglobal(J, TAG_BITMAP);
    js_getproperty(J, -1, "prototype");
    js_newcfunction(J, Bitmap_SaveQoiImage, "Bitmap.prototype.SaveQoiImage", 1);
    js_defproperty(J, -2, "SaveQoiImage", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    NPROTDEF(J, Bitmap, SaveQoiImage, 1);
}
