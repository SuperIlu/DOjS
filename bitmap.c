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

#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>

#include <allegro.h>

#include "DOjS.h"
#include "bitmap.h"
#include "color.h"
#include "util.h"

/*********************
** static functions **
*********************/
/**
 * @brief finalize an image and free resources.
 *
 * @param J VM state.
 */
static void Bitmap_Finalize(js_State *J, void *data) {
    BITMAP *bm = (BITMAP *)data;
    destroy_bitmap(bm);
}

/**
 * @brief load an image or create an empty bitmap.
 * new Bitmap(filename:string)
 * new Bitmap(width:number, height:number)
 *
 * @param J VM state.
 */
static void new_Bitmap(js_State *J) {
    const char *fname = "<<buffer>>";
    BITMAP *bm = NULL;
    if (js_isnumber(J, 1) && js_isnumber(J, 2)) {
        bm = create_bitmap(js_tonumber(J, 1), js_tonumber(J, 2));
    } else {
        fname = js_tostring(J, 1);

        bm = load_bitmap(fname, NULL);
        if (!bm) {
            js_error(J, "Can't load image '%s'", fname);
            return;
        }
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_BITMAP, bm, Bitmap_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, bm->w);
    js_defproperty(J, -2, "width", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, bm->h);
    js_defproperty(J, -2, "height", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
}

/**
 * @brief draw the image to the canvas.
 * img.Draw(x, y)
 *
 * @param J VM state.
 */
static void Bitmap_Draw(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
    blit(bm, DOjS.current_bm, 0, 0, x, y, bm->w, bm->h);
}

/**
 * @brief draw the image to the canvas.
 * img.DrawAdvanced(source_x, source_y, source_width, source_height, dest_x, dest_y, dest_width, dest_height)
 *
 * @param J VM state.
 */
static void Bitmap_DrawAdvanced(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    int srcX = js_toint16(J, 1);
    int srcY = js_toint16(J, 2);
    int srcW = js_toint16(J, 3);
    int srcH = js_toint16(J, 4);

    int destX = js_toint16(J, 5);
    int destY = js_toint16(J, 6);
    int destW = js_toint16(J, 7);
    int destH = js_toint16(J, 8);
    stretch_blit(bm, DOjS.current_bm, srcX, srcY, srcW, srcH, destX, destY, destW, destH);
}

/**
 * @brief clear the bitmap.
 * img.Clear()
 *
 * @param J VM state.
 */
static void Bitmap_Clear(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    clear_bitmap(bm);
}

/**
 * @brief draw the image to the canvas.
 * img.DrawTrans(x, y)
 *
 * @param J VM state.
 */
static void Bitmap_DrawTrans(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
    draw_trans_sprite(DOjS.current_bm, bm, x, y);
}

/**
 * @brief get the color of an image pixel.
 * img.GetPixel(x, y):Color
 *
 * @param J the JS context.
 */
static void Bitmap_GetPixel(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);

    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    js_pushnumber(J, getpixel(bm, x, y) | 0xFF000000);  // no alpha in bitmaps so far
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
        PROTDEF(J, Bitmap_DrawAdvanced, TAG_BITMAP, "DrawAdvanced", 8);
        PROTDEF(J, Bitmap_Clear, TAG_BITMAP, "Clear", 0);
        PROTDEF(J, Bitmap_DrawTrans, TAG_BITMAP, "DrawTrans", 2);
        PROTDEF(J, Bitmap_GetPixel, TAG_BITMAP, "GetPixel", 2);
    }
    js_newcconstructor(J, new_Bitmap, new_Bitmap, TAG_BITMAP, 2);
    js_defglobal(J, TAG_BITMAP, JS_DONTENUM);
}
