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

#ifndef __DOJS_H__
#define __DOJS_H__

#include <allegro.h>
#include <mujs.h>
#include <stdbool.h>
#include <stdio.h>

/************
** defines **
************/
#define CB_SETUP "Setup"  //!< name of setup function (required)
#define CB_LOOP "Loop"    //!< name of loop function (required)
#define CB_INPUT "Input"  //!< name of input function (optional)

#define SYSINFO ">>> "  //!< logfile line prefix for system messages

#define DOSJS_VERSION_STR "V1.10"  //!< version number as string

#define JSBOOT_DIR "JSBOOT/"     //!< directory with boot files.
#define JSBOOT_ZIP "JSBOOT.ZIP"  //!< filename for ZIP of JSBOOT
#define JSBOOT_VAR "JSBOOTPATH"  //!< global variable containing the prefix for JSBOOT

#define LOGFILE "JSLOG.TXT"     //!< filename for logfile
#define LOGSTREAM DOjS.logfile  //!< output stream for logging on DOS

#define JS_ENOMEM(j) js_error(j, "Out of memory")                     //!< use always the same message when memory runs out
#define JS_ENOARR(j) js_error(j, "Array expected")                    //!< use always the same message when array expected
#define JS_EIDX(j, idx) js_error(j, "Index out of bound (%ld)", idx)  //!< use always the same message when array index out of bound

#define DOJS_FULL_WIDTH 640   //!< full screen width
#define DOJS_FULL_HEIGHT 480  //!< full screen height

#define DOJS_HALF_WIDTH 320   //!< half screen width
#define DOJS_HALF_HEIGHT 240  //!< half screen height

#define TICK_DELAY 10  //!< system tick handler interval in ms

//! check if parameter has a certain usertype
#define JS_CHECKTYPE(j, idx, type)            \
    {                                         \
        if (!js_isuserdata(j, idx, type)) {   \
            js_error(j, "%s expected", type); \
            return;                           \
        }                                     \
    }

//! check if a number is positive
#define JS_CHECKPOS(j, num)                                                 \
    {                                                                       \
        if (num < 0) {                                                      \
            js_error(j, "Non negative number expected: %ld", (int32_t)num); \
            return;                                                         \
        }                                                                   \
    }

/***********
** macros **
***********/
//! define a new constructor
#define CTORDEF(j, f, t, p)                \
    {                                      \
        js_newcconstructor(J, f, f, t, p); \
        js_defglobal(J, t, JS_DONTENUM);   \
    }

