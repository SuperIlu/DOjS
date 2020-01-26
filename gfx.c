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
#include <dirent.h>
#include <dpmi.h>
#include <errno.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "DOjS.h"
#include "bitmap.h"
#include "color.h"
#include "funcs.h"
#include "gfx.h"
#include "util.h"

/************
** structs **
************/
//! polygon data as needed by the drawing functions.
typedef struct poly_array {
    int len;    //!< number of entries
    int *data;  //!< a dynamic array: int[][2]
} poly_array_t;

//! arc coordinates
typedef struct _arc_return {
    int16_t startX;   //!< start point X
    int16_t startY;   //!< start point Y
    int16_t endX;     //!< end point X
    int16_t endY;     //!< end point Y
    int16_t centerX;  //!< center point X
    int16_t centerY;  //!< center point Y
} arc_return_t;

/*********************
** static variables **
*********************/
/**
 * @brief info structure of the current arc.
 */
static arc_return_t arcReturn;

/**
 * @brief radius of the current custom primitive.
 */
static int customRadius;

/*********************
** static functions **
*********************/
/**
 * @brief free a polygon array[][2].
 *
 * @param array the array to free.
 */
static void f_freeArray(poly_array_t *array) {
    if (array) {
        free(array->data);
        free(array);
    }
}

/**
 * @brief allocate an array[len][2] for the given number of points.
 *
 * @param len the number of points.
 *
 * @return poly_array_t* or NULL if out of memory.
 */
static poly_array_t *f_allocArray(int len) {
    poly_array_t *array = malloc(sizeof(poly_array_t));
    if (!array) {
        return NULL;
    }

    array->len = len;
    array->data = malloc(sizeof(int) * len * 2);
    if (!array->data) {
        f_freeArray(array);
        return NULL;
    }
    return array;
}

/**
 * @brief convert JS-array to C array for polygon functions.
 *
 * @param J the JS context.
 * @param idx index of th JS-array on the stack.
 *
 * @return poly_array_t* or NULL if out of memory.
 */
static poly_array_t *f_convertArray(js_State *J, int idx) {
    if (!js_isarray(J, idx)) {
        JS_ENOARR(J);
        return NULL;
    } else {
        int len = js_getlength(J, idx);
        poly_array_t *array = f_allocArray(len);
        if (!array) {
            return NULL;
        }
        for (int i = 0; i < len; i++) {
            js_getindex(J, idx, i);
            {
                int pointlen = js_getlength(J, -1);
                if (pointlen < 2) {
                    js_error(J, "Points must have two values");
                    f_freeArray(array);
                    return NULL;
                }

                js_getindex(J, -1, 0);
                int x = js_toint16(J, -1);
                js_pop(J, 1);

                js_getindex(J, -1, 1);
                int y = js_toint16(J, -1);
                js_pop(J, 1);

                array->data[i * 2 + 0] = x;
                array->data[i * 2 + 1] = y;
            }
            js_pop(J, 1);
        }
        return array;
    }
}

/**
 * @brief get name of screen mode.
 * GetScreenMode():string
 *
 * @param J
 */
static void f_GetScreenMode(js_State *J) { js_pushnumber(J, get_color_depth()); }

/**
 * @brief get the width of the drawing area.
 * SizeX():number
 *
 * @param J the JS context.
 */
static void f_SizeX(js_State *J) { js_pushnumber(J, DOjS.current_bm->w); }

/**
 * @brief get the height of the drawing area.
 * SizeY():number
 *
 * @param J the JS context.
 */
static void f_SizeY(js_State *J) { js_pushnumber(J, DOjS.current_bm->h); }

/**
 * @brief create object with the coordinates of the last arc drawn.
 *
 * res={"centerX":50,"centerY":50,"endX":71,"endY":29,"startX":80,"startY":50}
 *
 * @param J the JS context.
 */
static void f_arcReturn(js_State *J, arc_return_t *art) {
    js_newobject(J);
    {
        js_pushnumber(J, art->startX);
        js_setproperty(J, -2, "startX");
        js_pushnumber(J, art->startY);
        js_setproperty(J, -2, "startY");
        js_pushnumber(J, art->endX);
        js_setproperty(J, -2, "endX");
        js_pushnumber(J, art->endY);
        js_setproperty(J, -2, "endY");
        js_pushnumber(J, art->centerX);
        js_setproperty(J, -2, "centerX");
        js_pushnumber(J, art->centerY);
        js_setproperty(J, -2, "centerY");
    }
}

/**
 * @brief clear the screen with given color.
 * ClearScreen(c:Color)
 *
 * @param J the JS context.
 */
static void f_ClearScreen(js_State *J) {
    int color = js_toint32(J, 1);

    clear_to_color(DOjS.current_bm, color);
}

