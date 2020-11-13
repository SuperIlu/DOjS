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
 * @property {string[]} ARGS the command line arguments including the script name.
 */
ARGS = [];

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
 * @module sndout
 */
/**
 * Get current play position for given voice.
 * @param {number} voc the voice number.
 * @returns {number} the position or -1 if not playing.
 */
function VoiceGetPosition(voc) { }

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
 * Get song position.
 * @returns {number} current song position in MIDI file.
 */
function MidiGetPos() { }

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
 * Save current screen to PNG file.
 * @param {string} fname filename.
 */
function SavePngImage(fname) { }

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
 * Load the contents of a ZIP file entry into a string. Throws exception if loading fails.
 * @param {string} filename name of file to read.
 * @param {string} entryname name of entry in the ZIP file to read.
 * @returns {string} the contents of the file.
 * @throws Throws an error if reading fails.
 */
function ReadZIP(filename, entryname) { }

/**
 * Get directory listing.
 * @param {string} dname name of directory to list.
 * @returns {string[]} array of entry names.
 * @throws Throws an error if listing fails.
 */
function List(dname) { }

/**
 * rename file/directory.
 * @param {string} from old name
 * @param {string} to new name
 */
function Rename(from, to) { }

/**
 * remove a directory (must be empty).
 * @param {string} name path/name of the directory.
 */
function RmDir(name) { }

/**
 * remove a file.
 * @param {string} name path/name of the file.
 */
function RmFile(name) { }

/**
 * make a directory.
 * @param {string} name path/name of the new directory
 */
function MkDir(name) { }


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

/**
 * Run a DOS command.
 * @param {string} cmd the command to execute with its parameters.
 * @param {SYSTEM} flags flags indicating which subsystems to shutdown (if any) during execution of cmd.
 * @returns {number} the return code of the command.
 */
function System(cmd, flags) { }

/**
 * write a byte value to a hardware io-port.
 * @param {number} port port address to write to.
 * @param {number} value 8-bit value to write to port.
 */
function OutPortByte(port, value) { }

/**
 * write a word value to a hardware io-port.
 * @param {number} port port address to write to.
 * @param {number} value 16-bit value to write to port.
 */
function OutPortWord(port, value) { }

/**
 * write a long value to a hardware io-port.
 * @param {number} port port address to write to.
 * @param {number} value 32-bit value to write to port.
 */
function OutPortLong(port, value) { }

/**
 * read a byte value from a hardware io-port.
 * @param {number} port port address to read from.
 * @returns {number} 8-bit value read from port.
 */
function InPortByte(port) { }

/**
 * read a word value from a hardware io-port.
 * @param {number} port port address to read from.
 * @returns {number} 16-bit value read from port.
 */
function InPortWord(port) { }

/**
 * read a long value from a hardware io-port.
 * @param {number} port port address to read from.
 * @returns {number} 32-bit value read from port.
 */
function InPortLong(port) { }

/**
 * get available parallel ports.
 * @returns {number[]} list of available parallel ports and their addresses.
 */
function GetParallelPorts() { }

/**
 * get available serial ports.
 * @returns {number[]} list of available serial ports and their addresses.
 */
function GetSerialPorts() { }

/**
 * Convert byte array to ASCII string. The string is terminated at the first NULL byte or at array length (whichever comes first).
 * 
 * @param {number[]} data array of numbers.
 * @returns {string} a string.
 */
function BytesToString(data) { }

/**
 * Convert ASCII string to byte array.
 * 
 * @param {string} str string to convert.
 * @returns {number[]} array of numbers.
 */
function StringToBytes(str) { }

/**
 * parse a string into a function. Works like Function() by a source file name can be provided.
 * 
 * @param {string} p name of the single parameter.
 * @param {string} s the source of the function.
 * @param {string} [f] an optional filename where the source came from.
 */
function NamedFunction(p, s, f) { }

/** @module color */

/**
 * create RGBA color.
 * 
 * @param {number} r red (0-255)
 * @param {number} g green (0-255)
 * @param {number} b blue (0-255)
 * @param {number} [a] alpha (0-255) (optional)
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

/**
 * Construct X axis rotation matrices. When applied to a point, these matrices will rotate it about the X axis by the specified angle (given in radians).
 * 
 * @param {number} r rotation in radians.
 * 
 * @returns a {@link Matrix}.
 */
function GetXRotateMatrix(r) { }

/**
 * Construct Y axis rotation matrices. When applied to a point, these matrices will rotate it about the Y axis by the specified angle (given in radians).
 * 
 * @param {number} r rotation in radians.
 * 
 * @returns a {@link Matrix}.
 */
