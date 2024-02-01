/**
 * Load a WAV-file.
 * @class
 * @param {string} filename 
 */
function Sample(filename) {
	/**
	 * Name of the WAV.
	 * @member {string}
	 */
	this.filename = null;
	/**
	 * Sound length.
	 * @member {number}
	 */
	this.length = 0;
	/**
	 * Sound frequency.
	 * @member {number}
	 */
	this.frequency = 0;
	/**
	 * Sound resolution.
	 * @member {number}
	 */
	this.bits = 0;
	/**
	 * mono/stereo indicator.
	 * @member {boolean}
	 */
	this.stereo = false;

}
/**
 * Play the WAV.
 * @param {number} volume between 0-255.
 * @param {number} panning between (left) 0-255 (right).
 * @param {boolean} loop true := sample will loop, false := sample will only be played once.
 * @returns {number} used voice number or null if not played.
 */
Sample.prototype.Play = function (volume, panning, loop) { };
/**
 * Stop playing.
 */
Sample.prototype.Stop = function () { };
/**
 * Get sample data.
 * @param {number} idx index to return.
 * @returns {number} The sample value at that position. The sample data are always in unsigned format.
 */
Sample.prototype.Get = function (idx) { };
