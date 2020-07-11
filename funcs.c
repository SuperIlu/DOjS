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

#include "funcs.h"

#include <allegro.h>
#include <dirent.h>
#include <dpmi.h>
#include <errno.h>
#include <go32.h>
#include <mujs.h>
#include <pc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/farptr.h>
#include <sys/stat.h>
#include <time.h>

#include "DOjS.h"
#include "color.h"
#include "util.h"

/*********************
** static functions **
*********************/
/**
 * @brief read a whole file as string.
 * Read(fname:string):string
 *
 * @param J the JS context.
 */
static void f_Read(js_State *J) {
    const char *fname = js_tostring(J, 1);
    FILE *f;
    char *s;
    int n, t;

    f = fopen(fname, "rb");
    if (!f) {
        js_error(J, "Can't open file '%s': %s", fname, strerror(errno));
        return;
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fclose(f);
        js_error(J, "Can't seek in file '%s': %s", fname, strerror(errno));
        return;
    }

    n = ftell(f);
    if (n < 0) {
        fclose(f);
        js_error(J, "Can't tell in file '%s': %s", fname, strerror(errno));
        return;
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        fclose(f);
        js_error(J, "Can't seek in file '%s': %s", fname, strerror(errno));
        return;
    }

    s = malloc(n + 1);
    if (!s) {
        fclose(f);
        JS_ENOMEM(J);
        return;
    }

    t = fread(s, 1, n, f);
    if (t != n) {
        free(s);
        fclose(f);
        js_error(J, "Can't read data from file '%s': %s", fname, strerror(errno));
        return;
    }
    s[n] = 0;

    js_pushstring(J, s);
    free(s);
    fclose(f);
}

/**
 * @brief get directory listing.
 * List(dname:string):[f1:string, f1:string, ...]
 *
 * @param J the JS context.
 */
static void f_List(js_State *J) {
    const char *dirname = js_tostring(J, 1);
    DIR *dir = opendir(dirname);
    if (!dir) {
        js_error(J, "Cannot open dir '%s': %s", dirname, strerror(errno));
        return;
    }

    struct dirent *de;
    js_newarray(J);
    int idx = 0;
    while ((de = readdir(dir)) != NULL) {
        js_pushstring(J, de->d_name);
        js_setindex(J, -2, idx);
        idx++;
    }

    closedir(dir);
}

/**
 * @brief convert time_t into something like a javascript time string.
 *
 * @param t the time_t
 *
 * @return char* pointer to a static buffer with the conversion result.
 */
static char *f_formatTime(time_t *t) {
    static char buf[100];
    struct tm *tm = localtime(t);
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S.000", tm);
    return buf;
}

/**
 * @brief file/directory info.
 * Stat(name:string):{}
 *
 * @param J the JS context.
 */
static void f_Stat(js_State *J) {
    const char *name = js_tostring(J, 1);

    struct stat s;
    if (stat(name, &s) != 0) {
        js_error(J, "Cannot stat '%s': %s", name, strerror(errno));
        return;
    }

    char *drive = "A:";
    drive[0] += s.st_dev;

    js_newobject(J);
    {
        js_pushstring(J, drive);
        js_setproperty(J, -2, "drive");
        js_pushstring(J, f_formatTime(&s.st_atime));
        js_setproperty(J, -2, "atime");
        js_pushstring(J, f_formatTime(&s.st_ctime));
        js_setproperty(J, -2, "ctime");
        js_pushstring(J, f_formatTime(&s.st_mtime));
        js_setproperty(J, -2, "mtime");
        js_pushnumber(J, s.st_size);
        js_setproperty(J, -2, "size");
        js_pushnumber(J, s.st_blksize);
        js_setproperty(J, -2, "blksize");
        js_pushnumber(J, s.st_nlink);
        js_setproperty(J, -2, "nlink");

        js_pushboolean(J, S_ISREG(s.st_mode));
        js_setproperty(J, -2, "is_regular");
        js_pushboolean(J, S_ISDIR(s.st_mode));
        js_setproperty(J, -2, "is_directory");
        js_pushboolean(J, S_ISCHR(s.st_mode));
        js_setproperty(J, -2, "is_chardev");
        js_pushboolean(J, S_ISBLK(s.st_mode));
        js_setproperty(J, -2, "is_blockdev");
    }
}

