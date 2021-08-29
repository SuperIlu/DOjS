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
LoadLibrary("noise");

var STEP_SIZE = 10;
var NOISE_SCALE = 0.005;

var fcount = 0;

function setup() {
	createCanvas(windowWidth, windowHeight);
	colorMode(HSB);
	ellipseMode(CENTER);
	noFill();
}

function draw() {
	var SSIZE2 = 2 * STEP_SIZE;
	var SSCALE = STEP_SIZE * NOISE_SCALE;

	background(0);
	var ny = 0;
	for (var x = 0; x < width; x += STEP_SIZE) {
		var nx = 0;
		for (var y = 0; y < height; y += STEP_SIZE) {
			var n = Noise(nx, ny, fcount);
			var r = map(n, 0, 1, 0, SSIZE2);
			var c = color(map(n, 0, 1, 0, 360), 100, 100);
			stroke(c);
			circle(x, y, r);

			nx += SSCALE;
		}
		ny += SSCALE;
	}

	fcount += NOISE_SCALE;
}