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
#include "a3d.h"
#include "bitmap.h"

/*********************
** static functions **
*********************/
/**
 * expects an array w/ 5 elements on the stack and returns a V3D.
 */
static V3D_f *array_to_v3d(js_State *J, int idx) {
    V3D_f *v = malloc(sizeof(V3D_f));
    if (!v) {
        return NULL;
    }

    js_getindex(J, idx, 0);
    v->x = js_tonumber(J, -1);
    js_pop(J, 1);
    js_getindex(J, idx, 1);
    v->y = js_tonumber(J, -1);
    js_pop(J, 1);
    js_getindex(J, idx, 2);
    v->z = js_tonumber(J, -1);
    js_pop(J, 1);
    js_getindex(J, idx, 3);
    v->u = js_tonumber(J, -1);
    js_pop(J, 1);
    js_getindex(J, idx, 4);
    v->v = js_tonumber(J, -1);
    js_pop(J, 1);

    js_getindex(J, idx, 5);
    v->c = js_toint32(J, -1);
    js_pop(J, 1);

    return v;
}

static V3D_f **v3d_array(js_State *J, int idx, int *vc) {
    if (!js_isarray(J, idx)) {
        *vc = 0;
        js_error(J, "Array expected");
        return NULL;
    } else {
        int len = js_getlength(J, idx);
        V3D_f **array = malloc(len * sizeof(V3D_f *));
        if (!array) {
            return NULL;
        }
        for (int i = 0; i < len; i++) {
            js_getindex(J, idx, i);
            {
                int vlen = js_getlength(J, -1);
                if (vlen < 6) {
                    js_error(J, "V3D must have 6 values");
                    free(array);
                    *vc = 0;
                    return NULL;
                }

                array[i] = array_to_v3d(J, -1);
            }
            js_pop(J, 1);
        }
        *vc = len;
        return array;
    }
}

static void free_v3d(V3D_f **v, int vc) {
    if (v) {
        for (int i = 0; i < vc; i++) {
            free(v[i]);
        }

        free(v);
    }
}

static BITMAP *bitmap_or_null(js_State *J, int idx) {
    BITMAP *texture = NULL;
    if (js_isuserdata(J, idx, TAG_BITMAP)) {
        texture = js_touserdata(J, idx, TAG_BITMAP);
    }
    return texture;
}

static void f_Triangle3D(js_State *J) {
    int type = js_toint16(J, 1);
    BITMAP *texture = bitmap_or_null(J, 2);

    V3D_f *v1 = array_to_v3d(J, 3);
    V3D_f *v2 = array_to_v3d(J, 4);
    V3D_f *v3 = array_to_v3d(J, 5);

    if (v1 && v2 && v3) {
        triangle3d_f(cur, type, texture, v1, v2, v3);
    } else {
        js_error(J, "Cannot convert vertex");
    }

    if (v1) {
        free(v1);
    }
    if (v2) {
        free(v2);
    }
    if (v3) {
        free(v3);
    }
}

static void f_Quad3D(js_State *J) {
    int type = js_toint16(J, 1);
    BITMAP *texture = bitmap_or_null(J, 2);

    V3D_f *v1 = array_to_v3d(J, 3);
    V3D_f *v2 = array_to_v3d(J, 4);
    V3D_f *v3 = array_to_v3d(J, 5);
    V3D_f *v4 = array_to_v3d(J, 6);

    if (v1 && v2 && v3 && v4) {
        quad3d_f(cur, type, texture, v1, v2, v3, v4);
    } else {
        js_error(J, "Cannot convert vertex");
    }

    if (v1) {
        free(v1);
    }
    if (v2) {
        free(v2);
    }
    if (v3) {
        free(v3);
    }
    if (v4) {
        free(v4);
    }
}

static void f_RenderScene(js_State *J) { render_scene(); }

static void f_DestroyScene(js_State *J) { destroy_scene(); }

static void f_ClearScene(js_State *J) { clear_scene(cur); }

static void f_CreateScene(js_State *J) {
    int nedge = js_toint16(J, 1);
    int npoly = js_toint16(J, 2);

    if (create_scene(nedge, npoly) < 0) {
        js_error(J, "Cannot allocate scene");
        return;
    }
}

static void f_Polygon3D(js_State *J) {
    int type = js_toint16(J, 1);
    BITMAP *texture = bitmap_or_null(J, 2);

    int vc = 0;
    V3D_f **vtx = v3d_array(J, 3, &vc);
    if (vtx) {
        polygon3d_f(cur, type, texture, vc, vtx);
    } else {
        js_error(J, "Cannot convert vertices");
    }

    free_v3d(vtx, vc);
}

static void f_VDebug(js_State *J) {
    int vc = 0;
    V3D_f **vtx = v3d_array(J, 1, &vc);
    if (vtx) {
        fprintf(LOGSTREAM, "Number of entries=%d\n", vc);
        for (int i = 0; i < vc; i++) {
            fprintf(LOGSTREAM, "  v[%d] = {x=%f, y=%f, z=%f, u=%f, v=%f, c=0x%X}\n", i, vtx[i]->x, vtx[i]->y, vtx[i]->z, vtx[i]->u, vtx[i]->v, vtx[i]->c);
        }
    } else {
        js_error(J, "Cannot convert vertices");
    }
    fflush(LOGSTREAM);

    free_v3d(vtx, vc);
}

// static void f_(js_State *J) {}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize a3d subsystem.
 *
 * @param J VM state.
 */
void init_a3d(js_State *J) {
    // define global functions
    FUNCDEF(J, f_RenderScene, "RenderScene", 0);
    FUNCDEF(J, f_DestroyScene, "DestroyScene", 0);
    FUNCDEF(J, f_ClearScene, "ClearScene", 0);
    FUNCDEF(J, f_CreateScene, "CreateScene", 2);

    FUNCDEF(J, f_Polygon3D, "Polygon3D", 3);
    FUNCDEF(J, f_Triangle3D, "Triangle3D", 5);
    FUNCDEF(J, f_Quad3D, "Quad3D", 6);

    FUNCDEF(J, f_VDebug, "VDebug", 1);
}

/**
 int clip3d_f(int type, float min_z, float max_z, int vc, const V3D_f *vtx[], V3D_f *vout[], V3D_f *vtmp[], int out[]);
zbuffer?

 */