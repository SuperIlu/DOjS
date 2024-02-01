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
--------
/**
Additional code taken from MDN:
https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String
https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/hypot

Code samples added on or after August 20, 2010 are in the public domain (CC0). 
No licensing notice is necessary, but if you need one, you can use: 
"Any copyright is dedicated to the Public Domain. http://creativecommons.org/publicdomain/zero/1.0/".
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
 * @property {string} ZIP_DELIM delimiter between ZIP filename and entry name.
 */
ZIP_DELIM = "=";

/**
* @property {string} JSBOOT_DIR path of the JSBOOT directory
*/
JSBOOT_DIR = "JSBOOT/";

/**
* @property {string} PACKAGE_DIR path of the JSBOOT packages directory
*/
PACKAGE_DIR = "PACKAGE/";

/**
 * @property {number} RAW_HDD_FLAG index for HDDs when using raw disk functions.
 */
RAW_HDD_FLAG = 0x80;

/**
 * @property {number} RAW_BLOCKSIZE size of sectors when reading/writing raw disks.
 */
RAW_BLOCKSIZE = 512;

/**
 * @property {number} Width screen width, alternative to calling SizeX()
 */
Width = SizeX();

/**
 * @property {number} Height screen height, alternative to calling SizeY()
 */
Height = SizeY();

/**
 * @property {String} navigator.appName can be used to detect DOjS
 */
navigator = {
	"appName": "DOjS"
};

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
 * enable IPX remote debugging output.
 */
