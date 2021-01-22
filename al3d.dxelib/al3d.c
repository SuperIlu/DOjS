#include <mujs.h>
#include <stdio.h>

#include "DOjS.h"
#include "a3d.h"
#include "util.h"
#include "zipfile.h"
#include "zbuffer.h"

void init_al3d(js_State *J);

void init_al3d(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    if (ut_file_exists(JSBOOT_ZIP)) {
        dojs_do_file(J, JSBOOT_ZIP ZIP_DELIM_STR JSINC_A3D);
    } else {
        dojs_do_file(J, JSINC_A3D);
    }

    init_a3d(J);
    init_zbuffer(J);
}
