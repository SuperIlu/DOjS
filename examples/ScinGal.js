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

function setup() {
	createCanvas(windowWidth, windowHeight);
	angleMode(DEGREES);

	frameRate(10);
	noFill();

	w2 = width / 2;
	h2 = height / 2;
	background(0);
}

function draw() {
	colorMode(RGB);
	background(0, 8);
	drawGalaxy(
		random(width),
		random(height),
		random(width / 20, width / 40),
		random(1.001, 1.002)
	);
}

// c := center point
// l := size
// d := shrink factor
function drawGalaxy(cX, cY, l, d) {
	var angle = random(360);
	while (l > 1) {
		var r = rotatePoint(angle, cX, cY, cX + l, cY);

		colorMode(HSB);
		stroke(angle % 360, 100, 100);
		point(r[0], r[1]);

		angle++;
		l /= d;
	}
}

// c := center for rotation
// p := point to rotate
// a := angle
function rotatePoint(a, cX, cY, pX, pY) {
	var x = pX - cX;
	var y = pY - cY;

	var xR = x * cos(a) - y * sin(a);
	var yR = y * cos(a) + x * sin(a);

	return [xR + cX, yR + cY];
}
