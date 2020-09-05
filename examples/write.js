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

var xPos = 0;
var yPos = 0;

var col = 0;

function setup() {
	createCanvas(windowWidth, windowHeight);
	background(30);
	colorMode(HSB);
	yPos = random(5, 20);
	frameRate(10);
}

function draw() {
	if (xPos > width) {
		xPos = 0;
		yPos += random(15, 30);
	}
	xPos += random(15, 30);
	var xLength = random(30, 80);
	col += xLength;
	stroke(col % 255, 255, 255);
	var edge = random(5, 20);
	strokeWeight(random(1, 4));
	line(xPos, yPos - edge, xPos, yPos);
	line(xPos, yPos, xPos + xLength, yPos);
	xPos += xLength;

	if (yPos > height) {
		clear();
		xPos = 0;
		yPos = random(5, 20);
	}
}
