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

var col;

function setup() {
	createCanvas(windowWidth, windowHeight);
	background(250);
	colorMode(HSB, 255);
	frameRate(4);

	col = 0;
}

function draw() {
	var t = true;

	for (var y = 50; y < height - 50; y += 40) {
		for (var x = 100; x < width - 100; x += 40) {
			drawCell(x, y, t, color(col, map(abs(width / 2 - x), 0, width / 2, 32, 255), 255));
			t = !t;
			col = (col + 1) % 0xFF;
		}
	}
}

function drawCell(x, y, t, c) {
	stroke(10);
	fill(c);
	beginShape();
	if (t) {
		var d1 = 5;
		var d2 = -5;
	} else {
		var d1 = -5;
		var d2 = 5;
	}
	vertex(x - 20, y - 20 + d1);
	vertex(x + 20, y - 20 - d1);
	vertex(x + 20, y + 20 - d1);
	vertex(x - 20, y + 20 + d1);
	endShape(CLOSE);
}
