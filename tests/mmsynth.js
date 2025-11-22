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

LoadLibrary("mmsynth");

function Setup() {
	MouseShowCursor(false);

	m = new MmSynth(
		2,		// number of voices
		8, 		// max number of nodes/voice
		1024	// audio buffer size
	);

	Println(m.samplerate);
	Println(m.buffer_size);
	Println(m.synth_voices);
	Println(m.synth_nodes);
	Println(m.MIN);
	Println(m.MAX);

	var lfoPhaseInc = m.HzToPhase(5);
	var vibratoInc = m.HzToPhase(10);


	//////
	// voice 1
	m.EnvelopeNode(
		1, 					// voice #1
		2, 					// node #2
		null, 				// gain (none)
		500, 				// attack:=500 
		150, 				// decay:=150 
		m.MAX * 0.8,	 	// sustain (MAX*0.8)
		150					// release:=150
	);
	m.OscillatorNode(
		1, 					// voice #1
		4, 					// node #4
		-2, 				// gain (from node #2)
		null, 				// phaseIncrement (from midi note)
		-3, 				// detune (from node #3)
		m.SAWTOOTH			// waveGen:=sawtooth
	);
	m.OscillatorNode(
		1, 					// voice #1
		3, 					// node #3
		vibratoInc, 		// gain (10Hz)
		lfoPhaseInc,		// phaseIncrement (5Hz)
		null, 				// detune (none)
		m.SINE				// waveGen:=sine
	);
	m.FilterLpNode(
		1,					// voice #1
		1,					// node #1; OUTPUT NODE
		null,				// gain (none)
		-4,					// input (node #4)
		8000				// factor:=4000
	);

	//////
	// voice 2
	m.EnvelopeNode(
		2, 					// voice #2
		2, 					// node #2
		null, 				// gain (none)
		500, 				// attack:=500 
		150, 				// decay:=150 
		m.MAX * 0.5,	 	// sustain (MAX/2)
		15					// release:=15
	);
	m.OscillatorNode(
		2, 					// voice #2
		3, 					// node #3
		-2,					// gain (from node #2)
		null, 				// detune (from midi note)
		m.SQUARE			// waveGen:=square
	);
	m.FilterLpNode(
		2,					// voice #2
		1,					// node #1; OUTPUT NODE
		null,				// gain (none)
		-3,					// input (node #3)
		4000				// factor:=4000
	);

	for (var v = 1; v <= 2; v++) {
		for (var n = 1; n <= 8; n++) {
			Println("Voice #" + v + "Node #" + n + " := " + JSON.stringify(m.GetNode(v, n)));
		}
		Println("");
	}

	SetFramerate(20);
}

// twinkle twinkle little star in midi notes. use note 0 to indicate rest
var twinkleTwinkle = [
	60, 60, 67, 67, 69, 69, 67, 0,
	65, 65, 64, 64, 62, 62, 60, 0,
	67, 67, 65, 65, 64, 64, 62, 0,
	67, 67, 65, 65, 64, 64, 62, 0,
	60, 60, 67, 67, 69, 69, 67, 0,
	65, 65, 64, 64, 62, 62, 60, 0
];

var twinkleTwinkleBeats = [
	4, 4, 4, 4, 4, 4, 2, 2,
	4, 4, 4, 4, 4, 4, 2, 2,
	4, 4, 4, 4, 4, 4, 2, 2,
	4, 4, 4, 4, 4, 4, 2, 2,
	4, 4, 4, 4, 4, 4, 2, 2,
	4, 4, 4, 4, 4, 4, 2, 2
];

var noteDuration = 0;
var noteIndex = 0;

var i = 0;

function Loop() {
	if (noteDuration <= 0) {
		noteDuration = 64 / twinkleTwinkleBeats[noteIndex];
		var note = twinkleTwinkle[noteIndex];
		if (note) {
			m.NoteOn(1, note);
			m.NoteOn(2, note - 24);
		}
		noteIndex++;
		if (noteIndex >= twinkleTwinkle.length) {
			Stop();
		}
	} else if (noteDuration < 5) {
		// cut the note short slightly to allow decay
		m.NoteOff(1);
		m.NoteOff(2);
	}
	noteDuration--;

	m.Play();

	i++;
}

function Input(e) {
}
