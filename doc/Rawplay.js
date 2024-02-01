/**
 * load RAW sound file (signed 16 bit, stereo, interleaved) for playback. This was just a test module for vorbis and mpeg1 sound output but I decided to keep it.
 * 
 * **Note: Rawplay module must be loaded by calling LoadLibrary("rawplay") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @class
 * 
 * @param {string} filename file name of the soundfile to load.
 * @param {number} samplerate sample rate in Hz.
 * @param {number} [buffersize] playback buffer size, default is 4KiB.
 */
function Rawplay(filename, samplerate, buffersize) {
	/**
	 * name of file
	 * @member {string}
	 */
	this.filename = null;
	/**
	 * sample rate.
	 * @member {number}
	 */
	this.samplerate = 0;
	/**
	 * playback buffer size
	 * @member {number}
	 */
	this.buffersize = 0;
	/**
	* length of the file in samples (file size / 2 channels / 16 bit)
	* @member {number}
	*/
	this.length = 0;
}

/**
 * Close sample after use
 */
Rawplay.prototype.Close = function () { }


/**
 * rewind to the start of the sample.
 */
Rawplay.prototype.Rewind = function () { }

/**
 * current play pos
 * @returns {number} the last used sample index
 */
Rawplay.prototype.CurrentSample = function () { }

/**
 * must be called periodically (e.g. every Loop()) to update the playback buffer
 * @param  {boolean} left true to play left channel
 * @param  {boolean} right true to play right channel
 */
Rawplay.prototype.Play = function (left, right) { }

/**
 * move audio to specified sample index.
 * 
 * @param {number} idx new play index
 */
Rawplay.prototype.Seek = function (idx) { }
