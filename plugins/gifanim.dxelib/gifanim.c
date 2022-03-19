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
#include <errno.h>
#include <mujs.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "DOjS.h"

#include "AnimatedGIF-1.4.3/src/AnimatedGIF.h"
#include "AnimatedGIF-1.4.3/src/gif.inl"

void init_gifanim(js_State *J);

/************
** defines **
************/
#define TAG_GIF "GIFAnim"  //!< pointer tag

/************
** structs **
************/
//! file userdata definition
typedef struct __gifanim {
    GIFIMAGE gif;   //!< the gif
    bool is_open;   //!< open flag
    uint16_t x, y;  //!< draw position
    bool skip;      //!< skip this frame.
} gifanim_t;

/*********************
** static functions **
*********************/

/**
 * @brief draw a RGB565 pixel as RGB8888
 *
 * @param x
 * @param y
 * @param color
 * @param gif
 */
static void GIF_drawPixel(uint32_t x, uint32_t y, uint16_t color, gifanim_t *gif) {
    int r = (color >> 8) & 0xFF;
    int g = (color >> 3) & 0xFC;
    int b = (color << 3) & 0xFF;

    putpixel(DOjS.current_bm, x + gif->x, y + gif->y, makeacol32(r, g, b, 255));
}

/**
 * @brief render current frame
 *
 * @param pDraw callback struct from AnimatedGIF
 */
static void GIF_DrawCallback(GIFDRAW *pDraw) {
    gifanim_t *user = (gifanim_t *)pDraw->pUser;

    if (!user->skip) {
        int y = pDraw->iY + pDraw->y;  // current line

        uint8_t *s = pDraw->pPixels;

        uint16_t *usPalette = pDraw->pPalette;
        if (pDraw->ucHasTransparency) {  // if transparency used
            for (int x = 0; x < pDraw->iWidth; x++) {
                int c = *s++;
                if (c != pDraw->ucTransparent) {
                    GIF_drawPixel(x, y, usPalette[c], user);
                }
            }
        } else {
            for (int x = 0; x < pDraw->iWidth; x++) {
                GIF_drawPixel(x, y, usPalette[*s++], user);
            }
        }
    }
} /* GIFDraw() */

/**
 * @brief finalize a file and free resources.
 *
 * @param J VM state.
 */
static void GIF_Finalize(js_State *J, void *data) {
    gifanim_t *g = (gifanim_t *)data;

    if (g->is_open) {
        GIF_close(&g->gif);
        g->is_open = false;
    }
    free(g);
}

/**
 * @brief open a GIF.
 * gif = new GIF(filename:str)
 *
 * @param J VM state.
 */
static void new_GIF(js_State *J) {
    GIFINFO info;

    NEW_OBJECT_PREP(J);

    gifanim_t *g = malloc(sizeof(gifanim_t));
    if (!g) {
        JS_ENOMEM(J);
        return;
    }

    const char *fname = js_tostring(J, 1);
    if (!GIF_openFile(&g->gif, fname, GIF_DrawCallback)) {
        js_error(J, "Could not open GIF '%s'.", fname);
        free(g);
        return;
    }
    g->is_open = true;

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_GIF, g, GIF_Finalize);

    GIF_getInfo(&g->gif, &info);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, GIF_getCanvasWidth(&g->gif));
    js_defproperty(J, -2, "width", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, GIF_getCanvasHeight(&g->gif));
    js_defproperty(J, -2, "height", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, info.iFrameCount);
    js_defproperty(J, -2, "frameCount", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, info.iDuration);
    js_defproperty(J, -2, "duration", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, info.iMaxDelay);
    js_defproperty(J, -2, "maxDelay", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, info.iMinDelay);
    js_defproperty(J, -2, "minDelay", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief close GIF.
 * gif.Close()
 *
 * @param J VM state.
 */
static void GIF_Close(js_State *J) {
    gifanim_t *g = js_touserdata(J, 0, TAG_GIF);

    if (g->is_open) {
        GIF_close(&g->gif);
        g->is_open = false;
    }
}

/**
 * @brief get GIF comment (if any)
 * gif.Close():string
 *
 * @param J VM state.
 */
static void GIF_GetComment(js_State *J) {
    gifanim_t *g = js_touserdata(J, 0, TAG_GIF);
    char *comment = malloc(g->gif.sCommentLen);
    if (!comment) {
        JS_ENOMEM(J);
        return;
    }
    GIF_getComment(&g->gif, comment);
    js_pushstring(J, comment);
    free(comment);
}

/**
 * @brief play next frame
 * gif.PlayFrame(x:int, y:int):number
 *
 * @param J VM state.
 */
static void GIF_PlayFrame(js_State *J) {
    int nextDelay;
    gifanim_t *g = js_touserdata(J, 0, TAG_GIF);

    g->x = js_toint16(J, 1);
    g->y = js_toint16(J, 2);

    g->skip = false;
    int res = GIF_playFrame(&g->gif, &nextDelay, g);
    if (res < 0) {
        js_error(J, "Error decoding frame");
    } else if (res == 0) {
        js_pushnumber(J, -1);
    } else {
        js_pushnumber(J, nextDelay);
    }
}

/**
 * @brief play next frame
 * gif.SkipFrame():number
 *
 * @param J VM state.
 */
static void GIF_SkipFrame(js_State *J) {
    int nextDelay;
    gifanim_t *g = js_touserdata(J, 0, TAG_GIF);

    g->skip = true;
    int res = GIF_playFrame(&g->gif, &nextDelay, g);
    if (res < 0) {
        js_error(J, "Error decoding frame");
    } else if (res == 0) {
        js_pushnumber(J, -1);
    } else {
        js_pushnumber(J, nextDelay);
    }
}
/*********************
** public functions **
*********************/
/**
 * @brief initialize neural subsystem.
 *
 * @param J VM state.
 */
void init_gifanim(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, GIF, Close, 0);
        NPROTDEF(J, GIF, GetComment, 0);
        NPROTDEF(J, GIF, PlayFrame, 2);
        NPROTDEF(J, GIF, SkipFrame, 0);
    }
    CTORDEF(J, new_GIF, TAG_GIF, 1);
}
