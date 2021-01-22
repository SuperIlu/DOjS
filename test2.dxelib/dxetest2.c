#include <mujs.h>
#include <stdio.h>
#include "DOjS.h"

void init_dxetest2(js_State *J);
void shutdown_dxetest2(void);

static void f_MakeString(js_State *J) { js_pushstring(J, "This is a test of the emergency broadcast system!"); }

void init_dxetest2(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);
    NFUNCDEF(J, MakeString, 0);
}

void shutdown_dxetest2() { LOGF("%s\n", __PRETTY_FUNCTION__); }
