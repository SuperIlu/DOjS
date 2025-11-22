/*
MIT License

Copyright (c) 2019-2025 Andre Seidelt <superilu@yahoo.com>

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

var NUM_SECT = 3;
var MAX_DEPTH = 4;

var currentDepth = 0;

var queue = [];

function setup() {
	createCanvas(
		windowWidth < windowHeight ? windowWidth : windowHeight,
		windowWidth < windowHeight ? windowWidth : windowHeight
	);

	queue.push(
		function () {
			iterate(0, 0, width, height, 0, random(360));
		}
	);
}

function draw() {
	if (queue.length > 0) {
		shuffle(queue, true);
		var f = queue.shift();
		f();
	}
}

function iterate(xStart, yStart, w, h, i, c) {
	// draw it
	var stepX = w / NUM_SECT;
	var stepY = h / NUM_SECT;
	rectMode(CORNER);
	colorMode(HSB);
	for (var y = 0; y < NUM_SECT; y++) {
		for (var x = 0; x < NUM_SECT; x++) {
			fill(c, 100, 100);
			noStroke();
			rect(xStart + x * stepX, yStart + y * stepY, stepX, stepY);
			if (i < MAX_DEPTH) {
				queue.push(makeClosure(xStart + x * stepX, yStart + y * stepY, stepX, stepY, i + 1, c + random(90)));
			}
		}
	}
}

function makeClosure(newX, newY, newW, newH, newI, newC) {
	return function () {
		iterate(newX, newY, newW, newH, newI, newC % 360);
	};
}
