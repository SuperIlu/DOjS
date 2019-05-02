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

#include "sound.h"
#include <allegro.h>
#include <mujs.h>
#include "DOjS.h"

/*********************
** static functions **
*********************/
/**
 * @brief finalize and free resources.
 *
 * @param J VM state.
 */
static void Sample_Finalize(js_State *J, void *data) {
    SAMPLE *snd = (SAMPLE *)data;
    destroy_sample(snd);
}

/**
 * @brief load a WAV and store it as userdata in JS object.
 * new Sample(filename:string)
 *
 * @param J VM state.
 */
static void new_Sample(js_State *J) {
    const char *fname = js_tostring(J, 1);

    SAMPLE *snd = load_sample(fname);
    if (!snd) {
        js_error(J, "Can't load sample '%s': %s", fname, allegro_error);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_SAMPLE, snd, Sample_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, snd->len);
    js_defproperty(J, -2, "length", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, snd->freq);
    js_defproperty(J, -2, "frequency", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
}

/**
 * @brief play back the sound.
 * snd.Play()
 *
 * @param J VM state.
 */
static void Sample_play(js_State *J) {
    if (sound_available) {
        SAMPLE *snd = js_touserdata(J, 0, TAG_SAMPLE);

        int vol = js_toint16(J, 1);
        int pan = js_toint16(J, 2);
        bool loop = js_toboolean(J, 3);

        play_sample(snd, vol, pan, 1000, loop);
    }
}

/**
 * @brief stop sound.
 * snd.Stop()
 *
 * @param J VM state.
 */
static void Sample_stop(js_State *J) {
    if (sound_available) {
        SAMPLE *snd = js_touserdata(J, 0, TAG_SAMPLE);
        stop_sample(snd);
    }
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize module/sample subsystem.
 *
 * @param J VM state.
 */
void init_sound(js_State *J) {
    js_newobject(J);
    { PROTDEF(J, Sample_play, TAG_SAMPLE, "Play", 0); }
    { PROTDEF(J, Sample_stop, TAG_SAMPLE, "Stop", 1); }
    js_newcconstructor(J, new_Sample, new_Sample, TAG_SAMPLE, 1);
    js_defglobal(J, TAG_SAMPLE, JS_DONTENUM);
}

/**
 * @brief shutdown module/sample subsystem.
 */
void shutdown_sound() {
    if (sound_available) {
    }
}
