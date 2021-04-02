/**
 * Create a ZBuffer. A ZBuffer must be set as current before using it.
 * 
 * **Note: al3d module must be loaded by calling LoadLibrary("al3d") before using!**
 * 
 * @class
 * @param {Bitmap} bitmap the bitmap to draw to or null for the screen.
 */
function ZBuffer(bitmap) { }
/**
 * Clear ZBuffer with given value.
 * @param {number} z value to use for clearing.
 */
ZBuffer.prototype.Clear = function (z) { };
/**
 * Set this ZBuffer as current.
 */
ZBuffer.prototype.Set = function () { };
