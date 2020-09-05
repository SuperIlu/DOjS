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

#include "3dfx-texinfo.h"
#include "DOjS.h"
#include "bitmap.h"
#include "util.h"

/************
** defines **
************/
#define FX_NONE 0xFFFFFFFF

/************
** structs **
************/
typedef FxU32 TlPalette[256];
typedef struct {
    FxU8 yRGB[16];
    FxI16 iRGB[4][3];
    FxI16 qRGB[4][3];
    FxU32 packed_data[12];
} TlNCCTable;

typedef union {
    TlPalette palette;
    TlNCCTable nccTable;
} TlTextureTable;

typedef struct {
    GrTexInfo info;
    GrTexTable_t tableType;
    FxU32 textureSize;
    FxU32 startAddress;
    GrChipID_t tmu;
    TlTextureTable tableData;
} TlTexture;

/*********************
** static functions **
*********************/
/**
 * @brief convert texture format to texture table type.
 *
 * @param format the format of the 3df file.
 *
 * @return GrTexTable_t the table format.
 */
static GrTexTable_t texTableType(GrTextureFormat_t format) {
    GrTexTable_t rv = (GrTexTable_t)FX_NONE;
    switch (format) {
        case GR_TEXFMT_YIQ_422:
        case GR_TEXFMT_AYIQ_8422:
            rv = GR_TEXTABLE_NCC0;
            break;
        case GR_TEXFMT_P_8:
        case GR_TEXFMT_AP_88:
            rv = GR_TEXTABLE_PALETTE;
            break;
    }
    return rv;
}

/**
 * @brief finalize and free resources.
 *
 * @param J VM state.
 */
static void Texinfo_Finalize(js_State *J, void *data) {
    TlTexture *ti = (TlTexture *)data;
    free(ti->info.data);
    free(ti);
}

/**
 * @brief new TexInfo(fname:string)
 *
 * @param J VM state.
 */
static void new_Texinfo(js_State *J) {
    NEW_OBJECT_PREP(J);
    Gu3dfInfo tdfInfo;

    TlTexture *ti = calloc(1, sizeof(TlTexture));
    if (!ti) {
        JS_ENOMEM(J);
        return;
    }

    const char *fname = "<<bitmap>>";
    if (js_isuserdata(J, 1, TAG_BITMAP)) {
        BITMAP *bm = js_touserdata(J, 1, TAG_BITMAP);

        // check source bitmap for sanity
        char *err = NULL;
        if (bm->w > 256 || bm->h > 256) {
            err = "Bitmap size exceeds 256x256.";
        }
        if (bm->w != bm->h) {
            err = "Bitmap is not rectangular.";
        }
        if (bm->w % 2) {
            err = "Bitmap size no multiple of 2.";
        }
        if (err) {
            js_error(J, err);
            free(ti);
            return;
        }

        // set texture format
        ti->tableType = FX_NONE;
        ti->info.format = GR_TEXFMT_RGB_565;
        ti->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;

        // determine lodt
        ti->info.largeLodLog2 = ti->info.smallLodLog2 = 0;
        for (int i = GR_LOD_LOG2_1; i <= GR_LOD_LOG2_256; i++) {
            if (bm->w == (1 << i)) {
                ti->info.largeLodLog2 = ti->info.smallLodLog2 = i;
                break;
            }
        }

        // allocate memory
        ti->textureSize = bm->w * bm->h * sizeof(uint16_t);
        ti->info.data = malloc(ti->textureSize);
        if (!ti->info.data) {
            JS_ENOMEM(J);
            free(ti);
            return;
        }

        // convert bitmap into new memory
        uint16_t *dst = ti->info.data;
        for (int y = 0; y < bm->h; y++) {
            for (int x = 0; x < bm->w; x++) {
                uint32_t argb = getpixel(bm, x, y);
                *dst = (((argb >> 8) & 0xF800) | ((argb >> 5) & 0x07E0) | ((argb >> 3) & 0x001F));
                dst++;
            }
        }
    } else {
        fname = js_tostring(J, 1);

        if (gu3dfGetInfo(fname, &tdfInfo)) {
            ti->textureSize = tdfInfo.mem_required;
            tdfInfo.data = malloc(tdfInfo.mem_required);
            if (!tdfInfo.data) {
                JS_ENOMEM(J);
                free(ti);
                return;
            }
            if (gu3dfLoad(fname, &tdfInfo)) {
                ti->info.smallLodLog2 = tdfInfo.header.small_lod;
                ti->info.largeLodLog2 = tdfInfo.header.large_lod;
                ti->info.aspectRatioLog2 = tdfInfo.header.aspect_ratio;
                ti->info.format = tdfInfo.header.format;
                ti->info.data = tdfInfo.data;
                ti->tableType = texTableType(ti->info.format);
                switch (ti->tableType) {
                    case GR_TEXTABLE_NCC0:
                    case GR_TEXTABLE_NCC1:
                    case GR_TEXTABLE_PALETTE:
                        memcpy(&ti->tableData, &(tdfInfo.table), sizeof(TlTextureTable));
                        break;
                    default:
                        break;
                }
            } else {
                free(tdfInfo.data);
                js_error(J, "Can't load '%s'!", fname);
                free(ti);
                return;
            }
        } else {
            js_error(J, "Can't load '%s'!", fname);
            free(ti);
            return;
        }
    }
    ti->startAddress = FX_NONE;
    ti->tmu = FX_NONE;

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_TEXINFO, ti, Texinfo_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, ti->info.largeLodLog2);
    js_defproperty(J, -2, "largeLod", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, ti->info.smallLodLog2);
    js_defproperty(J, -2, "smallLod", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, ti->info.aspectRatioLog2);
    js_defproperty(J, -2, "aspectRatio", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, ti->info.format);
    js_defproperty(J, -2, "format", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, ti->tableType);
    js_defproperty(J, -2, "tableType", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, ti->textureSize);
    js_defproperty(J, -2, "textureSize", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, ti->startAddress);
    js_defproperty(J, -2, "address", JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, ti->tmu);
    js_defproperty(J, -2, "tmu", JS_DONTENUM | JS_DONTCONF);
}

