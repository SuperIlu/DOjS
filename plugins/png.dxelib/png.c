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

#include <mujs.h>
#include <stdio.h>
#include <allegro.h>
#include <alpng.h>

#include "DOjS.h"
#include "bitmap.h"

void init_png(js_State *J);

/**
 * @brief save current screen to file.
 * SavePngImage(fname:string)
 *
 * @param J the JS context.
 */
static void f_SavePngImage(js_State *J) {
    const char *fname = js_tostring(J, 1);

    PALETTE pal;
    get_palette(pal);

    if (save_png(fname, DOjS.current_bm, (const struct RGB *)&pal) != 0) {
        js_error(J, "Can't save screen to PNG file '%s'", fname);
    }
}

/**
 * @brief save Bitmap to file.
 * SavePngImage(fname:string)
 *
 * @param J the JS context.
 */
static void Bitmap_SavePngImage(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    PALETTE pal;
    get_palette(pal);

    if (save_png(fname, bm, (const struct RGB *)&pal) != 0) {
        js_error(J, "Can't save Bitmap to PNG file '%s'", fname);
    }
}

/**
 * @brief initialize PNG loading/saving.
 *
 * @param J VM state.
 */
void init_png(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    /* Make Allegro aware of PNG file format. */
    alpng_init();

    NFUNCDEF(J, SavePngImage, 1);

    js_getglobal(J, TAG_BITMAP);
    js_getproperty(J, -1, "prototype");
    js_newcfunction(J, Bitmap_SavePngImage, "Bitmap.prototype.SavePngImage", 1);
    js_defproperty(J, -2, "SavePngImage", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    NPROTDEF(J, Bitmap, SavePngImage, 1);
}
