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

#include <mujs.h>
#include <stdio.h>

#include "DOjS.h"

#include "noise1234.h"
#include "simplexnoise1234.h"

/**************
** prototype **
**************/
void init_noise(js_State *J);

static void f_Noise(js_State *J) {
    float ret = 1.0;
    if (js_isnumber(J, 1), js_isnumber(J, 2), js_isnumber(J, 3), js_isnumber(J, 4)) {
        ret += noise4(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4));
    } else if (js_isnumber(J, 1), js_isnumber(J, 2), js_isnumber(J, 3)) {
        ret += noise3(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3));
    } else if (js_isnumber(J, 1), js_isnumber(J, 2), js_isnumber(J, 3)) {
        ret += noise2(js_tonumber(J, 1), js_tonumber(J, 2));
    } else {
        ret += noise1(js_tonumber(J, 1));
    }
    js_pushnumber(J, ret / 2.0f);
}

static void f_SNoise(js_State *J) {
    float ret = 1.0;
    if (js_isnumber(J, 1), js_isnumber(J, 2), js_isnumber(J, 3), js_isnumber(J, 4)) {
        ret += snoise4(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4));
    } else if (js_isnumber(J, 1), js_isnumber(J, 2), js_isnumber(J, 3)) {
        ret += snoise3(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3));
    } else if (js_isnumber(J, 1), js_isnumber(J, 2), js_isnumber(J, 3)) {
        ret += snoise2(js_tonumber(J, 1), js_tonumber(J, 2));
    } else {
        ret += snoise1(js_tonumber(J, 1));
    }
    js_pushnumber(J, ret / 2.0f);
}

static void f_PNoise(js_State *J) {
    float ret = 1.0;
    if (js_isnumber(J, 1), js_isnumber(J, 2), js_isnumber(J, 3), js_isnumber(J, 4), js_isnumber(J, 5), js_isnumber(J, 6), js_isnumber(J, 7), js_isnumber(J, 8)) {
        ret += pnoise4(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4), js_tonumber(J, 5), js_tonumber(J, 6), js_tonumber(J, 7), js_tonumber(J, 8));
    } else if (js_isnumber(J, 1), js_isnumber(J, 2), js_isnumber(J, 3), js_isnumber(J, 4), js_isnumber(J, 5), js_isnumber(J, 6)) {
        ret += pnoise3(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4), js_tonumber(J, 5), js_tonumber(J, 6));
    } else if (js_isnumber(J, 1), js_isnumber(J, 2), js_isnumber(J, 3), js_isnumber(J, 4)) {
        ret += pnoise2(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4));
    } else {
        ret += pnoise1(js_tonumber(J, 1), js_tonumber(J, 2));
    }
    js_pushnumber(J, ret / 2.0f);
}

/**
 * @brief initialize module.
 *
 * @param J VM state.
 */
void init_noise(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);
    NFUNCDEF(J, Noise, 4);
    NFUNCDEF(J, PNoise, 4);
    NFUNCDEF(J, SNoise, 4);
}
