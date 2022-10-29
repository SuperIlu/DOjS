/*
MIT License

Copyright (c) 2019-2022 Andre Seidelt <superilu@yahoo.com>

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
#include <stdio.h>
#include <string.h>

#include "pdfgen.h"
#include "bitmap.h"

#include "DOjS.h"

void init_pdfgen(js_State *J);

/************
** defines **
************/
#define TAG_PDFGEN "PDFGen"  //!< pointer tag

//! push named PDF_xxx value
#define PDF_PUSH_VALUE(n) \
    { PROPDEF_N(J, n, #n); }

//! convert Allegro color to PDF color
#define PDF_CONVERT_COLOR(c) PDF_ARGB(0xFF - geta(c), getr(c), getg(c), getb(c))

/************
** structs **
************/
//! file userdata definition
typedef struct __pdfgen {
    struct pdf_doc *pdf;
} pdfgen_t;

//! polygon data as needed by the drawing functions.
typedef struct poly_array {
    int len;   //!< number of entries
    float *x;  //!< an array: x coordinates
    float *y;  //!< an array: y coordinates
} poly_array_t;

/*********************
** static functions **
*********************/

/**
 * @brief finalize a file and free resources.
 *
 * @param J VM state.
 */
static void PG_Finalize(js_State *J, void *data) {
    pdfgen_t *pg = (pdfgen_t *)data;
    pdf_destroy(pg->pdf);
    free(pg);
}

/**
 * @brief create new PDF
 * gp = new PDFGen(width:number, height:number)
 *
 * @param J VM state.
 */
static void new_Pdfgen(js_State *J) {
    NEW_OBJECT_PREP(J);

    pdfgen_t *pg = calloc(1, sizeof(pdfgen_t));
    if (!pg) {
        JS_ENOMEM(J);
        return;
    }
    bzero(pg, sizeof(pdfgen_t));

    float width = js_tonumber(J, 1);
    float height = js_tonumber(J, 2);

    if ((width < 1) || (height < 1)) {
        js_error(J, "Invalid PDF size: negative");
        return;
    }

    struct pdf_info info = {
        .creator = "DOjS " DOSJS_VERSION_STR,  // DOjS
        .producer = "PDFGen 0.1.0",            // plugin
        .title = "My document",                // doc title
        .author = "My name",                   // author
        .subject = "My subject"                // subject
    };
    pg->pdf = pdf_create(width, height, &info);

    if (!pg->pdf) {
        js_error(J, "Unable to create PDF");
        return;
    }
    pdf_clear_err(pg->pdf);

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_PDFGEN, pg, PG_Finalize);

    // add properties
    js_pushnumber(J, pdf_width(pg->pdf));
    js_defproperty(J, -2, "width", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, pdf_height(pg->pdf));
    js_defproperty(J, -2, "height", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief write PDF.
 * pg.Save(filename:string)
 *
 * @param J VM state.
 */
static void PG_Save(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    const char *fname = js_tostring(J, 1);
    if (pdf_save(pg->pdf, fname) < 0) {
        js_error(J, "Could not save PDF to '%s': %s", fname, pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief add a page to the PDF and make it current page.
 * pg.AppendPage()
 *
 * @param J VM state.
 */
static void PG_AppendPage(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    if (!pdf_append_page(pg->pdf)) {
        js_error(J, "Could not add page: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief chose text font.
 * pg.SetFont(font:string)
 *
 * @param J VM state.
 */
static void PG_SetFont(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    const char *font = js_tostring(J, 1);
    if (pdf_set_font(pg->pdf, font) < 0) {
        js_error(J, "Could not set font to '%s': %s", font, pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief draw a line.
 * pg.AddLine(x1:number, y1:number, x2:number, y2:number, width:number, col:Color)
 *
 * @param J VM state.
 */
static void PG_AddLine(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    float x1 = js_tonumber(J, 1);
    float y1 = js_tonumber(J, 2);
    float x2 = js_tonumber(J, 3);
    float y2 = js_tonumber(J, 4);
    float bw = js_tonumber(J, 5);
    uint32_t c = js_touint32(J, 6);
    if (pdf_add_line(pg->pdf, NULL, x1, y1, x2, y2, bw, PDF_CONVERT_COLOR(c)) < 0) {
        js_error(J, "Could not add line: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief draw a circle.
 * pg.AddCircle(x:number, y:number, r:number, width:number, col:Color, fill_col:Color)
 *
 * @param J VM state.
 */
static void PG_AddCircle(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    float x = js_tonumber(J, 1);
    float y = js_tonumber(J, 2);
    float r = js_tonumber(J, 3);
    float bw = js_tonumber(J, 4);
    uint32_t c = js_touint32(J, 5);
    uint32_t fc = js_touint32(J, 6);
    if (pdf_add_circle(pg->pdf, NULL, x, y, r, bw, PDF_CONVERT_COLOR(c), PDF_CONVERT_COLOR(fc)) < 0) {
        js_error(J, "Could not add circle: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief draw an ellipse.
 * pg.AddEllipse(x:number, y:number, xr:number, yr:number, width:number, col:Color, fill_col:Color)
 *
 * @param J VM state.
 */
static void PG_AddEllipse(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    float x = js_tonumber(J, 1);
    float y = js_tonumber(J, 2);
    float xr = js_tonumber(J, 3);
    float yr = js_tonumber(J, 4);
    float bw = js_tonumber(J, 5);
    uint32_t c = js_touint32(J, 6);
    uint32_t fc = js_touint32(J, 7);
    if (pdf_add_ellipse(pg->pdf, NULL, x, y, xr, yr, bw, PDF_CONVERT_COLOR(c), PDF_CONVERT_COLOR(fc)) < 0) {
        js_error(J, "Could not add ellipse: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief draw a rectangle.
 * pg.AddRectangle(x:number, y:number, w:number, h:number, width:number, col:Color)
 *
 * @param J VM state.
 */
static void PG_AddRectangle(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    float x = js_tonumber(J, 1);
    float y = js_tonumber(J, 2);
    float w = js_tonumber(J, 3);
    float h = js_tonumber(J, 4);
    float bw = js_tonumber(J, 5);
    uint32_t c = js_touint32(J, 6);
    if (pdf_add_rectangle(pg->pdf, NULL, x, y, w, h, bw, PDF_CONVERT_COLOR(c)) < 0) {
        js_error(J, "Could not add rectangle: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief draw a filled rectangle.
 * pg.AddFilledRectangle(x:number, y:number, w:number, h:number, width:number, col:Color)
 *
 * @param J VM state.
 */
static void PG_AddFilledRectangle(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    float x = js_tonumber(J, 1);
    float y = js_tonumber(J, 2);
    float w = js_tonumber(J, 3);
    float h = js_tonumber(J, 4);
    float bw = js_tonumber(J, 5);
    uint32_t c = js_touint32(J, 6);
    if (pdf_add_filled_rectangle(pg->pdf, NULL, x, y, w, h, bw, PDF_CONVERT_COLOR(c)) < 0) {
        js_error(J, "Could not add filled rectangle: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief add text.
 * pg.AddText(txt:string, size:number, x:number, y:number, col:Color)
 *
 * @param J VM state.
 */
static void PG_AddText(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    const char *txt = js_tostring(J, 1);
    float s = js_tonumber(J, 2);
    float x = js_tonumber(J, 3);
    float y = js_tonumber(J, 4);
    uint32_t c = js_touint32(J, 5);
    if (pdf_add_text(pg->pdf, NULL, txt, s, x, y, PDF_CONVERT_COLOR(c)) < 0) {
        js_error(J, "Could not add text: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief add wrapped text.
 * pg.AddTextWrap(txt:string, size:number, x:number, y:number, col:Color, wwidth: number, align:number):number
 *
 * @param J VM state.
 */
static void PG_AddTextWrap(js_State *J) {
    float height = 0;
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    const char *txt = js_tostring(J, 1);
    float s = js_tonumber(J, 2);
    float x = js_tonumber(J, 3);
    float y = js_tonumber(J, 4);
    uint32_t c = js_touint32(J, 5);
    float ww = js_tonumber(J, 6);
    uint32_t align = js_touint32(J, 7);
    if (pdf_add_text_wrap(pg->pdf, NULL, txt, s, x, y, PDF_CONVERT_COLOR(c), ww, align, &height) < 0) {
        js_error(J, "Could not add text: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
    js_pushnumber(J, height);
}

/**
 * @brief draw a filled rectangle.
 * pg.AddImageFile(x:number, y:number, w:number, h:number, fname:string)
 *
 * @param J VM state.
 */
static void PG_AddImageFile(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    float x = js_tonumber(J, 1);
    float y = js_tonumber(J, 2);
    float w = js_tonumber(J, 3);
    float h = js_tonumber(J, 4);
    const char *fname = js_tostring(J, 5);
    if (pdf_add_image_file(pg->pdf, NULL, x, y, w, h, fname) < 0) {
        js_error(J, "Could not add image '%s': %s", fname, pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief set page size.
 * pg.PageSetSize(w:number, h:number)
 *
 * @param J VM state.
 */
static void PG_PageSetSize(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    float w = js_tonumber(J, 1);
    float h = js_tonumber(J, 2);
    if (pdf_page_set_size(pg->pdf, NULL, w, h) < 0) {
        js_error(J, "Could not set page size: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief get width for string.
 * pg.GetFontTextWidth(font:string, txt:string, size:number):number
 *
 * @param J VM state.
 */
static void PG_GetFontTextWidth(js_State *J) {
    float width = 0;
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    const char *font = js_tostring(J, 1);
    const char *txt = js_tostring(J, 2);
    float s = js_tonumber(J, 3);
    if (pdf_get_font_text_width(pg->pdf, font, txt, s, &width) < 0) {
        js_error(J, "Could not get text width: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
    js_pushnumber(J, width);
}

/**
 * @brief add bookmark at current position.
 * pg.AddBookmark(parent:number, txt:string):number
 *
 * @param J VM state.
 */
static void PG_AddBookmark(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    uint32_t id = js_touint32(J, 1);
    const char *txt = js_tostring(J, 2);
    int newId = pdf_add_bookmark(pg->pdf, NULL, id, txt);
    if (newId < 0) {
        js_error(J, "Could not add bookmark: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
    js_pushnumber(J, newId);
}

/**
 * @brief add bezier line.
 * pg.AddQuadraticBezier(x1:number, y1:number, x2:number, y2:number, xq:number, yq:number, width:number, col:Color)
 *
 * @param J VM state.
 */
static void PG_AddQuadraticBezier(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    float x1 = js_tonumber(J, 1);
    float y1 = js_tonumber(J, 2);
    float x2 = js_tonumber(J, 3);
    float y2 = js_tonumber(J, 4);
    float xq = js_tonumber(J, 5);
    float yq = js_tonumber(J, 6);
    float bw = js_tonumber(J, 7);
    uint32_t c = js_touint32(J, 8);
    if (pdf_add_quadratic_bezier(pg->pdf, NULL, x1, y1, x2, y2, xq, yq, bw, PDF_CONVERT_COLOR(c)) < 0) {
        js_error(J, "Could not add quadratic bezier: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief add bezier line.
 * pg.AddCubicBezier(x1:number, y1:number, x2:number, y2:number, xq1:number, yq1:number, xq2:number, yq2:number, width:number, col:Color)
 *
 * @param J VM state.
 */
static void PG_AddCubicBezier(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    float x1 = js_tonumber(J, 1);
    float y1 = js_tonumber(J, 2);
    float x2 = js_tonumber(J, 3);
    float y2 = js_tonumber(J, 4);
    float xq1 = js_tonumber(J, 5);
    float yq1 = js_tonumber(J, 6);
    float xq2 = js_tonumber(J, 7);
    float yq2 = js_tonumber(J, 8);
    float bw = js_tonumber(J, 9);
    uint32_t c = js_touint32(J, 10);
    if (pdf_add_cubic_bezier(pg->pdf, NULL, x1, y1, x2, y2, xq1, yq1, xq2, yq2, bw, PDF_CONVERT_COLOR(c)) < 0) {
        js_error(J, "Could not add cubic bezier: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief add barcode.
 * pg.AddBarcode(type:number, x:number, y:number, w:number, h:number, txt:string, col:Color)
 *
 * @param J VM state.
 */
static void PG_AddBarcode(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);
    uint32_t code = js_touint32(J, 1);
    float x = js_tonumber(J, 2);
    float y = js_tonumber(J, 3);
    float w = js_tonumber(J, 4);
    float h = js_tonumber(J, 5);
    const char *txt = js_tostring(J, 6);
    uint32_t c = js_touint32(J, 7);
    if (pdf_add_barcode(pg->pdf, NULL, code, x, y, w, h, txt, PDF_CONVERT_COLOR(c)) < 0) {
        js_error(J, "Could not add barcode: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
        return;
    }
}

/**
 * @brief add a Bitmap() to the PDF.
 * pg.AddRgb(x:number, y:number, w:number, h:number, bm:Bitmap)
 *
 * @param J VM state.
 */
static void PG_AddRgb(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);

    float x = js_tonumber(J, 1);
    float y = js_tonumber(J, 2);
    float w = js_tonumber(J, 3);
    float h = js_tonumber(J, 4);

    if (js_isuserdata(J, 5, TAG_BITMAP)) {
        BITMAP *bm = js_touserdata(J, 5, TAG_BITMAP);

        // allocate memory
        size_t imageSize = bm->w * bm->h * 3;
        uint8_t *data = malloc(imageSize);
        if (!data) {
            JS_ENOMEM(J);
            return;
        }

        // convert bitmap into new memory
        uint8_t *dst = data;
        for (int y = 0; y < bm->h; y++) {
            for (int x = 0; x < bm->w; x++) {
                uint32_t argb = getpixel(bm, x, y);
                *dst = getr(argb);
                dst++;
                *dst = getg(argb);
                dst++;
                *dst = getb(argb);
                dst++;
            }
        }
        if (pdf_add_rgb24(pg->pdf, NULL, x, y, w, h, data, bm->w, bm->h) < 0) {
            js_error(J, "Could not add Bitmap: %s", pdf_get_err(pg->pdf, NULL));
            pdf_clear_err(pg->pdf);
        }

        free(data);
    } else {
        js_error(J, "Bitmap expected.");
        return;
    }
}

/**
 * @brief free a polygon array.
 *
 * @param array the array to free.
 */
static void pg_freeArray(poly_array_t *array) {
    if (array) {
        free(array->x);
        free(array->y);
        free(array);
    }
}

/**
 * @brief allocate a polygon array for the given number of points.
 *
 * @param len the number of points.
 *
 * @return poly_array_t* or NULL if out of memory.
 */
static poly_array_t *pg_allocArray(int len) {
    poly_array_t *array = calloc(1, sizeof(poly_array_t));
    if (!array) {
        return NULL;
    }

    array->len = len;
    array->x = malloc(sizeof(float) * len);
    if (!array->x) {
        pg_freeArray(array);
        return NULL;
    }

    array->y = malloc(sizeof(float) * len);
    if (!array->y) {
        pg_freeArray(array);
        return NULL;
    }
    return array;
}

/**
 * @brief convert JS-array to C arrays for polygon functions.
 *
 * @param J the JS context.
 * @param idx index of th JS-array on the stack.
 *
 * @return poly_array_t* or NULL if out of memory.
 */
static poly_array_t *pg_convertArray(js_State *J, int idx) {
    if (!js_isarray(J, idx)) {
        JS_ENOARR(J);
        return NULL;
    } else {
        int len = js_getlength(J, idx);
        poly_array_t *array = pg_allocArray(len);
        if (!array) {
            JS_ENOMEM(J);
            return NULL;
        }
        for (int i = 0; i < len; i++) {
            js_getindex(J, idx, i);
            {
                int pointlen = js_getlength(J, -1);
                if (pointlen < 2) {
                    js_error(J, "Points must have two values");
                    pg_freeArray(array);
                    return NULL;
                }

                js_getindex(J, -1, 0);
                int x = js_toint16(J, -1);
                js_pop(J, 1);

                js_getindex(J, -1, 1);
                int y = js_toint16(J, -1);
                js_pop(J, 1);

                array->x[i] = x;
                array->y[i] = y;
            }
            js_pop(J, 1);
        }
        return array;
    }
}

/**
 * @brief add a polygon.
 * pg.AddPolygon(points:number[][2], width:number, col:Color)
 *
 * @param J VM state.
 */
static void PG_AddPolygon(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);

    poly_array_t *array = pg_convertArray(J, 1);
    if (!array) {
        return;
    }

    float bw = js_tonumber(J, 2);
    uint32_t c = js_touint32(J, 3);
    if (pdf_add_filled_polygon(pg->pdf, NULL, array->x, array->y, array->len, bw, PDF_CONVERT_COLOR(c)) < 0) {
        js_error(J, "Could not add polygon: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
    }
    pg_freeArray(array);
}

/**
 * @brief add a filled polygon.
 * pg.AddFilledPolygon(points:number[][2], width:number, col:Color)
 *
 * @param J VM state.
 */
static void PG_AddFilledPolygon(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);

    poly_array_t *array = pg_convertArray(J, 1);
    if (!array) {
        return;
    }

    float bw = js_tonumber(J, 2);
    uint32_t c = js_touint32(J, 3);
    if (pdf_add_polygon(pg->pdf, NULL, array->x, array->y, array->len, bw, PDF_CONVERT_COLOR(c)) < 0) {
        js_error(J, "Could not add polygon: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
    }
    pg_freeArray(array);
}

/**
 * @brief add a filled polygon.
 * pg.AddFilledPolygon(points:number[][2], width:number, col:Color, fill_col:Color)
 *
 * @param J VM state.
 */
static void PG_AddCustomPath(js_State *J) {
    pdfgen_t *pg = js_touserdata(J, 0, TAG_PDFGEN);

    struct pdf_path_operation *ops = NULL;
    int len = 0;
    if (!js_isarray(J, 1)) {
        JS_ENOARR(J);
        return;
    } else {
        len = js_getlength(J, 1);
        ops = calloc(len, sizeof(struct pdf_path_operation));
        if (!ops) {
            JS_ENOMEM(J);
            return;
        }
        for (int i = 0; i < len; i++) {
            js_getindex(J, 1, i);
            {
                int oplen = js_getlength(J, -1);
                if (oplen < 7) {
                    js_error(J, "Path ops must have 7 values");
                    free(ops);
                    return;
                }

                js_getindex(J, -1, 0);
                const char *str = js_tostring(J, -1);
                if (str[0]) {
                    ops[i].op = str[0];
                } else {
                    js_error(J, "Empty op!");
                    free(ops);
                    return;
                }
                js_pop(J, 1);

                js_getindex(J, -1, 1);
                ops[i].x1 = js_tonumber(J, -1);
                js_pop(J, 1);

                js_getindex(J, -1, 2);
                ops[i].y1 = js_tonumber(J, -1);
                js_pop(J, 1);

                js_getindex(J, -1, 3);
                ops[i].x2 = js_tonumber(J, -1);
                js_pop(J, 1);

                js_getindex(J, -1, 4);
                ops[i].y2 = js_tonumber(J, -1);
                js_pop(J, 1);

                js_getindex(J, -1, 5);
                ops[i].x3 = js_tonumber(J, -1);
                js_pop(J, 1);

                js_getindex(J, -1, 6);
                ops[i].y3 = js_tonumber(J, -1);
                js_pop(J, 1);
            }
            js_pop(J, 1);
        }
    }

    float bw = js_tonumber(J, 2);
    uint32_t c = js_touint32(J, 3);
    uint32_t fc = js_touint32(J, 4);

    if (pdf_add_custom_path(pg->pdf, NULL, ops, len, bw, PDF_CONVERT_COLOR(c), PDF_CONVERT_COLOR(fc)) < 0) {
        js_error(J, "Could not add path: %s", pdf_get_err(pg->pdf, NULL));
        pdf_clear_err(pg->pdf);
    }
    free(ops);
}

/*********************
** public functions **
*********************/
/**
 * @brief initialize neural subsystem.
 *
 * @param J VM state.
 */
void init_pdfgen(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    // constants for page sizes
    PDF_PUSH_VALUE(PDF_A3_WIDTH);
    PDF_PUSH_VALUE(PDF_A3_HEIGHT);
    PDF_PUSH_VALUE(PDF_A4_WIDTH);
    PDF_PUSH_VALUE(PDF_A4_HEIGHT);
    PDF_PUSH_VALUE(PDF_LETTER_WIDTH);
    PDF_PUSH_VALUE(PDF_LETTER_HEIGHT);

    // alignment constants
    PDF_PUSH_VALUE(PDF_ALIGN_LEFT);
    PDF_PUSH_VALUE(PDF_ALIGN_RIGHT);
    PDF_PUSH_VALUE(PDF_ALIGN_CENTER);
    PDF_PUSH_VALUE(PDF_ALIGN_JUSTIFY);
    PDF_PUSH_VALUE(PDF_ALIGN_JUSTIFY_ALL);
    PDF_PUSH_VALUE(PDF_ALIGN_NO_WRITE);

    // barcode constants
    PDF_PUSH_VALUE(PDF_BARCODE_128A);
    PDF_PUSH_VALUE(PDF_BARCODE_39);

    PROPDEF_N(J, -1, "PDF_TOPLEVEL_BOOKMARK");

    // constants for fonts
    PROPDEF_S(J, "Courier", "PDF_COURIER");
    PROPDEF_S(J, "Courier-Bold", "PDF_COURIER_BOLD");
    PROPDEF_S(J, "Courier-BoldOblique", "PDF_COURIER_BOLD_OBLIQUE");
    PROPDEF_S(J, "Courier-Oblique", "PDF_COURIER_OBLIQUE");
    PROPDEF_S(J, "Helvetica", "PDF_HELVETICA");
    PROPDEF_S(J, "Helvetica-Bold", "PDF_HELVETICA_BOLD");
    PROPDEF_S(J, "Helvetica-BoldOblique", "PDF_HELVETICA_BOLD_OBLIQUE");
    PROPDEF_S(J, "Helvetica-Oblique", "PDF_HELVETICA_OBLIQUE");
    PROPDEF_S(J, "Times-Roman", "PDF_TIMES");
    PROPDEF_S(J, "Times-Bold", "PDF_TIMES_BOLD");
    PROPDEF_S(J, "Times-Italic", "PDF_TIMES_BOLD_OBLIQUE");
    PROPDEF_S(J, "Times-BoldItalic", "PDF_TIMES_OBLIQUE");
    PROPDEF_S(J, "Symbol", "PDF_SYMBOL");
    PROPDEF_S(J, "ZapfDingbats", "PDF_ZAPF_DINGBATS");

    js_newobject(J);
    {
        NPROTDEF(J, PG, Save, 1);
        NPROTDEF(J, PG, SetFont, 1);
        NPROTDEF(J, PG, PageSetSize, 2);
        NPROTDEF(J, PG, AppendPage, 0);
        NPROTDEF(J, PG, AddLine, 6);
        NPROTDEF(J, PG, AddCircle, 6);
        NPROTDEF(J, PG, AddEllipse, 7);
        NPROTDEF(J, PG, AddRectangle, 6);
        NPROTDEF(J, PG, AddFilledRectangle, 6);
        NPROTDEF(J, PG, AddText, 5);
        NPROTDEF(J, PG, AddImageFile, 5);
        NPROTDEF(J, PG, AddTextWrap, 7);
        NPROTDEF(J, PG, GetFontTextWidth, 3);
        NPROTDEF(J, PG, AddBookmark, 2);
        NPROTDEF(J, PG, AddCubicBezier, 10);
        NPROTDEF(J, PG, AddQuadraticBezier, 8);
        NPROTDEF(J, PG, AddBarcode, 7);
        NPROTDEF(J, PG, AddRgb, 5);
        NPROTDEF(J, PG, AddPolygon, 3);
        NPROTDEF(J, PG, AddFilledPolygon, 3);
        NPROTDEF(J, PG, AddCustomPath, 4);
    }
    CTORDEF(J, new_Pdfgen, TAG_PDFGEN, 2);
}
