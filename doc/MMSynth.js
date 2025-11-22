/**
 * This is a software synthesizer based on https://github.com/simap/MicroModSynth.
 * 
 * **Note: MMSynth module must be loaded by calling LoadLibrary("mmsynth") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @class
 * 
 * @param {string} voices number of voices to allocate.
 * @param {number} nodes number of nodes to allocate per voice.
 * @param {number} [buffersize] playback buffer size, default is 4KiB.
 */
function MMSynth(voices, nodes, bufferSize) {
	/**
	 * sample rate.
	 * @member {number}
	 */
	this.samplerate = 0;
	/**
	 * playback buffer size
	 * @member {number}
	 */
	this.buffer_size = 0;
	/**
	 * number of voices
	 * @member {number}
	 */
	this.synth_voices = 0;
	/**
	 * number of nodes per voice
	 * @member {number}
	 */
	this.synth_nodes = 0;

	/**
	 * minimum value for numbers.
	 * @member {number}
	 */
	this.MAX = 0;
	/**
	 * maximum value for numbers.
	 * @member {number}
	 */
	this.MIN = 0;

	/**
	 * constant for SAWTOOTH.
	 * @member {number}
	 */
	this.SAWTOOTH = 0;
	/**
	 * constant for SINE.
	 * @member {number}
	 */
	this.SINE = 0;
	/**
	 * constant for SQUARE.
	 * @member {number}
	 */
	this.SQUARE = 0;
	/**
	 * constant for TRIANGLE.
	 * @member {number}
	 */
	this.TRIANGLE = 0;
	/**
	 * constant for FALLING.
	 * @member {number}
	 */
	this.FALLING = 0;
	/**
	 * constant for EXP_DECAY.
	 * @member {number}
	 */
	this.EXP_DECAY = 0;
};

/**
 * Close synthesizer.
 */
MMSynth.prototype.Close = function () { }


/**
 * must be called periodically (e.g. every Loop()) to update the playback buffer
 */
MMSynth.prototype.Play = function () { }

/**
 * convert Hz to Phase.
 * 
 * @param {number} value in Hz.
 * 
 * @return {number} the value converted to phase.
 */
MMSynth.prototype.HzToPhase = function (hz) { }

/**
 * play a note on a voice.
 * 
 * @param {number} voice the voice to play.
 * @param {number} note the note in play (midi note number).
 * 
 * @see https://computermusicresource.com/midikeys.html
 */
MMSynth.prototype.NoteOn = function (voice, note) { }

/**
 * silence voice.
 * 
 * @param {number} voice the voice to turn off
 */
MMSynth.prototype.NoteOff = function (voice) { }

/**
 * check if the given node for given voice is defined.
 * 
 * @param {number} voice the voice to check.
 * @param {number} node the node to check.
 * 
 * @return {bool} true if the node is defined, else false.
 */
MMSynth.prototype.IsNodeDefined = function (voice, node) { };

/**
 * return node as JS object.
 * 
 * @param {number} voice the voice to get
 * @param {number} node the noce to get
 * 
 * @return {*} returns an object representing the node. Will contain "type"=[null,"Oscillator","Envelope","FilterLp","FilterHp","Mixer"]. Will also contain properties matching the parameters when creating the node. 
 */
MMSynth.prototype.GetNode = function (voice, node) { };

/**
 * create an envelope node.
 * 
 * @param {number} voice The voice where the node shall be created (0..v).
 * @param {number} node The node to create (0..n).
 * @param {number} gain <0 to select another nodes output as gain (node #0 is -1, node #1 is -2, etc), >=0 to use a numeric value as gain or null for no gain.
 * @param {number} attack attack value.
 * @param {number} decay decay value.
 * @param {number} sustain sustain value.
 * @param {number} release release value.
 */
MMSynth.prototype.EnvelopeNode = function (voice, node, gain, attack, decay, sustain, release) { }

/**
 * create an oscillator node.
 * 
 * @param {number} voice The voice where the node shall be created (0..v).
 * @param {number} node The node to create (0..n).
 * @param {number} gain <0 to select another nodes output as gain (node #0 is -1, node #1 is -2, etc), >=0 to use a numeric value as gain or null for no gain.
 * @param {number} phaseIncrement <0 to select another nodes output as phaseIncrement (node #0 is -1, node #1 is -2, etc), >=0 to use a numeric value as phaseIncrement or null to get the value from the notes value.
 * @param {number} detune <0 to select another nodes output as detune (node #0 is -1, node #1 is -2, etc), >=0 to use a numeric value as detune.
 * @param {number} wavegen One of MMSynth.SAWTOOTH, MMSynth.SINE, MMSynth.SQUARE, MMSynth.TRIANGLE, MMSynth.FALLING, MMSynth.EXP_DECAY.
 */
MMSynth.prototype.OscillatorNode = function (voice, node, gain, phaseIncrement, detune, wavegen) { }

/**
 * create a LP filter node.
 * 
 * @param {number} voice The voice where the node shall be created (0..v).
 * @param {number} node The node to create (0..n).
 * @param {number} gain <0 to select another nodes output as gain (node #0 is -1, node #1 is -2, etc), >=0 to use a numeric value as gain or null for no gain.
 * @param {number} input <0 to select another nodes output as input (node #0 is -1, node #1 is -2, etc), >=0 to use a numeric value as input or null for no input.
 * @param {number} factor filter value.
 */
MMSynth.prototype.FilterLpNode = function (voice, node, gain, input, factor) { }

/**
 * create a HP filter node.
 * 
 * @param {number} voice The voice where the node shall be created (0..v).
 * @param {number} node The node to create (0..n).
 * @param {number} gain <0 to select another nodes output as gain (node #0 is -1, node #1 is -2, etc), >=0 to use a numeric value as gain or null for no gain.
 * @param {number} input <0 to select another nodes output as input (node #0 is -1, node #1 is -2, etc), >=0 to use a numeric value as input or null for no input.
 * @param {number} factor filter value.
 */
MMSynth.prototype.FilterHpNode = function (voice, node, gain, input, factor) { }

/**
 * create a mixer node.
 * 
 * @param {number} voice The voice where the node shall be created (0..v).
 * @param {number} node The node to create (0..n).
 * @param {number} gain <0 to select another nodes output as gain (node #0 is -1, node #1 is -2, etc), >=0 to use a numeric value as gain or null for no gain.
 * @param {number} input1 <0 to select another nodes output as input (node #0 is -1, node #1 is -2, etc), >=0 to use a numeric value as input or null for no input.
 * @param {number} input2 <0 to select another nodes output as input (node #0 is -1, node #1 is -2, etc), >=0 to use a numeric value as input or null for no input.
 * @param {number} input3 <0 to select another nodes output as input (node #0 is -1, node #1 is -2, etc), >=0 to use a numeric value as input or null for no input.
 */
MMSynth.prototype.MixerNode = function (voice, node, gain, input1, input2, input3) { }
