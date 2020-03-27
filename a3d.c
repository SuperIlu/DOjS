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
#include <math.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "DOjS.h"
#include "a3d.h"
#include "bitmap.h"

//! convert angle in radians to allegro 0..256 representation
#define RADTOALLEG(x) (x / (2 * M_PI) * 256)

/*********************
** static functions **
*********************/
/**
 * @brief free an array of V3D_f.
 *
 * @param v pointer to the array.
 * @param vc number of elements in that array.
 */
static void free_v3d(V3D_f **v, int vc) {
    if (v) {
        for (int i = 0; i < vc; i++) {
            if (v[i]) {
                free(v[i]);
            }
        }

        free(v);
    }
}

/**
 * @brief expects an array w/ 6 elements on the stack and returns a V3D_f. TODO: error handling?
 *
 * @param J VM state.
 * @param idx stack index where to find the array.
 * @param v space to store the V3D_f.
 */
static void array_to_v3d(js_State *J, int idx, V3D_f *v) {
    js_getindex(J, idx, 0);
    v->x = (float)js_tonumber(J, -1);
    js_pop(J, 1);
    js_getindex(J, idx, 1);
    v->y = (float)js_tonumber(J, -1);
    js_pop(J, 1);
    js_getindex(J, idx, 2);
    v->z = (float)js_tonumber(J, -1);
    js_pop(J, 1);
    js_getindex(J, idx, 3);
    v->u = (float)js_tonumber(J, -1);
    js_pop(J, 1);
    js_getindex(J, idx, 4);
    v->v = (float)js_tonumber(J, -1);
    js_pop(J, 1);

    js_getindex(J, idx, 5);
    v->c = js_toint32(J, -1);
    js_pop(J, 1);
}

/**
 * @brief expects an array of arrays w/ 6 elements on the stack and returns an array of pointers to V3D_f.
 *
 * @param J VM state.
 * @param idx stack index where to find the array.
 * @param vc space to store the number of converted elements.
 *
 * @return V3D_f** an array of pointers to V3D_f.
 */
static V3D_f **v3d_array(js_State *J, int idx, int *vc) {
    if (!js_isarray(J, idx)) {
        *vc = 0;
        JS_ENOARR(J);
        return NULL;
    } else {
        int len = js_getlength(J, idx);
        V3D_f **array = calloc(len, sizeof(V3D_f *));
        if (!array) {
            return NULL;
        }
        for (int i = 0; i < len; i++) {
            js_getindex(J, idx, i);
            {
                int vlen = js_getlength(J, -1);
                if (vlen < 6) {
                    js_error(J, "V3D must have 6 values");
                    free_v3d(array, len);
                    *vc = 0;
                    return NULL;
                }

                array[i] = malloc(sizeof(V3D_f));
                if (!array[i]) {
                    JS_ENOMEM(J);
                    free_v3d(array, len);
                    *vc = 0;
                    return NULL;
                }
                array_to_v3d(J, -1, array[i]);
            }
            js_pop(J, 1);
        }
        *vc = len;
        return array;
    }
}

/**
 * @brief create a temporary array of V3D_f that can be freed with free_v3d().
 *
 * @param J VM state.
 * @param len number of elements in that array.
 *
 * @return V3D_f** an array of pointers to V3D_f.
 */
static V3D_f **tmp_v3d(js_State *J, int len) {
    V3D_f **array = calloc(len, sizeof(V3D_f *));
    if (!array) {
        return NULL;
    }

    for (int i = 0; i < len; i++) {
        V3D_f *v = malloc(sizeof(V3D_f));
        array[i] = v;
        if (!array[i]) {
            JS_ENOMEM(J);
            free_v3d(array, len);
            return NULL;
        }
    }
    return array;
}

/**
 * @brief get bitmap or NULL from stack.
 *
 * @param J VM state.
 * @param idx stack index.
 *
 * @return BITMAP* pointer to a BITMAP or NULL.
 */
