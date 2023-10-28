/**
* Load a BMP, TGA, PCX, QOI, JPEG or PNG image.
*
* **Note: PNG module must be loaded by calling LoadLibrary("png") before using this function with PNG files!**
* **Note: JPEG module must be loaded by calling LoadLibrary("jpeg") before loading JPEG files!**
* **Note: JPEG module must be loaded by calling LoadLibrary("qoi") before loading QOI files!**
* 
* @see LoadLibrary()
*
* @constructor 
* @param {string} filename name of the file.
*//**
* create bitmap for given blurhash (see https://blurha.sh/).
* @constructor 
* @param {string} blurhash The blurhash.
* @param {number} width bitmap width.
* @param {number} height bitmap width.
* @param {number} [punch] The factor to improve the contrast, default = 1
*//**
* create empty bitmap of given size.
* @constructor 
* @param {number} width bitmap width.
* @param {number} height bitmap width.
* @param {number} [color] background color of the new bitmap.
*//**
* create Bitmap from integer array.
* @constructor 
* @param {number[]} data 32bit integer data interpreted as ARGB.
* @param {number} width bitmap width.
* @param {number} height bitmap height.
*//**
* create Bitmap from ByteArray (must contain a compatible image file format).
* @constructor 
* @param {ByteArray} data image file data like PCX, BMP, etc.
*//**
* create Bitmap from current(3dfx) screen.
* @constructor 
* @param { number } x screen x position.
* @param { number } y screen y position.
* @param { number } width bitmap width.
* @param { number } height bitmap height.
* @param { GR_BUFFER } [buffer] one of FRONTBUFFER, BACKBUFFER or AUXBUFFER for 3dfx access, omit for normal screen acccess.
*/
function Bitmap(filename) {
	/**
	 * Name of the file.
	 * @member {string}
	 */
	this.filename = null;
	/**
	 * Width in pixels
	 * @member {number}
	 */
	this.width = 0;
	/**
	 * Height in pixels
	 * @member {number}
	 */
	this.height = 0;
};

/**
 * Draw the image to the canvas at given coordinates.
 * @param {number} x position to draw to.
 * @param {number} y position to draw to.
 */
Bitmap.prototype.Draw = function (x, y) { };
/**
 * Draw the image to the canvas at given coordinates.
 * 
 * @param {number} srcX source position to draw from.
 * @param {number} srcY source position to draw from.
 * @param {number} srcW source size to draw from.
 * @param {number} srcH source size to draw from.
 * @param {number} destX position to draw to.
 * @param {number} destY position to draw to.
 * @param {number} destW size to draw.
 * @param {number} destH size to draw.
 */
Bitmap.prototype.DrawAdvanced = function (srcX, srcY, srcW, srcH, destX, destY, destW, destH) { };
/**
 * Draw the image to the canvas at given coordinates using the alpha channel transparency. Only works for 32bit TGA with alpha channel information.
 * @param {number} x position to draw to.
 * @param {number} y position to draw to.
 */
Bitmap.prototype.DrawTrans = function (x, y) { };
/**
 * Get the color of a pixel of this image.
 * @param {number} x position.
 * @param {number} y position.
 * @returns {number} the color of the pixel.
 */
Bitmap.prototype.GetPixel = function (x, y) { };
/**
 * draw the bitmap directly into the 3dfx/voodoo framebuffer (only works when fxInit() was called).
 * 
 * @param {number} x position to draw to.
 * @param {number} y position to draw to.
 * @param {GR_BUFFER} buffer one of FRONTBUFFER, BACKBUFFER or AUXBUFFER
 * @param {boolean} pipeline true if the pixels shall be processed by the voodoos pixel pipeline, false to just draw.
 */
Bitmap.prototype.FxDrawLfb = function (x, y, buffer, pipeline) { };
/**
 * clear the bitmap to EGA.BLACK.
 */
Bitmap.prototype.Clear = function () { };
/**
 * Save bitmap to BMP file.
 * @param {string} fname filename.
 */
Bitmap.prototype.SaveBmpImage = function (fname) { };
/**
 * Save bitmap to PCX file.
 * @param {string} fname filename.
 */
Bitmap.prototype.SavePcxImage = function (fname) { };
/**
 * Save bitmap to TGA file.
 * @param {string} fname filename.
 */
Bitmap.prototype.SaveTgaImage = function (fname) { };
/**
 * Save bitmap to PNG file.
 * 
 * **Note: PNG module must be loaded by calling LoadLibrary("png") before using this function!**
 * 
 * @see LoadLibrary()
 * @param {string} fname filename.
 */
Bitmap.prototype.SavePngImage = function (fname) { };
/**
 * Save bitmap to QOI file.
 * 
 * **Note: QOI module must be loaded by calling LoadLibrary("qoi") before using this function!**
 * 
 * @see LoadLibrary()
 * @param {string} fname filename.
 */
Bitmap.prototype.SaveQoiImage = function (fname) { };
/**
 * Save bitmap to WEBP file.
 * 
 * **Note: WEBP module must be loaded by calling LoadLibrary("webp") before using this function!**
 * 
 * @see LoadLibrary()
 * @param {string} fname filename.
 */
Bitmap.prototype.SaveWebpImage = function (fname) { };
