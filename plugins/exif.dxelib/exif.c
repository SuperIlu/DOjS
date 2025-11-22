/*
MIT License

Copyright (c) 2019-2025 Andre Seidelt <superilu@yahoo.com>

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
#include <libexif/exif-data.h>
#include <mujs.h>

#include "DOjS.h"
#include "bitmap.h"

void init_exif(js_State *J);

/************
** defines **
************/
#define TAG_EXIF "Exif"     //!< pointer tag
#define TAG_VALUE_BUF 1024  //!< tag value buffer size

/************
** structs **
************/
//! file userdata definition
typedef struct __exif {
    ExifData *ed;  //!< exif pointer
} exif_t;

/**
 * @brief free ressources.
 *
 * @param ov the exif_t with the ressources to free.
 */
static void EXIF_cleanup(exif_t *ex) {
    if (ex->ed) {
        exif_data_unref(ex->ed);
        ex->ed = NULL;
    }
}

/**
 * @brief finalize a file and free resources.
 *
 * @param J VM state.
 */
static void EXIF_Finalize(js_State *J, void *data) {
    exif_t *ex = (exif_t *)data;
    EXIF_cleanup(ex);
    free(ex);
}

/**
 * @brief open a Exif.
 * ex = new Exif(filename:str)
 *
 * @param J VM state.
 */
static void new_Exif(js_State *J) {
    NEW_OBJECT_PREP(J);

    exif_t *ex = calloc(1, sizeof(exif_t));
    if (!ex) {
        JS_ENOMEM(J);
        return;
    }
    memset(ex, 0, sizeof(exif_t));

    const char *fname = js_tostring(J, 1);
    ex->ed = exif_data_new_from_file(fname);
    if (!ex->ed) {
        free(ex);
        js_error(J, "Could not open '%s'", fname);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_EXIF, ex, EXIF_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief close EXIF.
 * ex.Close()
 *
 * @param J VM state.
 */
static void EXIF_Close(js_State *J) {
    exif_t *ex = js_touserdata(J, 0, TAG_EXIF);
    EXIF_cleanup(ex);
}

/**
 * callback for exif_data_foreach_entry()
 */
static void EXIF_for_entry(ExifEntry *e, void *data) {
    char v[TAG_VALUE_BUF];
    js_State *J = data;

    ExifIfd ifd = exif_entry_get_ifd(e);
    const char *name = exif_tag_get_name_in_ifd(e->tag, ifd);
    const char *value = exif_entry_get_value(e, v, sizeof(v));

    js_pushstring(J, value);
    js_setproperty(J, -2, name);
}

/**
 * callback for exif_data_foreach_content()
 */
static void EXIF_for_content(ExifContent *content, void *data) { exif_content_foreach_entry(content, EXIF_for_entry, data); }

/**
 * @brief get all EXIF tags as key->value object.
 * ex.GetTags()
 *
 * @param J VM state.
 */
static void EXIF_GetTags(js_State *J) {
    exif_t *ex = js_touserdata(J, 0, TAG_EXIF);
    if (!ex->ed) {
        js_error(J, "EXIF is closed");
        return;
    }

    js_newobject(J);
    exif_data_foreach_content(ex->ed, EXIF_for_content, J);
}

/*********************
** public functions **
*********************/
/**
 * @brief initialize subsystem.
 *
 * @param J VM state.
 */
void init_exif(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, EXIF, Close, 0);
        NPROTDEF(J, EXIF, GetTags, 0);
    }
    CTORDEF(J, new_Exif, TAG_EXIF, 1);
}