static BITMAP *bitmap_or_null(js_State *J, int idx) {
    BITMAP *texture = NULL;
    if (!js_isnull(J, idx) && js_isuserdata(J, idx, TAG_BITMAP)) {
        texture = js_touserdata(J, idx, TAG_BITMAP);
    }
    return texture;
}

/**
 * @brief draw 3d triangle.
 * Triangle3D(type, texture, p1, p2, p3)
 *
 * @param J VM state.
 */
static void f_Triangle3D(js_State *J) {
    V3D_f v1, v2, v3;

    int type = js_toint16(J, 1);
    BITMAP *texture = bitmap_or_null(J, 2);

    array_to_v3d(J, 3, &v1);
    array_to_v3d(J, 4, &v2);
    array_to_v3d(J, 5, &v3);

    triangle3d_f(DOjS.current_bm, type, texture, &v1, &v2, &v3);
}

/**
 * @brief draw 3d quad.
 * Quad3D(type, texture, p1, p2, p3, p4)
 *
 * @param J VM state.
 */
static void f_Quad3D(js_State *J) {
    V3D_f v1, v2, v3, v4;
    int type = js_toint16(J, 1);
    BITMAP *texture = bitmap_or_null(J, 2);

    array_to_v3d(J, 3, &v1);
    array_to_v3d(J, 4, &v2);
    array_to_v3d(J, 5, &v3);
    array_to_v3d(J, 6, &v4);

    quad3d_f(DOjS.current_bm, type, texture, &v1, &v2, &v3, &v4);
}

/**
 * @brief Draw 3d polygons onto the specified bitmap, using the specified rendering mode.
 * Polygon3D(type, texture, [p1...pN])
 *
 * @param J VM state.
 */
static void f_Polygon3D(js_State *J) {
    int type = js_toint16(J, 1);
    BITMAP *texture = bitmap_or_null(J, 2);

    int vc = 0;
    V3D_f **vtx = v3d_array(J, 3, &vc);
    if (vtx) {
        polygon3d_f(DOjS.current_bm, type, texture, vc, vtx);
    } else {
        js_error(J, "Cannot convert vertices");
    }

    free_v3d(vtx, vc);
}

/**
 * @brief Clips the polygon given.
 *
 * @param J VM state.
 */
static void f_Clip3D(js_State *J) {
    int type = js_toint16(J, 1);

    float min_z = (float)js_tonumber(J, 2);
    float max_z = (float)js_tonumber(J, 3);

    int vc = 0;
    const V3D_f **vtx = (const V3D_f **)v3d_array(J, 4, &vc);

    int tmp_size = vc * 12;  // should be at least vc * (1.5 ^ n), where `n' is the number of clipping planes (5 or 6), and `^' denotes "to the power of".

    V3D_f **vout = tmp_v3d(J, tmp_size);
    V3D_f **vtmp = tmp_v3d(J, tmp_size);
    int *out = malloc(tmp_size * sizeof(int));

    bool error;
    if (vout && vtmp && out) {
        // finally call function
        int num_out = clip3d_f(type, min_z, max_z, vc, vtx, vout, vtmp, out);

        js_newarray(J);
        for (int i = 0; i < num_out; i++) {
            js_newarray(J);
            js_pushnumber(J, vout[i]->x);
            js_setindex(J, -2, 0);
            js_pushnumber(J, vout[i]->y);
            js_setindex(J, -2, 1);
            js_pushnumber(J, vout[i]->z);
            js_setindex(J, -2, 2);
            js_pushnumber(J, vout[i]->u);
            js_setindex(J, -2, 3);
            js_pushnumber(J, vout[i]->v);
            js_setindex(J, -2, 4);
            js_pushnumber(J, vout[i]->c);
            js_setindex(J, -2, 5);

            js_setindex(J, -2, i);
        }

        error = false;
    } else {
        error = true;
    }

    // free temporary memory
    if (out) {
        free(out);
    }
    if (vout) {
        free_v3d(vout, tmp_size);
    }
    if (vtmp) {
        free_v3d(vtmp, tmp_size);
    }

    // check if error and set it
    if (error) {
        JS_ENOMEM(J);
    }
}

