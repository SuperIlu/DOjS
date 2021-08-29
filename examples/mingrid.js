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

var C_LEFT = 0;
var C_RIGHT = 1;
var C_UP = 2;
var C_DOWN = 3;

var N_FRAMES = 90;
var MAX_SIZE = 20;
var NUM_X = 16;
var NUM_Y = 8;

var vectors = [];
var grid = [];

function setup() {
	createCanvas(windowWidth, windowHeight);
	//createCanvas(800, 800);
	colorMode(RGB, 1);

	var xStep = width / NUM_X;
	var yStep = width / NUM_Y;
	var lastRow = null;
	for (var y = yStep / 2; y < height; y += xStep) {
		var currentRow = [];
		for (var x = xStep / 2; x < width; x += xStep) {
			var c = new Cell(x, y, random(0.5, 0.8));
			grid.push(c);
			if (currentRow.length > 0) {
				c.add(C_LEFT, currentRow[currentRow.length - 1]);
			}
			currentRow.push(c);
			if (lastRow) {
				c.add(C_UP, lastRow[currentRow.length - 1]);
			}
		}
		lastRow = currentRow;
	}
}

function draw() {
	var i;

	background(0);

	for (i = 0; i < grid.length; i++) {
		grid[i].drawGrid();
	}

	for (i = 0; i < vectors.length; i++) {
		vectors[i].draw();
	}

	for (i = 0; i < grid.length; i++) {
		grid[i].drawCell();
	}

	if (vectors.length > 0) {
		// step all vectors, only keep vectors that are not done
		var newVectors = [];
		for (i = 0; i < vectors.length; i++) {
			if (vectors[i].run()) {
				newVectors.push(vectors[i]);
			}
		}
		vectors = newVectors;
	} else {
		// no vectors left, step cells
		for (i = 0; i < grid.length; i++) {
			grid[i].run();
		}
	}
}

function Cell(x, y, th) {
	this.threshold = th;
	this.x = x;
	this.y = y;
	this.value = random(1);
	this.neighbors = [null, null, null, null];
}

Cell.prototype.add = function (dir, cell) {
	switch (dir) {
		case C_LEFT:
			cell.neighbors[C_RIGHT] = this;
			break;
		case C_RIGHT:
			cell.neighbors[C_LEFT] = this;
			break;
		case C_UP:
			cell.neighbors[C_DOWN] = this;
			break;
		case C_DOWN:
			cell.neighbors[C_UP] = this;
			break;
		default:
			throw "unknown direction";
	}
	this.neighbors[dir] = cell;
}

Cell.prototype.runMulti = function () {
	if (this.value > this.threshold) {
		var num = 0;
		for (var i = 0; i < this.neighbors.length; i++) {
			if (this.neighbors[i]) {
				num++;
			}
		}
		var transfer = this.value / 4;
		this.value -= transfer;
		for (i = 0; i < this.neighbors.length; i++) {
			if (this.neighbors[i]) {
				vectors.push(new Beam(this, this.neighbors[i], transfer / num));
			}
		}
	}
}

Cell.prototype.run = function () {
	if (this.value > this.threshold) {
		var minVal = 1;
		var minNeighbor = null;
		for (var i = 0; i < this.neighbors.length; i++) {
			if (this.neighbors[i] && this.neighbors[i].value < minVal) {
				minVal = this.neighbors[i].value;
				minNeighbor = this.neighbors[i];
			}
		}
		if (minNeighbor) {
			vectors.push(new Beam(this, minNeighbor, this.value / 2));
			this.value /= 2;
		}
	}
}

Cell.prototype.runMax = function () {
	if (this.value > this.threshold) {
		var maxVal = 0;
		var maxNeighbor = null;
		for (var i = 0; i < this.neighbors.length; i++) {
			if (this.neighbors[i] && this.neighbors[i].value > maxVal) {
				maxVal = this.neighbors[i].value;
				maxNeighbor = this.neighbors[i];
			}
		}
		if (maxNeighbor) {
			vectors.push(new Beam(this, maxNeighbor, this.value / 2));
			this.value /= 2;
		}
	}
}

// from https://easings.net/
function easeInOutCubic(x) {
	return x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
}

Cell.prototype.drawGrid = function () {
	// draw grid to all other cells
	noFill();
	stroke(0.2);
	for (var i = 0; i < this.neighbors.length; i++) {
		if (this.neighbors[i]) {
			line(this.x, this.y, this.neighbors[i].x, this.neighbors[i].y);
		}
	}
}

Cell.prototype.drawCell = function () {
	// draw cell
	stroke(0, 0, 1);
	fill(this.value);
	circle(this.x, this.y, this.value * MAX_SIZE);
}

function Beam(f, t, v) {
	this.from = f;
	this.to = t;
	this.value = v;
	this.tx = v / (N_FRAMES / 3);
	this.pos = p5.Vector.sub(createVector(this.to.x, this.to.y), createVector(this.from.x, this.from.y));
	this.count = 0;
	this.percent = 0;
}

Beam.prototype.run = function () {
	var percent = this.count / N_FRAMES;
	this.percent = easeInOutCubic(percent);
	if (this.percent < 1) {
		this.count++;
		return true;
	} else if (this.value > 0) {
		this.to.value += this.tx;
		this.value -= this.tx;
		return true;
	} else {
		return false;
	}
}

Beam.prototype.draw = function () {
	var x = this.from.x;
	var y = this.from.y;

	x += this.pos.x * this.percent;
	y += this.pos.y * this.percent;

	noStroke();
	fill(0.3, 0.3, 1);
	if (this.pos.x) {
		ellipse(x, y, 6 + this.value * MAX_SIZE, 3);
	} else {
		ellipse(x, y, 3, 6 + this.value * MAX_SIZE);
	}
}