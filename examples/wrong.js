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

/*
This is a sketch by @dec_hl aka SuperIlu for Raphaels weekly coding challenge: "RIGHT ANGLES"
see https://linktr.ee/sableraph for Raphael and https://twitter.com/dec_hl for Ilu
*/

Include('p5');

var LINE_LENGTH = 30;
var NOISE_FACTOR = 0.00005;

function setup() {
	createCanvas(windowWidth, windowHeight);
	angleMode(DEGREES);
	colorMode(HSB);
}

var i = 0;

function draw() {
	background(0);

	var t = millis() * NOISE_FACTOR;
	for (var y = 0; y < height; y += LINE_LENGTH) {
		for (var x = 0; x < width; x += LINE_LENGTH) {
			drawAngle(x, y, LINE_LENGTH, i + x + y, 360 * noise(x, y, t));
		}
	}
	i++;
}

function drawAngle(x, y, l, f, s) {
	f = f % 360;
	s = s % 360;

	var angleBetween = abs(f - s) % 90;
	var b = 100 - 2 * min(angleBetween % 90, 90 - (angleBetween % 180));
	strokeWeight(b / 100);
	push();
	translate(x, y);
	rotate(f);
	stroke(f, 100, b);
	line(0, 0, l, 0);
	rotate(s);
	stroke(f + s, 100, b);
	line(0, 0, l, 0);
	pop();
}