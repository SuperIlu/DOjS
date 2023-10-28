/*
MIT License

Copyright (c) 2019-2023 Andre Seidelt <superilu@yahoo.com>

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

#include "DOjS.h"

#if LINUX == 1
#include "linux/conio.h"
#include "linux/glue.h"
#else
#include <conio.h>
#include <glide.h>
#include <dos.h>
#include "3dfx-glide.h"
#include "3dfx-state.h"
#include "3dfx-texinfo.h"
#include "lowlevel.h"
#endif

#include <jsi.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <dlfcn.h>

#include "bitmap.h"
#include "color.h"
#include "edit.h"
#include "file.h"
#include "font.h"
#include "flic.h"
#include "funcs.h"
#include "gfx.h"
#include "joystick.h"
#include "midiplay.h"
#include "socket.h"
#include "sound.h"
#include "util.h"
#include "watt.h"
#include "zip.h"
#include "zipfile.h"
#include "intarray.h"
#include "bytearray.h"
#include "blender.h"
#include "ini.h"
#include "inifile.h"
#include "vgm.h"

#define AUTOSTART_FILE "=MAIN.JS"
#define DOJS_EXE_NAME "DOJS.EXE"

/**************
** Variables **
**************/
dojs_t DOjS;  //!< global data structure

/************************
** function prototypes **
************************/
static void tick_handler(void);

/*********************
** static functions **
*********************/
static void tick_handler() { DOjS.sys_ticks += TICK_DELAY; }
END_OF_FUNCTION(tick_handler)

/**
 * @brief show usage on console
 */
static void usage() {
    fputs("Usage: DOjS.EXE [<flags>] <script> [script parameters]\n", stderr);
    fputs("    -r             : Do not invoke the editor, just run the script.\n", stderr);
    fputs("    -l             : Use 50-line mode in the editor.\n", stderr);
    fputs("    -w <width>     : Screen width: 320 or 640, Default: 640.\n", stderr);
    fputs("    -b <bpp>       : Bit per pixel:8, 16, 24, 32. Default: 32.\n", stderr);
    fputs("    -s             : No wave sound.\n", stderr);
    fputs("    -f             : No FM sound.\n", stderr);
    fputs("    -a             : Disable alpha (speeds up rendering).\n", stderr);
    fputs("    -x             : Allow raw disk write (CAUTION!).\n", stderr);
    fputs("    -t             : Disable TCP-stack.\n", stderr);
    fputs("    -n             : Disable JSLOG.TXT.\n", stderr);
    fputs("    -j <file>      : Redirect JSLOG.TXT to <file>.\n", stderr);
    fputs("\n", stderr);
    fputs("This is DOjS " DOSJS_VERSION_STR "\n", stderr);
    fputs("(c) 2019-2022 by Andre Seidelt <superilu@yahoo.com> and others.\n", stderr);
    fputs("See LICENSE for detailed licensing information.\n", stderr);
    fputs("\n", stderr);
    exit(1);
}

/**
 * @brief initialize last error function.
 */
static void init_last_error() { DOjS.lastError = NULL; }

/**
 * @brief free any memnory of the last error.
 */
static void clear_last_error() {
    if (DOjS.lastError) {
        free(DOjS.lastError);
        DOjS.lastError = NULL;
    }
}

/**
 * @brief Set the last error. the string data is copied to malloc()ed RAM.
 *
 * @param err the last error string.
 */
static void set_last_error(const char *err) {
    clear_last_error();

    DOjS.lastError = calloc(1, strlen(err) + 1);
    if (DOjS.lastError) {
        strcpy(DOjS.lastError, err);
    }
}

/**
 * @brief write panic message.
 *
 * @param J VM state.
 */
static void Panic(js_State *J) { LOGF("!!! PANIC in %s !!!\n", J->filename); }

/**
 * @brief write 'report' message.
 *
 * @param J VM state.
 */
static void Report(js_State *J, const char *message) {
    set_last_error(message);
    LOGF("%s\n", message);
}

/**
 * @brief call a globally define JS function.
 *
 * @param J VM state.
 * @param name function name.
 *
 * @return true if the function was found.
 * @return false if the function was not found.
 */
static bool callGlobal(js_State *J, const char *name) {
    js_getglobal(J, name);
    js_pushnull(J);
    if (js_pcall(J, 0)) {
        set_last_error(js_trystring(J, -1, "Error"));
        LOGF("Error calling %s: %s\n", name, DOjS.lastError);
        return false;
    }
    js_pop(J, 1);
    return true;
}

