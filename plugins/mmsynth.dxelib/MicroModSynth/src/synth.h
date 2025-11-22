// a super fast simple sound synthesis engine
//  2025 Ben Hencke
#ifndef __SYNTH_H
#define __SYNTH_H

#include <stdint.h>
#include <stdlib.h>

// CONFIG stuff

// use 8 bit 128 sample LUT for sine wave, or 16 bit 256 sample LUT
#define SYNTH_SINE_LUT_8BIT 1
// enable interpolation between LUT values
#define SYNTH_INTERPOLATE 1

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 11025
#endif

#ifndef q15_t
typedef int16_t q15_t;
#endif
#ifndef Q15_MAX
#define Q15_MAX 0x7FFF
#endif
#ifndef Q15_MIN
#define Q15_MIN 0x8000
#endif

#ifndef q7_t
typedef int8_t q7_t;
#endif
#ifndef Q7_MAX
#define Q7_MAX 0x7F
#endif
#ifndef Q7_MIN
#define Q7_MIN 0x80
#endif

// idea, generic pointer wired nodes, can be mostly automated by engine after wired up
// when a note triggers, all phases are reset. osc and lfo and envelope state.
// envelopes could have two outputs, one negated, or maybe a special mode.
// oscillators have a pitch and gain input.

// oscilators use a phase accumulator to generate waveforms, which is fast since its just added each sample
// phase stays positive, wrapps at Q15_MAX (15 bits)
// phase incremeent2 is used for FM, and is added to the phase increment while keeping the base frequency
typedef struct SynthOscillator {
    q15_t *phaseIncrement;          // calculated from frequency
    q15_t phaseIncrementVal;        // value
    q15_t *detune;                  //+- phase increment for FM
    q15_t detuneVal;                //+- value
    q15_t (*wavegen)(q15_t input);  // waveform generator function
} SynthOscillator_t;

// basic envelope generator
typedef struct SynthEnvelope {
    q15_t attack;
    q15_t decay;
    q15_t sustain;
    q15_t release;
} SynthEnvelope_t;

// basic filter
typedef struct SynthFilter {
    q15_t *input;
    q15_t inputVal;
    int32_t accum;
    int32_t factor;
} SynthFilter_t;

// basic mixer
typedef struct SynthMixer {
    q15_t *inputs[3];
    q15_t inputsVals[3];
} SynthMixer_t;

typedef enum SynthNodeType {
    SYNTH_NODE_NONE = 0,
    SYNTH_NODE_OSCILLATOR,
    SYNTH_NODE_ENVELOPE,
    SYNTH_NODE_FILTER_LP,
    SYNTH_NODE_FILTER_HP,
    SYNTH_NODE_MIXER,
    SYNTH_NODE_END
} SynthNodeType_t;

typedef struct SynthNode {
    int32_t state;  // state for the node, could be phase, envelope state, etc. this gets reset when a note is triggered (aka gate)
    q15_t *gain;    // pointer to gain input
    q15_t gainVal;  // space for values
    q15_t output;
    SynthNodeType_t type;
    uint8_t param1;  // TODO maybe use this as gain range
    union {
        struct SynthOscillator osc;
        struct SynthEnvelope env;
        struct SynthFilter filter;
        struct SynthMixer mixer;
    };
} SynthNode_t;

typedef struct SynthVoice {
    uint8_t note;          // midi note
    uint8_t gate : 1;      // gate on/off
    q15_t phaseIncrement;  // calculated from frequency
    SynthNode_t *nodes;
    uint32_t synth_nodes;
} SynthVoice_t;

typedef struct Synth {
    SynthVoice_t *synthVoices;
    uint32_t synth_voices;
    int32_t *outputs;  // store the outputs of each node during calc, then update after
} Synth_t;

Synth_t *synthCreate(uint32_t numVoices, uint32_t numNodes);
void synthDestroy(Synth_t *synth);

q15_t midiToPhaseIncr(uint8_t note);

void synthVoiceNoteOn(SynthVoice_t *voice, uint8_t note);
void synthVoiceNoteOff(SynthVoice_t *voice);

void synthInitOscNode(SynthNode_t *node, q15_t *gain, q15_t *phaseIncrement, q15_t *detune, q15_t (*wavegen)(q15_t input));
void synthInitEnvelopeNode(SynthNode_t *node, q15_t *gain, q15_t attack, q15_t decay, q15_t sustain, q15_t release);
void synthInitFilterLpNode(SynthNode_t *node, q15_t *gain, q15_t *input, q15_t factor);
void synthInitFilterHpNode(SynthNode_t *node, q15_t *gain, q15_t *input, q15_t factor);
void synthInitMixerNode(SynthNode_t *node, q15_t *gain, q15_t *input1, q15_t *input2, q15_t *input3);

q15_t synthProcess(Synth_t *synth);

q15_t sawtoothWave(q15_t input);
q15_t sineWave(q15_t input);
q15_t squareWave(q15_t input);
q15_t triangleWave(q15_t input);
q15_t fallingWave(q15_t input);
q15_t expDecayWave(q15_t input);
q15_t noise(void);
q15_t softClipper(int32_t input);

#define SYNTH_HZ_TO_PHASE(frequency) ((frequency * Q15_MAX) / SAMPLE_RATE)

// convert milliseconds to samples
#define SYNTH_MS(ms) ((ms * SAMPLE_RATE) / 1000)

#endif  // __SYNTH_H