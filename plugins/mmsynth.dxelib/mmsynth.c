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
#include "util.h"
#include "zipfile.h"

#include "MicroModSynth/src/synth.h"

void init_mmsynth(js_State *J);

typedef enum { WAVE_SAWTOOTH, WAVE_SINE, WAVE_SQUARE, WAVE_TRIANGLE, WAVE_FALLING, WAVE_EXP_DECAY } wave_t;

/************
** defines **
************/
#define TAG_MMSYNTH "MmSynth"  //!< pointer tag

// get voice (starting @ 1)
#define MM_GET_VOICE()                                \
    mmsynth_t *ov = js_touserdata(J, 0, TAG_MMSYNTH); \
    int32_t v = js_touint32(J, 1);                    \
    if (v < 1 || v > ov->synth->synth_voices) {       \
        js_error(J, "Illegal voice %ld", v);          \
        return;                                       \
    }                                                 \
    v--;                                              \
    SynthVoice_t *voice = &ov->synth->synthVoices[v];

// get voice and node (both starting @ 1)
#define MM_GET_VOICE_AND_NODE()             \
    MM_GET_VOICE();                         \
    int32_t n = js_touint32(J, 2);          \
    if (n < 1 || n > voice->synth_nodes) {  \
        js_error(J, "Illegal node %ld", n); \
        return;                             \
    }                                       \
    n--;                                    \
    SynthNode_t *node = &voice->nodes[n];

/************
** structs **
************/
//! userdata definition
typedef struct {
    AUDIOSTREAM *stream;
    size_t buffer_size;
    uint32_t samplerate;
    Synth_t *synth;
} mmsynth_t;

/*********************
** static functions **
*********************/

/**
 * @brief get the address of a node output (<0), a literal value (>=0) or NULL/null
 *
 * @param J VM state.
 * @param idx JS parameter index
 * @param voice the voice this node belongs to
 * @param valPtr space to store the value (if any)
 * @param out output value to use
 *
 * @return true if all went well,
 */
static bool mmsynth_GetValueOrNode(js_State *J, int idx, SynthVoice_t *voice, q15_t *valPtr, q15_t **out) {
    if (js_isnull(J, idx)) {
        *out = NULL;
        return true;
    } else if (js_isnumber(J, idx)) {
        int32_t v = js_toint32(J, idx);
        if (v < 0) {
            // negative values are node numbers (starting @ 1)
            v *= -1;
            if (v >= voice->synth_nodes) {
                js_error(J, "Illegal node %ld", v);
                return false;
            } else {
                *out = &voice->nodes[v - 1].output;
                return true;
            }
        } else {
            // positive value are literals
            *valPtr = v;
            *out = valPtr;
            return true;
        }
    } else {
        js_error(J, "Illegal parameter");
        return false;
    }
}

/**
 * @brief free ressources.
 *
 * @param ov the mmsynth_t with the ressources to free.
 */
static void mmsynth_cleanup(mmsynth_t *ov) {
    if (ov->stream) {
        stop_audio_stream(ov->stream);
        ov->stream = NULL;
    }
}

/**
 * @brief finalize a file and free resources.
 *
 * @param J VM state.
 */
static void mmsynth_Finalize(js_State *J, void *data) {
    mmsynth_t *ov = (mmsynth_t *)data;
    mmsynth_cleanup(ov);
    free(ov);
}

/**
 * @brief create a mmsynth.
 * mm = new MMSynth(voices:number, nodes:number, bufferSize:number)
 *
 * @param J VM state.
 */