/**
 * @brief handle input.
 *
 * @param J VM state.
 *
 * @return true if the found event was exit_key.
 * @return false if no or any other event occured.
 */
static bool callInput(js_State *J) {
    int key;
    bool ret = false;

    if (keyboard_needs_poll()) {
        poll_keyboard();
    }
    if (mouse_needs_poll()) {
        poll_mouse();
    }

    if (keypressed()) {
        key = readkey();
        ret = ((key >> 8) == DOjS.exit_key);
    } else {
        key = -1;
        ret = false;
    }

#if LINUX == 1
    // check for CTRL_ENTER
    if (key_shifts & KB_CTRL_FLAG) {
        if (key == 0x430D) {
            key = 0x430A;
        }
    }

#endif

    // do not call JS if nothing changed or no input function
    if (!DOjS.input_available || ((key == -1) && (DOjS.last_mouse_x == mouse_x) && (DOjS.last_mouse_y == mouse_y) && (DOjS.last_mouse_b == mouse_b))) {
        return ret;
    }

    // store new values
    DOjS.last_mouse_x = mouse_x;
    DOjS.last_mouse_y = mouse_y;
    DOjS.last_mouse_b = mouse_b;

    // call JS
    js_getglobal(J, CB_INPUT);
    js_pushnull(J);
    js_newobject(J);
    {
        js_pushnumber(J, mouse_x);
        js_setproperty(J, -2, "x");
        js_pushnumber(J, mouse_y);
        js_setproperty(J, -2, "y");
        js_pushnumber(J, mouse_b);
        js_setproperty(J, -2, "buttons");
        js_pushnumber(J, key);
        js_setproperty(J, -2, "key");
        js_pushnumber(J, DOjS.sys_ticks);
        js_setproperty(J, -2, "ticks");
    }
    if (js_pcall(J, 1)) {
        set_last_error(js_trystring(J, -1, "Error"));
        LOGF("Error calling Input(): %s\n", DOjS.lastError);
    }
    js_pop(J, 1);

    return ret;
}

/**
 * @brief load and parse a javascript file from ZIP.
 *
 * @param J VM state.
 * @param fname fname, ZIP-files using ZIP_DELIM.
 */
static void dojs_loadfile_zip(js_State *J, const char *fname) {
    char *s, *p;
    size_t n;

    if (!read_zipfile1(fname, (void **)&s, &n)) {
        js_error(J, "cannot open file '%s'", fname);
        return;
    }

    if (js_try(J)) {
        free(s);
        js_throw(J);
    }

    /* skip first line if it starts with "#!" */
    p = s;
    if (p[0] == '#' && p[1] == '!') {
        p += 2;
        while (*p && *p != '\n') ++p;
    }

    DEBUGF("Parsing ZIP entry '%s'\n", fname);

    js_loadstring(J, fname, p);

    free(s);
    js_endtry(J);
}

/**
 * @brief load all system include JS files either from ZIP (if found) or from file system.
 *
 * @param J VM state.
 */
static void dojs_load_jsboot(js_State *J) {
    if (ut_file_exists(DOjS.jsboot)) {
        DEBUGF("%s found, using archive\n", DOjS.jsboot);

        char buffer[1024];  // this is a hack, I'm to lazy to calculate an appropriate buffer
        snprintf(buffer, sizeof(buffer), "%s%s", DOjS.jsboot, ZIP_DELIM_STR JSBOOT_DIR);
        PROPDEF_S(J, buffer, JSBOOT_VAR);

        dojs_do_zipfile(J, DOjS.jsboot, JSINC_FUNC);
        dojs_do_zipfile(J, DOjS.jsboot, JSINC_COLOR);
        dojs_do_zipfile(J, DOjS.jsboot, JSINC_FILE);
#if LINUX != 1
        dojs_do_zipfile(J, DOjS.jsboot, JSINC_3DFX);
#endif
        dojs_do_zipfile(J, DOjS.jsboot, JSINC_SOCKET);
    } else {
        DEBUGF("%s NOT found, using plain files\n", DOjS.jsboot);
        PROPDEF_S(J, JSBOOT_DIR, JSBOOT_VAR);
        dojs_do_file(J, JSINC_FUNC);
        dojs_do_file(J, JSINC_COLOR);
        dojs_do_file(J, JSINC_FILE);
#if LINUX != 1
        dojs_do_file(J, JSINC_3DFX);
#endif
        dojs_do_file(J, JSINC_SOCKET);
    }
}