/**
 * @brief draw a plot.
 *
 * Plot(x:number, y:number, c:Color)
 *
 * @param J the JS context.
 */
static void f_Plot(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    int color = js_toint32(J, 3);

    putpixel(DOjS.current_bm, x, y, color);
}

/**
 * @brief draw a line.
 *
 * Line(x1:number, y1:number, x2:number, y2:number, c:Color)
 *
 * @param J the JS context.
 */
static void f_Line(js_State *J) {
    int x1 = js_toint16(J, 1);
    int y1 = js_toint16(J, 2);
    int x2 = js_toint16(J, 3);
    int y2 = js_toint16(J, 4);

    int color = js_toint32(J, 5);

    line(DOjS.current_bm, x1, y1, x2, y2, color);
}

/**
 * @brief custom function to draw 'larger pixels'.
 *
 * @param bmp destination BITMAP.
 * @param x pixel coordinate.
 * @param y pixel coordinate.
 * @param d color.
 */
static void f_customPixel(BITMAP *bmp, int x, int y, int d) { circlefill(bmp, x, y, customRadius, d); }

/**
 * @brief draw a line with variable thinkness.
 *
 * CustomLine(x1:number, y1:number, x2:number, y2:number, w:number, c:Color)
 *
 * @param J the JS context.
 */
static void f_CustomLine(js_State *J) {
    int x1 = js_toint16(J, 1);
    int y1 = js_toint16(J, 2);
    int x2 = js_toint16(J, 3);
    int y2 = js_toint16(J, 4);
    int w = js_toint16(J, 5);

    int color = js_toint32(J, 6);

    if (w % 2) {
        customRadius = w / 2 + 1;
    } else {
        customRadius = w / 2;
    }
    do_line(DOjS.current_bm, x1, y1, x2, y2, color, f_customPixel);
}

/**
 * @brief draw a box.
 *
 * Box(x1:number, y1:number, x2:number, y2:number, c:Color)
 *
 * @param J the JS context.
 */
static void f_Box(js_State *J) {
    int x1 = js_toint16(J, 1);
    int y1 = js_toint16(J, 2);
    int x2 = js_toint16(J, 3);
    int y2 = js_toint16(J, 4);

    int color = js_toint32(J, 5);

    rect(DOjS.current_bm, x1, y1, x2, y2, color);
}

/**
 * @brief draw a circle.
 *
 * Circle(x1:number, y1:number, r:number, c:Color)
 *
 * @param J the JS context.
 */
static void f_Circle(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
    int r = js_toint16(J, 3);

    int color = js_toint32(J, 4);

    circle(DOjS.current_bm, x, y, r, color);
}

/**
 * @brief draw a circle with variable thinkness.
 *
 * CustomCircle(x1:number, y1:number, r:number, w:number, c:Color)
 *
 * @param J the JS context.
 */
static void f_CustomCircle(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
    int r = js_toint16(J, 3);
    int w = js_toint16(J, 4);

    int color = js_toint32(J, 5);

    if (w % 2) {
        customRadius = w / 2 + 1;
    } else {
        customRadius = w / 2;
    }
    do_circle(DOjS.current_bm, x, y, r, color, f_customPixel);
}

/**
 * @brief draw an ellipse.
 *
 * Ellipse(xc:number, yc:number, xa:number, ya:number, c:Color)
 *
 * @param J the JS context.
 */
static void f_Ellipse(js_State *J) {
    int xc = js_toint16(J, 1);
    int yc = js_toint16(J, 2);
    int xa = js_toint16(J, 3);
    int ya = js_toint16(J, 4);

    int color = js_toint32(J, 5);

    ellipse(DOjS.current_bm, xc, yc, xa, ya, color);
}

/**
 * @brief draw an ellipse with variable thinkness.
 *
 * CustomEllipse(xc:number, yc:number, xa:number, ya:number, c:Color)
 *
 * @param J the JS context.
 */
static void f_CustomEllipse(js_State *J) {
    int xc = js_toint16(J, 1);
    int yc = js_toint16(J, 2);
    int xa = js_toint16(J, 3);
    int ya = js_toint16(J, 4);
    int w = js_toint16(J, 5);

    int color = js_toint32(J, 6);

    if (w % 2) {
        customRadius = w / 2 + 1;
    } else {
        customRadius = w / 2;
    }
    do_ellipse(DOjS.current_bm, xc, yc, xa, ya, color, f_customPixel);
}

/**
 * @brief record the coordinates while drawing pixels.
 *
 * @param bmp destination BITMAP.
 * @param x pixel coordinate.
 * @param y pixel coordinate.
 * @param d color.
 */
static void f_recordingPixel(BITMAP *bmp, int x, int y, int d) {
    arcReturn.endX = x;
    arcReturn.endY = y;
    if (arcReturn.startX == -1) {
        arcReturn.startX = x;
        arcReturn.startY = y;
    }
    putpixel(bmp, x, y, d);
}

