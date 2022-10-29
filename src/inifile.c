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
#include <stdlib.h>

#include "DOjS.h"
#include "inifile.h"
#include "ini.h"

/*********************
** static functions **
*********************/
/**
 * @brief finalize a zbuffer and free resources.
 *
 * @param J VM state.
 */
static void IniFile_Finalize(js_State *J, void *data) {
    ini_t *ini = (ini_t *)data;
    ini_free(ini);
}

/**
 * @brief create new zbuffer for Bitmap().
 * new ZBuffer(bm:Bitmap)
 *
 * @param J VM state.
 */
static void new_IniFile(js_State *J) {
    NEW_OBJECT_PREP(J);
    const char *fname = js_tostring(J, 1);
    ini_t *ini = ini_load(fname);
    if (!ini) {
        js_error(J, "cannot open file '%s'", fname);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_INIFILE, ini, IniFile_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief set this zbuffer as current.
 * zb.Set()
 *
 * @param J VM state.
 */
static void IniFile_Get(js_State *J) {
    const char *p1 = js_tostring(J, 1);
    const char *p2 = js_tostring(J, 2);

    ini_t *ini = js_touserdata(J, 0, TAG_INIFILE);
    const char *val;
    if (js_isstring(J, 2)) {
        val = ini_get(ini, p1, p2);
    } else {
        val = ini_get(ini, NULL, p1);
    }
    if (val) {
        js_pushstring(J, val);
    } else {
        js_pushnull(J);
    }
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize zbuffer subsystem.
 *
 * @param J VM state.
 */
void init_inifile(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    { NPROTDEF(J, IniFile, Get, 2); }
    CTORDEF(J, new_IniFile, TAG_INIFILE, 1);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