function GetYRotateMatrix(r) { }

/**
 * Construct Z axis rotation matrices. When applied to a point, these matrices will rotate it about the Z axis by the specified angle (given in radians).
 * 
 * @param {number} r rotation in radians.
 * 
 * @returns a {@link Matrix}.
 */
function GetZRotateMatrix(r) { }

/**
 * Constructs a transformation matrix which will rotate points around all three axes by the specified amounts (given in radians). 
 * The direction of rotation can simply be found out with the right-hand rule: Point the dumb of your right hand towards the origin along the axis of rotation, and the fingers will curl in the positive direction of rotation. 
 * E.g. if you rotate around the y axis, and look at the scene from above, a positive angle will rotate in clockwise direction. 
 * 
 * @param {*} x x value or a vector as array.
 * @param {number} y y value.
 * @param {number} z y value.
 * 
 * @returns a {@link Matrix}.
 */
function GetRotationMatrix(x, y, z) { }

/**
 * Constructs a transformation matrix which will rotate points around all three axes by the specified amounts (given in radians), scale the result by the specified amount (pass 1 for no change of scale), and then translate to the requested x, y, z position.
 * 
 * @param {number} scale scaling value.
 * @param {number} xrot x-rotation value.
 * @param {number} yrot y-rotation value.
 * @param {number} zrot z-rotation value.
 * @param {number} x x value.
 * @param {number} y y value.
 * @param {number} z y value.
 * 
 * @returns a {@link Matrix}.
 */
function GetTransformationMatrix(scale, xrot, yrot, zrot, x, y, z) { }

/**
 * Multiplies two matrices. 
 * The resulting matrix will have the same effect as the combination of m1 and m2, ie. when applied to a point p, (p * out) = ((p * m1) * m2). 
 * Any number of transformations can be concatenated in this way. 
 * Note that matrix multiplication is not commutative, ie. matrix_mul(m1, m2) != matrix_mul(m2, m1). 
 * 
 * @param {Matrix} m1 first {@link Matrix}.
 * @param {Matrix} m2 second {@link Matrix}.
 * 
 * @returns a new {@link Matrix}.
 */
function MatrixMul(m1, m2) { }

/**
 * Finds the Z component of the normal vector to the specified three vertices (which must be part of a convex polygon). 
 * This is used mainly in back-face culling. The back-faces of closed polyhedra are never visible to the viewer, therefore they never need to be drawn. 
 * This can cull on average half the polygons from a scene. 
 * If the normal is negative the polygon can safely be culled. If it is zero, the polygon is perpendicular to the screen.
 * However, this method of culling back-faces must only be used once the X and Y coordinates have been projected into screen space using PerspProject() (or if an orthographic (isometric) projection is being used). 
 * Note that this function will fail if the three vertices are co-linear (they lie on the same line) in 3D space. 
 * 
 * @param {number[]} v1 first vector.
 * @param {number[]} v2 second vector.
 * @param {number[]} v3 third vector.
 * 
 * @returns {number} z component.
 */
function PolygonZNormal(v1, v2, v3) { }

/**
 * Multiplies the point (x, y, z) by the transformation matrix m.
 * 
 * @param {Matrix} m the matrix
 * @param {*} x x value or vector as array.
 * @param {number} y y value.
 * @param {number} z y value.
 * 
 * @returns {number[]} a new vector.
 */
function ApplyMatrix(m, x, y, z) { }

/**
 * Projects the 3d point (x, y, z) into 2d screen space and using the scaling parameters previously set by calling SetProjectionViewport(). 
 * This function projects from the normalized viewing pyramid, which has a camera at the origin and facing along the positive z axis. 
 * The x axis runs left/right, y runs up/down, and z increases with depth into the screen. 
 * The camera has a 90 degree field of view, ie. points on the planes x=z and -x=z will map onto the left and right edges of the screen, and the planes y=z and -y=z map to the top and bottom of the screen. 
 * If you want a different field of view or camera location, you should transform all your objects with an appropriate viewing matrix, eg. to get the effect of panning the camera 10 degrees to the left, rotate all your objects 10 degrees to the right. 
 * 
 * @param {*} x x value or vector as array.
 * @param {number} y y value.
 * @param {number} z y value.
 */
function PerspProject(x, y, z) { }

/**
 * @module joystick
 */

/**
 * @property {boolean} JOYSTICK_AVAILABLE true if joystick support is available.
 */