static void new_mmsynth(js_State *J) {
    NEW_OBJECT_PREP(J);

    mmsynth_t *ov = calloc(1, sizeof(mmsynth_t));
    if (!ov) {
        JS_ENOMEM(J);
        return;
    }

    uint32_t numVoices = 2;
    uint32_t numNodes = 8;
    ov->buffer_size = 1024 * 4;

    if (js_isnumber(J, 1)) {
        numVoices = js_touint32(J, 1);
    }
    if (js_isnumber(J, 2)) {
        numNodes = js_touint32(J, 2);
    }
    if (js_isnumber(J, 3)) {
        ov->buffer_size = js_touint32(J, 3);
    }

    if (numVoices < 1) {
        js_error(J, "numVoices must be > 0");
        free(ov);
        return;
    }

    if (numNodes < 1) {
        js_error(J, "numNodes must be > 0");
        free(ov);
        return;
    }

    if (ov->buffer_size < 1024) {
        js_error(J, "bufferSize must be >= 1024");
        free(ov);
        return;
    }

    ov->synth = synthCreate(numVoices, numNodes);
    ov->samplerate = SAMPLE_RATE;

    // allocate stream
    ov->stream = play_audio_stream(ov->buffer_size, 16, false, ov->samplerate, 255, 128);
    if (!ov->stream) {
        JS_ENOMEM(J);
        mmsynth_Finalize(J, ov);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_MMSYNTH, ov, mmsynth_Finalize);

    // add properties
    js_pushnumber(J, ov->samplerate);
    js_defproperty(J, -2, "samplerate", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->buffer_size);
    js_defproperty(J, -2, "buffer_size", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->synth->synth_voices);
    js_defproperty(J, -2, "synth_voices", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->synth->synthVoices[0].synth_nodes);
    js_defproperty(J, -2, "synth_nodes", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, Q15_MAX);
    js_defproperty(J, -2, "MAX", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, Q15_MIN);
    js_defproperty(J, -2, "MIN", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, WAVE_SAWTOOTH);
    js_defproperty(J, -2, "SAWTOOTH", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, WAVE_SINE);
    js_defproperty(J, -2, "SINE", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, WAVE_SQUARE);
    js_defproperty(J, -2, "SQUARE", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, WAVE_TRIANGLE);
    js_defproperty(J, -2, "TRIANGLE", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, WAVE_FALLING);
    js_defproperty(J, -2, "FALLING", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, WAVE_EXP_DECAY);
    js_defproperty(J, -2, "EXP_DECAY", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief close mmsynth.
 * mm.Close()
 *
 * @param J VM state.
 */
static void mmsynth_Close(js_State *J) {
    mmsynth_t *ov = js_touserdata(J, 0, TAG_MMSYNTH);
    mmsynth_cleanup(ov);
}

/**
 * @brief check if the given node for given voice is defined.
 * mm.IsNodeDefined(voice:number, node:number):bool
 *
 * @param J VM state.
 */
static void mmsynth_IsNodeDefined(js_State *J) {
    MM_GET_VOICE_AND_NODE();

    js_pushboolean(J, node->type != SYNTH_NODE_NONE);
}

/**
 * @brief create an envelope node
 * mm.EnvelopeNode(voice:number, node:number, gain:number, attack:number, decay:number, sustain:number, release:number)
 *
 * @param J VM state.
 */
static void mmsynth_EnvelopeNode(js_State *J) {
    MM_GET_VOICE_AND_NODE();

    // get gain
    q15_t *gain;
    if (!mmsynth_GetValueOrNode(J, 3, voice, &node->gainVal, &gain)) {
        return;
    }

    synthInitEnvelopeNode(node, gain,        // gain
                          js_toint32(J, 4),  // attack
                          js_toint32(J, 5),  // decay
                          js_toint32(J, 6),  // sustain
                          js_toint32(J, 7)   // release
    );
}

/**
 * @brief create an oscillator node
 * mm.OscillatorNode(voice:number, node:number, gain:number, phaseIncrement:number, detune:number, wavegen:number)
 *
 * @param J VM state.
 */
static void mmsynth_OscillatorNode(js_State *J) {
    MM_GET_VOICE_AND_NODE();

    // get gain
    q15_t *gain;
    if (!mmsynth_GetValueOrNode(J, 3, voice, &node->gainVal, &gain)) {
        return;
    }

    // get phase increment
    q15_t *phaseIncrement;
    if (!mmsynth_GetValueOrNode(J, 4, voice, &node->osc.phaseIncrementVal, &phaseIncrement)) {
        return;
    }
    if (phaseIncrement == NULL) {
        phaseIncrement = &voice->phaseIncrement;
    }

    // get detune
    q15_t *detune;
    if (!mmsynth_GetValueOrNode(J, 5, voice, &node->osc.detuneVal, &detune)) {
        return;
    }

    // get wavegen
    q15_t (*wavegen)(q15_t);
    switch (js_tointeger(J, 6)) {
        case WAVE_SAWTOOTH:
            wavegen = sawtoothWave;
            break;
        case WAVE_SINE:
            wavegen = sineWave;
            break;
        case WAVE_SQUARE:
            wavegen = squareWave;
            break;
        case WAVE_TRIANGLE:
            wavegen = triangleWave;
            break;
        case WAVE_FALLING:
            wavegen = fallingWave;
            break;
        case WAVE_EXP_DECAY:
            wavegen = expDecayWave;
            break;

        default:
            js_error(J, "Unknown wavegen");
            return;
    }

    synthInitOscNode(node, gain, phaseIncrement, detune, wavegen);
}

/**
 * @brief create a LP filter node
 * mm.FilterLpNode(voice:number, node:number, gain:number, input:number, factor:number)
 *
 * @param J VM state.
 */
static void mmsynth_FilterLpNode(js_State *J) {
    MM_GET_VOICE_AND_NODE();

    // get gain
    q15_t *gain;
    if (!mmsynth_GetValueOrNode(J, 3, voice, &node->gainVal, &gain)) {
        return;
    }

    // get input
    q15_t *input;
    if (!mmsynth_GetValueOrNode(J, 4, voice, &node->filter.inputVal, &input)) {
        return;
    }

    synthInitFilterLpNode(node, gain, input, js_toint32(J, 5));  // factor
}

/**
 * @brief create a HP filter node
 * mm.FilterHpNode(voice:number, node:number, gain:number, input:number, factor:number)
 *
 * @param J VM state.
 */
static void mmsynth_FilterHpNode(js_State *J) {
    MM_GET_VOICE_AND_NODE();

    // get gain
    q15_t *gain;
    if (!mmsynth_GetValueOrNode(J, 3, voice, &node->gainVal, &gain)) {
        return;
    }

    // get input
    q15_t *input;
    if (!mmsynth_GetValueOrNode(J, 4, voice, &node->filter.inputVal, &input)) {
        return;
    }

    synthInitFilterHpNode(node, gain, input, js_toint32(J, 5));  // factor
}

/**
 * @brief create a mixer node
 * mm.MixerNode(voice:number, node:number, gain:number, input1:number, input2:number, input3:number)
 *
 * @param J VM state.
 */
static void mmsynth_MixerNode(js_State *J) {
    MM_GET_VOICE_AND_NODE();

    // get gain
    q15_t *gain;
    if (!mmsynth_GetValueOrNode(J, 3, voice, &node->gainVal, &gain)) {
        return;
    }

    // get input1
    q15_t *input1;
    if (!mmsynth_GetValueOrNode(J, 4, voice, &node->mixer.inputsVals[0], &input1)) {
        return;
    }

    // get input2
    q15_t *input2;
    if (!mmsynth_GetValueOrNode(J, 5, voice, &node->mixer.inputsVals[1], &input2)) {
        return;
    }

    // get input3
    q15_t *input3;
    if (!mmsynth_GetValueOrNode(J, 6, voice, &node->mixer.inputsVals[2], &input3)) {
        return;
    }

    synthInitMixerNode(node, gain, input1, input2, input3);
}

/**
 * @brief convert Hz to Phase.
 * mm.HzToPhase(hz:number):number
 *
 * @param J VM state.
 */
static void mmsynth_HzToPhase(js_State *J) {
    q15_t in = js_toint16(J, 1);
    q15_t out = SYNTH_HZ_TO_PHASE(in);

    js_pushnumber(J, out);
}

/**
 * @brief play a note on a voice.
 * mm.NoteOn(voice:number, note:number)
 *
 * @param J VM state.
 */
static void mmsynth_NoteOn(js_State *J) {
    MM_GET_VOICE();
    synthVoiceNoteOn(voice, js_toint16(J, 2));
}

/**
 * @brief silence voice.
 * mm.NoteOff(voice:number)
 *
 * @param J VM state.
 */
static void mmsynth_NoteOff(js_State *J) {
    MM_GET_VOICE();
    synthVoiceNoteOff(voice);
}

/**
 * @brief render sound output
 * mm.Play()
 *
 * @param J VM state.
 */
static void mmsynth_Play(js_State *J) {
    mmsynth_t *ov = js_touserdata(J, 0, TAG_MMSYNTH);

    short *mem_chunk = get_audio_stream_buffer(ov->stream);
    if (mem_chunk != NULL) {
        int idx = 0;
        for (int i = 0; i < ov->buffer_size; i++) {
            mem_chunk[idx] = synthProcess(ov->synth) ^ 0x8000;
            idx++;
        }
        free_audio_stream_buffer(ov->stream);
    }
}

static bool mmsynth_NodeOrValue(js_State *J, SynthVoice_t *voice, q15_t *data, q15_t *val_addr) {
    if (data == NULL) {
        // NULL pointer --> push null
        js_pushnull(J);
        return true;
    } else {
        if (data == val_addr) {
            // the field points to the value space --> push value
            js_pushnumber(J, *data);
            return true;
        } else {
            // this must be a node --> try to find node number
            for (int n = 0; n < voice->synth_nodes; n++) {
                if (&voice->nodes[n].output == data) {
                    js_pushnumber(J, -(n + 1));
                    return true;
                }
            }
            return false;
        }
    }
}

static void mmsynth_ReturnNode(js_State *J, SynthVoice_t *voice, SynthNode_t *node, const char *name) {
    js_newobject(J);

    js_pushstring(J, name);
    js_setproperty(J, -2, "type");

    mmsynth_NodeOrValue(J, voice, node->gain, &node->gainVal);
    js_setproperty(J, -2, "gain");
}

/**
 * @brief return node as JS object
 * mm.GetNode(voice:number, node:number)
 *
 * @param J VM state.
 */
static void mmsynth_GetNode(js_State *J) {
    MM_GET_VOICE_AND_NODE();

    switch (node->type) {
        case SYNTH_NODE_NONE:
            js_newobject(J);
            {
                js_pushnull(J);
                js_setproperty(J, -2, "type");
            }
            break;
        case SYNTH_NODE_OSCILLATOR:
            int wavegen;
            if (node->osc.wavegen == sawtoothWave) {
                wavegen = WAVE_SAWTOOTH;
            } else if (node->osc.wavegen == sineWave) {
                wavegen = WAVE_SINE;
            } else if (node->osc.wavegen == squareWave) {
                wavegen = WAVE_SQUARE;
            } else if (node->osc.wavegen == triangleWave) {
                wavegen = WAVE_TRIANGLE;
            } else if (node->osc.wavegen == fallingWave) {
                wavegen = WAVE_FALLING;
            } else if (node->osc.wavegen == expDecayWave) {
                wavegen = WAVE_EXP_DECAY;
            } else {
                js_error(J, "Unknown node type for voice %ld, node %ld: %d", v, n, node->type);
                return;
            }

            mmsynth_ReturnNode(J, voice, node, "Oscillator");
            {
                if (node->osc.phaseIncrement == &voice->phaseIncrement) {
                    js_pushnull(J);
                } else {
                    mmsynth_NodeOrValue(J, voice, node->osc.phaseIncrement, &node->osc.phaseIncrementVal);
                }
                js_setproperty(J, -2, "phaseIncrement");

                mmsynth_NodeOrValue(J, voice, node->osc.detune, &node->osc.detuneVal);
                js_setproperty(J, -2, "detune");

                js_pushnumber(J, wavegen);
                js_setproperty(J, -2, "wavegen");
            }
            break;
        case SYNTH_NODE_ENVELOPE:
            mmsynth_ReturnNode(J, voice, node, "Envelope");
            {
                js_pushnumber(J, node->env.attack);
                js_setproperty(J, -2, "attack");

                js_pushnumber(J, node->env.decay);
                js_setproperty(J, -2, "decay");

                js_pushnumber(J, node->env.release);
                js_setproperty(J, -2, "release");

                js_pushnumber(J, node->env.sustain);
                js_setproperty(J, -2, "sustain");
            }
            break;
        case SYNTH_NODE_FILTER_LP:
            mmsynth_ReturnNode(J, voice, node, "FilterLp");
            {
                mmsynth_NodeOrValue(J, voice, node->filter.input, &node->filter.inputVal);
                js_setproperty(J, -2, "input");

                js_pushnumber(J, node->filter.factor);
                js_setproperty(J, -2, "factor");
            }
            break;
        case SYNTH_NODE_FILTER_HP:
            mmsynth_ReturnNode(J, voice, node, "FilterHp");
            {
                mmsynth_NodeOrValue(J, voice, node->filter.input, &node->filter.inputVal);
                js_setproperty(J, -2, "input");

                js_pushnumber(J, node->filter.factor);
                js_setproperty(J, -2, "factor");
            }
            break;
        case SYNTH_NODE_MIXER:
            mmsynth_ReturnNode(J, voice, node, "Mixer");
            {
                mmsynth_NodeOrValue(J, voice, node->mixer.inputs[0], &node->mixer.inputsVals[0]);
                js_setproperty(J, -2, "input1");

                mmsynth_NodeOrValue(J, voice, node->mixer.inputs[1], &node->mixer.inputsVals[1]);
                js_setproperty(J, -2, "input2");

                mmsynth_NodeOrValue(J, voice, node->mixer.inputs[2], &node->mixer.inputsVals[2]);
                js_setproperty(J, -2, "input3");
            }
        default:
            js_error(J, "Unknown node type for voice %ld, node %ld: %d", v, n, node->type);
            return;
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
void init_mmsynth(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, mmsynth, Play, 0);
        NPROTDEF(J, mmsynth, Close, 0);
        NPROTDEF(J, mmsynth, IsNodeDefined, 2);
        NPROTDEF(J, mmsynth, HzToPhase, 1);
        NPROTDEF(J, mmsynth, EnvelopeNode, 7);
        NPROTDEF(J, mmsynth, OscillatorNode, 6);
        NPROTDEF(J, mmsynth, FilterLpNode, 5);
        NPROTDEF(J, mmsynth, FilterHpNode, 5);
        NPROTDEF(J, mmsynth, MixerNode, 5);
        NPROTDEF(J, mmsynth, NoteOn, 2);
        NPROTDEF(J, mmsynth, NoteOff, 1);
        NPROTDEF(J, mmsynth, GetNode, 2);
    }
    CTORDEF(J, new_mmsynth, TAG_MMSYNTH, 3);
}
