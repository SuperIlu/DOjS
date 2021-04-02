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

var R_SIZE = 20;
var SPACING = 4;

var anim = [];

function setup() {
	createCanvas(windowWidth, windowHeight);

	noFill();
	stroke(255);
	angleMode(DEGREES);

	for (var l = R_SIZE; l < height; l += 2 * R_SIZE) {
		for (var r = R_SIZE; r < width; r += 2 * R_SIZE) {
			anim.push(random(0, 360));
		}
	}
}

function draw() {
	colorMode(RGB);
	background(32);

	var idx = 0;
	for (var l = R_SIZE; l < height; l += 2 * R_SIZE) {
		for (var r = R_SIZE; r < width; r += 2 * R_SIZE) {
			stroke(255);
			drawRhombus(r, l, 2 * R_SIZE - SPACING);

			colorMode(HSB, 1);
			stroke(sin(anim[idx]), 255, 255);
			drawRhombus(r, l, 2 * (R_SIZE - SPACING) * sin(anim[idx]));

			anim[idx] += 1;
			idx++;
		}
	}
}

function drawRhombus(x, y, size) {
	var size_2 = size / 2;
	beginShape();
	vertex(x, y - size_2);
	vertex(x + size_2, y);
	vertex(x, y + size_2);
	vertex(x - size_2, y);
	endShape(CLOSE);
}