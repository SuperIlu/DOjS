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
#include <errno.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

#include "DOjS.h"
#include "util.h"
#include "zipfile.h"

#include "micromod-c/micromod.h"

void init_micromod(js_State *J);

/****************
** static data **
****************/
static bool mm_initialized = false;
static uint8_t *mm_data = NULL;
static size_t mm_size = 0;
static AUDIOSTREAM *mm_stream = NULL;
static size_t mm_buffer_size = 0;

/*********************
** static functions **
*********************/
static void f_MicromodDeInit(js_State *J) {
    if (mm_initialized) {
        if (mm_stream) {
            stop_audio_stream(mm_stream);
            mm_stream = NULL;
        }

        if (mm_data) {
            free(mm_data);
            mm_data = NULL;
        }
        mm_initialized = false;
    }
}

/**
 * @brief open a audio file.
 * Micromod_Init(filename:str[, samplerate:number[, buffer_size:number]])
 *
 * @param J VM state.
 */
static void f_MicromodInit(js_State *J) {
    if (mm_initialized) {
        js_error(J, "Micromod is already initialized");
        return;
    }

    int32_t samplerate = 22000;
    if (js_isnumber(J, 2)) {
        samplerate = js_toint32(J, 2);
    }

    if (samplerate <= 0) {
        js_error(J, "Samplerate can't be <= 8000: '%ld'", samplerate);
        return;
    }

    const char *fname = js_tostring(J, 1);
    char *delim = strchr(fname, ZIP_DELIM);
    if (!delim) {
        if (!ut_read_file(fname, (void **)&mm_data, &mm_size)) {
            js_error(J, "Could not open '%s'", fname);
            return;
        }
    } else {
        if (!read_zipfile1(fname, (void **)&mm_data, &mm_size)) {
            js_error(J, "Could not open '%s'", fname);
            return;
        }
    }

    if (micromod_initialise((signed char *)mm_data, samplerate) != 0) {
        f_MicromodDeInit(J);
        js_error(J, "Could not open '%s': unsupported format", fname);
        return;
    }

    micromod_mute_channel(-1);

    if (js_isnumber(J, 3)) {
        mm_buffer_size = js_touint32(J, 3);
    } else {
        mm_buffer_size = 1024 * 4;
    }

    // allocate stream
    mm_stream = play_audio_stream(mm_buffer_size, 16, true, samplerate, 255, 128);
    if (!mm_stream) {
        f_MicromodDeInit(J);
        JS_ENOMEM(J);
        return;
    }
    mm_initialized = true;
}

/**
 * @brief rewind MOD
 *
 * @param J VM state.
 */
static void f_MicromodRewind(js_State *J) {
    if (!mm_initialized) {
        js_error(J, "Not initialized");
    } else {
        micromod_set_position(0);
    }
}

/**
 * @brief play stream
 *
 * @param J VM state.
 */
static void f_MicromodPlay(js_State *J) {
    if (!mm_initialized) {
        js_error(J, "Not initialized");
        return;
    }

    while (true) {
        short *mem_chunk = get_audio_stream_buffer(mm_stream);
        if (mem_chunk != NULL) {
            bzero(mem_chunk, mm_buffer_size * 2 * 2);  // clear whole stereo buffer with 16bit samples
            micromod_get_audio(mem_chunk, mm_buffer_size);
            for (int i = 0; i < mm_buffer_size * 2; i++) {
                mem_chunk[i] = mem_chunk[i] ^ 0x8000;
            }
            free_audio_stream_buffer(mm_stream);
        } else {
            break;
        }
    }
}

/*********************
** public functions **
*********************/
/**
 * @brief initialize neural subsystem.
 *
 * @param J VM state.
 */
void init_micromod(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    NFUNCDEF(J, MicromodInit, 3);
    NFUNCDEF(J, MicromodDeInit, 0);
    NFUNCDEF(J, MicromodRewind, 0);
    NFUNCDEF(J, MicromodPlay, 0);
}
