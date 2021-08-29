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

#include "3dfx-glide.h"

#include <allegro.h>
#include <dirent.h>
#include <dpmi.h>
#include <errno.h>
#include <glide.h>
#include <math.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "DOjS.h"

static GrContext_t fx_context;    //!< 3fx context
static FxU32 fx_vertex_size = 0;  //!< number of entries in vertex

#define FX_MAX_PARAMS 20  // 19 params + 1 to be safe

//! vertex definition
typedef struct fx_vertex {
    FxFloat p[FX_MAX_PARAMS];  // !< vertex has this number of components
} fx_vertex_t;

/*********************
** static functions **
*********************/
#ifdef VERTEX_DEBUG
/**
 * @brief print a vertex to debug.
 *
 * @param v the vertex.
 */
static void f_fxPrintVertex(fx_vertex_t *v) {
    DEBUG("  v = {");
    for (int i = 0; i < fx_vertex_size; i++) {
        if (i != 0) {
            DEBUG(", ");
        }
        DEBUGF("%f", v->p[i]);
    }
    DEBUG("}\n");
}
#endif

/**
 * @brief callback for errors.
 *
 * @param msg the message text.
 * @param fatal true if this is a fatal error
 */
static void fx_error_callback(const char *msg, FxBool fatal) { LOGF("[GLIDE %s] %s", fatal ? "fatal" : "error", msg); }

/**
 * @brief convert an array to a vertex. Order is defined by fxVertexLayout().
 *
 * @param J VM state.
 * @param idx parameter index.
 * @param v destination vertex.
 * @return true if conversion succeeded.
 * @return false if conversion fails.
 */
static bool fx_get_vertex(js_State *J, int idx, fx_vertex_t *v) {
    if (fx_vertex_size) {
        int len = js_getlength(J, idx);
        if (js_isarray(J, idx) && (len >= fx_vertex_size)) {
            for (int i = 0; i < fx_vertex_size; i++) {
                js_getindex(J, idx, i);
                v->p[i] = js_tonumber(J, -1);
                js_pop(J, 1);
            }
#ifdef VERTEX_DEBUG
            fx_print_vertex(v);
#endif
            return TRUE;
        } else {
            js_error(J, "Parameter is no array of length >= %ld", fx_vertex_size);
            return FALSE;
        }
    } else {
        js_error(J, "Vertex is undefined, use fxVertexLayout() first!");
        return FALSE;
    }
}

/**
 * @brief initialize glide.
 *
 * @param J VM state.
 */
static void f_fxInit(js_State *J) {
    grGlideInit();
    grSstSelect(0);
    fx_context = grSstWinOpen(0, GR_RESOLUTION_640x480, GR_REFRESH_60Hz, GR_COLORFORMAT_ARGB, GR_ORIGIN_UPPER_LEFT, 2, 1);
    if (!fx_context) {
        js_error(J, "Can't initialize GLIDE3.");
        return;
    }
    DOjS.glide_enabled = true;
    grErrorSetCallback(fx_error_callback);

    grCoordinateSpace(GR_WINDOW_COORDS);  // we use window coordinates by default

    FxI32 num;
    LOGF("GR_VENDOR: %s\n", grGetString(GR_VENDOR));
    LOGF("GR_RENDERER: %s\n", grGetString(GR_RENDERER));
    LOGF("GR_VERSION: %s\n", grGetString(GR_VERSION));
    LOGF("GR_HARDWARE: %s\n", grGetString(GR_HARDWARE));
    LOGF("GR_EXTENSION: %s\n", grGetString(GR_EXTENSION));
    if (grGet(GR_NUM_BOARDS, sizeof(num), &num)) {
        LOGF("GR_NUM_BOARDS: %ld\n", num);
    }
    if (grGet(GR_NUM_FB, sizeof(num), &num)) {
        LOGF("GR_NUM_FB: %ld\n", num);
    }
    if (grGet(GR_NUM_TMU, sizeof(num), &num)) {
        LOGF("GR_NUM_TMU: %ld\n", num);
    }
    if (grGet(GR_MEMORY_FB, sizeof(num), &num)) {
        LOGF("GR_MEMORY_FB: %ld\n", num);
    }
    if (grGet(GR_MEMORY_TMU, sizeof(num), &num)) {
        LOGF("GR_MEMORY_TMU: %ld\n", num);
    }
    if (grGet(GR_MEMORY_UMA, sizeof(num), &num)) {
        LOGF("GR_MEMORY_UMA: %ld\n", num);
    }
}

