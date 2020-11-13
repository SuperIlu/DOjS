/********
 * File
 */
/**
 * Open a file, for file modes see {@link FILE}. Files can only either be read or written, never both. Writing to a closed file throws an exception.
 * @class
 * @param {string} filename the name of the file.
 * @param {FILE} mode READ, WRITE or APPEND.
 */
function File(filename, mode) { }
/**
 * Read a single byte from file and return it as number.
 * @returns {number} the byte as a number or null for EOF.
 */
File.prototype.ReadByte = function () { };
/**
 * Write a single byte to a file.
 * @param {number} ch the byte to write.
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
/**
 * get file contents as number array.
 * @returns {number[]} the remaining contents of the file as array of numbers.
 */
File.prototype.ReadBytes = function () { };
/**
 * get file size.
 * @returns {number} the size of the file in bytes.
 */
File.prototype.GetSize = function () { };
/**
 * Write a bytes to a file.
 * @param {number[]} data the data to write as array of numbers (must be integers between 0-255).
 */
File.prototype.WriteBytes = function (data) { };

/********
 * Bitmap
 */
/**
* Load a BMP, TGA or PCX image.
* @constructor 
* @param {string} filename name of the BMP or PNG file.
*//**
* create empty bitmap of given size.
* @constructor 
* @param {number} width bitmap width.
* @param {number} height bitmap width.
*//**
* create Bitmap from integer array.
* @constructor 
* @param {number[]} data 32bit integer data interpreted as ARGB.
* @param {number} width bitmap width.
* @param {number} height bitmap height.
*//**
* create Bitmap from current(3dfx) screen.
* @constructor 
* @param { number } x screen x position.
* @param { number } y screen y position.
* @param { number } width bitmap width.
* @param { number } height bitmap height.
* @param { GR_BUFFER } [buffer] one of FRONTBUFFER, BACKBUFFER or AUXBUFFER for 3dfx access, omit for normal screen acccess.
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
 * @param {string} fname filename.
 */
Bitmap.prototype.SavePngImage = function (fname) { };

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
 * Font height
 */
Font.height = null;
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
 * Sound frequency.
 */
Sample.frequency = null;
/**
 * Sound resolution.
 */
Sample.bits = null;
/**
 * mono/stereo indicator.
 */
Sample.stereo = null;
/**
 * Play the WAV.
 * @param {number} volume between 0-255.
 * @param {number} panning between (left) 0-255 (right).
 * @param {boolean} loop true := sample will loop, false := sample will only be played once.
 * @returns {number} used voice number or null if not played.
 */
Sample.prototype.Play = function (volume, panning, loop) { };
/**
 * Stop playing.
 */
Sample.prototype.Stop = function () { };
/**
 * Get sample data.
 * @param {number} sample index to return.
 * @returns {number} The sample value at that position. The sample data are always in unsigned format.
 */
Sample.prototype.Get = function (idx) { };

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


/********
 * ZBuffer
 */
/**
 * Create a ZBuffer. A ZBuffer must be set as current before using it.
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

/********
 * TexInfo
 */
/**
 * create new texture from 3df file or a Bitmap.
 * @class
 * @param {(string|Bitmap)} src 3df file to load as texture or Bitmap to convert to texture
 */
function TexInfo(src) { }
/**
 * filename
 */
TexInfo.filename = null;
/**
 * large LOD
 */
TexInfo.largeLod = null;
/**
 * small LOD
 */
TexInfo.smallLod = null;
/**
 * aspect ratio
 */
TexInfo.aspectRatio = null;
/**
 * texture format
 */
TexInfo.format = null;
/**
 * table type
 */
TexInfo.tableType = null;
/**
 * size of texture
 */
TexInfo.textureSize = null;
/**
 * texture mem address (if downloaded)
 */
TexInfo.address = null;
/**
 * TMU (if downloaded)
 */
TexInfo.tmu = null;

/**
 * download the MIP map to texture memory.
 * @param {GR_TMU} tmu the TMU unit to download to.
 * @param {number} address destination memory address in texture memory.
 * @param {GR_MIPMAPLEVELMASK} evenOdd one of GR_MIPMAPLEVELMASK.
 */
