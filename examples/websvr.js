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
var svr = null;		// server socket
var lines = [""];	// on screen logfile
var max_lines;		// maximum number that fit on screen

/*
** This function is called once when the script is started.
*/
function Setup() {
	SetFramerate(30);
	MouseShowCursor(false);

	max_lines = (SizeY() / 10) - 1;
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	web_server();
	draw_log();
}

/**
 * This function is called on an input event.
 * @param {*} e an input event.
 */
function Input(e) {
}

/**
 * print an ERROR to the on screen logfile.
 * @param {*} s error text
 */
function print_err(s) {
	lines.push("E " + s);
}

/**
 * print a message to the on screen logfile.
 * @param {*} s message text
 */
function print_msg(s) {
	lines.push("I " + s);
}

/**
 * draw the contents of the 'lines' array on screen.
 */
function draw_log() {
	ClearScreen(EGA.BLACK);

	// draw text on screen
	for (var l = 0; l < lines.length; l++) {
		var txt = lines[l];
		if (txt.startsWith("E ")) {
			TextXY(2, 10 * (l + 1), txt, EGA.RED);
		} else {
			TextXY(2, 10 * (l + 1), txt, EGA.LIGHT_GREEN);
		}
	}

	// truncate to max lines
	while (lines.length > max_lines) {
		lines.shift();
	}
}

/**
 * handle a single HTTP request.
 * 
 * @param {Socket} con the socket connection.
 */
function handle_request(con) {
	// accumulate lines until empty line
	var req = [];
	while (con.Established() || con.DataReady()) {
		var ready = con.DataReady();
		var line = con.ReadLine();
		Debug("<<< [" + ready + "] '" + line + "'");
		if (line) {
			req.push(line);
		} else if (ready && !line) {
			break;
		}
	}

	if (req.length > 0) {
		if (req[0].startsWith("GET ")) {
			if (req[0].endsWith(" HTTP/1.0") || req[0].endsWith(" HTTP/1.1")) {
				var sIdx = req[0].indexOf(' ');
				var p1 = req[0].substring(sIdx + 1);
				var eIdx = p1.indexOf(' ');
				var uri = p1.substring(0, eIdx);

				if (uri.startsWith("/api/")) {
					call_function(con, uri);
				} else {
					var path = extract_path(uri);
					print_msg("<- '" + path + "'");
					send_file(con, path);
				}
			} else {
				send_response(con, 505, "HTTP Version Not Supported");
			}
		} else {
			send_response(con, 405, "Method Not Allowed");
		}
	} else {
		send_response(con, 400, "request empty");
	}
}

/**
 * send a HTTP response with given code and message.
 * 
 * @param {Socket} con the connection to use.
 * @param {number} code HTTP response code.
 * @param {string} msg server message.
 */
function send_response(con, code, msg) {
	print_msg("-> " + code + " " + msg);
	var resp = "HTTP/1.0 " + code + " " + msg + "\n" +
		"Content-Type: text/plain\n" +
		"Server: DOjS/" + DOJS_VERSION + "\n" +
		"Connection: close\n" +
		"Content-Length: " + msg.length + "\n\n" +
		msg;
	con.WriteString(resp);
}

/**
 * extract the path part from an URI.
 * 
 * @param {string} uri the URI.
 * 
 * @returns {string} the full path if given or null if this did not start with a '/'
 */
function extract_path(uri) {
	if (uri.startsWith("/")) {
		var idx = uri.indexOf("?");
		if (idx == -1) {
			var path = uri.substring(1);
		} else {
			var path = uri.substring(1, idx);
		}
		if (path.length > 0) {
			if (path.endsWith("/")) {
				return path + "INDEX.HTM";
			} else {
				return path;
			}
		} else {
			return "INDEX.HTM";
		}
	} else {
		return null;
	}
}

/**
 * extract all parameters from URI.
 * 
 * @param {string} uri the URI.
 * 
 * @returns {Object} a dictionary with all key/value pairs.
 */
