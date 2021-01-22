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
*/
Include('p5');
/*
 * a glyph consists of several lines connecting the following point matrix
 * 1 2 3
 * 4 5 6
 * 7 8 9
 * A B C
 * each Glyph has an entry and exit point in the first or last column respectively.
 */

var glyphs = [];
var point_distance = 5;
var x_start = 10;
var x_adv = point_distance * 4;
var y_adv = point_distance * 5;
var words = [];
var xPos = x_start;
var yPos = x_start;

function setup() {
	createCanvas(windowWidth, windowHeight);
	background(20);
	generateGlyphs();
	generateWords();
	frameRate(5);
	stroke(random(64, 255), random(64, 255), random(64, 255));
}

function draw() {
	var w = int(random(words.length));
	drawWord(w);
	if (yPos > height) {
		yPos = x_start;
		xPos = x_start;
		clear();
		stroke(random(64, 255), random(64, 255), random(64, 255));
	}
}

function drawWord(w) {
	var w = words[w];
	if (w.length * point_distance * 4 + xPos > width) {
		xPos = x_start;
		yPos += y_adv;
	}
	var last = null;
	for (var i = 0; i < w.length; i++) {
		last = drawGlyph(w[i], xPos, yPos, last);
		xPos += x_adv;
	}
	xPos += x_adv;
}

function drawGlyph(c, x, y, prev) {
	var g = glyphs[c];
	var lastX = null;
	var lastY = null;
	if (prev) {
		var lastX = prev[0];
		var lastY = prev[1];
	}
	for (var p = 0; p < g.glyph.length; p++) {
		var coords = toCoords(g.glyph[p], x, y);
		var curX = coords[0];
		var curY = coords[1];

		if (lastX != null && lastY != null) {
			line(lastX, lastY, curX, curY);
		}
		lastX = curX;
		lastY = curY;
	}
	return [lastX, lastY];
}

function toCoords(m, curX, curY) {
	switch (m) {
		// first line
		case 0:
			curX += 0 * point_distance;
			curY += 0 * point_distance;
			break;
		case 1:
			curX += 1 * point_distance;
			curY += 0 * point_distance;
			break;
		case 2:
			curX += 1 * point_distance;
			curY += 0 * point_distance;
			break;

		// second line
		case 3:
			curX += 0 * point_distance;
			curY += 1 * point_distance;
			break;
		case 4:
			curX += 1 * point_distance;
			curY += 1 * point_distance;
			break;
		case 5:
			curX += 2 * point_distance;
			curY += 1 * point_distance;
			break;

		// third line
		case 6:
			curX += 0 * point_distance;
			curY += 2 * point_distance;
			break;
		case 7:
			curX += 1 * point_distance;
			curY += 2 * point_distance;
			break;
		case 8:
			curX += 2 * point_distance;
			curY += 2 * point_distance;
			break;

		// fourth line
		case 9:
			curX += 0 * point_distance;
			curY += 3 * point_distance;
			break;
		case 10:
			curX += 1 * point_distance;
			curY += 3 * point_distance;
			break;
		case 11:
			curX += 2 * point_distance;
			curY += 3 * point_distance;
			break;
	}
	return [curX, curY];
}

function generateGlyphs() {
	var num_glyphs = random(20, 40);
	for (var i = 0; i < num_glyphs; i++) {
		var num_points = int(random(3, 8));
		var l = [0];
		for (var p = 0; p < num_points; p++) {
			l.push(int(random(12)));
		}
		l.push(11);
		glyphs.push({
			entry: 0,
			exit: 11,
			glyph: l
		});
	}
}

function generateWords() {
	var num_words = int(random(30, 100));
	for (var i = 0; i < num_words; i++) {
		var word_length = int(random(3, 13));
		var w = [];
		for (var j = 0; j < word_length; j++) {
			w.push(int(random(0, glyphs.length)));
		}
		words.push(w);
	}
}
