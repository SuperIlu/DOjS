/*
MIT License

Copyright (c) 2019-2020 Andre Seidelt <superilu@yahoo.com>

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
#include "zbuffer.h"

/*********************
** static functions **
*********************/
/**
 * @brief finalize a zbuffer and free resources.
 *
 * @param J VM state.
 */
static void ZBuffer_Finalize(js_State *J, void *data) {
    ZBUFFER *zb = (ZBUFFER *)data;
    destroy_zbuffer(zb);
}

/**
 * @brief create new zbuffer for Bitmap().
 * new ZBuffer(bm:Bitmap)
 *
 * @param J VM state.
 */
static void new_ZBuffer(js_State *J) {
    NEW_OBJECT_PREP(J);
    BITMAP *bm;
    if (js_isuserdata(J, 1, TAG_BITMAP)) {
        bm = js_touserdata(J, 1, TAG_BITMAP);
    } else {
        bm = DOjS.current_bm;
    }
    ZBUFFER *zb = create_zbuffer(bm);
    if (!zb) {
        JS_ENOMEM(J);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_ZBUFFER, zb, ZBuffer_Finalize);

    // add properties
    js_pushnumber(J, bm->w);
    js_defproperty(J, -2, "width", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, bm->h);
    js_defproperty(J, -2, "height", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
}

/**
 * @brief set this zbuffer as current.
 * zb.Set()
 *
 * @param J VM state.
 */
static void ZBuffer_Set(js_State *J) {
    ZBUFFER *zb = js_touserdata(J, 0, TAG_ZBUFFER);
    set_zbuffer(zb);

    // put this zbuffer JS object into a global variable as long as it is the current zbuffer.
    js_copy(J, 0);
    js_setglobal(J, "__zbuffer");
}

/**
 * @brief clear this zbuffer with z.
 * zb.Clear(z)
 *
 * @param J VM state.
 */
static void ZBuffer_Clear(js_State *J) {
    ZBUFFER *zb = js_touserdata(J, 0, TAG_ZBUFFER);
    float z = js_tonumber(J, 1);
    clear_zbuffer(zb, z);
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize zbuffer subsystem.
 *
 * @param J VM state.
 */
void init_zbuffer(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, ZBuffer, Set, 0);
        NPROTDEF(J, ZBuffer, Clear, 1);
    }
    CTORDEF(J, new_ZBuffer, TAG_ZBUFFER, 2);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