/**
 * @brief Renders all the specified scene_polygon3d()'s on the bitmap passed to clear_scene(). Rendering is done one scanline at a time, with no pixel being processed more than
 * once.
 *
 * @param J VM state.
 */
static void f_RenderScene(js_State *J) { render_scene(); }

/**
 * @brief Deallocate memory previously allocated by create_scene. Use this to avoid memory leaks in your program.
 *
 * @param J VM state.
 */
static void f_DestroyScene(js_State *J) { destroy_scene(); }

/**
 * @brief Initializes a scene. The bitmap is the bitmap you will eventually render on.
 *
 * @param J VM state.
 */
static void f_ClearScene(js_State *J) { clear_scene(DOjS.current_bm); }

/**
 * @brief Allocates memory for a scene, `nedge' and `npoly' are your estimates of how many edges and how many polygons you will render (you cannot get over the limit specified
 * here). If you use same values in successive calls, the space will be reused (no new malloc()).
 *
 * @param J VM state.
 */
static void f_CreateScene(js_State *J) {
    int nedge = js_toint16(J, 1);
    int npoly = js_toint16(J, 2);

    if (create_scene(nedge, npoly) < 0) {
        js_error(J, "Cannot allocate scene");
        return;
    }
}

/**
 * @brief This number (default value = 100.0) controls the behaviour of the z-sorting algorithm.
 * When an edge is very close to another's polygon plane, there is an interval of uncertainty in which you cannot tell which object is visible (which z is smaller).
 * This is due to cumulative numerical errors for edges that have undergone a lot of transformations and interpolations.
 * The default value means that if the 1/z values (in projected space) differ by only 1/100 (one percent), they are considered to be equal and the x-slopes of the planes are used
 * to find out which plane is getting closer when we move to the right. Larger values means narrower margins, and increasing the chance of missing true adjacent edges/planes.
 * Smaller values means larger margins, and increasing the chance of mistaking close polygons for adjacent ones.
 * The value of 100 is close to the optimum. However, the optimum shifts slightly with resolution, and may be application-dependent.
 * It is here for you to fine-tune.
 *
 * @param J VM state.
 */
static void f_SetSceneGap(js_State *J) { scene_gap = (float)js_tonumber(J, 1); }

/**
 * @brief Draw 3d polygons onto the specified bitmap, using the specified rendering mode.
 * ScenePolygon3D(type, texture, [p1...pN])
 *
 * @param J VM state.
 */
static void f_ScenePolygon3D(js_State *J) {
    int type = js_toint16(J, 1);
    BITMAP *texture = bitmap_or_null(J, 2);

    int vc = 0;
    V3D_f **vtx = v3d_array(J, 3, &vc);
    if (vtx) {
        scene_polygon3d_f(type, texture, vc, vtx);
    } else {
        js_error(J, "Cannot convert vertices");
    }

    free_v3d(vtx, vc);
}

#ifdef DEBUG_ENABLED
/**
 * @brief debug vertex conversion.
 *
 * @param J VM state.
 */
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

static void print_matrix(MATRIX_f *m) {
    fprintf(LOGSTREAM, "%s={\n", "matrix");
    for (int r = 0; r < 3; r++) {
        fprintf(LOGSTREAM, "  |%3.4f, %3.4f, %3.4f|\n", m->v[r][0], m->v[r][1], m->v[r][2]);
    }
    fprintf(LOGSTREAM, "  {%3.4f, %3.4f, %3.4f}\n}\n", m->t[0], m->t[1], m->t[2]);
    fflush(LOGSTREAM);
}

#endif

/**
 * @brief return a matrix
 *
 * @param J VM state.
 * @param m the matrix.
 */
