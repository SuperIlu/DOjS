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

/**************
** prototype **
**************/
void init_dxetest(js_State *J);
void shutdown_dxetest(void);

/**
 * @brief HelloWorld() dummy function
 *
 * @param J VM state.
 */
static void f_HelloWorld(js_State *J) {
    char buff[1024];
    const char *para = js_tostring(J, 1);

    snprintf(buff, sizeof(buff) - 1, "Hello World! %s", para);

    js_pushstring(J, buff);
}

/**
 * @brief initialize module.
 *
 * @param J VM state.
 */
void init_dxetest(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);
    NFUNCDEF(J, HelloWorld, 1);
}

/**
 * @brief shutdown module.
 *
 * @param J VM state.
 */
void shutdown_dxetest() { LOGF("%s\n", __PRETTY_FUNCTION__); }
