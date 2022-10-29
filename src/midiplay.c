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

#include "midiplay.h"

#include <allegro.h>
#include <mujs.h>

#include "DOjS.h"
#include "zipfile.h"

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
    NEW_OBJECT_PREP(J);
    MIDI *midi;
    const char *fname = js_tostring(J, 1);

    char *delim = strchr(fname, ZIP_DELIM);

    if (!delim) {
        midi = load_midi(fname);
        if (!midi) {
            js_error(J, "Can't load midi '%s'", fname);
            return;
        }
    } else {
        PACKFILE *pf = open_zipfile1(fname);
        if (!pf) {
            js_error(J, "Can't load midi '%s'", fname);
            return;
        }
        midi = load_midi_pf(pf);  // PACKFILE is closed by this function!
        if (!midi) {
            js_error(J, "Can't load midi '%s'", fname);
            return;
        }
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_MIDI, midi, Midi_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, get_midi_length(midi));
    js_defproperty(J, -2, "length", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief play back midi.
 * midi.Play(loop:boolean)
 *
 * @param J VM state.
 */
static void Midi_Play(js_State *J) {
    if (DOjS.midi_available) {
        MIDI *midi = js_touserdata(J, 0, TAG_MIDI);
        bool loop = js_toboolean(J, 1);

        play_midi(midi, loop);
    }
}

/**
 * @brief check if midi is still playing.
 * MidiIsPlaying():boolean
 *
 * @param J VM state.
 */
static void f_MidiIsPlaying(js_State *J) {
    if (DOjS.midi_available) {
        js_pushboolean(J, midi_pos >= 0);
    } else {
        js_pushboolean(J, false);
    }
}

/**
 * @brief stop MIDI playing
 *
 * @param J VM state.
 */
static void f_MidiStop(js_State *J) {
    if (DOjS.midi_available) {
        stop_midi();
    }
}

/**
 * @brief pause MIDI playing.
 *
 * @param J VM state.
 */
static void f_MidiPause(js_State *J) {
    if (DOjS.midi_available) {
        midi_pause();
    }
}

/**
 * @brief resume MIDI playing after pause.
 *
 * @param J VM state.
 */
static void f_MidiResume(js_State *J) {
    if (DOjS.midi_available) {
        midi_resume();
    }
}

/**
 * @brief get current play time.
 *
 * @param J VM state.
 */
static void f_MidiGetTime(js_State *J) {
    if (DOjS.midi_available) {
        js_pushnumber(J, midi_time);
    } else {
        js_pushnumber(J, -1);
    }
}

/**
 * @brief get current play position.
 *
 * @param J VM state.
 */
static void f_MidiGetPos(js_State *J) {
    if (DOjS.midi_available) {
        js_pushnumber(J, midi_pos);
    } else {
        js_pushnumber(J, -1);
    }
}

/**
 * @brief send MIDI command to playback device.
 *
 * @param J VM state.
 */
static void f_MidiOut(js_State *J) {
    if (DOjS.midi_available) {
        if (!js_isarray(J, 1)) {
            JS_ENOARR(J);
        } else {
            int len = js_getlength(J, 1);

            unsigned char *data = malloc(len);
            if (!data) {
                JS_ENOMEM(J);
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
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    PROPDEF_B(J, DOjS.midi_available, "MIDI_AVAILABLE");

    NFUNCDEF(J, MidiIsPlaying, 0);
    NFUNCDEF(J, MidiStop, 0);
    NFUNCDEF(J, MidiPause, 0);
    NFUNCDEF(J, MidiResume, 0);
    NFUNCDEF(J, MidiOut, 1);
    NFUNCDEF(J, MidiGetTime, 0);
    NFUNCDEF(J, MidiGetPos, 0);

    js_newobject(J);
    { NPROTDEF(J, Midi, Play, 1); }
    CTORDEF(J, new_Midi, TAG_MIDI, 1);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief shutdown MIDI subsystem.
 */
void shutdown_midi() {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);
    if (DOjS.midi_available) {
        stop_midi();
    }
    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
