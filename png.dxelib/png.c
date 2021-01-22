#include <mujs.h>
#include <stdio.h>
#include <allegro.h>
#include <alpng.h>

#include "DOjS.h"
#include "bitmap.h"

void init_png(js_State *J);

/**
 * @brief save current screen to file.
 * SavePngImage(fname:string)
 *
 * @param J the JS context.
 */
static void f_SavePngImage(js_State *J) {
    const char *fname = js_tostring(J, 1);

    PALETTE pal;
    get_palette(pal);

    if (save_png(fname, DOjS.current_bm, (const struct RGB *)&pal) != 0) {
        js_error(J, "Can't save screen to PNG file '%s': %s", fname, allegro_error);
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

void init_png(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    /* Make Allegro aware of PNG file format. */
    alpng_init();

    NFUNCDEF(J, SavePngImage, 1);

    js_getglobal(J, TAG_BITMAP);
    js_getproperty(J, -1, "prototype");
    js_newcfunction(J, Bitmap_SavePngImage, "Bitmap.prototype.SavePngImage", 1);
    js_defproperty(J, -2, "SavePngImage", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    NPROTDEF(J, Bitmap, SavePngImage, 1);
}