/**
 * @brief define vertex layout.
 *
 * @param J VM state.
 */
static void f_fxVertexLayout(js_State *J) {
    if (js_isarray(J, 1)) {
        int len = js_getlength(J, 1);
        for (int i = 0; i < len; i++) {
            js_getindex(J, 1, i);
            FxU32 param = js_toint32(J, -1);
            js_pop(J, 1);

            FxI32 offset = fx_vertex_size * 4;
            switch (param) {
                case GR_PARAM_XY:
                    if (i != 0) {
                        js_error(J, "GR_PARAM.XY must be first argument!");
                        fx_vertex_size = 0;
                        return;
                    }
                    fx_vertex_size += 2;
                    break;
                case GR_PARAM_RGB:
                    fx_vertex_size += 3;
                    break;
                case GR_PARAM_ST0:
                case GR_PARAM_ST1:
                case GR_PARAM_ST2:
                    fx_vertex_size += 2;
                    break;
                case GR_PARAM_Z:
                case GR_PARAM_W:
                case GR_PARAM_Q:
                case GR_PARAM_FOG_EXT:
                case GR_PARAM_A:
                case GR_PARAM_Q0:
                case GR_PARAM_Q1:
                case GR_PARAM_Q2:
                    fx_vertex_size += 1;
                    break;

                case GR_PARAM_PARGB:
                default:
                    js_error(J, "Parameter type not supported: %ld", param);
            }
            grVertexLayout(param, offset, GR_PARAM_ENABLE);
        }
    } else {
        JS_ENOARR(J);
    }
}

/**
 * @brief reset vertex layout
 *
 * @param J VM state.
 */
static void f_fxResetVertexLayout(js_State *J) {
    grReset(GR_VERTEX_PARAMETER);
    fx_vertex_size = 0;
}

/**
 * @brief get size of a vertex as defined by fxVertexLayout().
 *
 * @param J VM state.
 */
static void f_fxGetVertexSize(js_State *J) { js_pushnumber(J, fx_vertex_size); }

/**
 * @brief shutdown glide again.
 */
static void f_fxShutdown() {
    DOjS.glide_enabled = false;
    if (fx_context) {
        grSstWinClose(fx_context);
        grGlideShutdown();
    }
}

/**
 * @brief draw a point.
 *
 * @param J VM state.
 */
static void f_fxDrawPoint(js_State *J) {
    fx_vertex_t v1;
    if (fx_get_vertex(J, 1, &v1)) {
        grDrawPoint(&v1);
    }
}

/**
 * @brief draw a line.
 *
 * @param J VM state.
 */
static void f_fxDrawLine(js_State *J) {
    fx_vertex_t v1, v2;
    if (fx_get_vertex(J, 1, &v1) && fx_get_vertex(J, 2, &v2)) {
        grDrawLine(&v1, &v2);
    }
}

/**
 * @brief draw a triangle.
 *
 * @param J VM state.
 */
static void f_fxDrawTriangle(js_State *J) {
    fx_vertex_t v1, v2, v3;
    if (fx_get_vertex(J, 1, &v1) && fx_get_vertex(J, 2, &v2) && fx_get_vertex(J, 3, &v3)) {
        grDrawTriangle(&v1, &v2, &v3);
    }
}

/**
 * @brief draw anti aliased triangle.
 *
 * @param J VM state.
 */
static void f_fxAADrawTriangle(js_State *J) {
    fx_vertex_t v1, v2, v3;
    if (fx_get_vertex(J, 1, &v1) && fx_get_vertex(J, 2, &v2) && fx_get_vertex(J, 3, &v3)) {
        grAADrawTriangle(&v1, &v2, &v3, js_toboolean(J, 4), js_toboolean(J, 5), js_toboolean(J, 6));
    }
}

/**
 * @brief draw vertex array.
 *
 * @param J VM state.
 */
