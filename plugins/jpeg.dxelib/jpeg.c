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
#include "jpeg.h"

#include "DOjS.h"
#include "bitmap.h"

#define JPG_BUFFER_SIZE (4096 * 16)  //!< memory allocation increment while loading file data

/*
 * Include file for users of JPEG library.
 * You will need to have included system headers that define at least
 * the typedefs FILE and size_t before you can include jpeglib.h.
 * (stdio.h is sufficient on ANSI-conforming systems.)
 * You may also wish to include "jerror.h".
 */

#include "jpeglib.h"

/*
 * <setjmp.h> is used for the optional error recovery mechanism shown in
 * the second part of the example.
 */

#include <setjmp.h>

/*
 * ERROR HANDLING:
 *
 * The JPEG library's standard error handler (jerror.c) is divided into
 * several "methods" which you can override individually.  This lets you
 * adjust the behavior without duplicating a lot of code, which you might
 * have to update with each future release.
 *
 * Our example here shows how to override the "error_exit" method so that
 * control is returned to the library's caller when a fatal error occurs,
 * rather than calling exit() as the standard error_exit method does.
 *
 * We use C's setjmp/longjmp facility to return control.  This means that the
 * routine which calls the JPEG library must first execute a setjmp() call to
 * establish the return point.  We want the replacement error_exit to do a
 * longjmp().  But we need to make the setjmp buffer accessible to the
 * error_exit routine.  To do this, we make a private extension of the
 * standard JPEG error handler object.  (If we were using C++, we'd say we
 * were making a subclass of the regular error handler.)
 *
 * Here's the extended error handler struct:
 */

struct my_error_mgr {
    struct jpeg_error_mgr pub; /* "public" fields */

    jmp_buf setjmp_buffer; /* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit(j_common_ptr cinfo) {
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    my_error_ptr myerr = (my_error_ptr)cinfo->err;

    /* Create the message */
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message) (cinfo, buffer);
    /* Send it to stderr, adding a newline */
    DEBUGF("ERROR %s\n", buffer);

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}

void init_jpeg(js_State *J);

/**
 * @brief load JPG data from PACKFILE.
 *
 * @param f the PACKFILE to load from
 * @param pal pallette (is ignored)
 * @return BITMAP* or NULL if loading fails
 */
