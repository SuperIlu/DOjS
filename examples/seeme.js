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

function setup() {
	createCanvas(windowWidth, windowHeight);
	frameRate(1);
	angleMode(DEGREES);

	irisSize = (height - height / 20) / 2;
}

function draw() {
	// clear screen
	colorMode(RGB);
	background(0);

	// draw eye
	fill(255, 255, 255);
	ellipse(width / 2, height / 2, width, height);

	// draw iris
	colorMode(HSB, 255, 255, 255, 255);
	var col = random(0, 255);
	var sat = random(32, 224);
	var stepSize = irisSize / 255;
	for (var i = 0; i < 256; i++) {
		fill(col, sat, 255 - i);
		circle(width / 2, height / 2, 2 * irisSize - i * stepSize);
	}

	push();
	noFill();
	strokeWeight(1);
	stroke(random(0, 255), random(32, 224), 255, 200);
	translate(width / 2, height / 2);
	var inc = random(2, 4);
	for (var a = 0; a < 360; a += inc) {
		rotate(inc);
		beginShape();
		var verLen = 0;
		while (verLen < irisSize) {
			vertex(verLen, random(-4, 4));
			verLen += random(1, 5);
		}
		endShape();
	}
	pop();

	// draw pupil
	noStroke();
	colorMode(RGB);
	fill(0);
	circle(width / 2, height / 2, random(width / 5, width / 10));
}
