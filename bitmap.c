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

#include "DOjS.h"
#include "bitmap.h"

/************
** structs **
************/
//! bitmap userdata definition
typedef struct __bitmap {
    GrPattern *pat;         //!< GrPattern for the image
    GrBmpImageColors *pal;  //!< allocated colors for the image
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
    GrFreeBmpImageColors(bm->pal);
    GrDestroyPattern(bm->pat);
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

    bitmap_t *bm = malloc(sizeof(bitmap_t));
    if (!bm) {
        js_error(J, "No memory for image '%s'", fname);
        return;
    }

    GrBmpImage *bmp = GrLoadBmpImage((char *)fname);
    if (!bmp) {
        js_error(J, "Can't load image '%s'", fname);
        free(bm);
        return;
    }
    GrAllocBmpImageColors(bmp, bm->pal);
    GrPattern *pat = GrConvertBmpImageToStaticPattern(bmp);
    bm->pat = pat;

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_BITMAP, bm, Bitmap_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, pat->gp_pixmap.pxp_width);
    js_defproperty(J, -2, "width", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, pat->gp_pixmap.pxp_height);
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
    GrImageDisplay(x, y, GrImageFromPattern(bm->pat));
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
    { PROTDEF(J, Bitmap_Draw, TAG_BITMAP, "Draw", 2); }
    js_newcconstructor(J, new_Bitmap, new_Bitmap, TAG_BITMAP, 1);
    js_defglobal(J, TAG_BITMAP, JS_DONTENUM);
}
