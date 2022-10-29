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
#include "a3d.h"
#include "util.h"
#include "zipfile.h"
#include "zbuffer.h"

/**************
** prototype **
**************/
void init_al3d(js_State *J);

/***********************
** exported functions **
***********************/
void init_al3d(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    if (ut_file_exists(DOjS.jsboot)) {
        dojs_do_zipfile(J, DOjS.jsboot, JSINC_A3D);
    } else {
        dojs_do_file(J, JSINC_A3D);
    }

    init_a3d(J);
    init_zbuffer(J);
}
