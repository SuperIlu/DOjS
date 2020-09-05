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

var rectSize = 20;

function setup() {
	createCanvas(windowWidth, windowHeight);
	stroke(200);
	noFill();
	angleMode(DEGREES);
}

function draw() {
	background(32);
	for (var y = rectSize * 2; y < height - rectSize * 2; y += rectSize * 3) {
		for (var x = rectSize * 2; x < width - rectSize * 2; x += rectSize * 3) {
			drawRect(x, y);
		}
	}
}

function drawRect(x, y) {
	var c = {
		x: x,
		y: y
	};
	var ul = {
		x: x - rectSize,
		y: y - rectSize
	};
	var ur = {
		x: x + rectSize,
		y: y - rectSize
	};
	var ll = {
		x: x - rectSize,
		y: y + rectSize
	};
	var lr = {
		x: x + rectSize,
		y: y + rectSize
	};

	var a = map(x - mouseX + y - mouseY, 0, width + height, 0, 90);

	ul = rot(ul, a, c);
	ur = rot(ur, a, c);
	ll = rot(ll, a, c);
	lr = rot(lr, a, c);

	if ((mouseX > x - rectSize) && (mouseX < x + rectSize) && (mouseY > y - rectSize) && (mouseY < y + rectSize)) {
		fill(128);
	} else {
		noFill();
	}

	beginShape();
	vertex(ul.x, ul.y);
	vertex(ur.x, ur.y);
	vertex(lr.x, lr.y);
	vertex(ll.x, ll.y);
	endShape(CLOSE);
}

// https://academo.org/demos/rotation-about-point/
function rot(p, a, c) {
	var cA = cos(a);
	var sA = sin(a);

	var x = p.x - c.x;
	var y = p.y - c.y;

	return {
		x: (x * cA - y * sA) + c.x,
		y: (y * cA + x * sA) + c.y
	};
}
