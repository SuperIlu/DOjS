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
	background(100);
	angleMode(DEGREES);
	ellipseMode(CENTER);

	col_point = color(255, 0, 0);
}

var initialRot = 0;
var lastX;

function draw() {
	background(100);
	translate(width / 2, height / 2);
	rotate(initialRot);
	var rot = 0;
	lastX = 120;
	while (rot < 360) {
		rot += proteinS();
		dna(rot);
		rot += points();
		dna(rot);

		rot += proteinHE();
		dna(rot);
		rot += points();
		dna(rot);

		rot += proteinM();
		dna(rot);
		rot += points();
		dna(rot);

		rot += proteinHE();
		dna(rot);
		rot += points();
		dna(rot);
	}

	initialRot += 1;
}

function dna(r) {
	if (r > 60) {
		var amp = sin(r * 15);

		var x = 120 + amp * 50;

		stroke(3, 211, 252);
		fill(3, 211, 252);
		ellipse(x, 0, 4, 4);

		strokeWeight(1);
		stroke(3, 211, 252);
		line(lastX, 0, x, 0);

		lastX = x;
	}
}

function proteinM() {
	stroke(0);
	fill(0, 255, 0);

	strokeWeight(2);
	line(200, 0, 220, 0);
	ellipse(220, 0, 7, 7);

	return 0;
}

function proteinHE() {
	stroke(0);
	fill(255, 255, 0);

	strokeWeight(1);
	line(205, 0, 215, 0);
	ellipse(205, 0, 5, 5);

	return 0;
}

function proteinS() {
	stroke(0);
	fill(252, 3, 144);

	strokeWeight(3);
	line(200, 0, 230, 0);
	ellipse(230, 0, 10, 10);

	return 0;
}

function points() {
	stroke(col_point);
	fill(col_point);
	strokeWeight(1);

	rotate(1);
	ellipse(210, 0, 3, 3);
	ellipse(215, 0, 3, 3);
	rotate(1);

	return 2;
}