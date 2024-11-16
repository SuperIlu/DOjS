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

#include "funcs.h"

#include <dirent.h>
#include <mujs.h>
#include <stdio.h>
#include <sys/stat.h>
#if WINDOWS != 1
#include <dlfcn.h>
#endif

#include "util.h"
#include "socket.h"
#include "zipfile.h"
#include "jsi.h"
#include "jsparse.h"
#include "jscompile.h"

#if LINUX == 1
#if WINDOWS !=1
#include <sys/sysinfo.h>
#endif
#else
#include <sys/dxe.h>
#include <dos.h>
#endif

#define LL_BUFFER_SIZE 2014

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
    char *s;
    size_t n;

    if (ut_read_file(fname, (void **)&s, &n)) {
        js_pushstring(J, s);
        free(s);
    } else {
        js_error(J, "Can't read data from file '%s': %s", fname, strerror(errno));
        return;
    }
}

/**
 * @brief read a whole file as string.
 * Read(fname:string, entry_name:string):string
 *
 * @param J the JS context.
 */
static void f_ReadZIP(js_State *J) {
    const char *zname = js_tostring(J, 1);
    const char *ename = js_tostring(J, 2);

    char *buf = NULL;
    size_t esize;
    if (read_zipfile2(zname, ename, (void **)&buf, &esize)) {
        js_pushstring(J, buf);
        free(buf);
    } else {
        js_error(J, "Can't open ZIP '%s' = '%s'", zname, ename);
        return;
    }
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
 * @brief Check if a directory exists.
 * DirExists(dname:string):bool
 *
 * @param J the JS context.
 */
static void f_DirExists(js_State *J) {
    const char *dirname = js_tostring(J, 1);
    DIR *dir = opendir(dirname);
    if (!dir) {
        js_pushboolean(J, false);
    } else {
        closedir(dir);
        js_pushboolean(J, true);
    }
}

/**
 * @brief Check if a file exists.
 * FileExists(fname:string):bool
 *
 * @param J the JS context.
 */
static void f_FileExists(js_State *J) { js_pushboolean(J, ut_file_exists(js_tostring(J, 1))); }

/**
 * @brief RmFile(name:string)
 *
 * @param J the JS context.
 */
static void f_RmFile(js_State *J) {
    const char *file = js_tostring(J, 1);

    if (!file) {
        js_error(J, "No filename");
        return;
    }

    int ret = unlink(file);
    if (ret != 0) {
        js_error(J, "Could not delete file: %s", strerror(errno));
        return;
    }
}

/**
 * @brief RmDir(name:string)
 *
 * @param J the JS context.
 */
static void f_RmDir(js_State *J) {
    const char *dir = js_tostring(J, 1);

    if (!dir) {
        js_error(J, "No dirname");
        return;
    }

    int ret = rmdir(dir);
    if (ret != 0) {
        js_error(J, "Could not delete directory: %s", strerror(errno));
        return;
    }
}

/**
 * @brief MakeDir(name:string)
 *
 * @param J the JS context.
 */
static void f_MakeDir(js_State *J) {
    const char *dir = js_tostring(J, 1);

    if (!dir) {
        js_error(J, "No dirname");
        return;
    }

#if WINDOWS == 1
    int ret = _mkdir(dir);
#else
    int ret = mkdir(dir, 0);
#endif
    if (ret != 0) {
        js_error(J, "Could not create directory: %s", strerror(errno));
        return;
    }
}

/**
 * @brief Rename(from:string, to:string)
 *
 * @param J the JS context.
 */
static void f_Rename(js_State *J) {
    const char *from = js_tostring(J, 1);
    const char *to = js_tostring(J, 2);

    if (!from) {
        js_error(J, "No source filename");
        return;
    }

    if (!to) {
        js_error(J, "No destination filename");
        return;
    }

    int ret = rename(from, to);
    if (ret != 0) {
        js_error(J, "Could not rename file: %s", strerror(errno));
        return;
    }
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

#if LINUX == 1
    char *drive = "/";
#else
    char *drive = "A:";
    drive[0] += s.st_dev;
#endif

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
#if WINDOWS == 1
        js_pushnumber(J, 0);
#else
        js_pushnumber(J, s.st_blksize);
#endif
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
    if (LOGSTREAM) {
        int i, top = js_gettop(J);
        for (i = 1; i < top; ++i) {
            const char *s = js_tostring(J, i);
            if (i > 1) {
                putc(' ', LOGSTREAM);
            }
            fputs(s, LOGSTREAM);
        }
        putc('\n', LOGSTREAM);
    }
#if LINUX == 1
    int i, top = js_gettop(J);
    for (i = 1; i < top; ++i) {
        const char *s = js_tostring(J, i);
        if (i > 1) {
            putchar(' ');
        }
        puts(s);
    }
#endif
    js_pushundefined(J);
}

/**
 * @brief print to stdout.
 * Print(t1, t2, ...)
 *
 * @param J the JS context.
 */
static void f_Print(js_State *J) {
    if (LOGSTREAM) {
        int i, top = js_gettop(J);
        for (i = 1; i < top; ++i) {
            const char *s = js_tostring(J, i);
            if (i > 1) {
                putc(' ', LOGSTREAM);
            }
            fputs(s, LOGSTREAM);
        }
    }
#if LINUX == 1
    int i, top = js_gettop(J);
    for (i = 1; i < top; ++i) {
        const char *s = js_tostring(J, i);
        if (i > 1) {
            putchar(' ');
        }
        puts(s);
    }
#endif
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
}

/**
 * @brief get memory info
 * MemoryInfo():{"used":XXX, "available":XXX}
 *
 * @param J the JS context.
 */
static void f_MemoryInfo(js_State *J) {
#if WINDOWS == 1
    MEMORYSTATUS memstat;

    js_newobject(J);
    {
        GlobalMemoryStatus(&memstat);
        js_pushnumber(J, memstat.dwTotalPhys);
        js_setproperty(J, -2, "total");
        js_pushnumber(J, memstat.dwAvailPhys);
        js_setproperty(J, -2, "remaining");
    }
#elif LINUX == 1
    struct sysinfo info;

    js_newobject(J);
    {
        if (sysinfo(&info) == 0) {
            js_pushnumber(J, info.totalram);
            js_setproperty(J, -2, "total");
            js_pushnumber(J, info.freeram);
            js_setproperty(J, -2, "remaining");
        }
    }
#else
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
#endif
}

/**
 * @brief sleep for the given number of ms.
 * Sleep(ms:number)
 *
 * @param J the JS context.
 */
static void f_Sleep(js_State *J) { rest_callback(js_toint32(J, 1), tick_socket); }

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

/**
 * convert byte array to string.
 * BytesToString(data:number[]):string
 *
 * @param J VM state.
 */
static void f_BytesToString(js_State *J) {
    if (js_isarray(J, 1)) {
        int len = js_getlength(J, 1);

        char *data = malloc(len + 1);
        if (!data) {
            JS_ENOMEM(J);
            return;
        }

        for (int i = 0; i < len; i++) {
            js_getindex(J, 1, i);
            data[i] = (char)js_toint16(J, -1);
            js_pop(J, 1);
        }
        data[len] = 0;
        js_pushstring(J, data);

        free(data);
    } else {
        JS_ENOARR(J);
    }
}

/**
 * convert string to byte array.
 * StringToBytes(string):number[]
 *
 * @param J VM state.
 */
static void f_StringToBytes(js_State *J) {
    const char *data = js_tostring(J, 1);
    js_newarray(J);

    int idx = 0;
    while (data[idx]) {
        js_pushnumber(J, data[idx]);
        js_setindex(J, -2, idx);
        idx++;
    }
}

/**
 * @brief parse string and run it as function.
 * This works like Function(), but it only takes one parameter and the source of the parsed string can be provided.
 * NamedFunction(func_param, func_src, func_source_filename):function
 *
 * @param J
 */
static void f_NamedFunction(js_State *J) {
    js_Buffer *sb = NULL;
    const char *body, *fname;
    js_Ast *parse;
    js_Function *fun;

    if (js_try(J)) {
        js_free(J, sb);
        jsP_freeparse(J);
        js_throw(J);
    }

    js_puts(J, &sb, js_tostring(J, 1));
    js_putc(J, &sb, ')');
    js_putc(J, &sb, 0);

    /* body */
    body = js_isdefined(J, 2) ? js_tostring(J, 2) : "";
    fname = js_isdefined(J, 3) ? js_tostring(J, 3) : "[string]";

    parse = jsP_parsefunction(J, fname, sb ? sb->s : NULL, body);
    fun = jsC_compilefunction(J, parse);

    js_endtry(J);
    js_free(J, sb);
    jsP_freeparse(J);

    js_newfunction(J, fun, J->GE);
}

#if LINUX != 1
/**
 * @brief callback function when a symbol can't be found during library loading.
 * This does reporting only and will not provide a fallback implementation.
 *
 * @param symname name of the missing function
 * @return void* allways NULL.
 */
static void *dxe_res(const char *symname) {
    LOGF("%s: undefined symbol in dynamic module\n", symname);
    return NULL;
}
#endif

/**
 * @brief load a native library from disk, call it's init function and  register it in the list of loaded libraries.
 *
 * @param J VM state.
 */
static void f_LoadLibrary(js_State *J) {
#if LINUX == 1
    LOGF("Library loading is not supported on Linux: %s\n", js_tostring(J, 1));
#else
    int needed;
    char mod_name[LL_BUFFER_SIZE];
    char init_name[LL_BUFFER_SIZE];
    char shutdown_name[LL_BUFFER_SIZE];

    // sanity check
    if (!js_isdefined(J, 1) || !js_isstring(J, 1)) {
        js_error(J, "library name string expected");
        return;
    }
    const char *modname = js_tostring(J, 1);

    // bail out if this is already loaded
    if (dojs_check_library(modname)) {
        DEBUGF("%s is already loaded!", modname);
        return;
    }

    // set resolver error function
    _dlsymresolver = dxe_res;

    // generate string with <module>.dxe
    needed = snprintf(mod_name, sizeof(mod_name), "%s.dxe", modname);
    if (needed >= sizeof(mod_name)) {
        js_error(J, "Can't build DXE file name: %s", modname);
        return;
    }

    // try to open module
    void *mod = dlopen(mod_name, RTLD_GLOBAL);
    if (!mod) {
        js_error(J, "%s: Error loading %s: %s\n", modname, mod_name, dlerror());
        return;
    }

    // generate string with init_<module>
    needed = snprintf(init_name, sizeof(init_name), "_init_%s", modname);
    if (needed >= sizeof(init_name)) {
        js_error(J, "Can't build init function name: %s", modname);
        return;
    }

    // cast return value to init function pointer
    union {
        void *from;
        void (*to)(js_State *J);
    } func_ptr_cast_init;
    func_ptr_cast_init.from = dlsym(mod, init_name);
    void (*mod_init)(js_State *J) = func_ptr_cast_init.to;

    // check for valid pointer
    if (!func_ptr_cast_init.from) {
        js_error(J, "%s: Error resolving %s: %s\n", modname, init_name, dlerror());
        dlclose(mod);
        return;
    }

    // call module init
    mod_init(J);

    //////
    // now we try to find the optional shutdown() function to register that for exit

    // generate string with shutdown_<module>
    needed = snprintf(shutdown_name, sizeof(shutdown_name), "_shutdown_%s", modname);
    if (needed >= sizeof(shutdown_name)) {
        return;
    }

    // cast return value to init function pointer
    union {
        void *from;
        void (*to)(void);
    } func_ptr_cast_shutdown;
    func_ptr_cast_shutdown.from = dlsym(mod, shutdown_name);
    void (*mod_shutdown)(void) = func_ptr_cast_shutdown.to;

    // register library
    if (!dojs_register_library(modname, mod, mod_init, mod_shutdown)) {
        js_error(J, "Out of memory while registering native library. System will be unstable now!");
        return;
    }
#endif
}

/**
 * @brief get an array with the names of loaded libraries.
 *
 * @param J VM state.
 */
static void f_GetLoadedLibraries(js_State *J) {
    js_newarray(J);
#if LINUX != 1
    if (DOjS.loaded_libraries) {
        library_t *chain = DOjS.loaded_libraries;
        int idx = 0;
        while (chain) {
            js_pushstring(J, chain->name);
            js_setindex(J, -2, idx);
            idx++;
            chain = chain->next;
        }
    }
#endif
}

/**
 * @brief flush logfile to disk
 *
 * @param J VM state.
 */
static void f_FlushLog(js_State *J) { dojs_logflush(); }

/**
 * @brief set an exit message when shutting down.
 *
 * @param J VM state.
 */
static void f_SetExitMessage(js_State *J) {
    if (DOjS.exitMessage) {
        free(DOjS.exitMessage);
        DOjS.exitMessage = NULL;
    }

    const char *msg = js_tostring(J, 1);

    DOjS.exitMessage = calloc(1, strlen(msg) + 1);
    if (DOjS.exitMessage) {
        strcpy(DOjS.exitMessage, msg);
    }
}

/**
 * @brief get environment variable
 *
 * @param J VM state.
 */
static void f_GetEnv(js_State *J) {
    const char *var = js_tostring(J, 1);

    const char *val = getenv(var);

    if (val) {
        js_pushstring(J, val);
    } else {
        js_pushnull(J);
    }
}

/**
 * @brief call INT21, ah=0E, SetDrive()
 *
 * @param J VM state.
 */
static void f_SetDrive(js_State *J) {
#if LINUX == 1
    LOGF("SetDrive is not supported on Linux: %d\n", js_touint16(J, 1));
#else
    unsigned int num_drives;
    _dos_setdrive(js_touint16(J, 1), &num_drives);
    js_pushnumber(J, num_drives);
#endif
}

/**
 * @brief call INT21, ah=19, GetDrive()
 *
 * @param J VM state.
 */
static void f_GetDrive(js_State *J) {
#if LINUX == 1
    LOGF("GetDrive is not supported on Linux\n");
    js_pushnull(J);
#else
    unsigned int drive;
    _dos_getdrive(&drive);
    js_pushnumber(J, drive);
#endif
}

/**
 * @brief canonicalizes the input path
 *
 * @param J VM state.
 */
static void f_RealPath(js_State *J) {
    char newpath[PATH_MAX];
#if WINDOWS == 1
    _fullpath(newpath, js_tostring(J, 1), sizeof(newpath-1));
#else
    realpath(js_tostring(J, 1), newpath);
#endif
    js_pushstring(J, newpath);
}

/**
 * @brief convert text from UTF-8 to current encoding
 *
 * @param J VM state.
 */
static void f_FromUTF8(js_State *J) {
    const char *input = js_tostring(J, 1);
    if (need_uconvert(input, U_UTF8, U_CURRENT)) {
        size_t length = uconvert_size(input, U_UTF8, U_CURRENT);
        char *converted = malloc(length + 1);
        do_uconvert(input, U_UTF8, converted, U_CURRENT, length);
        js_pushstring(J, converted);
        free(converted);
    } else {
        js_copy(J, 1);
    }
}

/**
 * @brief convert text from current encoding to UTF-8
 *
 * @param J VM state.
 */
static void f_ToUTF8(js_State *J) {
    const char *input = js_tostring(J, 1);
    if (need_uconvert(input, U_CURRENT, U_UTF8)) {
        size_t length = uconvert_size(input, U_CURRENT, U_UTF8);
        char *converted = malloc(length + 1);
        do_uconvert(input, U_CURRENT, converted, U_UTF8, length);
        js_pushstring(J, converted);
        free(converted);
    } else {
        js_copy(J, 1);
    }
}

/**
 * @brief check allegro key array
 *
 * @param J VM state.
 */
static void f_KeyIsPressed(js_State *J) {
    uint16_t k = js_touint16(J, 1);

    if (k >= KEY_MAX) {
        js_error(J, "Key index out of range");
        return;
    }

    js_pushboolean(J, key[k]);
}

// #define DUMMY_FUNC

#ifdef DUMMY_FUNC
#include "zip.h"
static void f_Dummy(js_State *J) {
    struct zip_t *zip = zip_open("test.zip", 0, 'd');
    {
        char *entries[] = {"zipfile.h"};
        zip_entries_delete(zip, entries, 1);
    }
    zip_close(zip);
}
#endif

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

    PROPDEF_S(J, DOSJS_VERSION_STR, "DOJS_VERSION");  // global: DOjS version
    PROPDEF_S(J, DOjS.jsboot, "JSBOOT_ZIP");          // global: JSBOOT.ZIP filename
#if LINUX == 1
    PROPDEF_B(J, true, "LFN_SUPPORTED");  // global: LFN is supported
    PROPDEF_B(J, true, "LINUX");          // global: we are running on Linux
#else
    PROPDEF_B(J, _USE_LFN, "LFN_SUPPORTED");  // global: LFN is supported
    PROPDEF_B(J, false, "LINUX");             // global: we are running on DOS
#endif

    // global: command line arguments
    js_newarray(J);
    int idx = 0;
    for (int i = args; i < argc; i++) {
        js_pushstring(J, argv[i]);
        js_setindex(J, -2, idx);
        idx++;
    }
    js_setglobal(J, "ARGS");

    // set current text encoding
    char *text_encoding;
    switch (get_uformat()) {
        case U_ASCII:
            text_encoding = "ASCII";
            break;
        case U_ASCII_CP:
            text_encoding = "ASCII_CP";
            break;
        case U_UTF8:
            text_encoding = "UTF8";
            break;
        case U_UNICODE:
            text_encoding = "UNICODE";
            break;
        default:
            text_encoding = "UNKNOWN";
            break;
    }
    PROPDEF_S(J, text_encoding, "DOJS_ENCODING");
    DEBUGF("DOJS_ENCODING=%s\n", text_encoding);

    // define global functions
    NFUNCDEF(J, DirExists, 1);
    NFUNCDEF(J, FileExists, 1);
    NFUNCDEF(J, Read, 1);
    NFUNCDEF(J, ReadZIP, 2);
    NFUNCDEF(J, List, 1);
    NFUNCDEF(J, Stat, 1);
    NFUNCDEF(J, RmDir, 1);
    NFUNCDEF(J, RmFile, 1);
    NFUNCDEF(J, Rename, 2);
    NFUNCDEF(J, MakeDir, 1);
    NFUNCDEF(J, System, 2);
    NFUNCDEF(J, GetDrive, 0);
    NFUNCDEF(J, SetDrive, 1);
    NFUNCDEF(J, RealPath, 1);

    NFUNCDEF(J, Print, 0);
    NFUNCDEF(J, Println, 0);
    NFUNCDEF(J, Stop, 0);
    NFUNCDEF(J, Gc, 1);
    NFUNCDEF(J, MemoryInfo, 0);
    NFUNCDEF(J, Sleep, 1);
    NFUNCDEF(J, MsecTime, 0);
    NFUNCDEF(J, SetFramerate, 1);
    NFUNCDEF(J, GetFramerate, 0);
    NFUNCDEF(J, StringToBytes, 1);
    NFUNCDEF(J, BytesToString, 1);
    NFUNCDEF(J, NamedFunction, 3);
    NFUNCDEF(J, GetEnv, 1);

    NFUNCDEF(J, MouseSetSpeed, 2);
    NFUNCDEF(J, MouseSetLimits, 4);
    NFUNCDEF(J, MouseWarp, 2);
    NFUNCDEF(J, MouseShowCursor, 1);
    NFUNCDEF(J, MouseSetCursorMode, 1);

    NFUNCDEF(J, KeyIsPressed, 1);
    NFUNCDEF(J, SetExitKey, 1);
    NFUNCDEF(J, SetExitMessage, 1);

    NFUNCDEF(J, LoadLibrary, 1);
    NFUNCDEF(J, GetLoadedLibraries, 0);

    NFUNCDEF(J, FlushLog, 0);

    NFUNCDEF(J, ToUTF8, 1);
    NFUNCDEF(J, FromUTF8, 1);

#ifdef DUMMY_FUNC
    NFUNCDEF(J, Dummy, 0);
#endif

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
