/**
 * load SMACKER file (SMK) for video/audio playback.
 * 
 * **Note: SMACKER module must be loaded by calling LoadLibrary("smacker") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @class
 * 
 * @param {string} filename file name of the soundfile to load.
 * @param {boolean} [audio] enable audioo playback with true, disable with false (default: false).
 */
function SMACKER(filename, audio) {
	/**
	 * name of file
	 * @member {string}
	 */
	this.filename = null;
	/**
	 * frame rate.
	 * @member {number}
	 */
	this.framerate = 0;
	/**
	 * frame duration.
	 * @member {number}
	 */
	this.frame_duration = 0;
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
	/**
	* number of frames.
	* @member {number}
	*/
	this.num_frames = 0;
}

/**
 * Close SMACKER.
 */
MPEG1.prototype.Close = function () { }

/**
 * rewind to the start of the video.
 */
MPEG1.prototype.Rewind = function () { }

/**
 * must be called periodically (e.g. every Loop()) to render audio/video.
 * 
 * @param  {number} x x position of the upper left corner of the video
 * @param  {number} y y position of the upper left corner of the video
 * 
 * @returns {number} the current frame number. If this does not change anymore playback ended.
 */
MPEG1.prototype.Play = function (x, y) { }

/**
 * current play pos
 * 
 * @returns {number} the current video position in seconds
 */
MPEG1.prototype.CurrentTime = function () { }