TexInfo.prototype.DownloadMipMap = function (tmu, address, evenOdd) { };
/**
 * mark the texture as 'not downloaded' again.
 */
TexInfo.prototype.MarkUnused = function () { };
/**
 * specify this TexInfo as the current texture source for rendering.
 * @param {GR_MIPMAPLEVELMASK} evenOdd one of GR_MIPMAPLEVELMASK.
 */
TexInfo.prototype.Source = function (evenOdd) { };
/**
 * return the texture memory consumed by a texture
 * @param {GR_MIPMAPLEVELMASK} evenOdd one of GR_MIPMAPLEVELMASK.
 * @returns {number} size of texture in bytes.
 */
TexInfo.prototype.MemRequired = function (evenOdd) { };

/********
 * FxState
 */
/**
 * save current glide state into this object.
 * @class
 */
function FxState() { }
/**
 * restore glide state from this object.
 */
FxState.prototype.Set = function () { }

/********
 * COMPort
 */
/**
 * open a COM port.
 * 
 * @class
 * @param {number} port one of COM.PORT: COM1, COM2, COM3, COM4.
 * @param {number} baud one of COM.BAUD: B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, B2400, B4800, B9600, B19200, B38400, B57600, B115200
 * @param {number} bits one of COM.BIT: BITS_5, BITS_6, BITS_7, BITS_8
 * @param {number} parity one of COM.PARITY: NO_PARITY, ODD_PARITY, EVEN_PARITY, MARK_PARITY, SPACE_PARITY
 * @param {number} stop one of COM.STOP: STOP_1, STOP_2
 * @param {number} flow one of COM.FLOW: NO_CONTROL, XON_XOFF, RTS_CTS
 * @param {number} [addr] optional: port io address
 * @param {number} [irq] optional: port IRQ
 */
function COMPort(port, baud, bits, parity, stop, flow, addr, irq) { }
/**
 * close port.
 */
COMPort.prototype.Close = function () { };
/**
 * flush input buffer.
 */
COMPort.prototype.FlushInput = function () { };
/**
 * flush output buffer.
 */
COMPort.prototype.FlushOutput = function () { };
/**
 * check state of send buffer.
 * @returns {boolean} true if the output buffer is empty.
 */
COMPort.prototype.IsOutputEmpty = function () { };
/**
 * check state of send buffer.
 * @returns {boolean} true if the output buffer is full.
 */
COMPort.prototype.IsOutputFull = function () { };
/**
 * check state of receive buffer.
 * @returns {boolean} true if the input buffer is empty.
 */
COMPort.prototype.IsInputEmpty = function () { };
/**
 * check state of receive buffer.
 * @returns {boolean} true if the input buffer is full.
 */
COMPort.prototype.IsInputFull = function () { };
/**
 * Write a single byte to COM port.
 * @param {number} ch the byte to write.
 */
COMPort.prototype.WriteByte = function (ch) { };
/**
 * Write a string to COM port.
 * @param {string} txt the string to write.
 */
COMPort.prototype.WriteString = function (txt) { };
/**
 * read a byte from COM port
 * @returns {number} the value of the byte.
 */
COMPort.prototype.ReadByte = function () { };
/**
 * read a string from the receive buffer.
 * @returns {string} contents of the received buffer.
 */
COMPort.prototype.ReadBuffer = function () { };

/********
 * Socket
 */
/**
 * @class
 */
function Socket() { }

/**
 * flush the socket output to the network.
 */
Socket.prototype.Flush = function () { };

/**
 * close the socket.
 * @param {boolean} [doFlush=false] flush before close.
 */
Socket.prototype.Close = function (doFlush) { }

/**
 * Wait until all written data is flushed.
 */
Socket.prototype.WaitFlush = function () { }

/**
 * Wait on socket for incoming data with timeout.
 * @param {boolean} [timeout=1] max wait time.
 */
Socket.prototype.WaitInput = function (timeout) { }

/**
 * Get the next byte from the socket as number.
 * @returns {number} the next byte from the socket.
 */
Socket.prototype.ReadByte = function () { }


/**
 * write a byte to a socket.
 * @param {number} ch the byte to write.
 */
