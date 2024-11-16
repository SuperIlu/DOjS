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

// #define STB_VORBIS_MAX_CHANNELS 2
#define STB_VORBIS_NO_PUSHDATA_API
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

void init_vorbis(js_State *J);

/************
** defines **
************/
#define TAG_VORBIS "Ogg"  //!< pointer tag

/************
** structs **
************/
//! file userdata definition
typedef struct __vorbis {
    stb_vorbis *ogg;  //!< ogg pointer
    AUDIOSTREAM *stream;
    uint32_t buffer_size;
} vorbis_t;

/*********************
** static functions **
*********************/

/**
 * @brief free ressources.
 *
 * @param ov the vorbis_t with the ressources to free.
 */
static void VORBIS_cleanup(vorbis_t *ov) {
    if (ov->ogg) {
        stop_audio_stream(ov->stream);
        ov->stream = NULL;
        stb_vorbis_close(ov->ogg);
        ov->ogg = NULL;
    }
}

/**
 * @brief finalize a file and free resources.
 *
 * @param J VM state.
 */
static void VORBIS_Finalize(js_State *J, void *data) {
    vorbis_t *ov = (vorbis_t *)data;
    VORBIS_cleanup(ov);
    free(ov);
}

/**
 * @brief open a Ogg.
 * ov = new Ogg(filename:str)
 *
 * @param J VM state.
 */
static void new_Ogg(js_State *J) {
    NEW_OBJECT_PREP(J);

    vorbis_t *ov = calloc(1, sizeof(vorbis_t));
    if (!ov) {
        JS_ENOMEM(J);
        return;
    }
    memset(ov, 0, sizeof(vorbis_t));

    int err;
    const char *fname = js_tostring(J, 1);
    ov->ogg = stb_vorbis_open_filename(fname, &err, NULL);
    if (!ov->ogg) {
        free(ov);
        js_error(J, "Could not open '%s'", fname);
        return;
    }

    if (js_isnumber(J, 2)) {
        ov->buffer_size = js_touint32(J, 2);
    } else {
        ov->buffer_size = 1024 * 16;
    }

    // get metadata
    stb_vorbis_info info = stb_vorbis_get_info(ov->ogg);
    stb_vorbis_comment comment = stb_vorbis_get_comment(ov->ogg);

    // allocate stream
    ov->stream = play_audio_stream(ov->buffer_size, 16, false, info.sample_rate, 255, 128);
    if (!ov->stream) {
        stb_vorbis_close(ov->ogg);
        free(ov);
        JS_ENOMEM(J);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_VORBIS, ov, VORBIS_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, info.channels);
    js_defproperty(J, -2, "channels", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, stb_vorbis_stream_length_in_samples(ov->ogg));
    js_defproperty(J, -2, "numsamples", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, stb_vorbis_stream_length_in_seconds(ov->ogg));
    js_defproperty(J, -2, "duration", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, info.sample_rate);
    js_defproperty(J, -2, "samplerate", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, info.max_frame_size);
    js_defproperty(J, -2, "maxframesize", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->buffer_size);
    js_defproperty(J, -2, "buffersize", JS_READONLY | JS_DONTCONF);

    js_pushstring(J, comment.vendor);
    js_defproperty(J, -2, "vendor", JS_READONLY | JS_DONTCONF);

    js_newarray(J);
    for (int i = 0; i < comment.comment_list_length; i++) {
        js_pushstring(J, comment.comment_list[i]);
        js_setindex(J, -2, i);
    }
    js_defproperty(J, -2, "comments", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief close VORBIS.
 * ov.Close()
 *
 * @param J VM state.
 */
static void VORBIS_Close(js_State *J) {
    vorbis_t *ov = js_touserdata(J, 0, TAG_VORBIS);
    VORBIS_cleanup(ov);
}

/**
 * @brief get current play time
 *
 * @param J VM state.
 */
static void VORBIS_CurrentSample(js_State *J) {
    vorbis_t *ov = js_touserdata(J, 0, TAG_VORBIS);
    if (!ov->ogg) {
        js_error(J, "OGG is closed");
        return;
    }

    js_pushnumber(J, stb_vorbis_get_sample_offset(ov->ogg));
}

/**
 * @brief rewind stream
 *
 * @param J VM state.
 */
static void VORBIS_Rewind(js_State *J) {
    vorbis_t *ov = js_touserdata(J, 0, TAG_VORBIS);
    if (!ov->ogg) {
        js_error(J, "OGG is closed");
        return;
    }

    stb_vorbis_seek_start(ov->ogg);
}

/**
 * @brief play stream
 *
 * @param J VM state.
 */
static void VORBIS_Play(js_State *J) {
    vorbis_t *ov = js_touserdata(J, 0, TAG_VORBIS);
    if (!ov->ogg) {
        js_error(J, "OGG is closed");
        return;
    }

    while (true) {
        short *mem_chunk = get_audio_stream_buffer(ov->stream);
        if (mem_chunk != NULL) {
            int num_decoded = stb_vorbis_get_samples_short(ov->ogg, 1, &mem_chunk, ov->buffer_size);
            for (int i = 0; i < ov->buffer_size; i++) {
                if (i < num_decoded) {
                    mem_chunk[i] = mem_chunk[i] ^ 0x8000;
                } else {
                    mem_chunk[i] = 0x00;
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
static void VORBIS_Seek(js_State *J) {
    vorbis_t *ov = js_touserdata(J, 0, TAG_VORBIS);
    if (!ov->ogg) {
        js_error(J, "OGG is closed");
        return;
    }

    int32_t idx = js_toint32(J, 1);

    if (idx >= 0 && idx < stb_vorbis_stream_length_in_samples(ov->ogg)) {
        stb_vorbis_seek(ov->ogg, idx);
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
void init_vorbis(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, VORBIS, Close, 0);
        NPROTDEF(J, VORBIS, CurrentSample, 0);
        NPROTDEF(J, VORBIS, Rewind, 0);
        NPROTDEF(J, VORBIS, Play, 0);
        NPROTDEF(J, VORBIS, Seek, 1);
    }
    CTORDEF(J, new_Ogg, TAG_VORBIS, 2);
}
