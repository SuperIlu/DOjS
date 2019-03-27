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

/********
 * Other properties
 */
/**
 * true if WAV sound is available.
 * @property {boolean}
 */
SOUND_AVAILABLE = null;

/**
 * true if FM sound is available.
 * @property {boolean}
 */
SYNTH_AVAILABLE = true;

/**
 * @property {boolean} true if mouse is available.
 */
MOUSE_AVAILABLE = true;

/**
 * @property {boolean} true if midi is available.
 */
MIDI_AVAILABLE = true;

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
function IpxSendPacket(data, dest) { }

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
 * All functions for FM music.
 * 
 * @module fm
 */
/**
 * 
 * Convert the contents of a JS object into an instrument and set the parameters to the given voice.
 * @param {number} voice the voice between 0-8.
 * @param {FmInstrument} instrument an instrument definition.
 */
function SetInstrument(voice, instrument) { }

/**
 * Start playing a note.
 * @param {number} voice the voice between 0-8.
 * @param {number} note the note frequency.
 * @param {number} octave the octave.
 */
function NoteOn(voice, note, octave) { }

/**
 * Stop playing a note.
 * @param {number} voice the voice between 0-8.
 * @param {number} note the note frequency.
 * @param {number} octave the octave.
 */
function NoteOff(voice, note, octave) { }

/**
 * All graphics functions.
 * 
 * @module gfx
 */
/**
 * Get number of possible colors.
 * @returns { number } number of possible colors.
 */
function NumColors() { }

/**
 * Get number of remaining free colors.
 * @returns {number} number of remaining free colors.
 */
function NumFreeColors() { }

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
 * get the max X coordinate on the drawing area.
 * @returns {number} the max X coordinate on the drawing area.
 */
function MaxX() { }

/**
 * get the max Y coordinate on the drawing area.
 * @returns {number} the max Y coordinate on the drawing area.
 */
function MaxY() { }

/**
 * clear the screen with given color.
 * @param {Color} c the color.
 */
function ClearScreen(c) { }

/**
 * draw a point.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {Color} c color.
 */
function Plot(x, y, c) { }

/**
 * draw a line.
 * @param {number} x1 start x coordinate.
 * @param {number} y1 start y coordinate.
 * @param {number} x2 end x coordinate.
 * @param {number} y2 end y coordinate.
 * @param {Color} c color.
 */
function Line(x1, y1, x2, y2, c) { }

/**
 * draw a box.
 * @param {number} x1 start x coordinate.
 * @param {number} y1 start y coordinate.
 * @param {number} x2 end x coordinate.
 * @param {number} y2 end y coordinate.
 * @param {Color} c color.
 */
function Box(x1, y1, x2, y2, c) { }

/**
 * draw a circle.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} r radius.
 * @param {Color} c color.
 */
function Circle(x, y, r, c) { }

/**
 * draw a ellipse.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} xr radius.
 * @param {number} yr radius.
 * @param {Color} c color.
 */
function Ellipse(x, y, xr, yr, c) { }

/**
 * Draw a circle arc.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} r radius.
 * @param {number} start start angle in tenths of degrees.
 * @param {number} end end angle in tenths of degrees.
 * @param {*} style value from {@link ARC}.
 * @param {Color} c color.
 * @returns {ArcInfo} detailed info about the drawn arc.
 */
function CircleArc(x, y, r, start, end, style, c) { }

/**
 * Draw an ellipse arc.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} xr radius.
 * @param {number} yr radius.
 * @param {number} start start angle in tenths of degrees.
 * @param {number} end end angle in tenths of degrees.
 * @param {*} style value from {@link ARC}.
 * @param {Color} c color.
 * @returns {ArcInfo} detailed info about the drawn arc.
 */
function EllipseArc(x, y, rx, ry, start, end, style, c) { }

/**
 * draw a filled box.
 * @param {number} x1 start x coordinate.
 * @param {number} y1 start y coordinate.
 * @param {number} x2 end x coordinate.
 * @param {number} y2 end y coordinate.
 * @param {Color} c color.
 */
function FilledBox(x1, y1, x2, y2, c) { }

/**
 * draw a framed box (3D effect box).
 * @param {number} x1 start x coordinate.
 * @param {number} y1 start y coordinate.
 * @param {number} x2 end x coordinate.
 * @param {number} y2 end y coordinate.
 * @param {number} wdt border width.
 * @param {Color[]} colors [intcolor, topcolor, rightcolor, bottomcolor, leftcolor]
 */
function FramedBox(x1, y1, x2, y2, wdt, colors) { }

/**
 * draw a filled circle.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} r radius.
 * @param {Color} c color.
 */
function FilledCircle(x, y, r, c) { }

/**
 * draw a filled ellipse.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} xr radius.
 * @param {number} yr radius.
 * @param {Color} c color.
 */
function FilledEllipse(x, y, rx, ry, c) { }

/**
 * Draw a filled circle arc.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} r radius.
 * @param {number} start start angle in tenths of degrees.
 * @param {number} end end angle in tenths of degrees.
 * @param {*} style value from {@link ARC}.
 * @param {Color} c color.
 * @returns {ArcInfo} detailed info about the drawn arc.
 */
