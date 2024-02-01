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

HSBColor() was derived from the p5.js source code at
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
 * Color definition.
 * @module color */

/**
 * EGA Color definition.
 * @property {Color} BLACK EGA color.
 * @property {Color} BLUE EGA color.
 * @property {Color} GREEN EGA color.
 * @property {Color} CYAN EGA color.
 * @property {Color} RED EGA color.
 * @property {Color} MAGENTA EGA color.
 * @property {Color} BROWN EGA color.
 * @property {Color} LIGHT_GRAY EGA color.
 * @property {Color} LIGHT_GREY EGA color.
 * @property {Color} DARK_GRAY EGA color.
 * @property {Color} DARK_GREY EGA color.
 * @property {Color} LIGHT_BLUE EGA color.
 * @property {Color} LIGHT_GREEN EGA color.
 * @property {Color} LIGHT_CYAN EGA color.
 * @property {Color} LIGHT_RED EGA color.
 * @property {Color} LIGHT_MAGENTA EGA color.
 * @property {Color} YELLOW EGA color.
 * @property {Color} WHITE EGA color.
 */
var EGA = {
	BLACK: Color(0, 0, 0, 255),
	BLUE: Color(0, 0, 170, 255),
	GREEN: Color(0, 170, 0, 255),
	CYAN: Color(0, 170, 170, 255),
	RED: Color(170, 0, 0, 255),
	MAGENTA: Color(170, 0, 170, 255),
	BROWN: Color(170, 85, 0, 255),
	LIGHT_GRAY: Color(170, 170, 170, 255),
	DARK_GRAY: Color(85, 85, 85, 255),
	LIGHT_GREY: Color(170, 170, 170, 255),
	DARK_GREY: Color(85, 85, 85, 255),
	LIGHT_BLUE: Color(85, 85, 255, 255),
	LIGHT_GREEN: Color(85, 255, 85, 255),
	LIGHT_CYAN: Color(85, 255, 255, 255),
	LIGHT_RED: Color(255, 85, 85, 255),
	LIGHT_MAGENTA: Color(255, 85, 255, 255),
	YELLOW: Color(255, 255, 85, 255),
	WHITE: Color(255, 255, 255, 254)
};

/**
* @property {Color} NO_COLOR the transparent Color.
*/
var NO_COLOR = -1;

/**
 * blend mode definitions for TransparencyEnabled()
 * 
 * @property {number} REPLACE no blending at all
 * @property {number} ALPHA alpha blending
 * @property {number} ADD add with saturation
 * @property {number} DARKEST min() with alpha
 * @property {number} LIGHTEST max() with alpha
 * @property {number} DIFFERENCE difference always yielding a positive value
 * @property {number} EXCLUSION like difference but less contrast
 * @property {number} MULTIPLY multiplication
 * @property {number} SCREEN inverted multiplication
 * @property {number} OVERLAY A combination of multiply and screen
 * @property {number} HARD_LIGHT inverted OVERLAY
 * @property {number} DOGE divides the bottom layer by the inverted top layer
 * @property {number} BURN inverted doge
 */
var BLEND = {
	REPLACE: 0,
	ALPHA: 1,
	ADD: 2,
	DARKEST: 3,
	LIGHTEST: 4,
	DIFFERENCE: 5,
	EXCLUSION: 6,
	MULTIPLY: 7,
	SCREEN: 8,
	OVERLAY: 9,
	HARD_LIGHT: 10,
	DOGE: 11,
	BURN: 12,
};

/**
 * Create Color() from HSB[A].
 * 
 * @param {number} h the hue [0..255].
 * @param {number} s the saturation [0..255].
 * @param {number} b the brightness [0..255].
 * @param {number} a the alpha value [0..255].
 */
function HSBColor(h, s, b, a) {
	var hue = h * 6 / 255; // We will split hue into 6 sectors.
	var sat = s / 255;
	var val = b / 255;
	a = a || 255;

	var RGBA = [0, 0, 0, 255];

	if (sat === 0) {
		RGBA = [val, val, val, a]; // Return early if grayscale.
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
		RGBA = [red, green, blue, a];
	}

	return Color(RGBA[0] * 255, RGBA[1] * 255, RGBA[2] * 255, RGBA[3]);
};
