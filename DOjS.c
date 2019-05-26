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

#include <allegro.h>
#include <conio.h>
#include <jsi.h>
#include <mujs.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "DOjS.h"
#include "bitmap.h"
#include "color.h"
#include "edit.h"
#include "file.h"
#include "font.h"
#include "funcs.h"
#include "gfx.h"
#include "ipx.h"
#include "midiplay.h"
#include "sound.h"
#include "util.h"

#define TICK_DELAY 10

/**************
** Variables **
**************/
#ifndef PLATFORM_UNIX
FILE *logfile;  //!< logfile for LOGF(), LOG(), DEBUG() DEBUGF() and Print()
#endif

bool sound_available;         //!< indicates if WAV sound is available
bool mouse_available;         //!< indicates if the mouse is available
bool midi_available;          //!< indicates if midi is available
bool ipx_available;           //!< indicates if ipx is available
bool transparency_available;  //!< indicates if transparency is enabled.

bool mouse_visible;  //!< indicates if the cursor should be visible.
bool keep_running;   //!< indicates that the script should keep on running

float current_frame_rate;  //!< current frame rate
float wanted_frame_rate;   //!< wanted frame rate

const char *lastError;

int exit_key = KEY_ESC;  //!< the exit key that will stop the script

BITMAP *cur;

volatile unsigned long sys_ticks;

/*********************
** static functions **
*********************/
static void tick_handler() { sys_ticks += TICK_DELAY; }
END_OF_FUNCTION(tick_handler)

/**
 * @brief show usage on console
 */
static void usage() {
    fputs("Usage: DOjS.EXE [-r] [-s] [-f] [-a] <script>\n", stderr);
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
    lastError = message;
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
        lastError = js_trystring(J, -1, "Error");
        LOGF("Error calling %s: %s\n", name, lastError);
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

    ret = ((key >> 8) == exit_key);
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
        js_pushnumber(J, sys_ticks);
        js_setproperty(J, -2, "ticks");
    }
    if (js_pcall(J, 1)) {
        lastError = js_trystring(J, -1, "Error");
        LOGF("Error calling Input(): %s\n", lastError);
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
        return ret;
    }
}

/**
 * @brief run the given script.
 *
 * @param script the script filename.
 * @param width screen width.
 * @param bpp bit per pixel.
 * @param no_sound skip sound init.
 * @param no_fm skip fm sound init.
 * @param no_alpha disable alpha.
 */
