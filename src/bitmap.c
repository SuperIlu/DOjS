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

#include "bitmap.h"

#include <allegro.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>

#include "DOjS.h"
#include "color.h"
#include "util.h"
#include "zipfile.h"
#include "blurhash.h"
#include "bytearray.h"

#if LINUX == 1
#include "loadpng.h"
#include "qoi.h"
#include "webp.h"
#include "linux/glue.h"
#else
#include "3dfx-glide.h"
#ifdef LFB_3DFX
#include <glide.h>
#endif
#endif

/*********************
** static functions **
*********************/
/**
 * @brief finalize an image and free resources.
 *
 * @param J VM state.
 */
static void Bitmap_Finalize(js_State *J, void *data) {
    BITMAP *bm = (BITMAP *)data;

    DEBUGF("%s: finalize 0x%p\n", __PRETTY_FUNCTION__, data);

    // safeguard of someone GCs our current Bitmap
    if (DOjS.current_bm == bm) {
        DOjS.current_bm = DOjS.render_bm;
        LOG("GC of current render Bitmap!\n");
    }

    destroy_bitmap(bm);
}

/**
 * @brief load an image or create an empty bitmap.
 * new Bitmap(filename:string)
 * new Bitmap(width:number, height:number)
 * new Bitmap(data:number[], width:number, height:number)
 * new Bitmap(x:number, y:number, width:number, height:number)
 * new Bitmap(x:number, y:number, width:number, height:number, buffer:GR_BUFFER)
 *
 * @param J VM state.
 */