#ifdef MEMDEBUG
/**
 * @brief debugging version of js_alloc()
 *
 * @param actx context (unused).
 * @param ptr pointer for remalloc()/free()
 * @param size realloc()/malloc() size, 0 for free().
 *
 * @return void* (re)allocated memory
 */
static void *dojs_alloc(void *actx, void *ptr, int size) {
    void *ret = NULL;
    if (size == 0) {
        free(ptr);
        ret = NULL;
        DEBUGF("DBG FREE(0x%p, %d) := 0x%p\n", ptr, size, ret);
    } else if (!ptr) {
        DOjS.num_allocs++;
        ret = malloc((size_t)size);
        DEBUGF("DBG MALL(0x%p, %d) := 0x%p\n", ptr, size, ret);
        // if (!ret) {
        //     LOGF("MALLOC failed");
        //     exit(1);
        // }
    } else {
        DOjS.num_allocs++;
        ret = realloc(ptr, (size_t)size);
        DEBUGF("DBG   RE(0x%p, %d) := 0x%p\n", ptr, size, ret);
        // if (!ret) {
        //     LOGF("REALLOC failed");
        //     exit(1);
        // }
    }
    return ret;
}
#else
/**
 * @brief debugging version of js_alloc()
 *
 * @param actx context (unused).
 * @param ptr pointer for remalloc()/free()
 * @param size realloc()/malloc() size, 0 for free().
 *
 * @return void* (re)allocated memory
 */
static void *dojs_alloc(void *actx, void *ptr, int size) {
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    DOjS.num_allocs++;
    return realloc(ptr, (size_t)size);
}
#endif

#if LINUX != 1
/**
 * @brief call shutdown() on all registered libraries
 */
static void dojs_shutdown_libraries() {
    if (DOjS.loaded_libraries) {
        library_t *chain = DOjS.loaded_libraries;
        while (chain) {
            DEBUGF("%p: Library shutdown for %s. Shutdown function %p\n", chain, chain->name, chain->shutdown);

            // call shutdown if any
            if (chain->shutdown) {
                chain->shutdown();
            }
            chain->initialized = false;
            chain = chain->next;
        }
    }
}

/**
 * @brief call init() on all registered libraries (re-run of the sketch from editor)
 */
static void dojs_init_libraries(js_State *J) {
    if (DOjS.loaded_libraries) {
        library_t *chain = DOjS.loaded_libraries;
        while (chain) {
            DEBUGF("%p: Library init for %s. Shutdown function %p\n", chain, chain->name, chain->init);

            // call init if not already initialized
            if (chain->init && !chain->initialized) {
                chain->init(J);
                chain->initialized = true;
            }
            chain = chain->next;
        }
    }
}
#endif

/**
 * @brief check Input(), Loop() and Setup() are defined.
 *
 * @param J VM state.
 * @param fname name of the script file
 * @return true if all required functions are defined, false if at least one required is missing.
 */
static bool check_functions(js_State *J, const char *fname) {
    bool ret = true;
    js_pushglobal(J);

    if (js_hasproperty(J, 0, CB_INPUT)) {
        DOjS.input_available = true;
    } else {
        DOjS.input_available = false;
        LOGF("Function %s not found in %s -> input disabled\n", CB_INPUT, fname);
    }

    if (js_hasproperty(J, 0, CB_ONEXIT)) {
        DOjS.onexit_available = true;
    } else {
        DOjS.onexit_available = false;
    }

    if (!js_hasproperty(J, 0, CB_LOOP)) {
        set_last_error("Script has no " CB_LOOP "() function");
        LOG("Script has no " CB_LOOP "() function\n");
        ret = false;
    }

    if (!js_hasproperty(J, 0, CB_SETUP)) {
        set_last_error("Script has no " CB_SETUP "() function");
        LOG("Script has no " CB_SETUP "() function\n");
        ret = false;
    }

    js_pop(J, 1);

    return ret;
}

/**
 * @brief run the given script.
 *
 * @param argc number of parameters.
 * @param argv command line parameters.
 * @param args first script parameter.
 */
