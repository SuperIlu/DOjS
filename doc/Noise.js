/**
 * perlin noise functions
 * 
 * **Note: noise module must be loaded by calling LoadLibrary("noise") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @module noise
 */

/**
 * 1D, 2D, 3D and 4D float Perlin noise.
 * 
 * @param {number} x x-coordinate in noise space
 * @param {number} [y] y-coordinate in noise space
 * @param {number} [z] z-coordinate in noise space
 * @param {number} [w] w-coordinate in noise space
 * 
 * @returns {number} Perlin noise value (between 0 and 1) at specified coordinates.
 */
function Noise(x, y, z, w) { }

/**
 * 1D, 2D, 3D and 4D float Perlin simplex noise
 * 
 * @param {number} x x-coordinate in noise space
 * @param {number} [y] y-coordinate in noise space
 * @param {number} [z] z-coordinate in noise space
 * @param {number} [w] w-coordinate in noise space
 * 
 * @returns {number} Perlin simplex noise value (between 0 and 1) at specified coordinates.
 */
function SNoise(x, y, z, w) { }
