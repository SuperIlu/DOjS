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

#include "DOjS.h"

#include <allegro.h>
#include <conio.h>
#include <glide.h>
#include <jsi.h>
#include <mujs.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "3dfx-glide.h"
#include "3dfx-state.h"
#include "3dfx-texinfo.h"
#include "a3d.h"
#include "bitmap.h"
#include "color.h"
#include "comport.h"
#include "edit.h"
#include "file.h"
#include "font.h"
#include "funcs.h"
#include "gfx.h"
#include "ipx.h"
#include "joystick.h"
#include "midiplay.h"
#include "sound.h"
#include "util.h"
#include "zbuffer.h"

#define TICK_DELAY 10

/**************
** Variables **
**************/
static int last_mouse_x;
static int last_mouse_y;
static int last_mouse_b;

dojs_t DOjS;  //!< global data structure

/************************
** function prototypes **
************************/
static void tick_handler(void);
static void tick_handler_end(void);

/*********************
** static functions **
*********************/
static void tick_handler() { DOjS.sys_ticks += TICK_DELAY; }
END_OF_FUNCTION(tick_handler)

/**
 * @brief show usage on console
 */
static void usage() {
    fputs("Usage: DOjS.EXE [-r] [-s] [-f] [-a] <script> [script parameters]\n", stderr);
    fputs("    -r             : Do not invoke the editor, just run the script.\n", stderr);
    fputs("    -w <width>     : Screen width: 320 or 640, Default: 640.\n", stderr);
    fputs("    -b <bpp>       : Bit per pixel:8, 16, 24, 32. Default: 32.\n", stderr);
    fputs("    -s             : No wave sound.\n", stderr);
    fputs("    -f             : No FM sound.\n", stderr);
    fputs("    -a             : Disable alpha (speeds up rendering).\n", stderr);
    fputs("\n", stderr);
    fputs("This is DOjS " DOSJS_VERSION_STR "\n", stderr);
    fputs("(c) 2019 by Andre Seidelt <superilu@yahoo.com> and others.\n", stderr);
    fputs("See LICENSE for detailed licensing information.\n", stderr);
    fputs("\n", stderr);
    exit(1);
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
    DOjS.lastError = message;
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
        DOjS.lastError = js_trystring(J, -1, "Error");
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
    } else {
        key = -1;
    }

    ret = ((key >> 8) == DOjS.exit_key);

    // do not call JS if nothing changed
    if (key == -1 && last_mouse_x == mouse_x && last_mouse_y == mouse_y && last_mouse_b == mouse_b) {
        return ret;
    }

    // store new values
    last_mouse_x = mouse_x;
    last_mouse_y = mouse_y;
    last_mouse_b = mouse_b;

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
        DOjS.lastError = js_trystring(J, -1, "Error");
        LOGF("Error calling Input(): %s\n", DOjS.lastError);
    }
    js_pop(J, 1);

    return ret;
}

