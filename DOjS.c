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

#include <grx20.h>
#include <grxkeys.h>
#include <jsi.h>
#include <mujs.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "DOjS.h"
#include "bitmap.h"
#include "color.h"
#include "file.h"
#include "fmmusic.h"
#include "font.h"
#include "funcs.h"
#include "ipx.h"
#include "midiplay.h"
#include "sbsound.h"

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

const char *lastError;

/*********************
** static functions **
*********************/
/**
 * @brief show usage on console
 */
static void usage() {
    fprintf(stderr, "Usage: DOjS <script>\n");
    exit(1);
}

/**
 * @brief write panic message.
 *
 * @param J VM state.
 */
static void Panic(js_State *J) { LOGF("In line %d of %s\n!!! PANIC !!!\n", J->line, J->filename); }

/**
 * @brief write 'report' message.
 *
 * @param J VM state.
 */
static void Report(js_State *J, const char *message) { LOGF("In line %d of %s\n%s\n", J->line, J->filename, message); }

static bool callGlobal(js_State *J, const char *name) {
    js_getglobal(J, name);
    js_pushnull(J);
    if (js_pcall(J, 0)) {
        lastError = js_trystring(J, -1, "Error");
        LOGF("%s\n", lastError);
        return false;
    }
    js_pop(J, 1);
    return true;
}

/***********************
** exported functions **
***********************/
const char *getAdapterString() {
    switch (GrAdapterType()) {
        case GR_VGA:
            return ("GR_VGA");
        case GR_EGA:
            return ("GR_EGA");
        case GR_HERC:
            return ("GR_HERC");
        case GR_8514A:
            return ("GR_8514A");
        case GR_S3:
            return ("GR_S3");
        case GR_XWIN:
            return ("GR_XWIN");
        case GR_WIN32:
            return ("GR_WIN32");
        case GR_LNXFB:
            return ("GR_LNXFB");
        case GR_SDL:
            return ("GR_SDL");
        case GR_MEM:
            return ("GR_MEM");
        default:
            return ("Unknown");
    }
}

const char *getModeString() {
    switch (GrCurrentMode()) {
        case GR_80_25_text:
            return ("GR_80_25_text");
        case GR_default_text:
            return ("GR_default_text");
        case GR_width_height_text:
            return ("GR_width_height_text");
        case GR_biggest_text:
            return ("GR_biggest_text");
        case GR_320_200_graphics:
            return ("GR_320_200_graphics");
        case GR_default_graphics:
            return ("GR_default_graphics");
        case GR_width_height_graphics:
            return ("GR_width_height_graphics");
        case GR_biggest_noninterlaced_graphics:
            return ("GR_biggest_noninterlaced_graphics");
        case GR_biggest_graphics:
            return ("GR_biggest_graphics");
        case GR_width_height_color_graphics:
            return ("GR_width_height_color_graphics");
        case GR_width_height_color_text:
            return ("GR_width_height_color_text");
        case GR_custom_graphics:
            return ("GR_custom_graphics");
        case GR_NC_80_25_text:
            return ("GR_NC_80_25_text");
        case GR_NC_default_text:
            return ("GR_NC_default_text");
        case GR_NC_width_height_text:
            return ("GR_NC_width_height_text");
        case GR_NC_biggest_text:
            return ("GR_NC_biggest_text");
        case GR_NC_320_200_graphics:
            return ("GR_NC_320_200_graphics");
        case GR_NC_default_graphics:
            return ("GR_NC_default_graphics");
        case GR_NC_width_height_graphics:
            return ("GR_NC_width_height_graphics");
        case GR_NC_biggest_noninterlaced_graphics:
            return ("GR_NC_biggest_noninterlaced_graphics");
        case GR_NC_biggest_graphics:
            return ("GR_NC_biggest_graphics");
        case GR_NC_width_height_color_graphics:
            return ("GR_NC_width_height_color_graphics");
        case GR_NC_width_height_color_text:
            return ("GR_NC_width_height_color_text");
        case GR_NC_custom_graphics:
            return ("GR_NC_custom_graphics");
        case GR_width_height_bpp_graphics:
            return ("GR_width_height_bpp_graphics");
        case GR_width_height_bpp_text:
            return ("GR_width_height_bpp_text");
        case GR_custom_bpp_graphics:
            return ("GR_custom_bpp_graphics");
        case GR_NC_width_height_bpp_graphics:
            return ("GR_NC_width_height_bpp_graphics");
        case GR_NC_width_height_bpp_text:
            return ("GR_NC_width_height_bpp_text");
        case GR_NC_custom_bpp_graphics:
            return ("case GR_NC_custom_bpp_graphics");
        default:
            return ("Unknown");
    }
}

/**
 * @brief cleanly shut down the system by freeing resources.
 */
void cleanup() {
    LOG("DOjS Shutdown...\n");
    shutdown_midi();
    shutdown_sbsound();
    shutdown_fmmusic();
    shutdown_ipx();
#ifndef PLATFORM_UNIX
    fclose(logfile);
#endif
    GrSetMode(GR_default_text);
    if (mouse_available) {
        GrMouseUnInit();
    }
    if (lastError) {
        fputs(lastError, stderr);
        fputs("\nDOjS ERROR\n", stderr);
    } else {
        fputs("DOjS OK\n", stderr);
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
    js_State *J;

    // check parameters
    if (argc != 2) {
        usage();
    }

#ifndef PLATFORM_UNIX
    // make sure all stdout is redirected to logfile
    logfile = fopen(LOGFILE, "a");
    setbuf(logfile, 0);
    freopen(LOGFILE, "a", stdout);
    setbuf(stdout, 0);
#endif

    // create VM
    J = js_newstate(NULL, NULL, 0);
    js_atpanic(J, Panic);
    js_setreport(J, Report);

    // write startup message
    LOG("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    LOGF("DOjS %s starting with file %s\n", DOSJS_VERSION, argv[1]);

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
    sound_available = init_sbsound(J);
    ipx_available = init_ipx(J);
    init_funcs(J);  // must be called after initalizing the booleans above!
    init_color(J);
    init_bitmap(J);
    init_font(J);
    init_file(J);

    // create canvas
#ifndef PLATFORM_UNIX
    GrSetMode(GR_width_height_bpp_graphics, 640, 480, 24);
#else
    GrSetMode(GR_default_graphics);
#endif
    GrClearScreen(GrBlack());

    // do some more init from JS
    js_dofile(J, JSINC_FUNC);
    js_dofile(J, JSINC_FMMUSIC);
    js_dofile(J, JSINC_COLOR);
    js_dofile(J, JSINC_FONT);
    js_dofile(J, JSINC_FILE);
    js_dofile(J, JSINC_IPX);

    // load main file
    js_dofile(J, argv[1]);

    // call setup()
    keep_running = true;
    lastError = NULL;
    if (callGlobal(J, "Setup")) {
        // call loop() until someone calls Stop()
        while (keep_running) {
            if (!callGlobal(J, "Loop")) {
                break;
            }
        }
    }
    // clean exit
    cleanup();
}
