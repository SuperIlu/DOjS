/*
MIT License

Copyright (c) 2019-2020 Andre Seidelt <superilu@yahoo.com>

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

var lastX = 0;
var lastY = 10;
var baseline = 20;
var maxX = 0;

function setup() {
	createCanvas(windowWidth, windowHeight);
	background(30);
	colorMode(HSB);
}

function draw() {
	var x = lastX + random(-10, 20);
	var y = baseline + random(-10, 10);

	if (random(20) > 17) {
		lastX = maxX + random(20, 30);
		x = lastX;
		y = baseline;
		stroke(lastX % 255, 255, 255);
	}
	line(lastX, lastY, x, y);
	lastX = x;
	lastY = y;
	maxX = max(maxX, x);

	if (lastX >= width) {
		baseline += 30;
		lastX = 0;
		maxX = 0;
		lastY = baseline;
	}

	if (baseline >= height) {
		clear();
		lastX = 0;
		maxX = 0;
		lastY = 10;
		baseline = 20;
	}
}