static void f_fxDrawVertexArray(js_State *J) {
    FxU32 mode = js_toint32(J, 1);
    if (js_isarray(J, 2)) {
        int len = js_getlength(J, 2);
        fx_vertex_t *vlist = malloc(sizeof(fx_vertex_t) * len);
        if (vlist) {
            for (int i = 0; i < len; i++) {
                js_getindex(J, 2, i);
                if (!fx_get_vertex(J, -1, &vlist[i])) {
                    free(vlist);
                    return;
                }
            }
            grDrawVertexArrayContiguous(mode, len, vlist, sizeof(fx_vertex_t));
        } else {
            JS_ENOMEM(J);
        }
    } else {
        js_error(J, "Vertex list parameter is no array.");
    }
}

/**
 * @brief convert fog table entries to w.
 *
 * @param J VM state.
 */
static void f_fxFogTableIndexToW(js_State *J) {
    FxI32 nFog;
    grGet(GR_FOG_TABLE_ENTRIES, 4, &nFog);
    js_newarray(J);
    for (int i = 0; i < nFog; i++) {
        js_pushnumber(J, guFogTableIndexToW(i));
        js_setindex(J, -2, i);
    }
}

/**
 * @brief define fog table.
 *
 * @param J VM state.
 */
static void f_fxFogTable(js_State *J) {
    FxI32 nFog;
    grGet(GR_FOG_TABLE_ENTRIES, 4, &nFog);
    if (js_isarray(J, 1) && (js_getlength(J, 1) >= nFog)) {
        GrFog_t *table = malloc(nFog * sizeof(GrFog_t));
        if (table) {
            for (int i = 0; i < nFog; i++) {
                js_getindex(J, 1, i);
                table[i] = js_touint16(J, -1);
                js_pop(J, 1);
            }
            grFogTable(table);
            free(table);
        } else {
            JS_ENOMEM(J);
        }
    } else {
        js_error(J, "Parameter is no array of length >= %ld", nFog);
    }
}

/**
 * @brief generate fog table.
 *
 * @param J VM state.
 */
static void f_fxFogGenerateExp(js_State *J) {
    FxI32 nFog;
    grGet(GR_FOG_TABLE_ENTRIES, 4, &nFog);

    DEBUGF("fog table size=%ld\n", nFog);

    GrFog_t *table = malloc(nFog * sizeof(GrFog_t));
    if (!table) {
        JS_ENOMEM(J);
        return;
    }

    guFogGenerateExp(table, js_tonumber(J, 1));

    js_newarray(J);
    for (int i = 0; i < nFog; i++) {
        js_pushnumber(J, table[i]);
        js_setindex(J, -2, i);
    }

    free(table);
}

/**
 * @brief generate fog table.
 *
 * @param J VM state.
 */
static void f_fxFogGenerateExp2(js_State *J) {
    FxI32 nFog;
    grGet(GR_FOG_TABLE_ENTRIES, 4, &nFog);

    GrFog_t *table = malloc(nFog * sizeof(GrFog_t));
    if (!table) {
        JS_ENOMEM(J);
        return;
    }

    guFogGenerateExp2(table, js_tonumber(J, 1));

    js_newarray(J);
    for (int i = 0; i < nFog; i++) {
        js_pushnumber(J, table[i]);
        js_setindex(J, -2, i);
    }

    free(table);
}

/**
 * @brief generate fog table.
 *
 * @param J VM state.
 */
static void f_fxFogGenerateLinear(js_State *J) {
    FxI32 nFog;
    grGet(GR_FOG_TABLE_ENTRIES, 4, &nFog);

    GrFog_t *table = malloc(nFog * sizeof(GrFog_t));
    if (!table) {
        JS_ENOMEM(J);
        return;
    }

    guFogGenerateLinear(table, js_tonumber(J, 1), js_tonumber(J, 2));

    js_newarray(J);
    for (int i = 0; i < nFog; i++) {
        js_pushnumber(J, table[i]);
        js_setindex(J, -2, i);
    }

    free(table);
}

