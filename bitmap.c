/*
MIT License

Copyright (c) 2019 Andre Seidelt <superilu@yahoo.com>

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

#include <grx20.h>
#include <grxbmp.h>  // broken include file, can only be included once!
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>

#include <libgrx.h>

#include <clipping.h>

#include "DOjS.h"
#include "bitmap.h"
#include "color.h"
#include "util.h"

/************
** structs **
************/
//! bitmap userdata definition
typedef struct __bitmap {
    GrBmpImageColors *pal;  //!< allocated colors for the image
    GrContext *ctx;         //!< context data in case of a PNG
} bitmap_t;

/*********************
** static functions **
*********************/
/**
 * @brief finalize an image and free resources.
 *
 * @param J VM state.
 */
static void Bitmap_Finalize(js_State *J, void *data) {
    bitmap_t *bm = (bitmap_t *)data;
    if (bm->pal) {
        GrFreeBmpImageColors(bm->pal);
    }
    if (bm->ctx) {
        GrDestroyContext(bm->ctx);
    }
    free(bm);
}

/**
 * @brief load an image and store it as userdata in JS object.
 * new Bitmap(filename:string)
 *
 * @param J VM state.
 */
static void new_Bitmap(js_State *J) {
    const char *fname = js_tostring(J, 1);

    bitmap_t *bm = calloc(1, sizeof(bitmap_t));
    if (!bm) {
        js_error(J, "No memory for image '%s'", fname);
        return;
    }

    if (ut_endsWith(fname, ".BMP") || ut_endsWith(fname, ".bmp")) {
        GrBmpImage *bmp = GrLoadBmpImage((char *)fname);
        if (!bmp) {
            js_error(J, "Can't load BMP image '%s'", fname);
            free(bm);
            return;
        }
        GrAllocBmpImageColors(bmp, bm->pal);

        LOG("BMP loaded\n");

        GrPattern *pat = GrConvertBmpImageToPattern(bmp);
        if (!pat) {
            js_error(J, "No memory for BMP image '%s'", fname);
            free(bm);
            return;
        }
        GrUnloadBmpImage(bmp);

        bm->ctx = GrCreateContext(pat->gp_pixmap.pxp_width, pat->gp_pixmap.pxp_height, NULL, NULL);
        if (!bm->ctx) {
            js_error(J, "No memory for BMP image '%s'", fname);
            GrDestroyPattern(pat);
            free(bm);
            return;
        }

        GrImageDisplayC(bm->ctx, 0, 0, GrImageFromPattern(pat));
        GrDestroyPattern(pat);
    } else if (ut_endsWith(fname, ".PNG") || ut_endsWith(fname, ".png")) {
        int width, height;
        if (GrQueryPng((char *)fname, &width, &height) != 0) {
            js_error(J, "Can't determine size of PNG image '%s'", fname);
            free(bm);
            return;
        }
        bm->ctx = GrCreateContext(width, height, NULL, NULL);
        if (!bm->ctx) {
            js_error(J, "No memory for PNG image '%s'", fname);
            free(bm);
            return;
        }
        if (GrLoadContextFromPng(bm->ctx, (char *)fname, 1) != 0) {
            js_error(J, "Can't load PNG image '%s'", fname);
            GrDestroyContext(bm->ctx);
            free(bm);
            return;
        }
    } else {
        js_error(J, "Filename does not end with .BMP or .PNG '%s'", fname);
        free(bm);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_BITMAP, bm, Bitmap_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, bm->ctx->gc_xmax + 1);
    js_defproperty(J, -2, "width", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, bm->ctx->gc_ymax + 1);
    js_defproperty(J, -2, "height", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
}

/**
 * @brief draw the image to the canvas.
 * img.Draw(x, y)
 *
 * @param J VM state.
 */
static void Bitmap_Draw(js_State *J) {
    bitmap_t *bm = js_touserdata(J, 0, TAG_BITMAP);
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
    GrBitBlt(GrCurrentContext(), x, y, bm->ctx, 0, 0, bm->ctx->gc_xmax, bm->ctx->gc_ymax, GrWRITE);
}

/**
 * @brief get the color of an image pixel.
 * img.GetPixel(x, y):Color
 *
 * @param J the JS context.
 */
static void Bitmap_GetPixel(js_State *J) {
    GrColor *color = malloc(sizeof(GrColor));
    if (!color) {
        js_error(J, "Can't alloc color");
        return;
    }

    bitmap_t *bm = js_touserdata(J, 0, TAG_BITMAP);

    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    *color = (bm->ctx->gc_driver->readpixel)(&bm->ctx->gc_frame, x + bm->ctx->gc_xoffset, y + bm->ctx->gc_yoffset);

    js_getglobal(J, TAG_COLOR);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_COLOR, color, Color_Finalize);
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize bitmap subsystem.
 *
 * @param J VM state.
 */
void init_bitmap(js_State *J) {
    // define the Image() object
    js_newobject(J);
    {
        PROTDEF(J, Bitmap_Draw, TAG_BITMAP, "Draw", 2);
        PROTDEF(J, Bitmap_GetPixel, TAG_BITMAP, "GetPixel", 2);
    }
    js_newcconstructor(J, new_Bitmap, new_Bitmap, TAG_BITMAP, 1);
    js_defglobal(J, TAG_BITMAP, JS_DONTENUM);
}
