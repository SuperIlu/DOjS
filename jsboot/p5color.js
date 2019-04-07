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
exports._fill = EGA.WHITE;
exports._stroke = EGA.WHITE;
exports._background = EGA.BLACK;
exports._colorMode = RGB;

/**********************************************************************************************************************
 * creating/reading
 */
/**
 * @method colorMode
 * @param {Constant} mode
 * @param {Number} max1     range for the red or hue depending on the
 *                              current color mode
 * @param {Number} max2     range for the green or saturation depending
 *                              on the current color mode
 * @param {Number} max3     range for the blue or brightness/lightness
 *                              depending on the current color mode
 * @param {Number} [maxA]   range for the alpha
 * @chainable
 */
exports.colorMode = function (mode, max1, max2, max3, maxA) {
	if (
		mode === RGB ||
		mode === HSB ||
		mode === HSL
	) {
		// Set color mode.
		_colorMode = mode;
	}

	return this;
};

/**
 * Alpha is not supported, the value returned is always 255.
 *
 * @method alpha
 * @param {Color} color Color object
 * @return {Number} the alpha value
 */
exports.alpha = function (c) {
	return 255;
};

/**
 * Extracts the blue value from a color or pixel array.
 *
 * @method blue
 * @param {Color} color Color object
 * @return {Number} the blue value
 * @example
 * let c = color(175, 100, 220); // Define color 'c'
 * fill(c); // Use color variable 'c' as fill color
 * rect(15, 20, 35, 60); // Draw left rectangle
 *
 * let blueValue = blue(c); // Get blue in 'c'
 * print(blueValue); // Prints "220.0"
 * fill(0, 0, blueValue); // Use 'blueValue' in new fill
 * rect(50, 20, 35, 60); // Draw right rectangle
 */
exports.blue = function (c) {
	return c.GetBlue();
};

/**
 * @method color
 * @param  {Number}        v1      red or hue value relative to
 *                                 the current color range
 * @param  {Number}        v2      green or saturation value
 *                                 relative to the current color range
 * @param  {Number}        v3      blue or brightness value
 *                                 relative to the current color range
 * @param  {Number}        [alpha]
 * @return {Color}
 */
exports.color = function (r, g, b, a) {
	if (arguments.length > 2) {
		if (_colorMode === HSB) {
			var rgb = _hsbaToRGBA([r / 255, g / 255, b / 255, 1]);
			r = rgb[0] * 255;
			g = rgb[1] * 255;
			b = rgb[2] * 255;
		} else if (_colorMode === HSL) {
			var rgb = _hsblToRGBA([r / 255, g / 255, b / 255, 1]);
			r = rgb[0] * 255;
			g = rgb[1] * 255;
			b = rgb[2] * 255;
		}
		return new Color(r, g, b);
	} else {
		return new Color(r, r, r);
	}
};

/**
 * Extracts the green value from a color or pixel array.
 *
 * @method green
 * @param {Color} color Color object
 * @return {Number} the green value
 * @example
 * let c = color(20, 75, 200); // Define color 'c'
 * fill(c); // Use color variable 'c' as fill color
 * rect(15, 20, 35, 60); // Draw left rectangle
 *
 * let greenValue = green(c); // Get green in 'c'
 * print(greenValue); // Print "75.0"
 * fill(0, greenValue, 0); // Use 'greenValue' in new fill
 * rect(50, 20, 35, 60); // Draw right rectangle
 */
exports.green = function (c) {
	return c.GetGreen();
};


/**
 * Extracts the red value from a color or pixel array.
 *
 * @method red
 * @param {Color} color Color object
 * @return {Number} the red value
 * @example
 * let c = color(255, 204, 0); // Define color 'c'
 * fill(c); // Use color variable 'c' as fill color
 * rect(15, 20, 35, 60); // Draw left rectangle
 *
 * let redValue = red(c); // Get red in 'c'
 * print(redValue); // Print "255.0"
 * fill(redValue, 0, 0); // Use 'redValue' in new fill
 * rect(50, 20, 35, 60); // Draw right rectangle
 *
 * colorMode(RGB, 255);
 * let c = color(127, 255, 0);
 * colorMode(RGB, 1);
 * let myColor = red(c);
 * print(myColor);
 */
