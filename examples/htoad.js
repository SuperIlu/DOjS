/*
MIT License

Copyright (c) 2019 Andre Seidelt <superilu@yahoo.com>

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
	background(64);
	angleMode(DEGREES);
	colorMode(HSB);
	textAlign(CENTER, CENTER);
	textSize(32);

	NUM_STEPS = 15;
	STEP_SIZE = width / NUM_STEPS;

	ANGLE_INC = 1;
}

function draw() {
	background(frameCount, 255, 255);
	translate(width / 2, height / 2);
	for (var rot = 0; rot < 360; rot += ANGLE_INC) {
		for (var s = 0; s < NUM_STEPS; s++) {
			push();
			var y = sin(rot * 20) * 20;
			if (s % 2 == 0) {
				rotate(rot + frameCount / 2);
			} else {
				rotate(rot - frameCount / 2);
			}
			translate(0, 20 + s * STEP_SIZE + y);
			circle(0, 0, 4);
			pop();
		}
	}
	if (frameCount % 150 > 100) {
		text("FOLLOW THE HYPNOTOAD!", random(-4, 4), random(-4, 4));
	}
}
