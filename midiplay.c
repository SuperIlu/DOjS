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

#include "midiplay.h"
#include <mujs.h>
#include "DOjS.h"

#ifndef PLATFORM_UNIX
#include <allegro.h>

/*********************
** static functions **
*********************/
/**
 * @brief finalize an image and free resources.
 *
 * @param J VM state.
 */
static void Midi_Finalize(js_State *J, void *data) {
    MIDI *midi = (MIDI *)data;
    destroy_midi(midi);
}

/**
 * @brief load a MIDI and store it as userdata in JS object.
 * new Midi(filename:string)
 *
 * @param J VM state.
 */
static void new_Midi(js_State *J) {
    const char *fname = js_tostring(J, 1);

    MIDI *midi = load_midi(fname);
    if (!midi) {
        js_error(J, "Can't load midi '%s'", fname);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_MIDI, midi, Midi_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, get_midi_length(midi));
    js_defproperty(J, -2, "length", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
}

/**
 * @brief play back midi.
 * midi.Play(loop:boolean)
 *
 * @param J VM state.
 */
static void mid_Play(js_State *J) {
    if (sound_available) {
        MIDI *midi = js_touserdata(J, 0, TAG_MIDI);
        bool loop = js_toboolean(J, 1);

        play_midi(midi, loop);
    }
}

/**
 * @brief check if midi is still playing.
 * midi.IsPlaying():boolean
 *
 * @param J VM state.
 */
static void mid_IsPlaying(js_State *J) {
    if (sound_available) {
        js_pushboolean(J, midi_pos >= 0);
    } else {
        js_pushboolean(J, false);
    }
}

static void mid_Stop(js_State *J) {
    if (sound_available) {
        stop_midi();
    }
}

static void mid_Pause(js_State *J) {
    if (sound_available) {
        midi_pause();
    }
}

static void mid_Resume(js_State *J) {
    if (sound_available) {
        midi_resume();
    }
}

static void mid_GetTime(js_State *J) {
    if (sound_available) {
        js_pushnumber(J, midi_time);
    } else {
        js_pushnumber(J, -1);
    }
}

static void mid_Out(js_State *J) {
    if (sound_available) {
        if (!js_isarray(J, 1)) {
            js_error(J, "Array expected");
        } else {
            int len = js_getlength(J, 1);

            unsigned char *data = malloc(len);
            if (!data) {
                js_error(J, "No memory for array");
            }

            for (int i = 0; i < len; i++) {
                js_getindex(J, 1, i);
                data[i] = js_toint16(J, -1);
                js_pop(J, 1);
            }
            midi_out(data, len);
            free(data);
        }
    }
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize midi subsystem.
 *
 * @param J VM state.
 */
void init_midi(js_State *J) {
    FUNCDEF(J, mid_IsPlaying, "MidiIsPlaying", 0);
    FUNCDEF(J, mid_Stop, "MidiStop", 0);
    FUNCDEF(J, mid_Pause, "MidiPause", 0);
    FUNCDEF(J, mid_Resume, "MidiResume", 0);
    FUNCDEF(J, mid_Out, "MidiOut", 1);
    FUNCDEF(J, mid_GetTime, "MidiGetTime", 0);

    js_newobject(J);
    { PROTDEF(J, mid_Play, TAG_MIDI, "Play", 1); }
    js_newcconstructor(J, new_Midi, new_Midi, TAG_MIDI, 1);
    js_defglobal(J, TAG_MIDI, JS_DONTENUM);
}

/**
 * @brief shutdown MIDI subsystem.
 */
void shutdown_midi() {
    if (sound_available) {
        stop_midi();
    }
}

#else
void init_midi(js_State *J) { return false; }
void shutdown_midi() {}
#endif  // PLATFORM_UNIX
