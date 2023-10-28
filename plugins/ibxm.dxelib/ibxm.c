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

TODO: vorbis playback only works with mono downmixing.
If I request 2 channels using stb_vorbis_get_samples_short_interleaved() the system crashes.
I'll keep it that way for now.
*/

#include <errno.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "DOjS.h"
#include "util.h"
#include "zipfile.h"

#include "ibxm-ac/ibxm.h"

void init_ibxm(js_State *J);

/************
** defines **
************/
#define TAG_IBXM "IBXM"  //!< pointer tag

#define TICK_FACTOR 8  //!< number of decoding steps to do per call to play

/************
** structs **
************/
//! userdata definition
typedef struct {
    struct data moddata;
    struct module *mod;
    struct replay *replay;
    AUDIOSTREAM *stream;
    size_t tick_size;
    int *mix_buffer;

    char message[64];
} ibxm_t;

/*********************
** static functions **
*********************/

/**
 * @brief free ressources.
 *
 * @param ov the ibxm_t with the ressources to free.
 */
static void ibxm_cleanup(ibxm_t *ov) {
    if (ov->stream) {
        stop_audio_stream(ov->stream);
        ov->stream = NULL;
    }
    if (ov->replay) {
        dispose_replay(ov->replay);
        ov->replay = NULL;
    }
    if (ov->mod) {
        dispose_module(ov->mod);
        ov->mod = NULL;
    }
    if (ov->mix_buffer) {
        free(ov->mix_buffer);
        ov->mix_buffer = NULL;
    }
    if (ov->moddata.buffer) {
        free(ov->moddata.buffer);
        ov->moddata.buffer = NULL;
    }
}

/**
 * @brief finalize a file and free resources.
 *
 * @param J VM state.
 */
static void ibxm_Finalize(js_State *J, void *data) {
    ibxm_t *ov = (ibxm_t *)data;
    ibxm_cleanup(ov);
    free(ov);
}

/**
 * @brief open a MOD.
 * ov = new Ibxm(filename:str[, samplerate:number])
 *
 * @param J VM state.
 */
static void new_Ibxm(js_State *J) {
    NEW_OBJECT_PREP(J);

    ibxm_t *ov = calloc(1, sizeof(ibxm_t));
    if (!ov) {
        JS_ENOMEM(J);
        return;
    }

    uint32_t samplerate = 22000;
    if (js_isnumber(J, 2)) {
        samplerate = js_touint32(J, 2);
    }

    size_t size;
    const char *fname = js_tostring(J, 1);
    char *delim = strchr(fname, ZIP_DELIM);
    if (!delim) {
        if (!ut_read_file(fname, (void **)&ov->moddata.buffer, &size)) {
            js_error(J, "Could not open '%s'", fname);
            ibxm_Finalize(J, ov);
            return;
        }
    } else {
        if (!read_zipfile1(fname, (void **)&ov->moddata.buffer, &size)) {
            js_error(J, "Could not open '%s'", fname);
            ibxm_Finalize(J, ov);
            return;
        }
    }
    ov->moddata.length = size;

    // init module
    ov->mod = module_load(&ov->moddata, ov->message);
    if (!ov->mod) {
        js_error(J, "Could not open '%s': %s", fname, ov->message);
        ibxm_Finalize(J, ov);
        return;
    }

    // init replay
    ov->replay = new_replay(ov->mod, samplerate, 0);
    if (!ov->mod) {
        JS_ENOMEM(J);
        ibxm_Finalize(J, ov);
        return;
    }

    // calculate size of single decoding tick
    ov->tick_size = calculate_tick_size(ov->replay);

    // allocate mixbuffer
    ov->mix_buffer = calloc(sizeof(int), calculate_mix_buf_len(samplerate));
    if (!ov->mix_buffer) {
        JS_ENOMEM(J);
        ibxm_Finalize(J, ov);
        return;
    }

    // allocate stream
    ov->stream = play_audio_stream(ov->tick_size * TICK_FACTOR, 16, true, samplerate, 255, 128);
    if (!ov->stream) {
        JS_ENOMEM(J);
        ibxm_Finalize(J, ov);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_IBXM, ov, ibxm_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->mod->num_channels);
    js_defproperty(J, -2, "channels", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->mod->num_instruments);
    js_defproperty(J, -2, "instruments", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->mod->num_patterns);
    js_defproperty(J, -2, "patterns", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, samplerate);
    js_defproperty(J, -2, "samplerate", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, replay_calculate_duration(ov->replay));
    js_defproperty(J, -2, "duration", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->tick_size);
    js_defproperty(J, -2, "ticksize", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief close ibxm.
 * ov.Close()
 *
 * @param J VM state.
 */
static void ibxm_Close(js_State *J) {
    ibxm_t *ov = js_touserdata(J, 0, TAG_IBXM);
    ibxm_cleanup(ov);
}

/**
 * @brief rewind stream
 *
 * @param J VM state.
 */
static void ibxm_Rewind(js_State *J) {
    ibxm_t *ov = js_touserdata(J, 0, TAG_IBXM);
    if (!ov->mod) {
        js_error(J, "IBXM is closed");
        return;
    }

    replay_seek(ov->replay, 0);
}

/**
 * @brief play stream
 *
 * @param J VM state.
 */
static void ibxm_Play(js_State *J) {
    ibxm_t *ov = js_touserdata(J, 0, TAG_IBXM);
    if (!ov->mod) {
        js_error(J, "IBXM is closed");
        return;
    }

    while (true) {
        short *mem_chunk = get_audio_stream_buffer(ov->stream);
        if (mem_chunk != NULL) {
            for (int i = 0; i < TICK_FACTOR; i++) {
                int offset = i * ov->tick_size * 2;
                int samples = replay_get_audio(ov->replay, ov->mix_buffer, 0);
                for (int idx = 0; idx < samples * 2; idx++) {
                    int ampl = ov->mix_buffer[idx];
                    if (ampl > 32767) {
                        ampl = 32767;
                    }
                    if (ampl < -32768) {
                        ampl = -32768;
                    }

                    mem_chunk[offset + idx] = ((short)ampl) ^ 0x8000;
                }
            }
            free_audio_stream_buffer(ov->stream);
        } else {
            break;
        }
    }
}

/**
 * @brief move to specified sample index
 *
 * @param J VM state.
 */
static void ibxm_Seek(js_State *J) {
    ibxm_t *ov = js_touserdata(J, 0, TAG_IBXM);
    if (!ov->mod) {
        js_error(J, "IBXM is closed");
        return;
    }

    int32_t idx = js_toint32(J, 1);

    if (idx >= 0 && idx < replay_calculate_duration(ov->replay)) {
        replay_seek(ov->replay, idx);
    } else {
        js_error(J, "Index out of range: %ld", idx);
    }
}

/*********************
** public functions **
*********************/
/**
 * @brief initialize subsystem.
 *
 * @param J VM state.
 */
void init_ibxm(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, ibxm, Close, 0);
        NPROTDEF(J, ibxm, Rewind, 0);
        NPROTDEF(J, ibxm, Play, 0);
        NPROTDEF(J, ibxm, Seek, 1);
    }
    CTORDEF(J, new_Ibxm, TAG_IBXM, 2);
}