function FilledCircleArc(x, y, r, start, end, style, c) { }

/**
 * Draw a filled ellipse arc.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {number} xr radius.
 * @param {number} yr radius.
 * @param {number} start start angle in tenths of degrees.
 * @param {number} end end angle in tenths of degrees.
 * @param {*} style value from {@link ARC}.
 * @param {Color} c color.
 * @returns {ArcInfo} detailed info about the drawn arc.
 */
function FilledEllipseArc(x, y, rx, ry, start, end, style, c) { }

/**
 * do a flood fill.
 * @param {number} x x coordinate.
 * @param {number} y y coordinate.
 * @param {Color} bound bound color.
 * @param {Color} c fill color.
 */
function FloodFill(x, y, bound, c) { }

/**
 * replace a color with another in given area.
 * @param {number} x1 start x coordinate.
 * @param {number} y1 start y coordinate.
 * @param {number} x2 end x coordinate.
 * @param {number} y2 end y coordinate.
 * @param {*} old color to replace.
 * @param {*} c new color.
 */
function FloodSpill(x1, y1, x2, y2, old, c) { }

/**
 * replace two colors with another in given area.
 * @param {number} x1 start x coordinate.
 * @param {number} y1 start y coordinate.
 * @param {number} x2 end x coordinate.
 * @param {number} y2 end y coordinate.
 * @param {*} old1 color to replace.
 * @param {*} c1 new color.
 * @param {*} old2 color to replace.
 * @param {*} c2 new color.
 */
function FloodSpill2(x1, y1, x2, y2, old1, c1, old2, c2) { }

/**
 * draw a polyline.
 * @param {Color} c color.
 * @param {number[][]} points an array of arrays with two coordinates (e.g. [[1, 1], [1, 10], [10, 10], [10, 1]]).
 */
function PolyLine(points, c) { }

/**
 * draw a polygon.
 * @param {Color} c color.
 * @param {number[][]} points an array of arrays with two coordinates (e.g. [[1, 1], [1, 10], [10, 10], [10, 1]]).
 */
function Polygon(points, c) { }

/**
 * draw a filled polygon.
 * @param {Color} c color.
 * @param {number[][]} points an array of arrays with two coordinates (e.g. [[1, 1], [1, 10], [10, 10], [10, 1]]).
 */
function FilledPolygon(points, c) { }

/**
 * draw a filled convex polygon.
 * @param {Color} c color.
 * @param {number[][]} points an array of arrays with two coordinates (e.g. [[1, 1], [1, 10], [10, 10], [10, 1]]).
 */
function FilledConvexPolygon(points, c) { }

/**
 * Set mouse speed by multiplying by spmult and dividing by spdiv.
 * @param {number} spmul multiplicator.
 * @param {number} spdiv divider.
 */
function MouseSetSpeed(spmul, spdiv) { }

/**
 * Set mouse acceleration. If a mouse coordinate changes between two samplings by more than the thresh parameter, the change is multiplied by the accel parameter.
 * @param {number} thresh threshold
 * @param {number} accel acceleration.
 */
function MouseSetAccel(thresh, accel) { }

/**
 * Limit mouse movement.
 * @param {number} x1 start x coordinate.
 * @param {number} y1 start y coordinate.
 * @param {number} x2 end x coordinate.
 * @param {number} y2 end y coordinate.
 */
function MouseSetLimits(x1, y1, x2, y2) { }

/**
 * Get mouse limits.
 * @returns {MouseLimits} current mouse limits.
 */
function MouseGetLimits() { }

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
 * Check if the cursor is visible
 * @returns {boolean} true or false.
 */
function MouseCursorIsDisplayed() { }

/**
 * Set mouse pointer colors.
 * @param {Color} fg foreground color.
 * @param {Color} bg background color.
 */
function MouseSetColors(fg, bg) { }

/**
 * Change mode of the mouse cursor.
 * @param {*} mode a mode from {@link MOUSE}.
 * @param {Color} c color.
 * @param {number} x1 start x coordinate.
 * @param {number} y1 start y coordinate.
 * @param {number} x2 end x coordinate.
 * @param {number} y2 end y coordinate.
 * @param {number} xanchor anchor x coordinate.
 * @param {number} yanchor anchor y coordinate.
 * 
 * @example MouseSetCursorMode(MOUSE.Mode.NORMAL)
 * @example MouseSetCursorMode(MOUSE.Mode.RUBBER,xanchor,yanchor,c)
 * @example MouseSetCursorMode(MOUSE.Mode.LINE,xanchor,yanchor,c)
 * @example MouseSetCursorMode(MOUSE.Mode.BOX,x1,y1,x2,y2,c)
 */
function MouseSetCursorMode(mode) { }

/**
 * Draw a text with the default font.
* @param {number} x x coordinate.
* @param {number} y y coordinate.
* @param {*} text the text to display.
* @param {Color} fg foreground color.
* @param {Color} bg background color.
*/
function TextXY(x, y, text, fg, bg) { }

/**
 * Save current screen to BMP file.
 * @param {string} fname filename.
 */
function SaveBmpImage(fname) { }

/**
 * Save current screen to PNG file.
 * @param {string} fname filename.
 */
function SavePngImage(fname) { }

/**
 * @module other
 */
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

