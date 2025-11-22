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

var MAX_DEPTH = 5;
var running = true;

function setup() {
	createCanvas(min(windowWidth, windowHeight), min(windowWidth, windowHeight));
	noStroke();
	frameRate(1)
}

function draw() {
	if (running) {
		background(32);
		rectSelect(new Coords(0, 0, height), MAX_DEPTH);
	}
}

function mouseClicked() {
	running = !running;
}

function rectSelect(c, md) {
	fill(32);
	stroke(255)
	square(c.x, c.y, c.s);

	fill(200);
	var num = floor(random(5));
	var ret;
	switch (num) {
		case 0:
			ret = divTri_1(c);
			break;
		case 1:
			ret = divTri_2(c);
			break;
		case 2:
			ret = divRect_1(c);
			break;
		case 3:
			ret = divRect_2(c);
			break;
		case 4:
			ret = divRect_3(c);
			break;
	}

	if (md > 0) {
		for (var i = 0; i < ret.length; i++) {
			var c = ret[i];
			rectSelect(c, md - 1);
		}
	}
}

function divTri_1(c) {
	var s_2 = c.s / 2;
	triangle(
		c.x, c.y,
		c.x + c.s, c.y + c.s,
		c.x, c.y + c.s
	);

	return [
		new Coords(c.x, c.y + s_2, s_2)
		// new Coords(c.x + s_2, c.y, s_2)
	];
}

function divTri_2(c) {
	var s_2 = c.s / 2;
	triangle(
		c.x, c.y + c.s,
		c.x + c.s, c.y,
		c.x + c.s, c.y + c.s
	);

	return [
		// new Coords(c.x, c.y, s_2),
		new Coords(c.x + s_2, c.y + s_2, s_2)
	];
}

function divRect_1(c) {
	var s_2 = c.s / 2;
	var s_4 = c.s / 4;
	rect(c.x + s_4, c.y + s_4, s_2, s_2);

	return [new Coords(c.x + s_4, c.y + s_4, s_2)];
}

function divRect_2(c) {
	var s_2 = c.s / 2;
	var s_4 = c.s / 4;

	rect(c.x, c.y, s_2, s_2);
	rect(c.x + s_2, c.y + s_2, s_2, s_2);

	return [
		new Coords(c.x, c.y, s_2, s_2),
		new Coords(c.x + s_2, c.y + s_2, s_2, s_2)
	];
}

function divRect_3(c) {
	var s_2 = c.s / 2;
	var s_4 = c.s / 4;

	rect(c.x + s_2, c.y, s_2, s_2);
	rect(c.x, c.y + s_2, s_2, s_2);

	return [
		new Coords(c.x + s_2, c.y, s_2, s_2),
		new Coords(c.x, c.y + s_2, s_2, s_2)
	];
}

function Coords(x, y, s) {
	this.x = x;
	this.y = y;
	this.s = s;
}
Coords.prototype.Margin = function (m) {
	this.x += m;
	this.y += m;
	this.s -= 4 * m;
}