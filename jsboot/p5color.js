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
exports._background = EGA.BLACK;

/**********************************************************************************************************************
 * conversion
 */

/**
 * Conversions adapted from <http://www.easyrgb.com/en/math.php>.
 *
 * In these functions, hue is always in the range [0, 1], just like all other
 * components are in the range [0, 1]. 'Brightness' and 'value' are used
 * interchangeably.
 */

exports.ColorConversion = {};

/**
 * Convert an HSBA array to HSLA.
 */
exports.ColorConversion._hsbaToHSLA = function (hsba) {
	var hue = hsba[0];
	var sat = hsba[1];
	var val = hsba[2];

	// Calculate lightness.
	var li = (2 - sat) * val / 2;

	// Convert saturation.
	if (li !== 0) {
		if (li === 1) {
			sat = 0;
		} else if (li < 0.5) {
			sat = sat / (2 - sat);
		} else {
			sat = sat * val / (2 - li * 2);
		}
	}

	// Hue and alpha stay the same.
	return [hue, sat, li, hsba[3]];
};

/**
 * Convert an HSBA array to RGBA.
 */
exports.ColorConversion._hsbaToRGBA = function (hsba) {
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
 * Convert an HSLA array to HSBA.
 */
exports.ColorConversion._hslaToHSBA = function (hsla) {
	var hue = hsla[0];
	var sat = hsla[1];
	var li = hsla[2];

	// Calculate brightness.
	var val;
	if (li < 0.5) {
		val = (1 + sat) * li;
	} else {
		val = li + sat - li * sat;
	}

	// Convert saturation.
	sat = 2 * (val - li) / val;

	// Hue and alpha stay the same.
	return [hue, sat, val, hsla[3]];
};

/**
 * Convert an HSLA array to RGBA.
 *
 * We need to change basis from HSLA to something that can be more easily be
 * projected onto RGBA. We will choose hue and brightness as our first two
 * components, and pick a convenient third one ('zest') so that we don't need
 * to calculate formal HSBA saturation.
 */
exports.ColorConversion._hslaToRGBA = function (hsla) {
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

/**
 * Convert an RGBA array to HSBA.
 */
exports.ColorConversion._rgbaToHSBA = function (rgba) {
	var red = rgba[0];
	var green = rgba[1];
	var blue = rgba[2];

	var val = Math.max(red, green, blue);
	var chroma = val - Math.min(red, green, blue);

	var hue, sat;
	if (chroma === 0) {
		// Return early if grayscale.
		hue = 0;
		sat = 0;
	} else {
		sat = chroma / val;
		if (red === val) {
			// Magenta to yellow.
			hue = (green - blue) / chroma;
		} else if (green === val) {
			// Yellow to cyan.
			hue = 2 + (blue - red) / chroma;
		} else if (blue === val) {
			// Cyan to magenta.
			hue = 4 + (red - green) / chroma;
		}
		if (hue < 0) {
			// Confine hue to the interval [0, 1).
			hue += 6;
		} else if (hue >= 6) {
			hue -= 6;
		}
	}

	return [hue / 6, sat, val, rgba[3]];
};

/**
 * Convert an RGBA array to HSLA.
 */
exports.ColorConversion._rgbaToHSLA = function (rgba) {
	var red = rgba[0];
	var green = rgba[1];
	var blue = rgba[2];

	var val = Math.max(red, green, blue);
	var min = Math.min(red, green, blue);
	var li = val + min; // We will halve this later.
	var chroma = val - min;

	var hue, sat;
	if (chroma === 0) {
		// Return early if grayscale.
		hue = 0;
		sat = 0;
	} else {
		if (li < 1) {
			sat = chroma / li;
		} else {
			sat = chroma / (2 - li);
		}
		if (red === val) {
			// Magenta to yellow.
			hue = (green - blue) / chroma;
		} else if (green === val) {
			// Yellow to cyan.
			hue = 2 + (blue - red) / chroma;
		} else if (blue === val) {
			// Cyan to magenta.
			hue = 4 + (red - green) / chroma;
		}
		if (hue < 0) {
			// Confine hue to the interval [0, 1).
			hue += 6;
		} else if (hue >= 6) {
			hue -= 6;
		}
	}

	return [hue / 6, sat, li / 2, rgba[3]];
};

/**********************************************************************************************************************
 * class Color
 */

/**
* Each color stores the color mode and level maxes that applied at the
* time of its construction. These are used to interpret the input arguments
* (at construction and later for that instance of color) and to format the
* output e.g. when saturation() is requested.
*
* Internally we store an array representing the ideal RGBA values in floating
* point form, normalized from 0 to 1. From this we calculate the closest
* screen color (RGBA levels from 0 to 255) and expose this to the renderer.
*
* We also cache normalized, floating point components of the color in various
* representations as they are calculated. This is done to prevent repeating a
* conversion that has already been performed.
*
* @class Color
*/
exports.p5Color = function (vals) {
	// Record color mode and maxes at time of construction.
	this._storeModeAndMaxes(_currentEnv._colorMode, _currentEnv._colorMaxes);

	// Calculate normalized RGBA values.
	if (
		this.mode !== RGB &&
		this.mode !== HSL &&
		this.mode !== HSB
	) {
		throw new Error(this.mode + ' is an invalid colorMode.');
	} else {
		this._array = p5Color._parseInputs.apply(this, vals);
	}

	// Expose closest screen color.
	this._calculateLevels();
	return this;
};

exports.p5Color.prototype.toAllegro = function () {
	var a = this.levels;
	// FIX: alpha can never be 255 because the resulting integer for WHITE would be -1 and that is equal to 'no color'
	return Color(a[0], a[1], a[2], a[3] == 255 ? 254 : a[3]);
};

/**
 * This function returns the color formatted as a string. This can be useful
 * for debugging, or for using p5.js with other libraries.
 * @method toString
 * @param {String} [format] How the color string will be formatted.
 * Leaving this empty formats the string as rgba(r, g, b, a).
 * '#rgb' '#rgba' '#rrggbb' and '#rrggbbaa' format as hexadecimal color codes.
 * 'rgb' 'hsb' and 'hsl' return the color formatted in the specified color mode.
 * 'rgba' 'hsba' and 'hsla' are the same as above but with alpha channels.
 * 'rgb%' 'hsb%' 'hsl%' 'rgba%' 'hsba%' and 'hsla%' format as percentages.
 * @return {String} the formatted string
 * @example
 * let myColor;
 * function setup() {
 *   createCanvas(200, 200);
 *   stroke(255);
 *   myColor = color(100, 100, 250);
 *   fill(myColor);
 * }
 *
 * function draw() {
 *   rotate(HALF_PI);
 *   text(myColor.toString(), 0, -5);
 *   text(myColor.toString('#rrggbb'), 0, -30);
 *   text(myColor.toString('rgba%'), 0, -55);
 * }
 */
exports.p5Color.prototype.toString = function (format) {
	var a = this.levels;
	var f = this._array;
	var alpha = f[3]; // String representation uses normalized alpha

	switch (format) {
		case '#rrggbb':
			return '#'.concat(
				a[0] < 16 ? '0'.concat(a[0].toString(16)) : a[0].toString(16),
				a[1] < 16 ? '0'.concat(a[1].toString(16)) : a[1].toString(16),
				a[2] < 16 ? '0'.concat(a[2].toString(16)) : a[2].toString(16)
			);

		case '#rrggbbaa':
			return '#'.concat(
				a[0] < 16 ? '0'.concat(a[0].toString(16)) : a[0].toString(16),
				a[1] < 16 ? '0'.concat(a[1].toString(16)) : a[1].toString(16),
				a[2] < 16 ? '0'.concat(a[2].toString(16)) : a[2].toString(16),
				a[3] < 16 ? '0'.concat(a[2].toString(16)) : a[3].toString(16)
			);

		case '#rgb':
			return '#'.concat(
				Math.round(f[0] * 15).toString(16),
				Math.round(f[1] * 15).toString(16),
				Math.round(f[2] * 15).toString(16)
			);

		case '#rgba':
			return '#'.concat(
				Math.round(f[0] * 15).toString(16),
				Math.round(f[1] * 15).toString(16),
				Math.round(f[2] * 15).toString(16),
				Math.round(f[3] * 15).toString(16)
			);

		case 'rgb':
			return 'rgb('.concat(a[0], ', ', a[1], ', ', a[2], ')');

		case 'rgb%':
			return 'rgb('.concat(
				(100 * f[0]).toPrecision(3),
				'%, ',
				(100 * f[1]).toPrecision(3),
				'%, ',
				(100 * f[2]).toPrecision(3),
				'%)'
			);

		case 'rgba%':
			return 'rgba('.concat(
				(100 * f[0]).toPrecision(3),
				'%, ',
				(100 * f[1]).toPrecision(3),
				'%, ',
				(100 * f[2]).toPrecision(3),
				'%, ',
				(100 * f[3]).toPrecision(3),
				'%)'
			);

		case 'hsb':
		case 'hsv':
			if (!this.hsba) this.hsba = ColorConversion._rgbaToHSBA(this._array);
			return 'hsb('.concat(
				this.hsba[0] * this.maxes[HSB][0],
				', ',
				this.hsba[1] * this.maxes[HSB][1],
				', ',
				this.hsba[2] * this.maxes[HSB][2],
				')'
			);

		case 'hsb%':
		case 'hsv%':
			if (!this.hsba) this.hsba = ColorConversion._rgbaToHSBA(this._array);
			return 'hsb('.concat(
				(100 * this.hsba[0]).toPrecision(3),
				'%, ',
				(100 * this.hsba[1]).toPrecision(3),
				'%, ',
				(100 * this.hsba[2]).toPrecision(3),
				'%)'
			);

		case 'hsba':
		case 'hsva':
			if (!this.hsba) this.hsba = ColorConversion._rgbaToHSBA(this._array);
			return 'hsba('.concat(
				this.hsba[0] * this.maxes[HSB][0],
				', ',
				this.hsba[1] * this.maxes[HSB][1],
				', ',
				this.hsba[2] * this.maxes[HSB][2],
				', ',
				alpha,
				')'
			);

		case 'hsba%':
		case 'hsva%':
			if (!this.hsba) this.hsba = ColorConversion._rgbaToHSBA(this._array);
			return 'hsba('.concat(
				(100 * this.hsba[0]).toPrecision(3),
				'%, ',
				(100 * this.hsba[1]).toPrecision(3),
				'%, ',
				(100 * this.hsba[2]).toPrecision(3),
				'%, ',
				(100 * alpha).toPrecision(3),
				'%)'
			);

		case 'hsl':
			if (!this.hsla) this.hsla = ColorConversion._rgbaToHSLA(this._array);
			return 'hsl('.concat(
				this.hsla[0] * this.maxes[HSL][0],
				', ',
				this.hsla[1] * this.maxes[HSL][1],
				', ',
				this.hsla[2] * this.maxes[HSL][2],
				')'
			);

		case 'hsl%':
			if (!this.hsla) this.hsla = ColorConversion._rgbaToHSLA(this._array);
			return 'hsl('.concat(
				(100 * this.hsla[0]).toPrecision(3),
				'%, ',
				(100 * this.hsla[1]).toPrecision(3),
				'%, ',
				(100 * this.hsla[2]).toPrecision(3),
				'%)'
			);

		case 'hsla':
			if (!this.hsla) this.hsla = ColorConversion._rgbaToHSLA(this._array);
			return 'hsla('.concat(
				this.hsla[0] * this.maxes[HSL][0],
				', ',
				this.hsla[1] * this.maxes[HSL][1],
				', ',
				this.hsla[2] * this.maxes[HSL][2],
				', ',
				alpha,
				')'
			);

		case 'hsla%':
			if (!this.hsla) this.hsla = ColorConversion._rgbaToHSLA(this._array);
			return 'hsl('.concat(
				(100 * this.hsla[0]).toPrecision(3),
				'%, ',
				(100 * this.hsla[1]).toPrecision(3),
				'%, ',
				(100 * this.hsla[2]).toPrecision(3),
				'%, ',
				(100 * alpha).toPrecision(3),
				'%)'
			);

		case 'rgba':
		default:
			return 'rgba('.concat(a[0], ',', a[1], ',', a[2], ',', alpha, ')');
	}
};

/**
 * @method setRed
 * @param {Number} red the new red value
 * @example
 * let backgroundColor;
 *
 * function setup() {
 *   backgroundColor = color(100, 50, 150);
 * }
 *
 * function draw() {
 *   backgroundColor.setRed(128 + 128 * sin(millis() / 1000));
 *   background(backgroundColor);
 * }
 */
exports.p5Color.prototype.setRed = function (new_red) {
	this._array[0] = new_red / this.maxes[RGB][0];
	this._calculateLevels();
};

/**
 * @method setGreen
 * @param {Number} green the new green value
 * @example
 * let backgroundColor;
 *
 * function setup() {
 *   backgroundColor = color(100, 50, 150);
 * }
 *
 * function draw() {
 *   backgroundColor.setGreen(128 + 128 * sin(millis() / 1000));
 *   background(backgroundColor);
 * }
 **/
exports.p5Color.prototype.setGreen = function (new_green) {
	this._array[1] = new_green / this.maxes[RGB][1];
	this._calculateLevels();
};

/**
 * @method setBlue
 * @param {Number} blue the new blue value
 * @example
 * let backgroundColor;
 *
 * function setup() {
 *   backgroundColor = color(100, 50, 150);
 * }
 *
 * function draw() {
 *   backgroundColor.setBlue(128 + 128 * sin(millis() / 1000));
 *   background(backgroundColor);
 * }
 **/
exports.p5Color.prototype.setBlue = function (new_blue) {
	this._array[2] = new_blue / this.maxes[RGB][2];
	this._calculateLevels();
};

/**
 * @method setAlpha
 * @param {Number} alpha the new alpha value
 * @example
 * let squareColor;
 *
 * function setup() {
 *   ellipseMode(CORNERS);
 *   strokeWeight(4);
 *   squareColor = color(100, 50, 150);
 * }
 *
 * function draw() {
 *   background(255);
 *
 *   noFill();
 *   stroke(0);
 *   ellipse(10, 10, width - 10, height - 10);
 *
 *   squareColor.setAlpha(128 + 128 * sin(millis() / 1000));
 *   fill(squareColor);
 *   noStroke();
 *   rect(13, 13, width - 26, height - 26);
 * }
 **/
exports.p5Color.prototype.setAlpha = function (new_alpha) {
	this._array[3] = new_alpha / this.maxes[this.mode][3];
	this._calculateLevels();
};

// calculates and stores the closest screen levels
exports.p5Color.prototype._calculateLevels = function () {
	var array = this._array;
	// (loop backwards for performance)
	var levels = (this.levels = new Array(array.length));
	for (var i = array.length - 1; i >= 0; --i) {
		levels[i] = Math.round(array[i] * 255);
	}
};

exports.p5Color.prototype._getAlpha = function () {
	return this._array[3] * this.maxes[this.mode][3];
};

// stores the color mode and maxes in this instance of Color
// for later use (by _parseInputs())
exports.p5Color.prototype._storeModeAndMaxes = function (new_mode, new_maxes) {
	this.mode = new_mode;
	this.maxes = new_maxes;
};

exports.p5Color.prototype._getMode = function () {
	return this.mode;
};

exports.p5Color.prototype._getMaxes = function () {
	return this.maxes;
};

exports.p5Color.prototype._getBlue = function () {
	return this._array[2] * this.maxes[RGB][2];
};

exports.p5Color.prototype._getBrightness = function () {
	if (!this.hsba) {
		this.hsba = ColorConversion._rgbaToHSBA(this._array);
	}
	return this.hsba[2] * this.maxes[HSB][2];
};

exports.p5Color.prototype._getGreen = function () {
	return this._array[1] * this.maxes[RGB][1];
};

/**
 * Hue is the same in HSB and HSL, but the maximum value may be different.
 * This function will return the HSB-normalized saturation when supplied with
 * an HSB color object, but will default to the HSL-normalized saturation
 * otherwise.
 */
exports.p5Color.prototype._getHue = function () {
	if (this.mode === HSB) {
		if (!this.hsba) {
			this.hsba = ColorConversion._rgbaToHSBA(this._array);
		}
		return this.hsba[0] * this.maxes[HSB][0];
	} else {
		if (!this.hsla) {
			this.hsla = ColorConversion._rgbaToHSLA(this._array);
		}
		return this.hsla[0] * this.maxes[HSL][0];
	}
};

exports.p5Color.prototype._getLightness = function () {
	if (!this.hsla) {
		this.hsla = ColorConversion._rgbaToHSLA(this._array);
	}
	return this.hsla[2] * this.maxes[HSL][2];
};

exports.p5Color.prototype._getRed = function () {
	return this._array[0] * this.maxes[RGB][0];
};

/**
 * Saturation is scaled differently in HSB and HSL. This function will return
 * the HSB saturation when supplied with an HSB color object, but will default
 * to the HSL saturation otherwise.
 */
exports.p5Color.prototype._getSaturation = function () {
	if (this.mode === HSB) {
		if (!this.hsba) {
			this.hsba = ColorConversion._rgbaToHSBA(this._array);
		}
		return this.hsba[1] * this.maxes[HSB][1];
	} else {
		if (!this.hsla) {
			this.hsla = ColorConversion._rgbaToHSLA(this._array);
		}
		return this.hsla[1] * this.maxes[HSL][1];
	}
};

/**
 * CSS named colors.
 */
var namedColors = {
	aliceblue: '#f0f8ff',
	antiquewhite: '#faebd7',
	aqua: '#00ffff',
	aquamarine: '#7fffd4',
	azure: '#f0ffff',
	beige: '#f5f5dc',
	bisque: '#ffe4c4',
	black: '#000000',
	blanchedalmond: '#ffebcd',
	blue: '#0000ff',
	blueviolet: '#8a2be2',
	brown: '#a52a2a',
	burlywood: '#deb887',
	cadetblue: '#5f9ea0',
	chartreuse: '#7fff00',
	chocolate: '#d2691e',
	coral: '#ff7f50',
	cornflowerblue: '#6495ed',
	cornsilk: '#fff8dc',
	crimson: '#dc143c',
	cyan: '#00ffff',
	darkblue: '#00008b',
	darkcyan: '#008b8b',
	darkgoldenrod: '#b8860b',
	darkgray: '#a9a9a9',
	darkgreen: '#006400',
	darkgrey: '#a9a9a9',
	darkkhaki: '#bdb76b',
	darkmagenta: '#8b008b',
	darkolivegreen: '#556b2f',
	darkorange: '#ff8c00',
	darkorchid: '#9932cc',
	darkred: '#8b0000',
	darksalmon: '#e9967a',
	darkseagreen: '#8fbc8f',
	darkslateblue: '#483d8b',
	darkslategray: '#2f4f4f',
	darkslategrey: '#2f4f4f',
	darkturquoise: '#00ced1',
	darkviolet: '#9400d3',
	deeppink: '#ff1493',
	deepskyblue: '#00bfff',
	dimgray: '#696969',
	dimgrey: '#696969',
	dodgerblue: '#1e90ff',
	firebrick: '#b22222',
	floralwhite: '#fffaf0',
	forestgreen: '#228b22',
	fuchsia: '#ff00ff',
	gainsboro: '#dcdcdc',
	ghostwhite: '#f8f8ff',
	gold: '#ffd700',
	goldenrod: '#daa520',
	gray: '#808080',
	green: '#008000',
	greenyellow: '#adff2f',
	grey: '#808080',
	honeydew: '#f0fff0',
	hotpink: '#ff69b4',
	indianred: '#cd5c5c',
	indigo: '#4b0082',
	ivory: '#fffff0',
	khaki: '#f0e68c',
	lavender: '#e6e6fa',
	lavenderblush: '#fff0f5',
	lawngreen: '#7cfc00',
	lemonchiffon: '#fffacd',
	lightblue: '#add8e6',
	lightcoral: '#f08080',
	lightcyan: '#e0ffff',
	lightgoldenrodyellow: '#fafad2',
	lightgray: '#d3d3d3',
	lightgreen: '#90ee90',
	lightgrey: '#d3d3d3',
	lightpink: '#ffb6c1',
	lightsalmon: '#ffa07a',
	lightseagreen: '#20b2aa',
	lightskyblue: '#87cefa',
	lightslategray: '#778899',
	lightslategrey: '#778899',
	lightsteelblue: '#b0c4de',
	lightyellow: '#ffffe0',
	lime: '#00ff00',
	limegreen: '#32cd32',
	linen: '#faf0e6',
	magenta: '#ff00ff',
	maroon: '#800000',
	mediumaquamarine: '#66cdaa',
	mediumblue: '#0000cd',
	mediumorchid: '#ba55d3',
	mediumpurple: '#9370db',
	mediumseagreen: '#3cb371',
	mediumslateblue: '#7b68ee',
	mediumspringgreen: '#00fa9a',
	mediumturquoise: '#48d1cc',
	mediumvioletred: '#c71585',
	midnightblue: '#191970',
	mintcream: '#f5fffa',
	mistyrose: '#ffe4e1',
	moccasin: '#ffe4b5',
	navajowhite: '#ffdead',
	navy: '#000080',
	oldlace: '#fdf5e6',
	olive: '#808000',
	olivedrab: '#6b8e23',
	orange: '#ffa500',
	orangered: '#ff4500',
	orchid: '#da70d6',
	palegoldenrod: '#eee8aa',
	palegreen: '#98fb98',
	paleturquoise: '#afeeee',
	palevioletred: '#db7093',
	papayawhip: '#ffefd5',
	peachpuff: '#ffdab9',
	peru: '#cd853f',
	pink: '#ffc0cb',
	plum: '#dda0dd',
	powderblue: '#b0e0e6',
	purple: '#800080',
	red: '#ff0000',
	rosybrown: '#bc8f8f',
	royalblue: '#4169e1',
	saddlebrown: '#8b4513',
	salmon: '#fa8072',
	sandybrown: '#f4a460',
	seagreen: '#2e8b57',
	seashell: '#fff5ee',
	sienna: '#a0522d',
	silver: '#c0c0c0',
	skyblue: '#87ceeb',
	slateblue: '#6a5acd',
	slategray: '#708090',
	slategrey: '#708090',
	snow: '#fffafa',
	springgreen: '#00ff7f',
	steelblue: '#4682b4',
	tan: '#d2b48c',
	teal: '#008080',
	thistle: '#d8bfd8',
	tomato: '#ff6347',
	turquoise: '#40e0d0',
	violet: '#ee82ee',
	wheat: '#f5deb3',
	white: '#ffffff',
	whitesmoke: '#f5f5f5',
	yellow: '#ffff00',
	yellowgreen: '#9acd32'
};

/**
 * These regular expressions are used to build up the patterns for matching
 * viable CSS color strings: fragmenting the regexes in this way increases the
 * legibility and comprehensibility of the code.
 *
 * Note that RGB values of .9 are not parsed by IE, but are supported here for
 * color string consistency.
 */
var WHITESPACE = /\s*/; // Match zero or more whitespace characters.
var INTEGER = /(\d{1,3})/; // Match integers: 79, 255, etc.
var DECIMAL = /((?:\d+(?:\.\d+)?)|(?:\.\d+))/; // Match 129.6, 79, .9, etc.
var PERCENT = new RegExp(DECIMAL.source + '%'); // Match 12.9%, 79%, .9%, etc.

/**
 * Full color string patterns. The capture groups are necessary.
 */
var colorPatterns = {
	// Match colors in format #XXX, e.g. #416.
	HEX3: /^#([a-f0-9])([a-f0-9])([a-f0-9])$/i,

	// Match colors in format #XXXX, e.g. #5123.
	HEX4: /^#([a-f0-9])([a-f0-9])([a-f0-9])([a-f0-9])$/i,

	// Match colors in format #XXXXXX, e.g. #b4d455.
	HEX6: /^#([a-f0-9]{2})([a-f0-9]{2})([a-f0-9]{2})$/i,

	// Match colors in format #XXXXXXXX, e.g. #b4d45535.
	HEX8: /^#([a-f0-9]{2})([a-f0-9]{2})([a-f0-9]{2})([a-f0-9]{2})$/i,

	// Match colors in format rgb(R, G, B), e.g. rgb(255, 0, 128).
	RGB: new RegExp(
		[
			'^rgb\\(',
			INTEGER.source,
			',',
			INTEGER.source,
			',',
			INTEGER.source,
			'\\)$'
		].join(WHITESPACE.source),
		'i'
	),

	// Match colors in format rgb(R%, G%, B%), e.g. rgb(100%, 0%, 28.9%).
	RGB_PERCENT: new RegExp(
		[
			'^rgb\\(',
			PERCENT.source,
			',',
			PERCENT.source,
			',',
			PERCENT.source,
			'\\)$'
		].join(WHITESPACE.source),
		'i'
	),

	// Match colors in format rgb(R, G, B, A), e.g. rgb(255, 0, 128, 0.25).
	RGBA: new RegExp(
		[
			'^rgba\\(',
			INTEGER.source,
			',',
			INTEGER.source,
			',',
			INTEGER.source,
			',',
			DECIMAL.source,
			'\\)$'
		].join(WHITESPACE.source),
		'i'
	),

	// Match colors in format rgb(R%, G%, B%, A), e.g. rgb(100%, 0%, 28.9%, 0.5).
	RGBA_PERCENT: new RegExp(
		[
			'^rgba\\(',
			PERCENT.source,
			',',
			PERCENT.source,
			',',
			PERCENT.source,
			',',
			DECIMAL.source,
			'\\)$'
		].join(WHITESPACE.source),
		'i'
	),

	// Match colors in format hsla(H, S%, L%), e.g. hsl(100, 40%, 28.9%).
	HSL: new RegExp(
		[
			'^hsl\\(',
			INTEGER.source,
			',',
			PERCENT.source,
			',',
			PERCENT.source,
			'\\)$'
		].join(WHITESPACE.source),
		'i'
	),

	// Match colors in format hsla(H, S%, L%, A), e.g. hsla(100, 40%, 28.9%, 0.5).
	HSLA: new RegExp(
		[
			'^hsla\\(',
			INTEGER.source,
			',',
			PERCENT.source,
			',',
			PERCENT.source,
			',',
			DECIMAL.source,
			'\\)$'
		].join(WHITESPACE.source),
		'i'
	),

	// Match colors in format hsb(H, S%, B%), e.g. hsb(100, 40%, 28.9%).
	HSB: new RegExp(
		[
			'^hsb\\(',
			INTEGER.source,
			',',
			PERCENT.source,
			',',
			PERCENT.source,
			'\\)$'
		].join(WHITESPACE.source),
		'i'
	),

	// Match colors in format hsba(H, S%, B%, A), e.g. hsba(100, 40%, 28.9%, 0.5).
	HSBA: new RegExp(
		[
			'^hsba\\(',
			INTEGER.source,
			',',
			PERCENT.source,
			',',
			PERCENT.source,
			',',
			DECIMAL.source,
			'\\)$'
		].join(WHITESPACE.source),
		'i'
	)
};

/**
 * For a number of different inputs, returns a color formatted as [r, g, b, a]
 * arrays, with each component normalized between 0 and 1.
 *
 * @private
 * @param {Array} [...args] An 'array-like' object that represents a list of
 *                          arguments
 * @return {Number[]}       a color formatted as [r, g, b, a]
 *                          Example:
 *                          input        ==> output
 *                          g            ==> [g, g, g, 255]
 *                          g,a          ==> [g, g, g, a]
 *                          r, g, b      ==> [r, g, b, 255]
 *                          r, g, b, a   ==> [r, g, b, a]
 *                          [g]          ==> [g, g, g, 255]
 *                          [g, a]       ==> [g, g, g, a]
 *                          [r, g, b]    ==> [r, g, b, 255]
 *                          [r, g, b, a] ==> [r, g, b, a]
 * @example
 * // todo
 */
exports.p5Color._parseInputs = function (r, g, b, a) {
	var numArgs = arguments.length;
	var mode = this.mode;
	var maxes = this.maxes[mode];
	var results = [];
	var i;

	if (numArgs >= 3) {
		// Argument is a list of component values.

		results[0] = r / maxes[0];
		results[1] = g / maxes[1];
		results[2] = b / maxes[2];

		// Alpha may be undefined, so default it to 100%.
		if (typeof a === 'number') {
			results[3] = a / maxes[3];
		} else {
			results[3] = 1;
		}

		// Constrain components to the range [0,1].
		// (loop backwards for performance)
		for (i = results.length - 1; i >= 0; --i) {
			var result = results[i];
			if (result < 0) {
				results[i] = 0;
			} else if (result > 1) {
				results[i] = 1;
			}
		}

		// Convert to RGBA and return.
		if (mode === HSL) {
			return ColorConversion._hslaToRGBA(results);
		} else if (mode === HSB) {
			return ColorConversion._hsbaToRGBA(results);
		} else {
			return results;
		}
	} else if (numArgs === 1 && typeof r === 'string') {
		var str = r.trim().toLowerCase();

		// Return if string is a named colour.
		if (namedColors[str]) {
			return p5Color._parseInputs.call(this, namedColors[str]);
		}

		// Try RGBA pattern matching.
		if (colorPatterns.HEX3.test(str)) {
			// #rgb
			results = colorPatterns.HEX3.exec(str)
				.slice(1)
				.map(function (color) {
					return parseInt(color + color, 16) / 255;
				});
			results[3] = 1;
			return results;
		} else if (colorPatterns.HEX6.test(str)) {
			// #rrggbb
			results = colorPatterns.HEX6.exec(str)
				.slice(1)
				.map(function (color) {
					return parseInt(color, 16) / 255;
				});
			results[3] = 1;
			return results;
		} else if (colorPatterns.HEX4.test(str)) {
			// #rgba
			results = colorPatterns.HEX4.exec(str)
				.slice(1)
				.map(function (color) {
					return parseInt(color + color, 16) / 255;
				});
			return results;
		} else if (colorPatterns.HEX8.test(str)) {
			// #rrggbbaa
			results = colorPatterns.HEX8.exec(str)
				.slice(1)
				.map(function (color) {
					return parseInt(color, 16) / 255;
				});
			return results;
		} else if (colorPatterns.RGB.test(str)) {
			// rgb(R,G,B)
			results = colorPatterns.RGB.exec(str)
				.slice(1)
				.map(function (color) {
					return color / 255;
				});
			results[3] = 1;
			return results;
		} else if (colorPatterns.RGB_PERCENT.test(str)) {
			// rgb(R%,G%,B%)
			results = colorPatterns.RGB_PERCENT.exec(str)
				.slice(1)
				.map(function (color) {
					return parseFloat(color) / 100;
				});
			results[3] = 1;
			return results;
		} else if (colorPatterns.RGBA.test(str)) {
			// rgba(R,G,B,A)
			results = colorPatterns.RGBA.exec(str)
				.slice(1)
				.map(function (color, idx) {
					if (idx === 3) {
						return parseFloat(color);
					}
					return color / 255;
				});
			return results;
		} else if (colorPatterns.RGBA_PERCENT.test(str)) {
			// rgba(R%,G%,B%,A%)
			results = colorPatterns.RGBA_PERCENT.exec(str)
				.slice(1)
				.map(function (color, idx) {
					if (idx === 3) {
						return parseFloat(color);
					}
					return parseFloat(color) / 100;
				});
			return results;
		}

		// Try HSLA pattern matching.
		if (colorPatterns.HSL.test(str)) {
			// hsl(H,S,L)
			results = colorPatterns.HSL.exec(str)
				.slice(1)
				.map(function (color, idx) {
					if (idx === 0) {
						return parseInt(color, 10) / 360;
					}
					return parseInt(color, 10) / 100;
				});
			results[3] = 1;
		} else if (colorPatterns.HSLA.test(str)) {
			// hsla(H,S,L,A)
			results = colorPatterns.HSLA.exec(str)
				.slice(1)
				.map(function (color, idx) {
					if (idx === 0) {
						return parseInt(color, 10) / 360;
					} else if (idx === 3) {
						return parseFloat(color);
					}
					return parseInt(color, 10) / 100;
				});
		}
		results = results.map(function (value) {
			return Math.max(Math.min(value, 1), 0);
		});
		if (results.length) {
			return ColorConversion._hslaToRGBA(results);
		}

		// Try HSBA pattern matching.
		if (colorPatterns.HSB.test(str)) {
			// hsb(H,S,B)
			results = colorPatterns.HSB.exec(str)
				.slice(1)
				.map(function (color, idx) {
					if (idx === 0) {
						return parseInt(color, 10) / 360;
					}
					return parseInt(color, 10) / 100;
				});
			results[3] = 1;
		} else if (colorPatterns.HSBA.test(str)) {
			// hsba(H,S,B,A)
			results = colorPatterns.HSBA.exec(str)
				.slice(1)
				.map(function (color, idx) {
					if (idx === 0) {
						return parseInt(color, 10) / 360;
					} else if (idx === 3) {
						return parseFloat(color);
					}
					return parseInt(color, 10) / 100;
				});
		}

		if (results.length) {
			// (loop backwards for performance)
			for (i = results.length - 1; i >= 0; --i) {
				results[i] = Math.max(Math.min(results[i], 1), 0);
			}

			return ColorConversion._hsbaToRGBA(results);
		}

		// Input did not match any CSS color pattern: default to white.
		results = [1, 1, 1, 1];
	} else if ((numArgs === 1 || numArgs === 2) && typeof r === 'number') {
		// 'Grayscale' mode.

		/**
		 * For HSB and HSL, interpret the gray level as a brightness/lightness
		 * value (they are equivalent when chroma is zero). For RGB, normalize the
		 * gray level according to the blue maximum.
		 */
		results[0] = r / maxes[2];
		results[1] = r / maxes[2];
		results[2] = r / maxes[2];

		// Alpha may be undefined, so default it to 100%.
		if (typeof g === 'number') {
			results[3] = g / maxes[3];
		} else {
			results[3] = 1;
		}

		// Constrain components to the range [0,1].
		results = results.map(function (value) {
			return Math.max(Math.min(value, 1), 0);
		});
	} else {
		throw new Error(arguments + 'is not a valid color representation.');
	}

	return results;
};

/**********************************************************************************************************************
 * creating/reading
 */

/**
* Extracts the alpha value from a color or pixel array.
*
* @method alpha
* @param {Color|Number[]|String} color Color object, color components,
*                                         or CSS color
* @return {Number} the alpha value
* @example
* noStroke();
* let c = color(0, 126, 255, 102);
* fill(c);
* rect(15, 15, 35, 70);
* let value = alpha(c); // Sets 'value' to 102
* fill(value);
* rect(50, 15, 35, 70);
*/
exports.alpha = function (c) {
	return color(c)._getAlpha();
};

/**
 * Extracts the blue value from a color or pixel array.
 *
 * @method blue
 * @param {Color|Number[]|String} color Color object, color components,
 *                                         or CSS color
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
	return color(c)._getBlue();
};

/**
 * Extracts the HSB brightness value from a color or pixel array.
 *
 * @method brightness
 * @param {Color|Number[]|String} color Color object, color components,
 *                                         or CSS color
 * @return {Number} the brightness value
 * @example
 * noStroke();
 * colorMode(HSB, 255);
 * let c = color(0, 126, 255);
 * fill(c);
 * rect(15, 20, 35, 60);
 * let value = brightness(c); // Sets 'value' to 255
 * fill(value);
 * rect(50, 20, 35, 60);
 */
exports.brightness = function (c) {
	return color(c)._getBrightness();
};

/**
 * Creates colors for storing in variables of the color datatype. The
 * parameters are interpreted as RGB or HSB values depending on the
 * current colorMode(). The default mode is RGB values from 0 to 255
 * and, therefore, the function call color(255, 204, 0) will return a
 * bright yellow color.
 * <br><br>
 * Note that if only one value is provided to color(), it will be interpreted
 * as a grayscale value. Add a second value, and it will be used for alpha
 * transparency. When three values are specified, they are interpreted as
 * either RGB or HSB values. Adding a fourth value applies alpha
 * transparency.
 * <br><br>
 * If a single string argument is provided, RGB, RGBA and Hex CSS color
 * strings and all named color strings are supported. In this case, an alpha
 * number value as a second argument is not supported, the RGBA form should be
 * used.
 *
 * @method color
 * @param  {Number}        gray    number specifying value between white
 *                                 and black.
 * @param  {Number}        [alpha] alpha value relative to current color range
 *                                 (default is 0-255)
 * @return {Color}              resulting color
 *
 * @example
 * let c = color(255, 204, 0); // Define color 'c'
 * fill(c); // Use color variable 'c' as fill color
 * noStroke(); // Don't draw a stroke around shapes
 * rect(30, 20, 55, 55); // Draw rectangle
 *
 * let c = color(255, 204, 0); // Define color 'c'
 * fill(c); // Use color variable 'c' as fill color
 * noStroke(); // Don't draw a stroke around shapes
 * ellipse(25, 25, 80, 80); // Draw left circle
 *
 * // Using only one value with color()
 * // generates a grayscale value.
 * c = color(65); // Update 'c' with grayscale value
 * fill(c); // Use updated 'c' as fill color
 * ellipse(75, 75, 80, 80); // Draw right circle
 *
 * // Named SVG & CSS colors may be used,
 * let c = color('magenta');
 * fill(c); // Use 'c' as fill color
 * noStroke(); // Don't draw a stroke around shapes
 * rect(20, 20, 60, 60); // Draw rectangle
 *
 * // as can hex color codes:
 * noStroke(); // Don't draw a stroke around shapes
 * let c = color('#0f0');
 * fill(c); // Use 'c' as fill color
 * rect(0, 10, 45, 80); // Draw rectangle
 *
 * c = color('#00ff00');
 * fill(c); // Use updated 'c' as fill color
 * rect(55, 10, 45, 80); // Draw rectangle
 *
 * // RGB and RGBA color strings are also supported:
 * // these all set to the same color (solid blue)
 * let c;
 * noStroke(); // Don't draw a stroke around shapes
 * c = color('rgb(0,0,255)');
 * fill(c); // Use 'c' as fill color
 * rect(10, 10, 35, 35); // Draw rectangle
 *
 * c = color('rgb(0%, 0%, 100%)');
 * fill(c); // Use updated 'c' as fill color
 * rect(55, 10, 35, 35); // Draw rectangle
 *
 * c = color('rgba(0, 0, 255, 1)');
 * fill(c); // Use updated 'c' as fill color
 * rect(10, 55, 35, 35); // Draw rectangle
 *
 * c = color('rgba(0%, 0%, 100%, 1)');
 * fill(c); // Use updated 'c' as fill color
 * rect(55, 55, 35, 35); // Draw rectangle
 *
 * // HSL color is also supported and can be specified
 * // by value
 * let c;
 * noStroke(); // Don't draw a stroke around shapes
 * c = color('hsl(160, 100%, 50%)');
 * fill(c); // Use 'c' as fill color
 * rect(0, 10, 45, 80); // Draw rectangle
 *
 * c = color('hsla(160, 100%, 50%, 0.5)');
 * fill(c); // Use updated 'c' as fill color
 * rect(55, 10, 45, 80); // Draw rectangle
 *
 * // HSB color is also supported and can be specified
 * // by value
 * let c;
 * noStroke(); // Don't draw a stroke around shapes
 * c = color('hsb(160, 100%, 50%)');
 * fill(c); // Use 'c' as fill color
 * rect(0, 10, 45, 80); // Draw rectangle
 *
 * c = color('hsba(160, 100%, 50%, 0.5)');
 * fill(c); // Use updated 'c' as fill color
 * rect(55, 10, 45, 80); // Draw rectangle
 *
 * let c; // Declare color 'c'
 * noStroke(); // Don't draw a stroke around shapes
 *
 * // If no colorMode is specified, then the
 * // default of RGB with scale of 0-255 is used.
 * c = color(50, 55, 100); // Create a color for 'c'
 * fill(c); // Use color variable 'c' as fill color
 * rect(0, 10, 45, 80); // Draw left rect
 *
 * colorMode(HSB, 100); // Use HSB with scale of 0-100
 * c = color(50, 55, 100); // Update 'c' with new color
 * fill(c); // Use updated 'c' as fill color
 * rect(55, 10, 45, 80); // Draw right rect
 */
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

/**
 * @method color
 * @param  {String}        value   a color string
 * @return {Color}
 */
/**
 * @method color
 * @param  {Number[]}      values  an array containing the red,green,blue &
 *                                 and alpha components of the color
 * @return {Color}
 */
/**
 * @method color
 * @param  {Color}     color
 * @return {Color}
 */

exports.color = function () {
	if (arguments[0] instanceof p5Color) {
		return arguments[0]; // Do nothing if argument is already a color object.
	}

	var args = arguments[0] instanceof Array ? arguments[0] : arguments;
	return new p5Color(args);
};

/**
 * Extracts the green value from a color or pixel array.
 *
 * @method green
 * @param {Color|Number[]|String} color Color object, color components,
 *                                         or CSS color
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
	return color(c)._getGreen();
};

/**
 * Extracts the hue value from a color or pixel array.
 *
 * Hue exists in both HSB and HSL. This function will return the
 * HSB-normalized hue when supplied with an HSB color object (or when supplied
 * with a pixel array while the color mode is HSB), but will default to the
 * HSL-normalized hue otherwise. (The values will only be different if the
 * maximum hue setting for each system is different.)
 *
 * @method hue
 * @param {Color|Number[]|String} color Color object, color components,
 *                                         or CSS color
 * @return {Number} the hue
 * @example
 * noStroke();
 * colorMode(HSB, 255);
 * let c = color(0, 126, 255);
 * fill(c);
 * rect(15, 20, 35, 60);
 * let value = hue(c); // Sets 'value' to "0"
 * fill(value);
 * rect(50, 20, 35, 60);
 */
exports.hue = function (c) {
	return color(c)._getHue();
};

/**
 * Blends two colors to find a third color somewhere between them. The amt
 * parameter is the amount to interpolate between the two values where 0.0
 * equal to the first color, 0.1 is very near the first color, 0.5 is halfway
 * in between, etc. An amount below 0 will be treated as 0. Likewise, amounts
 * above 1 will be capped at 1. This is different from the behavior of lerp(),
 * but necessary because otherwise numbers outside the range will produce
 * strange and unexpected colors.
 * <br><br>
 * The way that colours are interpolated depends on the current color mode.
 *
 * @method lerpColor
 * @param  {Color} c1  interpolate from this color
 * @param  {Color} c2  interpolate to this color
 * @param  {Number}       amt number between 0 and 1
 * @return {Color}     interpolated color
 * @example
 * colorMode(RGB);
 * stroke(255);
 * background(51);
 * let from = color(218, 165, 32);
 * let to = color(72, 61, 139);
 * colorMode(RGB); // Try changing to HSB.
 * let interA = lerpColor(from, to, 0.33);
 * let interB = lerpColor(from, to, 0.66);
 * fill(from);
 * rect(10, 20, 20, 60);
 * fill(interA);
 * rect(30, 20, 20, 60);
 * fill(interB);
 * rect(50, 20, 20, 60);
 * fill(to);
 * rect(70, 20, 20, 60);
 */
exports.lerpColor = function (c1, c2, amt) {
	var mode = _currentEnv._colorMode;
	var maxes = _currentEnv._colorMaxes;
	var l0, l1, l2, l3;
	var fromArray, toArray;

	if (mode === RGB) {
		fromArray = c1.levels.map(function (level) {
			return level / 255;
		});
		toArray = c2.levels.map(function (level) {
			return level / 255;
		});
	} else if (mode === HSB) {
		c1._getBrightness(); // Cache hsba so it definitely exists.
		c2._getBrightness();
		fromArray = c1.hsba;
		toArray = c2.hsba;
	} else if (mode === HSL) {
		c1._getLightness(); // Cache hsla so it definitely exists.
		c2._getLightness();
		fromArray = c1.hsla;
		toArray = c2.hsla;
	} else {
		throw new Error(mode + 'cannot be used for interpolation.');
	}

	// Prevent extrapolation.
	amt = Math.max(Math.min(amt, 1), 0);

	// Define lerp here itself if user isn't using math module.
	// Maintains the definition as found in math/calculation.js
	if (typeof this.lerp === 'undefined') {
		this.lerp = function (start, stop, amt) {
			return amt * (stop - start) + start;
		};
	}

	// Perform interpolation.
	l0 = this.lerp(fromArray[0], toArray[0], amt);
	l1 = this.lerp(fromArray[1], toArray[1], amt);
	l2 = this.lerp(fromArray[2], toArray[2], amt);
	l3 = this.lerp(fromArray[3], toArray[3], amt);

	// Scale components.
	l0 *= maxes[mode][0];
	l1 *= maxes[mode][1];
	l2 *= maxes[mode][2];
	l3 *= maxes[mode][3];

	return color(l0, l1, l2, l3);
};

/**
 * Extracts the HSL lightness value from a color or pixel array.
 *
 * @method lightness
 * @param {Color|Number[]|String} color Color object, color components,
 *                                         or CSS color
 * @return {Number} the lightness
 * @example
 * noStroke();
 * colorMode(HSL);
 * let c = color(156, 100, 50, 1);
 * fill(c);
 * rect(15, 20, 35, 60);
 * let value = lightness(c); // Sets 'value' to 50
 * fill(value);
 * rect(50, 20, 35, 60);
 */
exports.lightness = function (c) {
	return color(c)._getLightness();
};

/**
 * Extracts the red value from a color or pixel array.
 *
 * @method red
 * @param {Color|Number[]|String} color Color object, color components,
 *                                         or CSS color
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
	return color(c)._getRed();
};

/**
 * Extracts the saturation value from a color or pixel array.
 *
 * Saturation is scaled differently in HSB and HSL. This function will return
 * the HSB saturation when supplied with an HSB color object (or when supplied
 * with a pixel array while the color mode is HSB), but will default to the
 * HSL saturation otherwise.
 *
 * @method saturation
 * @param {Color|Number[]|String} color Color object, color components,
 *                                         or CSS color
 * @return {Number} the saturation value
 * @example
 * noStroke();
 * colorMode(HSB, 255);
 * let c = color(0, 126, 255);
 * fill(c);
 * rect(15, 20, 35, 60);
 * let value = saturation(c); // Sets 'value' to 126
 * fill(value);
 * rect(50, 20, 35, 60);
 */

exports.saturation = function (c) {
	return color(c)._getSaturation();
};

/**********************************************************************************************************************
 * setting
 */

/**
* The background() function sets the color used for the background of the
* p5.js canvas. The default background is transparent. This function is
* typically used within draw() to clear the display window at the beginning
* of each frame, but it can be used inside setup() to set the background on
* the first frame of animation or if the background need only be set once.
* <br><br>
* The color is either specified in terms of the RGB, HSB, or HSL color
* depending on the current colorMode. (The default color space is RGB, with
* each value in the range from 0 to 255). The alpha range by default is also 0 to 255.
* <br><br>
* If a single string argument is provided, RGB, RGBA and Hex CSS color strings
* and all named color strings are supported. In this case, an alpha number
* value as a second argument is not supported, the RGBA form should be used.
* <br><br>
* A Color object can also be provided to set the background color.
* <br><br>
* An Image can also be provided to set the background image.
*
* @method background
* @param {Color} color     any value created by the color() function
*
* @example
* // Grayscale integer value
* background(51);
*
* // R, G & B integer values
* background(255, 204, 0);
*
* // H, S & B integer values
* colorMode(HSB);
* background(255, 204, 100);
*
* // Named SVG/CSS color string
* background('red');
*
* // three-digit hexadecimal RGB notation
* background('#fae');
*
* // six-digit hexadecimal RGB notation
* background('#222222');
*
* // integer RGB notation
* background('rgb(0,255,0)');
*
* // integer RGBA notation
* background('rgba(0,255,0, 0.25)');
*
* // percentage RGB notation
* background('rgb(100%,0%,10%)');
*
* // percentage RGBA notation
* background('rgba(100%,0%,100%,0.5)');
*
* // p5 Color object
* background(color(0, 0, 255));
*/

/**
 * @method background
 * @param {String} colorstring color string, possible formats include: integer
 *                         rgb() or rgba(), percentage rgb() or rgba(),
 *                         3-digit hex, 6-digit hex
 * @param {Number} [a]         opacity of the background relative to current
 *                             color range (default is 0-255)
 */

/**
 * @method background
 * @param {Number} gray   specifies a value between white and black
 * @param {Number} [a]
 */

/**
 * @method background
 * @param {Number} v1     red or hue value (depending on the current color
 *                        mode)
 * @param {Number} v2     green or saturation value (depending on the current
 *                        color mode)
 * @param {Number} v3     blue or brightness value (depending on the current
 *                        color mode)
 * @param  {Number} [a]
 */

/**
 * @method background
 * @param  {Number[]}      values  an array containing the red,green,blue &
 *                                 and alpha components of the color
 */

/**
 * @method background
 * @param {Bitmap} bitmap     image created with loadImage() or createImage(),
 *                             to set as background
 *                             (must be same size as the sketch window)
 * @param  {Number}  [a]
 */

exports.background = function () {
	if (arguments[0] instanceof Bitmap) {
		arguments[0].Draw(0, 0);
	} else {
		if (arguments[0] instanceof p5Color) {
			_background = arguments[0].toAllegro()
		} else {
			_background = new p5Color(arguments).toAllegro();
		}
		FilledBox(0, 0, SizeX(), SizeY(), _background);
	}
};

/**
 * Clears the pixels within a buffer. This function only clears the canvas.
 * It will not clear objects created by createX() methods such as
 * createVideo() or createDiv().
 * Unlike the main graphics context, pixels in additional graphics areas created
 * with createGraphics() can be entirely
 * or partially transparent. This function clears everything to make all of
 * the pixels 100% transparent.
 *
 * @method clear
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
};

/**
 * colorMode() changes the way p5.js interprets color data. By default, the
 * parameters for fill(), stroke(), background(), and color() are defined by
 * values between 0 and 255 using the RGB color model. This is equivalent to
 * setting colorMode(RGB, 255). Setting colorMode(HSB) lets you use the HSB
 * system instead. By default, this is colorMode(HSB, 360, 100, 100, 1). You
 * can also use HSL.
 * <br><br>
 * Note: existing color objects remember the mode that they were created in,
 * so you can change modes as you like without affecting their appearance.
 *
 *
 * @method colorMode
 * @param {Constant} mode   either RGB, HSB or HSL, corresponding to
 *                          Red/Green/Blue and Hue/Saturation/Brightness
 *                          (or Lightness)
 * @param {Number}  [max]  range for all values
 *
 * @example
 * noStroke();
 * colorMode(RGB, 100);
 * for (let i = 0; i < 100; i++) {
 *   for (let j = 0; j < 100; j++) {
 *     stroke(i, j, 0);
 *     point(i, j);
 *   }
 * }
 *
 * noStroke();
 * colorMode(HSB, 100);
 * for (let i = 0; i < 100; i++) {
 *   for (let j = 0; j < 100; j++) {
 *     stroke(i, j, 100);
 *     point(i, j);
 *   }
 * }
 *
 * colorMode(RGB, 255);
 * let c = color(127, 255, 0);
 *
 * colorMode(RGB, 1);
 * let myColor = c._getRed();
 * text(myColor, 10, 10, 80, 80);
 *
 * noFill();
 * colorMode(RGB, 255, 255, 255, 1);
 * background(255);
 *
 * strokeWeight(4);
 * stroke(255, 0, 10, 0.3);
 * ellipse(40, 40, 50, 50);
 * ellipse(50, 50, 40, 40);
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
 */
exports.colorMode = function (mode, max1, max2, max3, maxA) {
	if (
		mode === RGB ||
		mode === HSB ||
		mode === HSL
	) {
		// Set color mode.
		_currentEnv._colorMode = mode;

		// Set color maxes.
		var maxes = _currentEnv._colorMaxes[mode];
		if (arguments.length === 2) {
			maxes[0] = max1; // Red
			maxes[1] = max1; // Green
			maxes[2] = max1; // Blue
			maxes[3] = max1; // Alpha
		} else if (arguments.length === 4) {
			maxes[0] = max1; // Red
			maxes[1] = max2; // Green
			maxes[2] = max3; // Blue
		} else if (arguments.length === 5) {
			maxes[0] = max1; // Red
			maxes[1] = max2; // Green
			maxes[2] = max3; // Blue
			maxes[3] = maxA; // Alpha
		}
	}
};

/**
 * Sets the color used to fill shapes. For example, if you run
 * fill(204, 102, 0), all subsequent shapes will be filled with orange. This
 * color is either specified in terms of the RGB or HSB color depending on
 * the current colorMode(). (The default color space is RGB, with each value
 * in the range from 0 to 255). The alpha range by default is also 0 to 255.
 * <br><br>
 * If a single string argument is provided, RGB, RGBA and Hex CSS color strings
 * and all named color strings are supported. In this case, an alpha number
 * value as a second argument is not supported, the RGBA form should be used.
 * <br><br>
 * A p5 Color object can also be provided to set the fill color.
 *
 * @method fill
 * @param  {Number}        v1      red or hue value relative to
 *                                 the current color range
 * @param  {Number}        v2      green or saturation value
 *                                 relative to the current color range
 * @param  {Number}        v3      blue or brightness value
 *                                 relative to the current color range
 * @param  {Number}        [alpha]
 * @example
 * // Grayscale integer value
 * fill(51);
 * rect(20, 20, 60, 60);
 *
 * // R, G & B integer values
 * fill(255, 204, 0);
 * rect(20, 20, 60, 60);
 *
 * // H, S & B integer values
 * colorMode(HSB);
 * fill(255, 204, 100);
 * rect(20, 20, 60, 60);
 *
 * // Named SVG/CSS color string
 * fill('red');
 * rect(20, 20, 60, 60);
 *
 * // three-digit hexadecimal RGB notation
 * fill('#fae');
 * rect(20, 20, 60, 60);
 *
 * // six-digit hexadecimal RGB notation
 * fill('#222222');
 * rect(20, 20, 60, 60);
 *
 * // integer RGB notation
 * fill('rgb(0,255,0)');
 * rect(20, 20, 60, 60);
 *
 * // integer RGBA notation
 * fill('rgba(0,255,0, 0.25)');
 * rect(20, 20, 60, 60);
 *
 * // percentage RGB notation
 * fill('rgb(100%,0%,10%)');
 * rect(20, 20, 60, 60);
 *
 * // percentage RGBA notation
 * fill('rgba(100%,0%,100%,0.5)');
 * rect(20, 20, 60, 60);
 *
 * // p5 Color object
 * fill(color(0, 0, 255));
 * rect(20, 20, 60, 60);
 */

/**
 * @method fill
 * @param  {String}        value   a color string
 */

/**
 * @method fill
 * @param  {Number}        gray   a gray value
 * @param  {Number}        [alpha]
 */

/**
 * @method fill
 * @param  {Number[]}      values  an array containing the red,green,blue &
 *                                 and alpha components of the color
 */

/**
 * @method fill
 * @param  {Color}      color   the fill color
 */
exports.fill = function () {
	if (arguments[0] instanceof p5Color) {
		_currentEnv._fill = arguments[0].toAllegro();
	} else {
		_currentEnv._fill = new p5Color(arguments).toAllegro();
	}
};

/**
 * Disables filling geometry. If both noStroke() and noFill() are called,
 * nothing will be drawn to the screen.
 *
 * @method noFill
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
exports.noFill = function () {
	_currentEnv._fill = NO_COLOR;
};

/**
 * Disables drawing the stroke (outline). If both noStroke() and noFill()
 * are called, nothing will be drawn to the screen.
 *
 * @method noStroke
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
exports.noStroke = function () {
	_currentEnv._stroke = NO_COLOR;
};

/**
 * Sets the color used to draw lines and borders around shapes. This color
 * is either specified in terms of the RGB or HSB color depending on the
 * current colorMode() (the default color space is RGB, with each value in
 * the range from 0 to 255). The alpha range by default is also 0 to 255.
 * <br><br>
 * If a single string argument is provided, RGB, RGBA and Hex CSS color
 * strings and all named color strings are supported. In this case, an alpha
 * number value as a second argument is not supported, the RGBA form should be
 * used.
 * <br><br>
 * A p5 Color object can also be provided to set the stroke color.
 *
 *
 * @method stroke
 * @param  {Number}        v1      red or hue value relative to
 *                                 the current color range
 * @param  {Number}        v2      green or saturation value
 *                                 relative to the current color range
 * @param  {Number}        v3      blue or brightness value
 *                                 relative to the current color range
 * @param  {Number}        [alpha]
 *
 * @example
 * // Grayscale integer value
 * strokeWeight(4);
 * stroke(51);
 * rect(20, 20, 60, 60);
 *
 * // R, G & B integer values
 * stroke(255, 204, 0);
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 *
 * // H, S & B integer values
 * colorMode(HSB);
 * strokeWeight(4);
 * stroke(255, 204, 100);
 * rect(20, 20, 60, 60);
 *
 * // Named SVG/CSS color string
 * stroke('red');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 *
 * // three-digit hexadecimal RGB notation
 * stroke('#fae');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 *
 * // six-digit hexadecimal RGB notation
 * stroke('#222222');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 *
 * // integer RGB notation
 * stroke('rgb(0,255,0)');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 *
 * // integer RGBA notation
 * stroke('rgba(0,255,0,0.25)');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 *
 * // percentage RGB notation
 * stroke('rgb(100%,0%,10%)');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 *
 * // percentage RGBA notation
 * stroke('rgba(100%,0%,100%,0.5)');
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 *
 * // p5 Color object
 * stroke(color(0, 0, 255));
 * strokeWeight(4);
 * rect(20, 20, 60, 60);
 */

/**
 * @method stroke
 * @param  {String}        value   a color string
 */

/**
 * @method stroke
 * @param  {Number}        gray   a gray value
 * @param  {Number}        [alpha]
 */

/**
 * @method stroke
 * @param  {Number[]}      values  an array containing the red,green,blue &
 *                                 and alpha components of the color
 */

/**
 * @method stroke
 * @param  {Color}      color   the stroke color
 */
exports.stroke = function () {
	if (arguments[0] instanceof p5Color) {
		_currentEnv._stroke = arguments[0].toAllegro()
	} else {
		_currentEnv._stroke = new p5Color(arguments).toAllegro();
	}
};
