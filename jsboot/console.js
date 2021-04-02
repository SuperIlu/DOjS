/**
 * create a on-screen text console.
 * @class
 * @param {Color} [fg] default text color
 * @param {Color} [bg] background color.
 */
function Console(fg, bg) {
	this.fg = fg || EGA.GREEN;
	this.bg = bg || EGA.BLACK;
	this.lines = [[EGA.GREEN, ""]];	// on screen logfile
	this.max_lines = (SizeY() / 10) - 1;
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
		TextXY(2, 10 * (l + 1), txt, col);
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
}

exports.Console = Console;
