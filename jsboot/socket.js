/*
MIT License

Copyright (c) 2019-2020 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/** @module tcpip */

/**
 * socket mode definition.
 * @property {*} MODE.BINARY binary data stream
 * @property {*} MODE.ASCII ascii data stream
 * @property {*} MODE.CHKS use UDP checksum (default)
 * @property {*} MODE.NOCHK no UDP checksum
 * @property {*} MODE.NAGLE Nagle's algorithm
 * @property {*} MODE.NONAGLE don't use Nagle's algorithm
 * 
 * @property {*} TYPE.TCP_CLIENT tcp client socket
 * @property {*} TYPE.TCP_SERVER tcp server socket
 * @property {*} TYPE.UDP udp socket
 * 
 * @property {*} UDP.ANY receives any packet, outgoing packets are broadcast.
 * @property {*} UDP.FIRST No packets may be sent until a packet has been received. This first fills in the remote IP address and port number.
 * 
 * @property {*} SERVER.ANY TCP server socket bind address for any local ip address.
 */
SOCKET = {
	MODE: {
		BINARY: 0x01,   /* new name */
		ASCII: 0x02,   /* new name */
		CHK: 0x04,   /* defaults to checksum */
		NOCHK: 0x08,
		NAGLE: 0x10,   /* Nagle's algorithm */
		NONAGLE: 0x20
	},
	TYPE: {
		TCP_CLIENT: 0,
		TCP_SERVER: 1,
		UDP: 2,
	},
	UDP: {
		ANY: [-1, -1, -1, -1],	// receive from any address
		FIRST: [0, 0, 0, 0]	// first packet
	},
	SERVER: {
		ANY: [0, 0, 0, 0]	// receive from any address
	}
};

HTTP_PREFIX = "http://";

/**
 * Create a TCP client socket (outgoing connection).
 * 
 * @param {(IpAddress|string)} host IP or host name to connect to
 * @param {number} rport remote port.
 * @param {number} [lport] local port.
 * 
 * @returns {Socket} A TCP client socket.
 */
function Socket(host, rport, lport) {
	return new WattSocket(SOCKET.TYPE.TCP_CLIENT, host, rport, lport);
}

/**
 * Create a UDP socket.
 * 
 * @param {(IpAddress|string)} host IP or host name to receive from or one of SOCKET.UDP.
 * @param {number} rport remote port.
 * @param {number} [lport] local port.
 * 
 * @returns {Socket} An UDP socket.
 */
function UdpSocket(host, rport, lport) {
	return new WattSocket(SOCKET.TYPE.UDP, host, rport, lport);
}

/**
 * create a server socket.
 * 
 * @param {(IpAddress|string)} host hostname/IP-address to bind to. SOCKET.SERVER.ANY to bind to 0.0.0.0
 * @param {number} lport local port to listen for incoming connections.
 * 
 * @returns {Socket} A server socket.
 */
function ServerSocket(host, lport) {
	return new WattSocket(SOCKET.TYPE.TCP_SERVER, host, 0, lport);
}

/**
 * do a HTTP GET request and return the answer as array.
 * 
 * @param {string} url the URL to retrieve, must start with "http://".
 * @param {string[]} head an array containing extra header fields and their values. Only fields defined in that array will be send with the request, no extra headers will be added.
 * 
 * @returns {HTTPResult} an array containing the HTTP response code, message, header (as string) and the response body (as an array of integer, one int per byte).
 */
function http_get(url, head) {
	if (!url.startsWith(HTTP_PREFIX)) {
		throw "Protocol not supported";
	}

	// remove protocol
	url = url.substring(HTTP_PREFIX.length);

	// split host and port from path
	var firstSlash = url.indexOf('/');
	if (firstSlash == -1) {
		var hostPort = url;
		var path = '/';
	} else {
		var hostPort = url.substring(0, firstSlash);
		var path = url.substring(firstSlash)
	}

	// extract port
	var colon = hostPort.indexOf(':');
	if (colon == -1) {
		var host = hostPort;
		var port = 80;
	} else {
		var host = hostPort.substring(0, colon);
		var port = hostPort.substring(colon + 1);
	}

	Debug("host=" + host);
	Debug("port=" + port);
	Debug("path=" + path);

	// build request
	if (!head) {
		head = [
			"Host: " + host,
			"User-Agent: DOjS/" + DOJS_VERSION
		];
	}

	var request =
		"GET " + encodeURI(path) + " HTTP/1.0\r\n";

	for (var i = 0; i < head.length; i++) {
		request += head[i] + "\r\n";
	}
	request += "\r\n";

	// open socket
	var con = Socket(host, port);
	while (!con.Established()) {
		;
	}

	// send request
	Debug(">>> " + request);
	con.FlushNext();
	con.WriteString(request);

	// read response header
	var header = "";
	var wantedSize = -1;
	var firstLine = null;
	while (con.Established() || con.DataReady()) {
		var ready = con.DataReady();
		var line = con.ReadLine();
		Debug("<<< [" + ready + "] '" + line + "'");
		if (!firstLine) {
			firstLine = line
		}
		if (line) {
			header += line + "\n";
			if (line.startsWith("Content-Length: ")) {
				wantedSize = line.substring("Content-Length: ".length);
				Debug("!!! length='" + wantedSize + "'");
			}
		} else if (ready && !line) {
			break;
		}
	}

	// read data
	var data = [];
	while ((con.Established() || con.DataReady()) && ((wantedSize == -1) || (data.length < wantedSize))) {
		data.push(con.ReadByte());
	}

	// parse response
	var code = -1;
	var message = "No response";
	if (firstLine) {
		var parts = firstLine.split(" ");
		if (parts.length == 3) {
			var code = parts[1];
			var message = parts.slice(2).join(" ");
		} else {
			var code = -2;
			var message = "Invalid response";
		}
	}

	// close
	con.Close();

	return [code, message, header, data];
}

/**
 * convert the data array returned by http_get() into a string.
 * 
 * @param {HTTPResult} res The array returned from http_get()
 * 
 * @returns {string} a string or null if res was invalid.
 */
function http_string_content(res) {
	if (res.length >= 4) {
		var str = '';
		for (var i = 0; i < res[3].length; i++) {
			str += String.fromCharCode(res[3][i]);
		}
		return str;
	} else {
		return null;
	}
}

/**
 * check if a HTTP request succeeded.
 * 
 * @param {HTTPResult} res The array returned from http_get().
 * 
 * @returns {bool} TRUE is the request returned code 200.
 */
function http_ok(res) {
	return res && (res.length == 4) && (res[0] == 200);
}

/**
 * extract all response headers.
 * 
 * @param {HTTPResult} res The array returned from http_get().
 * 
 * @returns {Object} a dictionary with header->value mapping or null if res was invalid.
 */
function http_headers(res) {
	if (res.length >= 4) {
		var ret = {}

		var lines = res[2].split('\n');
		for (var i = 1; i < lines.length; i++) {
			var l = lines[i];
			if (l.length > 0) {
				var colon = l.indexOf(":");
				if (colon == -1) {
					throw "Illegal header line found: " + l;
				}
				var key = l.substring(0, colon);
				var val = l.substring(colon + 2);
				ret[key] = val;
			}
		}
		return ret;
	} else {
		return null;
	}
}
