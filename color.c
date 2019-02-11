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

#include "DOjS.h"
#include "color.h"

/*********************
** static functions **
*********************/
/**
 * @brief finalize a color and free resources.
 *
 * @param J VM state.
 */
static void Color_Finalize(js_State *J, void *data) {
  GrColor *color = (GrColor *)data;
  // DEBUGF("Finalizing Color 0x%08X\n", *color);
  GrFreeColor(*color);
  free(color);
}

/**
 * @brief create a color and store it as userdata in JS object.
 * new Color(red:number, green:number, blue:number)
 *
 * @param J VM state.
 */
static void new_Color(js_State *J) {
  if (js_isundefined(J, 1) || js_isundefined(J, 1) || js_isundefined(J, 1)) {
    js_error(J, "Color constructor needs three integer arguments");
  } else {
    GrColor *color = malloc(sizeof(GrColor));
    if (!color) {
      js_error(J, "Can't alloc color");
    }

    *color = GrAllocColor(js_toint16(J, 1), js_toint16(J, 2), js_toint16(J, 3));
    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_COLOR, color, Color_Finalize);

    // add properties
    js_pushnumber(J, *color);
    js_defproperty(J, -2, "value", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    // DEBUGF("Created Color 0x%08X\n", *color);
  }
}

/**
 * @brief get the red part of a color.
 * col.GetRed():number
 *
 * @param J VM state.
 */
static void Color_getRed(js_State *J) {
  int r, g, b;
  GrColor *color = js_touserdata(J, 0, TAG_COLOR);
  GrQueryColor(*color, &r, &g, &b);
  js_pushnumber(J, r);
}

/**
 * @brief get the green part of a color.
 * col.GetGreen():number
 *
 * @param J VM state.
 */
static void Color_getGreen(js_State *J) {
  int r, g, b;
  GrColor *color = js_touserdata(J, 0, TAG_COLOR);
  GrQueryColor(*color, &r, &g, &b);
  js_pushnumber(J, g);
}

/**
 * @brief get the blue part of a color.
 * col.GetBlue():number
 *
 * @param J VM state.
 */
static void Color_getBlue(js_State *J) {
  int r, g, b;
  GrColor *color = js_touserdata(J, 0, TAG_COLOR);
  GrQueryColor(*color, &r, &g, &b);
  js_pushnumber(J, b);
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize color subsystem.
 *
 * @param J VM state.
 */
void init_color(js_State *J) {
  js_getglobal(J, "Object");
  js_getproperty(J, -1, "prototype");
  {
    PROTDEF(J, Color_getRed, TAG_COLOR, "GetRed", 0);
    PROTDEF(J, Color_getGreen, TAG_COLOR, "GetGreen", 0);
    PROTDEF(J, Color_getBlue, TAG_COLOR, "GetBlue", 0);
  }
  js_newcconstructor(J, new_Color, new_Color, TAG_COLOR, 3);
  js_defglobal(J, TAG_COLOR, JS_DONTENUM);
}
