/*
MIT License

Copyright (c) 2020 Andre Seidelt <superilu@yahoo.com>

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

var PLAYBOX_HEIGHT = 0;
var MSGBOX_HEIGHT = 0;
var MSG_BORDER = 3;
var FONT_SPACING = 1;

var currentDir = null;
var dirFont = null;
var currentFile = null;

/*
** This function is called once when the script is started.
*/
function Setup() {
	MouseShowCursor(false);
	dirFont = new Font("jsboot/fonts/cour16b.fnt");

	PLAYBOX_HEIGHT = dirFont.height * 3 + 2 * MSG_BORDER;
	MSGBOX_HEIGHT = dirFont.height + 2 * MSG_BORDER;

	currentDir = initDirInfo("./tmp/");
}

/**
 * This function is repeatedly until ESC is pressed or Stop() is called.
 * The screen is split into three parts:
 * 1) info about currently playing MIDI
 * 2) file selector
 * 3) message box
 */
function Loop() {
	ClearScreen(EGA.BLACK);

	updatePlayBox();
	updateMsgBox();
	updateFileBox();
}

/*
** This function is called on any input.
*/
function Input(e) {
	if ((e.key & 0xFF) == CharCode("P") || (e.key & 0xFF) == CharCode("p")) {
		MidiPause();
	} else if ((e.key & 0xFF) == CharCode("S") || (e.key & 0xFF) == CharCode("s")) {
		MidiStop();
	} else if ((e.key & 0xFF) == CharCode("Q") || (e.key & 0xFF) == CharCode("q")) {
		Stop();
	} else if ((e.key >> 8) == KEY.Code.KEY_ENTER || (e.key & 0xFF) == CharCode(" ")) {
		onEnter();
	} else if ((e.key >> 8) == KEY.Code.KEY_UP) {
		cursorUp();
	} else if ((e.key >> 8) == KEY.Code.KEY_DOWN) {
		cursorDown();
	} else if ((e.key >> 8) == KEY.Code.KEY_PGUP) {
		cursorPageUp();
	} else if ((e.key >> 8) == KEY.Code.KEY_PGDN) {
		cursorPageDown();
	} else {
		Println(e.key.toString(16));
	}
}

/**
 * move one page up in MIDI file list.
 */
function cursorPageUp() {
	if (currentDir.top > currentDir.rows) {
		currentDir.top -= currentDir.rows;
	} else if (currentDir.top == 0) {
		currentDir.cursor = 0;
	} else {
		currentDir.top = 0;
	}
}

/**
 * move one page down in MIDI file list.
 */
function cursorPageDown() {
	if (currentDir.list.length < currentDir.rows) {
		currentDir.cursor = currentDir.list.length - 1;
	} else if (currentDir.top == currentDir.list.length - currentDir.rows) {
		currentDir.cursor = currentDir.rows - 1;
	} else {
		currentDir.top = Math.min(currentDir.top + currentDir.rows, currentDir.list.length - currentDir.rows);
	}
}

/**
 * move one file up in MIDI file list.
 */
function cursorUp() {
	if (currentDir.cursor > 0) {
		currentDir.cursor--;
	} else {
		if (currentDir.top > 0) {
			currentDir.top--;
		}
	}
}

/**
 * move one file down in MIDI file list.
 */
function cursorDown() {
	if ((currentDir.cursor < currentDir.rows - 1) && (currentDir.cursor < currentDir.list.length - 1)) {
		currentDir.cursor++;
	} else {
		if (currentDir.top + currentDir.rows < currentDir.list.length) {
			currentDir.top++;
		}
	}
}

/**
 * either play file or enter directory
 */
function onEnter() {
	var idx = currentDir.top + currentDir.cursor;
	var name = currentDir.list[idx];

	if (currentDir.info[name]["is_directory"]) {
		currentDir = initDirInfo(concatPath(currentDir.path, name));
	} else if (name.toLowerCase().endsWith(".mid")) {
		playFile(currentDir.path + name, false);
	}
}

/**
 * start playing a file
 * 
 * @param {string} file the full path of the file
 * @param {boolean} loop trut if the file shall be looped
 */
function playFile(file, loop) {
	MidiStop();
	currentFile = new Midi(file);
	currentFile.Play(loop);
}

/**
 * draw box with current MIDI file info.
 */
