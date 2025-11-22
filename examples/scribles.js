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

var MAX_BLOCKS = 7;

function setup() {
	createCanvas(
		windowWidth < windowHeight ? windowWidth : windowHeight,
		windowWidth < windowHeight ? windowWidth : windowHeight
	);
	reDraw();
	noLoop();
}

function draw() {
	noLoop();
}

function reDraw() {
	var blockSize = width / MAX_BLOCKS;
	colorMode(RGB);
	background(32);

	colorMode(HSB);
	for (var y = 0; y < MAX_BLOCKS; y++) {
		for (var x = 0; x < MAX_BLOCKS; x++) {
			makeBlock(blockSize, x, y);
		}
	}
}

function makeBlock(blockSize, x, y) {
	if (int(random(7)) < 4) {
		return;
	}

	var xStart = blockSize * x + random(-blockSize / 4, blockSize / 4);
	var yStart = blockSize * y + random(-blockSize / 4, blockSize / 4);

	var h = random(360);
	var b = 100;
	var a = random(0.4, 0.8);

	var num = random(blockSize * 0.15, blockSize * 0.65);
	var stepX = blockSize / num;

	for (var i = 0; i < num; i++) {
		var topOffset = random(-stepX * 1.5, stepX * 1.5);
		var bottomOffset = random(-stepX * 1.5, stepX * 1.5);

		var xPos = (i * stepX);
		var shorten = sin(map(min(xPos, blockSize - xPos), 0, blockSize / 2, 0, PI / 2)) * 15;

		var s = random(20, 100);
		stroke(h, s, b, a);
		strokeWeight(random(1, 2));
		line(xStart + xPos + topOffset, yStart - shorten, xStart + xPos + bottomOffset, yStart + (blockSize - 1) + shorten);
	}
}

function mouseClicked() {
	reDraw();
}
