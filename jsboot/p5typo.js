/*
	This file was derived from the p5.js source code at
	https://github.com/processing/p5.js

	Copyright (c) the p5.js contributors and Andre Seidelt <superilu@yahoo.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
* @module p5compat
*/

// internal variables
exports._txtAlignX = LEFT;
exports._txtAlignY = TOP;
exports._font = null;

/**
 * Loads a GRX font file (.FNT) from a file Font Object.
 * <br><br>
 *
 * @method loadFont
 * @param  {String}        path       name of the file or url to load
 * @return {Font}                  Font object
 */
exports.loadFont = function (name) {
	return new Font(name);
};


/**
 * Draws text to the screen. Displays the information specified in the first
 * parameter on the screen in the position specified by the additional
 * parameters. A default font will be used unless a font is set with the
 * <a href="#/p5/textFont">textFont()</a> function and a default size will be used unless a font is set
 * with <a href="#/p5/textSize">textSize()</a>. Change the color of the text with the <a href="#/p5/fill">fill()</a> function.
 * Change the outline of the text with the <a href="#/p5/stroke">stroke()</a> and <a href="#/p5/strokeWeight">strokeWeight()</a>
 * functions.
 * <br><br>
 * The text displays in relation to the <a href="#/p5/textAlign">textAlign()</a> function, which gives the
 * option to draw to the left, right, and center of the coordinates.
 * <br><br>
 * The x2 and y2 parameters define a rectangular area to display within and
 * may only be used with string data. When these parameters are specified,
 * they are interpreted based on the current <a href="#/p5/rectMode">rectMode()</a> setting. Text that
 * does not fit completely within the rectangle specified will not be drawn
 * to the screen. If x2 and y2 are not specified, the baseline alignment is the
 * default, which means that the text will be drawn upwards from x and y.
 * <br><br>
 * <b>WEBGL</b>: Only opentype/truetype fonts are supported. You must load a font using the
 * <a href="#/p5/loadFont">loadFont()</a> method (see the example above).
 * <a href="#/p5/stroke">stroke()</a> currently has no effect in webgl mode.
 *
 * @method text
 * @param {String|Object|Array|Number|Boolean} str the alphanumeric
 *                                             symbols to be displayed
 * @param {Number} x   x-coordinate of text
 * @param {Number} y   y-coordinate of text
 * @chainable
 * @example
 * text('word', 10, 30);
 * fill(0, 102, 153);
 * text('word', 10, 60);
 * fill(0, 102, 153, 51);
 * text('word', 10, 90);
 * 
 * let s = 'The quick brown fox jumped over the lazy dog.';
 * fill(50);
 * text(s, 10, 10, 70, 80); // Text wraps within text box
 *
 * avenir;
 * function setup() {
 *   avenir = loadFont('assets/Avenir.otf');
 *   textFont(avenir);
 *   textSize(width / 3);
 *   textAlign(CENTER, CENTER);
 * }
 * function draw() {
 *   background(0);
 *   text('p5.js', 0, 0);
 * }
 */
exports.text = function (str, x, y) {
	if (_font) {
		_font.DrawString(x, y, str, _fill, NO_COLOR, FONT.Direction.RIGHT, _txtAlignX, _txtAlignY);
	} else {
		TextXY(x, y, str, _fill, NO_COLOR);
	}
};

/**
 * Sets the current font that will be drawn with the <a href="#/p5/text">text()</a> function.
 * <br><br>
 * <b>WEBGL</b>: Only fonts loaded via <a href="#/p5/loadFont">loadFont()</a> are supported.
 *
 * @method textFont
 * @return {Object} the current font
 *
 * @example
 * fill(0);
 * textSize(12);
 * textFont('Georgia');
 * text('Georgia', 12, 30);
 * textFont('Helvetica');
 * text('Helvetica', 12, 60);
 * 
 * let fontRegular, fontItalic, fontBold;
 * function setup() {
 *   fontRegular = loadFont('assets/Regular.otf');
 *   fontItalic = loadFont('assets/Italic.ttf');
 *   fontBold = loadFont('assets/Bold.ttf');
 *   background(210);
 *   fill(0);
 *   textFont(fontRegular);
 *   text('Font Style Normal', 10, 30);
 *   textFont(fontItalic);
 *   text('Font Style Italic', 10, 50);
 *   textFont(fontBold);
 *   text('Font Style Bold', 10, 70);
 * }
 */
exports.textFont = function (f) {
	_font = f;
};

/**
 * Sets the current alignment for drawing text. Accepts two
 * arguments: horizAlign (LEFT, CENTER, or RIGHT) and
 * vertAlign (TOP, BOTTOM, CENTER, or BASELINE).
 *
 * The horizAlign parameter is in reference to the x value
 * of the <a href="#/p5/text">text()</a> function, while the vertAlign parameter is
 * in reference to the y value.
 *
 * So if you write textAlign(LEFT), you are aligning the left
 * edge of your text to the x value you give in <a href="#/p5/text">text()</a>. If you
 * write textAlign(RIGHT, TOP), you are aligning the right edge
 * of your text to the x value and the top of edge of the text
 * to the y value.
 *
 * @method textAlign
 * @param {Constant} horizAlign horizontal alignment, either LEFT,
 *                            CENTER, or RIGHT
 * @param {Constant} [vertAlign] vertical alignment, either TOP,
 *                            BOTTOM, CENTER, or BASELINE
 * @chainable
 * @example
 * textSize(16);
 * textAlign(RIGHT);
 * text('ABCD', 50, 30);
 * textAlign(CENTER);
 * text('EFGH', 50, 50);
 * textAlign(LEFT);
 * text('IJKL', 50, 70);
 *
 * textSize(16);
 * strokeWeight(0.5);
 *
 * line(0, 12, width, 12);
 * textAlign(CENTER, TOP);
 * text('TOP', 0, 12, width);
 *
 * line(0, 37, width, 37);
 * textAlign(CENTER, CENTER);
 * text('CENTER', 0, 37, width);
 *
 * line(0, 62, width, 62);
 * textAlign(CENTER, BASELINE);
 * text('BASELINE', 0, 62, width);
 *
 * line(0, 87, width, 87);
 * textAlign(CENTER, BOTTOM);
 * text('BOTTOM', 0, 87, width);
 */
exports.textAlign = function (modeX, modeY) {
	_txtAlignX = modeX;
	if (modeY) {
		_txtAlignY = modeY;
	}
};

/**
 * Calculates and returns the width of any character or text string.
 *
 * @method textWidth
 * @param {String} theText the String of characters to measure
 * @return {Number}
 * @example
 * textSize(28);
 *
 * let aChar = 'P';
 * let cWidth = textWidth(aChar);
 * text(aChar, 0, 40);
 * line(cWidth, 0, cWidth, 50);
 *
 * let aString = 'p5.js';
 * let sWidth = textWidth(aString);
 * text(aString, 0, 85);
 * line(sWidth, 50, sWidth, 100);
 */
exports.textWidth = function (theText) {
	if (_font) {
		return _font.StringWidth(theText);
	} else {
		throw "No font set, use textFont() first.";
	}
};

/**
 * Gets the current font size.
 *
 * @method textSize
 * @return {Number}
 * @chainable
 */
exports.textSize = function () {
	if (_font) {
		return _font.height;
	} else {
		throw "No font set, use textFont() first.";
	}
};