JOYSTICK_AVAILABLE = false;

/**
 * @property {number} NUM_JOYSTICKS number of available joysticks.
 */
NUM_JOYSTICKS = 0;

/**
 * save joystick calibration info to file.
 * 
 * @param {string} file the file to save to.
 */
function JoystickSaveData(file) { }

/**
 * load joystick calibration info from file.
 * 
 * @param {string} file the file to load from.
 */
function JoystickLoadData(file) { }

/**
 * Pass the number of the joystick you want to calibrate as the parameter. 
 * 
 * @param {number} num joystick index (starting with 0).
 * 
 * @returns {string} Returns a text description for the next type of calibration that will be done on the specified joystick, or null if no more calibration is required. 
 */
function JoystickCalibrateName(num) { }

/**
 * Most joysticks need to be calibrated before they can provide full analogue input. This function performs the next operation in the calibration series for the specified stick, assuming that the joystick has been positioned in the manner described by a previous call to calibrate_joystick_name().
 * 
 * @param {number} num joystick index (starting with 0).
 */
function JoystickCalibrate(num) { }

/**
 * The joystick handler is not interrupt driven, so you need to call this function every now and again to update the global position values.
 * 
 * @param {number} num joystick index (starting with 0).
 * 
 * @returns {JoyInfo} Information about the joystick state.
 */
function JoystickPoll(num) { }

/**
 * @module 3dfx
 */

/**
 * @property {number} voodoo graphics width
 */
FX_WIDTH = 640;
/**
 * @property {number} voodoo graphics height
 */
FX_HEIGHT = 480;

/**
 * init 3dfx glide.
 * the following parameters are used:
 * GR_RESOLUTION_640x480,
 * GR_REFRESH_60Hz,
 * GR_COLORFORMAT_ARGB,
 * GR_ORIGIN_UPPER_LEFT,
 * GR_WINDOW_COORDS
 */
function fxInit() { }

/**
 * shut down the Glide library
 */
function fxShutdown() { }

/**
 * flush the graphics FIFO
 */
function fxFlush() { }

/**
 * Reset grVertexLayout parameter offset to zero, and all parameter modes to GR_PARAM_DISABLE.
 */
function fxResetVertexLayout() { }

/**
 * specify the format of by-vertex arrays
 * @param {GR_PARAM[]} layout list of layout parameters.
 */
function fxVertexLayout(layout) { }

/**
 * @returns {number} the size of the currently active vertex layout in 'number of entries'.
 */
function fxGetVertexSize() { }

/**
 * force completion of all outstanding graphics commands.
 */
function fxFinish() { }

/**
 * exchange front and back buffers
 * @param {number} interval The number of vertical retraces to wait before swapping the front and back buffers.
 */
function fxBufferSwap(interval) { }

/**
 * clear the buffers to the specified values
 * @param {number} color The color value used for clearing the draw buffer.
 * @param {number} alpha The alpha value used for clearing the alpha buffer
 * @param {number} depth An unsigned value used for clearing the depth buffer
 */
function fxBufferClear(color, alpha, depth) { }

/**
 * set the size and location of the hardware clipping window
 * @param {number} minx The lower x screen coordinate of the clipping window.
 * @param {number} miny The lower y screen coordinate of the clipping window.
 * @param {number} maxx The upper x screen coordinate of the clipping window.
 * @param {number} maxy The upper y screen coordinate of the clipping window.
 */
function fxClipWindow(minx, miny, maxx, maxy) { }

/**
 * draw a point
 * @param {number[]} v1 a vertex.
 */
function fxDrawPoint(v1) { }

/**
 * draw a one-pixel-wide arbitrarily oriented line
 * @param {number[]} v1 a vertex.
 * @param {number[]} v2 a vertex.
 */
function fxDrawLine(v1, v2) { }

/**
 * draw a triangle
 * @param {number[]} v1 a vertex.
 * @param {number[]} v2 a vertex.
 * @param {number[]} v3 a vertex.
 */
function fxDrawTriangle(v1, v2, v3) { }

/**
 * set the global constant color
 * @param {number} color The new constant color.
 */
function fxConstantColorValue(color) { }

/**
 * set the cull mode.
 * @param {GR_CULL} mode the noew mode.
 */
function fxCullMode(mode) { }

/**
 * specify the alpha blending function
 * @param {GR_BLEND} rgb_sf rgb source blending factor
 * @param {GR_BLEND} rgb_df rgb destination blending factor
 * @param {GR_BLEND} alpha_sf alpha source blending factor
 * @param {GR_BLEND} alpha_df alpha destination blending factor
 */
