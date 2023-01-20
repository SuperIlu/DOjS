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

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_NO_SIMD
#define STBI_NO_HDR
#define STBI_MAX_DIMENSIONS (4 * 640)
#define STBI_NO_STDIO
#define STBI_NO_THREAD_LOCALS
#include "stb_image.h"

#define JPEG_BUFFER_SIZE (4096 * 16)  //!< memory allocation increment while loading file data
#define NUM_CHANNELS 4                //!< always use RGBA

void init_jpeg(js_State *J);

/**
 * @brief load JPEG data from PACKFILE.
 *
 * @param f the PACKFILE to load from
 * @param pal pallette (is ignored)
 * @return BITMAP* or NULL if loading fails
 */
static BITMAP *load_jpg_pf(PACKFILE *f, RGB *pal) {
    BITMAP *bm = NULL;
    uint8_t *buffer = NULL;
    unsigned int malloc_size = JPEG_BUFFER_SIZE;
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
                malloc_size += JPEG_BUFFER_SIZE;                     // increase buffer size
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

    int width;
    int height;
    int channels_in_file;
    uint8_t *rgba = stbi_load_from_memory(buffer, pos, &width, &height, &channels_in_file, NUM_CHANNELS);

    DEBUGF("JPEG is %dx%dx%d\n", width, height, channels_in_file);

    // create bitmap
    bm = create_bitmap_ex(32, width, height);
    if (!bm) {
        stbi_image_free(rgba);
        return NULL;
    }

    // copy RGBA data in BITMAP
    for (int y = 0; y < height; y++) {
        unsigned int yIdx = y * NUM_CHANNELS * width;
        for (int x = 0; x < width; x++) {
            unsigned int xIdx = x * NUM_CHANNELS;
            bm->line[y][x * 4 + _rgb_r_shift_32 / 8] = rgba[0 + yIdx + xIdx];
            bm->line[y][x * 4 + _rgb_g_shift_32 / 8] = rgba[1 + yIdx + xIdx];
            bm->line[y][x * 4 + _rgb_b_shift_32 / 8] = rgba[2 + yIdx + xIdx];
            bm->line[y][x * 4 + _rgb_a_shift_32 / 8] = rgba[3 + yIdx + xIdx];
        }
    }

    stbi_image_free(rgba);
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
    return load_jpg_pf(f, 0);
}

/**
 * @brief load from file system
 *
 * @param filename the name of the file
 * @param pal pallette (is ignored)
 * @return BITMAP* or NULL if loading fails
 */
static BITMAP *load_jpg(AL_CONST char *filename, RGB *pal) {
    BITMAP *b;
    PACKFILE *f;

    ASSERT(filename);

    f = pack_fopen(filename, F_READ);
    if (!f) {
        return 0;
    }

    b = load_jpg_pf(f, pal);

    pack_fclose(f);
    return b;
}

/*********************
** public functions **
*********************/
/**
 * @brief initialize subsystem.
 *
 * @param J VM state.
 */
void init_jpeg(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    register_bitmap_file_type("jpg", load_jpg, NULL, load_jpg_pf);
    register_datafile_object(DAT_ID('J', 'P', 'G', ' '), load_from_datafile, (void (*)(void *))destroy_bitmap);
}
