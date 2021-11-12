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
#include <errno.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "DOjS.h"
#include "util.h"
#include "zipfile.h"

void init_rawplay(js_State *J);

/************
** defines **
************/
#define TAG_RAWPLAY "Rawplay"  //!< pointer tag

/************
** structs **
************/
//! file userdata definition
typedef struct __rawplay {
    unsigned short *data;
    size_t size;
    size_t pos;
    AUDIOSTREAM *stream;
    size_t buffer_size;
} rawplay_t;

/*********************
** static functions **
*********************/

static void RP_cleanup(rawplay_t *ov) {
    if (ov->data) {
        stop_audio_stream(ov->stream);
        ov->stream = NULL;
        free(ov->data);
        ov->data = NULL;
    }
}

/**
 * @brief finalize a file and free resources.
 *
 * @param J VM state.
 */
static void RP_Finalize(js_State *J, void *data) {
    rawplay_t *ov = (rawplay_t *)data;
    RP_cleanup(ov);
    free(ov);
}

/**
 * @brief open a audio file.
 * ov = new Rawplay(filename:str, samplerate:number, buffer_size:number)
 *
 * @param J VM state.
 */
static void new_Rawplay(js_State *J) {
    NEW_OBJECT_PREP(J);

    rawplay_t *ov = malloc(sizeof(rawplay_t));
    if (!ov) {
        JS_ENOMEM(J);
        return;
    }
    bzero(ov, sizeof(rawplay_t));

    const char *fname = js_tostring(J, 1);
    int32_t samplerate = js_toint32(J, 2);

    if (samplerate <= 0) {
        js_error(J, "Samplerate can't be <= 0: '%ld'", samplerate);
        return;
    }

    char *delim = strchr(fname, ZIP_DELIM);
    if (!delim) {
        if (!ut_read_file(fname, (void **)&ov->data, &ov->size)) {
            free(ov);
            js_error(J, "Could not open '%s'", fname);
            return;
        }
    } else {
        if (!read_zipfile1(fname, (void **)&ov->data, &ov->size)) {
            free(ov);
            js_error(J, "Could not open '%s'", fname);
            return;
        }
    }
    if (ov->size % 4 != 0) {
        free(ov->data);
        free(ov);
        js_error(J, "Size not multiple of 4");
        return;
    }
    ov->size = ov->size / 4;

    if (js_isnumber(J, 3)) {
        ov->buffer_size = js_touint32(J, 3);
    } else {
        ov->buffer_size = 1024 * 4;
    }

    // allocate stream
    ov->stream = play_audio_stream(ov->buffer_size, 16, true, samplerate, 255, 128);
    if (!ov->stream) {
        free(ov->data);
        free(ov);
        JS_ENOMEM(J);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_RAWPLAY, ov, RP_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->size);
    js_defproperty(J, -2, "length", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, samplerate);
    js_defproperty(J, -2, "samplerate", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->buffer_size);
    js_defproperty(J, -2, "buffersize", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief close VORBIS.
 * ov.Close()
 *
 * @param J VM state.
 */
static void RP_Close(js_State *J) {
    rawplay_t *ov = js_touserdata(J, 0, TAG_RAWPLAY);
    RP_cleanup(ov);
}

/**
 * @brief get current play time
 *
 * @param J VM state.
 */
static void RP_CurrentSample(js_State *J) {
    rawplay_t *ov = js_touserdata(J, 0, TAG_RAWPLAY);
    if (!ov->data) {
        js_error(J, "Rawplay is closed");
        return;
    }

    js_pushnumber(J, ov->pos);
}

/**
 * @brief rewind stream
 *
 * @param J VM state.
 */
static void RP_Rewind(js_State *J) {
    rawplay_t *ov = js_touserdata(J, 0, TAG_RAWPLAY);
    if (!ov->data) {
        js_error(J, "Rawplay is closed");
        return;
    }

    ov->pos = 0;
}

/**
 * @brief play stream
 *
 * @param J VM state.
 */
static void RP_Play(js_State *J) {
    rawplay_t *ov = js_touserdata(J, 0, TAG_RAWPLAY);
    if (!ov->data) {
        js_error(J, "Rawplay is closed");
        return;
    }

    bool left = js_toboolean(J, 1);
    bool right = js_toboolean(J, 2);

    short *mem_chunk = get_audio_stream_buffer(ov->stream);
    if (mem_chunk != NULL) {
        int idx = 0;
        for (int i = 0; i < ov->buffer_size; i++) {
            if (ov->pos < ov->size) {
                if (left) {
                    mem_chunk[idx] = ov->data[ov->pos] ^ 0x8000;
                } else {
                    mem_chunk[idx] = 0x00;
                }
                if (right) {
                    mem_chunk[idx + 1] = ov->data[ov->pos] ^ 0x8000;
                } else {
                    mem_chunk[idx + 1] = 0x00;
                }
                ov->pos++;
            } else {
                mem_chunk[idx + 1] = mem_chunk[idx] = 0;
            }
            idx += 2;
        }
        free_audio_stream_buffer(ov->stream);
    }
}

/**
 * @brief move to specified index
 *
 * @param J VM state.
 */
static void RP_Seek(js_State *J) {
    rawplay_t *ov = js_touserdata(J, 0, TAG_RAWPLAY);
    if (!ov->data) {
        js_error(J, "Rawplay is closed");
        return;
    }

    int32_t idx = js_toint32(J, 1);
    if (idx >= 0 && idx < ov->size) {
        ov->pos = idx;
    } else {
        js_error(J, "Index out of range: %ld", idx);
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
void init_rawplay(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, RP, Close, 0);
        NPROTDEF(J, RP, CurrentSample, 0);
        NPROTDEF(J, RP, Rewind, 0);
        NPROTDEF(J, RP, Play, 2);
        NPROTDEF(J, RP, Seek, 1);
    }
    CTORDEF(J, new_Rawplay, TAG_RAWPLAY, 2);
}
