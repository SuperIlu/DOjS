/**
 * Micromod MOD player plugin.
 * 
 * **Note: Micromod module must be loaded by calling LoadLibrary("micromod") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @module micromod
 */

/**
 * Load a MOD file and initialize Micromod.
 * 
 * @param {String} filename the file name of the module.
 * @param {number} [samplerate] samplerate, default is 22kHz
 * @param {number} [buffersize] decoding buffer size, default is 4KiB.
*/
function MicromodInit(filename, samplerate, buffersize) { }
/**
 * unload the current MOD. this needs to be called before another module can be loaded.
 */
function MicromodDeInit() { }
/**
 * rewind to the start of the current MOD.
 */
function MicromodRewind() { }
/**
 * must be called periodically (e.g. every Loop()) to update the playback buffer.
 */
function MicromodPlay() { }