/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxBufferSwap(js_State *J) { grBufferSwap(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxBufferClear(js_State *J) { grBufferClear(js_touint32(J, 1), js_touint32(J, 2), js_touint32(J, 3)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxClipWindow(js_State *J) { grClipWindow(js_touint32(J, 1), js_touint32(J, 2), js_touint32(J, 3), js_touint32(J, 4)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxConstantColorValue(js_State *J) { grConstantColorValue(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxCullMode(js_State *J) { grCullMode(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxAlphaBlendFunction(js_State *J) { grAlphaBlendFunction(js_touint32(J, 1), js_touint32(J, 2), js_touint32(J, 3), js_touint32(J, 4)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxAlphaCombine(js_State *J) { grAlphaCombine(js_touint32(J, 1), js_touint32(J, 2), js_touint32(J, 3), js_touint32(J, 4), js_toboolean(J, 5)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxColorCombine(js_State *J) { grColorCombine(js_touint32(J, 1), js_touint32(J, 2), js_touint32(J, 3), js_touint32(J, 4), js_toboolean(J, 5)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxColorMask(js_State *J) { grColorMask(js_toboolean(J, 1), js_toboolean(J, 2)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxDepthMask(js_State *J) { grDepthMask(js_toboolean(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxEnable(js_State *J) { grEnable(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxDisable(js_State *J) { grDisable(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxDisableAllEffects(js_State *J) { grDisableAllEffects(); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxDitherMode(js_State *J) { grDitherMode(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxAlphaControlsITRGBLighting(js_State *J) { grAlphaControlsITRGBLighting(js_toboolean(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxGammaCorrectionRGB(js_State *J) { guGammaCorrectionRGB(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxDepthRange(js_State *J) { grDepthRange(js_tonumber(J, 1), js_tonumber(J, 2)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxDepthBufferMode(js_State *J) { grDepthBufferMode(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxDepthBufferFunction(js_State *J) { grDepthBufferFunction(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxDepthBiasLevel(js_State *J) { grDepthBiasLevel(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxFogMode(js_State *J) { grFogMode(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxFogColorValue(js_State *J) { grFogColorValue(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxChromakeyMode(js_State *J) { grChromakeyMode(js_toboolean(J, 1) ? GR_CHROMAKEY_ENABLE : GR_CHROMAKEY_DISABLE); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxChromakeyValue(js_State *J) { grChromakeyValue(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxAlphaTestFunction(js_State *J) { grAlphaTestFunction(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxAlphaTestReferenceValue(js_State *J) { grAlphaTestReferenceValue(js_toint16(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxTexFilterMode(js_State *J) { grTexFilterMode(js_toint32(J, 1), js_toint32(J, 2), js_toint32(J, 3)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxTexClampMode(js_State *J) { grTexClampMode(js_toint32(J, 1), js_toint32(J, 2), js_toint32(J, 3)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxTexMipMapMode(js_State *J) { grTexMipMapMode(js_toint32(J, 1), js_toint32(J, 2), js_toboolean(J, 3)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxTexLodBiasValue(js_State *J) { grTexLodBiasValue(js_toint32(J, 1), js_tonumber(J, 2)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxTexCombine(js_State *J) {
    grTexCombine(js_toint32(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4), js_tonumber(J, 5), js_toboolean(J, 6), js_toboolean(J, 7));
}
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxTexDetailControl(js_State *J) { grTexDetailControl(js_toint32(J, 1), js_toint32(J, 2), js_toint16(J, 3), js_tonumber(J, 4)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxTexNCCTable(js_State *J) { grTexNCCTable(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxOrigin(js_State *J) { grSstOrigin(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxRenderBuffer(js_State *J) { grRenderBuffer(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxFlush(js_State *J) { grFlush(); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxFinish(js_State *J) { grFinish(); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxSplash(js_State *J) { grSplash(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4), js_toint32(J, 5)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxViewport(js_State *J) { grViewport(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxTexCalcMemRequired(js_State *J) { js_pushnumber(J, grTexCalcMemRequired(js_toint32(J, 1), js_toint32(J, 2), js_toint32(J, 3), js_toint32(J, 4))); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxTexMinAddress(js_State *J) { js_pushnumber(J, grTexMinAddress(js_toint32(J, 1))); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxTexMaxAddress(js_State *J) { js_pushnumber(J, grTexMaxAddress(js_toint32(J, 1))); }

/**
 * @brief get a single value from grGet().
 *
 * @param J VM state.
 * @param pname parameter name
 */
static void fx_get_single(js_State *J, FxU32 pname) {
    FxI32 num;

    if (grGet(pname, sizeof(num), &num)) {
        js_pushnumber(J, num);
    } else {
        js_error(J, "Can't determine value.");
    }
}

/**
 * @brief get multiple values from grGet() as [].
 *
 * @param J VM state.
 * @param pname parameter name
 * @param num_vals number of values.
 */
static void fx_get_multiple(js_State *J, FxU32 pname, int num_vals) {
    FxI32 values[16];  // this is the current max value for all parameters that can be returned

    if (grGet(pname, sizeof(FxI32) * num_vals, values)) {
        js_newarray(J);
        for (int i = 0; i < num_vals; i++) {
            js_pushnumber(J, values[i]);
            js_setindex(J, -2, i);
        }
    } else {
        js_error(J, "Can't determine values.");
    }
}

/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetNumBoards(js_State *J) { fx_get_single(J, GR_NUM_BOARDS); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetNumFb(js_State *J) { fx_get_single(J, GR_NUM_FB); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetNumTmu(js_State *J) { fx_get_single(J, GR_NUM_TMU); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetBitsDepth(js_State *J) { fx_get_single(J, GR_BITS_DEPTH); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetFogTableEntries(js_State *J) { fx_get_single(J, GR_FOG_TABLE_ENTRIES); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetGammaTableEntries(js_State *J) { fx_get_single(J, GR_GAMMA_TABLE_ENTRIES); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxIsBusy(js_State *J) { fx_get_single(J, GR_IS_BUSY); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetMemoryFb(js_State *J) { fx_get_single(J, GR_MEMORY_FB); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetMemoryTmu(js_State *J) { fx_get_single(J, GR_MEMORY_TMU); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetMemoryUma(js_State *J) { fx_get_single(J, GR_MEMORY_UMA); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetMaxTextureSize(js_State *J) { fx_get_single(J, GR_MAX_TEXTURE_SIZE); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetMaxTextureAspectRatio(js_State *J) { fx_get_single(J, GR_MAX_TEXTURE_ASPECT_RATIO); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetNumPendingBufferSwaps(js_State *J) { fx_get_single(J, GR_PENDING_BUFFERSWAPS); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetRevisionFb(js_State *J) { fx_get_single(J, GR_REVISION_FB); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetRevisionTmu(js_State *J) { fx_get_single(J, GR_REVISION_TMU); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetZDepthMinMax(js_State *J) { fx_get_multiple(J, GR_ZDEPTH_MIN_MAX, 2); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void f_fxGetWDepthMinMax(js_State *J) { fx_get_multiple(J, GR_WDEPTH_MIN_MAX, 2); }

#ifdef LFB_3DFX
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxLfbConstantAlpha(js_State *J) { grLfbConstantAlpha(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void f_fxLfbConstantDepth(js_State *J) { grLfbConstantDepth(js_toint32(J, 1)); }
#endif

/**
 * @brief initialize glide.
 *
 * @param J VM state.
 */
static void f_dummy_fxInit(js_State *J) {
    js_error(J, "GLIDE3X.DXE missing, please run one of the V_x.BAT scripts to get the driver matching your hardware! All 3dfx functions are disabled!");
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize a3d subsystem.
 *
 * @param J VM state.
 */
void init_3dfx(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    FILE *f = fopen("GLIDE3X.DXE", "r");
    if (!f) {
        LOG("GLIDE3X.DXE missing, please run one of the V_x.BAT scripts to get the driver matching your hardware! All 3dfx functions are disabled!");

        js_newcfunction(J, f_dummy_fxInit, "fxInit", 0);
        js_setglobal(J, "fxInit");

    } else {
        fclose(f);

        PROPDEF_N(J, WIDTH_3DFX, "FX_WIDTH");
        PROPDEF_N(J, HEIGHT_3DFX, "FX_HEIGHT");

        NFUNCDEF(J, fxInit, 0);
        NFUNCDEF(J, fxShutdown, 0);
        NFUNCDEF(J, fxSplash, 5);
        NFUNCDEF(J, fxFlush, 0);
        NFUNCDEF(J, fxResetVertexLayout, 0);
        NFUNCDEF(J, fxVertexLayout, 1);
        NFUNCDEF(J, fxGetVertexSize, 0);
        NFUNCDEF(J, fxFinish, 0);
        NFUNCDEF(J, fxBufferSwap, 1);
        NFUNCDEF(J, fxBufferClear, 3);
        NFUNCDEF(J, fxClipWindow, 4);
        NFUNCDEF(J, fxDrawPoint, 1);
        NFUNCDEF(J, fxDrawLine, 2);
        NFUNCDEF(J, fxDrawTriangle, 3);
        NFUNCDEF(J, fxConstantColorValue, 1);
        NFUNCDEF(J, fxCullMode, 1);
        NFUNCDEF(J, fxAlphaBlendFunction, 4);
        NFUNCDEF(J, fxAlphaCombine, 5);
        NFUNCDEF(J, fxColorCombine, 5);
        NFUNCDEF(J, fxColorMask, 2);
        NFUNCDEF(J, fxDepthMask, 1);
        NFUNCDEF(J, fxDrawVertexArray, 2);
        NFUNCDEF(J, fxEnable, 1);
        NFUNCDEF(J, fxDisable, 1);
        NFUNCDEF(J, fxDisableAllEffects, 0);
        NFUNCDEF(J, fxAADrawTriangle, 6);
        NFUNCDEF(J, fxDitherMode, 1);
        NFUNCDEF(J, fxAlphaControlsITRGBLighting, 1);
        NFUNCDEF(J, fxGammaCorrectionRGB, 3);
        NFUNCDEF(J, fxOrigin, 1);
        NFUNCDEF(J, fxRenderBuffer, 1);
        NFUNCDEF(J, fxViewport, 4);

        NFUNCDEF(J, fxDepthBufferMode, 1);
        NFUNCDEF(J, fxDepthBufferFunction, 1);
        NFUNCDEF(J, fxDepthBiasLevel, 1);
        NFUNCDEF(J, fxDepthRange, 2);

        NFUNCDEF(J, fxFogMode, 1);
        NFUNCDEF(J, fxFogColorValue, 1);
        NFUNCDEF(J, fxFogTableIndexToW, 0);
        NFUNCDEF(J, fxFogTable, 1);
        NFUNCDEF(J, fxFogGenerateExp, 1);
        NFUNCDEF(J, fxFogGenerateExp2, 1);
        NFUNCDEF(J, fxFogGenerateLinear, 2);

        NFUNCDEF(J, fxChromakeyMode, 1);
        NFUNCDEF(J, fxChromakeyValue, 1);

        NFUNCDEF(J, fxAlphaTestFunction, 1);
        NFUNCDEF(J, fxAlphaTestReferenceValue, 1);

        NFUNCDEF(J, fxTexFilterMode, 3);
        NFUNCDEF(J, fxTexClampMode, 3);
        NFUNCDEF(J, fxTexMipMapMode, 3);
        NFUNCDEF(J, fxTexLodBiasValue, 2);
        NFUNCDEF(J, fxTexCombine, 7);
        NFUNCDEF(J, fxTexDetailControl, 4);
        NFUNCDEF(J, fxTexNCCTable, 1);

        NFUNCDEF(J, fxTexCalcMemRequired, 4);
        NFUNCDEF(J, fxTexMinAddress, 1);
        NFUNCDEF(J, fxTexMaxAddress, 1);

        // GrGet() values
        NFUNCDEF(J, fxGetZDepthMinMax, 0);
        NFUNCDEF(J, fxGetWDepthMinMax, 0);

        NFUNCDEF(J, fxGetBitsDepth, 0);
        NFUNCDEF(J, fxGetFogTableEntries, 0);
        NFUNCDEF(J, fxGetGammaTableEntries, 0);
        NFUNCDEF(J, fxIsBusy, 0);
        NFUNCDEF(J, fxGetMemoryFb, 0);
        NFUNCDEF(J, fxGetMemoryTmu, 0);
        NFUNCDEF(J, fxGetMemoryUma, 0);
        NFUNCDEF(J, fxGetMaxTextureSize, 0);
        NFUNCDEF(J, fxGetMaxTextureAspectRatio, 0);
        NFUNCDEF(J, fxGetNumBoards, 0);
        NFUNCDEF(J, fxGetNumFb, 0);
        NFUNCDEF(J, fxGetNumTmu, 0);
        NFUNCDEF(J, fxGetNumPendingBufferSwaps, 0);
        NFUNCDEF(J, fxGetRevisionFb, 0);
        NFUNCDEF(J, fxGetRevisionTmu, 0);

#ifdef LFB_3DFX
        NFUNCDEF(J, fxLfbConstantAlpha, 1);
        NFUNCDEF(J, fxLfbConstantDepth, 1);
#endif

        FxI32 num;
        if (grGet(GR_NUM_BOARDS, sizeof(num), &num)) {
            LOGF("3dfx cards detected: %ld\n", num);
        }
    }
    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief shutdown glide if needed.
 */
void shutdown_3dfx() {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);
    f_fxShutdown();
    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