static void return_matrix(js_State *J, MATRIX_f *m) {
    js_newobject(J);
    {
        js_newarray(J);
        for (int j = 0; j < 3; j++) {
            js_newarray(J);
            for (int i = 0; i < 3; i++) {
                js_pushnumber(J, m->v[j][i]);
                js_setindex(J, -2, i);
            }
            js_setindex(J, -2, j);
        }
        js_setproperty(J, -2, "v");

        // push the 't'
        js_newarray(J);
        for (int i = 0; i < 3; i++) {
            js_pushnumber(J, m->t[i]);
            js_setindex(J, -2, i);
        }
        js_setproperty(J, -2, "t");
    }
}

/**
 * @brief helper: Get a matrix parameter.
 *
 * @param J VM state.
 * @param m where to store the matrix.
 * @param idx stack index.
 */
static void get_matrix(js_State *J, MATRIX_f *m, int idx) {
    js_getproperty(J, idx, "v");
    for (int j = 0; j < 3; j++) {
        js_getindex(J, -1, j);
        for (int i = 0; i < 3; i++) {
            js_getindex(J, -1, i);
            m->v[j][i] = js_tonumber(J, -1);
            js_pop(J, 1);
        }
        js_pop(J, 1);
    }
    js_pop(J, 1);

    js_getproperty(J, idx, "t");
    for (int i = 0; i < 3; i++) {
        js_getindex(J, -1, i);
        m->t[i] = js_tonumber(J, -1);
        js_pop(J, 1);
    }
    js_pop(J, 1);
}

/**
 * @brief helper: return a V3D_f as array.
 *
 * @param J VM state.
 * @param x x value.
 * @param y y value.
 * @param z z value.
 * @param u u value.
 * @param v v value.
 * @param c c value.
 */
static void return_array(js_State *J, float x, float y, float z, float u, float v, int c) {
    js_newarray(J);
    js_pushnumber(J, x);
    js_setindex(J, -2, 0);
    js_pushnumber(J, y);
    js_setindex(J, -2, 1);
    js_pushnumber(J, z);
    js_setindex(J, -2, 2);
    js_pushnumber(J, u);
    js_setindex(J, -2, 3);
    js_pushnumber(J, v);
    js_setindex(J, -2, 4);
    js_pushnumber(J, c);
    js_setindex(J, -2, 5);
}

/**
 * @brief create rotation matrix, C version.
 *
 * @param J VM state.
 */
static void f_GetRotationMatrix(js_State *J) {
    MATRIX_f m;

    float x = js_tonumber(J, 1);
    float y = js_tonumber(J, 2);
    float z = js_tonumber(J, 3);

    get_rotation_matrix_f(&m, RADTOALLEG(x), RADTOALLEG(y), RADTOALLEG(z));

    return_matrix(J, &m);
}

/**
 * @brief create rotation matrix, X, C version.
 *
 * @param J VM state.
 */
static void f_GetXRotateMatrix(js_State *J) {
    MATRIX_f m;

    float r = js_tonumber(J, 1);

    get_x_rotate_matrix_f(&m, RADTOALLEG(r));

    return_matrix(J, &m);
}

/**
 * @brief create rotation matrix, Y, C version.
 *
 * @param J VM state.
 */
static void f_GetYRotateMatrix(js_State *J) {
    MATRIX_f m;

    float r = js_tonumber(J, 1);

    get_y_rotate_matrix_f(&m, RADTOALLEG(r));

    return_matrix(J, &m);
}

/**
 * @brief create rotation matrix, Z, C version.
 *
 * @param J VM state.
 */
static void f_GetZRotateMatrix(js_State *J) {
    MATRIX_f m;

    float r = js_tonumber(J, 1);

    get_z_rotate_matrix_f(&m, RADTOALLEG(r));

    return_matrix(J, &m);
}

/**
 * @brief creater transformation matrix, C version.
 *
 * @param J VM state.
 */