//! define a global function (new version)
#define NFUNCDEF(j, n, p)                 \
    {                                     \
        js_newcfunction(j, f_##n, #n, p); \
        js_setglobal(j, #n);              \
    }

//! define a method in a class (new version)
#define NPROTDEF(j, t, n, p)                                                \
    {                                                                       \
        js_newcfunction(j, t##_##n, #t ".prototype." #n, p);                \
        js_defproperty(j, -2, #n, JS_READONLY | JS_DONTENUM | JS_DONTCONF); \
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

//! define a global property of type string
#define PROPDEF_S(j, i, n)  \
    {                       \
        js_newstring(j, i); \
        js_setglobal(j, n); \
    }

//! printf-style write info to logfile/console
#define LOGF(str, ...)                                  \
    if (LOGSTREAM) {                                    \
        fprintf(LOGSTREAM, SYSINFO str, ##__VA_ARGS__); \
        fflush(LOGSTREAM);                              \
    }

//! write info to logfile/console
#define LOG(str)                       \
    if (LOGSTREAM) {                   \
        fputs(SYSINFO str, LOGSTREAM); \
        fflush(LOGSTREAM);             \
    }

//! write info to logfile/console
#define LOGV(str)                  \
    if (LOGSTREAM) {               \
        fputs(SYSINFO, LOGSTREAM); \
        fputs(str, LOGSTREAM);     \
        fflush(LOGSTREAM);         \
    }

#ifdef DEBUG_ENABLED
//! printf-style debug message to logfile/console
#define DEBUGF(str, ...)                                   \
    if (LOGSTREAM) {                                       \
        fprintf(LOGSTREAM, "[DEBUG] " str, ##__VA_ARGS__); \
        printf("[DEBUG] " str, ##__VA_ARGS__);             \
        fflush(LOGSTREAM);                                 \
        fflush(stdout);                                    \
    }

//! print debug message to logfile/console
#define DEBUG(str)                        \
    if (LOGSTREAM) {                      \
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

/**********
** types **
**********/
typedef enum {
    BLEND_REPLACE = 0,     // C=A
    BLEND_ALPHA = 1,       // C = A*alpha + B
    BLEND_ADD = 2,         // C = A+B
    BLEND_DARKEST = 3,     // C = min(A, B)
    BLEND_LIGHTEST = 4,    // C = max(A, B)
    BLEND_DIFFERENCE = 5,  // C = ABS(A-B)
    BLEND_EXCLUSION = 6,   // C := A+B - 2*A*B
    BLEND_MULTIPLY = 7,    // C := A*B
    BLEND_SCREEN = 8,      // C := 1 - (1-A)*(1-B)
    BLEND_OVERLAY = 9,     // C := A<0.5 ? (2.0*A*B):(1.0-2.0*(1.0-A)*(1.0-B))
    BLEND_HARDLIGHT = 10,  //
    BLEND_DOGE = 11,       //
    BLEND_BURN = 12,       //
    BLEND_SUBSTRACT = 13,  // C := A+B-255
} blend_mode_t;

/************
** structs **
************/
typedef struct __library_t {
    struct __library_t *next;   //!< next entry
    const char *name;           //!< name of the library (string must be free()d)
    void *handle;               //!< open library handle
    void (*init)(js_State *J);  //!< init function
    void (*shutdown)(void);     //!< shutdown function or NULL
    bool initialized;           //!< indicates initialization status
} library_t;

typedef struct {
    const char *script;  //!< script name/path
    bool run;            //!< skip editor invocation
    bool no_sound;       //!< do not initialize sound
    bool no_fm;          //!< do not initialize fm sound
    bool no_alpha;       //!< disable alpha blending
    bool highres;        //!< use 50-line mode in editor
    bool raw_write;      //!< allow raw writes in JS
    bool no_tcpip;       //!< disable Watt32 TCP stack
    int width;           //!< requested screen with
    int bpp;             //!< requested bit depth
} cmd_params_t;

typedef struct {
    cmd_params_t params;                  //!< command line parameters
    bool joystick_available;              //!< indicates if a joystick is available
    bool sound_available;                 //!< indicates if WAV sound is available
    bool midi_available;                  //!< indicates if MIDI sound is available
    bool sndin_available;                 //!< indicates if sound recording is available
    bool mouse_available;                 //!< indicates if the mouse is available
    bool ipx_available;                   //!< indicates if ipx is available
    bool mouse_visible;                   //!< indicates if the cursor should be visible.
    blend_mode_t transparency_available;  //!< indicates blend mode
    bool glide_enabled;                   //!< indicates if glide is active
    bool do_logfile;                      //!< indicates if a logfile should be created
    const char *logfile_name;             //!< name of the logfile
    float current_frame_rate;             //!< current frame rate
    float wanted_frame_rate;              //!< wanted frame rate
    bool keep_running;                    //!< indicates that the script should keep on running
    int exit_key;                         //!< the exit key that will stop the script
    BITMAP *current_bm;                   //!< current bitmap that is rendered on
    BITMAP *render_bm;                    //!< default render bitmap created at start
    volatile unsigned long sys_ticks;     //!< tick counter
    FILE *logfile;                        //!< file for log output.
    char *lastError;                      //!< last error message generated by Report()
    int num_allocs;                       //!< number of allocations for extra GC runs
    library_t *loaded_libraries;          //!< linked list of loaded libraries
    int last_mouse_x;                     //!< last reported mouse pos X
    int last_mouse_y;                     //!< last reported mouse pos y
    int last_mouse_b;                     //!< last reported mouse button
    bool input_available;                 //!< indicates if the input callback function is available
    char *exitMessage;                    //!< a message to print to the console when DOjS shuts down
    const char *jsboot;                   //!< path/name of jsboot-file.
} dojs_t;

/*********************
** global variables **
*********************/
extern dojs_t DOjS;

/***********************
** exported functions **
***********************/
extern void dojs_update_transparency(void);
extern bool dojs_register_library(const char *name, void *handle, void (*init)(js_State *J), void (*shutdown)(void));
extern bool dojs_check_library(const char *name);
extern int dojs_do_file(js_State *J, const char *fname);
extern int dojs_do_zipfile(js_State *J, const char *zipname, const char *fname);
extern void dojs_logflush(void);

#endif  // __DOJS_H__
