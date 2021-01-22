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
// colors taken from https://www.schemecolor.com/lgbt-flag-colors.php
var flagColors = [];

function setup() {
	createCanvas(windowWidth, windowHeight);

	// create rainbow colors
	var a = 32;
	flagColors.push(color(231, 0, 0, a));
	flagColors.push(color(255, 140, 0, a));
	flagColors.push(color(255, 239, 0, a));
	flagColors.push(color(0, 129, 31, a));
	flagColors.push(color(0, 68, 255, a));
	flagColors.push(color(118, 0, 137, a));

	fieldHeight = int(height / flagColors.length);

	lastX = [];
	lastY = [];
	for (var c = 0; c < flagColors.length; c++) {
		lastX.push(random(width));
		lastY.push(random(c * fieldHeight, (c + 1) * fieldHeight));
	}
}



function draw() {
	for (var c = 0; c < flagColors.length; c++) {
		var startX = random(width);
		var startY = random(c * fieldHeight, (c + 1) * fieldHeight);
		var endX = random(width);
		var endY = random(c * fieldHeight, (c + 1) * fieldHeight);

		stroke(flagColors[c]);
		strokeWeight(random(1, 4));

		line(lastX[c], lastY[c], endX, endY);
		lastX[c] = endX;
		lastY[c] = endY;
	}
}
