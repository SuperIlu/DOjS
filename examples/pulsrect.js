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
if (navigator.appName === "DOjS") {
	Include("p5");
}

var DECAY = 0.2;
var MAX_RECTS = 20;
var MIN_SIZE = 6;
var MAX_SIZE = 20;

var allRects = [];

function setup() {
	createCanvas(windowWidth, windowHeight);
}

function draw() {
	var i;

	colorMode(RGB);
	background(32);

	colorMode(HSB, 360, 100, 100, 1);
	rectMode(CORNERS);
	for (i = 0; i < allRects.length; i++) {
		allRects[i].draw();
	}
	for (i = 0; i < allRects.length; i++) {
		allRects[i].update();
	}
	if (allRects.length < MAX_RECTS) {
		allRects.push(new Rect());
	}
}

function Rect() {
	this.x = random(windowWidth);
	this.y = random(windowHeight);
	this.size = random(MIN_SIZE, MAX_SIZE) * 3;
	this.currentSize = this.size;
	this.color = random(360);
	this.mode = random([0, 1, 2, 3]);
};
Rect.prototype.draw = function () {
	noStroke();
	fill(this.color, 60, 60, this.currentSize / this.size / 2);
	rect(
		this.x - this.size, this.y - this.size,
		this.x + this.size, this.y + this.size
	);

	strokeWeight(this.currentSize / 10);
	stroke(this.color, 100, 100, 1);
	var currentSize_2 = this.currentSize * 2;
	switch (this.mode) {
		case 0:
			line(
				this.x - this.size, this.y - this.currentSize,
				this.x - this.size, this.y + this.currentSize
			);
			line(
				this.x + this.size, this.y - this.currentSize,
				this.x + this.size, this.y + this.currentSize
			);

			line(
				this.x - this.currentSize, this.y - this.size,
				this.x + this.currentSize, this.y - this.size
			);
			line(
				this.x - this.currentSize, this.y + this.size,
				this.x + this.currentSize, this.y + this.size
			);
			break;

		case 1:
			line(
				this.x - this.size, this.y - this.size,
				this.x - this.size, this.y - this.size + currentSize_2
			);
			line(
				this.x - this.size, this.y - this.size,
				this.x - this.size + currentSize_2, this.y - this.size
			);

			line(
				this.x + this.size, this.y + this.size,
				this.x + this.size, this.y + this.size - currentSize_2
			);
			line(
				this.x + this.size, this.y + this.size,
				this.x + this.size - currentSize_2, this.y + this.size
			);
			break;

		case 2:
			line(
				this.x + this.size, this.y - this.size,
				this.x + this.size, this.y - this.size + currentSize_2
			);
			line(
				this.x - this.size, this.y + this.size,
				this.x - this.size + currentSize_2, this.y + this.size
			);

			line(
				this.x - this.size, this.y + this.size,
				this.x - this.size, this.y + this.size - currentSize_2
			);
			line(
				this.x + this.size, this.y - this.size,
				this.x + this.size - currentSize_2, this.y - this.size
			);
			break;

		case 3:
			line(
				this.x - this.size, this.y - this.size,
				this.x - this.size, this.y - this.size + this.currentSize
			);
			line(
				this.x - this.size, this.y - this.size,
				this.x - this.size + this.currentSize, this.y - this.size
			);

			line(
				this.x + this.size, this.y + this.size,
				this.x + this.size, this.y + this.size - this.currentSize
			);
			line(
				this.x + this.size, this.y + this.size,
				this.x + this.size - this.currentSize, this.y + this.size
			);

			line(
				this.x + this.size, this.y - this.size,
				this.x + this.size, this.y - this.size + this.currentSize
			);
			line(
				this.x - this.size, this.y + this.size,
				this.x - this.size + this.currentSize, this.y + this.size
			);

			line(
				this.x - this.size, this.y + this.size,
				this.x - this.size, this.y + this.size - this.currentSize
			);
			line(
				this.x + this.size, this.y - this.size,
				this.x + this.size - this.currentSize, this.y - this.size
			);
			break;

	}
};
Rect.prototype.update = function () {
	this.currentSize -= DECAY;
	if (this.currentSize <= 1) {
		var index = allRects.indexOf(this);
		if (index > -1) { // only splice array when item is found
			allRects.splice(index, 1); // 2nd parameter means remove one item only
		}
	}
};