/**
 * This is inspired by https://tixy.land/ done by Martin Kleppe (@aemkei).
 * Implementation is my own.
 * 
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

TIXY_SIZE = 16;		// number of dots in x and y direction
TIXY_DOT = 12;		// max radius of each dot
TIXY_SPACING = 2;	// spacing between dots
TXT_SIZE = 9;		// text height
TXT_MAX = 85;		// max length of function

var xOffset = (Width / 2) - (((TIXY_SIZE * TIXY_DOT * 2) + (TIXY_SIZE * TIXY_SPACING)) / 2);	// x offset for the dots
var yOffset = 2 * TIXY_DOT;																		// y offset for the dots
var lineY = ((TIXY_SIZE + 1) * TIXY_DOT * 2) + (TIXY_SIZE * TIXY_SPACING);						// y pos of the line
var textStart = lineY + TIXY_SIZE;																// y pos of the text/code

var currentFunction = "tan(t+x/10+y/10)";														// current function with a start

var frame = 0;																					// frame nuber

/**
 * init
 */
function Setup() {
	// copy all Math functions/constants to global context
	var mathmembers = [
		"E", "LN10", "LN2", "LOG2E", "LOG10E", "PI", "SQRT1_2", "SQRT2", "abs", "acos",
		"asin", "atan", "atan2", "ceil", "cos", "exp", "floor", "log", "max", "min",
		"pow", "random", "round", "sin", "sqrt", "tan"
	];

	for (var i = 0; i < mathmembers.length; i++) {
		var key = mathmembers[i];
		global[key] = Math[key];
	}

	// hide cursor
	MouseShowCursor(false);
}

/**
 * draw loop
 */
function Loop() {
	// clear screen
	ClearScreen(EGA.BLACK);

	// build a valid JS function with export and evaluate it
	try {
		var content = "exports.f = function(t,i,x,y){return " + currentFunction + ";};"; // build JS
		var exports = { "f": null };						// build object to import into
		NamedFunction('exports', content, "f")(exports);	// evaluate JS
		exports.f(0, 0, 0, 0);								// do a testcall to check the function is valid
		f = exports.f;										// set as current function
	} catch (e) {
		f = null;
	}

	// if we have a vallid function, draw dots
	if (f) {
		var i = 0;
		var t = MsecTime() / 1000;
		for (var y = 0; y < TIXY_SIZE; y++) {
			for (var x = 0; x < TIXY_SIZE; x++) {
				// get valud
				var s = f(t, i, x, y);

				// get color and invert if negative
				if (s < 0) {
					var c = EGA.RED;
					s *= -1;
				} else {
					var c = EGA.WHITE;
				}
				if (s > 1) {
					s = 1;
				}

				// calculate pos and draw dot
				var xPos = (x * TIXY_DOT * 2) + (x * TIXY_SPACING);
				var yPos = (y * TIXY_DOT * 2) + (y * TIXY_SPACING);
				var radius = TIXY_DOT * s;
				FilledCircle(xOffset + xPos, yOffset + yPos, radius, c);

				i++; // increment i
			}
		}
	}

	// draw a line, the function prompt and the current function
	Line(0, lineY, Width, lineY, EGA.GREEN);
	TextXY(TIXY_SIZE, textStart + 0 * TXT_SIZE, "(t,i,x,y) => ", EGA.WHITE, NO_COLOR);
	TextXY(TIXY_SIZE, textStart + 1 * TXT_SIZE, currentFunction, EGA.WHITE, NO_COLOR);

	// draw blinking cursor
	if (Math.ceil(frame / 10) % 2) {
		var cursorString = "";
		for (var j = 0; j < currentFunction.length; j++) {
			cursorString += " ";
		}
		cursorString += "_";
		TextXY(TIXY_SIZE, textStart + 1 * TXT_SIZE, cursorString, EGA.YELLOW, NO_COLOR);
	}
	frame++;
}

/**
 * handle input
 * @param {*} e DOjS event object.
 */
function Input(e) {
	if (e.key != -1) {
		var key = e.key & 0xFF;
		var keyCode = e.key;
		if (keyCode >> 8 == KEY.Code.KEY_BACKSPACE) {
			// delete last character
			currentFunction = currentFunction.slice(0, currentFunction.length - 1);
		} else {
			if (key >= CharCode(" ") && (currentFunction.length < TXT_MAX)) {
				// add character if not max length and charcode at least a SPACE
				currentFunction += String.fromCharCode(key);
			}
		}
	}
}
