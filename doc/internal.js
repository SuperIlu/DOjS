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

/**
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
 * sound recording functions.
 * 
 * @module sndin
 */

/**
* @property {boolean} SNDIN_AVAILABLE true if sound recording is available.
*/
SNDIN_AVAILABLE = true;

/**
 * @property {boolean} SNDIN_8BIT true if 8bit input is available.
 */
SNDIN_8BIT = true;

/**
 * @property {boolean} SNDIN_16BIT true if 16bit input is available.
 */
SNDIN_16BIT = true;

/**
 * @property {boolean} SNDIN_STEREO true if stereo input is available.
 */
SNDIN_STEREO = true;

/**
 * select sound input source.
 * 
 * @param {*} src a value from {@link SOUND}.
 */
function SoundInputSource(src) { }

/**
 * start sound recording with given parameters.
 * 
 * @param {number} rate sample rate.
 * @param {number} bits 8 or 16 bits.
 * @param {bool} stereo true for stereo recording.
 */
function SoundStartInput(rate, bits, stereo) { }

/**
 * stop sound recording.
 */
function SoundStopInput() { }

/**
 * get the actuall sound data.
 * 
 * @returns {number[]} returns one or two arrays with sound data or null if no data available.
 */
function ReadSoundInput() { }

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
 * set the current render destination.
 * @param {Bitmap} bm A Bitmap to render on or null to use the screen as rendering destination.
 */
function SetRenderBitmap(bm) { }

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
 * @throws Throws an error if reading fails.
 */
function Read(filename) { }

/**
 * Get directory listing.
 * @param {string} dname name of directory to list.
 * @returns {string[]} array of entry names.
 * @throws Throws an error if listing fails.
 */
function List(dname) { }

/**
 * Get information about a file / directory.
 * @param {string} name name of the file to get info for.
 * @returns {StatInfo} an info object.
 * @throws Throws an error if stat fails.
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
 * @param {number} a alpha (0-255) (optional)
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

/**
 * @module 3d
 */
/**
 * This number (default value = 100.0) controls the behaviour of the z-sorting algorithm. When an edge is very close to another's polygon plane, there is an interval of uncertainty in which you cannot tell which object is visible (which z is smaller). This is due to cumulative numerical errors for edges that have undergone a lot of transformations and interpolations.<br/>
 * The default value means that if the 1/z values (in projected space) differ by only 1/100 (one percent), they are considered to be equal and the x-slopes of the planes are used to find out which plane is getting closer when we move to the right.<br/>
 * Larger values means narrower margins, and increasing the chance of missing true adjacent edges/planes. Smaller values means larger margins, and increasing the chance of mistaking close polygons for adjacent ones. The value of 100 is close to the optimum. However, the optimum shifts slightly with resolution, and may be application-dependent. It is here for you to fine-tune. <br/>
 * @param {number} gap gap value.
 */
function SetSceneGap(gap) { }

/**
 * Draw 3d polygons using the specified rendering mode. Unlike the regular polygon() function, these routines don't support concave or self-intersecting shapes. The width and height of the texture bitmap must be powers of two, but can be different, eg. a 64x16 texture is fine
 * How the vertex data is used depends on the rendering mode:
 * <br/><br/>
 * The `x' and `y' values specify the position of the vertex in 2d screen coordinates.
 * <br/><br/>
 * The `z' value is only required when doing perspective correct texture mapping, and specifies the depth of the point in 3d world coordinates.
 * <br/><br/>
 * The `u' and `v' coordinates are only required when doing texture mapping, and specify a point on the texture plane to be mapped on to this vertex. The texture plane is an infinite plane with the texture bitmap tiled across it. Each vertex in the polygon has a corresponding vertex on the texture plane, and the image of the resulting polygon in the texture plane will be mapped on to the polygon on the screen.
 * <br/><br/>
 * We refer to pixels in the texture plane as texels. Each texel is a block, not just a point, and whole numbers for u and v refer to the top-left corner of a texel. This has a few implications. If you want to draw a rectangular polygon and map a texture sized 32x32 on to it, you would use the texture coordinates (0,0), (0,32), (32,32) and (32,0), assuming the vertices are specified in anticlockwise order. The texture will then be mapped perfectly on to the polygon. However, note that when we set u=32, the last column of texels seen on the screen is the one at u=31, and the same goes for v. This is because the coordinates refer to the top-left corner of the texels. In effect, texture coordinates at the right and bottom on the texture plane are exclusive.
 * <br/><br/>
 * There is another interesting point here. If you have two polygons side by side sharing two vertices (like the two parts of folded piece of cardboard), and you want to map a texture across them seamlessly, the values of u and v on the vertices at the join will be the same for both polygons. For example, if they are both rectangular, one polygon may use (0,0), (0,32), (32,32) and (32,0), and the other may use (32,0), (32,32), (64,32), (64,0). This would create a seamless join.
 * Of course you can specify fractional numbers for u and v to indicate a point part-way across a texel. In addition, since the texture plane is infinite, you can specify larger values than the size of the texture. This can be used to tile the texture several times across the polygon. 
 * 
 * @param {POLYTYPE} type one of POLYTYPE.
 * @param {Bitmap} texture texture Bitmap.
 * @param {V3D[]} v an array of vertices.
 */
function Polygon3D(type, texture, v) { }
/**
 * Draw 3d triangles, using vertices.
 * 
 * @param {POLYTYPE} type one of POLYTYPE.
 * @param {Bitmap} texture texture Bitmap.
 * @param {V3D} v1 a vertex.
 * @param {V3D} v2 a vertex.
 * @param {V3D} v3 a vertex.
 */
function Triangle3D(type, texture, v1, v2, v3) { }
/**
 * Draw 3d quads using vertex.
 * 
 * @param {POLYTYPE} type one of POLYTYPE.
 * @param {Bitmap} texture texture Bitmap.
 * @param {V3D} v1 a vertex.
 * @param {V3D} v2 a vertex.
 * @param {V3D} v3 a vertex.
 * @param {V3D} v4 a vertex.
 */
function Quad3D(type, texture, v1, v2, v3, v4) { }
/**
 * Clips the polygon given in `v'. The frustum (viewing volume) is defined by -z&lt;x&lt;z, -z&lt;y&lt;z, 0&lt;min_z&lt;z&lt;max_z. If max_z&lt;=min_z, the z&lt;max_z clipping is not done. As you can see, clipping is done in the camera space, with perspective in mind, so this routine should be called after you apply the camera matrix, but before the perspective projection. The routine will correctly interpolate u, v, and c in the vertex structure. However, no provision is made for high/truecolor GCOL. 
 * 
 * @param {POLYTYPE} type one of POLYTYPE.
 * @param {number} min_z minimum z value.
 * @param {number} max_z maximum z value.
 * @param {V3D[]} v an array of vertices.
 * 
 * @returns {V3D[]} an array of vertices.
 */
function Clip3D(type, min_z, max_z, v) { }