static void run_script(int argc, char **argv, int args) {
    js_State *J;

    // create logfile
    if (DOjS.logfile_name) {
        DOjS.logfile = fopen(DOjS.logfile_name, "a");
        if (!DOjS.logfile) {
            fprintf(stderr, "Could not open/create logfile %s.\n", DOjS.logfile_name);
            exit(1);
        }
        setbuf(DOjS.logfile, 0);
    } else {
        DOjS.logfile = NULL;
    }
#ifdef DEBUG_ENABLED
    (void *)freopen("STDOUT.DJS", "a", stdout);
    (void *)freopen("STDERR.DJS", "a", stderr);
#endif

    // (re)init out DOjS struct
    DOjS.num_allocs = 0;
    DOjS.exit_key = KEY_ESC;  // the exit key that will stop the script
    DOjS.sys_ticks = 0;
    DOjS.glide_enabled = false;
    DOjS.mouse_available = false;
    DOjS.mouse_visible = false;
    clear_last_error();

    // create VM
    J = js_newstate(dojs_alloc, NULL, 0);
    js_atpanic(J, Panic);
    js_setreport(J, Report);

    // write startup message
    LOG("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    LOGF("DOjS %s (%s %s) starting with file %s\n", DOSJS_VERSION_STR, __DATE__, __TIME__, DOjS.params.script);
    DEBUGF("Running on %s %d.%d\n", _os_flavor, _osmajor, _osminor);
#ifdef DEBUG_ENABLED
    // ut_dumpVideoModes();
#endif

    // detect hardware and initialize subsystems
    allegro_init();
    install_timer();
    LOCK_VARIABLE(DOjS.sys_ticks);
    LOCK_FUNCTION(tick_handler);
    install_int(tick_handler, TICK_DELAY);
    install_keyboard();
    if (install_mouse() >= 0) {
        LOGF("Mouse detected: %s\n", mouse_driver->name);
        enable_hardware_cursor();
        select_mouse_cursor(MOUSE_CURSOR_ARROW);
        DOjS.mouse_available = true;
        DOjS.mouse_visible = true;
    } else {
        LOGF("NO Mouse detected: %s\n", allegro_error);
    }
    PROPDEF_B(J, DOjS.mouse_available, "MOUSE_AVAILABLE");
    init_sound(J);  // sound init must be before midi init!
    init_midi(J);
    init_funcs(J, argc, argv, args);  // must be called after initalizing the booleans above!
    init_gfx(J);
    init_color(J);
    init_bitmap(J);
    init_font(J);
    init_file(J);
    init_joystick(J);
    init_watt(J);
    init_socket(J);
    init_zipfile(J);
    init_intarray(J);
    init_bytearray(J);
    init_flic(J);
    init_inifile(J);
#if LINUX != 1
    init_vgm(J);
    init_lowlevel(J);
    init_3dfx(J);
    init_texinfo(J);
    init_fxstate(J);
#endif

    // create canvas
    bool screenSuccess = true;
#if LINUX == 1
    int gfx_mode = GFX_AUTODETECT_WINDOWED;
#else
    int gfx_mode = GFX_AUTODETECT;
#endif
    while (true) {
        set_color_depth(DOjS.params.bpp);
        if (DOjS.params.width == DOJS_FULL_WIDTH) {
            if (set_gfx_mode(gfx_mode, DOJS_FULL_WIDTH, DOJS_FULL_HEIGHT, 0, 0) != 0) {
                LOGF("Couldn't set a %d bit color resolution at 640x480: %s\n", DOjS.params.bpp, allegro_error);
            } else {
                break;
            }
        } else {
            if (set_gfx_mode(gfx_mode, DOJS_HALF_WIDTH, DOJS_HALF_HEIGHT, 0, 0) != 0) {
                LOGF("Couldn't set a %d bit color resolution at 320x240: %s\n", DOjS.params.bpp, allegro_error);
            } else {
                break;
            }
        }
        if (DOjS.params.bpp == 32) {
            DOjS.params.bpp = 24;
            LOG("32 bit color resolution not available, trying 24 bit fallback...\n");
        } else {
            screenSuccess = false;
            break;
        }
    }
    if (DOjS.params.bpp < 24) {
        DOjS.params.no_alpha = true;
        LOG("BPP < 24, disabling alpha\n");
    }
    if (screenSuccess) {
        DOjS.render_bm = DOjS.current_bm = create_bitmap(SCREEN_W, SCREEN_H);
        clear_bitmap(DOjS.render_bm);
        DOjS.transparency_available = DOjS.params.no_alpha ? BLEND_REPLACE : BLEND_ALPHA;
        dojs_update_transparency();

        DEBUGF("GFX_Capabilities=%08X\n", gfx_capabilities);

        // do some more init from JS
        dojs_load_jsboot(J);

        // load main file
        if (dojs_do_file(J, DOjS.params.script) == 0) {
            if (check_functions(J, DOjS.params.script)) {
                // call setup()
                DOjS.keep_running = true;
                DOjS.wanted_frame_rate = 30;
#if LINUX != 1
                dojs_init_libraries(J);  // re-init all already loaded libraries
#else
                glue_init(J);
#endif
                if (callGlobal(J, CB_SETUP)) {
                    // call loop() until someone calls Stop()
                    while (DOjS.keep_running) {
                        long start = DOjS.sys_ticks;
                        if (DOjS.num_allocs > 1000) {
#ifdef MEMDEBUG
                            js_gc(J, 1);
#else
                            js_gc(J, 0);
#endif
                            DOjS.num_allocs = 0;
                        }
                        tick_socket();
                        if (!callGlobal(J, CB_LOOP)) {
                            if (!DOjS.lastError) {
                                set_last_error("Loop() not found.");
                            }
                            break;
                        }
                        if (callInput(J)) {
                            DOjS.keep_running = false;
                        }
#if LINUX != 1
                        if (DOjS.glide_enabled) {
                            grBufferSwap(1);
                        } else {
#endif
                            show_mouse(NULL);
                            blit(DOjS.render_bm, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
                            if (DOjS.mouse_visible) {
                                show_mouse(screen);
                            }
#if LINUX != 1
                        }
#endif
                        long end = DOjS.sys_ticks;
                        long runtime = (end - start) + 1;
                        DOjS.current_frame_rate = 1000 / runtime;
                        if (DOjS.current_frame_rate > DOjS.wanted_frame_rate) {
                            unsigned int delay = (1000 / DOjS.wanted_frame_rate) - runtime;
                            rest(delay);
                        }
                        end = DOjS.sys_ticks;
                        runtime = (end - start) + 1;
                        DOjS.current_frame_rate = 1000 / runtime;
                    }
                    if (DOjS.onexit_available) {
                        callGlobal(J, CB_ONEXIT);
                    }
                }
            } else {
                if (!DOjS.lastError) {
                    set_last_error("Setup() not found.");
                }
            }
        }
    } else {
        if (!DOjS.lastError) {
            set_last_error("Screen resolution and depth not available.");
        }
    }
    LOG("DOjS Shutdown...\n");
    js_freestate(J);
#if LINUX != 1
    dojs_shutdown_libraries();
    shutdown_3dfx();
    shutdown_vgm();
#else
    glue_shutdown();
#endif
    shutdown_flic();
    shutdown_midi();
    shutdown_sound();
    shutdown_joystick();
    if (DOjS.logfile) {
        fclose(DOjS.logfile);
    }
    allegro_exit();
    textmode(C80);

    if (DOjS.exitMessage && (strlen(DOjS.exitMessage) > 0)) {
        fputs(DOjS.exitMessage, stdout);
    }

    if (DOjS.lastError) {
        fputs(DOjS.lastError, stdout);
        fputs("\nDOjS ERROR\n", stdout);
    } else {
        fputs("DOjS OK\n", stdout);
    }
    fflush(stdout);
}

/***********************
** exported functions **
***********************/
/**
 * @brief load and parse a file from ZIP.
 *
 * @param J VM state.
 * @param zipname the name of the zip to read from.
 * @param fname the name of the file.
 *
 * @return int TRUE if successfull, FALSE if not.
 */
int dojs_do_zipfile(js_State *J, const char *zipname, const char *fname) {
    char buffer[1024];  // this is a hack, I'm to lazy to calculate an appropriate buffer
    snprintf(buffer, sizeof(buffer), "%s" ZIP_DELIM_STR "%s", DOjS.jsboot, fname);
    return dojs_do_file(J, buffer);
}

/**
 * @brief load and parse a file from filesystem or ZIP.
 *
 * @param J VM state.
 * @param fname filename, ZIP-files using ZIP_DELIM.
 *
 * @return int TRUE if successfull, FALSE if not.
 */
int dojs_do_file(js_State *J, const char *fname) {
    char *delim = strchr(fname, ZIP_DELIM);
    if (!delim) {
        DEBUGF("Parsing plain file '%s'\n", fname);
        return js_dofile(J, fname);
    } else {
        DEBUGF("Parsing ZIP file '%s'\n", fname);
        if (js_try(J)) {
            js_report(J, js_trystring(J, -1, "Error"));
            js_pop(J, 1);
            return 1;
        }
        dojs_loadfile_zip(J, fname);
        js_pushundefined(J);
        js_call(J, 0);
        js_pop(J, 1);
        js_endtry(J);
        return 0;
    }
}

#if LINUX != 1
/**
 * @brief register a library.
 *
 * @param name pointer to a name. This function will make a copy of the string.
 * @param handle the handle returned by dlopen().
 * @param init the init function.
 * @param shutdown function to be called for shutdown or NULL.
 *
 * @return true if registration succeeded, else false.
 */
bool dojs_register_library(const char *name, void *handle, void (*init)(js_State *J), void (*shutdown)(void)) {
    DEBUGF("Registering library %s. Shutdown function %p\n", name, shutdown);

    // get new entry
    library_t *new_entry = calloc(1, sizeof(library_t));
    if (!new_entry) {
        LOGF("WARNING: Could not register shutdown hook for loaded library %s!", name);
        return false;
    }

    // copy name
    char *name_copy = malloc(strlen(name) + 1);
    if (!name_copy) {
        LOGF("WARNING: Could not register shutdown hook for loaded library %s!", name);
        free(new_entry);
        return false;
    }
    strcpy(name_copy, name);

    // store values
    new_entry->name = name_copy;
    new_entry->handle = handle;
    new_entry->init = init;
    new_entry->shutdown = shutdown;
    new_entry->initialized = true;
    DEBUGF("%s: Created %p with init=%p and shutdown=%p\n", new_entry->name, new_entry, new_entry->init, new_entry->shutdown);

    // insert at start of list
    new_entry->next = DOjS.loaded_libraries;
    DOjS.loaded_libraries = new_entry;
    return true;
}

/**
 * @brief check if a given library is already registered. Call init() if wanted
 *
 * @param name name to search for.
 *
 * @return true if the library is already in the list, else false.
 */
bool dojs_check_library(const char *name) {
    if (DOjS.loaded_libraries) {
        library_t *chain = DOjS.loaded_libraries;
        while (chain) {
            if (strcmp(name, chain->name) == 0) {
                DEBUGF("%s: Found %p\n", chain->name, chain);
                return true;
            }

            chain = chain->next;
        }
    }
    return false;
}
#endif

/**
 * @brief set the active blender func from DOjS.transparency_available
 */
void dojs_update_transparency() {
    BLENDER_FUNC bfunc = NULL;
    switch (DOjS.transparency_available) {
        case BLEND_ALPHA:
            bfunc = blender_alpha;
            break;
        case BLEND_ADD:
            bfunc = blender_add;
            break;
        case BLEND_DARKEST:
            bfunc = blender_darkest;
            break;
        case BLEND_LIGHTEST:
            bfunc = blender_lightest;
            break;
        case BLEND_DIFFERENCE:
            bfunc = blender_difference;
            break;
        case BLEND_EXCLUSION:
            bfunc = blender_exclusion;
            break;
        case BLEND_MULTIPLY:
            bfunc = blender_multiply;
            break;
        case BLEND_SCREEN:
            bfunc = blender_screen;
            break;
        case BLEND_OVERLAY:
            bfunc = blender_overlay;
            break;
        case BLEND_HARDLIGHT:
            bfunc = blender_hardlight;
            break;
        case BLEND_DOGE:
            bfunc = blender_doge;
            break;
        case BLEND_BURN:
            bfunc = blender_burn;
            break;
        case BLEND_SUBSTRACT:
            bfunc = blender_substract;
            break;
        default:
            bfunc = NULL;
            break;
    }

    if (bfunc) {
        DEBUGF("Using blender %p\n", bfunc);
        set_blender_mode(bfunc, bfunc, bfunc, 0, 0, 0, 0);
        drawing_mode(DRAW_MODE_TRANS, DOjS.render_bm, 0, 0);
    } else {
        DEBUGF("Using solid mode\n");
        solid_mode();
    }
}

/**
 * @brief cloe and re-open logfile to flush() all data and make the logfile accessible from Javascript.
 */
void dojs_logflush() {
    if (DOjS.logfile) {
        // close current logfile
        fclose(DOjS.logfile);

        // recreate logfile
        if (DOjS.logfile_name) {
            DOjS.logfile = fopen(DOjS.logfile_name, "a");
            if (!DOjS.logfile) {
                fprintf(stderr, "Could not open/create logfile %s.\n", DOjS.logfile_name);
                exit(1);
            }
            setbuf(DOjS.logfile, 0);
        } else {
            DOjS.logfile = NULL;
        }
    }
}

/**
 * @brief main entry point.
 *
 * @param argc number of parameters.
 * @param argv command line parameters.
 *
 * @return int exit code.
 */
int main(int argc, char **argv) {
    const char *script_param = NULL;

    // initialize DOjS main structure
    bzero(&DOjS, sizeof(DOjS));
    DOjS.params.width = DOJS_FULL_WIDTH;
    DOjS.params.bpp = 32;
    DOjS.do_logfile = true;
    DOjS.logfile_name = LOGFILE;
    DOjS.jsboot = JSBOOT_ZIP;
    init_last_error();

    ini_t *config = ini_load("dojs.ini");
    if (config) {
        const char *value;

        value = ini_get(config, NULL, "w");
        if (value) {
            DOjS.params.width = atoi(value);
        }

        value = ini_get(config, NULL, "b");
        if (value) {
            DOjS.params.bpp = atoi(value);
        }

        value = ini_get(config, NULL, "r");
        if (value) {
            DOjS.params.run = true;
        }

        value = ini_get(config, NULL, "l");
        if (value) {
            DOjS.params.highres = true;
        }

        value = ini_get(config, NULL, "s");
        if (value) {
            DOjS.params.no_sound = true;
        }

        value = ini_get(config, NULL, "t");
        if (value) {
            DOjS.params.no_tcpip = true;
        }

        value = ini_get(config, NULL, "f");
        if (value) {
            DOjS.params.no_fm = true;
        }

        value = ini_get(config, NULL, "a");
        if (value) {
            DOjS.params.no_alpha = true;
        }

        value = ini_get(config, NULL, "x");
        if (value) {
            DOjS.params.raw_write = true;
        }

        value = ini_get(config, NULL, "n");
        if (value) {
            DOjS.do_logfile = false;
        }

        value = ini_get(config, NULL, "j");
        if (value) {
            DOjS.logfile_name = value;
        }

        script_param = ini_get(config, NULL, "script");
    }

    // check command line parameters
    int opt;
    while ((opt = getopt(argc, argv, "tnxlrsfahw:b:j:")) != -1) {
        switch (opt) {
            case 'w':
                DOjS.params.width = atoi(optarg);
                break;
            case 'b':
                DOjS.params.bpp = atoi(optarg);
                break;
            case 'r':
                DOjS.params.run = true;
                break;
            case 'l':
                DOjS.params.highres = true;
                break;
            case 's':
                DOjS.params.no_sound = true;
                break;
            case 't':
                DOjS.params.no_tcpip = true;
                break;
            case 'f':
                DOjS.params.no_fm = true;
                break;
            case 'a':
                DOjS.params.no_alpha = true;
                break;
            case 'x':
                DOjS.params.raw_write = true;
                break;
            case 'n':
                DOjS.do_logfile = false;
                break;
            case 'j':
                DOjS.logfile_name = optarg;
                break;
            case 'h':
            default: /* '?' */
                usage();
                exit(EXIT_FAILURE);
        }
    }

    // 'n' takes preceedence over redirection
    if (!DOjS.do_logfile) {
        DOjS.logfile_name = NULL;
    }

    if (optind < argc) {
        script_param = argv[optind];
    }

    if (!script_param) {
        // we got no script parameter, this means Usage or try to run script from a ZIP file with our EXE name
        if (!ut_endsWith(argv[0], DOJS_EXE_NAME)) {
            // exe name was changed --> try to run xxx.ZIP=MAIN.JS
            char *autostart_zip = ut_clone_string(argv[0]);
            if (!autostart_zip) {
                fprintf(stderr, "Out of memory.\n\n");
                exit(EXIT_FAILURE);
            }
            size_t autostart_len = strlen(autostart_zip);
            autostart_zip[autostart_len - 3] = 'Z';
            autostart_zip[autostart_len - 2] = 'I';
            autostart_zip[autostart_len - 1] = 'P';

            // try autostart
            size_t script_len = autostart_len + 1 + strlen(AUTOSTART_FILE);
            char *autostart_script = calloc(1, script_len);
            if (!autostart_script) {
                fprintf(stderr, "Out of memory.\n\n");
                exit(EXIT_FAILURE);
            }
            strcpy(autostart_script, autostart_zip);
            strcat(autostart_script, AUTOSTART_FILE);
            if (check_zipfile1(autostart_script)) {
                // !!! we found a ZIP file with the name of the EXE and it has a MAIN.JS -> run MAIN.JS from this ZIP
                DOjS.params.script = autostart_script;
                DOjS.params.run = true;
                DOjS.jsboot = autostart_zip;
            }
        } else {
            // !!! someone called dojs.exe w/o parameters -> try JSBOOT.ZIP=MAIN.JS
            if (check_zipfile1(JSBOOT_ZIP AUTOSTART_FILE)) {
                // !!! we found a ZIP file with the name of the EXE and it has a MAIN.JS -> run MAIN.JS from this ZIP
                DOjS.params.script = JSBOOT_ZIP AUTOSTART_FILE;
                DOjS.params.run = true;
            }
        }
    } else {
        // we got a parameter, check if it is a JS, a ZIP or a JS in a ZIP
        if (check_zipfile1(script_param)) {
            // !!! we found the specified JS in a ZIP
            DOjS.params.script = script_param;
            DOjS.params.run = true;
            DOjS.jsboot = ut_clone_string(script_param);
            char *delim = strchr(DOjS.jsboot, ZIP_DELIM);
            *delim = 0;
        } else {
            // try if this is a ZIP file with a MAIN.JS in it
            char *autostart_zip = malloc(strlen(script_param) + 1 + strlen(AUTOSTART_FILE));
            if (!autostart_zip) {
                fprintf(stderr, "Out of memory.\n\n");
                exit(EXIT_FAILURE);
            }
            strcpy(autostart_zip, script_param);
            strcat(autostart_zip, AUTOSTART_FILE);
            if (check_zipfile1(autostart_zip)) {
                // !!! ZIP with MAIN.JS in it
                DOjS.params.script = autostart_zip;
                DOjS.params.run = true;
                DOjS.jsboot = script_param;
            } else {
                // !!! this should be a plain JS file
                DOjS.params.script = script_param;
            }
        }
    }

    // check if the above yielded a script name and if the combination is valid
    if (!DOjS.params.script) {
        fprintf(stderr, "Script name missing.\n\n");
        usage();
        exit(EXIT_FAILURE);
    }
    if (!DOjS.params.run && strchr(DOjS.params.script, ZIP_DELIM)) {
        fprintf(stderr, "ZIP-Scripts are only supported with option '-r'.\n\n");
        usage();
        exit(EXIT_FAILURE);
    }

    // check screen size parameters
    if (DOjS.params.width != DOJS_FULL_WIDTH && DOjS.params.width != DOJS_HALF_WIDTH) {
        fprintf(stderr, "Screen width must be 640 or 320 pixel, not %d.\n\n", DOjS.params.width);
        usage();
        exit(EXIT_FAILURE);
    }

    // check bpp parameters
    if (DOjS.params.bpp != 8 && DOjS.params.bpp != 16 && DOjS.params.bpp != 24 && DOjS.params.bpp != 32) {
        fprintf(stderr, "Bits per pixel must be 8, 16, 24 or 32 pixel, not %d.\n\n", DOjS.params.bpp);
        usage();
        exit(EXIT_FAILURE);
    }

#if LINUX != 1
    // ignore ctrl-c, we need it in the editor!
    signal(SIGINT, SIG_IGN);
#endif

    while (true) {
        edi_exit_t exit = EDI_KEEPRUNNING;
        if (!DOjS.params.run) {
            exit = edi_edit(DOjS.params.script, DOjS.params.highres, DOjS.lastError);
        } else {
            exit = EDI_RUNSCRIPT;
        }

        if (exit == EDI_RUNSCRIPT) {
            run_script(argc, argv, optind);
        }

        if (DOjS.params.run || exit == EDI_QUIT || exit == EDI_ERROR) {
            break;
        }
    }

    clear_last_error();
    if (DOjS.exitMessage) {
        free(DOjS.exitMessage);
        DOjS.exitMessage = NULL;
    }

    // free INI files (if any)
    if (config) {
        ini_free(config);
    }

    exit(0);
}
END_OF_MAIN()