static void f_GetTransformationMatrix(js_State *J) {
    MATRIX_f m;
    float scale = js_tonumber(J, 1);
    float xr = js_tonumber(J, 2);
    float yr = js_tonumber(J, 3);
    float zr = js_tonumber(J, 4);

    float x = js_tonumber(J, 5);
    float y = js_tonumber(J, 6);
    float z = js_tonumber(J, 7);

    get_transformation_matrix_f(&m, scale, RADTOALLEG(xr), RADTOALLEG(yr), RADTOALLEG(zr), x, y, z);

    return_matrix(J, &m);
}

/**
 * @brief multiply two matrices, C version.
 *
 * @param J VM state.
 */
static void f_MatrixMul(js_State *J) {
    MATRIX_f m1, m2, mo;

    get_matrix(J, &m1, 1);
    get_matrix(J, &m2, 2);
    matrix_mul_f(&m1, &m2, &mo);
    return_matrix(J, &mo);
}

/**
 * @brief apply matrix, C version.
 *
 * @param J VM state.
 */
static void f_ApplyMatrix(js_State *J) {
    MATRIX_f m;
    float x, y, z, u, v, xo, yo, zo;
    int c;

    get_matrix(J, &m, 1);
    if (js_isarray(J, 2)) {
        js_getindex(J, 2, 0);
        x = js_tonumber(J, -1);
        js_pop(J, 1);
        js_getindex(J, 2, 1);
        y = js_tonumber(J, -1);
        js_pop(J, 1);
        js_getindex(J, 2, 2);
        z = js_tonumber(J, -1);
        js_pop(J, 1);

        js_getindex(J, 2, 3);
        u = js_tonumber(J, -1);
        js_pop(J, 1);
        js_getindex(J, 2, 4);
        v = js_tonumber(J, -1);
        js_pop(J, 1);

        js_getindex(J, 2, 5);
        c = js_toint32(J, -1);
        js_pop(J, 1);

    } else {
        x = js_tonumber(J, 2);
        y = js_tonumber(J, 3);
        z = js_tonumber(J, 4);
        u = v = c = 0;
    }
    apply_matrix_f(&m, x, y, z, &xo, &yo, &zo);
    return_array(J, xo, yo, zo, u, v, c);
}

/**
 * @brief project to viewport, C version.
 *
 * @param J VM state.
 */
static void f_PerspProject(js_State *J) {
    float x, y, z, u, v, xo, yo;
    int c;

    if (js_isarray(J, 1)) {
        js_getindex(J, 1, 0);
        x = js_tonumber(J, -1);
        js_pop(J, 1);
        js_getindex(J, 1, 1);
        y = js_tonumber(J, -1);
        js_pop(J, 1);
        js_getindex(J, 1, 2);
        z = js_tonumber(J, -1);
        js_pop(J, 1);

        js_getindex(J, 1, 3);
        u = js_tonumber(J, -1);
        js_pop(J, 1);
        js_getindex(J, 1, 4);
        v = js_tonumber(J, -1);
        js_pop(J, 1);

        js_getindex(J, 1, 5);
        c = js_toint32(J, -1);
        js_pop(J, 1);

    } else {
        x = js_tonumber(J, 1);
        y = js_tonumber(J, 2);
        z = js_tonumber(J, 3);
        u = v = c = 0;
    }
    persp_project_f(x, y, z, &xo, &yo);
    return_array(J, xo, yo, z, u, v, c);
}

/**
 * @brief set viewport, C version.
 *
 * @param J VM state.
 */
static void f_SetProjectionViewport(js_State *J) {
    int x = js_toint32(J, 1);
    int y = js_toint32(J, 2);
    int w = js_toint32(J, 3);
    int h = js_toint32(J, 4);

    set_projection_viewport(x, y, w, h);
}

/**
 * @brief calculate y-normal for polygon, C version.
 *
 * @param J VM state.
 */
