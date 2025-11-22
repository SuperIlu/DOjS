// a super fast simple sound synthesis engine
//  2025 Ben Hencke

#include "synth.h"
#include "string.h"

// store the phase increments for each note for the highest notes, which will have the largest phase increments
// then divide them by 2 for each octave below that
#define BASE_OCTAVE 8
const q15_t octavePhases[12] = {
    // start at C8
    SYNTH_HZ_TO_PHASE(4186.01),  // C
    SYNTH_HZ_TO_PHASE(4434.92),  // C#
    SYNTH_HZ_TO_PHASE(4698.63),  // D
    SYNTH_HZ_TO_PHASE(4978.03),  // D#
    SYNTH_HZ_TO_PHASE(5274.04),  // E
    SYNTH_HZ_TO_PHASE(5587.65),  // F
    SYNTH_HZ_TO_PHASE(5919.91),  // F#
    SYNTH_HZ_TO_PHASE(6271.93),  // G
    SYNTH_HZ_TO_PHASE(6644.88),  // G#
    SYNTH_HZ_TO_PHASE(7040.00),  // A
    SYNTH_HZ_TO_PHASE(7458.62),  // A#
    SYNTH_HZ_TO_PHASE(7902.13),  // B
};

q15_t midiToPhaseIncr(uint8_t note) {
    int octave = note / 12;
    int noteIndex = note - (octave * 12);
    q15_t phaseIncr = octavePhases[noteIndex];

    phaseIncr >>= (BASE_OCTAVE - octave + 1);
    return phaseIncr;
}

void synthVoiceNoteOn(SynthVoice_t *voice, uint8_t note) {
    voice->note = note;
    voice->gate = 1;
    voice->phaseIncrement = midiToPhaseIncr(note);
    // reset the state of all nodes
    for (int i = 0; i < voice->synth_nodes; i++) {
        SynthNode_t *node = &voice->nodes[i];
        node->state = 0;
    }
}
void synthVoiceNoteOff(SynthVoice_t *voice) { voice->gate = 0; }

void synthInitOscNode(SynthNode_t *node, q15_t *gain, q15_t *phaseIncrement, q15_t *detune, q15_t (*wavegen)(q15_t input)) {
    memset(node, 0, sizeof(SynthNode_t));
    node->gain = gain;
    node->type = SYNTH_NODE_OSCILLATOR;
    node->osc.phaseIncrement = phaseIncrement;
    node->osc.detune = detune;
    node->osc.wavegen = wavegen;
}

void synthInitEnvelopeNode(SynthNode_t *node, q15_t *gain, q15_t attack, q15_t decay, q15_t sustain, q15_t release) {
    memset(node, 0, sizeof(SynthNode_t));
    node->gain = gain;
    node->type = SYNTH_NODE_ENVELOPE;
    node->env.attack = attack;
    node->env.decay = decay;
    node->env.sustain = sustain;
    node->env.release = release;
}
void synthInitFilterLpNode(SynthNode_t *node, q15_t *gain, q15_t *input, q15_t factor) {
    memset(node, 0, sizeof(SynthNode_t));
    node->gain = gain;
    node->type = SYNTH_NODE_FILTER_LP;
    node->filter.input = input;
    node->filter.factor = factor;
}
void synthInitFilterHpNode(SynthNode_t *node, q15_t *gain, q15_t *input, q15_t factor) {
    memset(node, 0, sizeof(SynthNode_t));
    node->gain = gain;
    node->type = SYNTH_NODE_FILTER_HP;
    node->filter.input = input;
    node->filter.factor = factor;
}
void synthInitMixerNode(SynthNode_t *node, q15_t *gain, q15_t *input1, q15_t *input2, q15_t *input3) {
    memset(node, 0, sizeof(SynthNode_t));
    node->gain = gain;
    node->type = SYNTH_NODE_MIXER;
    node->mixer.inputs[0] = input1;
    node->mixer.inputs[1] = input2;
    node->mixer.inputs[2] = input3;
}