exports.red = function (c) {
	return c.GetRed();
};

/**
 * @method background
 * @param {Number} r     red
 * @param {Number} g     green
 * @param {Number} b     blue 
 * @param  {Number} [a]
 * @chainable
 */

/**
 * @method background
 * @param  {Color}      values  background color.
 * @chainable
 */
exports.background = function (r, g, b, a) {
	if (arguments.length > 2) {
		if (_colorMode === HSB) {
			var rgb = _hsbaToRGBA([r / 255, g / 255, b / 255, 1]);
			r = rgb[0] * 255;
			g = rgb[1] * 255;
			b = rgb[2] * 255;
		} else if (_colorMode === HSL) {
			var rgb = _hsblToRGBA([r / 255, g / 255, b / 255, 1]);
			r = rgb[0] * 255;
			g = rgb[1] * 255;
			b = rgb[2] * 255;
		}
		_background = new Color(r, g, b);
	} else if (r instanceof Color) {
		_background = r;
	} else {
		_background = new Color(r, r, r);
	}
	clear();
};

/**
 * Clears the pixels within a buffer. This function only clears the canvas.
 *
 * @method clear
 * @chainable
 * @example
 * // Clear the screen on mouse press.
 * function setup() {
 *   createCanvas(100, 100);
 * }
 *
 * function draw() {
 *   ellipse(mouseX, mouseY, 20, 20);
 * }
 *
 * function mousePressed() {
 *   clear();
 * }
 */
exports.clear = function () {
	ClearScreen(_background);
}

/**
 * Sets the color used to fill shapes.
 * @method fill
 * @param  {Color}      color   the fill color
 * @chainable
 */
exports.fill = function (r, g, b) {
	if (r instanceof Color) {
		_fill = r;
	} else if (arguments.length > 2) {
		if (_colorMode === HSB) {
			var rgb = _hsbaToRGBA([r / 255, g / 255, b / 255, 1]);
			r = rgb[0] * 255;
			g = rgb[1] * 255;
			b = rgb[2] * 255;
		} else if (_colorMode === HSL) {
			var rgb = _hsblToRGBA([r / 255, g / 255, b / 255, 1]);
			r = rgb[0] * 255;
			g = rgb[1] * 255;
			b = rgb[2] * 255;
		}
		_fill = new Color(r, g, b);
	} else {
		_fill = new Color(r, r, r);
	}
};

/**
 * Disables filling geometry. If both noStroke() and noFill() are called,
 * nothing will be drawn to the screen.
 *
 * @method noFill
 * @chainable
 * @example
 * rect(15, 10, 55, 55);
 * noFill();
 * rect(20, 20, 60, 60);
 *
 * function setup() {
 *   createCanvas(100, 100, WEBGL);
 * }
 *
 * function draw() {
 *   background(0);
 *   noFill();
 *   stroke(100, 100, 240);
 *   rotateX(frameCount * 0.01);
 *   rotateY(frameCount * 0.01);
 *   box(45, 45, 45);
 * }
 */
exports.noFill = function (c) {
	_fill = NO_COLOR;
};

/**
 * Disables drawing the stroke (outline). If both noStroke() and noFill()
 * are called, nothing will be drawn to the screen.
 *
 * @method noStroke
 * @chainable
 * @example
 * noStroke();
 * rect(20, 20, 60, 60);
 *
 * function setup() {
 *   createCanvas(100, 100, WEBGL);
 * }
 *
 * function draw() {
 *   background(0);
 *   noStroke();
 *   fill(240, 150, 150);
 *   rotateX(frameCount * 0.01);
 *   rotateY(frameCount * 0.01);
 *   box(45, 45, 45);
 * }
 */
exports.noStroke = function (c) {
	_stroke = NO_COLOR;
};

