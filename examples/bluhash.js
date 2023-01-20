/*
MIT License

Copyright (c) 2019-2022 Andre Seidelt <superilu@yahoo.com>

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
LoadLibrary("png");

var TXT_MAX = 60; // max text length that can be entered

var blurtxt = "";	// current input
var img = null;		// current image
var paddedtxt = "";	// valid hash created from input
var punch = 1;		// current punch
var img_num = 0;	// image save counter

/**
 * init
 */
function Setup() {
	img = null;
	MouseShowCursor(false);
}

/**
 * redraw
 */
function Loop() {
	ClearScreen(32);
	if (img) {
		img.Draw(160, 120);
		TextXY(0, 10, "Padded length = " + paddedtxt.length, EGA.RED, EGA.BLACK);
		TextXY(0, 20, paddedtxt + " " + punch, EGA.RED, EGA.BLACK);
	}
	TextXY(0, 0, "> " + blurtxt, EGA.RED, EGA.BLACK);
	if (blurtxt.length > 0) {
		paddedtxt = paddBlurhash(blurtxt);
		img = new Bitmap(paddedtxt, 320, 240, punch);
	}
}

/**
 * handle input
 * @param {Event} e input event
 */
function Input(e) {
	if (e.key != -1) {
		var key = e.key & 0xFF;
		var keyCode = e.key >> 8;
		switch (keyCode) {
			case KEY.Code.KEY_F1:
				// save "large" image of current hash
				var big = new Bitmap(paddedtxt, 640, 480, punch);
				big.SavePngImage(img_num + ".PNG");
				var f = new File(img_num + ".TXT", FILE.WRITE);
				f.WriteLine(JSON.stringify({
					"input": blurtxt,
					"blurhash": paddedtxt,
					"punch": punch
				}));
				f.Close();
				img_num++;
				break;
			case KEY.Code.KEY_F2:
				// randomly generate a valid hash
				blurtxt = randomHash();
				img = "";
				break;
			case KEY.Code.KEY_UP:
				// increase punch
				punch++;
				img = null;
				break;
			case KEY.Code.KEY_DOWN:
				// decrease punch
				if (punch > 1) {
					punch--;
					img = null;
				}
				break;
			case KEY.Code.KEY_DEL:
				// clear current hash
				blurtxt = "";
				img = null;
				break;
			case KEY.Code.KEY_BACKSPACE:
				// delete last character
				blurtxt = blurtxt.slice(0, blurtxt.length - 1);
				img = null;
				break;
			default:
				if (key >= CharCode(" ") && (blurtxt.length < TXT_MAX)) {
					// add character if not max length and charcode at least a SPACE
					blurtxt += String.fromCharCode(key);
					img = null;
				}
				break;
		}
	}
}

/**
 * generate a random integer
 * @param {number} max max number (excluding)
 * @returns int
 */
function getRandomInt(max) {
	return Math.floor(Math.random() * max);
}

/**
 * random hash
 * @returns a random but valid hash
 */
function randomHash() {
	var blurhash = digitCharacters[getRandomInt(digitCharacters.length)];

	var sizeFlag = decode83(blurhash[0]);
	var numY = Math.floor(sizeFlag / 9) + 1;
	var numX = (sizeFlag % 9) + 1;
	var wanted = 4 + 2 * numX * numY;

	while (blurhash.length < wanted) {
		blurhash += digitCharacters[getRandomInt(digitCharacters.length)];
	}

	return blurhash;
}

var digitCharacters = [
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F",
	"G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V",
	"W", "X", "Y", "Z", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l",
	"m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "#", "$",
	"%", "*", "+", ",", "-", ".", ":", ";", "=", "?", "@", "[", "]", "^", "_", "{",
	"|", "}", "~"
];

function decode83(str) {
	var value = 0;
	for (var i = 0; i < str.length; i++) {
		var c = str[i];
		var digit = digitCharacters.indexOf(c);
		value = value * 83 + digit;
	}
	return value;
}

function paddBlurhash(blurhash) {
	var pos = 0;
	while (blurhash.length < 6) {
		blurhash += blurhash[pos++];
	}

	var sizeFlag = decode83(blurhash[0]);
	var numY = Math.floor(sizeFlag / 9) + 1;
	var numX = (sizeFlag % 9) + 1;
	var wanted = 4 + 2 * numX * numY;

	pos = 0;
	while (blurhash.length < wanted) {
		blurhash += blurhash[pos++];
	}

	if (blurhash.length > wanted) {
		blurhash = blurhash.substring(0, wanted);
	}

	return blurhash;
}