function fxAlphaBlendFunction(rgb_sf, rgb_df, alpha_sf, alpha_df) { }

/**
 * configure the alpha combine unit.
 * @param {GR_COMBINE_FUNCTION} func function
 * @param {GR_COMBINE_FACTOR} factor scaling factor
 * @param {GR_COMBINE_LOCAL} local local alpha
 * @param {GR_COMBINE_OTHER} other other alpha
 * @param {boolean} invert invert generated alpha.
 */
function fxAlphaCombine(func, factor, local, other, invert) { }

/**
 * configure the color combine unit.
 * @param {GR_COMBINE_FUNCTION} func function
 * @param {GR_COMBINE_FACTOR} factor scaling factor
 * @param {GR_COMBINE_LOCAL} local local alpha
 * @param {GR_COMBINE_OTHER} other other alpha
 * @param {boolean} invert invert generated alpha.
 */
function fxColorCombine(func, factor, local, other, invert) { }

/**
 * enable/disable writing into the color and alpha buffers
 * @param {number} rgb color mask
 * @param {number} alpha alpha mask
 */
function fxColorMask(rgb, alpha) { }

/**
 * enable/disable writing into the depth buffer
 * @param {boolean} enable enable/disable
 */
function fxDepthMask(enable) { }

/**
 * draw a list of by-vertex vertices
 * @param {GR_VERTEX} mode vertex type
 * @param {number[][]} vertices array of vertices.
 */
function fxDrawVertexArray(mode, vertices) { }

/**
 * enable Glide operating modes
 * @param {GR_ENABLE} val one of GR_ENABLE.
 */
function fxEnable(val) { }

/**
 * enable Glide operating modes
 * @param {GR_ENABLE} val one of GR_ENABLE.
 */
function fxDisable(val) { }

/**
 * disable all special effects in the graphics subsystem
 */
function fxDisableAllEffects() { }

/**
 * draw an anti-aliased triangle
 * @param {number[]} a a vertex
 * @param {number[]} b a vertex
 * @param {number[]} c a vertex
 * @param {boolean} b1 anti alias AB edge
 * @param {boolean} b2 anti alias BC edge
 * @param {boolean} b3 anti alias CA edge
 */
function fxAADrawTriangle(a, b, c, b1, b2, b3) { }

/**
 * set dither mode.
 * @param {GR_DITHER} mode the new dither mode.
 */
function fxDitherMode(mode) { }

/**
 * enables/disables alpha controlled lighting
 * @param {boolean} enable enable/disable
 */
function fxAlphaControlsITRGBLighting(enable) { }

/**
 * set up gamma correction tables
 * @param {number} r 
 * @param {*} g 
 * @param {*} b 
 */
function fxGammaCorrectionRGB(r, g, b) { };

/**
 * establishes a y origin
 * @param {GR_ORIGIN} origin set y origin.
 */
function fxOrigin(origin) { }

/**
 * set the depth buffering mode
 * @param {GR_DEPTHBUFFER} mode the mode 
 */
function fxDepthBufferMode(mode) { }

/**
 * specify the depth buffer comparison function
 * @param {GR_CMP} func the new function
 */
function fxDepthBufferFunction(func) { }

/**
 * set the depth bias level
 * @param {number} level th new level.
 */
function fxDepthBiasLevel(level) { }

/**
 * specify viewport depth range
 * @param {number} near min range
 * @param {number} far max range
 */
function fxDepthRange(near, far) { }

/**
 * define a viewport
 * @param {number} x The origin of the viewport, relative to the screen origin
 * @param {number} y The origin of the viewport, relative to the screen origin
 * @param {number} width The width and height of the viewport.
 * @param {number} height The width and height of the viewport.
 */
function fxViewport(x, y, width, height) { }

/**
 * enable/disable per-pixel fog blending operations
 * @param {GR_FOG} mode the new fog mode.
 */
function fxFogMode(mode) { }

/**
 * set the global fog color
 * @param {number} color the new fog color
 */
function fxFogColorValue(color) { }

/**
 * convert a fog table index to a floating point eye-space w value
 * @param {number} i The fog table index, between 0 and GR_FOG_TABLE_SIZE. 
 */
function fxFogTableIndexToW(table) { }

/**
 * download a fog table
 * @param {number[]} table a new table with at least fxGetFogTableEntries() entries
 */
function fxFogTable(table) { }

