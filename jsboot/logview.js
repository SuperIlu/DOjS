/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

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

LoadLibrary("ipx");

var nodes = [];
var lines = [""];
var max_lines;

function Setup() {
	SetFramerate(30);
	IpxSocketOpen(IPX.DEBUG_SOCKET);
	MouseShowCursor(false);

	max_lines = (SizeY() / 10) - 1;
}

function Loop() {
	ClearScreen(EGA.BLACK);
	if (nodes.length > 0) {
		TextXY(SizeX() - 120, 5, 'CONNECTED', EGA.CYAN);
	} else {
		TextXY(SizeX() - 120, 5, 'NOT CONNECTED', EGA.RED);
	}


	for (var l = 0; l < lines.length; l++) {
		TextXY(2, 10 * (l + 1), lines[l], EGA.LIGHT_GREEN);
	}

	var success = IpxFindNodes(2, nodes);
	while (success && IpxCheckPacket()) {
		var pck = IpxGetPacket();
		if (pck.data.lastIndexOf("_$HELLO=", 0) === 0) {
			// ignore PING packets
			return;
		}
		var parts = pck.data.split("\n");

		// append first part to last line
		lines[lines.length - 1] = lines[lines.length - 1] + parts[0];

		// append other parts as new lines
		for (var i = 1; i < parts.length; i++) {
			lines.push(parts[i]);
		}

		// truncate to max lines
		while (lines.length > max_lines) {
			lines.shift();
		}
	}
}

function Input(e) {
}