q15_t synthProcess(Synth_t *synth) {
    int32_t mainOutput = 0;
    for (int vi = 0; vi < synth->synth_voices; vi++) {
        SynthVoice_t *voice = &synth->synthVoices[vi];
        SynthNode_t *nodes = voice->nodes;
        // first pass, generate outputs from current state

        for (int i = 0; i < voice->synth_nodes && nodes[i].type != SYNTH_NODE_NONE; i++) {
            synth->outputs[i] = 0;
            SynthNode_t *node = &nodes[i];
            q15_t tmp;
            switch (node->type) {
                case SYNTH_NODE_OSCILLATOR:
                    tmp = node->state & 0x7FFF;  // mask to positive q15
                    synth->outputs[i] = node->osc.wavegen(tmp);
                    break;
                case SYNTH_NODE_ENVELOPE:
                    synth->outputs[i] = (node->state & 0x7FFFFF) >> 4;
                    synth->outputs[i] = (synth->outputs[i] * synth->outputs[i]) >> 15;  // square the envelope
                    // if sustain is negative, this is a negative envelope, so invert the output
                    if (node->env.sustain < 0) {
                        synth->outputs[i] = -synth->outputs[i];
                    }
                    break;
                case SYNTH_NODE_FILTER_LP:
                    synth->outputs[i] = (node->filter.accum * node->filter.factor) >> 15;
                    break;
                case SYNTH_NODE_FILTER_HP:
                    // same as lp, then subtract from input
                    synth->outputs[i] = (node->filter.accum * node->filter.factor) >> 15;
                    synth->outputs[i] = *node->filter.input - synth->outputs[i];
                    break;
                case SYNTH_NODE_MIXER: {
                    int32_t sum = 0;
                    // tmp = 0;
                    for (int j = 0; j < 3; j++) {
                        if (node->mixer.inputs[j]) {
                            sum += *node->mixer.inputs[j];
                            // tmp++;
                        }
                    }
                    // NOTE: this can be done in the gain setting, can avoid extra calc
                    //  if (tmp == 2) {
                    //      sum >>= 1; //divide by 2 by shifting
                    //  } else if (tmp == 3) {
                    //      sum = sum * 10922 >> 15; //divide by 3 by multiplying by 1/3 in q15
                    //  }
                    synth->outputs[i] = sum;
                    break;
                }
                default:
                    break;
            }

            if (node->gain) {
                // TODO for now, just linear gain
                // positive gain should amplify the signal, while negative gain should attenuate the signal
                // but also need to pair well with envelop generators
                synth->outputs[i] = (synth->outputs[i] * *node->gain) >> 15;
            }
        }

        // second pass, update state
        for (int i = 0; i < voice->synth_nodes && nodes[i].type != SYNTH_NODE_NONE; i++) {
            SynthNode_t *node = &nodes[i];
            node->output = synth->outputs[i];
            switch (node->type) {
                case SYNTH_NODE_OSCILLATOR:
                    // add in the phase increment and detune
                    node->state += *node->osc.phaseIncrement;
                    if (node->osc.detune) {
                        node->state += *node->osc.detune;
                    }
                    // TODO is this needed? would negative increments and state be OK if we mask it before wavegen?
                    node->state &= 0x7FFF;  // wrap around q15
                    break;
                case SYNTH_NODE_ENVELOPE:
                    // the top bit of state is reserved for the decay mode (after attack)
                    if (voice->gate) {  // while gate is on, do attack, then decay
                        int modeBit = node->state & 0x80000000;
                        int32_t value = node->state & 0x7FFFFFFF;
                        if (modeBit) {
                            // decay
                            value -= node->env.decay;
                            const q15_t sustainAbs = node->env.sustain < 0 ? -node->env.sustain : node->env.sustain;
                            if (value < sustainAbs << 4) {
                                value = sustainAbs << 4;
                            }
                        } else {
                            // attack
                            value += node->env.attack;
                            if (value > (int32_t)Q15_MAX << 4) {
                                value = (int32_t)Q15_MAX << 4;
                                modeBit = 0x80000000;  // switch to decay
                            }
                        }
                        node->state = value | modeBit;
                    } else {  // when gate is off, do release
                        // clear the modeBit (if set), so the next gate on will start with attack
                        node->state &= 0x7FFFFFFF;
                        node->state -= node->env.release;
                        if (node->state < 0) {
                            node->state = 0;
                        }
                    }
                    break;
                case SYNTH_NODE_FILTER_LP:
                    node->filter.accum += (*node->filter.input - node->output);
                    break;
                case SYNTH_NODE_FILTER_HP:
                    node->filter.accum += (*node->filter.input - node->output);
                    break;
                case SYNTH_NODE_MIXER:
                    break;
                default:
                    break;
            }
        }

        // add the output of the voice to the main output
        mainOutput += voice->nodes[0].output;
    }

    if (synth->synth_voices > 1) {
        const q15_t mainMixerGain = Q15_MAX / synth->synth_voices;
        return (mainOutput * mainMixerGain) >> 15;
    } else {
        return mainOutput;
    }
}

