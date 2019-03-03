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

#include <dirent.h>
#include <errno.h>
#include <grx20.h>
#include <grxkeys.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include <grxbmp.h>  // broken include file, can only be included once!

#include "DOjS.h"
#include "color.h"
#include "funcs.h"
#include "util.h"

/************
** structs **
************/
//! polygon data as needed by the drawing functions.
typedef struct poly_array {
    int len;    //!< number of entries
    int *data;  //!< a dynamic array: int[][2]
} poly_array_t;

/**
 * @brief read a whole file as string.
 * Read(fname:string):string
 *
 * @param J the JS context.
 */
static void f_Read(js_State *J) {
    const char *fname = js_tostring(J, 1);
    FILE *f;
    char *s;
    int n, t;

    f = fopen(fname, "rb");
    if (!f) {
        js_error(J, "Can't open file '%s': %s", fname, strerror(errno));
        return;
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fclose(f);
        js_error(J, "Can't seek in file '%s': %s", fname, strerror(errno));
        return;
    }

    n = ftell(f);
    if (n < 0) {
        fclose(f);
        js_error(J, "Can't tell in file '%s': %s", fname, strerror(errno));
        return;
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        fclose(f);
        js_error(J, "Can't seek in file '%s': %s", fname, strerror(errno));
        return;
    }

    s = malloc(n + 1);
    if (!s) {
        fclose(f);
        js_error(J, "out of memory");
        return;
    }

    t = fread(s, 1, n, f);
    if (t != n) {
        free(s);
        fclose(f);
        js_error(J, "Can't read data from file '%s': %s", fname, strerror(errno));
        return;
    }
    s[n] = 0;

    js_pushstring(J, s);
    free(s);
    fclose(f);
}

/**
 * @brief get directory listing.
 * List(dname:string):[f1:string, f1:string, ...]
 *
 * @param J the JS context.
 */
static void f_List(js_State *J) {
    const char *dirname = js_tostring(J, 1);
    DIR *dir = opendir(dirname);
    if (!dir) {
        js_error(J, "Cannot open dir '%s': %s", dirname, strerror(errno));
        return;
    }

    struct dirent *de;
    js_newarray(J);
    int idx = 0;
    while ((de = readdir(dir)) != NULL) {
        js_pushstring(J, de->d_name);
        js_setindex(J, -2, idx);
        idx++;
    }

    closedir(dir);
}

/**
 * @brief convert time_t into something like a javascript time string.
 *
 * @param t the time_t
 *
 * @return char* pointer to a static buffer with the conversion result.
 */
static char *f_formatTime(time_t *t) {
    static char buf[100];
    struct tm *tm = localtime(t);
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S.000", tm);
    return buf;
}

/**
 * @brief file/directory info.
 * Stat(name:string):{}
 *
 * @param J the JS context.
 */
static void f_Stat(js_State *J) {
    const char *name = js_tostring(J, 1);

    struct stat s;
    if (stat(name, &s) != 0) {
        js_error(J, "Cannot stat '%s': %s", name, strerror(errno));
        return;
    }

    char *drive = "A:";
    drive[0] += s.st_dev;

    js_newobject(J);
    {
        js_pushstring(J, drive);
        js_setproperty(J, -2, "drive");
        js_pushstring(J, f_formatTime(&s.st_atime));
        js_setproperty(J, -2, "atime");
        js_pushstring(J, f_formatTime(&s.st_ctime));
        js_setproperty(J, -2, "ctime");
        js_pushstring(J, f_formatTime(&s.st_mtime));
        js_setproperty(J, -2, "mtime");
        js_pushnumber(J, s.st_size);
        js_setproperty(J, -2, "size");
        js_pushnumber(J, s.st_blksize);
        js_setproperty(J, -2, "blksize");
        js_pushnumber(J, s.st_nlink);
        js_setproperty(J, -2, "nlink");

        js_pushboolean(J, S_ISREG(s.st_mode));
        js_setproperty(J, -2, "is_regular");
        js_pushboolean(J, S_ISDIR(s.st_mode));
        js_setproperty(J, -2, "is_directory");
        js_pushboolean(J, S_ISCHR(s.st_mode));
        js_setproperty(J, -2, "is_chardev");
        js_pushboolean(J, S_ISBLK(s.st_mode));
        js_setproperty(J, -2, "is_blockdev");
    }
}