/**
 * @brief record the coordinates while drawing custom pixels.
 *
 * @param bmp destination BITMAP.
 * @param x pixel coordinate.
 * @param y pixel coordinate.
 * @param d color.
 */
static void f_recordingCustomPixel(BITMAP *bmp, int x, int y, int d) {
    arcReturn.endX = x;
    arcReturn.endY = y;
    if (arcReturn.startX == -1) {
        arcReturn.startX = x;
        arcReturn.startY = y;
    }
    circlefill(bmp, x, y, customRadius, d);
}

/**
 * @brief Draw a circle arc.
 * CircleArc(
 *    x:number,
 *    y:number,
 *    r:number,
 *    start:number,
 *    end:number,
 *    c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
 *
 * @param J the JS context.
 */
static void f_CircleArc(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
    int r = js_toint16(J, 3);

    double start = js_tonumber(J, 4);
    double end = js_tonumber(J, 5);

    int color = js_toint32(J, 6);

    arcReturn.startX = arcReturn.startY = -1;
    arcReturn.centerX = x;
    arcReturn.centerY = y;
    do_arc(DOjS.current_bm, x, y, ftofix(start), ftofix(end), r, color, f_recordingPixel);

    f_arcReturn(J, &arcReturn);
}

/**
 * @brief Draw a circle arc with variable thinkness.
 * CustomCircleArc(
 *    x:number,
 *    y:number,
 *    r:number,
 *    start:number,
 *    end:number,
 *    w:number,
 *    c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
 *
 * @param J the JS context.
 */
static void f_CustomCircleArc(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
    int r = js_toint16(J, 3);

    double start = js_tonumber(J, 4);
    double end = js_tonumber(J, 5);
    int w = js_toint16(J, 6);

    int color = js_toint32(J, 7);

    if (w % 2) {
        customRadius = w / 2 + 1;
    } else {
        customRadius = w / 2;
    }
    arcReturn.startX = arcReturn.startY = -1;
    arcReturn.centerX = x;
    arcReturn.centerY = y;
    do_arc(DOjS.current_bm, x, y, ftofix(start), ftofix(end), r, color, f_recordingCustomPixel);

    f_arcReturn(J, &arcReturn);
}

/**
 * @brief draw a filled box.
 *
 * FilledBox(x1:number, y1:number, x2:number, y2:number, c:Color)
 *
 * @param J the JS context.
 */
static void f_FilledBox(js_State *J) {
    int x1 = js_toint16(J, 1);
    int y1 = js_toint16(J, 2);
    int x2 = js_toint16(J, 3);
    int y2 = js_toint16(J, 4);

    int color = js_toint32(J, 5);

    rectfill(DOjS.current_bm, x1, y1, x2, y2, color);
}

/**
 * @brief draw a filled circle.
 *
 * FilledCircle(x1:number, y1:number, r:number, c:Color)
 *
 * @param J the JS context.
 */
static void f_FilledCircle(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
    int r = js_toint16(J, 3);

    int color = js_toint32(J, 4);

    circlefill(DOjS.current_bm, x, y, r, color);
}

/**
 * @brief draw an ellipse.
 *
 * FilledEllipse(xc:number, yc:number, xa:number, ya:number, c:Color)
 *
 * @param J the JS context.
 */
static void f_FilledEllipse(js_State *J) {
    int xc = js_toint16(J, 1);
    int yc = js_toint16(J, 2);
    int xa = js_toint16(J, 3);
    int ya = js_toint16(J, 4);

    int color = js_toint32(J, 5);

    ellipsefill(DOjS.current_bm, xc, yc, xa, ya, color);
}

/**
 * @brief do a flood fill.
 * FloodFill(x:number, y:number, c:Color)
 *
 * @param J the JS context.
 */
static void f_FloodFill(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    int color = js_toint32(J, 3);

    floodfill(DOjS.current_bm, x, y, color);
}

/**
 * @brief draw a polyline.
 * PolyLine([[x1, x2], [..], [xN, yN]], c:Color)
 *
 * @param J the JS context.
 */
static void f_FilledPolygon(js_State *J) {
    poly_array_t *array = f_convertArray(J, 1);
    int color = js_toint32(J, 2);

    polygon(DOjS.current_bm, array->len, array->data, color);

    f_freeArray(array);
}

/**
 * @brief draw a text with the default font.
 * TextXY(x:number, y:number, text:string, fg:Color, bg:Color)
 *
 * @param J the JS context.
 */
static void f_TextXY(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    const char *str = js_tostring(J, 3);

    int fg = js_toint32(J, 4);
    int bg = js_toint32(J, 5);

    textout_ex(DOjS.current_bm, font, (char *)str, x, y, fg, bg);
}