// these wave generator functions all take a basic ramping sawtooth between 0 and 1 as input
// and return a waveform between -1 and 1

// lut = []; for (i = 0; i < 128; i++) {a = (i/128) * Math.PI*2 ; lut[i] = Math.round(Math.sin(a) * 127);}; console.log(JSON.stringify(lut))
#if SYNTH_SINE_LUT_8BIT
static const q7_t sineLut[128 + 1] = {
    0,    6,    12,   19,   25,   31,   37,   43,   49,   54,   60,   65,   71,   76,   81,   85,   90,   94,   98,   102,  106,  109,  112,  115,  117,  120,
    122,  123,  125,  126,  126,  127,  127,  127,  126,  126,  125,  123,  122,  120,  117,  115,  112,  109,  106,  102,  98,   94,   90,   85,   81,   76,
    71,   65,   60,   54,   49,   43,   37,   31,   25,   19,   12,   6,    0,    -6,   -12,  -19,  -25,  -31,  -37,  -43,  -49,  -54,  -60,  -65,  -71,  -76,
    -81,  -85,  -90,  -94,  -98,  -102, -106, -109, -112, -115, -117, -120, -122, -123, -125, -126, -126, -127, -127, -127, -126, -126, -125, -123, -122, -120,
    -117, -115, -112, -109, -106, -102, -98,  -94,  -90,  -85,  -81,  -76,  -71,  -65,  -60,  -54,  -49,  -43,  -37,  -31,  -25,  -19,  -12,  -6,
    0  // extra value to make interpolation easier
};

#else

static const q15_t sineLut16[256 + 1] = {
    0,      804,    1608,   2410,   3212,   4011,   4808,   5602,   6393,   7179,   7962,   8739,   9512,   10278,  11039,  11793,  12539,  13279,  14010,  14732,  15446,  16151,
    16846,  17530,  18204,  18868,  19519,  20159,  20787,  21403,  22005,  22594,  23170,  23731,  24279,  24811,  25329,  25832,  26319,  26790,  27245,  27683,  28105,  28510,
    28898,  29268,  29621,  29956,  30273,  30571,  30852,  31113,  31356,  31580,  31785,  31971,  32137,  32285,  32412,  32521,  32609,  32678,  32728,  32757,  32767,  32757,
    32728,  32678,  32609,  32521,  32412,  32285,  32137,  31971,  31785,  31580,  31356,  31113,  30852,  30571,  30273,  29956,  29621,  29268,  28898,  28510,  28105,  27683,
    27245,  26790,  26319,  25832,  25329,  24811,  24279,  23731,  23170,  22594,  22005,  21403,  20787,  20159,  19519,  18868,  18204,  17530,  16846,  16151,  15446,  14732,
    14010,  13279,  12539,  11793,  11039,  10278,  9512,   8739,   7962,   7179,   6393,   5602,   4808,   4011,   3212,   2410,   1608,   804,    0,      -804,   -1608,  -2410,
    -3212,  -4011,  -4808,  -5602,  -6393,  -7179,  -7962,  -8739,  -9512,  -10278, -11039, -11793, -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868,
    -19519, -20159, -20787, -21403, -22005, -22594, -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790, -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956,
    -30273, -30571, -30852, -31113, -31356, -31580, -31785, -31971, -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757, -32767, -32757, -32728, -32678, -32609, -32521,
    -32412, -32285, -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571, -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683, -27245, -26790, -26319, -25832,
    -25329, -24811, -24279, -23731, -23170, -22594, -22005, -21403, -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279, -12539, -11793,
    -11039, -10278, -9512,  -8739,  -7962,  -7179,  -6393,  -5602,  -4808,  -4011,  -3212,  -2410,  -1608,  -804,
    0  // extra value to make interpolation easier
};
#endif

