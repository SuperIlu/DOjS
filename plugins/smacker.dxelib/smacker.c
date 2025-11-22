/*
MIT License

Copyright (c) 2019-2025 Andre Seidelt <superilu@yahoo.com>

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

#include "smacker.h"

void init_smacker(js_State *J);

/************
** defines **
************/
#define TAG_SMACKER "SMACKER"  //!< pointer tag
#define NUM_AUDIO_CHANNELS 7   //!< max number of audio channels
#define NUM_AUDIO_SEGMENTS 16  //!< number of chunks in playback buffer
#define SAMPLE_BUFFER_SIZE (NUM_AUDIO_SEGMENTS * 1024 * 16)

/************
** structs **
************/
//! file userdata definition
typedef struct __smk {
    smk smk;               //!< smk data
    unsigned long width;   //!< w
    unsigned long height;  //!< h
    BITMAP *video_buffer;  //!< color conversion buffer
    SAMPLE *stream[NUM_AUDIO_CHANNELS];
    int voice[NUM_AUDIO_CHANNELS];
    int offset[NUM_AUDIO_CHANNELS];
    bool do_sound;  //!< do audio decoding?
} smk_t;

/*********************
** static functions **
*********************/

/**
 * @brief free ressources
 *
 * @param s the SMACKER playback struct
 */
static void SMACKER_cleanup(smk_t *s) {
    if (s->smk) {
        for (int i = 0; i < NUM_AUDIO_CHANNELS; i++) {
            if (s->voice[i] != -1) {
                voice_stop(s->voice[i]);
                deallocate_voice(s->voice[i]);
            }
            if (s->stream[i]) {
                destroy_sample(s->stream[i]);
            }
        }

        destroy_bitmap(s->video_buffer);
        s->video_buffer = NULL;

        smk_close(s->smk);
        s->smk = NULL;
    }
}

/**
 * @brief finalize a file and free resources.
 *
 * @param J VM state.
 */
static void SMACKER_Finalize(js_State *J, void *data) {
    smk_t *s = (smk_t *)data;
    SMACKER_cleanup(s);
    free(s);
}

/**
 * @brief open a SMK.
 * s = new SMACKER(filename:str, play_sound:bool)
 *
 * @param J VM state.
 */
