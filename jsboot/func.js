/*
MIT License

Copyright (c) 2019 Andre Seidelt <superilu@yahoo.com>

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

/** @module other */

/**
 * @property {boolean} DEBUG enable/disable Debug() output.
 */
DEBUG = false;

/**
 * @property {boolean} REMOTE_DEBUG enable/disable Debug() sending via IPX.
 */
REMOTE_DEBUG = false;

/**
 * print javascript debug output if DEBUG is true.
 * 
 * @param {string} str the message to print.
 */
function Debug(str) {
	_Debug(str);
	if (REMOTE_DEBUG) {
		IpxDebug(str + "\n");
	}
}

/**
 * Internal debug which does not redirect to IPX if enabled.
 * 
 * @param {string} str the message to print.
 */
function _Debug(str) {
	if (DEBUG) {
		Println("-=> ", str);
	}
}

/**
 * import a module.
 * @param {string} name module file name.
 * @returns the imported module.
 */
function Require(name) {
	// look in cache
	if (name in Require.cache) {
		Debug("Require(cached) " + name);
		return Require.cache[name];
	}

	var names = [name, name + '.JS', 'JSBOOT/' + name, 'JSBOOT/' + name + '.JS'];
	Debug("Require(names) " + JSON.stringify(names));

	for (var i = 0; i < names.length; i++) {
		var n = names[i];
		Debug("Require() Trying '" + n + "'");
		var content;
		try {
			content = Read(n);
		} catch (e) {
			Debug("Require() " + n + " Not found");
			continue;
		}
		var exports = {};
		Require.cache[name] = exports;
		Function('exports', content)(exports);
		return exports;
	};


	throw 'Could not load "' + name + '"';
}
Require.cache = Object.create(null);

/**
 * include a module. The exported functions are copied into global scope.
 * 
 * @param {string} name module file name.
 */
function Include(name) {
	var e = Require(name);
	for (var key in e) {
		Debug("Include(toGlobal) " + key);
		global[key] = e[key];
	}
}

/**
 * add toString() to Error class.
 */
Error.prototype.toString = function () {
	if (this.stackTrace) { return this.name + ': ' + this.message + this.stackTrace; }
	return this.name + ': ' + this.message;
};

/**
 * print startup info with screen details.
 */
function StartupInfo() {
	var mode = GetScreenMode();
	var width = SizeX();
	var height = SizeY();

	if (DEBUG) {
		_Debug("Screen size=" + width + "x" + height + "x" + mode);
		_Debug("Memory=" + JSON.stringify(MemoryInfo()));

		var funcs = [];
		var other = [];
		for (var e in global) {
			if (typeof global[e] === "function") {
				funcs.push(e + "()");
			} else {
				other.push(e);
			}
		}
		funcs.sort();
		other.sort();
		_Debug("Known globals:");
		for (var i = 0; i < other.length; i++) {
			_Debug("    " + other[i]);
		}
		_Debug("Known functions:");
		for (var i = 0; i < funcs.length; i++) {
			_Debug("    " + funcs[i]);
		}
	}
}
StartupInfo();

/**
 * get char code.
 * 
 * @param {string} s a string
 * @returns the ASCII-code of the first character.
 */
function CharCode(s) {
	return s.charCodeAt(0);
}

/**
 * compare a keycode with a character.
 * 
 * @param {number} k keycode from an Event
 * @param {string} s a string with one char
 */
function CompareKey(k, s) {
	return (k & 0xFF) == CharCode(s);
}

/**
 * event interface.
 * @property {*} Mode.NONE no cursor
 * @property {*} Mode.ARROW arrow cursor
 * @property {*} Mode.BUSY busy cursor
 * @property {*} Mode.QUESTION questionmark cursor
 * @property {*} Mode.CURSOR_EDIT edit cursor
 * @property {*} Buttons mouse button definitions
 * @property {*} Buttons.LEFT
 * @property {*} Buttons.RIGHT
 * @property {*} Buttons.MIDDLE
 */
MOUSE = {
	Mode: {
		NONE: 0,
		ARROW: 2,
		BUSY: 3,
		QUESTION: 4,
		CURSOR_EDIT: 5
	},
	Buttons: {
		LEFT: 0x01,
		RIGHT: 0x02,
		MIDDLE: 0x04
	}
};

/**
 * keyboard input.
 * @property {*} Code key definitions.
 */