/**
 * @brief print to stdout with newline.
 * Println(t1, t2, ...)
 *
 * @param J the JS context.
 */
static void f_Println(js_State *J) {
    int i, top = js_gettop(J);
    for (i = 1; i < top; ++i) {
        const char *s = js_tostring(J, i);
        if (i > 1) {
            putc(' ', LOGSTREAM);
        }
        fputs(s, LOGSTREAM);
    }
    putc('\n', LOGSTREAM);
    js_pushundefined(J);
}

/**
 * @brief print to stdout.
 * Print(t1, t2, ...)
 *
 * @param J the JS context.
 */
static void f_Print(js_State *J) {
    int i, top = js_gettop(J);
    for (i = 1; i < top; ++i) {
        const char *s = js_tostring(J, i);
        if (i > 1) {
            putc(' ', LOGSTREAM);
        }
        fputs(s, LOGSTREAM);
    }
    js_pushundefined(J);
}

/**
 * @brief stop DOjS loop().
 * Quit()
 *
 * @param J the JS context.
 */
static void f_Stop(js_State *J) { DOjS.keep_running = false; }

/**
 * @brief do garbage collection.
 * Gc(report:boolean)
 *
 * @param J the JS context.
 */
static void f_Gc(js_State *J) {
    bool report = js_toboolean(J, 1);
    js_gc(J, report);
    DOjS.lastError = NULL;
}

/**
 * @brief get memory info
 * MemoryInfo():{"used":XXX, "available":XXX}
 *
 * @param J the JS context.
 */
static void f_MemoryInfo(js_State *J) {
    _go32_dpmi_meminfo info;

    js_newobject(J);
    {
        if ((_go32_dpmi_get_free_memory_information(&info) == 0) && (info.total_physical_pages != -1)) {
            js_pushnumber(J, info.total_physical_pages * 4096);
            js_setproperty(J, -2, "total");
            js_pushnumber(J, _go32_dpmi_remaining_physical_memory());
            js_setproperty(J, -2, "remaining");
        }
    }
}

/**
 * @brief sleep for the given number of ms.
 * Sleep(ms:number)
 *
 * @param J the JS context.
 */
static void f_Sleep(js_State *J) { rest(js_toint32(J, 1)); }

/**
 * @brief get current time in ms.
 * MsecTime():number
 *
 * @param J the JS context.
 */
static void f_MsecTime(js_State *J) { js_pushnumber(J, DOjS.sys_ticks); }

/**
 * @brief get current framerate.
 * GetFramerate():number
 *
 * @param J the JS context.
 */
static void f_GetFramerate(js_State *J) { js_pushnumber(J, DOjS.current_frame_rate); }

/**
 * @brief set wanted framerate.
 * SetFramerate(wanted:number)
 *
 * @param J the JS context.
 */
static void f_SetFramerate(js_State *J) { DOjS.wanted_frame_rate = (float)js_tonumber(J, 1); }

/**
 * @brief set mouse acceleration
 * MouseSetSpeed(x:number, y:number)
 *
 * @param J the JS context.
 */
static void f_MouseSetSpeed(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    set_mouse_speed(x, y);
}

/**
 * @brief set mouse limits
 * MouseSetLimits(x1:number, y1:number, x2:number, y2:number)
 *
 * @param J the JS context.
 */
static void f_MouseSetLimits(js_State *J) {
    int x1 = js_toint16(J, 1);
    int y1 = js_toint16(J, 2);
    int x2 = js_toint16(J, 3);
    int y2 = js_toint16(J, 4);

    set_mouse_range(x1, y1, x2, y2);
}

/**
 * @brief mouse mouse cursor
 * MouseWarp(x:number, y:number)
 *
 * @param J the JS context.
 */
static void f_MouseWarp(js_State *J) {
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    position_mouse(x, y);
}

/**
 * @brief show hide mouse cursor
 * MouseShowCursor(b:boolean)
 *
 * @param J the JS context.
 */
