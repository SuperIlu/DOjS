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

#include "DOjS.h"
#include "color.h"

/*********************
** static functions **
*********************/
static void f_Color(js_State *J) {
    if (js_isundefined(J, 1) || js_isundefined(J, 2) || js_isundefined(J, 3)) {
        js_error(J, "Color needs at least three integer arguments");
        return;
    } else {
        int r = js_toint16(J, 1);
        int g = js_toint16(J, 2);
        int b = js_toint16(J, 3);
        int a = 255;
        if (js_isdefined(J, 4)) {
            a = js_toint16(J, 4);
        }
        uint32_t rgba = makeacol(r, g, b, a);
        js_pushnumber(J, rgba);
    }
}

/**
 * @brief get the red part of a color.
 * GetRed(c:color):number
 *
 * @param J VM state.
 */
static void f_getRed(js_State *J) { js_pushnumber(J, getr(js_toint32(J, 1))); }

/**
 * @brief get the green part of a color.
 * GetGreen(c:color):number
 *
 * @param J VM state.
 */
static void f_getGreen(js_State *J) { js_pushnumber(J, getg(js_toint32(J, 1))); }

/**
 * @brief get the blue part of a color.
 * GetBlue(c:Color):number
 *
 * @param J VM state.
 */
static void f_getBlue(js_State *J) { js_pushnumber(J, getb(js_toint32(J, 1))); }

/**
 * @brief get the blue part of a color.
 * GetAlpha(c:color):number
 *
 * @param J VM state.
 */
static void f_getAlpha(js_State *J) { js_pushnumber(J, geta(js_toint32(J, 1))); }

/***********************
** exported functions **
***********************/
/**
 * @brief initialize color subsystem.
 *
 * @param J VM state.
 */
void init_color(js_State *J) {
    FUNCDEF(J, f_Color, "Color", 4);
    FUNCDEF(J, f_getRed, "GetRed", 1);
    FUNCDEF(J, f_getGreen, "GetGreen", 1);
    FUNCDEF(J, f_getBlue, "GetBlue", 1);
    FUNCDEF(J, f_getAlpha, "GetAlpha", 1);
}