function EnableRemoteDebug() {
	LoadLibrary("ipx");
	REMOTE_DEBUG = true;
	Info("Remote debug logging enabled.");
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
 * Print info message.
 * 
 * @param {string} str the message to print.
 */
function Info(str) {
	Println(">>>", str);
}

/**
 * print an object as JSON to logfile.
 * 
 * @param {*} obj the object to print
 */
function Dump(obj) {
	Println(JSON.stringify(obj));
}

/**
 * try a specific filename which can ba a plain file or a ZIP-file entry. Throws an Exception if the file cant be read.
 * 
 * @param {string} name module name.
 * @param {string} fname the file name to try.
 * 
 * @returns the imported module.
 */
function RequireFile(name, fname) {
	var content;
	try {
		if (fname.indexOf(ZIP_DELIM) != -1) {
			var parts = fname.split(ZIP_DELIM);
			var zname = parts[0];
			var ename = parts[1];
			Debug("Require(zip) " + zname + " -> " + ename);

			content = ReadZIP(zname, ename);
		} else {
			content = Read(fname);
		}
	} catch (e) {
		return null;	// file errors are ignored
	}
	var exports = {};
	Require._cache[name] = exports;
	NamedFunction('exports', content, name)(exports);
	return exports;
}

/**
 * import a module.
 * DOjS modules are CommonJS modules where all exported symbols must be put into an object called 'exports'.
 * A module may provide an optional version using the __VERSION__ member.
 * 
 * @param {string} name module file name.
 * 
 * @returns the imported module.
 * 
 * @example
 * exports.__VERSION__ = 23;        // declare module version
 * exports.myVar = 0;               // will be exported
 * exports.myFunc = function() {};  // will also be exported
 * var localVar;                    // will only be accessible in the module
 * function localFunction() {};     // will also only be accessible in the module
 */
function Require(name) {
	// look in cache
	if (name in Require._cache) {
		Debug("Require(cached) " + name);
		return Require._cache[name];
	}

	var names = [
		name,													// try local dir, plain name
		name + '.js',											// try local dir, name with .js
		JSBOOT_ZIP + ZIP_DELIM + JSBOOT_DIR + name,				// try jsboot.zip, core packages, plain name
		JSBOOT_ZIP + ZIP_DELIM + JSBOOT_DIR + name + '.js',		// try jsboot.zip, core packages, name with .js
		JSBOOT_DIR + name,										// try jsboot directory, core packages, plain name
		JSBOOT_DIR + name + '.js',								// try jsboot directory, core packages, name with .js
		JSBOOT_ZIP + ZIP_DELIM + PACKAGE_DIR + name + '.js'		// try jsboot.zip, installed packages, name with .js
	];
	Debug("Require(names) " + JSON.stringify(names));

	for (var i = 0; i < names.length; i++) {
		var n = names[i];
		Debug("Require() Trying '" + n + "'");

		// try to load. non-existend files lead to the next filename to be tried, parse errors will result in an error thrown
		var parsed = RequireFile(name, n);
		if (!parsed) {
			Debug("RequireFile() " + n + " Not found");
			continue;
		} else {
			return parsed;
		}
	}

	throw new Error('Could not load "' + name + '"');
}
Require._cache = Object.create(null);

/**
 * Alias of Require()
 * 
 * @see Require
 * 
 * @param {string} name module file name.
 * 
 * @returns the imported module.
 */
require = Require;

/**
 * include a module. The exported functions are copied into global scope.
 * @see {@link Require}
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
 * Alias for LoadLibrary().
 * @see LoadLibrary()
 */
function LoadModule(name) {
	LoadLibrary(name);
}

/**
 * add toString() to Error class.
 */
Error.prototype.toString = function () {
	if (this.stackTrace) { return this.name + ': ' + this.message + this.stackTrace; }
	return this.name + ': ' + this.message;
};

/**
 * prefix a filename with the ZIP file the started script came from. The filename is not modified if the script was not loaded from a ZIP file.
 * 
 * @param {string} fname file name.
 * 
 * @returns {string} a ZIP-filename if the running script was loaded from a ZIP file or the passed filename.
 */
function ZipPrefix(fname) {
	if (ARGS[0].indexOf(ZIP_DELIM) != -1) {
		var parts = ARGS[0].split(ZIP_DELIM);
		return parts[0] + ZIP_DELIM + fname;
	} else {
		return fname;
	}
}

/**
 * print startup info with screen details.
 */
function StartupInfo() {
	var mode = GetScreenMode();
	var width = SizeX();
	var height = SizeY();

	Info("Screen size: " + width + "x" + height + "x" + mode);
	Info("Memory: " + JSON.stringify(MemoryInfo()));
	Info("Long file names: " + LFN_SUPPORTED);
	Info("Command line args: " + JSON.stringify(ARGS));
	if (!LINUX) {
		Info("SerialPorts: " + JSON.stringify(GetSerialPorts().map(function (e) { return "0x" + e.toString(16) })));
		Info("ParallelPorts: " + JSON.stringify(GetParallelPorts().map(function (e) { return "0x" + e.toString(16) })));
		Info("FDD: " + GetNumberOfFDD() + ", HDD: " + GetNumberOfHDD());
	}

	if (DEBUG) {
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
 * get random integer between min and max (or between 0 and min if max is not provided).
 * 
 * @param {number} min min
 * @param {number} max max
 * 
 * @returns {number} an integer between min and max.
 */
function RandomInt(min, max) {
	if (max === undefined) {
		max = min;
		min = 0;
	}

	return Math.floor(Math.random() * (max - min) + min);
}

// add startsWith() endsWith() to String class
if (!String.prototype.startsWith) {
	Object.defineProperty(String.prototype, 'startsWith', {
		value: function (search, rawPos) {
			var pos = rawPos > 0 ? rawPos | 0 : 0;
			return this.substring(pos, pos + search.length) === search;
		}
	});
}
if (!String.prototype.endsWith) {
	String.prototype.endsWith = function (search, this_len) {
		if (this_len === undefined || this_len > this.length) {
			this_len = this.length;
		}
		return this.substring(this_len - search.length, this_len) === search;
	};
}

// add hypot to Math
if (!Math.hypot) Math.hypot = function () {
	var max = 0;
	var s = 0;
	var containsInfinity = false;
	for (var i = 0; i < arguments.length; ++i) {
		var arg = Math.abs(Number(arguments[i]));
		if (arg === Infinity)
			containsInfinity = true
		if (arg > max) {
			s *= (max / arg) * (max / arg);
			max = arg;
		}
		s += arg === 0 && max === 0 ? 0 : (arg / max) * (arg / max);
	}
	return containsInfinity ? Infinity : (max === 1 / 0 ? 1 / 0 : max * Math.sqrt(s));
};


/**
 * create stop watch for benchmarking
 */
function StopWatch() {
	this.Reset();
};
/**
 * start stopwatch.
 */
StopWatch.prototype.Start = function () {
	this.start = MsecTime();
	this.stop = null;
};
/**
 * stop stopwatch.
 */
StopWatch.prototype.Stop = function () {
	this.stop = MsecTime();
	if (!this.start) {
		this.Reset();
		throw new Error("StopWatch.Stop() called before StopWatch.Start()!");
	}
};
/**
 * reset stopwatch.
 */
StopWatch.prototype.Reset = function () {
	this.start = null;
	this.stop = null;
};
/**
 * get runtime in ms.
 * @returns {number} runtime in ms.
 */
StopWatch.prototype.ResultMs = function () {
	if (!this.start || !this.stop) {
		throw new Error("start or end time missing!");
	}
	return this.stop - this.start;
};
/**
* convert result to a readable string.
* 
* @param {string} [name] name of the measured value.
*
* @returns {string} a string describing the runtime.
*/
StopWatch.prototype.Result = function (name) {
	var total = this.ResultMs();

	var ret = "Runtime is ";
	if (name) {
		ret = "Runtime for '" + name + "' is ";
	}


	var msecs = Math.floor(total % 1000);
	var secs = Math.floor((total / 1000) % 60);
	var mins = Math.floor((total / 1000 / 60));

	if (mins) {
		ret += mins + "m ";
	}
	if (secs) {
		ret += secs + "s ";
	}
	ret += msecs + "ms";

	return ret;
};
/**
 * print result as a readable string.
 * 
 * @param {string} [name] name of the measured value.
 */
StopWatch.prototype.Print = function (name) {
	Println(this.Result(name));
};

/**
 * get a random integer between [0..max[
 * 
 * @param {number} max max value to return (eclusive).
 */
function GetRandomInt(max) {
	return Math.floor(Math.random() * Math.floor(max));
}


/**
 * Write the given value to io-port 80h to be displayed by a POST card.
 * 
 * @param {number} val value to write to 0x80.
 */
function POST(val) {
	OutPortByte(0x80, val);
}

/*
see:
https://en.wikipedia.org/wiki/Parallel_port
http://electrosofts.com/parallel/
https://web.archive.org/web/20120301022928/http://retired.beyondlogic.org/spp/parallel.pdf
https://stanislavs.org/helppc/bios_data_area.html
*/

if (LINUX) {
	var _lptPorts = [];
} else {
	var _lptPorts = GetParallelPorts();
}

/**
 * read/write data to LPT data register.
 * 
 * @param {number} port port number (0-3).
 * @param {number} data data to write, null to read
 * 
 * @returns {number} current LPT value if data was null.
 * 
 * @see GetParallelPorts
 */
function LPTRawData(port, data) {
	if (port < 0 && ports >= _lptPorts.length) {
		throw new Error('LPT port out of range');
	}
	var addr = _lptPorts[port];

	var old = InPortByte(addr + PARALLEL.CONTROL.ADDR);
	if (data) {
		OutPortByte(addr + PARALLEL.CONTROL.ADDR, old & ~PARALLEL.CONTROL.BIDI);

		OutPortByte(addr + PARALLEL.DATA.ADDR, data);
	} else {
		OutPortByte(addr + PARALLEL.CONTROL.ADDR, old | PARALLEL.CONTROL.BIDI);

		return InPortByte(addr + PARALLEL.DATA.ADDR);
	}
}

/**
 * read status register of LPT port.
 * 
 * @param {number} port port number (0-3).
 * 
 * @see GetParallelPorts
 */
function LPTRawStatus(port) {
	if (port < 0 && ports >= _lptPorts.length) {
		throw new Error('LPT port out of range');
	}
	var addr = _lptPorts[port];

	return InPortByte(addr + PARALLEL.STATUS.ADDR);
}

/**
 * write bits to LPT control register.
 * 
 * @param {number} port port number (0-3).
 * @param {number} bits data to write
 * 
 * @see GetParallelPorts
 */
function LPTRawControl(port, bits) {
	if (port < 0 && ports >= _lptPorts.length) {
		throw new Error('LPT port out of range');
	}
	var addr = _lptPorts[port];

	OutPortByte(addr + PARALLEL.CONTROL.ADDR, bits);
}

/**
 * reset parallel port.
 * @param {number} port port number (0-3).
 * 
 * @see GetParallelPorts
 */
function LPTReset(port) {
	if (port < 0 && ports >= _lptPorts.length) {
		throw new Error('LPT port out of range');
	}
	_LPTReset(port);
}

/**
 * send data to parallel port.
 * @param {number} port port number (0-3).
 * @param {string} data data to transfer.
 * 
 * @see GetParallelPorts
 */
function LPTSend(port, data) {
	if (port < 0 && ports >= _lptPorts.length) {
		throw new Error('LPT port out of range');
	}
	_LPTSend(port, data);
}

/**
 * read parallel port status.
 * @param {number} port port number (0-3).
 * 
 * @see GetParallelPorts
 */
function LPTStatus(port) {
	if (port < 0 && ports >= _lptPorts.length) {
		throw new Error('LPT port out of range');
	}
	return _LPTStatus(port);
}

/**
 * parallel port IO register definitions.
 * 
 *  [*] CONTROL.BIDI must be set to 1 in order to read from DATA. Not supported by all ports!
 * 
 * @property {*} DATA.BIT0 Data 0, pin 2 (out/in*)
 * @property {*} DATA.BIT1 Data 1, pin 3 (out/in*)
 * @property {*} DATA.BIT2 Data 2, pin 4 (out/in*)
 * @property {*} DATA.BIT3 Data 3, pin 5 (out/in*)
 * @property {*} DATA.BIT4 Data 4, pin 6 (out/in*)
 * @property {*} DATA.BIT5 Data 5, pin 7 (out/in*)
 * @property {*} DATA.BIT6 Data 6, pin 8 (out/in*)
 * @property {*} DATA.BIT7 Data 7, pin 9 (out/in*)
 * 
 * @property {*} STATUS.BUSY pin 11, inverted (in)
 * @property {*} STATUS.ACK pin 10 (in)
 * @property {*} STATUS.PAPER_OUT pin 12 (in)
 * @property {*} STATUS.SELECT_IN pin 13 (in)
 * @property {*} STATUS.ERROR pin 15 (in)
 * @property {*} STATUS.TIMEOUT LPTStatus() only
 * 
 * @property {*} CONTROL.BIDI this bit must be set in order to read DATA
 * @property {*} CONTROL.SELECT_OUT pin 17, inverted (out)
 * @property {*} CONTROL.RESET pin 16 (out)
 * @property {*} CONTROL.LINEFEED pin 14, inverted (out)
 * @property {*} CONTROL.STROBE pin 1, inverted (out)
 */
var PARALLEL = {
	DATA: {
		ADDR: 0,
		BIT0: (1 << 0),
		BIT1: (1 << 1),
		BIT2: (1 << 2),
		BIT3: (1 << 3),
		BIT4: (1 << 4),
		BIT5: (1 << 5),
		BIT6: (1 << 6),
		BIT7: (1 << 7)
	},
	STATUS: {
		ADDR: 1,
		BUSY: (1 << 7),
		ACK: (1 << 6),
		PAPER_OUT: (1 << 5),
		SELECT_IN: (1 << 4),
		ERROR: (1 << 3),
		TIMEOUT: (1 << 1)
	},
	CONTROL: {
		ADDR: 2,
		BIDI: (1 << 5),
		SELECT_OUT: (1 << 3),
		RESET: (1 << 2),
		LINEFEED: (1 << 1),
		STROBE: (1 << 0)
	}
};

/**
 * sound input selection.
 * @property {*} Input.MIC use microphone input
 * @property {*} Input.LINE use line input
 * @property {*} Input.CD use CD input
 * @property {*} Bits.BITS8 use 8 bits
 * @property {*} Bits.BITS16 use 16 bits
 */
var SOUND = {
	Input: {
		MIC: 1,
		LINE: 2,
		CD: 3
	},
	Bits: {
		BITS8: 8,
		BITS16: 16
	}
};

/**
 * System() flags for subsystem shutdown/restart.
 * @property {*} MOUSE de/reinit mouse
 * @property {*} SOUND de/reinit sound
 * @property {*} JOYSTICK de/reinit joystick
 * @property {*} KEYBOARD de/reinit keyboard
 * @property {*} TIMER de/reinit timer
 */
var SYSTEM = {
	MOUSE: 0x01,
	SOUND: 0x02,
	JOYSTICK: 0x04,
	KEYBOARD: 0x08,
	TIMER: 0x10
};

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
var MOUSE = {
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
var KEY = {
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