/**
 * @brief ti.DownloadMipMap(tmu:number, address:number, evenOdd:number)
 *
 * @param J VM state.
 */
static void Texinfo_DownloadMipMap(js_State *J) {
    TlTexture *ti = js_touserdata(J, 0, TAG_TEXINFO);
    GrChipID_t tmu = js_toint32(J, 1);
    FxU32 startAddress = js_toint32(J, 2);
    FxU32 evenOdd = js_toint32(J, 3);
    grTexDownloadMipMap(tmu, startAddress, evenOdd, &ti->info);
    if (ti->tableType != FX_NONE) {
        grTexDownloadTable(ti->tableType, &ti->tableData);
    }

    ti->startAddress = startAddress;
    js_pushnumber(J, ti->startAddress);
    js_setproperty(J, -2, "address");

    ti->tmu = tmu;
    js_pushnumber(J, ti->tmu);
    js_setproperty(J, -2, "tmu");
}

/**
 * @brief ti.MemRequired(evenOdd:number)
 *
 * @param J VM state.
 */
static void Texinfo_TexMemRequired(js_State *J) {
    TlTexture *ti = js_touserdata(J, 0, TAG_TEXINFO);
    FxU32 evenOdd = js_toint32(J, 1);

    js_pushnumber(J, grTexTextureMemRequired(evenOdd, &ti->info));
}

/**
 * @brief ti.MemRequired()
 *
 * @param J VM state.
 */
static void Texinfo_MarkUnused(js_State *J) {
    TlTexture *ti = js_touserdata(J, 0, TAG_TEXINFO);

    ti->startAddress = FX_NONE;
    js_pushnumber(J, ti->startAddress);
    js_setproperty(J, -2, "address");

    ti->tmu = FX_NONE;
    js_pushnumber(J, ti->tmu);
    js_setproperty(J, -2, "tmu");
}

/**
 * @brief ti.TexSource(evenOdd:number)
 *
 * @param J VM state.
 */
static void Texinfo_TexSource(js_State *J) {
    TlTexture *ti = js_touserdata(J, 0, TAG_TEXINFO);
    FxU32 evenOdd = js_toint32(J, 1);

    if (ti->startAddress != FX_NONE) {
        grTexSource(ti->tmu, ti->startAddress, evenOdd, &ti->info);
    } else {
        js_error(J, "Texture not downloaded to TMU!");
        return;
    }
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize texinfo class.
 *
 * @param J VM state.
 */
void init_texinfo(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        PROTDEF(J, Texinfo_DownloadMipMap, TAG_TEXINFO, "DownloadMipMap", 3);
        PROTDEF(J, Texinfo_MarkUnused, TAG_TEXINFO, "MarkUnused", 0);
        PROTDEF(J, Texinfo_TexSource, TAG_TEXINFO, "Source", 1);
        PROTDEF(J, Texinfo_TexMemRequired, TAG_TEXINFO, "MemRequired", 1);
    }
    js_newcconstructor(J, new_Texinfo, new_Texinfo, TAG_TEXINFO, 1);
    js_defglobal(J, TAG_TEXINFO, JS_DONTENUM);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
