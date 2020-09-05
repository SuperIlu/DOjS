/*
MIT License

Copyright (c) 2019-2020 Andre Seidelt <superilu@yahoo.com>

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

/**************
** Variables **
**************/
static char *snd_sample_data = NULL;     //!< pointer to recorded sample data
static int snd_buffer_size = 0;          //!< size of sample buffer in samples
static bool snd_stereo = false;          //!< stereo selected?
static bool snd_16bit = false;           //!< 16 bit selected
static bool snd_data_available = false;  //!< is data available to be pulled?

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
    NEW_OBJECT_PREP(J);
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

    js_pushnumber(J, snd->bits);
    js_defproperty(J, -2, "bits", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushboolean(J, snd->stereo);
    js_defproperty(J, -2, "stereo", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
}

/**
 * @brief play back the sound.
 * snd.Play()
 *
 * @param J VM state.
 */
static void Sample_play(js_State *J) {
    if (DOjS.sound_available) {
        SAMPLE *snd = js_touserdata(J, 0, TAG_SAMPLE);

        int vol = js_toint16(J, 1);
        int pan = js_toint16(J, 2);
        bool loop = js_toboolean(J, 3);

        int voc = play_sample(snd, vol, pan, 1000, loop);
        if (voc >= 0) {
            js_pushnumber(J, voc);
        } else {
            js_pushnull(J);
        }
    } else {
        js_pushnull(J);
    }
}

/**
 * @brief stop sound.
 * snd.Stop()
 *
 * @param J VM state.
 */
static void Sample_stop(js_State *J) {
    if (DOjS.sound_available) {
        SAMPLE *snd = js_touserdata(J, 0, TAG_SAMPLE);
        stop_sample(snd);
    }
}

/**
 * @brief get sample data.
 * snd.Get(idx)
 *
 * @param J VM state.
 */
static void Sample_get(js_State *J) {
    SAMPLE *snd = js_touserdata(J, 0, TAG_SAMPLE);
    unsigned int idx = js_touint32(J, 1);

    if (idx >= snd->len) {
        js_error(J, "Sample index %d out of range, %ld max.", idx, snd->len);
    }

    if (snd->bits == 8) {
        uint8_t *buf = snd->data;
        if (snd->stereo) {
            js_newarray(J);
            js_pushnumber(J, buf[idx * 2]);
            js_setindex(J, -2, 0);
            js_pushnumber(J, buf[idx * 2 + 1]);
            js_setindex(J, -2, 1);
        } else {
            js_newarray(J);
            js_pushnumber(J, buf[idx]);
            js_setindex(J, -2, 0);
            js_pushnumber(J, buf[idx]);
            js_setindex(J, -2, 1);
        }
    } else {
        uint16_t *buf = snd->data;
        if (snd->stereo) {
            js_newarray(J);
            js_pushnumber(J, buf[idx * 2]);
            js_setindex(J, -2, 0);
            js_pushnumber(J, buf[idx * 2 + 1]);
            js_setindex(J, -2, 1);
        } else {
            js_newarray(J);
            js_pushnumber(J, buf[idx]);
            js_setindex(J, -2, 0);
            js_pushnumber(J, buf[idx]);
            js_setindex(J, -2, 1);
        }
    }
}

/**
 * @brief got current play position of given voice.
 * VoiceGetPosition(voc:number):number
 *
 * @param J VM state.
 */
static void snd_get_pos(js_State *J) {
    int voc = js_touint32(J, 1);
    js_pushnumber(J, voice_get_position(voc));
}

/**
 * @brief recoding callback for new data available.
 */
static void snd_recorder() { snd_data_available = true; }

/**
 * @brief select sound input source.
 * SoundInputSource(source:number)
 *
 * @param J VM state.
 */
static void snd_input_source(js_State *J) {
    if (DOjS.sndin_available) {
        int src = js_toint16(J, 1);
        if (set_sound_input_source(src) < 0) {
            js_error(J, "Cannot select sound source: hardware does not provide an input select register");
            return;
        }
    }
}

/**
 * @brief start recording sound data.
 * SoundStartInput(rate:number, bits:number, stereo:bool)
 *
 * @param J VM state.
 */
static void snd_start_input(js_State *J) {
    if (DOjS.sndin_available) {
        int freq = js_toint16(J, 1);
        int bits = js_toint16(J, 2);
        switch (bits) {
            case 8:
                snd_16bit = false;
                break;
            case 16:
                snd_16bit = true;
                break;
            default:
                js_error(J, "wrong bit size");
                break;
        }
        snd_stereo = js_toboolean(J, 3);

        digi_recorder = &snd_recorder;

        snd_buffer_size = start_sound_input(freq, bits, snd_stereo);
        if (snd_buffer_size <= 0) {
            js_error(J, "Cannot record sound: wrong parameters?");
            return;
        }
        int bytes_per_sample = ((bits == 8) ? 1 : sizeof(short)) * ((snd_stereo) ? 2 : 1);
        DEBUGF("snd_buffer_size=%d, bytes/sample=%d\n", snd_buffer_size, bytes_per_sample);
        snd_sample_data = malloc(snd_buffer_size * bytes_per_sample);
        if (!snd_sample_data) {
            JS_ENOMEM(J);
        }
        snd_data_available = true;
    }
}

/**
 * @brief end recording.
 * SoundStopInput()
 *
 * @param J VM state.
 */
static void snd_stop_input(js_State *J) {
    if (DOjS.sndin_available) {
        stop_sound_input();

        free(snd_sample_data);

        snd_sample_data = NULL;
    }
}

/**
 * @brief get current sample buffer.
 * ReadSoundInput(): array[number] or array[array[number], array[number]]
 *
 * @param J VM state.
 */
static void snd_read_input(js_State *J) {
    if (DOjS.sndin_available && snd_sample_data && snd_data_available) {
        read_sound_input(snd_sample_data);
        snd_data_available = false;
        if (snd_stereo) {
            js_newarray(J);

            // left
            js_newarray(J);
            for (int i = 0; i < snd_buffer_size; i++) {
                if (snd_16bit) {
                    js_pushnumber(J, ((unsigned short *)snd_sample_data)[i * 2]);
                } else {
                    js_pushnumber(J, ((unsigned char *)snd_sample_data)[i * 2]);
                }
                js_setindex(J, -2, i);
            }
            js_setindex(J, -2, 0);

            // right
            js_newarray(J);
            for (int i = 0; i < snd_buffer_size; i++) {
                if (snd_16bit) {
                    js_pushnumber(J, ((unsigned short *)snd_sample_data)[i * 2 + 1]);
                } else {
                    js_pushnumber(J, ((unsigned char *)snd_sample_data)[i * 2 + 1]);
                }
                js_setindex(J, -2, i);
            }
            js_setindex(J, -2, 1);

        } else {
            js_newarray(J);
            for (int i = 0; i < snd_buffer_size; i++) {
                if (snd_16bit) {
                    js_pushnumber(J, ((unsigned short *)snd_sample_data)[i]);
                } else {
                    js_pushnumber(J, ((unsigned char *)snd_sample_data)[i]);
                }
                js_setindex(J, -2, i);
            }
        }
    } else {
        js_pushnull(J);
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
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    // sound output
    bool sound_ok = install_sound(DOjS.params.no_sound ? DIGI_NONE : DIGI_AUTODETECT, DOjS.params.no_fm ? MIDI_NONE : MIDI_AUTODETECT, NULL) == 0;
    if (!sound_ok) {
        LOGF("Sound output: %s\n", allegro_error);
    } else {
        LOGF("Sound output: OK\n");
    }
    DOjS.midi_available = sound_ok && !DOjS.params.no_fm;
    DOjS.sound_available = sound_ok && !DOjS.params.no_sound;
    PROPDEF_B(J, DOjS.sound_available, "SOUND_AVAILABLE");

    js_newobject(J);
    {
        PROTDEF(J, Sample_play, TAG_SAMPLE, "Play", 0);
        PROTDEF(J, Sample_stop, TAG_SAMPLE, "Stop", 1);
        PROTDEF(J, Sample_get, TAG_SAMPLE, "Get", 1);
    }
    js_newcconstructor(J, new_Sample, new_Sample, TAG_SAMPLE, 1);
    js_defglobal(J, TAG_SAMPLE, JS_DONTENUM);

    // sound input
    DOjS.sndin_available = install_sound_input(DOjS.params.no_sound ? DIGI_NONE : DIGI_AUTODETECT, DOjS.params.no_fm ? MIDI_NONE : MIDI_AUTODETECT) == 0;
    if (!DOjS.sndin_available) {
        LOGF("Sound input: %s\n", allegro_error);
    } else {
        LOGF("Sound input: OK\n");
    }

    int cap = get_sound_input_cap_bits();

    // report capabilities
    PROPDEF_B(J, DOjS.sndin_available, "SNDIN_AVAILABLE");       // do we have sound input?
    PROPDEF_B(J, ((cap & 8) != 0), "SNDIN_8BIT");                // do we have 8bit input?
    PROPDEF_B(J, ((cap & 16) != 0), "SNDIN_16BIT");              // do we have 16bit input?
    PROPDEF_B(J, get_sound_input_cap_stereo(), "SNDIN_STEREO");  // do we have stereo input?

    FUNCDEF(J, snd_get_pos, "VoiceGetPosition", 1);

    FUNCDEF(J, snd_input_source, "SoundInputSource", 1);
    FUNCDEF(J, snd_start_input, "SoundStartInput", 3);
    FUNCDEF(J, snd_stop_input, "SoundStopInput", 0);
    FUNCDEF(J, snd_read_input, "ReadSoundInput", 0);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief shutdown module/sample subsystem.
 */
void shutdown_sound() {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);
    if (DOjS.sound_available) {
        snd_stop_input(NULL);
    }
    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
