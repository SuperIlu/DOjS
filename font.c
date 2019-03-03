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
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "DOjS.h"
#include "color.h"
#include "font.h"

/************
** structs **
************/
//! font userdata definition
typedef struct __font {
    GrFont *font;  //!< GrFont data
} font_t;

/*********************
** static functions **
*********************/
/**
 * @brief finalize a font and free resources.
 *
 * @param J VM state.
 */
static void Font_Finalize(js_State *J, void *data) {
    font_t *f = (font_t *)data;
    GrUnloadFont(f->font);
    free(f);
}

/**
 * @brief load an image and store it as userdata in JS object.
 * new Bitmap(filename:string)
 *
 * @param J VM state.
 */
static void new_Font(js_State *J) {
    const char *fname = js_tostring(J, 1);

    font_t *f = malloc(sizeof(font_t));
    if (!f) {
        js_error(J, "No memory for font '%s'", fname);
        return;
    }

    f->font = GrLoadFont((char *)fname);
    if (!f->font) {
        js_error(J, "Can't load font '%s'", fname);
        free(f);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_FONT, f, Font_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, f->font->minwidth);
    js_defproperty(J, -2, "minwidth", 0);

    js_pushnumber(J, f->font->maxwidth);
    js_defproperty(J, -2, "maxwidth", 0);

    js_pushnumber(J, f->font->h.height);
    js_defproperty(J, -2, "height", 0);
}

/**
 * @brief draw a string to the canvas.
 * font.DrawString(x:number, y:number, text:string, foreground:Color,
 * background: Color, direction:number, alignX:number, alignY:number)
 *
 * @param J VM state.
 */
static void Font_DrawString(js_State *J) {
    GrTextOption opt;

    font_t *f = js_touserdata(J, 0, TAG_FONT);
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    const char *str = js_tostring(J, 3);

    GrColor *fg = js_touserdata(J, 4, TAG_COLOR);
    GrColor *bg = js_touserdata(J, 5, TAG_COLOR);

    int dir = js_toint16(J, 6);
    int xalign = js_toint16(J, 7);
    int yalign = js_toint16(J, 8);

    opt.txo_font = f->font;
    opt.txo_bgcolor.v = *bg;
    opt.txo_fgcolor.v = *fg;
    opt.txo_direct = dir;
    opt.txo_xalign = xalign;
    opt.txo_yalign = yalign;
    opt.txo_chrtype = GR_BYTE_TEXT;

    GrDrawString((char *)str, strlen(str), x, y, &opt);
}

/**
 * @brief calculate string width for this font.
 * font.StringWidth(text:string):number
 *
 * @param J VM state.
 */
static void Font_StringWidth(js_State *J) {
    font_t *f = js_touserdata(J, 0, TAG_FONT);
    const char *str = js_tostring(J, 1);

    js_pushnumber(J, GrFontStringWidth(f->font, str, strlen(str), GR_BYTE_TEXT));
}

/**
 * @brief calculate string height for this font.
 * font.StringHeight(text:string):number
 *
 * @param J VM state.
 */
static void Font_StringHeight(js_State *J) {
    font_t *f = js_touserdata(J, 0, TAG_FONT);
    const char *str = js_tostring(J, 1);
    (void)str;
    js_pushnumber(J, GrFontStringHeight(f->font, str, strlen(str), GR_BYTE_TEXT));
}

/**
 * @brief resize font to new width/height.
 * font.Resize(w:number, h:number)
 *
 * @param J VM state.
 */
static void Font_Resize(js_State *J) {
    font_t *f = js_touserdata(J, 0, TAG_FONT);
    int w = js_toint16(J, 1);
    int h = js_toint16(J, 2);

    GrFont *new = GrBuildConvertedFont(f->font, GR_FONTCVT_RESIZE, w, h, 0, 0);
    if (new) {
        GrUnloadFont(f->font);
        f->font = new;
    }

    js_pushnumber(J, f->font->minwidth);
    js_setproperty(J, 0, "minwidth");

    js_pushnumber(J, f->font->maxwidth);
    js_setproperty(J, 0, "maxwidth");

    js_pushnumber(J, f->font->h.height);
    js_setproperty(J, 0, "height");
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
        PROTDEF(J, Font_DrawString, TAG_FONT, "DrawString", 6);
        PROTDEF(J, Font_StringWidth, TAG_FONT, "StringWidth", 1);
        PROTDEF(J, Font_StringHeight, TAG_FONT, "StringHeight", 1);
        PROTDEF(J, Font_Resize, TAG_FONT, "Resize", 2);
    }
    js_newcconstructor(J, new_Font, new_Font, TAG_FONT, 1);
    js_defglobal(J, TAG_FONT, JS_DONTENUM);
}
