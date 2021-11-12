/**
 * load a GIF animation.
 * 
 * **Note: GIFanim module must be loaded by calling LoadLibrary("gifanim") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @class
 * 
 * @param {string} filename file name of the GIF to load
 */
function GIFanim(filename) {
	/**
	 * name of gif
	 * @member {string}
	 */
	this.filename = null;
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
GIFanim.prototype.Close = function () { }

/**
 * get gif anim comment
 * @returns {string} the gif anim comment
 */
GIFanim.prototype.GetComment = function () { }

/**
 * play the next frame of the animation. Frames are rendered consecutive with each call.
 * It is not possible to directly render a specific frame except by using SkipFrame().
 * 
 * @param {number} x x position to render the left, upper edge of the animation
 * @param {number} y y position to render the left, upper edge of the animation
 * @returns {number} -1 if this was the last frame, else the delay for the next frame.
 */
GIFanim.prototype.PlayFrame = function (x, y) { }

/**
 * skip the rendering of a frame.
 * @returns {number} -1 if this was the last frame, else the delay for the next frame.
 */
GIFanim.prototype.SkipFrame = function () { }
