/********
 * Script definition
 */

/**
 * This function is called once at startup. It can initialize variables, setup hardware, etc.
 */
function Setup() { }

/**
 * This function is called after setup repeatedly until {@link Stop} is called. After calling {@link Stop} the program ends when {@link Loop} exits.
 */
function Loop() { }

/**
 * This function is called whenever mouse / keyboard input happens.
 * @param {Event} event the current event.
 */
function Input(event) { }

/**
 * @property {object} global the global context.
 */
global = null;

/********
 * Other properties
 * 
 * @module other
 */
/**
 * @property {boolean} SOUND_AVAILABLE true if sound is available.
 */
SOUND_AVAILABLE = null;

/**
 * @property {boolean} MOUSE_AVAILABLE true if mouse is available.
 */
MOUSE_AVAILABLE = true;

/**
 * @property {boolean} IPX_AVAILABLE true if networking is available.
 */
IPX_AVAILABLE = true;

/**
 * @property {number} DOJS_VERSION the version
 */
DOJS_VERSION = 0.0;

/**
 * IPX network functions.
 * 
 * @module ipx
 */
/**
 * Open an IPX socket. See {@link IPX} for DEFAULT_SOCKET.
 * @param {*} num the socket number to use.
 * 
 */
function IpxSocketOpen(num) { }

/**
 * Close IPX socket (if any).
 */
function IpxSocketClose() { }

/**
 * Send packet via IPX. Max length 79 byte. Node addresses are arrays of 6 numbers between 0-255. See {@link IPX} for BROADCAST address.
 * @param {string} data data to send.
 * @param {IpxAddress} dest destination address.
 */
function IpxSend(data, dest) { }

/**
 * Check for packet in receive buffer.
 * @returns {boolean} true if a packet is available.
 */
function IpxCheckPacket() { }

/**
 * Get packet from receive buffer(or NULL).
 * @returns {IpxPacket} a data packet or null if none available.
 */
function IpxGetPacket() { }

/**
 * Get the local address.
 * @returns {IpxAddress} an array containing the own address.
 */
function IpxGetLocalAddress() { }

/**
 * MIDI music functions. See {@link Midi} on how to load MIDI files.
 * 
 * @module midi
 */
/**
 * Check if the file is still playing.
 * @returns {boolean} true if the file is still playing.
 */
function MidiIsPlaying() { }

/**
 * Stop playing midi.
 */
function MidiStop() { }

/**
 * Pause playing midi.
 */
function MidiPause() { }

/**
 * Resume playing midi after MidiPause().
 */
function MidiResume() { }

/**
 * Get song position.
 * @returns {number} current song position in seconds.
 */
function MidiGetTime() { }

/**
 * Send MIDI commands to output.
 * @param {number[]} data an array of midi commands.
 */
function MidiOut(data) { }

/**
 * All graphics functions.
 * 
 * @module gfx
 */
/**
 * get the width of the drawing area.
 * @returns {number} the width of the drawing area.
 */
function SizeX() { }

/**
 * get the height of the drawing area.
 * @returns {number} the height of the drawing area.
 */
function SizeY() { }

/**
 * Get color depth info.
 * @returns {number} bits per pixel.
 */
function GetScreenMode() { }

/**
 * clear the screen with given color.
 * @param {number} c the color.
 */
function ClearScreen(c) { }

/**
 * draw a point.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} c color.
 */
function Plot(x, y, c) { }

/**
 * draw a line.
 * @param {number} x1 start x coordinate.
 * @param {number} y1 start y coordinate.
 * @param {number} x2 end x coordinate.
 * @param {number} y2 end y coordinate.
 * @param {number} c color.
 */
function Line(x1, y1, x2, y2, c) { }

/**
 * draw a line with given width.
 * @param {number} x1 start x coordinate.
 * @param {number} y1 start y coordinate.
 * @param {number} x2 end x coordinate.
 * @param {number} y2 end y coordinate.
 * @param {number} w line width.
 * @param {number} c color.
 */
function CustomLine(x1, y1, x2, y2, w, c) { }

/**
 * draw a box.
 * @param {number} x1 start x coordinate.
 * @param {number} y1 start y coordinate.
 * @param {number} x2 end x coordinate.
 * @param {number} y2 end y coordinate.
 * @param {number} c color.
 */
function Box(x1, y1, x2, y2, c) { }

/**
 * draw a circle.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} r radius.
 * @param {number} c color.
 */
function Circle(x, y, r, c) { }

/**
 * draw a circle with given width.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} r radius.
 * @param {number} w line width.
 * @param {number} c color.
 */
function CustomCircle(x, y, r, w, c) { }

/**
 * draw a ellipse.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} xr radius.
 * @param {number} yr radius.
 * @param {number} c color.
 */
function Ellipse(x, y, xr, yr, c) { }

/**
 * draw a ellipse with given width.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} xr radius.
 * @param {number} yr radius.
 * @param {number} w line width.
 * @param {number} c color.
 */
function CustomEllipse(x, y, xr, yr, w, c) { }

/**
 * Draw a circle arc.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} r radius.
 * @param {number} start start angle in tenths of degrees.
 * @param {number} end end angle in tenths of degrees.
 * @param {*} style value from {@link ARC}.
 * @param {number} c color.
 * @returns {ArcInfo} detailed info about the drawn arc.
 */
function CircleArc(x, y, r, start, end, style, c) { }

/**
 * Draw a circle arc with given width.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} r radius.
 * @param {number} start start angle in tenths of degrees.
 * @param {number} end end angle in tenths of degrees.
 * @param {*} style value from {@link ARC}.
 * @param {number} w line width.
 * @param {number} c color.
 * @returns {ArcInfo} detailed info about the drawn arc.
 */
