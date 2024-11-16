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
if (navigator.appName === "DOjS") {
	Include("p5");
}

var POINT_SIZE = 20;
var POINT_SIZE_10 = POINT_SIZE / 10;
var MAX_AGE = 6;
var STEP_DIVIDER = 10;
var CENTER_DIST = POINT_SIZE * 4;

var points = [];
var current = null;
var distX, distY;

var windowWidth_2;
var windowHeight_2;

function setup() {
	createCanvas(windowWidth, windowHeight);

	windowWidth_2 = int(windowWidth / 2);
	windowHeight_2 = int(windowHeight / 2);

	current = new Point();
	points.push(current);
	fullDist = createVector(current.x, current.y).dist(createVector(windowWidth_2, windowHeight_2));
	distX = windowWidth_2 - current.x;
	distY = windowHeight_2 - current.y;
}

function draw() {
	var i;
	colorMode(RGB);
	background(32);

	colorMode(RGB);
	noFill();
	for (i = 0; i < points.length; i++) {
		points[i].Draw(false);
	}
	colorMode(HSB);
	for (i = 0; i < points.length; i++) {
		points[i].Draw(true);
	}

	if (current.IsCentered()) {
		if (current.children.length == 0) {
			for (i = 0; i < points.length; i++) {
				points[i].Age();
			}
			points = points.filter(function (e, i) {
				return e.age < MAX_AGE;
			});
			current.AddChildren();
			current.ElectChild();
		}
	} else {
		moveGrid();
	}
}

function easeInOut(x) {
	var ret = (x < 0.5) ? x : 1 - x;
	if (ret > 1) {
		ret - 1;
	} else if (ret < 0.05) {
		ret = 0.05;
	}
	return ret;
}

function moveGrid() {
	var currentDist = createVector(current.x, current.y).dist(createVector(windowWidth_2, windowHeight_2));
	var percent = currentDist / fullDist;
	var step = easeInOut(percent);

	for (var i = 0; i < points.length; i++) {
		points[i].Move(step * distX / STEP_DIVIDER, step * distY / STEP_DIVIDER);
	}
}

function Point(x, y) {
	this.x = x || random(windowWidth_2 - CENTER_DIST, windowWidth_2 + CENTER_DIST);
	this.y = y || random(windowHeight_2 - CENTER_DIST, windowHeight_2 + CENTER_DIST);
	this.c = random(360);
	this.s = random(POINT_SIZE / 2, POINT_SIZE);
	this.children = [];
	this.age = 0;
}
Point.prototype.ElectChild = function () {
	current = random(this.children);
	current.x = (current.x - windowWidth_2) * random(2, 6) + windowWidth_2;
	current.y = (current.y - windowHeight_2) * random(2, 6) + windowHeight_2;
	fullDist = createVector(current.x, current.y).dist(createVector(windowWidth_2, windowHeight_2));
	distX = windowWidth_2 - current.x;
	distY = windowHeight_2 - current.y;
	current.age--;
}
Point.prototype.AddChildren = function () {
	var childs = random(3, 10);
	for (var i = 0; i < childs; i++) {
		var nc = new Point();
		points.push(nc);
		this.children.push(nc);
	}
}
Point.prototype.Move = function (dx, dy) {
	this.x += dx;
	this.y += dy;
}
Point.prototype.Draw = function (final) {
	if (final) {
		stroke(this.c, 100, 100);
		fill(this.c, 50, 50);
		circle(this.x, this.y, this.s);
		fill(this.c, 100, 100);
		circle(this.x, this.y, POINT_SIZE_10);
	} else {
		for (var i = 0; i < this.children.length; i++) {
			var dr = this.children[i];
			if (dr == current) {
				stroke(255, 64, 64);
			} else {
				stroke(128);
			}
			line(this.x, this.y, dr.x, dr.y);
		}
	}
}
Point.prototype.IsCentered = function () {
	return abs(this.x - windowWidth_2) < 3 && abs(this.y - windowHeight_2) < 3;
}
Point.prototype.Age = function (x) {
	this.age++;
}