KEY = {
	Code: {
		NoKey: -1,		/* no key available */
		/* Letters and numbers are missing from the definitions but can be obtained by e.g. CharCode('A'). */

		KEY_A: 1,
		KEY_B: 2,
		KEY_C: 3,
		KEY_D: 4,
		KEY_E: 5,
		KEY_F: 6,
		KEY_G: 7,
		KEY_H: 8,
		KEY_I: 9,
		KEY_J: 10,
		KEY_K: 11,
		KEY_L: 12,
		KEY_M: 13,
		KEY_N: 14,
		KEY_O: 15,
		KEY_P: 16,
		KEY_Q: 17,
		KEY_R: 18,
		KEY_S: 19,
		KEY_T: 20,
		KEY_U: 21,
		KEY_V: 22,
		KEY_W: 23,
		KEY_X: 24,
		KEY_Y: 25,
		KEY_Z: 26,
		KEY_0: 27,
		KEY_1: 28,
		KEY_2: 29,
		KEY_3: 30,
		KEY_4: 31,
		KEY_5: 32,
		KEY_6: 33,
		KEY_7: 34,
		KEY_8: 35,
		KEY_9: 36,
		KEY_0_PAD: 37,
		KEY_1_PAD: 38,
		KEY_2_PAD: 39,
		KEY_3_PAD: 40,
		KEY_4_PAD: 41,
		KEY_5_PAD: 42,
		KEY_6_PAD: 43,
		KEY_7_PAD: 44,
		KEY_8_PAD: 45,
		KEY_9_PAD: 46,
		KEY_F1: 47,
		KEY_F2: 48,
		KEY_F3: 49,
		KEY_F4: 50,
		KEY_F5: 51,
		KEY_F6: 52,
		KEY_F7: 53,
		KEY_F8: 54,
		KEY_F9: 55,
		KEY_F10: 56,
		KEY_F11: 57,
		KEY_F12: 58,
		KEY_ESC: 59,
		KEY_TILDE: 60,
		KEY_MINUS: 61,
		KEY_EQUALS: 62,
		KEY_BACKSPACE: 63,
		KEY_TAB: 64,
		KEY_OPENBRACE: 65,
		KEY_CLOSEBRACE: 66,
		KEY_ENTER: 67,
		KEY_COLON: 68,
		KEY_QUOTE: 69,
		KEY_BACKSLASH: 70,
		KEY_BACKSLASH2: 71,
		KEY_COMMA: 72,
		KEY_STOP: 73,
		KEY_SLASH: 74,
		KEY_SPACE: 75,
		KEY_INSERT: 76,
		KEY_DEL: 77,
		KEY_HOME: 78,
		KEY_END: 79,
		KEY_PGUP: 80,
		KEY_PGDN: 81,
		KEY_LEFT: 82,
		KEY_RIGHT: 83,
		KEY_UP: 84,
		KEY_DOWN: 85,
		KEY_SLASH_PAD: 86,
		KEY_ASTERISK: 87,
		KEY_MINUS_PAD: 88,
		KEY_PLUS_PAD: 89,
		KEY_DEL_PAD: 90,
		KEY_ENTER_PAD: 91,
		KEY_PRTSCR: 92,
		KEY_PAUSE: 93,
		KEY_ABNT_C1: 94,
		KEY_YEN: 95,
		KEY_KANA: 96,
		KEY_CONVERT: 97,
		KEY_NOCONVERT: 98,
		KEY_AT: 99,
		KEY_CIRCUMFLEX: 100,
		KEY_COLON2: 101,
		KEY_KANJI: 102,
		KEY_EQUALS_PAD: 103,  /* MacOS X */
		KEY_BACKQUOTE: 104,  /* MacOS X */
		KEY_SEMICOLON: 105,  /* MacOS X */
		KEY_COMMAND: 106,  /* MacOS X */
		KEY_UNKNOWN1: 107,
		KEY_UNKNOWN2: 108,
		KEY_UNKNOWN3: 109,
		KEY_UNKNOWN4: 110,
		KEY_UNKNOWN5: 111,
		KEY_UNKNOWN6: 112,
		KEY_UNKNOWN7: 113,
		KEY_UNKNOWN8: 114,

		KEY_MODIFIERS: 115,

		KEY_LSHIFT: 115,
		KEY_RSHIFT: 116,
		KEY_LCONTROL: 117,
		KEY_RCONTROL: 118,
		KEY_ALT: 119,
		KEY_ALTGR: 120,
		KEY_LWIN: 121,
		KEY_RWIN: 122,
		KEY_MENU: 123,
		KEY_SCRLOCK: 124,
		KEY_NUMLOCK: 125,
		KEY_CAPSLOCK: 126
	}
};
