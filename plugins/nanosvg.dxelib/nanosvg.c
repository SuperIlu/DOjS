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

#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

void init_nanosvg(js_State *J);
float fminf(float x, float y);

/**
 * @brief render SVG file to bitmap.
 * RenderSVG(fname:string)
 *
 * @param J the JS context.
 */
static void Bitmap_RenderSVG(js_State *J) {
    NSVGimage *image = NULL;
    NSVGrasterizer *rast = NULL;
    unsigned char *img = NULL;
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    image = nsvgParseFromFile(fname, "px", 96.0f);
    if (image == NULL) {
        js_error(J, "Could not open SVG image '%s'.", fname);
        nsvgDeleteRasterizer(rast);
        nsvgDelete(image);
        return;
    }
    float scaling = fminf((float)bm->w / image->width, (float)bm->h / image->height);

    DEBUGF("'%s' image=%p\n", fname, image);
    DEBUGF("BM is %dx%d\n", bm->w, bm->h);
    DEBUGF("SVG is %fx%f\n", image->width, image->height);
    DEBUGF("--> %fx%f := %f\n", (float)bm->w / image->width, (float)bm->h / image->height, scaling);
    DEBUGF("--> %dx%d\n", (int)(scaling * image->width), (int)(scaling * image->height));

    rast = nsvgCreateRasterizer();
    if (rast == NULL) {
        js_error(J, "Could not init rasterizer.");
        nsvgDeleteRasterizer(rast);
        nsvgDelete(image);
        return;
    }
    DEBUGF("Rasterizer=%p\n", rast);

    int img_size = bm->w * bm->h * sizeof(uint32_t);
    img = malloc(img_size);
    if (img == NULL) {
        printf("Could not alloc image buffer.");
        nsvgDeleteRasterizer(rast);
        nsvgDelete(image);
        return;
    }
    DEBUGF("img=%p (%d)\n", img, img_size);

    nsvgRasterize(rast, image, 0, 0, scaling, img, bm->w, bm->h, bm->w * sizeof(uint32_t));

    int pixel_size = bm->w * bm->h;
    int idx = 0;
    for (int i = 0; i < pixel_size; i++) {
        putpixel(bm, i % bm->w, i / bm->w, makeacol32(img[idx + 0], img[idx + 1], img[idx + 2], img[idx + 3]));
        idx += 4;
    }

    free(img);
    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);
}

/*********************
** public functions **
*********************/
/**
 * @brief initialize neural subsystem.
 *
 * @param J VM state.
 */
void init_nanosvg(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    js_getglobal(J, TAG_BITMAP);
    js_getproperty(J, -1, "prototype");
    js_newcfunction(J, Bitmap_RenderSVG, "Bitmap.prototype.RenderSVG", 1);
    js_defproperty(J, -2, "RenderSVG", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
}