/**
 * @brief print to stdout.
 * Print(t1, t2, ...)
 *
 * @param J the JS context.
 */
static void f_Print(js_State *J) {
    int i, top = js_gettop(J);
    for (i = 1; i < top; ++i) {
        const char *s = js_tostring(J, i);
        if (i > 1) {
            putc(' ', LOGSTREAM);
        }
        fputs(s, LOGSTREAM);
    }
    putc('\n', LOGSTREAM);
    js_pushundefined(J);
}

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
        js_error(J, "Array expected");
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
 * @brief stop DOjS loop().
 * Quit()
 *
 * @param J the JS context.
 */
static void f_Stop(js_State *J) { keep_running = false; }

/**
 * @brief do garbage collection.
 * Gc(report:boolean)
 *
 * @param J the JS context.
 */
static void f_Gc(js_State *J) {
    bool report = js_toboolean(J, 1);
    js_gc(J, report);
}

/**
 * @brief get number of possible colors.
 * NumColors():number
 *
 * @param J
 */
static void f_NumColors(js_State *J) { js_pushnumber(J, GrNumColors()); }

/**
 * @brief get name of screen mode.
 * GetScreenMode():string
 *
 * @param J
 */
static void f_GetScreenMode(js_State *J) { js_pushstring(J, ut_getModeString()); }

/**
 * @brief get name of grafics adapter.
 * GetScreenAdapter():string
 *
 * @param J
 */
static void f_GetScreenAdapter(js_State *J) { js_pushstring(J, ut_getAdapterString()); }

/**
 * @brief get number of remaining free colors.
 * NumFreeColors():number
 *
 * @param J the JS context.
 */
static void f_NumFreeColors(js_State *J) { js_pushnumber(J, GrNumFreeColors()); }

/**
 * @brief sleep for the given number of ms.
 * Sleep(ms:number)
 *
 * @param J the JS context.
 */
static void f_Sleep(js_State *J) { GrSleep(js_toint32(J, 1)); }

/**
 * @brief get current time in ms.
 * MsecTime():number
 *
 * @param J the JS context.
 */
static void f_MsecTime(js_State *J) { js_pushnumber(J, GrMsecTime()); }

/**
 * @brief get current framerate.
 * GetFramerate():number
 *
 * @param J the JS context.
 */
static void f_GetFramerate(js_State *J) { js_pushnumber(J, current_frame_rate); }

/**
 * @brief set wanted framerate.
 * SetFramerate(wanted:number)
 *
 * @param J the JS context.
 */
static void f_SetFramerate(js_State *J) { wanted_frame_rate = (float)js_tonumber(J, 1); }

/**
 * @brief get the width of the drawing area.
 * SizeX():number
 *
 * @param J the JS context.
 */
static void f_SizeX(js_State *J) { js_pushnumber(J, GrSizeX()); }

/**
 * @brief get the height of the drawing area.
 * SizeY():number
 *
 * @param J the JS context.
 */
static void f_SizeY(js_State *J) { js_pushnumber(J, GrSizeY()); }

/**
 * @brief get the max X coordinate on the drawing area.
 * MaxX():number
 *
 * @param J the JS context.
 */
static void f_MaxX(js_State *J) { js_pushnumber(J, GrMaxX()); }

/**
 * @brief get the max Y coordinate on the drawing area.
 * MaxY():number
 *
 * @param J the JS context.
 */
static void f_MaxY(js_State *J) { js_pushnumber(J, GrMaxY()); }

/**
 * @brief create object with the coordinates of the last arc drawn.
 *
 * res={"centerX":50,"centerY":50,"endX":71,"endY":29,"startX":80,"startY":50}
 *
 * @param J the JS context.
 */
