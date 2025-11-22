
#include "synth.h"
#include "stdio.h"
#include "stdlib.h"

q15_t half = Q15_MAX / 2;
q15_t lfoPhaseInc = SYNTH_HZ_TO_PHASE(5);
q15_t vibratoInc = SYNTH_HZ_TO_PHASE(10);

int writeWav(char *filename, int16_t *audioBuffer, uint32_t sampleCount);

int main() {
    Synth_t *s = synthCreate(2, 8);

    // wire up an oscillator to an envelope. envelope controls gain and detune
    // convention: the first node in a chain is the output node
    SynthVoice_t *voice = &s->synthVoices[0];
    synthInitEnvelopeNode(&voice->nodes[1],
                          NULL,          // gain
                          500,           // attack
                          150,           // decay
                          Q15_MAX * .8,  // sustain
                          150            // release
    );
    synthInitOscNode(&voice->nodes[3],
                     &voice->nodes[1].output,  // gain - from envelope
                     &voice->phaseIncrement,   // phase increment - from midi note
                     &voice->nodes[2].output,  // detune - from envelope
                     sawtoothWave              // sawtoothWave sineWave triangleWave squareWave  fallingWave expDecayWave
    );
    // LFO
    synthInitOscNode(&voice->nodes[2],
                     &vibratoInc,   // gain
                     &lfoPhaseInc,  // phase increment
                     // &voice->nodes[1].output,  //detune - from envelope
                     NULL,     // detune
                     sineWave  // sawtoothWave sineWave triangleWave squareWave  fallingWave expDecayWave
    );
    synthInitFilterLpNode(&voice->nodes[0],
                          NULL,                     // gain
                          &voice->nodes[3].output,  // input
                          8000                      // factor
    );

    voice = &s->synthVoices[1];
    synthInitEnvelopeNode(&voice->nodes[1],
                          NULL,           // gain
                          100,            // attack
                          500,            // decay
                          Q15_MAX * 0.5,  // sustain
                          15              // release
    );
    synthInitOscNode(&voice->nodes[2],
                     &voice->nodes[1].output,  // gain - from envelope
                     &voice->phaseIncrement,   // phase increment - from midi note
                     // &voice->nodes[1].output,  //detune - from envelope
                     NULL,
                     squareWave  // sawtoothWave sineWave triangleWave squareWave  fallingWave expDecayWave
    );
    synthInitFilterLpNode(&voice->nodes[0],
                          NULL,                     // gain
                          &voice->nodes[2].output,  // input
                          4000                      // factor
    );

    // fill up a buffer with audio
    int16_t *audioBuffer = malloc(SAMPLE_RATE * 60);  // 60 seconds
    uint32_t sampleCount = 0;

    // twinkle twinkle little star in midi notes. use note 0 to indicate rest
    const uint8_t twinkleTwinkle[] = {60, 60, 67, 67, 69, 69, 67, 0, 65, 65, 64, 64, 62, 62, 60, 0, 67, 67, 65, 65, 64, 64, 62, 0,
                                      67, 67, 65, 65, 64, 64, 62, 0, 60, 60, 67, 67, 69, 69, 67, 0, 65, 65, 64, 64, 62, 62, 60, 0};

    const uint8_t twinkleTwinkleBeats[] = {4, 4, 4, 4, 4, 4, 2, 2, 4, 4, 4, 4, 4, 4, 2, 2, 4, 4, 4, 4, 4, 4, 2, 2,
                                           4, 4, 4, 4, 4, 4, 2, 2, 4, 4, 4, 4, 4, 4, 2, 2, 4, 4, 4, 4, 4, 4, 2, 2};

    uint32_t noteDuration = 0;
    uint32_t noteIndex = 0;
    for (;;) {
        if (noteDuration == 0) {
            noteDuration = SYNTH_MS(2000 / twinkleTwinkleBeats[noteIndex]);
            uint8_t note = twinkleTwinkle[noteIndex];
            if (note) {
                synthVoiceNoteOn(&s->synthVoices[0], note);
                synthVoiceNoteOn(&s->synthVoices[1], note - 24);
            }
            noteIndex++;
            if (noteIndex >= sizeof(twinkleTwinkle)) {
                break;
            }
        } else if (noteDuration < 500) {
            // cut the note short slightly to allow decay
            synthVoiceNoteOff(&s->synthVoices[0]);
            synthVoiceNoteOff(&s->synthVoices[1]);
        }
        noteDuration--;

        q15_t v = synthProcess(s);
        audioBuffer[sampleCount++] = v;
    }

    // write the audio buffer to a wav file
    int res = writeWav("output.wav", audioBuffer, sampleCount);
    free(audioBuffer);
    return res;
}

int writeWav(char *filename, int16_t *audioBuffer, uint32_t sampleCount) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        printf("Failed to open output file\n");
        return 1;
    }
    uint32_t sampleRate = SAMPLE_RATE;
    // write the wav header
    uint32_t fileSize = sampleCount * 2 + 36;
    uint32_t byteRate = SAMPLE_RATE * 2;
    uint32_t blockAlign = 2;
    uint32_t bitsPerSample = 16;
    uint32_t dataSize = sampleCount * 2;
    fwrite("RIFF", 1, 4, f);
    fwrite(&fileSize, 4, 1, f);
    fwrite("WAVE", 1, 4, f);
    fwrite("fmt ", 1, 4, f);
    uint32_t fmtSize = 16;
    fwrite(&fmtSize, 4, 1, f);
    uint16_t format = 1;
    fwrite(&format, 2, 1, f);
    uint16_t channels = 1;
    fwrite(&channels, 2, 1, f);
    fwrite(&sampleRate, 4, 1, f);
    fwrite(&byteRate, 4, 1, f);
    fwrite(&blockAlign, 2, 1, f);
    fwrite(&bitsPerSample, 2, 1, f);
    fwrite("data", 1, 4, f);
    fwrite(&dataSize, 4, 1, f);
    fwrite(audioBuffer, 2, sampleCount, f);
    fclose(f);
    return 0;
}