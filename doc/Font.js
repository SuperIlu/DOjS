/**
 * Load a '.FNT' file for GRX.
 * @class
 * @param {*} filename name of the font file.
 */
function Font(filename) {
	/**
	 * Name of the FNT file.
	 * @member {string}
	 */
	this.filename = null;
	/**
	 * Font height
	 * @member {number}
	 */
	this.height = 0;
}
/**
 * Draw a left aligned string to the canvas.
 * @param {number} x x position
 * @param {number} y y position.
 * @param {string} text the string to draw.
 * @param {Color} foreground foreground color.
 * @param {Color} background background color.
 */
Font.prototype.DrawStringLeft = function (x, y, text, foreground, background) { };
/**
 * Draw a center aligned string to the canvas.
 * @param {number} x x position
 * @param {number} y y position.
 * @param {string} text the string to draw.
 * @param {Color} foreground foreground color.
 * @param {Color} background background color.
 */
Font.prototype.DrawStringCenter = function (x, y, text, foreground, background) { };
/**
 * Draw a right aligned string to the canvas.
 * @param {number} x x position
 * @param {number} y y position.
 * @param {string} text the string to draw.
 * @param {Color} foreground foreground color.
 * @param {Color} background background color.
 */
Font.prototype.DrawStringRight = function (x, y, text, foreground, background) { };
/**
 * Calculate string width for this font.
 * @param {string} text the string to check.
 * @returns {number} the width in pixels.
 */
Font.prototype.StringWidth = function (text) { };
/**
 * Calculate string height for this font.
 * @param {string} text the string to check.
 * @returns {number} the height in pixels.
 */
Font.prototype.StringHeight = function (text) { };
