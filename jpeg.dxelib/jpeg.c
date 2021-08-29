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

#include "DOjS.h"

#define _NJ_INCLUDE_HEADER_ONLY
#include "nanojpeg.c"

#define JPEG_BUFFER_SIZE (4096 * 4)  //!< memory allocation increment while loading file data

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
    char *buffer = NULL;
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
            buffer[pos++] = ch;                         // store byte
            if (pos >= malloc_size) {                   // check buffer bounds
                malloc_size += JPEG_BUFFER_SIZE;        // increase buffer size
                buffer = realloc(buffer, malloc_size);  // try re-alloc
                if (!buffer) {                          // bail out on failure
                    return NULL;
                }
                DEBUGF("realloc : mem_size = %d, file_size = %d\n", malloc_size, pos);
            }
        }
    }
    DEBUGF("final   : mem_size = %d, file_size = %d\n", malloc_size, pos);

    njInit();
    if (njDecode(buffer, pos) == NJ_OK) {
        DEBUGF("Decode OK, size %dx%d\n", njGetWidth(), njGetHeight());
        bm = create_bitmap_ex(32, njGetWidth(), njGetHeight());
        unsigned char *jpgdata = njGetImage();

        if (njIsColor()) {
            LOG("Color JPEG\n");
            for (int y = 0; y < bm->h; y++) {
                for (int x = 0; x < bm->w; x++) {
                    int offs = y * bm->w * 3 + x * 3;
                    bm->line[y][x * 4 + _rgb_r_shift_32 / 8] = jpgdata[0 + offs];
                    bm->line[y][x * 4 + _rgb_g_shift_32 / 8] = jpgdata[1 + offs];
                    bm->line[y][x * 4 + _rgb_b_shift_32 / 8] = jpgdata[2 + offs];
                    bm->line[y][x * 4 + _rgb_a_shift_32 / 8] = 0xFF;
                }
            }
        } else {
            DEBUGF("BW JPEG\n");
            for (int y = 0; y < bm->h; y++) {
                for (int x = 0; x < bm->w; x++) {
                    int offs = y * bm->w + x;
                    bm->line[y][x * 4 + _rgb_r_shift_32 / 8] = jpgdata[offs];
                    bm->line[y][x * 4 + _rgb_g_shift_32 / 8] = jpgdata[offs];
                    bm->line[y][x * 4 + _rgb_b_shift_32 / 8] = jpgdata[offs];
                    bm->line[y][x * 4 + _rgb_a_shift_32 / 8] = 0xFF;
                }
            }
        }
    }
    free(buffer);
    njDone();

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
 * @brief initialize neural subsystem.
 *
 * @param J VM state.
 */
void init_jpeg(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    register_bitmap_file_type("jpg", load_jpg, NULL, load_jpg_pf);
    register_datafile_object(DAT_ID('J', 'P', 'G', ' '), load_from_datafile, (void (*)(void *))destroy_bitmap);
}