/**
 * generate an exponential fog table
 * @param {number} density The fog density, typically between 0.0 and 1.0.
 * @returns {number[]} a fog table.
 */
function fxFogGenerateExp(density) { }

/**
 * generate an exponential squared fog table
 * @param {*} density The fog density, typically between 0.0 and 1.0.
 * @returns {number[]} a fog table.
 */
function fxFogGenerateExp2(density) { }

/**
 * generate a linear fog table
 * @param {number} near The eye-space w coordinate where minimum fog exists.
 * @param {number} far The eye-space w coordinate where maximum fog exists.
 * @returns {number[]} a fog table.
 */
function fxFogGenerateLinear(near, far) { }

/**
 * enable/disable hardware chroma-keying
 * @param {boolean} mode enable/disable
 */
function fxChromakeyMode(mode) { }

/**
 * set the global chroma-key reference value
 * @param {number} val The new chroma-key reference value.
 */
function fxChromakeyValue(val) { }

/**
 * specify the alpha test function
 * @param {GR_CMP} func the function
 */
function fxAlphaTestFunction(func) { }

/**
 * specify the alpha test reference value
 * @param {number} value The new alpha test reference value.
 */
function fxAlphaTestReferenceValue(value) { }

/**
 * specify the texture minification and magnification filters
 * @param {GR_TMU} tmu the TMU.
 * @param {GR_TEXTUREFILTER} minFilter The minification filter
 * @param {GR_TEXTUREFILTER} magFilter The magnification filter
 */
function fxTexFilterMode(tmu, minFilter, magFilter) { }

/**
 * set the texture map clamping/wrapping mode
 * @param {GR_TMU} tmu the TMU.
 * @param {GR_TEXTURECLAMP} sMode The new mode for the s direction
 * @param {GR_TEXTURECLAMP} tMode The new mode for the t direction
 */
function fxTexClampMode(tmu, sMode, tMode) { }

/**
 * set the mipmapping mode
 * @param {GR_TMU} tmu the TMU.
 * @param {GR_MIPMAP} mode The new mipmapping mode
 * @param {boolean} lodBlend enables/disables LOD blending
 */
function fxTexMipMapMode(tmu, mode, lodBlend) { }

/**
 * set the LOD bias value
 * @param {GR_TMU} tmu the TMU.
 * @param {number} bias The new LOD bias value, a signed floating point value in the range [-8..7.75].
 */
function fxTexLodBiasValue(tmu, bias) { }

/**
 * configure a texture combine unit
 * @param {GR_TMU} tmu the TMU.
 * @param {GR_COMBINE_FUNCTION} rgb_func Specifies the function used in texture color generation
 * @param {GR_COMBINE_FACTOR} rgb_factor Specifies the scaling factor f used in texture color generation
 * @param {GR_COMBINE_FUNCTION} alpha_func Specifies the function used in texture alpha generation
 * @param {GR_COMBINE_FACTOR} alpha_factor Specifies the scaling factor f used in texture alpha generation
 * @param {boolean} rgb_invert Specifies whether the generated texture color should be bitwise inverted as a final step.
 * @param {boolean} alpha_invert Specifies whether the generated texture alpha should be bitwise inverted as a final step.
 */
function fxTexCombine(tmu, rgb_func, rgb_factor, alpha_func, alpha_factor, rgb_invert, alpha_invert) { }

/**
 * set the detail texturing controls
 * @param {GR_TMU} tmu the TMU.
 * @param {number} lodBias Controls where the blending between the two textures begins. This value is an LOD bias value in the range [–32.. +31].
 * @param {number} detailScale Controls the steepness of the blend. Values are in the range [0..7] are valid. The scale is computed as 2^detailScale.
 * @param {number} detailMax Controls the maximum blending that occurs. Values in the range [0.0..1.0] are valid.
 */
function fxTexDetailControl(tmu, lodBias, detailScale, detailMax) { }

/**
 * return the texture memory consumed by a texture.
 * @param {GR_LOD} smallLod smallest level of detail
 * @param {GR_LOD} largeLod larges level of detail
 * @param {GR_ASPECT} aspect texture aspect ratio
 * @param {GR_TEXFMT} format texture format
 * 
 * @returns {number} number of bytes required
 */
function fxTexCalcMemRequired(smallLod, largeLod, aspect, format) { }

/**
 * selects the current color buffer for drawing and clearing
 * @param {GR_BUFFER} buffer Selects the current color buffer. Valid values are GR_BUFFER_FRONTBUFFER and GR_BUFFER_BACKBUFFER.
 */
