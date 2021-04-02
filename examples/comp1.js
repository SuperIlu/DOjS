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

var CELL_WIDTH = 10;
var CELL_HEIGHT = 20;
var CELL_SPACING = 2;

var C_WIDTH = CELL_WIDTH + CELL_SPACING;
var C_HEIGHT = CELL_HEIGHT + CELL_SPACING;

var LINE_WIDTH = 1;

var cellLines = [];

function setup() {
	createCanvas(windowWidth, windowHeight);
	noFill();
	stroke(255);
	strokeWeight(LINE_WIDTH);
	frameRate(15);
}

function draw() {
	background(32);

	for (var l = 0; l < cellLines.length; l++) {
		for (var c = 0; c < cellLines[l].length; c++) {
			drawCell(c * C_WIDTH, l * C_HEIGHT, cellLines[l][c]);
		}
	}

	// create new lines and append
	cellLines.push(createLine());

	// delete first element until everything fits on the screen again
	var maxCells = height / C_HEIGHT;
	while (cellLines.length > maxCells) {
		cellLines.splice(0, 1);
	}
}

function drawCell(x, y, c) {
	for (var i = 0; i < c.num; i++) {
		line(x, y + i * c.spacing, x + CELL_WIDTH, y + i * c.spacing);
	}
}

function createLine() {
	var numCells = width / C_WIDTH;

	var ret = [];
	for (var i = 0; i < numCells; i++) {
		ret.push(createCell());
	}
	return ret;
}

function createCell() {
	var numLines = random(4);
	var spacing = random(CELL_HEIGHT / numLines / 2);

	return {
		'num': numLines,
		'spacing': spacing
	};
}
