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

function Require(name) {
	var cache = Require.cache;
	if (name in cache) { return cache[name]; }
	var exports = {};
	cache[name] = exports;
	Function('exports', Read(name + '.js'))(exports);
	return exports;
}
Require.cache = Object.create(null);

Error.prototype.toString = function () {
	if (this.stackTrace) { return this.name + ': ' + this.message + this.stackTrace; }
	return this.name + ': ' + this.message;
};

function StartupInfo() {
	var mode = GetScreenMode();
	var adapter = GetScreenAdapter();
	var width = SizeX();
	var height = SizeY();
	var colors = NumColors();

	Print(">>> Screen size=" + width + "x" + height + " with " + colors + " colors, mode=" + mode + ", adapter=" + adapter);
}
StartupInfo();

//! arc style definition
ARC = {
	OPEN: 0,
	CLOSE1: 1,
	CLOSE2: 2
};

//! mouse/event constants
MOUSE = {
	Mode: {
		NORMAL: 0,	/* MOUSE CURSOR modes: just the cursor */
		RUBBER: 1,	/* rect. rubber band (XOR-d to the screen) */
		LINE: 2,	/* line attached to the cursor */
		BOX: 3		/* rectangular box dragged by the cursor */
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

//! keycode/state constants
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
		Key_NoKey: 0x0000,		/* no key available */

		Key_OutsideValidRange: 0x0100,	/* key typed but code outside 1..Gr,Key_LastDefinedKeycode */

		/* standard ASCII key codes */
		Key_Control_A: 0x0001,
		Key_Control_B: 0x0002,
		Key_Control_C: 0x0003,
		Key_Control_D: 0x0004,
		Key_Control_E: 0x0005,
		Key_Control_F: 0x0006,
		Key_Control_G: 0x0007,
		Key_Control_H: 0x0008,
		Key_BackSpace: 0x0008,
		Key_Control_I: 0x0009,
		Key_Tab: 0x0009,
		Key_Control_J: 0x000a,
		Key_LineFeed: 0x000a,
		Key_Control_K: 0x000b,
		Key_Control_L: 0x000c,
		Key_Control_M: 0x000d,
		Key_Return: 0x000d,
		Key_Control_N: 0x000e,
		Key_Control_O: 0x000f,
		Key_Control_P: 0x0010,
		Key_Control_Q: 0x0011,
		Key_Control_R: 0x0012,
		Key_Control_S: 0x0013,
		Key_Control_T: 0x0014,
		Key_Control_U: 0x0015,
		Key_Control_V: 0x0016,
		Key_Control_W: 0x0017,
		Key_Control_X: 0x0018,
		Key_Control_Y: 0x0019,
		Key_Control_Z: 0x001a,
		Key_Control_LBracket: 0x001b,
		Key_Escape: 0x001b,
		Key_Control_BackSlash: 0x001c,
		Key_Control_RBracket: 0x001d,
		Key_Control_Caret: 0x001e,
		Key_Control_Underscore: 0x001f,
		Key_Space: 0x0020,
		Key_ExclamationPoint: 0x0021,
		Key_DoubleQuote: 0x0022,
		Key_Hash: 0x0023,
		Key_Dollar: 0x0024,
		Key_Percent: 0x0025,
		Key_Ampersand: 0x0026,
		Key_Quote: 0x0027,
		Key_LParen: 0x0028,
		Key_RParen: 0x0029,
		Key_Star: 0x002a,
		Key_Plus: 0x002b,
		Key_Comma: 0x002c,
		Key_Dash: 0x002d,
		Key_Period: 0x002e,
		Key_Slash: 0x002f,
		Key_0: 0x0030,
		Key_1: 0x0031,
		Key_2: 0x0032,
		Key_3: 0x0033,
		Key_4: 0x0034,
		Key_5: 0x0035,
		Key_6: 0x0036,
		Key_7: 0x0037,
		Key_8: 0x0038,
		Key_9: 0x0039,
		Key_Colon: 0x003a,
		Key_SemiColon: 0x003b,
		Key_LAngle: 0x003c,
		Key_Equals: 0x003d,
		Key_RAngle: 0x003e,
		Key_QuestionMark: 0x003f,
		Key_At: 0x0040,
		Key_A: 0x0041,
		Key_B: 0x0042,
		Key_C: 0x0043,
		Key_D: 0x0044,
		Key_E: 0x0045,
		Key_F: 0x0046,
		Key_G: 0x0047,
		Key_H: 0x0048,
		Key_I: 0x0049,
		Key_J: 0x004a,
		Key_K: 0x004b,
		Key_L: 0x004c,
		Key_M: 0x004d,
		Key_N: 0x004e,
		Key_O: 0x004f,
		Key_P: 0x0050,
		Key_Q: 0x0051,
		Key_R: 0x0052,
		Key_S: 0x0053,
		Key_T: 0x0054,
		Key_U: 0x0055,
		Key_V: 0x0056,
		Key_W: 0x0057,
		Key_X: 0x0058,
		Key_Y: 0x0059,
		Key_Z: 0x005a,
		Key_LBracket: 0x005b,
		Key_BackSlash: 0x005c,
		Key_RBracket: 0x005d,
		Key_Caret: 0x005e,
		Key_UnderScore: 0x005f,
		Key_BackQuote: 0x0060,
		Key_a: 0x0061,
		Key_b: 0x0062,
		Key_c: 0x0063,
		Key_d: 0x0064,
		Key_e: 0x0065,
		Key_f: 0x0066,
		Key_g: 0x0067,
		Key_h: 0x0068,
		Key_i: 0x0069,
		Key_j: 0x006a,
		Key_k: 0x006b,
		Key_l: 0x006c,
		Key_m: 0x006d,
		Key_n: 0x006e,
		Key_o: 0x006f,
		Key_p: 0x0070,
		Key_q: 0x0071,
		Key_r: 0x0072,
		Key_s: 0x0073,
		Key_t: 0x0074,
		Key_u: 0x0075,
		Key_v: 0x0076,
		Key_w: 0x0077,
		Key_x: 0x0078,
		Key_y: 0x0079,
		Key_z: 0x007a,
		Key_LBrace: 0x007b,
		Key_Pipe: 0x007c,
		Key_RBrace: 0x007d,
		Key_Tilde: 0x007e,
		Key_Control_Backspace: 0x007f,

		/* extended key codes as defined in DJGPP */
		Key_Alt_Escape: 0x0101,
		Key_Control_At: 0x0103,
		Key_Alt_Backspace: 0x010e,
		Key_BackTab: 0x010f,
		Key_Alt_Q: 0x0110,
		Key_Alt_W: 0x0111,
		Key_Alt_E: 0x0112,
		Key_Alt_R: 0x0113,
		Key_Alt_T: 0x0114,
		Key_Alt_Y: 0x0115,
		Key_Alt_U: 0x0116,
		Key_Alt_I: 0x0117,
		Key_Alt_O: 0x0118,
		Key_Alt_P: 0x0119,
		Key_Alt_LBracket: 0x011a,
		Key_Alt_RBracket: 0x011b,
		Key_Alt_Return: 0x011c,
		Key_Alt_A: 0x011e,
		Key_Alt_S: 0x011f,
		Key_Alt_D: 0x0120,
		Key_Alt_F: 0x0121,
		Key_Alt_G: 0x0122,
		Key_Alt_H: 0x0123,
		Key_Alt_J: 0x0124,
		Key_Alt_K: 0x0125,
		Key_Alt_L: 0x0126,
		Key_Alt_Semicolon: 0x0127,
		Key_Alt_Quote: 0x0128,
		Key_Alt_Backquote: 0x0129,
		Key_Alt_Backslash: 0x012b,
		Key_Alt_Z: 0x012c,
		Key_Alt_X: 0x012d,
		Key_Alt_C: 0x012e,
		Key_Alt_V: 0x012f,
		Key_Alt_B: 0x0130,
		Key_Alt_N: 0x0131,
		Key_Alt_M: 0x0132,
		Key_Alt_Comma: 0x0133,
		Key_Alt_Period: 0x0134,
		Key_Alt_Slash: 0x0135,
		Key_Alt_KPStar: 0x0137,
		Key_F1: 0x013b,
		Key_F2: 0x013c,
		Key_F3: 0x013d,
		Key_F4: 0x013e,
		Key_F5: 0x013f,
		Key_F6: 0x0140,
		Key_F7: 0x0141,
		Key_F8: 0x0142,
		Key_F9: 0x0143,
		Key_F10: 0x0144,
		Key_Home: 0x0147,
		Key_Up: 0x0148,
		Key_PageUp: 0x0149,
		Key_Alt_KPMinus: 0x014a,
		Key_Left: 0x014b,
		Key_Center: 0x014c,
		Key_Right: 0x014d,
		Key_Alt_KPPlus: 0x014e,
		Key_End: 0x014f,
		Key_Down: 0x0150,
		Key_PageDown: 0x0151,
		Key_Insert: 0x0152,
		Key_Delete: 0x0153,
		Key_Shift_F1: 0x0154,
		Key_Shift_F2: 0x0155,
		Key_Shift_F3: 0x0156,
		Key_Shift_F4: 0x0157,
		Key_Shift_F5: 0x0158,
		Key_Shift_F6: 0x0159,
		Key_Shift_F7: 0x015a,
		Key_Shift_F8: 0x015b,
		Key_Shift_F9: 0x015c,
		Key_Shift_F10: 0x015d,
		Key_Control_F1: 0x015e,
		Key_Control_F2: 0x015f,
		Key_Control_F3: 0x0160,
		Key_Control_F4: 0x0161,
		Key_Control_F5: 0x0162,
		Key_Control_F6: 0x0163,
		Key_Control_F7: 0x0164,
		Key_Control_F8: 0x0165,
		Key_Control_F9: 0x0166,
		Key_Control_F10: 0x0167,
		Key_Alt_F1: 0x0168,
		Key_Alt_F2: 0x0169,
		Key_Alt_F3: 0x016a,
		Key_Alt_F4: 0x016b,
		Key_Alt_F5: 0x016c,
		Key_Alt_F6: 0x016d,
		Key_Alt_F7: 0x016e,
		Key_Alt_F8: 0x016f,
		Key_Alt_F9: 0x0170,
		Key_Alt_F10: 0x0171,
		Key_Control_Print: 0x0172,
		Key_Control_Left: 0x0173,
		Key_Control_Right: 0x0174,
		Key_Control_End: 0x0175,
		Key_Control_PageDown: 0x0176,
		Key_Control_Home: 0x0177,
		Key_Alt_1: 0x0178,
		Key_Alt_2: 0x0179,
		Key_Alt_3: 0x017a,
		Key_Alt_4: 0x017b,
		Key_Alt_5: 0x017c,
		Key_Alt_6: 0x017d,
		Key_Alt_7: 0x017e,
		Key_Alt_8: 0x017f,
		Key_Alt_9: 0x0180,
		Key_Alt_0: 0x0181,
		Key_Alt_Dash: 0x0182,
		Key_Alt_Equals: 0x0183,
		Key_Control_PageUp: 0x0184,
		Key_F11: 0x0185,
		Key_F12: 0x0186,
		Key_Shift_F11: 0x0187,
		Key_Shift_F12: 0x0188,
		Key_Control_F11: 0x0189,
		Key_Control_F12: 0x018a,
		Key_Alt_F11: 0x018b,
		Key_Alt_F12: 0x018c,
		Key_Control_Up: 0x018d,
		Key_Control_KPDash: 0x018e,
		Key_Control_Center: 0x018f,
		Key_Control_KPPlus: 0x0190,
		Key_Control_Down: 0x0191,
		Key_Control_Insert: 0x0192,
		Key_Control_Delete: 0x0193,
		Key_Control_Tab: 0x0194,
		Key_Control_KPSlash: 0x0195,
		Key_Control_KPStar: 0x0196,
		Key_Alt_KPSlash: 0x01a4,
		Key_Alt_Tab: 0x01a5,
		Key_Alt_Enter: 0x01a6,

		/* some additional codes not in DJGPP */
		Key_Alt_LAngle: 0x01b0,
		Key_Alt_RAngle: 0x01b1,
		Key_Alt_At: 0x01b2,
		Key_Alt_LBrace: 0x01b3,
		Key_Alt_Pipe: 0x01b4,
		Key_Alt_RBrace: 0x01b5,
		Key_Print: 0x01b6,
		Key_Shift_Insert: 0x01b7,
		Key_Shift_Home: 0x01b8,
		Key_Shift_End: 0x01b9,
		Key_Shift_PageUp: 0x01ba,
		Key_Shift_PageDown: 0x01bb,
		Key_Alt_Up: 0x01bc,
		Key_Alt_Left: 0x01bd,
		Key_Alt_Center: 0x01be,
		Key_Alt_Right: 0x01c0,
		Key_Alt_Down: 0x01c1,
		Key_Alt_Insert: 0x01c2,
		Key_Alt_Delete: 0x01c3,
		Key_Alt_Home: 0x01c4,
		Key_Alt_End: 0x01c5,
		Key_Alt_PageUp: 0x01c6,
		Key_Alt_PageDown: 0x01c7,
		Key_Shift_Up: 0x01c8,
		Key_Shift_Down: 0x01c9,
		Key_Shift_Right: 0x01ca,
		Key_Shift_Left: 0x01cb,
		Key_LastDefinedKeycode: 0x01cb
	}
};