static void f_arcReturn(js_State *J) {
    int startX, startY, endX, endY, centerX, centerY;
    GrLastArcCoords(&startX, &startY, &endX, &endY, &centerX, &centerY);
    js_newobject(J);
    {
        js_pushnumber(J, startX);
        js_setproperty(J, -2, "startX");
        js_pushnumber(J, startY);
        js_setproperty(J, -2, "startY");
        js_pushnumber(J, endX);
        js_setproperty(J, -2, "endX");
        js_pushnumber(J, endY);
        js_setproperty(J, -2, "endY");
        js_pushnumber(J, centerX);
        js_setproperty(J, -2, "centerX");
        js_pushnumber(J, centerY);
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
    GrColor *color = js_touserdata(J, 1, TAG_COLOR);

    GrClearScreen(*color);
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

    GrColor *color = js_touserdata(J, 3, TAG_COLOR);

    GrPlot(x, y, *color);
}

/**
 * @brief draw a line.
 *
 * Plot(x1:number, y1:number, x2:number, y2:number, c:Color)
 *
 * @param J the JS context.
 */
static void f_Line(js_State *J) {
    int x1 = js_toint16(J, 1);
    int y1 = js_toint16(J, 2);
    int x2 = js_toint16(J, 3);
    int y2 = js_toint16(J, 4);

    GrColor *color = js_touserdata(J, 5, TAG_COLOR);

    GrLine(x1, y1, x2, y2, *color);
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

    GrColor *color = js_touserdata(J, 5, TAG_COLOR);

    GrBox(x1, y1, x2, y2, *color);
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

    GrColor *color = js_touserdata(J, 4, TAG_COLOR);

    GrCircle(x, y, r, *color);
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

    GrColor *color = js_touserdata(J, 5, TAG_COLOR);

    GrEllipse(xc, yc, xa, ya, *color);
}

/**
 * @brief Draw a circle arc.
 * CircleArc(
 *    x:number,
 *    y:number,
 *    r:number,
 *    start:number,
 *    end:number,
 *    style:number,
 *    c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
 *
 * @param J the JS context.
 */
static void f_CircleArc(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
    int r = js_toint16(J, 3);

    int start = js_toint16(J, 4);
    int end = js_toint16(J, 5);
    int style = js_toint16(J, 6);

    GrColor *color = js_touserdata(J, 7, TAG_COLOR);

    GrCircleArc(x, y, r, start, end, style, *color);

    f_arcReturn(J);
}

/**
 * @brief Draw an ellipse arc.
 * EllipseArc(
 *    xc:number,
 *    yc:number,
 *    xa:number,
 *    start:number,
 *    end:number,
 *    style:number,
 *    c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
 *
 * @param J the JS context.
 */
static void f_EllipseArc(js_State *J) {
    int xc = js_toint16(J, 1);
    int yc = js_toint16(J, 2);
    int xa = js_toint16(J, 3);
    int ya = js_toint16(J, 4);

    int start = js_toint16(J, 5);
    int end = js_toint16(J, 6);
    int style = js_toint16(J, 7);

    GrColor *color = js_touserdata(J, 8, TAG_COLOR);

    GrEllipseArc(xc, yc, xa, ya, start, end, style, *color);

    f_arcReturn(J);
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

    GrColor *color = js_touserdata(J, 5, TAG_COLOR);

    GrFilledBox(x1, y1, x2, y2, *color);
}

/**
 * @brief draw a framed box.
 * FramedBox(x1:number, y1:number, x2:number, y2:number, [
 *    intcolor:Color,
 *    topcolor:Color,
 *    rightcolor:Color,
 *    bottomcolor:Color,
 *    leftcolor:Color,
 * ])
 *
 * @param J
 */
static void f_FramedBox(js_State *J) {
    int x1 = js_toint16(J, 1);
    int y1 = js_toint16(J, 2);
    int x2 = js_toint16(J, 3);
    int y2 = js_toint16(J, 4);

    int wdt = js_toint16(J, 5);

    GrFBoxColors colors;
    if (!js_isarray(J, 6)) {
        js_error(J, "Array expected");
        return;
    } else {
        GrColor *color;
        int len = js_getlength(J, 6);
        if (len < 5) {
            js_error(J, "Five Colors expected");
            return;
        }
        js_getindex(J, 6, 0);
        color = js_touserdata(J, -1, TAG_COLOR);
        colors.fbx_intcolor = *color;
        js_pop(J, 1);
        js_getindex(J, 6, 1);
        color = js_touserdata(J, -1, TAG_COLOR);
        colors.fbx_topcolor = *color;
        js_pop(J, 1);
        js_getindex(J, 6, 2);
        color = js_touserdata(J, -1, TAG_COLOR);
        colors.fbx_rightcolor = *color;
        js_pop(J, 1);
        js_getindex(J, 6, 3);
        color = js_touserdata(J, -1, TAG_COLOR);
        colors.fbx_bottomcolor = *color;
        js_pop(J, 1);
        js_getindex(J, 6, 4);
        color = js_touserdata(J, -1, TAG_COLOR);
        colors.fbx_leftcolor = *color;
        js_pop(J, 1);
    }

    GrFramedBox(x1, y1, x2, y2, wdt, &colors);
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

    GrColor *color = js_touserdata(J, 4, TAG_COLOR);

    GrFilledCircle(x, y, r, *color);
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

    GrColor *color = js_touserdata(J, 5, TAG_COLOR);

    GrFilledEllipse(xc, yc, xa, ya, *color);
}

/**
 * @brief Draw a filled circle arc.
 * FilledCircleArc(
 *    x:number,
 *    y:number,
 *    r:number,
 *    start:number,
 *    end:number,
 *    style:number,
 *    c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
 *
 * @param J the JS context.
 */
static void f_FilledCircleArc(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
    int r = js_toint16(J, 3);

    int start = js_toint16(J, 4);
    int end = js_toint16(J, 5);
    int style = js_toint16(J, 6);

    GrColor *color = js_touserdata(J, 7, TAG_COLOR);

    GrFilledCircleArc(x, y, r, start, end, style, *color);

    f_arcReturn(J);
}

/**
 * @brief Draw a filled ellipse arc.
 * FilledEllipseArc(
 *    xc:number,
 *    yc:number,
 *    xa:number,
 *    start:number,
 *    end:number,
 *    style:number,
 *    c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
 *
 * @param J the JS context.
 */
static void f_FilledEllipseArc(js_State *J) {
    int xc = js_toint16(J, 1);
    int yc = js_toint16(J, 2);
    int xa = js_toint16(J, 3);
    int ya = js_toint16(J, 4);

    int start = js_toint16(J, 5);
    int end = js_toint16(J, 6);
    int style = js_toint16(J, 7);

    GrColor *color = js_touserdata(J, 8, TAG_COLOR);

    GrFilledEllipseArc(xc, yc, xa, ya, start, end, style, *color);

    f_arcReturn(J);
}

/**
 * @brief do a flood fill.
 * FloodFill(x:number, y:number, border:Color, c:Color)
 *
 * @param J the JS context.
 */
static void f_FloodFill(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    GrColor *border = js_touserdata(J, 3, TAG_COLOR);
    GrColor *color = js_touserdata(J, 4, TAG_COLOR);

    GrFloodFill(x, y, *border, *color);
}

/**
 * @brief do a flood spill.
 * FloodSpill(x1:number, y1:number, x2:number, y2:number, border:Color, c:Color)
 *
 * @param J the JS context.
 */
static void f_FloodSpill(js_State *J) {
    int x1 = js_toint16(J, 1);
    int y1 = js_toint16(J, 2);
    int x2 = js_toint16(J, 3);
    int y2 = js_toint16(J, 4);

    GrColor *border = js_touserdata(J, 5, TAG_COLOR);
    GrColor *color = js_touserdata(J, 6, TAG_COLOR);

    GrFloodSpill(x1, y1, x2, y2, *border, *color);
}

/**
 * @brief do a flood spill2.
 * FloodSpill2(x1:number, y1:number, x2:number, y2:number, old1:Color,
 * new1:Color, old2:Color, new2:Color)
 *
 * @param J the JS context.
 */
static void f_FloodSpill2(js_State *J) {
    int x1 = js_toint16(J, 1);
    int y1 = js_toint16(J, 2);
    int x2 = js_toint16(J, 3);
    int y2 = js_toint16(J, 4);

    GrColor *old_c1 = js_touserdata(J, 5, TAG_COLOR);
    GrColor *new_c1 = js_touserdata(J, 6, TAG_COLOR);

    GrColor *old_c2 = js_touserdata(J, 7, TAG_COLOR);
    GrColor *new_c2 = js_touserdata(J, 8, TAG_COLOR);

    GrFloodSpill2(x1, y1, x2, y2, *old_c1, *new_c1, *old_c2, *new_c2);
}

/**
 * @brief draw a polyline.
 * PolyLine(c:Color, [[x1, x2], [..], [xN, yN]])
 *
 * @param J the JS context.
 */
static void f_PolyLine(js_State *J) {
    poly_array_t *array = f_convertArray(J, 1);
    GrColor *color = js_touserdata(J, 2, TAG_COLOR);

    GrPolyLine(array->len, (int(*)[2])array->data, *color);

    f_freeArray(array);
}

/**
 * @brief draw a polygon.
 * Polygon(c:Color, [[x1, x2], [..], [xN, yN]])
 *
 * @param J the JS context.
 */
static void f_Polygon(js_State *J) {
    poly_array_t *array = f_convertArray(J, 1);
    GrColor *color = js_touserdata(J, 2, TAG_COLOR);

    GrPolygon(array->len, (int(*)[2])array->data, *color);

    f_freeArray(array);
}

/**
 * @brief draw a filled polygon.
 * FilledPolygon(c:Color, [[x1, x2], [..], [xN, yN]])
 *
 * @param J the JS context.
 */
static void f_FilledPolygon(js_State *J) {
    poly_array_t *array = f_convertArray(J, 1);
    GrColor *color = js_touserdata(J, 2, TAG_COLOR);

    GrFilledPolygon(array->len, (int(*)[2])array->data, *color);

    f_freeArray(array);
}

/**
 * @brief draw a filled convex polygon.
 * FilledConvexPolygon(c:Color, [[x1, x2], [..], [xN, yN]])
 *
 * @param J the JS context.
 */
static void f_FilledConvexPolygon(js_State *J) {
    poly_array_t *array = f_convertArray(J, 1);
    GrColor *color = js_touserdata(J, 2, TAG_COLOR);

    GrFilledConvexPolygon(array->len, (int(*)[2])array->data, *color);

    f_freeArray(array);
}

/*
TODO:
     int  GrMouseBlock(GrContext *c,int x1,int y1,int x2,int y2);
     void GrMouseUnBlock(int return_value_from_GrMouseBlock);
*/

/**
 * @brief set mosue speed
 * MouseSetSpeed(spmul:number, spdiv:number)
 *
 * @param J the JS context.
 */
static void f_MouseSetSpeed(js_State *J) {
    int spmul = js_toint16(J, 1);
    int spdiv = js_toint16(J, 2);

    GrMouseSetSpeed(spmul, spdiv);
}

/**
 * @brief set mosue acceleration
 * MouseSetAccel(thresh:number, accel:number)
 *
 * @param J the JS context.
 */
static void f_MouseSetAccel(js_State *J) {
    int thresh = js_toint16(J, 1);
    int accel = js_toint16(J, 2);

    GrMouseSetAccel(thresh, accel);
}

/**
 * @brief set mouse limits
 * MouseSetLimits(x1:number, y1:number, x2:number, y2:number)
 *
 * @param J the JS context.
 */
static void f_MouseSetLimits(js_State *J) {
    int x1 = js_toint16(J, 1);
    int y1 = js_toint16(J, 2);
    int x2 = js_toint16(J, 3);
    int y2 = js_toint16(J, 4);

    GrMouseSetLimits(x1, y1, x2, y2);
}

/**
 * @brief get mouse limits
 * MouseGetLimits():{"x1":XXX, "y1":XXX, "x2":XXX, "y2":XXX}
 *
 * @param J the JS context.
 */
static void f_MouseGetLimits(js_State *J) {
    int x1, x2, y1, y2;
    GrMouseGetLimits(&x1, &y1, &x2, &y2);

    js_newobject(J);
    {
        js_pushnumber(J, x1);
        js_setproperty(J, -2, "x1");
        js_pushnumber(J, x2);
        js_setproperty(J, -2, "x2");
        js_pushnumber(J, y1);
        js_setproperty(J, -2, "y1");
        js_pushnumber(J, y2);
        js_setproperty(J, -2, "y2");
    }
}

/**
 * @brief mose mouse cursor
 * MouseWarp(x:number, y:number)
 *
 * @param J the JS context.
 */
static void f_MouseWarp(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    GrMouseWarp(x, y);
}

/**
 * @brief show hide mouse cursor
 * MouseShowCursor(b:boolean)
 *
 * @param J the JS context.
 */
static void f_MouseShowCursor(js_State *J) {
    bool show = js_toboolean(J, 1);
    DEBUGF("Cursor=%d\n", show);
    if (show) {
        GrMouseDisplayCursor();
    } else {
        GrMouseEraseCursor();  // TODO: does not work?
    }
}

/**
 * @brief change mode of the cursor.
 * MouseSetCursorMode(MOUSE.Mode.NORMAL) or
 * MouseSetCursorMode(MOUSE.Mode.RUBBER,xanchor,yanchor,GrColor) or
 * MouseSetCursorMode(MOUSE.Mode.LINE,xanchor,yanchor,GrColor) or
 * MouseSetCursorMode(MOUSE.Mode.BOX,dx1,dy1,dx2,dy2,GrColor) or
 *
 * @param J the JS context.
 */
static void f_MouseSetCursorMode(js_State *J) {
    int mode = js_toint16(J, 1);
    switch (mode) {
        case GR_M_CUR_NORMAL:
            GrMouseSetCursorMode(mode);
            break;
        case GR_M_CUR_RUBBER:
        case GR_M_CUR_LINE: {
            int xanchor = js_toint16(J, 2);
            int yanchor = js_toint16(J, 3);
            GrColor *color = js_touserdata(J, 4, TAG_COLOR);

            GrMouseSetCursorMode(mode, xanchor, yanchor, color);
        } break;
        case GR_M_CUR_BOX: {
            int dx1 = js_toint16(J, 2);
            int dy1 = js_toint16(J, 3);
            int dx2 = js_toint16(J, 4);
            int dy2 = js_toint16(J, 5);
            GrColor *color = js_touserdata(J, 6, TAG_COLOR);

            GrMouseSetCursorMode(mode, dx1, dy1, dx2, dy2, color);
        } break;
    }
}

/**
 * @brief check if the cursor is visible
 * MouseCursorIsDisplayed():boolean
 *
 * @param J the JS context.
 */
static void f_MouseCursorIsDisplayed(js_State *J) { js_pushboolean(J, GrMouseCursorIsDisplayed()); }

/**
 * @brief set mouse pointer colors.
 * MouseSetColors(fg:Color, bg:Color)
 *
 * @param J the JS context.
 */
static void f_MouseSetColors(js_State *J) {
    GrColor *fg = js_touserdata(J, 1, TAG_COLOR);
    GrColor *bg = js_touserdata(J, 2, TAG_COLOR);

    GrMouseSetColors(*fg, *bg);
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

    GrColor *fg = js_touserdata(J, 4, TAG_COLOR);
    GrColor *bg = js_touserdata(J, 5, TAG_COLOR);

    GrTextXY(x, y, (char *)str, *fg, *bg);
}

/**
 * @brief change the exit-key for the script. Default: ESCAPE.
 * SetExitKey(key:number)
 *
 * @param J the JS context.
 */
static void f_SetExitKey(js_State *J) { exit_key = js_toint32(J, 1); }

/**
 * @brief save current screen to file.
 * SaveBmpImage(fname:string)
 *
 * @param J the JS context.
 */
static void f_SaveBmpImage(js_State *J) {
    const char *fname = js_tostring(J, 1);

    GrContext *ctx = GrScreenContext();

    if (!GrSaveBmpImage((char *)fname, ctx, 0, 0, ctx->gc_xmax, ctx->gc_ymax)) {
        js_error(J, "Can't save screen to BMP file '%s'", fname);
    }
}

/**
 * @brief save current screen to file.
 * SavePngImage(fname:string)
 *
 * @param J the JS context.
 */
static void f_SavePngImage(js_State *J) {
    const char *fname = js_tostring(J, 1);

    if (GrSaveContextToPng(GrScreenContext(), (char *)fname) != 0) {
        js_error(J, "Can't save screen to PNG file '%s'", fname);
    }
}

#ifdef DEBUG_ENABLED
/**
 * @brief test function.
 *
 * @param J the JS context.
 */
static void f_Test(js_State *J) {
    /*
    if (!js_isarray(J, 1)) {
      js_error(J, "Array expected");
    } else {
      int len = js_getlength(J, 1);
      DEBUGF("Array length = %d\n", len);
      for (int i = 0; i < len; i++) {
        js_getindex(J, 1, i);
        DEBUGF("  %d := %d\n", i, js_toint16(J, -1));
        js_pop(J, 1);
      }
    }
    */
    poly_array_t *array = f_convertArray(J, 1);
    if (array) {
        DEBUGF("Success, len=%d\n", array->len);
    } else {
        DEBUG("Ups!\n");
    }
}
#endif

/***********************
** exported functions **
***********************/
/**
 * @brief initialize grx subsystem.
 *
 * @param J VM state.
 */
void init_funcs(js_State *J) {
    // define some global properties
    PROPDEF_B(J, sound_available, "SOUND_AVAILABLE");
    PROPDEF_B(J, synth_available, "SYNTH_AVAILABLE");
    PROPDEF_B(J, mouse_available, "MOUSE_AVAILABLE");
    PROPDEF_B(J, ipx_available, "IPX_AVAILABLE");

    // define global functions
    FUNCDEF(J, f_Read, "Read", 1);
    FUNCDEF(J, f_List, "List", 1);
    FUNCDEF(J, f_Stat, "Stat", 1);
    FUNCDEF(J, f_Print, "Print", 0);
    FUNCDEF(J, f_Stop, "Stop", 0);
    FUNCDEF(J, f_Gc, "Gc", 1);
    FUNCDEF(J, f_Sleep, "Sleep", 1);
    FUNCDEF(J, f_MsecTime, "MsecTime", 0);
    FUNCDEF(J, f_SetFramerate, "SetFramerate", 1);
    FUNCDEF(J, f_GetFramerate, "GetFramerate", 0);

    FUNCDEF(J, f_GetScreenAdapter, "GetScreenAdapter", 0);
    FUNCDEF(J, f_GetScreenMode, "GetScreenMode", 0);
    FUNCDEF(J, f_NumColors, "NumColors", 0);
    FUNCDEF(J, f_NumFreeColors, "NumFreeColors", 0);
    FUNCDEF(J, f_SizeX, "SizeX", 0);
    FUNCDEF(J, f_SizeY, "SizeY", 0);
    FUNCDEF(J, f_MaxX, "MaxX", 0);
    FUNCDEF(J, f_MaxY, "MaxY", 0);

    FUNCDEF(J, f_ClearScreen, "ClearScreen", 1);
    FUNCDEF(J, f_TextXY, "TextXY", 5);

    FUNCDEF(J, f_Plot, "Plot", 3);
    FUNCDEF(J, f_Line, "Line", 5);
    FUNCDEF(J, f_Box, "Box", 5);
    FUNCDEF(J, f_Circle, "Circle", 4);
    FUNCDEF(J, f_Ellipse, "Ellipse", 5);
    FUNCDEF(J, f_CircleArc, "CircleArc", 7);
    FUNCDEF(J, f_EllipseArc, "EllipseArc", 8);
    FUNCDEF(J, f_PolyLine, "PolyLine", 2);
    FUNCDEF(J, f_Polygon, "Polygon", 2);

    FUNCDEF(J, f_FilledBox, "FilledBox", 5);
    FUNCDEF(J, f_FramedBox, "FramedBox", 6);
    FUNCDEF(J, f_FilledCircle, "FilledCircle", 4);
    FUNCDEF(J, f_FilledEllipse, "FilledEllipse", 5);
    FUNCDEF(J, f_FilledCircleArc, "FilledCircleArc", 7);
    FUNCDEF(J, f_FilledEllipseArc, "FilledEllipseArc", 8);
    FUNCDEF(J, f_FilledPolygon, "FilledPolygon", 2);
    FUNCDEF(J, f_FilledConvexPolygon, "FilledConvexPolygon", 2);

    FUNCDEF(J, f_FloodFill, "FloodFill", 4);
    FUNCDEF(J, f_FloodSpill, "FloodSpill", 6);
    FUNCDEF(J, f_FloodSpill2, "FloodSpill2", 8);

    FUNCDEF(J, f_MouseSetSpeed, "MouseSetSpeed", 2);
    FUNCDEF(J, f_MouseSetAccel, "MouseSetAccel", 2);
    FUNCDEF(J, f_MouseSetLimits, "MouseSetLimits", 4);
    FUNCDEF(J, f_MouseGetLimits, "MouseGetLimits", 0);
    FUNCDEF(J, f_MouseWarp, "MouseWarp", 2);
    FUNCDEF(J, f_MouseShowCursor, "MouseShowCursor", 1);
    FUNCDEF(J, f_MouseCursorIsDisplayed, "MouseCursorIsDisplayed", 0);
    FUNCDEF(J, f_MouseSetColors, "MouseSetColors", 2);
    FUNCDEF(J, f_MouseSetCursorMode, "MouseSetCursorMode", 1);

    FUNCDEF(J, f_SetExitKey, "SetExitKey", 1);
    FUNCDEF(J, f_SaveBmpImage, "SaveBmpImage", 1);
    FUNCDEF(J, f_SavePngImage, "SavePngImage", 1);

#ifdef DEBUG_ENABLED
    FUNCDEF(J, f_Test, "TestFunc", 1);
#endif
}
