/*
MIT License

Copyright (c) 2019-2020 Andre Seidelt <superilu@yahoo.com>

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

#ifndef __DOJS_H__
#define __DOJS_H__

#include <allegro.h>
#include <mujs.h>
#include <stdbool.h>
#include <stdio.h>

/************
** defines **
************/
#define CB_SETUP "Setup"  //!< name of setup function
#define CB_LOOP "Loop"    //!< name of loop function
#define CB_INPUT "Input"  //!< name of input function

#define SYSINFO ">>> "  //!< logfile line prefix for system messages

#define DOSJS_VERSION 1.21          //!< version number
#define DOSJS_VERSION_STR "V1.2.1"  //!< version number as string

#define BOOT_DIR "JSBOOT/"  //!< directory with boot files.

#define LOGFILE "JSLOG.TXT"     //!< filename for logfile
#define LOGSTREAM DOjS.logfile  //!< output stream for logging on DOS

#define JS_ENOMEM(j) js_error(j, "Out of memory")   //!< use always the same message when memory runs out
#define JS_ENOARR(j) js_error(j, "Array expected")  //!< use always the same message when array expected

/***********
** macros **
***********/
//! define a global function
#define FUNCDEF(j, f, n, p)          \
    {                                \
        js_newcfunction(j, f, n, p); \
        js_setglobal(j, n);          \
    }

//! define a global property of type number
#define PROPDEF_N(j, i, n)  \
    {                       \
        js_newnumber(j, i); \
        js_setglobal(j, n); \
    }

//! define a global property of type boolean
#define PROPDEF_B(j, i, n)   \
    {                        \
        js_newboolean(j, i); \
        js_setglobal(j, n);  \
    }

//! define a method in a class
#define PROTDEF(j, f, t, n, p)                                             \
    {                                                                      \
        js_newcfunction(j, f, t ".prototype." n, p);                       \
        js_defproperty(j, -2, n, JS_READONLY | JS_DONTENUM | JS_DONTCONF); \
    }

//! printf-style write info to logfile/console
#define LOGF(str, ...)                                  \
    {                                                   \
        fprintf(LOGSTREAM, SYSINFO str, ##__VA_ARGS__); \
        fflush(LOGSTREAM);                              \
    }

//! write info to logfile/console
#define LOG(str)                       \
    {                                  \
        fputs(SYSINFO str, LOGSTREAM); \
        fflush(LOGSTREAM);             \
    }

//! write info to logfile/console
#define LOGV(str)                  \
    {                              \
        fputs(SYSINFO, LOGSTREAM); \
        fputs(str, LOGSTREAM);     \
        fflush(LOGSTREAM);         \
    }

#ifdef DEBUG_ENABLED
//! printf-style debug message to logfile/console
#define DEBUGF(str, ...)                                   \
    {                                                      \
        fprintf(LOGSTREAM, "[DEBUG] " str, ##__VA_ARGS__); \
        printf("[DEBUG] " str, ##__VA_ARGS__);             \
        fflush(LOGSTREAM);                                 \
        fflush(stdout);                                    \
    }

//! print debug message to logfile/console
#define DEBUG(str)                        \
    {                                     \
        fputs("[DEBUG] " str, LOGSTREAM); \
        puts("[DEBUG] " str);             \
        fflush(LOGSTREAM);                \
        fflush(stdout);                   \
    }
#else
#define DEBUGF(str, ...)
#define DEBUG(str)
#endif

#ifdef GC_BEFORE_MALLOC
#define NEW_OBJECT_PREP(j) js_gc(j, false)
#else
#define NEW_OBJECT_PREP(j)
#endif

/************
** structs **
************/
typedef struct {
    char *script;   //!< script name/path
    bool run;       //!< skip editor invocation
    bool no_sound;  //!< do not initialize sound
    bool no_fm;     //!< do not initialize fm sound
    bool no_alpha;  //!< disable alpha blending
    bool highres;   //!< use 50-line mode in editor
    int width;      //!< requested screen with
    int bpp;        //!< requested bit depth
} cmd_params_t;

typedef struct {
    cmd_params_t params;               //!< command line parameters
    bool joystick_available;           //!< indicates if a joystick is available
    bool sound_available;              //!< indicates if WAV sound is available
    bool midi_available;               //!< indicates if MIDI sound is available
    bool sndin_available;              //!< indicates if sound recording is available
    bool mouse_available;              //!< indicates if the mouse is available
    bool ipx_available;                //!< indicates if ipx is available
    bool mouse_visible;                //!< indicates if the cursor should be visible.
    bool transparency_available;       //!< indicates if transparency is enabled.
    bool glide_enabled;                //!< indicates if glide is active
    float current_frame_rate;          //!< current frame rate
    float wanted_frame_rate;           //!< wanted frame rate
    bool keep_running;                 //!< indicates that the script should keep on running
    int exit_key;                      //!< the exit key that will stop the script
    BITMAP *current_bm;                //!< current bitmap that is rendered on
    BITMAP *render_bm;                 //!< default render bitmap created at start
    volatile unsigned long sys_ticks;  //!< tick counter
    FILE *logfile;                     //!< file for log output.
    const char *lastError;             //!< last error message generated by Report()
} dojs_t;

/*********************
** global variables **
*********************/
extern dojs_t DOjS;

/***********************
** exported functions **
***********************/
extern void update_transparency(void);

#endif  // __DOJS_H__
