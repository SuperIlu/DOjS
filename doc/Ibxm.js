/**
 * load MOD, S3M or XM module for sound playback.
 * 
 * **Note: IBXM module must be loaded by calling LoadLibrary("ibxm") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @class
 * 
 * @param {string} filename file name of the soundfile to load.
 * @param {number} [buffersize] playback buffer size, default is 16KiB.
 */
function IBXM(filename, samplerate) {
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
	 * number of instruments
	 * @member {number}
	 */
	this.instruments = 0;
	/**
	 * number of patterns
	 * @member {number}
	 */
	this.patterns = 0;
	/**
	 * MOD file duration
	 * @member {number}
	 */
	this.duration = 0;
};

/**
 * Close MOD after use.
 */
IBXM.prototype.Close = function () { };

/**
 * rewind to the start of the MOD.
 */
IBXM.prototype.Rewind = function () { };

/**
 * must be called periodically (e.g. every Loop()) to update the playback buffer.
 */
IBXM.prototype.Play = function () { };

/**
 * move audio to specified sample index.
 * 
 * @param {number} idx new play index
 */
IBXM.prototype.Seek = function (idx) { };