function CustomCircleArc(x, y, r, start, end, style, w, c) { }

/**
 * draw a filled box.
 * @param {number} x1 start x coordinate.
 * @param {number} y1 start y coordinate.
 * @param {number} x2 end x coordinate.
 * @param {number} y2 end y coordinate.
 * @param {number} c color.
 */
function FilledBox(x1, y1, x2, y2, c) { }

/**
 * draw a filled circle.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} r radius.
 * @param {number} c color.
 */
function FilledCircle(x, y, r, c) { }

/**
 * draw a filled ellipse.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} xr radius.
 * @param {number} yr radius.
 * @param {number} c color.
 */
function FilledEllipse(x, y, rx, ry, c) { }

/**
 * do a flood fill.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} bound bound color.
 * @param {number} c fill color.
 */
function FloodFill(x, y, bound, c) { }

/**
 * draw a filled polygon.
 * @param {number} c color.
 * @param {number[][]} points an array of arrays with two coordinates (e.g. [[1, 1], [1, 10], [10, 10], [10, 1]]).
 */
function FilledPolygon(points, c) { }

/**
 * Draw a text with the default font.
* @param {number} x x coordinate.
* @param {number} y y coordinate.
* @param {*} text the text to display.
* @param {number} fg foreground color.
* @param {number} bg background color.
*/
function TextXY(x, y, text, fg, bg) { }

/**
 * Save current screen to BMP file.
 * @param {string} fname filename.
 */
function SaveBmpImage(fname) { }

/**
 * Save current screen to PCX file.
 * @param {string} fname filename.
 */
function SavePcxImage(fname) { }

/**
 * Save current screen to TGA file.
 * @param {string} fname filename.
 */
function SaveTgaImage(fname) { }

/**
 * get color of on-screen pixel.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @returns {number} pixel color.
 */
function GetPixel(x, y) { }

/**
 * Enable/disable the transparency when drawing.
 * @param {boolean} en true to enable transparency when drawing (might slow down drawing).
 */
function TransparencyEnabled(en) { }

/**
 * @module other
 */
/**
 * Set mouse speed.
 * @param {number} x horizontal speed.
 * @param {number} y vertical speed.
 */
function MouseSetSpeed(x, y) { }

/**
 * Limit mouse movement.
 * @param {number} x1 start x coordinate.
 * @param {number} y1 start y coordinate.
 * @param {number} x2 end x coordinate.
 * @param {number} y2 end y coordinate.
 */
function MouseSetLimits(x1, y1, x2, y2) { }

/**
 * Move mouse cursor.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 */
function MouseWarp(x, y) { }

/**
 * Show hide mouse cursor.
 * @param {boolean} b true or false.
 */
function MouseShowCursor(b) { }

/**
 * Change mode of the mouse cursor.
 * @param {*} mode a mode from {@link MOUSE}.
 */
function MouseSetCursorMode(mode) { }

/**
 * Write data to JSLOG.TXT logfile.
 * @param {string} s the string to print.
 */
function Print(s) { }
/**
 * Write data to JSLOG.TXT logfile with a newline.
 * @param {string} s the string to print.
 */
function Println(s) { }

/**
 * DOjS will exit after the current call to {@link Loop}.
 */
function Stop() { }

/**
 * Sleep for the given number of ms.
 * @param {number} ms time to sleep.
 */
function Sleep(ms) { }

/**
 * Get ms timestamp.
 * @return {number} ms time.
 */
function MsecTime() { }

/**
 * Load the contents of a file into a string. Throws exception if loading fails.
 * @param {string} filename name of file to read.
 * @returns {string} the contents of the file.
 */
function Read(filename) { }

/**
 * Get directory listing.
 * @param {string} dname name of directory to list.
 * @returns {string[]} array of entry names.
 */
function List(dname) { }

/**
 * Get information about a file / directory.
 * @param {string} name name of the file to get info for.
 * @returns {StatInfo} an info object.
 */
function Stat(name) { }

/**
 * Run garbage collector, print statistics to logfile if 'info==true'.
 * @param {boolean} info true to print collection stats to logfile.
 */
function Gc(info) { }

/**
 * Get information system memory.
 * @returns {MemInfo} an info object.
 */
function MemoryInfo() { }

/**
 * Set maximum frame rate. If {@link Loop} takes longer than '1/rate' seconds then the framerate will not be reached.
 * @param {number} rate max frame rate wanted.
 */
function SetFramerate(rate) { }

/**
 * Current frame rate.
 * @returns {number} current framerate.
 */
function GetFramerate() { }

/**
 * Change the exit key from ESCAPE to any other keycode from {@link KEY}}.
 * @param {number} key 
 */
function SetExitKey(key) { }

/** @module color */

/**
 * create RGBA color.
 * 
 * @param {number} r red (0-255)
 * @param {number} g green (0-255)
 * @param {number} b blue (0-255)
 * @param {number} a alpha (0-255)
 * @returns {number} a color.
 */
function Color(r, g, b, a) { }

/**
 * get red part of color.
 * @param {number} c a color
 * @returns {number} the red part.
 */
function GetRed(c) { }

/**
 * get green part of color.
 * @param {number} c a color
 * @returns {number} the green part.
 */
function GetGreen(c) { }

/**
 * get blue part of color.
 * @param {number} c a color
 * @returns {number} the blue part.
 */
function GetBlue(c) { }

/**
 * get alpha part of color.
 * @param {number} c a color
 * @returns {number} the alpha part.
 */
function GetAlpha(c) { }