/**
 * @brief alpha calculation function used with transparent colors.
 * see https://www.gamedev.net/forums/topic/34688-alpha-blend-formula/
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
static unsigned long my_blender(unsigned long src, unsigned long dest, unsigned long n) {
    int a = (src >> 24) & 0xFF;
    if (a >= 254) {
        return src;  // no alpha, just return new color
    } else {
        int r1 = (src >> 16) & 0xFF;
        int g1 = (src >> 8) & 0xFF;
        int b1 = src & 0xFF;

        int r2 = (dest >> 16) & 0xFF;
        int g2 = (dest >> 8) & 0xFF;
        int b2 = dest & 0xFF;

        unsigned long ret = 0xFF000000 |                                          // alpha
                            (((((a * (r1 - r2)) >> 8) + r2) << 16) & 0xFF0000) |  // new r
                            (((((a * (g1 - g2)) >> 8) + g2) << 8) & 0x00FF00) |   // new g
                            ((((a * (b1 - b2)) >> 8) + b2) & 0x0000FF);           // new b
        // DEBUGF("0x%08lX 0x%08lX 0x%08lX 0x%08lX\n", src, dest, n, ret);
        return ret;
    }
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
    DOjS.logfile = fopen(LOGFILE, "a");
    setbuf(DOjS.logfile, 0);
#ifdef DEBUG_ENABLED
    freopen("STDOUT.DJS", "a", stdout);
    freopen("STDERR.DJS", "a", stderr);
#endif

    // create VM
    J = js_newstate(NULL, NULL, 0);
    js_atpanic(J, Panic);
    js_setreport(J, Report);

    // write startup message
    LOG("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    LOGF("DOjS %s (%s %s) starting with file %s\n", DOSJS_VERSION_STR, __DATE__, __TIME__, DOjS.params.script);
#ifdef DEBUG_ENABLED
    // ut_dumpVideoModes();
#endif

    // detect hardware and initialize subsystems
    DOjS.exit_key = KEY_ESC;  // the exit key that will stop the script
    DOjS.sys_ticks = 0;
    allegro_init();
    install_timer();
    LOCK_VARIABLE(DOjS.sys_ticks);
    LOCK_FUNCTION(tick_handler);
    install_int(tick_handler, TICK_DELAY);
    install_keyboard();
    if (install_mouse() >= 0) {
        LOG("Mouse detected\n");
        enable_hardware_cursor();
        select_mouse_cursor(MOUSE_CURSOR_ARROW);
        DOjS.mouse_available = true;
        DOjS.mouse_visible = true;
    } else {
        LOGF("NO Mouse detected: %s\n", allegro_error);
        DOjS.mouse_available = false;
    }
    PROPDEF_B(J, DOjS.mouse_available, "MOUSE_AVAILABLE");
    DOjS.glide_enabled = false;
    init_sound(J);  // sound init must be before midi init!
    init_midi(J);
    init_ipx(J);
    init_funcs(J, argc, argv, args);  // must be called after initalizing the booleans above!
    init_gfx(J);
    init_color(J);
    init_bitmap(J);
    init_font(J);
    init_file(J);
    init_a3d(J);
    init_zbuffer(J);
    init_3dfx(J);
    init_texinfo(J);
    init_fxstate(J);
    init_joystick(J);
    init_comport(J);

    // create canvas
    bool screenSuccess = true;
    while (true) {
        set_color_depth(DOjS.params.bpp);
        if (DOjS.params.width == 640) {
            if (set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0) != 0) {
                LOGF("Couldn't set a %d bit color resolution at 640x480: %s\n", DOjS.params.bpp, allegro_error);
            } else {
                break;
            }
        } else {
            if (set_gfx_mode(GFX_AUTODETECT, 320, 240, 0, 0) != 0) {
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
        DOjS.transparency_available = !DOjS.params.no_alpha;
        update_transparency();

        // do some more init from JS
        js_dofile(J, JSINC_FUNC);
        js_dofile(J, JSINC_COLOR);
        js_dofile(J, JSINC_FILE);
        js_dofile(J, JSINC_IPX);
        js_dofile(J, JSINC_A3D);
        js_dofile(J, JSINC_3DFX);

        // load main file
        DOjS.lastError = NULL;
        if (js_dofile(J, DOjS.params.script) == 0) {
            // call setup()
            DOjS.keep_running = true;
            DOjS.wanted_frame_rate = 30;
            if (callGlobal(J, CB_SETUP)) {
                // call loop() until someone calls Stop()
                while (DOjS.keep_running) {
                    long start = DOjS.sys_ticks;
                    if (!callGlobal(J, CB_LOOP)) {
                        if (!DOjS.lastError) {
                            DOjS.lastError = "Loop() not found.";
                        }
                        break;
                    }
                    if (callInput(J)) {
                        DOjS.keep_running = false;
                    }
                    if (DOjS.glide_enabled) {
                        grBufferSwap(1);
                    } else {
                        blit(DOjS.render_bm, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
                        if (DOjS.mouse_visible) {
                            show_mouse(screen);
                        }
                    }
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
            } else {
                if (!DOjS.lastError) {
                    DOjS.lastError = "Setup() not found.";
                }
            }
        }
    } else {
        if (!DOjS.lastError) {
            DOjS.lastError = "Screen resolution and depth now available.";
        }
    }
    LOG("DOjS Shutdown...\n");
    shutdown_midi();
    shutdown_sound();
    shutdown_joystick();
    shutdown_ipx();
    shutdown_3dfx();
    shutdown_comport();
    fclose(DOjS.logfile);
    allegro_exit();
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
void update_transparency() {
    if (DOjS.transparency_available) {
        set_blender_mode(my_blender, my_blender, my_blender, 0, 0, 0, 0);
        drawing_mode(DRAW_MODE_TRANS, DOjS.render_bm, 0, 0);
    } else {
        drawing_mode(DRAW_MODE_SOLID, DOjS.render_bm, 0, 0);
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
    DOjS.params.script = NULL;
    DOjS.params.run = false;
    DOjS.params.no_sound = false;
    DOjS.params.no_fm = false;
    DOjS.params.no_alpha = false;
    DOjS.params.width = 640;
    DOjS.params.bpp = 32;
    int opt;

    // check parameters
    while ((opt = getopt(argc, argv, "rsfahw:b:")) != -1) {
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
            case 's':
                DOjS.params.no_sound = true;
                break;
            case 'f':
                DOjS.params.no_fm = true;
                break;
            case 'a':
                DOjS.params.no_alpha = true;
                break;
            case 'h':
            default: /* '?' */
                usage();
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Script name missing.\n");
        usage();
        exit(EXIT_FAILURE);
    } else {
        DOjS.params.script = argv[optind];
    }

    if (DOjS.params.width != 640 && DOjS.params.width != 320) {
        fprintf(stderr, "Screen width must be 640 or 320 pixel, not %d.\n", DOjS.params.width);
        usage();
        exit(EXIT_FAILURE);
    }

    if (DOjS.params.bpp != 8 && DOjS.params.bpp != 16 && DOjS.params.bpp != 24 && DOjS.params.bpp != 32) {
        fprintf(stderr, "Bits per pixel must be 8, 16, 24 or 32 pixel, not %d.\n", DOjS.params.bpp);
        usage();
        exit(EXIT_FAILURE);
    }

    // ignore ctrl-c, we need it in the editor!
    signal(SIGINT, SIG_IGN);

    while (true) {
        edi_exit_t exit = EDI_KEEPRUNNING;
        if (!DOjS.params.run) {
            exit = edi_edit(DOjS.params.script);
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

    exit(0);
}
END_OF_MAIN()