/**
 * @method stroke
 * @param  {Color}      color   the stroke color
 * @chainable
 */
exports.stroke = function (r, g, b) {
	if (r instanceof Color) {
		_stroke = r;
	} else if (arguments.length > 2) {
		if (_colorMode === HSB) {
			var rgb = _hsbaToRGBA([r / 255, g / 255, b / 255, 1]);
			r = rgb[0] * 255;
			g = rgb[1] * 255;
			b = rgb[2] * 255;
		} else if (_colorMode === HSL) {
			var rgb = _hsblToRGBA([r / 255, g / 255, b / 255, 1]);
			r = rgb[0] * 255;
			g = rgb[1] * 255;
			b = rgb[2] * 255;
		}
		_stroke = new Color(r, g, b);
	} else {
		_stroke = new Color(r, r, r);
	}
};

/**
 * Convert an HSBA array to RGBA.
 */
exports._hsbaToRGBA = function (hsba) {
	var hue = hsba[0] * 6; // We will split hue into 6 sectors.
	var sat = hsba[1];
	var val = hsba[2];

	var RGBA = [];

	if (sat === 0) {
		RGBA = [val, val, val, hsba[3]]; // Return early if grayscale.
	} else {
		var sector = Math.floor(hue);
		var tint1 = val * (1 - sat);
		var tint2 = val * (1 - sat * (hue - sector));
		var tint3 = val * (1 - sat * (1 + sector - hue));
		var red, green, blue;
		if (sector === 1) {
			// Yellow to green.
			red = tint2;
			green = val;
			blue = tint1;
		} else if (sector === 2) {
			// Green to cyan.
			red = tint1;
			green = val;
			blue = tint3;
		} else if (sector === 3) {
			// Cyan to blue.
			red = tint1;
			green = tint2;
			blue = val;
		} else if (sector === 4) {
			// Blue to magenta.
			red = tint3;
			green = tint1;
			blue = val;
		} else if (sector === 5) {
			// Magenta to red.
			red = val;
			green = tint1;
			blue = tint2;
		} else {
			// Red to yellow (sector could be 0 or 6).
			red = val;
			green = tint3;
			blue = tint1;
		}
		RGBA = [red, green, blue, hsba[3]];
	}

	return RGBA;
};

/**
 * Convert an HSLA array to RGBA.
 *
 * We need to change basis from HSLA to something that can be more easily be
 * projected onto RGBA. We will choose hue and brightness as our first two
 * components, and pick a convenient third one ('zest') so that we don't need
 * to calculate formal HSBA saturation.
 */
exports._hslaToRGBA = function (hsla) {
	var hue = hsla[0] * 6; // We will split hue into 6 sectors.
	var sat = hsla[1];
	var li = hsla[2];

	var RGBA = [];

	if (sat === 0) {
		RGBA = [li, li, li, hsla[3]]; // Return early if grayscale.
	} else {
		// Calculate brightness.
		var val;
		if (li < 0.5) {
			val = (1 + sat) * li;
		} else {
			val = li + sat - li * sat;
		}

		// Define zest.
		var zest = 2 * li - val;

		// Implement projection (project onto green by default).
		var hzvToRGB = function (hue, zest, val) {
			if (hue < 0) {
				// Hue must wrap to allow projection onto red and blue.
				hue += 6;
			} else if (hue >= 6) {
				hue -= 6;
			}
			if (hue < 1) {
				// Red to yellow (increasing green).
				return zest + (val - zest) * hue;
			} else if (hue < 3) {
				// Yellow to cyan (greatest green).
				return val;
			} else if (hue < 4) {
				// Cyan to blue (decreasing green).
				return zest + (val - zest) * (4 - hue);
			} else {
				// Blue to red (least green).
				return zest;
			}
		};

		// Perform projections, offsetting hue as necessary.
		RGBA = [
			hzvToRGB(hue + 2, zest, val),
			hzvToRGB(hue, zest, val),
			hzvToRGB(hue - 2, zest, val),
			hsla[3]
		];
	}

	return RGBA;
};