static void new_Bitmap(js_State *J) {
    NEW_OBJECT_PREP(J);
    const char *fname = "<<buffer>>";
    BITMAP *bm = NULL;

#if LINUX != 1
    if (js_isnumber(J, 1) && js_isnumber(J, 2) && js_isnumber(J, 3) && js_isnumber(J, 4) && js_isnumber(J, 5)) {
        // 3dfx framebuffer
        uint16_t x = js_touint16(J, 1);
        uint16_t y = js_touint16(J, 2);
        uint16_t w = js_touint16(J, 3);
        uint16_t h = js_touint16(J, 4);
        uint16_t b = js_touint16(J, 5);

        if (x < 0 || x > WIDTH_3DFX || y < 0 || y > HEIGHT_3DFX || w < 0 || h < 0 || x + w > WIDTH_3DFX || y + h > HEIGHT_3DFX) {
            js_error(J, "Bitmap rectangle out of range %dx%d -> %dx%d.", x, y, x + w, y + h);
            return;
        }

        uint16_t *buf = malloc(w * h * sizeof(uint16_t));
        if (!buf) {
            JS_ENOMEM(J);
            return;
        }

        bm = create_bitmap(w, h);
        if (!bm) {
            free(buf);
            JS_ENOMEM(J);
            return;
        }

        grLfbReadRegion(b, x, y, w, h, w * 2, buf);

        /* Create Bitmap from framebuffer */
        for (int py = 0; py < bm->h; py++) {
            for (int px = 0; px < bm->w; px++) {
                uint16_t rgb = buf[bm->w * py + px];
                uint8_t red = (rgb & 0xF800) >> 8;
                uint8_t grn = (rgb & 0x07E0) >> 3;
                uint8_t blu = (rgb & 0x001F) << 3;
                uint32_t argb = 0xFF000000 | (red << 16) | (grn << 8) | blu;
                putpixel(bm, px, py, argb);
            }
        }

        free(buf);
    } else
#endif
        if (js_isnumber(J, 1) && js_isnumber(J, 2) && js_isnumber(J, 3) && js_isnumber(J, 4)) {
        // allegro framebuffer
        uint16_t x = js_touint16(J, 1);
        uint16_t y = js_touint16(J, 2);
        uint16_t w = js_touint16(J, 3);
        uint16_t h = js_touint16(J, 4);

        if (x < 0 || x > DOjS.current_bm->w || y < 0 || y > DOjS.current_bm->w || w < 0 || h < 0 || x + w > DOjS.current_bm->w || y + h > DOjS.current_bm->h) {
            js_error(J, "Bitmap rectangle out of range %dx%d -> %dx%d.", x, y, x + w, y + h);
            return;
        }

        bm = create_bitmap(w, h);
        if (!bm) {
            JS_ENOMEM(J);
            return;
        }

        blit(DOjS.current_bm, bm, x, y, 0, 0, w, h);
    } else if (js_isnumber(J, 1) && js_isnumber(J, 2)) {
        // new Bitmap(width, height [, color])
        bm = create_bitmap_ex(32, js_touint16(J, 1), js_touint16(J, 2));
        if (!bm) {
            DEBUG("No Memory for Bitmap\n");
            JS_ENOMEM(J);
            return;
        }
        DEBUGF("new Bitmap 0x%p with data=%p\n", bm, bm->dat);

        // clear with given color
        if (js_isdefined(J, 3)) {
            clear_to_color(bm, js_toint32(J, 3));
        } else {
            clear_to_color(bm, 0);
        }
    } else if (js_isstring(J, 1) && js_isnumber(J, 2) && js_isnumber(J, 3)) {
        // new Bitmap("blurhash", width, height, [punch])
        const char *hash = js_tostring(J, 1);
        uint16_t w = js_touint16(J, 2);
        uint16_t h = js_touint16(J, 3);

        int punch = 1;
        if (js_isnumber(J, 4)) {
            punch = js_toint32(J, 4);
        }

        if (!isValidBlurhash(hash)) {
            js_error(J, "No valid blurhash: %s", hash);
            return;
        }

        size_t pixel_size = w * h;
        uint8_t *buf = malloc(pixel_size * 3);
        if (!buf) {
            JS_ENOMEM(J);
            return;
        }

        bm = create_bitmap_ex(32, w, h);
        if (!bm) {
            free(buf);
            DEBUG("No Memory for Bitmap\n");
            JS_ENOMEM(J);
            return;
        }
        clear_bitmap(bm);

        if (decodeToArray(hash, w, h, punch, 3, buf) != 0) {
            free(buf);
            destroy_bitmap(bm);
            js_error(J, "No valid blurhash: %s", hash);
            return;
        }

        int src = 0;
        int dst = 0;
        while (dst < pixel_size) {
            int red = buf[src++];
            int grn = buf[src++];
            int blu = buf[src++];

            uint32_t argb = 0xFF000000 | (red << 16) | (grn << 8) | blu;
            putpixel(bm, dst % w, dst / w, argb);
            dst++;
        }
        free(buf);
    } else if (js_isuserdata(J, 1, TAG_BYTE_ARRAY)) {
        byte_array_t *ba = js_touserdata(J, 1, TAG_BYTE_ARRAY);
        char *type = ut_getBitmapType(ba);
        if (!type) {
            js_error(J, "Unknown image format in ByteArray");
            return;
        }

        PACKFILE *pf = open_bytearray(ba);
        if (!pf) {
            js_error(J, "Can't load image from ByteArray(1): %s", type);
            return;
        }
        bm = load_bitmap_pf(pf, NULL, type);
        pack_fclose(pf);

        if (!bm) {
            js_error(J, "Can't load image from ByteArray(2): %s", type);
            return;
        }
    } else if (js_isstring(J, 1)) {
        // new Bitmap("filename")
        fname = js_tostring(J, 1);

        char *delim = strchr(fname, ZIP_DELIM);

        if (!delim) {
            bm = load_bitmap(fname, NULL);
            if (!bm) {
                js_error(J, "Can't load image '%s'", fname);
                return;
            }
        } else {
            PACKFILE *pf = open_zipfile1(fname);
            if (!pf) {
                js_error(J, "Can't load image '%s'", fname);
                return;
            }
            bm = load_bitmap_pf(pf, NULL, ut_getFilenameExt(fname));
            pack_fclose(pf);

            if (!bm) {
                js_error(J, "Can't load image '%s'", fname);
                return;
            }
        }
    } else if (js_isarray(J, 1) && js_isnumber(J, 2) && js_isnumber(J, 3)) {
        // new Bitmap(data[], width, height)
        uint16_t w = js_touint16(J, 2);
        uint16_t h = js_touint16(J, 3);
        bm = create_bitmap_ex(32, w, h);
        if (!bm) {
            DEBUG("No Memory for Bitmap\n");
            JS_ENOMEM(J);
            return;
        }
        clear_bitmap(bm);
        int arr_len = js_getlength(J, 1);
        int len = arr_len < w * h ? arr_len : w * h;
        for (int i = 0; i < len; i++) {
            js_getindex(J, 1, i);
            putpixel(bm, i % w, i / w, js_touint32(J, -1));
            js_pop(J, 1);
        }
    } else {
        js_error(J, "Unsupported contructor call.");
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_BITMAP, bm, Bitmap_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, bm->w);
    js_defproperty(J, -2, "width", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, bm->h);
    js_defproperty(J, -2, "height", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief draw the image to the canvas.
 * img.Draw(x, y)
 *
 * @param J VM state.
 */
static void Bitmap_Draw(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    uint16_t x = js_touint16(J, 1);
    uint16_t y = js_touint16(J, 2);
    blit(bm, DOjS.current_bm, 0, 0, x, y, bm->w, bm->h);
}

/**
 * @brief draw the image to the canvas.
 * img.DrawAdvanced(source_x, source_y, source_width, source_height, dest_x, dest_y, dest_width, dest_height)
 *
 * @param J VM state.
 */
static void Bitmap_DrawAdvanced(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    uint16_t srcX = js_touint16(J, 1);
    uint16_t srcY = js_touint16(J, 2);
    uint16_t srcW = js_touint16(J, 3);
    uint16_t srcH = js_touint16(J, 4);
    if (srcW > bm->w) {
        srcW = bm->w;
    }
    if (srcH > bm->h) {
        srcH = bm->h;
    }

    unsigned int destX = js_touint16(J, 5);
    unsigned int destY = js_touint16(J, 6);
    unsigned int destW = js_touint16(J, 7);
    unsigned int destH = js_touint16(J, 8);
    stretch_blit(bm, DOjS.current_bm, srcX, srcY, srcW, srcH, destX, destY, destW, destH);
}

/**
 * @brief clear the bitmap.
 * img.Clear()
 *
 * @param J VM state.
 */
static void Bitmap_Clear(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    clear_bitmap(bm);
}

/**
 * @brief draw the image to the canvas.
 * img.DrawTrans(x, y)
 *
 * @param J VM state.
 */
static void Bitmap_DrawTrans(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    uint16_t x = js_touint16(J, 1);
    uint16_t y = js_touint16(J, 2);
    if (DOjS.params.no_alpha) {
        blit(bm, DOjS.current_bm, 0, 0, x, y, bm->w, bm->h);
    } else {
        draw_trans_sprite(DOjS.current_bm, bm, x, y);
    }
}

#ifdef LFB_3DFX
static void Bitmap_FxDrawLfb(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    uint16_t *buf = malloc(bm->w * bm->h * sizeof(uint16_t));
    if (!buf) {
        JS_ENOMEM(J);
        return;
    }

    /* Create Source Bitmap to be copied to framebuffer */
    for (int y = 0; y < bm->h; y++) {
        for (int x = 0; x < bm->w; x++) {
            uint32_t argb = getpixel(bm, x, y);
            FxU8 red = (argb >> 16) & 0xFF;
            FxU8 grn = (argb >> 8) & 0xFF;
            FxU8 blu = argb & 0xFF;
            buf[bm->w * y + x] = (red & 0xF8) << 8 | (grn & 0xFC) << 3 | (blu & 0xF8) >> 3;
        }
    }

    uint16_t x = js_touint16(J, 1);
    uint16_t y = js_touint16(J, 2);
    int buffer = js_toint16(J, 3);
    bool pPipeline = js_toboolean(J, 4);

    FxBool ret = grLfbWriteRegion(buffer, x, y, GR_LFB_SRC_FMT_565, bm->w, bm->h, pPipeline, bm->w * 2, buf);

    free(buf);

    if (!ret) {
        js_error(J, "LfbWriteRegion failed.");
        return;
    }
}
#endif

/**
 * @brief get the color of an image pixel.
 * img.GetPixel(x, y):Color
 *
 * @param J the JS context.
 */
static void Bitmap_GetPixel(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);

    uint16_t x = js_touint16(J, 1);
    uint16_t y = js_touint16(J, 2);

    js_pushnumber(J, getpixel(bm, x, y) | 0xFF000000);  // no alpha in bitmaps so far
}

/**
 * @brief save Bitmap to file.
 * SaveBmpImage(fname:string)
 *
 * @param J the JS context.
 */
static void Bitmap_SaveBmpImage(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    PALETTE pal;
    get_palette(pal);

    if (save_bmp(fname, bm, (const struct RGB *)&pal) != 0) {
        js_error(J, "Can't save Bitmap to BMP file '%s'", fname);
    }
}

/**
 * @brief save Bitmap to file.
 * SavePcxImage(fname:string)
 *
 * @param J the JS context.
 */
static void Bitmap_SavePcxImage(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    PALETTE pal;
    get_palette(pal);

    if (save_pcx(fname, bm, (const struct RGB *)&pal) != 0) {
        js_error(J, "Can't save Bitmap to PCX file '%s'", fname);
    }
}

#if LINUX == 1
/**
 * @brief save Bitmap to file.
 * SavePngImage(fname:string)
 *
 * @param J the JS context.
 */
static void Bitmap_SavePngImage(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    PALETTE pal;
    get_palette(pal);

    if (save_png(fname, bm, (const struct RGB *)&pal) != 0) {
        js_error(J, "Can't save Bitmap to PNG file '%s'", fname);
    }
}

/**
 * @brief save Bitmap to file.
 * SaveWoiImage(fname:string)
 *
 * @param J the JS context.
 */
static void Bitmap_SaveQoiImage(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    if (!save_qoi(bm, fname)) {
        js_error(J, "Can't save Bitmap to QOI file '%s'", fname);
    }
}

/**
 * @brief save Bitmap to file.
 * SaveWebpImage(fname:string)
 *
 * @param J the JS context.
 */
static void Bitmap_SaveWebpImage(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    if (!save_webp(bm, fname)) {
        js_error(J, "Can't save Bitmap to WEBP file '%s'", fname);
    }
}
#endif

/**
 * @brief save Bitmap to file.
 * SaveTgaImage(fname:string)
 *
 * @param J the JS context.
 */
static void Bitmap_SaveTgaImage(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    const char *fname = js_tostring(J, 1);

    PALETTE pal;
    get_palette(pal);

    if (save_tga(fname, bm, (const struct RGB *)&pal) != 0) {
        js_error(J, "Can't save Bitmap to TGA file '%s'", fname);
    }
}

/***********************
** exported functions **
***********************/

/**
 * @brief create a bitmap from RGBA data.
 *
 * @param J VM state.
 * @param data RGBA data encoded in uint8_t, must contain w*h*4 bytes
 * @param w width of the image
 * @param h height ob the image
 */
void Bitmap_fromRGBA(js_State *J, const uint8_t *data, int w, int h) {
    NEW_OBJECT_PREP(J);
    const char *fname = "<<buffer>>";
    BITMAP *bm = NULL;

    bm = create_bitmap_ex(32, w, h);
    if (!bm) {
        DEBUG("No Memory for Bitmap\n");
        JS_ENOMEM(J);
        return;
    }
    clear_bitmap(bm);
    int len = w * h * 4 * sizeof(uint8_t);
    int idx = 0;
    for (int i = 0; i < len; i++) {
        int r = data[idx++];
        int g = data[idx++];
        int b = data[idx++];
        int a = data[idx++];
        putpixel(bm, i % w, i / w, makeacol32(r, g, b, a));
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_BITMAP, bm, Bitmap_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, bm->w);
    js_defproperty(J, -2, "width", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, bm->h);
    js_defproperty(J, -2, "height", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief initialize bitmap subsystem.
 *
 * @param J VM state.
 */
void init_bitmap(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    // define the Bitmap() object
    js_newobject(J);
    {
        NPROTDEF(J, Bitmap, Draw, 2);
        NPROTDEF(J, Bitmap, DrawAdvanced, 8);
        NPROTDEF(J, Bitmap, Clear, 0);
        NPROTDEF(J, Bitmap, DrawTrans, 2);
        NPROTDEF(J, Bitmap, GetPixel, 2);
        NPROTDEF(J, Bitmap, SaveBmpImage, 1);
        NPROTDEF(J, Bitmap, SavePcxImage, 1);
        NPROTDEF(J, Bitmap, SaveTgaImage, 1);
#if LINUX == 1
        NPROTDEF(J, Bitmap, SavePngImage, 1);
        NPROTDEF(J, Bitmap, SaveQoiImage, 1);
        NPROTDEF(J, Bitmap, SaveWebpImage, 1);
#endif
#ifdef LFB_3DFX
        NPROTDEF(J, Bitmap, FxDrawLfb, 4);
#endif
    }
    CTORDEF(J, new_Bitmap, TAG_BITMAP, 5);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
