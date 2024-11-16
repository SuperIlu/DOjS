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

#include "jasper/jasper.h"

#define JASPER_BUFFER_SIZE (4096 * 16)  //!< memory allocation increment while loading file data

void init_jasper(js_State *J);

#ifdef DEBUG_ENABLED
static int jp2_vlogmsgf_stdout(jas_logtype_t type, const char *fmt, va_list ap) {
    JAS_UNUSED(type);
    int result = vfprintf(stdout, fmt, ap);
    return result;
}
#endif

/**
 * @brief load QOI data from PACKFILE.
 *
 * @param f the PACKFILE to load from
 * @param pal pallette (is ignored)
 * @return BITMAP* or NULL if loading fails
 */
#if LINUX == 1
BITMAP *load_jasper_pf(PACKFILE *f, RGB *pal) {
#else
static BITMAP *load_jasper_pf(PACKFILE *f, RGB *pal) {
#endif
    uint8_t *buffer = NULL;
    unsigned int malloc_size = JASPER_BUFFER_SIZE;
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
                malloc_size += JASPER_BUFFER_SIZE;                   // increase buffer size
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

    // init jasper
    jas_conf_clear();
    static jas_std_allocator_t allocator;
    jas_std_allocator_init(&allocator);
    jas_conf_set_allocator(&allocator.base);
    jas_conf_set_debug_level(0);
    jas_conf_set_max_mem_usage(JAS_DEFAULT_MAX_MEM_USAGE);

#ifdef DEBUG_ENABLED
    jas_conf_set_debug_level(99);
    jas_conf_set_vlogmsgf(jp2_vlogmsgf_stdout);
#endif

    if (jas_init_library()) {
        DEBUGF("cannot initialize JasPer library\n");
        free(buffer);
        return NULL;
    }
    if (jas_init_thread()) {
        DEBUGF("cannot initialize thread\n");
        jas_cleanup_library();
        free(buffer);
        return NULL;
    }

    DEBUGF("LOAD: jasper initialized\n");

    // open image stream
    jas_stream_t *in;
    if (!(in = jas_stream_memopen((char *)buffer, pos))) {
        DEBUGF("error: cannot open input image\n");
        jas_cleanup_thread();
        jas_cleanup_library();
        free(buffer);
        return NULL;
    }

    // load image data
    jas_image_t *image;
    if (!(image = jas_image_decode(in, -1, ""))) {
        DEBUGF("error: cannot load image data\n");
        jas_cleanup_thread();
        jas_cleanup_library();
        free(buffer);
        return NULL;
    }
    jas_stream_close(in);

    int components = jas_image_numcmpts(image);
    DEBUGF("num components = %d\n", components);

    if (components != 3) {
        DEBUGF("error: wrong number of components: %d\n", components);
        jas_cleanup_thread();
        jas_cleanup_library();
        free(buffer);
        return NULL;
    }

    int width = jas_image_width(image);
    int height = jas_image_height(image);

    DEBUGF("width  = %d\n", width);
    DEBUGF("height = %d\n", height);

    // convert colors
    jas_cmprof_t *outprof = NULL;
    jas_image_t *altimage;
    if (!(outprof = jas_cmprof_createfromclrspc(JAS_CLRSPC_SRGB))) {
        DEBUGF("Can't change colorspace 1\n");
        jas_image_destroy(image);
        jas_cleanup_thread();
        jas_cleanup_library();
        free(buffer);
        return NULL;
    };
    if (!(altimage = jas_image_chclrspc(image, outprof, JAS_CMXFORM_INTENT_PER))) {
        DEBUGF("Can't change colorspace 2\n");
        jas_cmprof_destroy(outprof);
        jas_image_destroy(image);
        jas_cleanup_thread();
        jas_cleanup_library();
        free(buffer);
        return NULL;
    };
    jas_cmprof_destroy(outprof);

    // create bitmap
    BITMAP *bm = create_bitmap_ex(32, width, height);
    if (!bm) {
        DEBUGF("Can't create bitmap\n");
        jas_image_destroy(image);
        jas_image_destroy(altimage);
        jas_cleanup_thread();
        jas_cleanup_library();
        free(buffer);
        return NULL;
    }
    DEBUGF("bm = %p\n", bm);

    int comp_r, comp_g, comp_b;
    if ((comp_r = jas_image_getcmptbytype(altimage, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R))) < 0 ||
        (comp_g = jas_image_getcmptbytype(altimage, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G))) < 0 ||
        (comp_b = jas_image_getcmptbytype(altimage, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B))) < 0) {
        DEBUGF("Can't create components\n");
        jas_image_destroy(image);
        jas_image_destroy(altimage);
        jas_cleanup_thread();
        jas_cleanup_library();
        destroy_bitmap(bm);
        free(buffer);
        return NULL;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bm->line[y][x * 4 + _rgb_r_shift_32 / 8] = jas_image_readcmptsample(image, comp_r, x, y);
            bm->line[y][x * 4 + _rgb_g_shift_32 / 8] = jas_image_readcmptsample(image, comp_g, x, y);
            bm->line[y][x * 4 + _rgb_b_shift_32 / 8] = jas_image_readcmptsample(image, comp_b, x, y);
            bm->line[y][x * 4 + _rgb_a_shift_32 / 8] = 0xFF;
        }
    }

    jas_image_destroy(image);
    jas_image_destroy(altimage);
    jas_cleanup_thread();
    jas_cleanup_library();

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
    return load_jasper_pf(f, 0);
}

