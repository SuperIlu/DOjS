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
#include <glide.h>
#include <math.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "3dfx-glide.h"
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
static void fx_print_vertex(fx_vertex_t *v) {
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
static void fx_init(js_State *J) {
    grGlideInit();
    grSstSelect(0);
    fx_context = grSstWinOpen(0, GR_RESOLUTION_640x480, GR_REFRESH_60Hz, GR_COLORFORMAT_ARGB, GR_ORIGIN_UPPER_LEFT, 2, 1);
    if (!fx_context) {
        js_error(J, "Can't initialize GLIDE3.");
    }
    DOjS.glide_enabled = true;
    grErrorSetCallback(fx_error_callback);

    grCoordinateSpace(GR_WINDOW_COORDS);  // we use window coordinates by default

    LOGF("GR_VENDOR: %s, ", grGetString(GR_VENDOR));
    LOGF("GR_RENDERER: %s, ", grGetString(GR_RENDERER));
    LOGF("GR_VERSION: %s, ", grGetString(GR_VERSION));
    LOGF("GR_HARDWARE: %s, ", grGetString(GR_HARDWARE));
    LOGF("GR_EXTENSION: %s\n", grGetString(GR_EXTENSION));
}

/**
 * @brief define vertex layout.
 *
 * @param J VM state.
 */
static void fx_vertex_layout(js_State *J) {
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
static void fx_reset_vertex_layout(js_State *J) {
    grReset(GR_VERTEX_PARAMETER);
    fx_vertex_size = 0;
}

/**
 * @brief get size of a vertex as defined by fxVertexLayout().
 *
 * @param J VM state.
 */
static void fx_get_vertex_size(js_State *J) { js_pushnumber(J, fx_vertex_size); }

/**
 * @brief shutdown glide again.
 */
static void fx_shutdown() {
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
static void fx_draw_point(js_State *J) {
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
static void fx_draw_line(js_State *J) {
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
static void fx_draw_triangle(js_State *J) {
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
static void fx_aa_draw_triangle(js_State *J) {
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
static void fx_draw_vertex_array(js_State *J) {
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
static void fx_fog_table_index_to_w(js_State *J) {
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
static void fx_fog_table(js_State *J) {
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
static void fx_fog_generate_exp(js_State *J) {
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
static void fx_fog_generate_exp2(js_State *J) {
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
static void fx_fog_generate_linear(js_State *J) {
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
static void fx_buffer_swap(js_State *J) { grBufferSwap(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_buffer_clear(js_State *J) { grBufferClear(js_touint32(J, 1), js_touint32(J, 2), js_touint32(J, 3)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_clip_window(js_State *J) { grClipWindow(js_touint32(J, 1), js_touint32(J, 2), js_touint32(J, 3), js_touint32(J, 4)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_constant_color_value(js_State *J) { grConstantColorValue(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_cull_mode(js_State *J) { grCullMode(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_alpha_blend_function(js_State *J) { grAlphaBlendFunction(js_touint32(J, 1), js_touint32(J, 2), js_touint32(J, 3), js_touint32(J, 4)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_alpha_combine(js_State *J) { grAlphaCombine(js_touint32(J, 1), js_touint32(J, 2), js_touint32(J, 3), js_touint32(J, 4), js_toboolean(J, 5)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_color_combine(js_State *J) { grColorCombine(js_touint32(J, 1), js_touint32(J, 2), js_touint32(J, 3), js_touint32(J, 4), js_toboolean(J, 5)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_color_mask(js_State *J) { grColorMask(js_toboolean(J, 1), js_toboolean(J, 2)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_depth_mask(js_State *J) { grDepthMask(js_toboolean(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_enable(js_State *J) { grEnable(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_disable(js_State *J) { grDisable(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_disable_all_effects(js_State *J) { grDisableAllEffects(); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_dither_mode(js_State *J) { grDitherMode(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_alpha_controls_lighting(js_State *J) { grAlphaControlsITRGBLighting(js_toboolean(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_gamma_correction_rgb(js_State *J) { guGammaCorrectionRGB(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_depth_range(js_State *J) { grDepthRange(js_tonumber(J, 1), js_tonumber(J, 2)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_depth_buffer_mode(js_State *J) { grDepthBufferMode(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_depth_buffer_function(js_State *J) { grDepthBufferFunction(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_depth_bias_level(js_State *J) { grDepthBiasLevel(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_fog_mode(js_State *J) { grFogMode(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_fog_color_value(js_State *J) { grFogColorValue(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_chromakey_mode(js_State *J) { grChromakeyMode(js_toboolean(J, 1) ? GR_CHROMAKEY_ENABLE : GR_CHROMAKEY_DISABLE); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_chromakey_value(js_State *J) { grChromakeyValue(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_alpha_test_mode(js_State *J) { grAlphaTestFunction(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_alpha_test_reference_value(js_State *J) { grAlphaTestReferenceValue(js_toint16(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_tex_filter_mode(js_State *J) { grTexFilterMode(js_toint32(J, 1), js_toint32(J, 2), js_toint32(J, 3)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_tex_clamp_mode(js_State *J) { grTexClampMode(js_toint32(J, 1), js_toint32(J, 2), js_toint32(J, 3)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_tex_mip_map_mode(js_State *J) { grTexMipMapMode(js_toint32(J, 1), js_toint32(J, 2), js_toboolean(J, 3)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_tex_lod_bias_value(js_State *J) { grTexLodBiasValue(js_toint32(J, 1), js_tonumber(J, 2)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_tex_combine(js_State *J) {
    grTexCombine(js_toint32(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4), js_tonumber(J, 5), js_toboolean(J, 6), js_toboolean(J, 7));
}
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_tex_detail_control(js_State *J) { grTexDetailControl(js_toint32(J, 1), js_toint32(J, 2), js_toint16(J, 3), js_tonumber(J, 4)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_tex_ncc_table(js_State *J) { grTexNCCTable(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_origin(js_State *J) { grSstOrigin(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_render_buffer(js_State *J) { grRenderBuffer(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_flush(js_State *J) { grFlush(); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_finish(js_State *J) { grFinish(); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_splash(js_State *J) { grSplash(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4), js_toint32(J, 5)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_viewport(js_State *J) { grViewport(js_tonumber(J, 1), js_tonumber(J, 2), js_tonumber(J, 3), js_tonumber(J, 4)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_tex_calc_mem_required(js_State *J) { js_pushnumber(J, grTexCalcMemRequired(js_toint32(J, 1), js_toint32(J, 2), js_toint32(J, 3), js_toint32(J, 4))); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_tex_min_address(js_State *J) { js_pushnumber(J, grTexMinAddress(js_toint32(J, 1))); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_tex_max_address(js_State *J) { js_pushnumber(J, grTexMaxAddress(js_toint32(J, 1))); }

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
static void fx_get_num_boards(js_State *J) { fx_get_single(J, GR_NUM_BOARDS); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_num_fb(js_State *J) { fx_get_single(J, GR_NUM_FB); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_num_tmu(js_State *J) { fx_get_single(J, GR_NUM_TMU); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_bits_depth(js_State *J) { fx_get_single(J, GR_BITS_DEPTH); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_fog_table_entries(js_State *J) { fx_get_single(J, GR_FOG_TABLE_ENTRIES); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_gamma_table_entries(js_State *J) { fx_get_single(J, GR_GAMMA_TABLE_ENTRIES); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_is_busy(js_State *J) { fx_get_single(J, GR_IS_BUSY); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_memory_fb(js_State *J) { fx_get_single(J, GR_MEMORY_FB); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_memory_tmu(js_State *J) { fx_get_single(J, GR_MEMORY_TMU); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_memory_uma(js_State *J) { fx_get_single(J, GR_MEMORY_UMA); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_max_texture_size(js_State *J) { fx_get_single(J, GR_MAX_TEXTURE_SIZE); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_max_texture_aspect_ratio(js_State *J) { fx_get_single(J, GR_MAX_TEXTURE_ASPECT_RATIO); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_num_pending_buffer_swaps(js_State *J) { fx_get_single(J, GR_PENDING_BUFFERSWAPS); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_revision_fb(js_State *J) { fx_get_single(J, GR_REVISION_FB); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_revision_tmu(js_State *J) { fx_get_single(J, GR_REVISION_TMU); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_zdepth_min_max(js_State *J) { fx_get_multiple(J, GR_ZDEPTH_MIN_MAX, 2); }
/**
 * @brief grGet()
 * @param J VM state.
 */
static void fx_get_wdepth_min_max(js_State *J) { fx_get_multiple(J, GR_WDEPTH_MIN_MAX, 2); }

#ifdef LFB_3DFX
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_lfb_constant_alpha(js_State *J) { grLfbConstantAlpha(js_toint32(J, 1)); }
/**
 * @brief wrapper
 *
 * @param J VM state.
 */
static void fx_lfb_constant_depth(js_State *J) { grLfbConstantDepth(js_toint32(J, 1)); }
#endif

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

    PROPDEF_N(J, WIDTH_3DFX, "FX_WIDTH");
    PROPDEF_N(J, HEIGHT_3DFX, "FX_HEIGHT");

    FUNCDEF(J, fx_init, "fxInit", 0);
    FUNCDEF(J, fx_shutdown, "fxShutdown", 0);
    FUNCDEF(J, fx_splash, "fxSplash", 5);
    FUNCDEF(J, fx_flush, "fxFlush", 0);
    FUNCDEF(J, fx_reset_vertex_layout, "fxResetVertexLayout", 0);
    FUNCDEF(J, fx_vertex_layout, "fxVertexLayout", 1);
    FUNCDEF(J, fx_get_vertex_size, "fxGetVertexSize", 0);
    FUNCDEF(J, fx_finish, "fxFinish", 0);
    FUNCDEF(J, fx_buffer_swap, "fxBufferSwap", 1);
    FUNCDEF(J, fx_buffer_clear, "fxBufferClear", 3);
    FUNCDEF(J, fx_clip_window, "fxClipWindow", 4);
    FUNCDEF(J, fx_draw_point, "fxDrawPoint", 1);
    FUNCDEF(J, fx_draw_line, "fxDrawLine", 2);
    FUNCDEF(J, fx_draw_triangle, "fxDrawTriangle", 3);
    FUNCDEF(J, fx_constant_color_value, "fxConstantColorValue", 1);
    FUNCDEF(J, fx_cull_mode, "fxCullMode", 1);
    FUNCDEF(J, fx_alpha_blend_function, "fxAlphaBlendFunction", 4);
    FUNCDEF(J, fx_alpha_combine, "fxAlphaCombine", 5);
    FUNCDEF(J, fx_color_combine, "fxColorCombine", 5);
    FUNCDEF(J, fx_color_mask, "fxColorMask", 2);
    FUNCDEF(J, fx_depth_mask, "fxDepthMask", 1);
    FUNCDEF(J, fx_draw_vertex_array, "fxDrawVertexArray", 2);
    FUNCDEF(J, fx_enable, "fxEnable", 1);
    FUNCDEF(J, fx_disable, "fxDisable", 1);
    FUNCDEF(J, fx_disable_all_effects, "fxDisableAllEffects", 0);
    FUNCDEF(J, fx_aa_draw_triangle, "fxAADrawTriangle", 6);
    FUNCDEF(J, fx_dither_mode, "fxDitherMode", 1);
    FUNCDEF(J, fx_alpha_controls_lighting, "fxAlphaControlsITRGBLighting", 1);
    FUNCDEF(J, fx_gamma_correction_rgb, "fxGammaCorrectionRGB", 3);
    FUNCDEF(J, fx_origin, "fxOrigin", 1);
    FUNCDEF(J, fx_render_buffer, "fxRenderBuffer", 1);
    FUNCDEF(J, fx_viewport, "fxViewport", 4);

    FUNCDEF(J, fx_depth_buffer_mode, "fxDepthBufferMode", 1);
    FUNCDEF(J, fx_depth_buffer_function, "fxDepthBufferFunction", 1);
    FUNCDEF(J, fx_depth_bias_level, "fxDepthBiasLevel", 1);
    FUNCDEF(J, fx_depth_range, "fxDepthRange", 2);

    FUNCDEF(J, fx_fog_mode, "fxFogMode", 1);
    FUNCDEF(J, fx_fog_color_value, "fxFogColorValue", 1);
    FUNCDEF(J, fx_fog_table_index_to_w, "fxFogTableIndexToW", 0);
    FUNCDEF(J, fx_fog_table, "fxFogTable", 1);
    FUNCDEF(J, fx_fog_generate_exp, "fxFogGenerateExp", 1);
    FUNCDEF(J, fx_fog_generate_exp2, "fxFogGenerateExp2", 1);
    FUNCDEF(J, fx_fog_generate_linear, "fxFogGenerateLinear", 2);

    FUNCDEF(J, fx_chromakey_mode, "fxChromakeyMode", 1);
    FUNCDEF(J, fx_chromakey_value, "fxChromakeyValue", 1);

    FUNCDEF(J, fx_alpha_test_mode, "fxAlphaTestFunction", 1);
    FUNCDEF(J, fx_alpha_test_reference_value, "fxAlphaTestReferenceValue", 1);

    FUNCDEF(J, fx_tex_filter_mode, "fxTexFilterMode", 3);
    FUNCDEF(J, fx_tex_clamp_mode, "fxTexClampMode", 3);
    FUNCDEF(J, fx_tex_mip_map_mode, "fxTexMipMapMode", 3);
    FUNCDEF(J, fx_tex_lod_bias_value, "fxTexLodBiasValue", 2);
    FUNCDEF(J, fx_tex_combine, "fxTexCombine", 7);
    FUNCDEF(J, fx_tex_detail_control, "fxTexDetailControl", 4);
    FUNCDEF(J, fx_tex_ncc_table, "fxTexNCCTable", 1);

    FUNCDEF(J, fx_tex_calc_mem_required, "fxTexCalcMemRequired", 4);
    FUNCDEF(J, fx_tex_min_address, "fxTexMinAddress", 1);
    FUNCDEF(J, fx_tex_max_address, "fxTexMaxAddress", 1);

    // GrGet() values
    FUNCDEF(J, fx_get_zdepth_min_max, "fxGetZDepthMinMax", 0);
    FUNCDEF(J, fx_get_wdepth_min_max, "fxGetWDepthMinMax", 0);

    FUNCDEF(J, fx_get_bits_depth, "fxGetBitsDepth", 0);
    FUNCDEF(J, fx_get_fog_table_entries, "fxGetFogTableEntries", 0);
    FUNCDEF(J, fx_get_gamma_table_entries, "fxGetGammaTableEntries", 0);
    FUNCDEF(J, fx_get_is_busy, "fxIsBusy", 0);
    FUNCDEF(J, fx_get_memory_fb, "fxGetMemoryFb", 0);
    FUNCDEF(J, fx_get_memory_tmu, "fxGetMemoryTMU", 0);
    FUNCDEF(J, fx_get_memory_uma, "fxGetMemoryUma", 0);
    FUNCDEF(J, fx_get_max_texture_size, "fxGetMaxTextureSize", 0);
    FUNCDEF(J, fx_get_max_texture_aspect_ratio, "fxGetMaxTextureAspectRatio", 0);
    FUNCDEF(J, fx_get_num_boards, "fxGetNumBoards", 0);
    FUNCDEF(J, fx_get_num_fb, "fxGetNumFb", 0);
    FUNCDEF(J, fx_get_num_tmu, "fxGetNumTmu", 0);
    FUNCDEF(J, fx_get_num_pending_buffer_swaps, "fxGetNumPendingBufferSwaps", 0);
    FUNCDEF(J, fx_get_revision_fb, "fxGetRevisionFb", 0);
    FUNCDEF(J, fx_get_revision_tmu, "fxGetRevisionTmu", 0);

#ifdef LFB_3DFX
    FUNCDEF(J, fx_lfb_constant_alpha, "fxLfbConstantAlpha", 1);
    FUNCDEF(J, fx_lfb_constant_depth, "fxLfbConstantDepth", 1);
#endif

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief shutdown glide if needed.
 */
void shutdown_3dfx() { fx_shutdown(); }
