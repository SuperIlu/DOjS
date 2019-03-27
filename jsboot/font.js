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
 * constants for text direction and alignment
 * @namespace FONT
 * @property {*} Direction drawing directions
 * @property {*} Align text alignment
 * @property {*} Direction.DEFAULT
 * @property {*} Direction.RIGHT normal
 * @property {*} Direction.DOWN downward
 * @property {*} Direction.LEFT upside down, right to left
 * @property {*} Direction.UP upward
 * @property {*} Align.DEFAULT
 * @property {*} Align.LEFT X only
 * @property {*} Align.TOP Y only
 * @property {*} Align.CENTER X, Y
 * @property {*} Align.RIGHT X only
 * @property {*} Align.BOTTOM Y only
 * @property {*} Align.BASELINE Y only
 */
FONT = {
	Direction: {
		DEFAULT: 0,
		RIGHT: 0,
		DOWN: 1,
		LEFT: 2,
		UP: 3
	},
	Align: {
		DEFAULT: 0,
		LEFT: 0,
		TOP: 0,
		CENTER: 1,
		RIGHT: 2,
		BOTTOM: 2,
		BASELINE: 3
	}
};