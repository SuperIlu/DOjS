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

var NUM_RECTS = 20;
var xStep, yStep;

function setup() {
	createCanvas(min(windowWidth, windowHeight), min(windowWidth, windowHeight));
	background(100);

	xStep = height / NUM_RECTS;
	yStep = height / NUM_RECTS;
}

function draw() {
	mouseClicked();
	noLoop();
}

function mouseClicked() {
	colorMode(RGB);
	background(32);
	noFill();
	colorMode(HSB);
	for (var y = 0; y < height; y += yStep) {
		for (var x = 0; x < width; x += xStep) {
			// stroke(0);
			// rect(x, y, xStep, yStep);
			stroke(random(360), 100, 100);
			randomizeRect(x, y, xStep, yStep);
		}
	}
}

function randomizeRect(x, y, w, h) {
	// var numItems = 1;
	var numItems = int(random(1, 4));
	for (var i = 0; i < numItems; i++) {
		// var type = 7;
		var type = int(random(10));
		switch (type) {
			case 0:
				line(x, y, x + w, y + h); // \
				break;
			case 1:
				line(x + w, y, x, y + h); // /
				break;
			case 2:
				arc(x, y, w * 2, h * 2, 0, HALF_PI);
				break;
			case 3:
				arc(x + w, y, w * 2, h * 2, HALF_PI, PI);
				break;
			case 4:
				arc(x + w, y + h, w * 2, h * 2, PI, PI + HALF_PI);
				break;
			case 5:
				arc(x, y + h, w * 2, h * 2, PI + HALF_PI, TWO_PI);
				break;
			case 6:
				arc(x + w / 2, y, w, h, 0, PI);
				break;
			case 7:
				arc(x, y + h / 2, w, h, PI + HALF_PI, TWO_PI + HALF_PI);
				break;
			case 8:
				arc(x + w / 2, y + h, w, h, PI, TWO_PI);
				break;
			case 9:
				arc(x + w, y + h / 2, w, h, HALF_PI, PI + HALF_PI);
				break;
		}
	}
}