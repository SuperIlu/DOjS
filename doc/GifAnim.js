/**
 * GITanim loader/render
 * 
 * **Note: GIFanim module must be loaded by calling LoadLibrary("gifanim") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @module gifanim
 */

/**
 * load a GIF animation.
 * 
 * @param {string} filename file name of the GIT to load
 */
function GIFanim(filename) {
	/**
	 * name of gif
	 * @member {string}
	 */
	this.filename = 0;
	/**
	 * width of GIF
	 * @member {number}
	 */
	this.width = 0;
	/**
	 * height of GIF
	 * @member {number}
	 */
	this.height = 0;
	/**
	 * total frames in file
	 * @member {number}
	 */
	this.frameCount = 0;
	/**
	 * duration of animation in milliseconds
	 * @member {number}
	 */
	this.duration = 0;
	/**
	 * maximum frame delay
	 * @member {number}
	 */
	this.maxDelay = 0;
	/**
	 * minimum frame delay
	 * @member {number}
	 */
	this.minDelay = 0;
}

/**
 * Close GIFanim after use
 */
function Close() { }

/**
 * get gif anim comment
 * @returns {string} the gif anim comment
 */
function GetComment() { }

/**
 * play the next frame of the animation. Frames are rendered consecutive with each call.
 * It is not possible to directly render a specific frame except by using SkipFrame().
 * 
 * @param {number} x x position to render the left, upper edge of the animation
 * @param {number} y y position to render the left, upper edge of the animation
 * @returns {number} -1 if this was the last frame, else the delay for the next frame.
 */
function PlayFrame(x, y) { }

/**
 * skip the rendering of a frame.
 * @returns {number} -1 if this was the last frame, else the delay for the next frame.
 */
function SkipFrame() { }
