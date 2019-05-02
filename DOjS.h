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

#ifndef __DOJS_H__
#define __DOJS_H__

#include <mujs.h>
#include <stdbool.h>
#include <stdio.h>

#include <allegro.h>

/************
** defines **
************/
#define CB_SETUP "Setup"  //!< name of setup function
#define CB_LOOP "Loop"    //!< name of loop function
#define CB_INPUT "Input"  //!< name of input function

#define SYSINFO ">>> "  //!< logfile line prefix for system messages

#define DOSJS_VERSION 0.95         //!< version number
#define DOSJS_VERSION_STR "V0.95"  //!< version number as string

#define BOOT_DIR "JSBOOT/"  //!< directory with boot files.

#ifndef PLATFORM_UNIX
#define LOGFILE "JSLOG.TXT"  //!< filename for logfile

#define LOGSTREAM logfile  //!< output stream for logging on DOS
#else
#define LOGSTREAM stdout  //!< output stream for logging on Unix
#endif

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
        fflush(LOGSTREAM);                                 \
    }

//! print debug message to logfile/console
#define DEBUG(str)                        \
    {                                     \
        fputs("[DEBUG] " str, LOGSTREAM); \
        fflush(LOGSTREAM);                \
    }
#else
#define DEBUGF(str, ...)
#define DEBUG(str)
#endif

/*********************
** global variables **
*********************/
#ifndef PLATFORM_UNIX
extern FILE *logfile;  //!< file for log output.
#endif

extern bool sound_available;  //!< indicates if WAV sound is available
extern bool mouse_available;  //!< indicates if the mouse is available
extern bool ipx_available;    //!< indicates if ipx is available
extern bool mouse_visible;    //!< indicates if the cursor should be visible.
bool transparency_available;  //!< indicates if transparency is enabled.

extern float current_frame_rate;  //!< current frame rate
extern float wanted_frame_rate;   //!< wanted frame rate

extern bool keep_running;  //!< indicates that the script should keep on running
extern int exit_key;       //!< the exit key that will stop the script

extern BITMAP *cur;                       //!< current drawing bitmap
extern volatile unsigned long sys_ticks;  //!< tick counter

/***********************
** exported functions **
***********************/
extern void update_transparency();

#endif  // __DOJS_H__
