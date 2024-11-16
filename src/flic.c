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

#include "flic.h"

#include <allegro.h>
#if WINDOWS==1
#include <winalleg.h>
#endif
#include <mujs.h>

#include "DOjS.h"
#include "zipfile.h"

/*********************
** static functions **
*********************/

/**
 * @brief open FLI/FLC file
 *
 * @param J VM state.
 */
static void f_FlicOpen(js_State *J) {
    const char *fname = js_tostring(J, 1);

    close_fli();

    if (open_fli(fname) == FLI_ERROR) {
        js_error(J, "Could not open FLC/FLI %s", fname);
        return;
    }

    next_fli_frame(true);

    PROPDEF_N(J, fli_bitmap->w, "FLIC_WIDTH");
    PROPDEF_N(J, fli_bitmap->h, "FLIC_HEIGHT");
}

/**
 * @brief play animation
 *
 * @param J VM state.
 */
static void f_FlicPlay(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
    bool loop = js_toboolean(J, 3);

    if (fli_timer <= 0) {
        return;
    }

    /* update the palette */
    if (fli_pal_dirty_from <= fli_pal_dirty_to) {
        set_palette_range(fli_palette, fli_pal_dirty_from, fli_pal_dirty_to, true);
    }

    /* update the screen */
    blit(fli_bitmap, DOjS.current_bm,    // src and dest
         0, fli_bmp_dirty_from,          // src x/y
         x, y,                           // dest x/y
         fli_bitmap->w, fli_bitmap->h);  // width and height
    int ret = next_fli_frame(loop);

    if (ret == FLI_EOF) {
        js_pushnumber(J, -1);
    } else {
        js_pushnumber(J, fli_frame);
    }
}

/**
 * @brief close animation
 *
 * @param J VM state.
 */
static void f_FlicClose(js_State *J) { close_fli(); }

/***********************
** exported functions **
***********************/
/**
 * @brief initialize midi subsystem.
 *
 * @param J VM state.
 */
void init_flic(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    NFUNCDEF(J, FlicClose, 0);
    NFUNCDEF(J, FlicOpen, 1);
    NFUNCDEF(J, FlicPlay, 3);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief shutdown MIDI subsystem.
 */
void shutdown_flic() {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    close_fli();

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
