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

CMD_LENGTH = 75;	// max length of command line
TXT_SIZE = 10;		// line height

/**
 * create a on-screen text console.
 * @class
 * @param {Color} [fg] default text color
 * @param {Color} [bg] background color.
 * @param {boolean} [cmd] command callback function.
 * @param {Color} [cmdCol] color for command line.
 */
function Console(fg, bg, cmd, cmdCol) {
	this.fg = fg || EGA.GREEN;
	this.bg = bg || EGA.BLACK;
	this.cmd = cmd || null;
	this.cmdCol = cmdCol || EGA.YELLOW;
	this.lines = [[EGA.GREEN, ""]];	// on screen logfile
	this.max_lines = (SizeY() / 10) - 1;
	this.currentCmd = "";
	this.frame = 0;

	if (this.cmd) {
		this.max_lines--;
	}
};

/**
 * set the current command buffer.
 * 
 * @param {String} txt the command line. use "" to clear current command.
 */
Console.prototype.SetInput = function (txt) {
	this.currentCmd = txt;
};

/**
 * draw the console to the screen.
 * 
 * @param {boolean} [clear] true to clear the screen before drawing the text.
 */
Console.prototype.Draw = function (clear) {
	if (typeof clear === 'undefined') {
		clear = true;
	}
	if (clear) {
		ClearScreen(this.bg);
	}

	// draw text on screen
	for (var l = 0; l < this.lines.length; l++) {
		var col = this.lines[l][0];
		var txt = this.lines[l][1];
		TextXY(2, TXT_SIZE * (l + 1), txt, col);
	}

	if (this.cmd) {
		l++;
		var prompt = ">";
		if (this.currentCmd.length > CMD_LENGTH) {
			prompt = "<";
		}
		var displayTxt = prompt + this.currentCmd.slice(-CMD_LENGTH);
		TextXY(2, TXT_SIZE * (l + 1), displayTxt, this.cmdCol);

		// draw blinking cursor
		if (Math.ceil(this.frame / 10) % 2) {
			var cursorString = "";
			for (var j = 0; j < displayTxt.length; j++) {
				cursorString += " ";
			}
			cursorString += "_";
			TextXY(2, TXT_SIZE * (l + 1), cursorString, this.cmdCol, NO_COLOR);
		}
		this.frame++;
	}
};

/**
 * get the contents of the command buffer.
 * 
 * @returns {string} the current command line content.
 */
Console.prototype.GetInput = function () {
	return this.currentCmd;
};

/**
 * handle user input. Usually called in Input().
 * 
 * @param {Event} e the input event as provided by Input().
 * @see Input()
 */
Console.prototype.HandleInput = function (e) {
	if (e.key != -1) {
		var key = e.key & 0xFF;
		var keyCode = e.key;
		if ((keyCode >> 8 == KEY.Code.KEY_ENTER) || (keyCode >> 8 == KEY.Code.KEY_ENTER_PAD)) {
			this.cmd(this);
		} else if (keyCode >> 8 == KEY.Code.KEY_BACKSPACE) {
			// delete last character
			this.currentCmd = this.currentCmd.slice(0, this.currentCmd.length - 1);
		} else {
			if ((key >= CharCode(" ")) && key <= CharCode("~")) {
				// add character if charcode is valid
				this.currentCmd += String.fromCharCode(key);
			}
		}
	}
};

/**
 * add a logmessage to the console.
 * 
 * @param {string} txt the logmessage.
 * @param {Color} [col] the color for this logmessage
 */
Console.prototype.Log = function (txt, col) {
	col = col || this.fg;

	this.lines.push([col, txt]);

	// truncate to max lines
	while (this.lines.length > this.max_lines) {
		this.lines.shift();
	}
};

// export functions and version
exports.__VERSION__ = 2;
exports.Console = Console;
