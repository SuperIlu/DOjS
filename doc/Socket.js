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
 * send string.
 * @param {string} str data to send.
 */
Socket.prototype.WriteString = function (str) { }
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
 * Return data as array of numbers.
 * This method blocks until 'len' bytes have been read.
 * 
 * @param {number} len number of bytes to read from socket.
 * 
 * @returns {number[]} data as array.
 */
Socket.prototype.ReadBytes = function (len) { }
/**
 * send binary data.
 * @param {number[]} data data to write as number array.
 */
Socket.prototype.WriteBytes = function (data) { }
/**
 * Return data as ByteArray.
 * This method blocks until 'len' bytes have been read.
 * 
 * @param {number} len number of bytes to read from socket.
 * 
 * @returns {ByteArray} data as ByteArray.
 */
Socket.prototype.ReadInts = function (len) { }
/**
 * send binary data.
 * @param {ByteArray} data data to write as ByteArray.
 */
Socket.prototype.WriteInts = function (data) { }
