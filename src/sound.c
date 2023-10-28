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

#include "sound.h"

#include <allegro.h>
#include <mujs.h>

#include "DOjS.h"
#include "util.h"
#include "zipfile.h"
#include "intarray.h"

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
    SAMPLE *snd = NULL;
    NEW_OBJECT_PREP(J);
    const char *fname = js_tostring(J, 1);

    char *delim = strchr(fname, ZIP_DELIM);

    if (!delim) {
        snd = load_sample(fname);
        if (!snd) {
            js_error(J, "Can't load sample '%s'", fname);
            return;
        }
    } else {
        PACKFILE *pf = open_zipfile1(fname);
        if (!pf) {
            js_error(J, "Can't load sample '%s'", fname);
            return;
        }
        if (stricmp("voc", ut_getFilenameExt(fname)) == 0) {
            snd = load_voc_pf(pf);
        } else if (stricmp("wav", ut_getFilenameExt(fname)) == 0) {
            snd = load_wav_pf(pf);
        } else {
            snd = NULL;
        }
        pack_fclose(pf);

        if (!snd) {
            js_error(J, "Can't load sample '%s'", fname);
            return;
        }
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_SAMPLE, snd, Sample_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, snd->len);
    js_defproperty(J, -2, "length", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, snd->freq);
    js_defproperty(J, -2, "frequency", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, snd->bits);
    js_defproperty(J, -2, "bits", JS_READONLY | JS_DONTCONF);

    js_pushboolean(J, snd->stereo);
    js_defproperty(J, -2, "stereo", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief play back the sound.
 * snd.Play()
 *
 * @param J VM state.
 */
static void Sample_Play(js_State *J) {
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
static void Sample_Stop(js_State *J) {
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
static void Sample_Get(js_State *J) {
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
static void f_VoiceGetPosition(js_State *J) {
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
static void f_SoundInputSource(js_State *J) {
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
static void f_SoundStartInput(js_State *J) {
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
static void f_SoundStopInput(js_State *J) {
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
static void f_ReadSoundInput(js_State *J) {
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

/**
 * @brief get current sample buffer.
 * ReadSoundInput(): IntArray or array[IntArray, IntArray]
 *
 * @param J VM state.
 */
static void f_ReadSoundInputInts(js_State *J) {
    if (DOjS.sndin_available && snd_sample_data && snd_data_available) {
        read_sound_input(snd_sample_data);
        snd_data_available = false;
        if (snd_stereo) {
            // create JavaScript array for left/right channel
            js_newarray(J);

            // create left IntArray
            int_array_t *ia = IntArray_create();
            if (!ia) {
                JS_ENOMEM(J);
                return;
            }
            for (int i = 0; i < snd_buffer_size; i++) {
                IA_TYPE val;
                if (snd_16bit) {
                    val = ((unsigned short *)snd_sample_data)[i * 2];
                } else {
                    val = ((unsigned char *)snd_sample_data)[i * 2];
                }
                if (IntArray_push(ia, val) < 0) {
                    IntArray_destroy(ia);
                    JS_ENOMEM(J);
                    return;
                }
            }
            // create JavaScript object and put into array
            IntArray_fromStruct(J, ia);
            js_setindex(J, -2, 0);

            // create right IntArray
            ia = IntArray_create();
            if (!ia) {
                JS_ENOMEM(J);
                return;
            }
            for (int i = 0; i < snd_buffer_size; i++) {
                IA_TYPE val;
                if (snd_16bit) {
                    val = ((unsigned short *)snd_sample_data)[i * 2 + 1];
                } else {
                    val = ((unsigned char *)snd_sample_data)[i * 2 + 1];
                }
                if (IntArray_push(ia, val) < 0) {
                    IntArray_destroy(ia);
                    JS_ENOMEM(J);
                    return;
                }
            }
            // create JavaScript object and put into array
            IntArray_fromStruct(J, ia);
            js_setindex(J, -2, 1);

        } else {
            // create IntArray
            int_array_t *ia = IntArray_create();
            if (!ia) {
                JS_ENOMEM(J);
                return;
            }

            // put values into it
            for (int i = 0; i < snd_buffer_size; i++) {
                IA_TYPE val;
                if (snd_16bit) {
                    val = ((unsigned short *)snd_sample_data)[i];
                } else {
                    val = ((unsigned char *)snd_sample_data)[i];
                }
                if (IntArray_push(ia, val) < 0) {
                    IntArray_destroy(ia);
                    JS_ENOMEM(J);
                    return;
                }
            }

            // create JavaScript object
            IntArray_fromStruct(J, ia);
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
        LOGF("Sound output: PCM[%s; %s; %d], FM[%s; %s; %d]\n", digi_driver->name, digi_driver->desc, digi_driver->max_voices, midi_driver->name, midi_driver->desc,
             midi_driver->max_voices);
    }
    DOjS.midi_available = sound_ok && !DOjS.params.no_fm;
    DOjS.sound_available = sound_ok && !DOjS.params.no_sound;
    PROPDEF_B(J, DOjS.sound_available, "SOUND_AVAILABLE");

    js_newobject(J);
    {
        NPROTDEF(J, Sample, Play, 0);
        NPROTDEF(J, Sample, Stop, 1);
        NPROTDEF(J, Sample, Get, 1);
    }
    CTORDEF(J, new_Sample, TAG_SAMPLE, 1);

#if LINUX != 1
    // sound input
    DOjS.sndin_available = install_sound_input(DOjS.params.no_sound ? DIGI_NONE : DIGI_AUTODETECT, DOjS.params.no_fm ? MIDI_NONE : MIDI_AUTODETECT) == 0;
    if (!DOjS.sndin_available) {
        LOGF("Sound input: %s\n", allegro_error);
    } else {
        LOGF("Sound input: OK\n");
    }
#endif

    int cap = get_sound_input_cap_bits();

    // report capabilities
    PROPDEF_B(J, DOjS.sndin_available, "SNDIN_AVAILABLE");       // do we have sound input?
    PROPDEF_B(J, ((cap & 8) != 0), "SNDIN_8BIT");                // do we have 8bit input?
    PROPDEF_B(J, ((cap & 16) != 0), "SNDIN_16BIT");              // do we have 16bit input?
    PROPDEF_B(J, get_sound_input_cap_stereo(), "SNDIN_STEREO");  // do we have stereo input?

    NFUNCDEF(J, VoiceGetPosition, 1);

    NFUNCDEF(J, SoundInputSource, 1);
    NFUNCDEF(J, SoundStartInput, 3);
    NFUNCDEF(J, SoundStopInput, 0);
    NFUNCDEF(J, ReadSoundInput, 0);
    NFUNCDEF(J, ReadSoundInputInts, 0);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief shutdown module/sample subsystem.
 */
void shutdown_sound() {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);
    if (DOjS.sound_available) {
        f_SoundStopInput(NULL);
    }
    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
