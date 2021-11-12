/**
 * load OggVorbis for sound playback.
 * 
 * **Note: OggVorbis module must be loaded by calling LoadLibrary("vorbis") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @class
 * 
 * @param {string} filename file name of the soundfile to load.
 * @param {number} [buffersize] playback buffer size, default is 16KiB.
 */
function Ogg(filename, buffersize) {
	/**
	 * name of file
	 * @member {string}
	 */
	this.filename = null;
	/**
	 * number of channels.
	 * @member {number}
	 */
	this.channels = 0;
	/**
	 * sample rate.
	 * @member {number}
	 */
	this.samplerate = 0;
	/**
	 * max frame size
	 * @member {number}
	 */
	this.maxframesize = 0;
	/**
	 * playback buffer size
	 * @member {number}
	 */
	this.buffersize = 0;
	/**
	 * vendor field
	 * @member {string}
	 */
	this.vendor = null;
	/**
	 * ogg vorbis comments
	 * @member {string[]}
	 */
	this.comments = [];
	/**
	 * length of the ogg in samples
	 * @member {number}
	 */
	this.numsamples = 0;
	/**
	 * length of the ogg in seconds
	 * @member {number}
	 */
	this.duration = 0;
};

/**
 * Close Ogg after use.
 */
Ogg.prototype.Close = function () { };

/**
 * rewind to the start of the Ogg.
 */
Ogg.prototype.Rewind = function () { };

/**
 * current play pos
 * @returns {number} the current sample index.
 */
Ogg.prototype.CurrentSample = function () { };

/**
 * must be called periodically (e.g. every Loop()) to update the playback buffer.
 */
Ogg.prototype.Play = function () { };

/**
 * move audio to specified sample index.
 * 
 * @param {number} idx new play index
 */
Ogg.prototype.Seek = function (idx) { };
