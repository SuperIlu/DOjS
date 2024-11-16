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

#include "font.h"

#include <allegro.h>
#if WINDOWS==1
#include <winalleg.h>
#endif
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "DOjS.h"
#include "color.h"
#include "zipfile.h"

#if LINUX == 1
#include "linux/glue.h"
#endif

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
    NEW_OBJECT_PREP(J);
    FONT *f;
    const char *fname;
    if (js_isdefined(J, 1)) {
        fname = js_tostring(J, 1);

        char *delim = strchr(fname, ZIP_DELIM);

        if (!delim) {
            f = load_font((char *)fname, NULL, NULL);
            if (!f) {
                js_error(J, "Can't load font '%s'", fname);
                return;
            }
        } else {
            PACKFILE *pf = open_zipfile1(fname);
            if (!pf) {
                js_error(J, "Can't load font '%s'", fname);
                return;
            }
            f = load_grx_font_pf(pf, NULL, NULL);  // PACKFILE is closed by this function!
            if (!f) {
                js_error(J, "Can't load font '%s'", fname);
                return;
            }
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
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, text_height(f));
    js_defproperty(J, -2, "height", JS_READONLY | JS_DONTCONF);

    js_newarray(J);
    {
        int range = get_font_ranges(f);
        for (int n = 0; n < range; n++) {
            js_newarray(J);
            {
                js_pushnumber(J, get_font_range_begin(f, n));
                js_setindex(J, -2, 0);
                js_pushnumber(J, get_font_range_end(f, n));
                js_setindex(J, -2, 1);
            }
            js_setindex(J, -2, n);
        }
    }
    js_defproperty(J, -2, "ranges", JS_READONLY | JS_DONTCONF);
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

    textout_ex(DOjS.current_bm, f, str, x, y, fg, bg);
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

    textout_centre_ex(DOjS.current_bm, f, str, x, y, fg, bg);
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

    textout_right_ex(DOjS.current_bm, f, str, x, y, fg, bg);
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

/**
 * @brief set the render charactor for unknown code points
 *
 * @param J VM state.
 */
static void f_SetMissingCharacter(js_State *J) {
    const char *missing = js_tostring(J, 1);
    if (strlen(missing) > 0) {
        allegro_404_char = missing[0];
    }
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
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, Font, DrawStringLeft, 5);
        NPROTDEF(J, Font, DrawStringRight, 5);
        NPROTDEF(J, Font, DrawStringCenter, 5);
        NPROTDEF(J, Font, StringWidth, 1);
        NPROTDEF(J, Font, StringHeight, 1);
    }
    CTORDEF(J, new_Font, TAG_FONT, 1);

    NFUNCDEF(J, SetMissingCharacter, 1);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
