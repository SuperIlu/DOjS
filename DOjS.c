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

#include <conio.h>
#include <grx20.h>
#include <grxkeys.h>
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
#include "fmmusic.h"
#include "font.h"
#include "funcs.h"
#include "ipx.h"
#include "midiplay.h"
#include "sbsound.h"
#include "util.h"

/**************
** Variables **
**************/
#ifndef PLATFORM_UNIX
FILE *logfile;  //!< logfile for LOGF(), LOG(), DEBUG() DEBUGF() and Print()
#endif

bool sound_available;  //!< indicates if WAV sound is available
bool synth_available;  //!< indicates if FM sound is available
bool mouse_available;  //!< indicates if the mouse is available
bool midi_available;   //!< indicates if midi is available
bool ipx_available;    //!< indicates if ipx is available

bool keep_running;  //!< indicates that the script should keep on running

float current_frame_rate;  //!< current frame rate
float wanted_frame_rate;   //!< wanted frame rate

const char *lastError;

GrKeyType exit_key = GrKey_Escape;  //!< the exit key that will stop the script

/*********************
** static functions **
*********************/
/**
 * @brief show usage on console
 */
static void usage() {
    fputs("Usage: DOjS.EXE [-r] [-s <p>:<i>:<d>] <script>\n", stderr);
    fputs("    -r             : Do not invoke the editor, just run the script.\n", stderr);
    fputs("    -w <width>     : Screen width: 320 or 640, Default: 640.\n", stderr);
    fputs("    -b <bbp>       : Bit per pixel:8, 16, 24, 32. Default: 24.\n", stderr);
    fputs("    -s <p>:<i>:<d> : Override sound card auto detection with given values.\n", stderr);
    fputs("                     p := Port (220h - 280h).\n", stderr);
    fputs("                     i := IRQ  (2 - 11).\n", stderr);
    fputs("                     d := DMA  (0 - 7).\n", stderr);
    fputs("                     Example: -s 220:5:1\n", stderr);
    fputs("                              -s none to disable sound\n", stderr);
    fputs("\n", stderr);
    fputs("This is DOjS " DOSJS_VERSION "\n", stderr);
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
    GrMouseEvent event;
    bool ret = false;

    if (mouse_available) {
        GrMouseGetEvent(GR_M_EVENT | GR_M_POLL, &event);
    } else {
        event.flags = 0;
        event.x = 0;
        event.y = 0;
        event.buttons = 0;
        event.key = 0;
        event.kbstat = 0;
        event.dtime = 0;
        if (GrKeyPressed()) {
            event.key = GrKeyRead();
            event.flags = GR_M_KEYPRESS;
        }
    }

    if (event.flags) {
        ret = event.key == exit_key;
        js_getglobal(J, CB_INPUT);
        js_pushnull(J);
        js_newobject(J);
        {
            js_pushnumber(J, event.x);
            js_setproperty(J, -2, "x");
            js_pushnumber(J, event.y);
            js_setproperty(J, -2, "y");
            js_pushnumber(J, event.flags);
            js_setproperty(J, -2, "flags");
            js_pushnumber(J, event.buttons);
            js_setproperty(J, -2, "buttons");
            js_pushnumber(J, event.key);
            js_setproperty(J, -2, "key");
            js_pushnumber(J, event.kbstat);
            js_setproperty(J, -2, "kbstat");
            js_pushnumber(J, event.dtime);
            js_setproperty(J, -2, "dtime");
        }
        if (js_pcall(J, 1)) {
            lastError = js_trystring(J, -1, "Error");
            LOGF("Error calling Input(): %s\n", lastError);
        }
        js_pop(J, 1);
    }
    return ret;
}

/**
 * @brief run the given script.
 *
 * @param sb_set soundblaster override string.
 * @param script the script filename.
 * @param width screen width.
 * @param bbp bit per pixel.
 */
static void run_script(char *sb_set, char *script, int width, int bbp) {
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
    LOGF("DOjS %s starting with file %s\n", DOSJS_VERSION, script);
#ifdef DEBUG_ENABLED
    // ut_dumpVideoModes();
#endif

    // detect hardware and initialize subsystems
    if (GrMouseDetect()) {
        LOG("Mouse detected\n");
        GrMouseInit();
        mouse_available = true;
    } else {
        LOG("NO Mouse detected\n");
        mouse_available = false;
    }
    synth_available = init_fmmusic(J);
    midi_available = init_midi(J);
    sound_available = init_sbsound(J, sb_set);
    ipx_available = init_ipx(J);
    init_funcs(J);  // must be called after initalizing the booleans above!
    init_color(J);
    init_bitmap(J);
    init_font(J);
    init_file(J);

    // create canvas
#ifndef PLATFORM_UNIX
    if (width == 640) {
        GrSetMode(GR_width_height_bpp_graphics, 640, 480, bbp);
    } else {
        GrSetMode(GR_width_height_bpp_graphics, 320, 240, bbp);
    }
#else
    GrSetMode(GR_default_graphics);
#endif

    // create context to use for drawing, will be blitted to the screen after Loop()
    GrClearScreen(GrBlack());
    GrContext *ctx = GrCreateContext(GrSizeX(), GrSizeY(), NULL, NULL);
    GrSetContext(ctx);
    GrClearContext(GrBlack());

    // do some more init from JS
    js_dofile(J, JSINC_FUNC);
    js_dofile(J, JSINC_FMMUSIC);
    js_dofile(J, JSINC_COLOR);
    js_dofile(J, JSINC_FONT);
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
                long start = GrMsecTime();
                if (!callGlobal(J, CB_LOOP)) {
                    lastError = "Loop() not found.";
                    LOG("Loop() not found.");
                    break;
                }
                GrBitBltNC(GrScreenContext(), 0, 0, ctx, 0, 0, GrMaxX(), GrMaxY(), GrWRITE);
                if (callInput(J)) {
                    keep_running = false;
                }
                long end = GrMsecTime();
                long runtime = (end - start) + 1;
                current_frame_rate = 1000 / runtime;
                if (current_frame_rate > wanted_frame_rate) {
                    long delay = (1000 / wanted_frame_rate) - runtime;
                    GrSleep(delay);
                }
                end = GrMsecTime();
                runtime = (end - start) + 1;
                current_frame_rate = 1000 / runtime;
            }
        } else {
            lastError = "Setup() not found.";
            LOG("Setup() not found.");
        }
    }

    LOG("DOjS Shutdown...\n");
    shutdown_midi();
    shutdown_sbsound();
    shutdown_fmmusic();
    shutdown_ipx();
#ifndef PLATFORM_UNIX
    fclose(logfile);
#endif
    if (mouse_available) {
        GrMouseUnInit();
    }
    GrSetContext(NULL);
    GrDestroyContext(ctx);
    GrSetMode(GR_default_text);
    if (lastError) {
        fputs(lastError, stdout);
        fputs("\nDOjS ERROR\n", stdout);
    } else {
        fputs("DOjS OK\n", stdout);
    }
    fflush(stdout);
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
    char *sb_set = NULL;
    char *script = NULL;
    bool run = false;
    int width = 640;
    int bpp = 24;
    int opt;

    // check parameters
    while ((opt = getopt(argc, argv, "rs:w:b:")) != -1) {
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
                sb_set = optarg;
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
            run_script(sb_set, script, width, bpp);
        }

        if (run || exit == EDI_QUIT || exit == EDI_ERROR) {
            break;
        }
    }

    exit(0);
}