/**
 * @brief load from file system
 *
 * @param filename the name of the file
 * @param pal pallette (is ignored)
 * @return BITMAP* or NULL if loading fails
 */
static BITMAP *load_jasper(AL_CONST char *filename, RGB *pal) {
    BITMAP *b;
    PACKFILE *f;

    ASSERT(filename);

    f = pack_fopen(filename, F_READ);
    if (!f) {
        return 0;
    }

    b = load_jasper_pf(f, pal);

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
#if LINUX == 1
bool save_jasper(BITMAP *bm, const char *fname, const char *format, int quality) {
#else
static bool save_jasper(BITMAP *bm, const char *fname, const char *format, int quality) {
#endif
    if (quality < 1) {
        quality = 1;
    }
    if (quality > 100) {
        quality = 100;
    }

    char outopts[100];
    snprintf(outopts, sizeof(outopts), "rate=%f", (float)quality / 100.0f);

    DEBUGF("Encoder options: %s\n", outopts);

    // init jasper
    jas_conf_clear();
    static jas_std_allocator_t allocator;
    jas_std_allocator_init(&allocator);
    jas_conf_set_allocator(&allocator.base);
    jas_conf_set_debug_level(0);
    jas_conf_set_max_mem_usage(JAS_DEFAULT_MAX_MEM_USAGE);

#ifdef DEBUG_ENABLED
    jas_conf_set_debug_level(99);
    jas_conf_set_vlogmsgf(jp2_vlogmsgf_stdout);
#else
    jas_conf_set_vlogmsgf(jas_vlogmsgf_discard);
#endif

    if (jas_init_library()) {
        DEBUGF("cannot initialize JasPer library\n");
        return -1;
    }
    if (jas_init_thread()) {
        DEBUGF("cannot initialize thread\n");
        jas_cleanup_library();
        return -1;
    }

    DEBUGF("WRITE: jasper initialized\n");

    int outfmt;
    if ((outfmt = jas_image_strtofmt(format)) < 0) {
        DEBUGF("Unknown format for %s\n", format);
        jas_cleanup_thread();
        jas_cleanup_library();
        return false;
    }

    jas_stream_t *out;
    if (!(out = jas_stream_fopen(fname, "w+b"))) {
        DEBUGF("error: cannot open output image file %s\n", fname);
        jas_cleanup_thread();
        jas_cleanup_library();
        return false;
    }

    /* Create an image of the correct size. */
    const int NUM_COMPONENTS = 3;
    jas_image_t *image;
    jas_image_cmptparm_t cmptparms[3];
    for (int i = 0; i < NUM_COMPONENTS; ++i) {
        cmptparms[i].tlx = 0;
        cmptparms[i].tly = 0;
        cmptparms[i].hstep = 1;
        cmptparms[i].vstep = 1;
        cmptparms[i].width = bm->w;
        cmptparms[i].height = bm->h;
        cmptparms[i].prec = 8;
        cmptparms[i].sgnd = false;
    }
    if (!(image = jas_image_create(NUM_COMPONENTS, cmptparms, JAS_CLRSPC_UNKNOWN))) {
        DEBUGF("error: cannot create image\n");
        jas_cleanup_thread();
        jas_cleanup_library();
        return false;
    }

    jas_image_setclrspc(image, JAS_CLRSPC_SRGB);
    jas_image_setcmpttype(image, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
    jas_image_setcmpttype(image, 1, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
    jas_image_setcmpttype(image, 2, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));

    jas_matrix_t *data[3];
    for (int cmptno = 0; cmptno < NUM_COMPONENTS; ++cmptno) {
        if (!(data[cmptno] = jas_matrix_create(1, bm->w))) {
            DEBUGF("error: cannot create matrix\n");
            jas_image_destroy(image);
            jas_cleanup_thread();
            jas_cleanup_library();
            return false;
        }
    }

    for (int y = 0; y < bm->h; ++y) {
        for (int x = 0; x < bm->w; ++x) {
            uint32_t argb = getpixel(bm, x, y);
            int_fast64_t r = (argb >> 16) & 0xFF;
            int_fast64_t g = (argb >> 8) & 0xFF;
            int_fast64_t b = argb & 0xFF;

            jas_matrix_set(data[0], 0, x, r);
            jas_matrix_set(data[1], 0, x, g);
            jas_matrix_set(data[2], 0, x, b);
        }

        for (int cmptno = 0; cmptno < NUM_COMPONENTS; ++cmptno) {
            if (jas_image_writecmpt(image, cmptno, 0, y, bm->w, 1, data[cmptno])) {
                DEBUGF("error: cannot write component\n");
                for (int cmptno = 0; cmptno < NUM_COMPONENTS; ++cmptno) {
                    jas_matrix_destroy(data[cmptno]);
                }
                jas_image_destroy(image);
                jas_stream_close(out);
                jas_cleanup_thread();
                jas_cleanup_library();
                return false;
            }
        }
    }

    if (jas_image_encode(image, out, outfmt, outopts)) {
        DEBUGF("error: cannot encode image\n");
        for (int cmptno = 0; cmptno < NUM_COMPONENTS; ++cmptno) {
            jas_matrix_destroy(data[cmptno]);
        }

        jas_image_destroy(image);
        jas_stream_close(out);
        jas_cleanup_thread();
        jas_cleanup_library();
        return false;
    }
    jas_stream_flush(out);
    jas_image_destroy(image);

    for (int cmptno = 0; cmptno < NUM_COMPONENTS; ++cmptno) {
        jas_matrix_destroy(data[cmptno]);
    }

    /* Close the output image stream. */
    if (jas_stream_close(out)) {
        DEBUGF("error: cannot close output image file\n");
        jas_cleanup_thread();
        jas_cleanup_library();
        return false;
    }

    jas_cleanup_thread();
    jas_cleanup_library();

    return true;
}

/**
 * @brief save current screen to file.
 * SaveJp2Image(fname:string, quality:number)
 *
 * @param J the JS context.
 */
static void f_SaveJp2Image(js_State *J) {
    BITMAP *bm = DOjS.current_bm;
    const char *fname = js_tostring(J, 1);

    int quality = 95;
    if (js_isnumber(J, 2)) {
        quality = js_touint16(J, 2);
    }

    if (!save_jasper(bm, fname, "jp2", quality)) {
        js_error(J, "Can't save screen to JP2 file '%s'", fname);
    }
}

/**
 * @brief save current screen to file.
 * SaveRasImage(fname:string)
 *
 * @param J the JS context.
 */
static void f_SaveRasImage(js_State *J) {
    BITMAP *bm = DOjS.current_bm;
    const char *fname = js_tostring(J, 1);

    if (!save_jasper(bm, fname, "ras", 100)) {
        js_error(J, "Can't save screen to RAS file '%s'", fname);
    }
}

#if LINUX != 1
/**
 * @brief save Bitmap to file.
 * SaveJp2Image(fname:string, quality:number)
 *
 * @param J the JS context.
 */
static void Bitmap_SaveJp2Image(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    int quality = 95;
    if (js_isnumber(J, 2)) {
        quality = js_touint16(J, 2);
    }

    if (!save_jasper(bm, fname, "jp2", quality)) {
        js_error(J, "Can't save Bitmap to JP2 file '%s'", fname);
    }
}

/**
 * @brief save Bitmap to file.
 * SaveRasImage(fname:string)
 *
 * @param J the JS context.
 */
static void Bitmap_SaveRasImage(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    if (!save_jasper(bm, fname, "ras", 100)) {
        js_error(J, "Can't save Bitmap to RAS file '%s'", fname);
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
void init_jasper(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

#if LINUX == 1
    register_bitmap_file_type("jp2", load_jasper, save_jasper, NULL);
    register_bitmap_file_type("ras", load_jasper, save_jasper, NULL);
#else
    register_bitmap_file_type("jp2", load_jasper, NULL, load_jasper_pf);
    register_bitmap_file_type("ras", load_jasper, NULL, load_jasper_pf);
#endif
    register_datafile_object(DAT_ID('J', 'P', '2', ' '), load_from_datafile, (void (*)(void *))destroy_bitmap);
    register_datafile_object(DAT_ID('R', 'A', 'S', ' '), load_from_datafile, (void (*)(void *))destroy_bitmap);

    NFUNCDEF(J, SaveJp2Image, 2);
    NFUNCDEF(J, SaveRasImage, 1);

#if LINUX != 1
    js_getglobal(J, TAG_BITMAP);
    js_getproperty(J, -1, "prototype");
    js_newcfunction(J, Bitmap_SaveJp2Image, "Bitmap.prototype.SaveJp2Image", 2);
    js_defproperty(J, -2, "SaveJp2Image", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
    NPROTDEF(J, Bitmap, SaveJp2Image, 2);

    js_getglobal(J, TAG_BITMAP);
    js_getproperty(J, -1, "prototype");
    js_newcfunction(J, Bitmap_SaveRasImage, "Bitmap.prototype.SaveRasImage", 1);
    js_defproperty(J, -2, "SaveRasImage", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
    NPROTDEF(J, Bitmap, SaveRasImage, 1);
#endif
}
