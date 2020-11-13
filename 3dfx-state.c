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

#include <glide.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>

#include "3dfx-state.h"
#include "DOjS.h"
#include "util.h"

/*********************
** static functions **
*********************/
/**
 * @brief finalize and free resources.
 *
 * @param J VM state.
 */
static void FxState_Finalize(js_State *J, void *s) { free(s); }

/**
 * @brief new FxState()
 *
 * @param J VM state.
 */
static void new_FxState(js_State *J) {
    FxI32 state_size;
    void *s;

    grGet(GR_GLIDE_STATE_SIZE, 4, &state_size);
    s = malloc(state_size);

    if (!s) {
        JS_ENOMEM(J);
        return;
    }
    grGlideGetState(s);

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_FXSTATE, s, FxState_Finalize);
}

/**
 * @brief s.Set()
 *
 * @param J VM state.
 */
static void FxState_Set(js_State *J) {
    void *s = js_touserdata(J, 0, TAG_FXSTATE);
    grGlideSetState(s);
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize FxState class.
 *
 * @param J VM state.
 */
void init_fxstate(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    { NPROTDEF(J, FxState, Set, 0); }
    CTORDEF(J, new_FxState, TAG_FXSTATE, 0);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
