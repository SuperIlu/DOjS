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

#include "DOjS.h"
#include "bitmap.h"
#include "webp.h"

#include "webp/decode.h"
#include "webp/encode.h"

void init_webp(js_State *J);

#define WEBP_BUFFER_SIZE (4096 * 16)  //!< memory allocation increment while loading file data
#define NUM_CHANNELS 4                //!< always use RGBA

/**
 * @brief load WEBP data from PACKFILE.
 *
 * @param f the PACKFILE to load from
 * @param pal pallette (is ignored)
 *
 * @return BITMAP* or NULL if loading fails
 */
#if LINUX == 1
BITMAP *load_webp_pf(PACKFILE *f, RGB *pal) {
#else
static BITMAP *load_webp_pf(PACKFILE *f, RGB *pal) {
#endif
    BITMAP *bm = NULL;
    uint8_t *buffer = NULL;
    unsigned int malloc_size = WEBP_BUFFER_SIZE;
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
                malloc_size += WEBP_BUFFER_SIZE;                     // increase buffer size
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

    int width, height;
    uint8_t *rgba = WebPDecodeRGBA(buffer, pos, &width, &height);
    if (rgba) {
        DEBUGF("WEBP is %dx%d\n", width, height);
        // create bitmap
        bm = create_bitmap_ex(32, width, height);
        if (!bm) {
            WebPFree(rgba);
            free(buffer);
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

        WebPFree(rgba);
        free(buffer);

        return bm;
    } else {
        free(buffer);
        return NULL;
    }
}

/**
 * @brief loader for datafiles
 *
 * @param f the PACKFILE to load from
 * @param size size is ignored
 *
 * @return BITMAP* or NULL if loading fails
 */
static void *load_from_datafile(PACKFILE *f, long size) {
    (void)size;
    return load_webp_pf(f, 0);
}

/**
 * @brief load from file system
 *
 * @param filename the name of the file
 * @param pal pallette (is ignored)
 *
 * @return BITMAP* or NULL if loading fails
 */
static BITMAP *load_webp(AL_CONST char *filename, RGB *pal) {
    BITMAP *b;
    PACKFILE *f;

    ASSERT(filename);

    f = pack_fopen(filename, F_READ);
    if (!f) {
        return 0;
    }

    b = load_webp_pf(f, pal);

    pack_fclose(f);
    return b;
}

/**
 * @brief convert BITMAP to rgba buffer and save as losless webp
 *
 * @param bm BITMAP
 * @param fname file name
 *
 * @return true for success, else false
 */
#if LINUX == 1
bool save_webp(BITMAP *bm, const char *fname, int quality) {
#else
static bool save_webp(BITMAP *bm, const char *fname, int quality) {
#endif
    bool ret = false;

    if (quality < 10) {
        quality = 10;
    }
    if (quality > 100) {
        quality = 100;
    }

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

    uint8_t *output;
    // size_t size = WebPEncodeLosslessRGBA(rgba, bm->w, bm->h, bm->w * NUM_CHANNELS, &output); // this uses to much memory!
    size_t size = WebPEncodeRGBA(rgba, bm->w, bm->h, bm->w * NUM_CHANNELS, quality, &output);
    if (size > 0) {
        FILE *out = fopen(fname, "wb");
        if (out) {
            size_t num_written = fwrite(output, 1, size, out);
            ret = num_written != size;
            fclose(out);
        } else {
            DEBUGF("Could not create %s\n", fname);
            ret = false;
        }
        WebPFree(output);
    } else {
        ret = false;
    }

    free(rgba);
    return size > 0;
}

/**
 * @brief save current screen to file.
 * SaveWebpImage(fname:string, quality:number)
 *
 * @param J the JS context.
 */
static void f_SaveWebpImage(js_State *J) {
    BITMAP *bm = DOjS.current_bm;
    const char *fname = js_tostring(J, 1);

    int quality = 95;
    if (js_isnumber(J, 2)) {
        quality = js_touint16(J, 2);
    }

    if (!save_webp(bm, fname, quality)) {
        js_error(J, "Can't save screen to WEBP file '%s'", fname);
    }
}

#if LINUX != 1
/**
 * @brief save Bitmap to file.
 * SaveWebpImage(fname:string, quality:number)
 *
 * @param J the JS context.
 */
static void Bitmap_SaveWebpImage(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    int quality = 95;
    if (js_isnumber(J, 2)) {
        quality = js_touint16(J, 2);
    }

    if (!save_webp(bm, fname, quality)) {
        js_error(J, "Can't save Bitmap to WEBP file '%s'", fname);
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
void init_webp(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

#if LINUX == 1
    register_bitmap_file_type("webp", load_webp, NULL);
    register_bitmap_file_type("web", load_webp, NULL);
    register_bitmap_file_type("wep", load_webp, NULL);
#else
    register_bitmap_file_type("webp", load_webp, NULL, load_webp_pf);
    register_bitmap_file_type("web", load_webp, NULL, load_webp_pf);
    register_bitmap_file_type("wep", load_webp, NULL, load_webp_pf);
#endif
    register_datafile_object(DAT_ID('W', 'E', 'P', ' '), load_from_datafile, (void (*)(void *))destroy_bitmap);
    register_datafile_object(DAT_ID('W', 'E', 'B', ' '), load_from_datafile, (void (*)(void *))destroy_bitmap);
    register_datafile_object(DAT_ID('W', 'E', 'B', 'P'), load_from_datafile, (void (*)(void *))destroy_bitmap);

    NFUNCDEF(J, SaveWebpImage, 2);

#if LINUX != 1
    js_getglobal(J, TAG_BITMAP);
    js_getproperty(J, -1, "prototype");
    js_newcfunction(J, Bitmap_SaveWebpImage, "Bitmap.prototype.SaveWebpImage", 2);
    js_defproperty(J, -2, "SaveWebpImage", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    NPROTDEF(J, Bitmap, SaveWebpImage, 2);
#endif
}
