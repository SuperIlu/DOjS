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

var vert = [];
var gridSize = 20;
var maxLength = 1000;

function setup() {
	createCanvas(windowWidth, windowHeight);
	background(100);

	vert.push([width / 2, height / 2]);
	noFill();
}

function draw() {
	colorMode(RGB);
	background(100);


	for (var i = 1; i < vert.length; i++) {
		colorMode(HSB);
		stroke(i % 255, 255, 255);
		strokeWeight(3);
		line(vert[i][0], vert[i][1], vert[i - 1][0], vert[i - 1][1]);
	}

	var dir = int(random(4));
	var last = vert[vert.length - 1];
	var x, y;
	if (dir == 0) {
		// left
		x = last[0] - gridSize;
		if (x < 0) {
			x = last[0] + gridSize;
		}
		y = last[1];
	} else if (dir == 1) {
		// right
		x = last[0] + gridSize;
		if (x > width) {
			x = last[0] - gridSize;
		}
		y = last[1];
	} else if (dir == 2) {
		// up
		y = last[1] - gridSize;
		if (y < 0) {
			y = last[1] + gridSize;
		}
		x = last[0];
	} else {
		// down
		y = last[1] + gridSize;
		if (y > height) {
			y = last[1] - gridSize;
		}
		x = last[0];
	}
	vert.push([x, y]);

	while (vert.length > maxLength) {
		vert.shift();
	}
}