#if LINUX == 1
BITMAP *load_jpg_pf(PACKFILE *f, RGB *pal) {
#else
static BITMAP *load_jpg_pf(PACKFILE *f, RGB *pal) {
#endif
    uint8_t *pbuffer = NULL;
    unsigned int malloc_size = JPG_BUFFER_SIZE;
    unsigned int pos = 0;

    // try loading the whole JPEG to memory, Allegro4 PACKFILES have no way of determining file size, so we need this ugly hack!
    pbuffer = realloc(pbuffer, malloc_size);
    if (!pbuffer) {
        return NULL;
    }
    DEBUGF("realloc : mem_size = %d, file_size = %d\n", malloc_size, pos);
    while (true) {
        int ch = pack_getc(f);
        if (ch == EOF) {
            break;  // reading done
        } else {
            pbuffer[pos++] = ch;                                      // store byte
            if (pos >= malloc_size) {                                 // check buffer bounds
                malloc_size += JPG_BUFFER_SIZE;                       // increase buffer size
                uint8_t *new_buffer = realloc(pbuffer, malloc_size);  // try re-alloc
                if (!new_buffer) {                                    // bail out on failure
                    free(pbuffer);
                    return NULL;
                } else {
                    pbuffer = new_buffer;
                }
                DEBUGF("realloc : mem_size = %d, file_size = %d\n", malloc_size, pos);
            }
        }
    }
    DEBUGF("final   : mem_size = %d, file_size = %d\n", malloc_size, pos);

    /* This struct contains the JPEG decompression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     */
    struct jpeg_decompress_struct cinfo={0};
    /* We use our private extension JPEG error handler.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    struct my_error_mgr jerr={0};
    /* More stuff */
    JSAMPARRAY buffer; /* Output row buffer */
    int row_stride;    /* physical row width in output buffer */

    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer)) {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        jpeg_destroy_decompress(&cinfo);
        free(pbuffer);
        return NULL;
    }
    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */
    jpeg_mem_src(&cinfo, pbuffer, pos);

    /* Step 3: read file parameters with jpeg_read_header() */
    (void)jpeg_read_header(&cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.txt for more info.
     */

    /* Step 4: set parameters for decompression */
    /* In this example, we don't need to change any of the defaults set by
     * jpeg_read_header(), so we do nothing here.
     */

    /* Step 5: Start decompressor */
    (void)jpeg_start_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    DEBUGF("JPEG has %dx%dx%d\n", cinfo.output_width, cinfo.output_height, cinfo.num_components);

    if ((cinfo.num_components != 1) && (cinfo.num_components != 3)) {
        DEBUGF("Wrong number of components: %d", cinfo.num_components);
        jpeg_destroy_decompress(&cinfo);
        free(pbuffer);
        return NULL;
    }

    BITMAP *bm = create_bitmap_ex(32, cinfo.output_width, cinfo.output_height);
    if (!bm) {
        DEBUGF("Can't create bitmap: %s", allegro_error);
        jpeg_destroy_decompress(&cinfo);
        free(pbuffer);
        return NULL;
    }

    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     * In this example, we need to make an output work buffer of the right size.
     */
    /* JSAMPLEs per row in output buffer */
    row_stride = cinfo.output_width * cinfo.output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo.output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */
    while (cinfo.output_scanline < cinfo.output_height) {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could ask for
         * more than one scanline at a time if that's more convenient.
         */
        int numread = jpeg_read_scanlines(&cinfo, buffer, 1);
        (void)numread;
        // DEBUGF("num read = %d, current = %d\n", numread, cinfo.output_scanline);
        /* Assume put_scanline_someplace wants a pointer and sample count. */
        // put_scanline_someplace(buffer[0], row_stride);

        for (int x = 0; x < cinfo.output_width; x++) {
            unsigned int xIdx = x * cinfo.output_components;
            if (cinfo.num_components == 1) {
                bm->line[cinfo.output_scanline - 1][x * 4 + _rgb_r_shift_32 / 8] = bm->line[cinfo.output_scanline - 1][x * 4 + _rgb_g_shift_32 / 8] =
                    bm->line[cinfo.output_scanline - 1][x * 4 + _rgb_b_shift_32 / 8] = buffer[0][xIdx];
            } else {
                bm->line[cinfo.output_scanline - 1][x * 4 + _rgb_r_shift_32 / 8] = buffer[0][0 + xIdx];
                bm->line[cinfo.output_scanline - 1][x * 4 + _rgb_g_shift_32 / 8] = buffer[0][1 + xIdx];
                bm->line[cinfo.output_scanline - 1][x * 4 + _rgb_b_shift_32 / 8] = buffer[0][2 + xIdx];
            }
            bm->line[cinfo.output_scanline - 1][x * 4 + _rgb_a_shift_32 / 8] = 0xFF;
        }
    }

    /* Step 7: Finish decompression */

    (void)jpeg_finish_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    /* After finish_decompress, we can free input data.
     * Here we postpone it until after no more JPEG errors are possible,
     * so as to simplify the setjmp error logic above.  (Actually, I don't
     * think that jpeg_destroy can do an error exit, but why assume anything...)
     */
    free(pbuffer);

    /* At this point you may want to check to see whether any corrupt-data
     * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
     */

    /* And we're done! */
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

    DEBUGF("Loading %s\n", filename);

    f = pack_fopen(filename, F_READ);
    if (!f) {
        DEBUGF("can't open %s\n", filename);
        return 0;
    }

    b = load_jpg_pf(f, pal);

    pack_fclose(f);
    return b;
}

/**
 * @brief convert BITMAP to rgba buffer and save as JPG
 *
 * @param bm BITMAP
 * @param fname file name
 * @return true for success, else false
 */
