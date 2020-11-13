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

#include "bitmap.h"

#include <allegro.h>
#include <loadpng.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>

#include "3dfx-glide.h"
#include "DOjS.h"
#include "color.h"
#include "util.h"
#include "zipfile.h"

#ifdef LFB_3DFX
#include <glide.h>
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

    DEBUGF("finalize 0x%p\n", data);

    // safeguard of someone GCs our current Bitmap
    if (DOjS.current_bm == bm) {
        DOjS.current_bm = DOjS.render_bm;
        LOG("GC of current render Bitmap!");
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
    if (js_isnumber(J, 1) && js_isnumber(J, 2) && js_isnumber(J, 3) && js_isnumber(J, 4) && js_isnumber(J, 5)) {
        int x = js_tonumber(J, 1);
        int y = js_tonumber(J, 2);
        int w = js_tonumber(J, 3);
        int h = js_tonumber(J, 4);
        int b = js_tonumber(J, 5);

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
    } else if (js_isnumber(J, 1) && js_isnumber(J, 2) && js_isnumber(J, 3) && js_isnumber(J, 4)) {
        int x = js_tonumber(J, 1);
        int y = js_tonumber(J, 2);
        int w = js_tonumber(J, 3);
        int h = js_tonumber(J, 4);

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
        // new Bitmap(width, height)
        bm = create_bitmap_ex(32, js_tonumber(J, 1), js_tonumber(J, 2));
        if (!bm) {
            DEBUG("No Memory for Bitmap\n");
            JS_ENOMEM(J);
            return;
        }
        DEBUGF("new Bitmap 0x%p with data=%p\n", bm, bm->dat);
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
            if (stricmp("bmp", ut_getFilenameExt(fname)) == 0) {
                bm = load_bmp_pf(pf, NULL);
            } else if (stricmp("tga", ut_getFilenameExt(fname)) == 0) {
                bm = load_tga_pf(pf, NULL);
            } else if (stricmp("pcx", ut_getFilenameExt(fname)) == 0) {
                bm = load_pcx_pf(pf, NULL);
            } else if (stricmp("png", ut_getFilenameExt(fname)) == 0) {
                bm = load_png_pf(pf, NULL);
            } else {
                bm = NULL;
            }
            pack_fclose(pf);

            if (!bm) {
                js_error(J, "Can't load image '%s'", fname);
                return;
            }
        }
    } else if (js_isarray(J, 1) && js_isnumber(J, 2) && js_isnumber(J, 3)) {
        // new Bitmap(data[], width, height)
        int w = js_tonumber(J, 2);
        int h = js_tonumber(J, 3);
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
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, bm->w);
    js_defproperty(J, -2, "width", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, bm->h);
    js_defproperty(J, -2, "height", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
}

/**
 * @brief draw the image to the canvas.
 * img.Draw(x, y)
 *
 * @param J VM state.
 */
static void Bitmap_Draw(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
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
    int srcX = js_toint16(J, 1);
    int srcY = js_toint16(J, 2);
    int srcW = js_toint16(J, 3);
    int srcH = js_toint16(J, 4);

    int destX = js_toint16(J, 5);
    int destY = js_toint16(J, 6);
    int destW = js_toint16(J, 7);
    int destH = js_toint16(J, 8);
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
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
    draw_trans_sprite(DOjS.current_bm, bm, x, y);
}

#ifdef LFB_3DFX
static void Bitmap_FxDrawLfb(js_State *J) {
    BITMAP *bm = js_touserdata(J, 0, TAG_BITMAP);
    uint16_t *buf = malloc(bm->w * bm->h * sizeof(uint16_t));
    if (!buf) {
        JS_ENOMEM(J);
        return;
    }

    // TODO: buffer conversion!

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

    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);
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

    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

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
        js_error(J, "Can't save Bitmap to BMP file '%s': %s", fname, allegro_error);
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
        js_error(J, "Can't save Bitmap to PCX file '%s': %s", fname, allegro_error);
    }
}

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
        js_error(J, "Can't save Bitmap to TGA file '%s': %s", fname, allegro_error);
    }
}

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
        js_error(J, "Can't save Bitmap to PNG file '%s': %s", fname, allegro_error);
    }
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize bitmap subsystem.
 *
 * @param J VM state.
 */
void init_bitmap(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    /* Make Allegro aware of PNG file format. */
    register_png_file_type();

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
        NPROTDEF(J, Bitmap, SavePngImage, 1);
#ifdef LFB_3DFX
        NPROTDEF(J, Bitmap, FxDrawLfb, 4);
#endif
    }
    CTORDEF(J, new_Bitmap, TAG_BITMAP, 5);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