Socket.prototype.WriteByte = function (ch) { }

/**
 * send binary data.
 * @param {number[]} data data to write as number array.
 */
Socket.prototype.WriteBytes = function (data) { }

/**
 * send string.
 * @param {string} str data to send.
 */
Socket.prototype.WriteString = function (str) { }

/**
 * Set binary or ascii mode for UDP/TCP sockets.
 * @param {number} mode one of SOCKET.MODE.
 */
Socket.prototype.Mode = function (mode) { }

/**
 * Send pending TCP data.
 * @param {boolean} [flushWait=false] wait until data is flushed.
 */
Socket.prototype.Flush = function (flushWait) { }

/**
 * Sets non-flush mode on next TCP write.
 */
Socket.prototype.NoFlush = function () { }

/**
 * Causes next transmission to have a flush (PUSH bit set).
 */
Socket.prototype.FlushNext = function () { }

/**
 * Get number of bytes waiting to be read.
 * @returns {number} number of bytes waiting to be read.
 */
Socket.prototype.DataReady = function () { }

/**
 * Check if the socket connection is established.
 * @returns {boolean} true if the connection is established.
 */
Socket.prototype.Established = function () { }

/**
 * Get the remote host ip
 * @returns {IpAddress} IP of the remote host.
 */
Socket.prototype.GetRemoteHost = function () { }

/**
 * Get the local port number.
 * @returns {number} the local port number.
 */
Socket.prototype.GetLocalPort = function () { }

/**
 * Get the remote port number.
 * @returns {number} the remote port number.
 */
Socket.prototype.GetRemotePort = function () { }

/**
 * Return the next line from the socket as string.
 * This method blocks until a newline is read.
 * @returns {string} the next line from the socket as string.
 */
Socket.prototype.ReadLine = function () { }

/**
 * Return data as string.
 * This method blocks until 'len' bytes have been read.
 * 
 * @param {number} len number of bytes to read from socket.
 * 
 * @returns {string} data as string.
 */
Socket.prototype.ReadString = function (len) { }

/**
 * Return data as array of numbers.
 * This method blocks until 'len' bytes have been read.
 * 
 * @param {number} len number of bytes to read from socket.
 * 
 * @returns {number[]} data as array.
 */
Socket.prototype.ReadBytes = function (len) { }

/********
 * Zip
 */
/**
 * Open a ZIP, for file modes see {@link ZIPFILE}.
 * @class
 * @param {string} filename the name of the file.
 * @param {FILE} mode READ, WRITE or APPEND.
 * @param {number} [compression] 1..9 to specify compression level.
 */
function Zip(filename, mode, compression) { }
/**
 * close ZIP.
 */
Zip.prototype.Close = function () { };
/**
 * get number of entries in ZIP.
 * 
 * @returns {number} number of entries in this ZIP.
 */
Zip.prototype.NumEntries = function () { };
/**
 * get an array with the file entries in the ZIP.
 * 
 * @returns {*} an array containing the file entries of the ZIP in the following format:
 * @example
 * [
 *      [name:string, is_directory:bool, size:number, crc32:number],
 *      ...
 * ]
 */
Zip.prototype.GetEntries = function () { };
/**
 * add a file to a ZIP.
 * 
 * @param {string} zip_name the full path the file shall have in the ZIP.
 * @param {string} hdd_name the full path to the file to add.
 */
Zip.prototype.AddFile = function (zip_name, hdd_name) { };
/**
 * extract a file from a ZIP.
 * 
 * @param {string} zip_name the full path of the file in the ZIP.
 * @param {string} hdd_name the full path the extracted file should have on the HDD.
 */
Zip.prototype.ExtractFile = function (zip_name, hdd_name) { };
/**
 * get file contents as number array.
 * @param {string} zip_name the full path of the file in the ZIP.
 * @returns {number[]} the content of the file as array of numbers.
 */
Zip.prototype.ReadBytes = function (zip_name) { };
/**
 * Write a bytes to a file in the ZIP.
 * @param {string} zip_name the full path of the file in the ZIP.
 * @param {number[]} data the data to write as array of numbers (must be integers between 0-255).
 */
File.prototype.WriteBytes = function (zip_name, data) { };
