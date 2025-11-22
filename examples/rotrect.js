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

var NUM_RECTS = 3;
var NUM_FIGURES = 5;
var ROTATION = 0.5;
var RATE = 30;

var rectSize;
var rectSize_2;
var rects = [
	[]
];

var rot = 0;

function setup() {
	createCanvas(min(windowWidth, windowHeight), min(windowWidth, windowHeight));

	rectSize = min(width, height) / (NUM_FIGURES + 1);
	rectSize_2 = rectSize / 2;
	// rects = [];
	// for (var y = 0; y < NUM_FIGURES - 1; y++) {
	//  var line = [];
	//  for (var x = 0; x < NUM_FIGURES - 1; x++) {
	//      line.push(random(360 / NUM_RECTS));
	//  }
	//  rects.push(line);
	// }

	mouseClicked();

	angleMode(DEGREES);
	rectMode(CENTER);
	frameRate(RATE);
}

function draw() {
	colorMode(RGB);
	background(32);

	noFill();
	colorMode(HSB);
	for (var y = 0; y < NUM_FIGURES; y++) {
		for (var x = 0; x < NUM_FIGURES; x++) {
			if (rects[y] && rects[y][x]) {
				push();
				translate(rectSize + x * rectSize, rectSize + y * rectSize);
				rotate(rot);
				for (var i = 0; i < NUM_RECTS; i++) {
					rotate(rects[y][x]);
					stroke((rot + rects[y][x] * i) % 360, 100, 100);
					square(0, 0, rectSize * 0.75);
				}
				pop();
			}
		}
	}

	if (frameCount % RATE == 0) {
		if ((rects[rects.length - 1].length >= NUM_FIGURES) && (rects.length < NUM_FIGURES)) {
			rects.push([]);
		}
		if (rects[rects.length - 1].length < NUM_FIGURES) {
			rects[rects.length - 1].push(random(360 / NUM_RECTS));
		}
	}

	rot += ROTATION;
}

function mouseClicked() {
	rects = [
		[]
	];
	rects[rects.length - 1].push(random(360 / NUM_RECTS));
}