static void new_SMACKER(js_State *J) {
    NEW_OBJECT_PREP(J);

    smk_t *s = calloc(1, sizeof(smk_t));
    if (!s) {
        JS_ENOMEM(J);
        return;
    }
    memset(s, 0, sizeof(smk_t));
    for (int i = 0; i < NUM_AUDIO_CHANNELS; i++) {
        s->voice[i] = -1;
    }

    const char *fname = js_tostring(J, 1);
    s->smk = smk_open_file(fname, SMK_MODE_DISK);
    if (!s->smk) {
        free(s);
        js_error(J, "Could not open '%s'", fname);
        return;
    }

    s->do_sound = DOjS.sound_available && js_toboolean(J, 2);

    // get metadata
    double usf;
    unsigned long num_frames;
    smk_info_all(s->smk, NULL, &num_frames, &usf);
    double fps = 1000000.0 / usf;

    smk_info_video(s->smk, &s->width, &s->height, NULL);

    unsigned char a_trackmask;
    unsigned char a_channels[NUM_AUDIO_CHANNELS];
    unsigned char a_depth[NUM_AUDIO_CHANNELS];
    unsigned long a_rate[NUM_AUDIO_CHANNELS];
    smk_info_audio(s->smk, &a_trackmask, a_channels, a_depth, a_rate);
    if (s->do_sound) {
        for (int i = 0; i < NUM_AUDIO_CHANNELS; i++) {
            if (a_trackmask & (1 << i)) {
                LOGF("#%d: channels=%d, depth=%d, rate=%ld\n", i, a_channels[i], a_depth[i], a_rate[i]);
                smk_enable_audio(s->smk, i, true);

                // allocate buffer for audio (multiple segments of "audio per frame" length)
                s->stream[i] = create_sample(a_depth[i], a_channels[i] > 1, a_rate[i], SAMPLE_BUFFER_SIZE);
                if (!s->stream[i]) {
                    SMACKER_cleanup(s);
                    free(s);
                    JS_ENOMEM(J);
                    return;
                }

                // play the sample in looped mode
                s->voice[i] = allocate_voice(s->stream[i]);
                if (s->voice[i] < 0) {
                    SMACKER_cleanup(s);
                    free(s);
                    JS_ENOMEM(J);
                    return;
                }

                voice_set_playmode(s->voice[i], PLAYMODE_LOOP);
                voice_set_volume(s->voice[i], 255);
                voice_set_pan(s->voice[i], 128);
            } else {
                s->stream[i] = NULL;
                s->voice[i] = -1;
            }
        }
    }

    // process first frame
    smk_enable_video(s->smk, true);
    smk_first(s->smk);

    // allocate buffer for RGBA decoding
    s->video_buffer = create_bitmap_ex(32, s->width, s->height);
    if (!s->video_buffer) {
        smk_close(s->smk);
        free(s);
        JS_ENOMEM(J);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_SMACKER, s, SMACKER_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, s->width);
    js_defproperty(J, -2, "width", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, s->height);
    js_defproperty(J, -2, "height", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, fps);
    js_defproperty(J, -2, "framerate", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, usf);
    js_defproperty(J, -2, "frame_duration", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, num_frames);
    js_defproperty(J, -2, "num_frames", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief close SMACKER.
 * s.Close()
 *
 * @param J VM state.
 */
static void SMACKER_Close(js_State *J) {
    smk_t *s = js_touserdata(J, 0, TAG_SMACKER);
    SMACKER_cleanup(s);
}

/**
 * @brief rewind to start of video
 * s.Rewind()
 *
 * @param J VM state.
 */
static void SMACKER_Rewind(js_State *J) {
    smk_t *s = js_touserdata(J, 0, TAG_SMACKER);

    if (!s->smk) {
        js_error(J, "SMK is closed");
        return;
    }

    smk_first(s->smk);
}

/**
 * @brief play a video at position x, y. videos are rendered directly to the screen, there is no direct access to the pixels of a video.
 * m.Play(x:number, y:number)
 *
 * @param J VM state.
 */
static void SMACKER_Play(js_State *J) {
    smk_t *s = js_touserdata(J, 0, TAG_SMACKER);
    if (!s->smk) {
        js_error(J, "SMK is closed");
        return;
    }

    // play position
    int x = js_toint16(J, 1);
    int y = js_toint16(J, 2);

    unsigned long cur_frame, num_frames;
    smk_info_all(s->smk, &cur_frame, &num_frames, NULL);

    // get next video frame
    smk_next(s->smk);
    const unsigned char *palette_data = smk_get_palette(s->smk);
    const unsigned char *image_data = smk_get_video(s->smk);

    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x++) {
            int pal_idx = image_data[x + s->width * y] * 3;
            int red = palette_data[pal_idx + 0];
            int grn = palette_data[pal_idx + 1];
            int blu = palette_data[pal_idx + 2];
            uint32_t argb = 0xFF000000 | (red << 16) | (grn << 8) | blu;
            putpixel(s->video_buffer, x, y, argb);
        }
    }

    // blit data to screen
    blit(s->video_buffer, DOjS.current_bm, 0, 0, x, y, s->width, s->height);

    // get next audio frame
    if (s->do_sound) {
        for (int i = 0; i < NUM_AUDIO_CHANNELS; i++) {
            if (s->stream[i]) {
                const unsigned char *a_data = smk_get_audio(s->smk, i);
                unsigned long a_len = smk_get_audio_size(s->smk, i);

                if (a_len > 0) {
                    for (int a = 0; a < a_len; a++) {
                        uint8_t *ptr = s->stream[i]->data;
                        ptr[s->offset[i]] = a_data[a];
                        s->offset[i] = (s->offset[i] + 1) % SAMPLE_BUFFER_SIZE;
                    }

                    if (voice_get_position(s->voice[i]) == -1) {
                        voice_start(s->voice[i]);
                    }
                } else {
                    voice_stop(s->voice[i]);
                }
            }
        }
    }

    js_pushnumber(J, cur_frame);
}

/*********************
** public functions **
*********************/
/**
 * @brief initialize neural subsystem.
 *
 * @param J VM state.
 */
void init_smacker(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, SMACKER, Close, 0);
        NPROTDEF(J, SMACKER, Rewind, 0);
        NPROTDEF(J, SMACKER, Play, 2);
    }
    CTORDEF(J, new_SMACKER, TAG_SMACKER, 2);
}