static void run_script(char *script, int width, int bpp, bool no_sound, bool no_fm, bool no_alpha) {
    js_State *J;
#ifndef PLATFORM_UNIX
    // create logfile
    logfile = fopen(LOGFILE, "a");
    setbuf(logfile, 0);
#endif

    // create VM
    J = js_newstate(NULL, NULL, 0);
    js_atpanic(J, Panic);
    js_setreport(J, Report);

    // write startup message
    LOG("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    LOGF("DOjS %s starting with file %s\n", DOSJS_VERSION_STR, script);
#ifdef DEBUG_ENABLED
    // ut_dumpVideoModes();
#endif

    if (bpp < 24) {
        no_alpha = true;
        LOG("BPP < 24, disabling alpha\n");
    }

    // detect hardware and initialize subsystems
    sys_ticks = 0;
    allegro_init();
    install_timer();
    LOCK_VARIABLE(sys_ticks);
    LOCK_FUNCTION(tick_handler);
    install_int(tick_handler, TICK_DELAY);
    install_keyboard();
    if (install_mouse() >= 0) {
        LOG("Mouse detected\n");
        enable_hardware_cursor();
        select_mouse_cursor(MOUSE_CURSOR_ARROW);
        mouse_available = true;
        mouse_visible = true;
    } else {
        LOG("NO Mouse detected\n");
        mouse_available = false;
    }
    bool sound_ok = install_sound(no_sound ? DIGI_NONE : DIGI_AUTODETECT, no_fm ? MIDI_NONE : MIDI_AUTODETECT, NULL) == 0;
    midi_available = sound_ok && !no_fm;
    sound_available = sound_ok && !no_sound;
    init_midi(J);
    init_sound(J);
    ipx_available = init_ipx(J);
    init_funcs(J);  // must be called after initalizing the booleans above!
    init_gfx(J);
    init_color(J);
    init_bitmap(J);
    init_font(J);
    init_file(J);

    // create canvas
    set_color_depth(bpp);
    if (width == 640) {
        if (set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0) != 0) {
            LOGF("Couldn't set a %d bit color resolution at 640x480\n", bpp);
            return;  // TODO: clean up
        }
    } else {
        if (set_gfx_mode(GFX_AUTODETECT, 320, 240, 0, 0) != 0) {
            LOGF("Couldn't set a %d bit color resolution at 320x240\n", bpp);
            return;  // TODO: clean up
        }
    }
    cur = create_bitmap(SCREEN_W, SCREEN_H);
    clear_bitmap(cur);
    transparency_available = !no_alpha;
    update_transparency();

    // do some more init from JS
    js_dofile(J, JSINC_FUNC);
    js_dofile(J, JSINC_COLOR);
    js_dofile(J, JSINC_FILE);
    js_dofile(J, JSINC_IPX);

    // load main file
    lastError = NULL;
    if (js_dofile(J, script) == 0) {
        // call setup()
        keep_running = true;
        wanted_frame_rate = 30;
        if (callGlobal(J, CB_SETUP)) {
            // call loop() until someone calls Stop()
            while (keep_running) {
                long start = sys_ticks;
                if (!callGlobal(J, CB_LOOP)) {
                    if (!lastError) {
                        lastError = "Loop() not found.";
                    }
                    break;
                }
                if (callInput(J)) {
                    keep_running = false;
                }
                blit(cur, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
                if (mouse_visible) {
                    show_mouse(screen);
                }
                long end = sys_ticks;
                long runtime = (end - start) + 1;
                current_frame_rate = 1000 / runtime;
                if (current_frame_rate > wanted_frame_rate) {
                    unsigned int delay = (1000 / wanted_frame_rate) - runtime;
                    rest(delay);
                }
                end = sys_ticks;
                runtime = (end - start) + 1;
                current_frame_rate = 1000 / runtime;
            }
        } else {
            if (!lastError) {
                lastError = "Setup() not found.";
            }
        }
    }
    LOG("DOjS Shutdown...\n");
    shutdown_midi();
    shutdown_sound();
    shutdown_ipx();
#ifndef PLATFORM_UNIX
    fclose(logfile);
#endif
    allegro_exit();
    if (lastError) {
        fputs(lastError, stdout);
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
    if (transparency_available) {
        set_blender_mode(my_blender, my_blender, my_blender, 0, 0, 0, 0);
        drawing_mode(DRAW_MODE_TRANS, cur, 0, 0);
    } else {
        drawing_mode(DRAW_MODE_SOLID, cur, 0, 0);
    }
}

/**
 * @brief main entry point.
 *
 * @param argc command line parameters.
 * @param argv number of parameters.
 *
 * @return int exit code.
 */
int main(int argc, char **argv) {
    char *script = NULL;
    bool run = false;
    bool no_sound = false;
    bool no_fm = false;
    bool no_alpha = false;
    int width = 640;
    int bpp = 32;
    int opt;

    // check parameters
    while ((opt = getopt(argc, argv, "rsfaw:b:")) != -1) {
        switch (opt) {
            case 'w':
                width = atoi(optarg);
                break;
            case 'b':
                bpp = atoi(optarg);
                break;
            case 'r':
                run = true;
                break;
            case 's':
                no_sound = true;
                break;
            case 'f':
                no_fm = true;
                break;
            case 'a':
                no_alpha = true;
                break;
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
        script = argv[optind];
    }

    if (width != 640 && width != 320) {
        fprintf(stderr, "Screen width must be 640 or 320 pixel, not %d.\n", width);
        usage();
        exit(EXIT_FAILURE);
    }

    if (bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32) {
        fprintf(stderr, "Bits per pixel must be 8, 16, 24 or 32 pixel, not %d.\n", bpp);
        usage();
        exit(EXIT_FAILURE);
    }

    while (true) {
        edi_exit_t exit = EDI_KEEPRUNNING;
        if (!run) {
            exit = edi_edit(script);
        } else {
            exit = EDI_RUNSCRIPT;
        }

        if (exit == EDI_RUNSCRIPT) {
            run_script(script, width, bpp, no_sound, no_fm, no_alpha);
        }

        if (run || exit == EDI_QUIT || exit == EDI_ERROR) {
            break;
        }
    }

    exit(0);
}
END_OF_MAIN()