function extract_parameters(uri) {
	var ret = {
		"__order__": []
	};
	var idx = uri.indexOf("?");
	if (idx != -1) {
		var params = uri.substring(idx + 1).split("&");
		for (var i = 0; i < params.length; i++) {
			var parts = params[i].split("=");
			if (parts.length == 2) {
				ret["__order__"].push(parts[0]);
				ret[parts[0]] = parts[1];
			}
		}
	}
	return ret;
}

/**
 * try to figure out a content type from the file extension of a path.
 * 
 * @param {string} f the path.
 * 
 * @returns {string} a valid 
 */
function content_type(f) {
	var f = f.toLowerCase();
	if (f.endsWith(".htm")) {
		return "text/html";
	} else if (f.endsWith(".txt")) {
		return "text/plain";
	} else if (f.endsWith(".png")) {
		return "image/png";
	} else if (f.endsWith(".jpg")) {
		return "image/jpeg";
	} else {
		return "application/octet-stream";
	}
}

/**
 * send the contents of a file as HTTP response.
 * 
 * @param {Socket} con the connection to use.
 * @param {string} f file path.
 */
function send_file(con, f) {
	try {
		var mime_type = content_type(f);
		var file = new File(f, FILE.READ);
		var size = file.GetSize();

		// send header
		var resp = "HTTP/1.0 200 OK\n" +
			"Content-Type: " + mime_type + "\n" +
			"Server: DOjS/" + DOJS_VERSION + "\n" +
			"Connection: close\n" +
			"Content-Length: " + size + "\n\n";
		con.WriteString(resp);

		print_msg("size=" + size);

		// send file content
		var data = file.ReadBytes();
		con.WriteBytes(data);
		con.Flush();
		file.Close();
	} catch (err) {
		send_response(con, 404, "" + err);
		Println(err);
	}
}

/**
 * call a Javascript function with the supplied GET parameters and send the return value as JSON.
 * 
 * @param {Socket} con the connection to use.
 * @param {string} uri requets uri.
 */
function call_function(con, uri) {
	var path = extract_path(uri).replace("/", "_");
	var parameters = extract_parameters(uri);

	var fun = global[path];
	if (fun) {
		var params = [];
		for (var i = 0; i < parameters["__order__"].length; i++) {
			params.push(parameters[parameters["__order__"][i]]);
		}
		print_msg("Calling " + path + "(" + params + ")");
		try {
			var res = fun.apply(null, params);
			var res_str = JSON.stringify(res);

			// send header
			var resp = "HTTP/1.0 200 OK\n" +
				"Content-Type: application/json\n" +
				"Server: DOjS/" + DOJS_VERSION + "\n" +
				"Connection: close\n" +
				"Content-Length: " + res_str.length + "\n\n";
			con.WriteString(resp);
			con.WriteString(res_str);
			con.Flush();
		} catch (err) {
			send_response(con, 500, "" + err);
			Println(err);
		}
	} else {
		send_response(con, 404, "unknown function " + path + "()");
	}
}

/**
 * example function for the API call of the webserver.
 * 
 * @param {number} a a number
 * @param {number} b another number
 * 
 * @returns {number} a+b
 */
function api_add(a, b) {
	return parseInt(a) + parseInt(b);
}

/**
 * server state machine. creates ServerSocket, polls for connection and sends an answer.
 */
function web_server() {
	if (svr) {
		try {
			if (svr.Established()) {
				Println("Connection from " + JSON.stringify(svr.GetRemoteHost()) + ":" + svr.GetRemotePort());
				handle_request(svr);
				svr.Flush();
				svr.WaitFlush();
				svr.Close();
				svr = null;
			}
		} catch (err) {
			print_err(err);
			Println(err);
			try {
				svr.Close();
			} catch (err2) {
				print_err(err2);
				Println(err2);
			}
			svr = null;
		}
	} else {
		svr = ServerSocket(SOCKET.SERVER.ANY, 80);
		print_msg("Listening on " + JSON.stringify(GetLocalIpAddress()) + ", port 80");
	}
}
