# Micro Modular Synth

Designed for microcontrollers, this uses little RAM and flash.
Intended for 32-bit microcontrollers that have a fast multiply, like ARM Cortex-M0.

The idea is that you have some number of voices, these are generators, could either be different "patches" or the same if you wanted polyphony. Then each voice has a number of audio nodes that can be envelopes, oscillators, filters, mixers, etc. Every node has an output and gain pointer input, and then specialized by type. It's fairly lightweight, but does have some overhead.

On an STM32G030 at 16MHz, it can do 2 voices with 4-5 nodes each with a sample rate of 12,500Hz. 

You call synthVoiceNoteOn(SynthVoice_t *voice, uint8_t note) (passing midi notes) and synthVoiceNoteOff(SynthVoice_t *voice) and it does the rest. Or set the voice's phaseIncrement to anything if you want something that isn't a midi note.

# Example / Test
[This audio example](output.wav) is a super basic sequencer playing twinkle twinkle little star. Two voices are used: A brassy sawtooth with vibrato, and a lowpass square wave bass 2 octaves below it.

https://github.com/user-attachments/assets/e7a0665f-ba6d-4196-ace2-f6fcd5c5edee

## Running the test

  gcc test.c src/synth.c -I src -o test ; ./test
