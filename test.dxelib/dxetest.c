#include <mujs.h>
#include <stdio.h>
#include "DOjS.h"

void init_dxetest(js_State *J);
void shutdown_dxetest(void);

static void f_HelloWorld(js_State *J) {
    char buff[1024];
    const char *para = js_tostring(J, 1);

    snprintf(buff, sizeof(buff) - 1, "Hello World! %s", para);

    js_pushstring(J, buff);
}

void init_dxetest(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);
    NFUNCDEF(J, HelloWorld, 1);
}

void shutdown_dxetest() { LOGF("%s\n", __PRETTY_FUNCTION__); }