static void f_MouseShowCursor(js_State *J) { DOjS.mouse_visible = js_toboolean(J, 1); }

/**
 * @brief change mode of the cursor.
 * MouseSetCursorMode(MOUSE.Mode.NONE) or
 * MouseSetCursorMode(MOUSE.Mode.ARROW) or
 * MouseSetCursorMode(MOUSE.Mode.BUSY) or
 * MouseSetCursorMode(MOUSE.Mode.QUESTION) or
 * MouseSetCursorMode(MOUSE.Mode.EDIT)
 *
 * @param J the JS context.
 */
static void f_MouseSetCursorMode(js_State *J) {
    int mode = js_toint16(J, 1);
    select_mouse_cursor(mode);
}

/**
 * @brief change the exit-key for the script. Default: ESCAPE.
 * SetExitKey(key:number)
 *
 * @param J the JS context.
 */
static void f_SetExitKey(js_State *J) { DOjS.exit_key = js_toint32(J, 1); }

/**
 * @brief call external program.
 * System(cmd:string):number
 *
 * @param J the JS context.
 */
static void f_System(js_State *J) {
    const char *cmd = js_tostring(J, 1);
    int flags = js_touint16(J, 2);

    if (flags & SYS_FLAG_MOUSE) {
        remove_mouse();
    }

    if (flags & SYS_FLAG_SOUND) {
        remove_sound_input();
        remove_sound();
    }

    if (flags & SYS_FLAG_JOYSTICK) {
        install_joystick(JOY_TYPE_AUTODETECT);
    }

    if (flags & SYS_FLAG_KEYBOARD) {
        remove_keyboard();
    }

    if (flags & SYS_FLAG_TIMER) {
        remove_timer();
    }

    int ret = system(cmd);

    if (flags & SYS_FLAG_TIMER) {
        install_timer();
    }

    if (flags & SYS_FLAG_KEYBOARD) {
        install_keyboard();
    }

    if (flags & SYS_FLAG_JOYSTICK) {
        install_joystick(JOY_TYPE_AUTODETECT);
    }

    if (flags & SYS_FLAG_SOUND) {
        install_sound(DOjS.params.no_sound ? DIGI_NONE : DIGI_AUTODETECT, DOjS.params.no_fm ? MIDI_NONE : MIDI_AUTODETECT, NULL);
        install_sound_input(DOjS.params.no_sound ? DIGI_NONE : DIGI_AUTODETECT, DOjS.params.no_fm ? MIDI_NONE : MIDI_AUTODETECT);
    }

    if (flags & SYS_FLAG_MOUSE) {
        install_mouse();
    }

    js_pushnumber(J, ret);
}

static void f_OutPortByte(js_State *J) { outportb(js_toint16(J, 1), js_toint16(J, 2) & 0xFF); }
static void f_OutPortWord(js_State *J) { outportw(js_toint16(J, 1), js_toint16(J, 2)); }
static void f_OutPortLong(js_State *J) { outportl(js_toint16(J, 1), js_toint32(J, 2)); }

static void f_InPortByte(js_State *J) { js_pushnumber(J, inportb(js_toint16(J, 1)) & 0xFF); }
static void f_InPortWord(js_State *J) { js_pushnumber(J, inportw(js_toint16(J, 1)) & 0xFFFF); }
static void f_InPortLong(js_State *J) { js_pushnumber(J, inportl(js_toint16(J, 1))); }

/**
 * @brief get a list of parallel port addresses from BIOS
 *
 * @param J VM state.
 */
static void f_GetParallelPorts(js_State *J) {
    unsigned long ptraddr = 0x0408;  // Base Address: segment is zero
    int idx = 0;

    js_newarray(J);
    for (int j = 0; j < 3; j++) {
        unsigned int port = _farpeekw(_dos_ds, ptraddr);
        ptraddr += 2;
        if (port) {
            js_pushnumber(J, port);
            js_setindex(J, -2, idx);
            idx++;
        }
    }
}

/**
 * @brief get a list of serial port addresses from BIOS
 *
 * @param J VM state.
 */
