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

#include "DOjS.h"

#define PLM_AUDIO_SEPARATE_CHANNELS
#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg/pl_mpeg.h"

void init_mpeg1(js_State *J);

/************
** defines **
************/
#define MPEG1_NUM_SEGMENTS 16

#define TAG_MPEG1 "MPEG1"  //!< pointer tag

/************
** structs **
************/
//! file userdata definition
typedef struct __mpeg1 {
    plm_t *plm;               //!< mpeg pointer
    uint16_t x;               //!< x start coordinate
    uint16_t y;               //!< y start coordinate
    unsigned long last_call;  //!< time of last call
    BITMAP *video_buffer;     //!< color conversion buffer
    bool do_sound;            //!< do audio decoding?
    SAMPLE *stream;
    int voice;
    int segment;
} mpeg1_t;

/*********************
** static functions **
*********************/

/**
 * @brief called for audio playback
 *
 * @param self the MPEG data
 * @param samples sound data
 * @param user the MPEG playback struct
 */
static void MPEG1_audio_decode_callback(plm_t *self, plm_samples_t *samples, void *user) {
    mpeg1_t *m = (mpeg1_t *)user;

    if (!m->do_sound) {
        return;
    }

    // get current buffer part, copy data and switch to next buffer part
    uint16_t *buf = &(((uint16_t *)m->stream->data)[PLM_AUDIO_SAMPLES_PER_FRAME * m->segment]);

    for (int i = 0; i < PLM_AUDIO_SAMPLES_PER_FRAME; i++) {
        buf[i] = ((signed short)(samples->left[i] + samples->left[i] * (0x7FFF / 2))) ^ 0x8000;
    }

    m->segment = (m->segment + 1) % MPEG1_NUM_SEGMENTS;

    if (voice_get_position(m->voice) == -1) {
        voice_start(m->voice);
    }
}

/**
 * @brief called for every video frame
 *
 * @param self the MPEG data
 * @param frame frame data
 * @param user the MPEG playback struct
 */
static void MPEG1_video_decode_callback(plm_t *self, plm_frame_t *frame, void *user) {
    mpeg1_t *m = (mpeg1_t *)user;

    plm_frame_to_rgba(frame, (uint8_t *)m->video_buffer->dat, frame->width * sizeof(uint32_t));

    blit(m->video_buffer, DOjS.current_bm, 0, 0, m->x, m->y, frame->width, frame->height);
}

/**
 * @brief free ressources
 *
 * @param m the MPEG playback struct
 */
static void MPEG1_cleanup(mpeg1_t *m) {
    if (m->plm) {
        voice_stop(m->voice);
        deallocate_voice(m->voice);
        destroy_sample(m->stream);
        m->stream = NULL;
        destroy_bitmap(m->video_buffer);
        m->video_buffer = NULL;
        plm_destroy(m->plm);
        m->plm = NULL;
    }
}

/**
 * @brief fill stream buffer with silence.
 *
 * @param m the MPEG playback struct
 */
static void MPEG1_silence(mpeg1_t *m) {
    uint16_t *p = m->stream->data;
    for (int i = 0; i < PLM_AUDIO_SAMPLES_PER_FRAME * MPEG1_NUM_SEGMENTS; i++) {
        p[i] = 0x8000;
    }
}

/**
 * @brief finalize a file and free resources.
 *
 * @param J VM state.
 */
static void MPEG1_Finalize(js_State *J, void *data) {
    mpeg1_t *m = (mpeg1_t *)data;
    MPEG1_cleanup(m);
    free(m);
}

/**
 * @brief open a MPEG1.
 * m = new MPEG1(filename:str, play_sound:bool)
 *
 * @param J VM state.
 */
