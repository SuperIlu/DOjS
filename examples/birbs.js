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

var BIRB_WIDTH = 30;
var BIRB_HEIGHT = 60;
var BIRB_WIDTH_2 = BIRB_WIDTH / 2;
var BIRB_WIDTH_4 = BIRB_WIDTH / 4;
var BIRB_HEIGHT_2 = BIRB_HEIGHT / 2;

function setup() {
	createCanvas(windowWidth, windowHeight);
	stroke(255);
	noFill();
	colorMode(HSB);
	frameRate(1);
}

function draw() {
	background(0);

	for (var x = 0; x < width; x += BIRB_WIDTH * 2) {
		for (var y = BIRB_HEIGHT_2; y < height; y += BIRB_HEIGHT * 2) {
			drawBirb(x, y);
		}
	}
}

function drawBirb(x, y) {
	// body
	stroke(random(255), 255, 255);
	arc(x, y, BIRB_WIDTH, BIRB_WIDTH, PI, TWO_PI);
	line(x - BIRB_WIDTH_2, y, x - BIRB_WIDTH_2, y + (BIRB_HEIGHT - BIRB_WIDTH) * random(0.5, 1));
	line(x + BIRB_WIDTH_2, y, x + BIRB_WIDTH_2, y + (BIRB_HEIGHT - BIRB_WIDTH) * random(0.5, 1));

	// hair
	var hS = BIRB_WIDTH_4 * random(0.5, 1);
	stroke(random(255), 255, 255);
	line(x, y - BIRB_WIDTH_2, x + hS, y - BIRB_WIDTH_2 - 10);
	line(x, y - BIRB_WIDTH_2, x - hS, y - BIRB_WIDTH_2 - 10);

	// eye
	var eX = x;
	var eY = y;
	var eS = 8 * random(0.2, 1);
	stroke(random(255), 255, 255);
	circle(eX + BIRB_WIDTH_4, eY - BIRB_WIDTH_4, eS);

	// beak
	var bX = x + BIRB_WIDTH_2;
	var bY = y - 8;
	var bXend = bX + BIRB_WIDTH * random(0.3, 0.8);
	stroke(random(255), 255, 255);
	line(bX + BIRB_WIDTH_4, bY + BIRB_WIDTH_4, bXend, bY + BIRB_WIDTH_4);
	stroke(random(255), 255, 255);
	line(bX, bY, bXend, bY + BIRB_WIDTH_4);
	line(bX, bY + BIRB_WIDTH_2, bXend, bY + BIRB_WIDTH_4);

	// wing
	var wX = x;
	var wY = y;
	var wS = random(0.5, 1);
	stroke(random(255), 255, 255);
	line(wX, wY, wX - BIRB_WIDTH_2 * wS, wY + BIRB_WIDTH_4);
	line(wX, wY + BIRB_WIDTH_2, wX - BIRB_WIDTH_2 * wS, wY + BIRB_WIDTH_4);

	// feet
	var fX = x;
	var fY = y;
	stroke(random(255), 255, 255);
	line(fX - BIRB_WIDTH_4, fY + BIRB_HEIGHT - BIRB_WIDTH * random(1, 1.5), fX - BIRB_WIDTH_4, fY + BIRB_HEIGHT - BIRB_WIDTH + BIRB_WIDTH_4);
	line(fX + BIRB_WIDTH_4, fY + BIRB_HEIGHT - BIRB_WIDTH * random(1, 1.5), fX + BIRB_WIDTH_4, fY + BIRB_HEIGHT - BIRB_WIDTH + BIRB_WIDTH_4);
	line(fX - BIRB_WIDTH_4, fY + BIRB_HEIGHT - BIRB_WIDTH + BIRB_WIDTH_4, fX - BIRB_WIDTH_2, fY + BIRB_HEIGHT - BIRB_WIDTH + BIRB_WIDTH_4);
	line(fX + BIRB_WIDTH_4, fY + BIRB_HEIGHT - BIRB_WIDTH + BIRB_WIDTH_4, fX + BIRB_WIDTH_2, fY + BIRB_HEIGHT - BIRB_WIDTH + BIRB_WIDTH_4);
}