q15_t sawtoothWave(q15_t input) { return input * 2 - Q15_MAX; }

q15_t sineWave(q15_t input) {
#if SYNTH_SINE_LUT_8BIT
    int index = (input >> 8) & 0x7F;
    q15_t res = sineLut[index] * 258;  // multiply by 258 to get full range
#if SYNTH_INTERPOLATE
    q15_t next = sineLut[index + 1] * 258;
    res += ((next - res) * (input & 0xFF) >> 8);
#endif

#else  // 16 bit
    int index = (input >> 7) & 0xFF;
    q15_t res = sineLut16[index];
#if SYNTH_INTERPOLATE
    q15_t next = sineLut16[index + 1];
    res += ((next - res) * (input & 0x7F) >> 7);
#endif

#endif  // 16
    return res;
}

q15_t squareWave(q15_t input) { return input < Q15_MAX / 2 ? Q15_MAX : Q15_MIN; }

q15_t triangleWave(q15_t input) {
    int32_t res = input << 1;
    if (res > Q15_MAX) {
        res = Q15_MAX - (res - Q15_MAX);
    }
    return res * 2 - Q15_MAX;
}

// sawtooth wave falling
q15_t fallingWave(q15_t input) { return Q15_MAX - input * 2; }

// exponentially decaying
q15_t expDecayWave(q15_t input) {
    // invert input so that it decays
    input = (Q15_MAX - input);
    input = (input * input) >> 15;
    input = (input * input) >> 15;
    return input;
}

q15_t noise(void) {
    // based on ranqd1 random number generator from Numerical Recipes
    // it will cycle every possible 32 bit value before repeating
    static uint32_t seed = 0x12345678;
    seed = (seed * 1664525 + 1013904223);
    return seed & 0xFFFF;
}

// sine based clipper. close to linear up to 50% of q15, then smooths out
q15_t softClipper(int32_t input) {
    // use the first quadrant of a sine wave as a compressor
    int sign = input < 0 ? -1 : 1;
    input = input > 0 ? input : -input;
    int32_t a = input >> 3;  // divide by 8, a 50% vallue will end up around 70%, while higher values will be compressed with the top of the sine wave
    // clip to a quadrant
    if (a > Q15_MAX / 4) {
        // __asm("BKPT #0\n");
        a = Q15_MAX / 4;
    }
    q15_t res = sineWave(a);
    res = (res * sign);
    return res;
}

Synth_t *synthCreate(uint32_t numVoices, uint32_t numNodes) {
    // alloc parent struct
    Synth_t *synth = calloc(1, sizeof(Synth_t));
    if (!synth) {
        return NULL;
    }

    // alloc outputs
    synth->outputs = calloc(numNodes, sizeof(int32_t));  // store the outputs of each node during calc, then update after

    // alloc voices
    synth->synth_voices = numVoices;
    synth->synthVoices = calloc(numVoices, sizeof(SynthVoice_t));
    if (!synth->synthVoices) {
        synthDestroy(synth);
        return NULL;
    }

    // alloc nodes
    for (int i = 0; i < synth->synth_voices; i++) {
        synth->synthVoices[i].synth_nodes = numNodes;
        synth->synthVoices[i].nodes = calloc(numNodes, sizeof(SynthNode_t));
        if (!synth->synthVoices[i].nodes) {
            synthDestroy(synth);
            return NULL;
        }
    }

    return synth;
}

void synthDestroy(Synth_t *synth) {
    if (synth) {
        for (int i = 0; i < synth->synth_voices; i++) {
            free(synth->synthVoices[i].nodes);
        }
        free(synth->synthVoices);
        free(synth->outputs);
        free(synth);
    }
}