function fxRenderBuffer(buffer) { }

/**
 * return the lowest start address for texture downloads
 * @param {GR_TMU} tmu the TMU.
 */
function fxTexMinAddress(tmu) { }

/**
 * return the highest start address for texture downloads
 * @param {GR_TMU} tmu the TMU.
 */
function fxTexMaxAddress(tmu) { }

/**
 * select an NCC table
 * @param {GR_TEXTABLE} table NCC table to use for decompressing compressed textures. Valid values are GR_TEXTABLE_NCC0 and GR_TEXTABLE_NCC1.
 */
function fxTexNCCTable(table) { }

// GrGet() values
/**
 * @returns {number[]} The minimum and maximum allowable z buffer values.
 */
function fxGetZDepthMinMax() { }
/**
 * @returns {number[]} The minimum and maximum allowable w buffer values.
 */
function fxGetWDepthMinMax() { }
/**
 * @returns {number} The number of bits of depth (z or w) in the frame buffer.
 */
function fxGetBitsDepth() { }
/**
 * @returns {number} The number of entries in the hardware fog table.
 */
function fxGetFogTableEntries() { }
/**
 * @returns {number} The number of entries in the hardware gamma table. Returns FXFALSE if it is not possible to manipulate gamma (e.g. on a Macronix card, or in windowed mode).
 */
function fxGetGammaTableEntries() { }
/**
 * @returns {number} Returns FXFALSE if idle, FXTRUE if busy.
 */
function fxIsBusy() { }
/**
 * @returns {number} The total number of bytes per Pixelfx chip if a non-UMA configuration is used, else 0. In non-UMA configurations, the total FB memory is GR_MEMORY_FB * GR_NUM_FB.
 */
function fxGetMemoryFb() { }
/**
 * @returns {number} The total number of bytes per Texelfx chip if a non-UMA configuration is used, else FXFALSE. In non-UMA configurations, the total usable texture memory is GR_MEMORY_TMU * GR_NUM_TMU.
 */
function fxGetMemoryTmu() { }
/**
 * @returns {number} The total number of bytes if a UMA configuration, else 0.
 */
function fxGetMemoryUma() { }
/**
 * @returns {number} The width of the largest texture supported on this configuration (e.g. Voodoo Graphics returns 256).
 */
function fxGetMaxTextureSize() { }
/**
 * @returns {number} The logarithm base 2 of the maximum aspect ratio supported for power-of-two, mipmap-able textures (e.g. Voodoo Graphics returns 3).
 */
function fxGetMaxTextureAspectRatio() { }
/**
 * @returns {number} The number of installed boards supported by Glide.
 */
function fxGetNumBoards() { }
/**
 * @returns {number} The number of Pixelfx chips present. This number will always be 1 except for SLI configurations.
 */
function fxGetNumFb() { }
/**
 * @returns {number} The number of Texelfx chips per Pixelfx chip. For integrated chips, the number of TMUs will be returned.
 */
function fxGetNumTmu() { }
/**
 * @returns {number} The number of buffer swaps pending.
 */
function fxGetNumPendingBufferSwaps() { }
/**
 * @returns {number} The revision of the Pixelfx chip(s).
 */
function fxGetRevisionFb() { }
/**
 * @returns {number} The revision of the Texelfx chip(s).
 */
function fxGetRevisionTmu() { }

/**
 * Alpha value to use for direct framebuffer access.
 * 
 * @param {number} val the constant alpha value.
 */
function fxLfbConstantAlpha(val) { }

/**
 * Depth value to use for direct framebuffer access.
 * 
 * @param {number} val the constant depth value.
 */
function fxLfbConstantDepth(val) { }

/**
 * @module tcpip
 */

/**
 * get the local IP address.
 * @returns {IpAddress} the local IP address as an array of numbers
 */
function GetLocalIpAddress() { }
/**
 * get the netmask.
 * @returns {IpAddress} the netmask as an array of numbers
 */
function GetNetworkMask() { }
/**
 * get the hostname.
 * @returns {string} the hostname
 */
function GetHostname() { }
/**
 * get the domain name.
 * @returns {string} the domain name
 */
function GetDomainname() { }
/**
 * look up a hostname in DNS.
 * @param {string} host the hostname.
 * @returns {IpAddress} The IP address of the host or an exception.
 */
function Resolve(host) { }
/**
 * reverse look up a host in DNS.
 * @param {IpAddress} host the ip address.
 * @returns {string} The name of the host or an exception.
 */
function ResolveIp(ip) { }
