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

#include <allegro.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "DOjS.h"
#include "color.h"
#include "font.h"

/*********************
** static functions **
*********************/
/**
 * @brief finalize a font and free resources.
 *
 * @param J VM state.
 */
static void Font_Finalize(js_State *J, void *data) {
    FONT *f = (FONT *)data;
    if (f != font) {
        destroy_font(f);
    }
}

/**
 * @brief load an image and store it as userdata in JS object.
 * new Bitmap(filename:string)
 *
 * @param J VM state.
 */
static void new_Font(js_State *J) {
    FONT *f;
    const char *fname;
    if (js_isdefined(J, 1)) {
        fname = js_tostring(J, 1);

        f = load_font((char *)fname, NULL, NULL);
        if (!f) {
            js_error(J, "Can't load font '%s'", allegro_error);
            return;
        }
    } else {
        fname = "<<INTERNAL>>";
        f = font;
    }
    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_FONT, f, Font_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, text_height(f));
    js_defproperty(J, -2, "height", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
}

/**
 * @brief draw a left aligned string to the canvas.
 * font.DrawStringLeft(x:number, y:number, text:string, foreground:Color, background: Color)
 *
 * @param J VM state.
 */
static void Font_DrawStringLeft(js_State *J) {
    FONT *f = js_touserdata(J, 0, TAG_FONT);
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    const char *str = js_tostring(J, 3);

    int fg = js_toint32(J, 4);
    int bg = js_toint32(J, 5);

    textout_ex(current_bm, f, str, x, y, fg, bg);
}

/**
 * @brief draw a center aligned string to the canvas.
 * font.DrawStringCenter(x:number, y:number, text:string, foreground:Color, background: Color)
 *
 * @param J VM state.
 */
static void Font_DrawStringCenter(js_State *J) {
    FONT *f = js_touserdata(J, 0, TAG_FONT);
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    const char *str = js_tostring(J, 3);

    int fg = js_toint32(J, 4);
    int bg = js_toint32(J, 5);

    textout_centre_ex(current_bm, f, str, x, y, fg, bg);
}

/**
 * @brief draw a right aligned string to the canvas.
 * font.DrawStringRight(x:number, y:number, text:string, foreground:Color, background: Color)
 *
 * @param J VM state.
 */
static void Font_DrawStringRight(js_State *J) {
    FONT *f = js_touserdata(J, 0, TAG_FONT);
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    const char *str = js_tostring(J, 3);

    int fg = js_toint32(J, 4);
    int bg = js_toint32(J, 5);

    textout_right_ex(current_bm, f, str, x, y, fg, bg);
}

/**
 * @brief calculate string width for this font.
 * font.StringWidth(text:string):number
 *
 * @param J VM state.
 */
static void Font_StringWidth(js_State *J) {
    FONT *f = js_touserdata(J, 0, TAG_FONT);
    const char *str = js_tostring(J, 1);

    js_pushnumber(J, text_length(f, str));
}

/**
 * @brief calculate string height for this font.
 * font.StringHeight(text:string):number
 *
 * @param J VM state.
 */
static void Font_StringHeight(js_State *J) {
    FONT *f = js_touserdata(J, 0, TAG_FONT);
    const char *str = js_tostring(J, 1);
    (void)str;
    js_pushnumber(J, text_height(f));
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize font subsystem.
 *
 * @param J VM state.
 */
void init_font(js_State *J) {
    js_newobject(J);
    {
        PROTDEF(J, Font_DrawStringLeft, TAG_FONT, "DrawStringLeft", 5);
        PROTDEF(J, Font_DrawStringRight, TAG_FONT, "DrawStringRight", 5);
        PROTDEF(J, Font_DrawStringCenter, TAG_FONT, "DrawStringCenter", 5);
        PROTDEF(J, Font_StringWidth, TAG_FONT, "StringWidth", 1);
        PROTDEF(J, Font_StringHeight, TAG_FONT, "StringHeight", 1);
    }
    js_newcconstructor(J, new_Font, new_Font, TAG_FONT, 1);
    js_defglobal(J, TAG_FONT, JS_DONTENUM);
}
