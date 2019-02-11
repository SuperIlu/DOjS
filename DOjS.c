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
#include "fmmusic.h"
#include "font.h"
#include "funcs.h"
#include "sbsound.h"

/**************
** Variables **
**************/
#ifndef PLATFORM_UNIX
FILE *logfile; //!< logfile for DEBUG() DEBUGF() and Print()
#endif

bool sound_available; //!< indicates if WAV sound is available
bool synth_available; //!< indicates if FM sound is available
bool mouse_available; //!< indicates if the mouse is available

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
static void Panic(js_State *J) {
  DEBUGF("In line %d of %s\n!!! PANIC !!!\n", J->line, J->filename);
}

/**
 * @brief write 'report' message.
 *
 * @param J VM state.
 */
static void Report(js_State *J, const char *message) {
  DEBUGF("In line %d of %s\n%s\n", J->line, J->filename, message);
}

/***********************
** exported functions **
***********************/
/**
 * @brief cleanly shut down the system by freeing resources.
 */
void cleanup() {
  shutdown_sbsound();
  shutdown_fmmusic();
#ifndef PLATFORM_UNIX
  fclose(logfile);
#endif
  GrSetMode(GR_default_text);
  if (mouse_available) {
    GrMouseUnInit();
  }
  DEBUG("DOjS Shutdown...\n");
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
  // make sure all output is redirected to logfile
  logfile = fopen(LOGFILE, "a");
  setbuf(logfile, 0);
  freopen(LOGFILE, "a", stdout);
  setbuf(stdout, 0);
  freopen(LOGFILE, "a", stderr);
  setbuf(stderr, 0);
#endif

  // create VM
  J = js_newstate(NULL, NULL, 0);
  js_atpanic(J, Panic);
  js_setreport(J, Report);

  // write startup message
  DEBUG("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
  DEBUGF("DOjS %s starting with file %s\n", DOSJS_VERSION, argv[1]);

  // detect hardware and initialize subsystems
  if (GrMouseDetect()) {
    DEBUG("Mouse detected\n");
    GrMouseInit();
    mouse_available = true;
  } else {
    DEBUG("NO Mouse detected\n");
    mouse_available = false;
  }
  synth_available = init_fmmusic(J);
  sound_available = init_sbsound(J);
  init_funcs(J);
  init_color(J);
  init_bitmap(J);
  init_font(J);

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

  // load main file
  js_dofile(J, argv[1]);

  // clean exit
  cleanup();
}