static void new_MPEG1(js_State *J) {
    NEW_OBJECT_PREP(J);

    mpeg1_t *m = calloc(1, sizeof(mpeg1_t));
    if (!m) {
        JS_ENOMEM(J);
        return;
    }
    bzero(m, sizeof(mpeg1_t));

    const char *fname = js_tostring(J, 1);
    m->plm = plm_create_with_filename(fname);
    if (!m->plm) {
        free(m);
        js_error(J, "Could not open '%s'", fname);
        return;
    }

    if (!plm_has_headers(m->plm)) {
        free(m);
        js_error(J, "MPEG1 header not found in '%s'", fname);
        return;
    }

    // get metadata
    int width = plm_get_width(m->plm);
    int height = plm_get_height(m->plm);
    int samplerate = plm_get_samplerate(m->plm);

    // allocate buffer for RGBA decoding
    m->video_buffer = create_bitmap_ex(32, width, height);
    if (!m->video_buffer) {
        plm_destroy(m->plm);
        free(m);
        JS_ENOMEM(J);
        return;
    }

    // allocate buffer for audio (multiple segments of "audio per frame" length)
    m->stream = create_sample(16, false, samplerate, PLM_AUDIO_SAMPLES_PER_FRAME * MPEG1_NUM_SEGMENTS);
    if (!m->stream) {
        free(m->video_buffer);
        plm_destroy(m->plm);
        free(m);
        JS_ENOMEM(J);
        return;
    }

    // fill with silence
    MPEG1_silence(m);

    // play the sample in looped mode
    m->voice = allocate_voice(m->stream);
    if (m->voice < 0) {
        destroy_sample(m->stream);
        free(m->video_buffer);
        plm_destroy(m->plm);
        free(m);
        JS_ENOMEM(J);
        return;
    }

    voice_set_playmode(m->voice, PLAYMODE_LOOP);
    voice_set_volume(m->voice, 255);
    voice_set_pan(m->voice, 128);

    // set lead time to buffer size
    plm_set_audio_lead_time(m->plm, (double)PLM_AUDIO_SAMPLES_PER_FRAME * MPEG1_NUM_SEGMENTS / (double)samplerate);

    // set callback functions
    plm_set_video_decode_callback(m->plm, MPEG1_video_decode_callback, m);
    plm_set_audio_decode_callback(m->plm, MPEG1_audio_decode_callback, m);

    m->do_sound = DOjS.sound_available && js_toboolean(J, 2);

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_MPEG1, m, MPEG1_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, width);
    js_defproperty(J, -2, "width", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, height);
    js_defproperty(J, -2, "height", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, plm_get_framerate(m->plm));
    js_defproperty(J, -2, "framerate", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, samplerate);
    js_defproperty(J, -2, "samplerate", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, plm_get_duration(m->plm));
    js_defproperty(J, -2, "duration", JS_READONLY | JS_DONTCONF);

    js_pushboolean(J, plm_get_num_video_streams(m->plm));
    js_defproperty(J, -2, "has_video", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief close MPEG1.
 * m.Close()
 *
 * @param J VM state.
 */
static void MPEG1_Close(js_State *J) {
    mpeg1_t *m = js_touserdata(J, 0, TAG_MPEG1);
    MPEG1_cleanup(m);
}

/**
 * @brief rewind to start of video
 * m.Rewind()
 *
 * @param J VM state.
 */
static void MPEG1_Rewind(js_State *J) {
    mpeg1_t *m = js_touserdata(J, 0, TAG_MPEG1);

    if (!m->plm) {
        js_error(J, "MPEG1 is closed");
        return;
    }

    MPEG1_silence(m);
    plm_rewind(m->plm);
    m->last_call = 0;
}

/**
 * @brief check if end of video was reached
 * m.HasEnded():bool
 *
 * @param J VM state.
 */
static void MPEG1_HasEnded(js_State *J) {
    mpeg1_t *m = js_touserdata(J, 0, TAG_MPEG1);

    if (!m->plm) {
        js_error(J, "MPEG1 is closed");
        return;
    }

    js_pushboolean(J, plm_has_ended(m->plm));
}

/**
 * @brief play a video at position x, y. videos are rendered directly to the screen, there is no direct access to the pixels of a video.
 * m.Play(x:number, y:number)
 *
 * @param J VM state.
 */
static void MPEG1_Play(js_State *J) {
    mpeg1_t *m = js_touserdata(J, 0, TAG_MPEG1);
    if (!m->plm) {
        js_error(J, "MPEG1 is closed");
        return;
    }

    if (m->last_call == 0) {
        m->last_call = DOjS.sys_ticks;
        return;
    }

    // remember play position
    m->x = js_toint16(J, 1);
    m->y = js_toint16(J, 2);

    // calculate time delta and decode
    double time = (DOjS.sys_ticks - m->last_call) / 1000.0;
    m->last_call = DOjS.sys_ticks;
    plm_decode(m->plm, time);

    // silence audio if end of video
    if (plm_has_ended(m->plm)) {
        MPEG1_silence(m);
    }
}

/**
 * @brief seek to specific time index.
 *
 * @param J VM state.
 */
static void MPEG1_Seek(js_State *J) {
    mpeg1_t *m = js_touserdata(J, 0, TAG_MPEG1);
    if (!m->plm) {
        js_error(J, "MPEG1 is closed");
        return;
    }

    if (!plm_seek(m->plm, js_tonumber(J, 1), true)) {
        js_error(J, "Seek failed");
        return;
    }
}

/**
 * @brief get current play time
 *
 * @param J VM state.
 */
static void MPEG1_CurrentTime(js_State *J) {
    mpeg1_t *m = js_touserdata(J, 0, TAG_MPEG1);
    if (!m->plm) {
        js_error(J, "MPEG1 is closed");
        return;
    }

    js_pushnumber(J, plm_get_time(m->plm));
}

/*********************
** public functions **
*********************/
/**
 * @brief initialize neural subsystem.
 *
 * @param J VM state.
 */
void init_mpeg1(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, MPEG1, Close, 0);
        NPROTDEF(J, MPEG1, Rewind, 0);
        NPROTDEF(J, MPEG1, HasEnded, 0);
        NPROTDEF(J, MPEG1, Play, 2);
        NPROTDEF(J, MPEG1, Seek, 1);
        NPROTDEF(J, MPEG1, CurrentTime, 0);
    }
    CTORDEF(J, new_MPEG1, TAG_MPEG1, 2);
}