function updatePlayBox() {
	FilledBox(0, 0, SizeX(), PLAYBOX_HEIGHT, EGA.LIGHT_GRAY);

	if (MidiIsPlaying()) {
		var txtCol = EGA.LIGHT_GREEN;
	} else {
		var txtCol = EGA.RED;
	}

	// get current file and playtime
	if (currentFile) {
		var playFile = "File     : " + currentFile.filename;
		var playTime = "Time     : " + formatSeconds(MidiGetTime()) + " / " + formatSeconds(currentFile.length);
	} else {
		var playFile = "File     : -/-";
		var playTime = "Time     : -/-";
	}

	// get current pos
	if (MidiGetPos() != -1) {
		var playPos = "Position : " + Math.abs(MidiGetPos());
	} else {
		var playPos = "Position : -/-";
	}

	var yPos = FONT_SPACING
	dirFont.DrawStringLeft(MSG_BORDER, yPos, playFile, txtCol, NO_COLOR);
	yPos += dirFont.height + FONT_SPACING;
	dirFont.DrawStringLeft(MSG_BORDER, yPos, playTime, txtCol, NO_COLOR);
	yPos += dirFont.height + FONT_SPACING;
	dirFont.DrawStringLeft(MSG_BORDER, yPos, playPos, txtCol, NO_COLOR);
}

/**
 * convert an amount of seconds to the format HH:MM:SS.
 * 
 * @param {number} sec the amount of seconds to convert.
 * 
 * @returns {string} a time in the format HH:MM:SS.
 */
function formatSeconds(sec) {
	var hours = formatNum(sec / 60 / 60);
	var minutes = formatNum(sec / 60);
	var seconds = formatNum(sec % 60);
	return hours + ":" + minutes + ":" + seconds;
}

/**
 * make a two digit number with leading "0".
 * 
 * @param {number} num an integer.
 * 
 * @returns {string} a string with at least 2 digits.
 */
function formatNum(num) {
	var num = Math.floor(num) + "";
	if (num.length < 2) {
		return "0" + num;
	} else {
		return num;
	}
}

/**
 * draw message box with cursor position and current path.
 */
function updateMsgBox() {
	FilledBox(0, SizeY() - MSGBOX_HEIGHT, SizeX(), SizeY(), EGA.LIGHT_GREEN);

	var txt = "[" + (currentDir.top + currentDir.cursor + 1) + "/" + currentDir.list.length + "] " + currentDir.path;
	dirFont.DrawStringLeft(MSG_BORDER, SizeY() - dirFont.height - MSG_BORDER, txt, EGA.BLACK, NO_COLOR);
}

/**
 * draw file selector box.
 */
function updateFileBox() {
	Box(0, PLAYBOX_HEIGHT, SizeX(), SizeY() - MSGBOX_HEIGHT, EGA.RED);

	for (var r = 0; r < currentDir.rows; r++) {
		var idx = r + currentDir.top;
		if (idx >= currentDir.list.length) {
			break;
		}
		var name = currentDir.list[idx];
		var filCol = EGA.DARK_GRAY;
		if (name.toLowerCase().endsWith(".mid")) {
			filCol = EGA.LIGHT_GRAY;
		} else if (currentDir.info[name]["is_directory"]) {
			name += "/";
			filCol = EGA.WHITE;
		}

		var yPos = PLAYBOX_HEIGHT + r * (dirFont.height + FONT_SPACING);

		if (r == currentDir.cursor) {
			FilledBox(0, yPos, SizeX(), yPos + dirFont.height, EGA.LIGHT_BLUE);
		}

		dirFont.DrawStringLeft(MSG_BORDER, yPos, name, filCol, NO_COLOR);
	}
}

/**
 * create a 'dir info' object containing all information about the current directory.
 * 
 * @param {string} dir path to the directory
 * 
 * @returns {*} 'dir info' object.
 */
function initDirInfo(dir) {
	// create directory object
	var ret = {
		path: dir,
		top: 0,
		cursor: 0,
		rows: Math.floor((SizeY() - PLAYBOX_HEIGHT - MSGBOX_HEIGHT) / (dirFont.height + FONT_SPACING)),
		list: [],
		info: {}
	};

	// fill 'list' and 'info' properties
	dir.list = List(dir.path);
	dir.list.sort();
	var self = dir.list.indexOf(".");
	dir.list.splice(self, 1);
	for (var l = 0; l < dir.list.length; l++) {
		var name = dir.list[l];
		dir.info[name] = Stat(dir.path + "/" + name);
	}

	return ret;
}

/**
 * update the 'list' and 'info' properties of a directory object.
 * 
 * @param {*} dir a directory object to update.
 */
function updateDir(dir) {
}

/**
 * concatenate path with directory, handle ".." for parent directory.
 * 
 * @param {*} orig current path, must end with a "/"
 * @param {*} sub directory to concatenate or ".." for parent.
 */
function concatPath(orig, sub) {
	if (sub == "..") {
		var parts = orig.split("/");
		if (parts.length >= 2) {
			parts.splice(-2, 1);
			return parts.join("/");
		} else {
			return orig;	// this is just a drive letter
		}
	} else {
		return orig + sub + "/";
	}
}
