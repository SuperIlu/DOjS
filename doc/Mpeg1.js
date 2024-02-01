/**
 * load MPEG1 for video/audio playback.
 * Videos can be converted using ffmpeg, e.g. for converting to MPEG1 with 320 width:
 * "ffmpeg -i infile -filter:v scale=320:-1 -c:v mpeg1video -c:a mp2 -format mpeg outfile"
 * 
 * **Note: MPEG1 module must be loaded by calling LoadLibrary("mpeg1") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @class
 * 
 * @param {string} filename file name of the soundfile to load.
 * @param {boolean} [audio] enable audioo playback with true, disable with false (default: false).
 */
function MPEG1(filename, audio) {
	/**
	 * name of file
	 * @member {string}
	 */
	this.filename = null;
	/**
	 * audio sample rate.
	 * @member {number}
	 */
	this.samplerate = 0;
	/**
	 * frame rate.
	 * @member {number}
	 */
	this.framerate = 0;
	/**
	 * video width
	 * @member {number}
	 */
	this.width = 0;
	/**
	 * video height
	 * @member {number}
	 */
	this.height = 0;
	/**
	* video duration in seconds
	* @member {number}
	*/
	this.duration = 0;
}

/**
 * Close MPEG1.
 */
MPEG1.prototype.Close = function () { }

/**
 * seek to given time index.
 * 
 * @param {number} tidx the index in seconds
 */
MPEG1.prototype.Seek = function (tidx) { }

/**
 * rewind to the start of the video.
 */
MPEG1.prototype.Rewind = function () { }

/**
 * check if playback has ended.
 * 
 * @returns true if the video has ended, else false.
 */
MPEG1.prototype.HasEnded = function () { }

/**
 * must be called periodically (e.g. every Loop()) to render audio/video.
 * 
 * @param  {number} x x position of the upper left corner of the video
 * @param  {number} y y position of the upper left corner of the video
 */
MPEG1.prototype.Play = function (x, y) { }

/**
 * current play pos
 * 
 * @returns {number} the current video position in seconds
 */
MPEG1.prototype.CurrentTime = function () { }