static void f_GetSerialPorts(js_State *J) {
    unsigned long ptraddr = 0x0400;  // Base Address: segment is zero
    int idx = 0;

    js_newarray(J);
    for (int j = 0; j < 3; j++) {
        unsigned int port = _farpeekw(_dos_ds, ptraddr);
        ptraddr += 2;
        if (port) {
            js_pushnumber(J, port);
            js_setindex(J, -2, idx);
            idx++;
        }
    }
}

/**
 * reset lpt port
 * @param J VM state.
 */
static void f_LPTReset(js_State *J) { biosprint(1, 0, js_toint16(J, 1)); }

/**
 * send data to lpt port
 * @param J VM state.
 */
static void f_LPTSend(js_State *J) {
    int port = js_toint16(J, 1);
    char *data = js_tostring(J, 2);

    while (*data) {
        biosprint(0, *data++, port);
    }
}

/**
 * get status of lpt port
 * @param J VM state.
 */
static void f_LPTStatus(js_State *J) { js_pushnumber(J, biosprint(2, 0, js_toint16(J, 1))); }

/***********************
** exported functions **
***********************/
/**
 * @brief initialize grx subsystem.
 *
 * @param J VM state.
 * @param argc number of parameters.
 * @param argv command line parameters.
 * @param args first script parameter.
 */
void init_funcs(js_State *J, int argc, char **argv, int args) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    // define some global properties
    js_pushglobal(J);
    js_setglobal(J, "global");

    PROPDEF_N(J, DOSJS_VERSION, "DOJS_VERSION");

    //
    js_newarray(J);
    int idx = 0;
    for (int i = args; i < argc; i++) {
        js_pushstring(J, argv[i]);
        js_setindex(J, -2, idx);
        idx++;
    }
    js_setglobal(J, "ARGS");

    // define global functions
    FUNCDEF(J, f_Read, "Read", 1);
    FUNCDEF(J, f_List, "List", 1);
    FUNCDEF(J, f_Stat, "Stat", 1);
    FUNCDEF(J, f_Print, "Print", 0);
    FUNCDEF(J, f_Println, "Println", 0);
    FUNCDEF(J, f_Stop, "Stop", 0);
    FUNCDEF(J, f_Gc, "Gc", 1);
    FUNCDEF(J, f_MemoryInfo, "MemoryInfo", 0);
    FUNCDEF(J, f_Sleep, "Sleep", 1);
    FUNCDEF(J, f_MsecTime, "MsecTime", 0);
    FUNCDEF(J, f_SetFramerate, "SetFramerate", 1);
    FUNCDEF(J, f_GetFramerate, "GetFramerate", 0);

    FUNCDEF(J, f_MouseSetSpeed, "MouseSetSpeed", 2);
    FUNCDEF(J, f_MouseSetLimits, "MouseSetLimits", 4);
    FUNCDEF(J, f_MouseWarp, "MouseWarp", 2);
    FUNCDEF(J, f_MouseShowCursor, "MouseShowCursor", 1);
    FUNCDEF(J, f_MouseSetCursorMode, "MouseSetCursorMode", 1);

    FUNCDEF(J, f_SetExitKey, "SetExitKey", 1);

    FUNCDEF(J, f_System, "System", 2);
    FUNCDEF(J, f_OutPortByte, "OutPortByte", 2);
    FUNCDEF(J, f_OutPortWord, "OutPortWord", 2);
    FUNCDEF(J, f_OutPortLong, "OutPortLong", 2);
    FUNCDEF(J, f_InPortByte, "InPortByte", 1);
    FUNCDEF(J, f_InPortWord, "InPortWord", 1);
    FUNCDEF(J, f_InPortLong, "InPortLong", 1);
    FUNCDEF(J, f_GetParallelPorts, "GetParallelPorts", 0);
    FUNCDEF(J, f_GetSerialPorts, "GetSerialPorts", 0);
    FUNCDEF(J, f_LPTReset, "_LPTReset", 1);
    FUNCDEF(J, f_LPTSend, "_LPTSend", 2);
    FUNCDEF(J, f_LPTStatus, "_LPTStatus", 1);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