#if LINUX == 1
bool save_jpg(BITMAP *bm, const char *fname, int quality) {
#else
static bool save_jpg(BITMAP *bm, const char *fname, int quality) {
#endif
    const int NUM_COMPONENTS = 3;

    if (quality < 10) {
        quality = 10;
    }
    if (quality > 100) {
        quality = 100;
    }

    /* This struct contains the JPEG compression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     * It is possible to have several such structures, representing multiple
     * compression/decompression processes, in existence at once.  We refer
     * to any one struct (and its associated working data) as a "JPEG object".
     */
    struct jpeg_compress_struct cinfo;
    /* This struct represents a JPEG error handler.  It is declared separately
     * because applications often want to supply a specialized error handler
     * (see the second half of this file for an example).  But here we just
     * take the easy way out and use the standard error handler, which will
     * print a message on stderr and call exit() if compression fails.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    struct jpeg_error_mgr jerr;
    /* More stuff */
    FILE *outfile;           /* target file */
    JSAMPROW row_pointer[1]; /* pointer to JSAMPLE row[s] */
    int row_stride;          /* physical row width in image buffer */

    /* Step 1: allocate and initialize JPEG compression object */

    /* We have to set up the error handler first, in case the initialization
     * step fails.  (Unlikely, but it could happen if you are out of memory.)
     * This routine fills in the contents of struct jerr, and returns jerr's
     * address which we place into the link field in cinfo.
     */
    cinfo.err = jpeg_std_error(&jerr);
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    /* Step 2: specify data destination (eg, a file) */
    /* Note: steps 2 and 3 can be done in either order. */

    /* Here we use the library-supplied code to send compressed data to a
     * stdio stream.  You can also write your own code to do something else.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to write binary files.
     */
    if ((outfile = fopen(fname, "wb")) == NULL) {
        DEBUGF("can't open %s\n", fname);
        return false;
    }
    jpeg_stdio_dest(&cinfo, outfile);

    /* Step 3: set parameters for compression */

    /* First we supply a description of the input image.
     * Four fields of the cinfo struct must be filled in:
     */
    cinfo.image_width = bm->w; /* image width and height, in pixels */
    cinfo.image_height = bm->h;
    cinfo.input_components = NUM_COMPONENTS; /* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB;          /* colorspace of input image */
    /* Now use the library's routine to set default compression parameters.
     * (You must set at least cinfo.in_color_space before calling this,
     * since the defaults depend on the source color space.)
     */
    jpeg_set_defaults(&cinfo);
    /* Now you can set any non-default parameters you wish to.
     * Here we just illustrate the use of quality (quantization table) scaling:
     */
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

    /* Step 4: Start compressor */

    /* TRUE ensures that we will write a complete interchange-JPEG file.
     * Pass TRUE unless you are very sure of what you're doing.
     */
    jpeg_start_compress(&cinfo, TRUE);

    /* Step 5: while (scan lines remain to be written) */
    /*           jpeg_write_scanlines(...); */

    /* Here we use the library's state variable cinfo.next_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     * To keep things simple, we pass one scanline per call; you can pass
     * more if you wish, though.
     */
    row_stride = bm->w * NUM_COMPONENTS; /* JSAMPLEs per row in image_buffer */

    uint8_t *rgba = malloc(row_stride * NUM_COMPONENTS);
    if (!rgba) {
        fclose(outfile);
        jpeg_destroy_compress(&cinfo);
        return false;
    }

    while (cinfo.next_scanline < cinfo.image_height) {
        /* jpeg_write_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could pass
         * more than one scanline at a time if that's more convenient.
         */
        uint8_t *ptr = rgba;
        int y = cinfo.next_scanline;
        for (int x = 0; x < bm->w; x++) {
            uint32_t argb = getpixel(bm, x, y);
            uint8_t r = (argb >> 16) & 0xFF;
            uint8_t g = (argb >> 8) & 0xFF;
            uint8_t b = argb & 0xFF;

            *ptr++ = r;
            *ptr++ = g;
            *ptr++ = b;
        }
        row_pointer[0] = rgba;
        (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    /* Step 6: Finish compression */

    jpeg_finish_compress(&cinfo);
    /* After finish_compress, we can close the output file. */
    fclose(outfile);

    /* Step 7: release JPEG compression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_compress(&cinfo);

    /* And we're done! */
    return true;
}

/**
 * @brief save current screen to file.
 * SaveJpgImage(fname:string, quality:number)
 *
 * @param J the JS context.
 */
static void f_SaveJpgImage(js_State *J) {
    BITMAP *bm = DOjS.current_bm;
    const char *fname = js_tostring(J, 1);

    int quality = 95;
    if (js_isnumber(J, 2)) {
        quality = js_touint16(J, 2);
    }

    if (!save_jpg(bm, fname, quality)) {
        js_error(J, "Can't save screen to JPG file '%s'", fname);
    }
}

#if LINUX != 1
/**
 * @brief save Bitmap to file.
 * SaveJpgImage(fname:string, quality:number)
 *
 * @param J the JS context.
 */
static void Bitmap_SaveJpgImage(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    int quality = 95;
    if (js_isnumber(J, 2)) {
        quality = js_touint16(J, 2);
    }

    if (!save_jpg(bm, fname, quality)) {
        js_error(J, "Can't save Bitmap to JPG file '%s'", fname);
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
void init_jpeg(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

#if LINUX == 1
    register_bitmap_file_type("jpg", load_jpg, NULL);
#else
    register_bitmap_file_type("jpg", load_jpg, NULL, load_jpg_pf);
    DEBUGF("Loaders %p and %p", load_jpg, load_jpg_pf);
#endif
    register_datafile_object(DAT_ID('J', 'P', 'G', ' '), load_from_datafile, (void (*)(void *))destroy_bitmap);
    DEBUGF("Datafile %p", load_from_datafile);

    NFUNCDEF(J, SaveJpgImage, 2);

#if LINUX != 1
    js_getglobal(J, TAG_BITMAP);
    js_getproperty(J, -1, "prototype");
    js_newcfunction(J, Bitmap_SaveJpgImage, "Bitmap.prototype.SaveJpgImage", 2);
    js_defproperty(J, -2, "SaveJpgImage", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    NPROTDEF(J, Bitmap, SaveJpgImage, 2);
#endif
}