/**
 * @brief save current screen to file.
 * SaveBmpImage(fname:string)
 *
 * @param J the JS context.
 */
static void f_SaveBmpImage(js_State *J) {
    const char *fname = js_tostring(J, 1);

    PALETTE pal;
    get_palette(pal);

    if (save_bmp(fname, DOjS.current_bm, (const struct RGB *)&pal) != 0) {
        js_error(J, "Can't save screen to BMP file '%s': %s", fname, allegro_error);
    }
}

/**
 * @brief save current screen to file.
 * SaveBmpImage(fname:string)
 *
 * @param J the JS context.
 */
static void f_SavePcxImage(js_State *J) {
    const char *fname = js_tostring(J, 1);

    PALETTE pal;
    get_palette(pal);

    if (save_pcx(fname, DOjS.current_bm, (const struct RGB *)&pal) != 0) {
        js_error(J, "Can't save screen to PCX file '%s': %s", fname, allegro_error);
    }
}

/**
 * @brief save current screen to file.
 * SaveBmpImage(fname:string)
 *
 * @param J the JS context.
 */
static void f_SaveTgaImage(js_State *J) {
    const char *fname = js_tostring(J, 1);

    PALETTE pal;
    get_palette(pal);

    if (save_tga(fname, DOjS.current_bm, (const struct RGB *)&pal) != 0) {
        js_error(J, "Can't save screen to TGA file '%s': %s", fname, allegro_error);
    }
}

/**
 * @brief get the color of an on-screen pixel.
 * GetPixel(x, y):Color
 *
 * @param J the JS context.
 */
static void f_GetPixel(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
    js_pushnumber(J, getpixel(DOjS.current_bm, x, y) | 0xFE000000);
}

/**
 * @brief enable/disable transparency.
 * TransparencyEnabled(boolean)
 *
 * @param J the JS context.
 */
static void f_TransparencyEnabled(js_State *J) {
    DOjS.transparency_available = js_toboolean(J, 1);
    update_transparency();
}

/**
 * @brief set the current rendering destination.
 * SetRenderBitmap(bm:Bitmap)
 *
 * @param J the JS context.
 */
static void f_SetRenderBitmap(js_State *J) {
    if (js_isundefined(J, 1) || js_isnull(J, 1)) {
        DOjS.current_bm = DOjS.render_bm;
    } else {
        BITMAP *bm = js_touserdata(J, 1, TAG_BITMAP);
        DOjS.current_bm = bm;
    }
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize grx subsystem.
 *
 * @param J VM state.
 */
void init_gfx(js_State *J) {
    // define some global properties
    js_pushglobal(J);
    js_setglobal(J, "global");

    // define global functions
    FUNCDEF(J, f_SetRenderBitmap, "SetRenderBitmap", 0);
    FUNCDEF(J, f_GetScreenMode, "GetScreenMode", 0);
    FUNCDEF(J, f_SizeX, "SizeX", 0);
    FUNCDEF(J, f_SizeY, "SizeY", 0);

    FUNCDEF(J, f_ClearScreen, "ClearScreen", 1);
    FUNCDEF(J, f_TextXY, "TextXY", 5);

    FUNCDEF(J, f_Plot, "Plot", 3);
    FUNCDEF(J, f_Line, "Line", 5);
    FUNCDEF(J, f_CustomLine, "CustomLine", 6);
    FUNCDEF(J, f_Box, "Box", 5);
    FUNCDEF(J, f_Circle, "Circle", 4);
    FUNCDEF(J, f_CustomCircle, "CustomCircle", 5);
    FUNCDEF(J, f_Ellipse, "Ellipse", 5);
    FUNCDEF(J, f_CustomEllipse, "CustomEllipse", 6);
    FUNCDEF(J, f_CircleArc, "CircleArc", 7);
    FUNCDEF(J, f_CustomCircleArc, "CustomCircleArc", 8);

    FUNCDEF(J, f_FilledBox, "FilledBox", 5);
    FUNCDEF(J, f_FilledCircle, "FilledCircle", 4);
    FUNCDEF(J, f_FilledEllipse, "FilledEllipse", 5);
    FUNCDEF(J, f_FilledPolygon, "FilledPolygon", 2);

    FUNCDEF(J, f_FloodFill, "FloodFill", 4);

    FUNCDEF(J, f_SaveBmpImage, "SaveBmpImage", 1);
    FUNCDEF(J, f_SavePcxImage, "SavePcxImage", 1);
    FUNCDEF(J, f_SaveTgaImage, "SaveTgaImage", 1);

    FUNCDEF(J, f_GetPixel, "GetPixel", 2);

    FUNCDEF(J, f_TransparencyEnabled, "TransparencyEnabled", 2);
}
