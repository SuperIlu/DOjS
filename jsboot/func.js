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


/**
 * @property {boolean} DEBUG enable/disable Debug() output.
 */
DEBUG = false;

/** @module other */

/**
 * print javascript debug output if DEBUG is true.
 * @param {string} str the message to print.
 */
function Debug(str) {
	if (DEBUG) {
		var str = "";
		for (var i = 0; i < arguments.length; i++) {
			str += arguments[i] + " ";
		};
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
		Debug("Require(cached)", name);
		return Require.cache[name];
	}

	var names = [name, name + '.JS', 'JSBOOT/' + name, 'JSBOOT/' + name + '.JS'];
	Debug("Require(names)", JSON.stringify(names));

	for (var i = 0; i < names.length; i++) {
		var n = names[i];
		Debug("Require()", 'Trying "' + n + '"');
		var content;
		try {
			content = Read(n);
		} catch (e) {
			Debug("Require()", n + " Not found");
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
		Debug("Include(toGlobal)", key);
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
	var adapter = GetScreenAdapter();
	var width = SizeX();
	var height = SizeY();
	var colors = NumColors();

	Debug("Screen size=" + width + "x" + height + " with " + colors + " colors, mode=" + mode + ", adapter=" + adapter);
	Debug("Memory=" + JSON.stringify(MemoryInfo()));
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
 * arc style definition
 * @namespace ARC
 * @property {*} OPEN Open arc.
 * @property {*} CLOSE1 closes the arc with a line between his start and end point.
 * @property {*} CLOSE2 draws the typical cake slice.
 */
ARC = {
	OPEN: 0,
	CLOSE1: 1,
	CLOSE2: 2
};

/* @module */

/**
 * event interface.
 * @namespace MOUSE
 * @property {*} Mode.NORMAL just the cursor
 * @property {*} Mode.RUBBER rect. rubber band (XOR-d to the screen)
 * @property {*} Mode.LINE line attached to the cursor
 * @property {*} Mode.BOX rectangular box dragged by the cursor
 * @property {*} Flags event flags
 * @property {*} Buttons mouse button definitions
 * @property {*} Flags.MOTION
 * @property {*} Flags.LEFT_DOWN
 * @property {*} Flags.LEFT_UP
 * @property {*} Flags.RIGHT_DOWN
 * @property {*} Flags.RIGHT_UP
 * @property {*} Flags.MIDDLE_DOWN
 * @property {*} Flags.MIDDLE_UP
 * @property {*} Flags.P4_DOWN
 * @property {*} Flags.P4_UP
 * @property {*} Flags.P5_DOWN
 * @property {*} Flags.P5_UP
 * @property {*} Flags.BUTTON_DOWN
 * @property {*} Flags.BUTTON_UP
 * @property {*} Flags.BUTTON_CHANGE
 * @property {*} Flags.KEYPRESS
 * @property {*} Flags.POLL
 * @property {*} Flags.NOPAINT
 * @property {*} Flags.COMMAND
 * @property {*} Flags.EVENT
 * @property {*} Buttons.LEFT
 * @property {*} Buttons.RIGHT
 * @property {*} Buttons.MIDDLE
 * @property {*} Buttons.P4
 * @property {*} Buttons.WHEEL_UP
 * @property {*} Buttons.P5
 * @property {*} Buttons.WHEEL_DOWN
 */
MOUSE = {
	Mode: {
		NORMAL: 0,
		RUBBER: 1,
		LINE: 2,
		BOX: 3
	},
	Flags: {
		MOTION: 0x001,
		LEFT_DOWN: 0x002,
		LEFT_UP: 0x004,
		RIGHT_DOWN: 0x008,
		RIGHT_UP: 0x010,
		MIDDLE_DOWN: 0x020,
		MIDDLE_UP: 0x040,
		P4_DOWN: 0x400,
		P4_UP: 0x800,
		P5_DOWN: 0x2000,
		P5_UP: 0x4000,
		BUTTON_DOWN: (0x002 | 0x020 | 0x008 | 0x400 | 0x2000),
		BUTTON_UP: (0x004 | 0x040 | 0x010 | 0x800 | 0x4000),
		BUTTON_CHANGE: ((0x004 | 0x040 | 0x010 | 0x800 | 0x4000) | (0x002 | 0x020 | 0x008 | 0x400 | 0x2000)),
		KEYPRESS: 0x080,
		POLL: 0x100,
		NOPAINT: 0x200,
		COMMAND: 0x1000,
		EVENT: (0x001 | 0x080 | ((0x004 | 0x040 | 0x010 | 0x800 | 0x4000) | (0x002 | 0x020 | 0x008 | 0x400 | 0x2000)) | 0x1000)

	},
	Buttons: {
		LEFT: 0x01,
		RIGHT: 0x02,
		MIDDLE: 0x04,
		P4: 0x08,
		WHEEL_UP: 0x08,
		P5: 0x10,
		WHEEL_DOWN: 0x10,
	}
};

/**
 * keyboard input.
 * @namespace KEY
 * @property {*} State kbdstate definitions.
 * @property {*} Code key definitions.
 */
KEY = {
	State: {
		RIGHTSHIFT: 0x01,	/* Keybd states: right shift key depressed */
		LEFTSHIFT: 0x02,		/* left shift key depressed */
		CTRL: 0x04,			/* CTRL depressed */
		ALT: 0x08,			/* ALT depressed */
		SCROLLOCK: 0x10,		/* SCROLL LOCK active */
		NUMLOCK: 0x20,			/* NUM LOCK active */
		CAPSLOCK: 0x40,			/* CAPS LOCK active */
		INSERT: 0x80,			/* INSERT state active */
		SHIFT: (0x02 | 0x01)
	},
	Code: {
		NoKey: 0x0000,		/* no key available */
		/* Letters and numbers are missing from the definitions but can be obtained by e.g. CharCode('A'). */

		OutsideValidRange: 0x0100,	/* key typed but code outside 1..Gr,LastDefinedKeycode */

		/* standard ASCII key codes */
		Control_A: 0x0001,
		Control_B: 0x0002,
		Control_C: 0x0003,
		Control_D: 0x0004,
		Control_E: 0x0005,
		Control_F: 0x0006,
		Control_G: 0x0007,
		Control_H: 0x0008,
		BackSpace: 0x0008,
		Control_I: 0x0009,
		Tab: 0x0009,
		Control_J: 0x000a,
		LineFeed: 0x000a,
		Control_K: 0x000b,
		Control_L: 0x000c,
		Control_M: 0x000d,
		Return: 0x000d,
		Control_N: 0x000e,
		Control_O: 0x000f,
		Control_P: 0x0010,
		Control_Q: 0x0011,
		Control_R: 0x0012,
		Control_S: 0x0013,
		Control_T: 0x0014,
		Control_U: 0x0015,
		Control_V: 0x0016,
		Control_W: 0x0017,
		Control_X: 0x0018,
		Control_Y: 0x0019,
		Control_Z: 0x001a,
		Control_LBracket: 0x001b,
		Escape: 0x001b,
		Control_BackSlash: 0x001c,
		Control_RBracket: 0x001d,
		Control_Caret: 0x001e,
		Control_Underscore: 0x001f,
		Space: 0x0020,
		ExclamationPoint: 0x0021,
		DoubleQuote: 0x0022,
		Hash: 0x0023,
		Dollar: 0x0024,
		Percent: 0x0025,
		Ampersand: 0x0026,
		Quote: 0x0027,
		LParen: 0x0028,
		RParen: 0x0029,
		Star: 0x002a,
		Plus: 0x002b,
		Comma: 0x002c,
		Dash: 0x002d,
		Period: 0x002e,
		Slash: 0x002f,
		Colon: 0x003a,
		SemiColon: 0x003b,
		LAngle: 0x003c,
		Equals: 0x003d,
		RAngle: 0x003e,
		QuestionMark: 0x003f,
		At: 0x0040,
		LBracket: 0x005b,
		BackSlash: 0x005c,
		RBracket: 0x005d,
		Caret: 0x005e,
		UnderScore: 0x005f,
		BackQuote: 0x0060,
		LBrace: 0x007b,
		Pipe: 0x007c,
		RBrace: 0x007d,
		Tilde: 0x007e,
		Control_Backspace: 0x007f,

		/* extended key codes as defined in DJGPP */
		Alt_Escape: 0x0101,
		Control_At: 0x0103,
		Alt_Backspace: 0x010e,
		BackTab: 0x010f,
		Alt_Q: 0x0110,
		Alt_W: 0x0111,
		Alt_E: 0x0112,
		Alt_R: 0x0113,
		Alt_T: 0x0114,
		Alt_Y: 0x0115,
		Alt_U: 0x0116,
		Alt_I: 0x0117,
		Alt_O: 0x0118,
		Alt_P: 0x0119,
		Alt_LBracket: 0x011a,
		Alt_RBracket: 0x011b,
		Alt_Return: 0x011c,
		Alt_A: 0x011e,
		Alt_S: 0x011f,
		Alt_D: 0x0120,
		Alt_F: 0x0121,
		Alt_G: 0x0122,
		Alt_H: 0x0123,
		Alt_J: 0x0124,
		Alt_K: 0x0125,
		Alt_L: 0x0126,
		Alt_Semicolon: 0x0127,
		Alt_Quote: 0x0128,
		Alt_Backquote: 0x0129,
		Alt_Backslash: 0x012b,
		Alt_Z: 0x012c,
		Alt_X: 0x012d,
		Alt_C: 0x012e,
		Alt_V: 0x012f,
		Alt_B: 0x0130,
		Alt_N: 0x0131,
		Alt_M: 0x0132,
		Alt_Comma: 0x0133,
		Alt_Period: 0x0134,
		Alt_Slash: 0x0135,
		Alt_KPStar: 0x0137,
		F1: 0x013b,
		F2: 0x013c,
		F3: 0x013d,
		F4: 0x013e,
		F5: 0x013f,
		F6: 0x0140,
		F7: 0x0141,
		F8: 0x0142,
		F9: 0x0143,
		F10: 0x0144,
		Home: 0x0147,
		Up: 0x0148,
		PageUp: 0x0149,
		Alt_KPMinus: 0x014a,
		Left: 0x014b,
		Center: 0x014c,
		Right: 0x014d,
		Alt_KPPlus: 0x014e,
		End: 0x014f,
		Down: 0x0150,
		PageDown: 0x0151,
		Insert: 0x0152,
		Delete: 0x0153,
		Shift_F1: 0x0154,
		Shift_F2: 0x0155,
		Shift_F3: 0x0156,
		Shift_F4: 0x0157,
		Shift_F5: 0x0158,
		Shift_F6: 0x0159,
		Shift_F7: 0x015a,
		Shift_F8: 0x015b,
		Shift_F9: 0x015c,
		Shift_F10: 0x015d,
		Control_F1: 0x015e,
		Control_F2: 0x015f,
		Control_F3: 0x0160,
		Control_F4: 0x0161,
		Control_F5: 0x0162,
		Control_F6: 0x0163,
		Control_F7: 0x0164,
		Control_F8: 0x0165,
		Control_F9: 0x0166,
		Control_F10: 0x0167,
		Alt_F1: 0x0168,
		Alt_F2: 0x0169,
		Alt_F3: 0x016a,
		Alt_F4: 0x016b,
		Alt_F5: 0x016c,
		Alt_F6: 0x016d,
		Alt_F7: 0x016e,
		Alt_F8: 0x016f,
		Alt_F9: 0x0170,
		Alt_F10: 0x0171,
		Control_Print: 0x0172,
		Control_Left: 0x0173,
		Control_Right: 0x0174,
		Control_End: 0x0175,
		Control_PageDown: 0x0176,
		Control_Home: 0x0177,
		Alt_1: 0x0178,
		Alt_2: 0x0179,
		Alt_3: 0x017a,
		Alt_4: 0x017b,
		Alt_5: 0x017c,
		Alt_6: 0x017d,
		Alt_7: 0x017e,
		Alt_8: 0x017f,
		Alt_9: 0x0180,
		Alt_0: 0x0181,
		Alt_Dash: 0x0182,
		Alt_Equals: 0x0183,
		Control_PageUp: 0x0184,
		F11: 0x0185,
		F12: 0x0186,
		Shift_F11: 0x0187,
		Shift_F12: 0x0188,
		Control_F11: 0x0189,
		Control_F12: 0x018a,
		Alt_F11: 0x018b,
		Alt_F12: 0x018c,
		Control_Up: 0x018d,
		Control_KPDash: 0x018e,
		Control_Center: 0x018f,
		Control_KPPlus: 0x0190,
		Control_Down: 0x0191,
		Control_Insert: 0x0192,
		Control_Delete: 0x0193,
		Control_Tab: 0x0194,
		Control_KPSlash: 0x0195,
		Control_KPStar: 0x0196,
		Alt_KPSlash: 0x01a4,
		Alt_Tab: 0x01a5,
		Alt_Enter: 0x01a6,

		/* some additional codes not in DJGPP */
		Alt_LAngle: 0x01b0,
		Alt_RAngle: 0x01b1,
		Alt_At: 0x01b2,
		Alt_LBrace: 0x01b3,
		Alt_Pipe: 0x01b4,
		Alt_RBrace: 0x01b5,
		Print: 0x01b6,
		Shift_Insert: 0x01b7,
		Shift_Home: 0x01b8,
		Shift_End: 0x01b9,
		Shift_PageUp: 0x01ba,
		Shift_PageDown: 0x01bb,
		Alt_Up: 0x01bc,
		Alt_Left: 0x01bd,
		Alt_Center: 0x01be,
		Alt_Right: 0x01c0,
		Alt_Down: 0x01c1,
		Alt_Insert: 0x01c2,
		Alt_Delete: 0x01c3,
		Alt_Home: 0x01c4,
		Alt_End: 0x01c5,
		Alt_PageUp: 0x01c6,
		Alt_PageDown: 0x01c7,
		Shift_Up: 0x01c8,
		Shift_Down: 0x01c9,
		Shift_Right: 0x01ca,
		Shift_Left: 0x01cb,
		LastDefinedKeycode: 0x01cb
	}
};
