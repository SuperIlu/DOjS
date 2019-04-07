/********
 * Color
 */
/**
 * Create a RGB color with optional mask (see 'jsboot/color.js').
 * @class
 * @param {number} r the red value.
 * @param {number} g the green value.
 * @param {number} b the blue value.
 * @param {number} [mask] the Color mask.
 */
function Color(r, g, b, mask) { }
/**
 * @property {number} value 24bit value with the actual color.
 */
Color.value = 0;
/**
 * get the red part of a color.
 * @returns the red part of a color.
 */
Color.prototype.GetRed = function () { };
/**
 * get the green part of a color.
 * @returns the green part of a color.
 */
Color.prototype.GetGreen = function () { };
/**
 * get the blue part of a color.
 * @returns the blue part of a color.
 */
Color.prototype.GetBlue = function () { };

/********
 * File
 */
/**
 * Open a file, for file modes see {@link FILE}. Files can only either be read or written, never both.Writing to a closed file throws an exception.
 * @class
 * @param {string} filename the name of the file.
 * @param {string} mode READ, WRITE or APPEND.
 */
function File(filename, mode) { }
/**
 * Read a single byte from file and return it as number.
 * @returns {number} the byte as a number or null for EOF.
 */
File.prototype.ReadByte = function () { };
/**
 * Write a single byte to a file.
 */
File.prototype.WriteByte = function (ch) { };
/**
 * Read a line of text from file. The maximum line length is 4096 byte.
 * @returns {string} the next line or null for EOF.
 */
File.prototype.ReadLine = function () { };
/**
 * Write a NEWLINE terminated string to a file.
 * @param {string} txt the string to write.
 */
File.prototype.WriteLine = function (txt) { };
/**
 * Write a string to a file.
 * @param {string} txt the string to write.
 */
File.prototype.WriteString = function (txt) { };
/**
 * Close the file.
 */
File.prototype.Close = function () { };

/********
 * Bitmap
 */
/**
 * Load a BMP or PNG image.
 * @class
 * @param {string} filename name of the BMP or PNG file.
 */
function Bitmap(filename) { };
/**
 * Name of the file.
 */
Bitmap.filename = null;
/**
 * Width in pixels
 */
Bitmap.width = null;
/**
 * Height in pixels
 */
Bitmap.height = null;

/**
 * Draw the image to the canvas at given coordinates.
 * @param {number} x position to draw to.
 * @param {number} y position to draw to.
 */
Bitmap.prototype.Draw = function (x, y) { };

/********
 * Font
 */
/**
 * Load a '.FNT' file for GRX.
 * @class
 * @param {*} filename name of the font file.
 */
function Font(filename) { }
/**
 * Name of the FNT file.
 */
Font.filename = null;
/**
 * Width of smallest character
 */
Font.minwidth = null;
/**
 * Width of widest character
 */
Font.maxwidth = null;
/**
 * Draw a string to the canvas.
 * @param {number} x x position
 * @param {number} y y position.
 * @param {Color} foreground foreground color.
 * @param {Color} background background color.
 * @param {number} direction direction from {@link FONT}
 * @param {number} alignX alignment from {@link FONT}
 * @param {number} alignY alignment from {@link FONT}
 */
Font.prototype.DrawString = function (x, y, text, foreground, background, direction, alignX, alignY) { };
/**
 * Calculate string width for this font.
 * @param {string} the string to check.
 * @returns {number} the width in pixels.
 */
Font.prototype.StringWidth = function (text) { };
/**
 * Calculate string height for this font.
 * @param {string} the string to check.
 * @returns {number} the height in pixels.
 */
Font.prototype.StringHeight = function (text) { };
/**
 * Resize font to new width / height.
 * @param {number} w the new width.
 * @param {number} h the new height.
 */
Font.prototype.Resize = function (w, h) { };


/********
 * Sample/Module
 */
/**
 * Load a WAV-file.
 * @class
 * @param {string} filename 
 */
function Sample(filename) { }
/**
 * Name of the WAV.
 */
Sample.filename = null;
/**
 * Sound length.
 */
Sample.length = null;
/**
 * Sound speed.
 */
Sample.speed = null;
/**
 * Play the WAV once.
 */
Sample.prototype.Play = function () { };
/**
 * Stop playing.
 */
Sample.prototype.Stop = function () { };

/**
 * Load a MOD/S3M/XM/etc-file.
 * @class
 * @param {string} filename 
 */
function Module(filename) { }
/**
 * Name of the MOD-file.
 */
Module.filename = null;
/**
 * internal name of the song.
 */
Module.songname = null;
/**
 * file format of the song.
 */
Module.modtype = null;
/**
 * mod file comment (if any).
 */
Module.comment = null;
/**
 * Play the MOD-file once.
 */
Module.prototype.Play = function () { };

/********
 * MIDI
 */
/**
 * Load a midi file.
 * @class
 * @param {string} filename the name of the MIDI file.
 */
function Midi(filename) { }
/**
 * Play the midi file.
 */
Midi.prototype.Play = function () { };