static void f_PolygonZNormal(js_State *J) {
    js_getindex(J, 1, 0);
    float v1_x = js_tonumber(J, -1);
    js_pop(J, 1);

    js_getindex(J, 2, 0);
    float v2_x = js_tonumber(J, -1);
    js_pop(J, 1);

    js_getindex(J, 3, 0);
    float v3_x = js_tonumber(J, -1);
    js_pop(J, 1);

    js_getindex(J, 1, 1);
    float v1_y = js_tonumber(J, -1);
    js_pop(J, 1);

    js_getindex(J, 2, 1);
    float v2_y = js_tonumber(J, -1);
    js_pop(J, 1);

    js_getindex(J, 3, 1);
    float v3_y = js_tonumber(J, -1);
    js_pop(J, 1);

    js_pushnumber(J, ((v2_x - v1_x) * (v3_y - v2_y)) - ((v3_x - v2_x) * (v2_y - v1_y)));
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize a3d subsystem.
 *
 * @param J VM state.
 */
void init_a3d(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    // scene based rendering
    FUNCDEF(J, f_ScenePolygon3D, "_ScenePolygon3D", 3);
    FUNCDEF(J, f_RenderScene, "_RenderScene", 0);
    FUNCDEF(J, f_DestroyScene, "_DestroyScene", 0);
    FUNCDEF(J, f_ClearScene, "_ClearScene", 0);
    FUNCDEF(J, f_CreateScene, "_CreateScene", 2);
    FUNCDEF(J, f_SetSceneGap, "SetSceneGap", 1);

    // direct rendering
    FUNCDEF(J, f_Polygon3D, "Polygon3D", 3);
    FUNCDEF(J, f_Triangle3D, "Triangle3D", 5);
    FUNCDEF(J, f_Quad3D, "Quad3D", 6);
    FUNCDEF(J, f_Clip3D, "Clip3D", 4);

#ifdef DEBUG_ENABLED
    FUNCDEF(J, f_VDebug, "VDebug", 1);
#endif

    // non JS 3D-math
    FUNCDEF(J, f_GetRotationMatrix, "GetRotationMatrix", 3);
    FUNCDEF(J, f_GetTransformationMatrix, "GetTransformationMatrix", 7);
    FUNCDEF(J, f_GetXRotateMatrix, "GetXRotateMatrix", 1);
    FUNCDEF(J, f_GetYRotateMatrix, "GetYRotateMatrix", 1);
    FUNCDEF(J, f_GetZRotateMatrix, "GetZRotateMatrix", 1);
    FUNCDEF(J, f_MatrixMul, "MatrixMul", 2);
    FUNCDEF(J, f_ApplyMatrix, "ApplyMatrix", 2);
    FUNCDEF(J, f_PerspProject, "PerspProject", 2);
    FUNCDEF(J, f_SetProjectionViewport, "_SetProjectionViewport", 4);
    FUNCDEF(J, f_PolygonZNormal, "PolygonZNormal", 3);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
C := implemented in C
J := implemented in JS
" " := not yet implemented
? := find out if needed

Polygon rendering
C    polygon3d_f — Draws a 3d polygon onto the specified bitmap.
C    triangle3d_f — Draws a 3d triangle onto the specified bitmap.
C    quad3d_f — Draws a 3d quad onto the specified bitmap.
C    clip3d_f — Clips the polygon given in vtx using floating point math,

Z buffer rendering
C    create_zbuffer — Creates a Z-buffer for a bitmap.
    create_sub_zbuffer — Creates a sub-z-buffer.
C    set_zbuffer — Makes the given Z-buffer the active one.
C    clear_zbuffer — Writes a depth value into the given Z-buffer.
C    destroy_zbuffer — Destroys a Z-buffer.

Scene rendering
C    create_scene — Allocates memory for a 3d scene.
C    clear_scene — Initializes a scene.
C    destroy_scene — Deallocates the memory used by a scene.
C    render_scene — Renders all the queued scene polygons.
JC   scene_polygon3d_f — Puts a polygon in the scene rendering list.
C    scene_gap — Number controlling the scene z-sorting algorithm behaviour.
 */