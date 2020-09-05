/**
 * This work is licensed under Attribution-ShareAlike 3.0 Unported.
 * https://creativecommons.org/licenses/by-sa/3.0/
 * 
 * Original by @reona396, http://reona396.information.jp/
 * 
 * The original source can be found here:
 * https://www.openprocessing.org/sketch/739905/
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

var objs = [];
var ox, oy;

function setup() {
	colorMode(HSB, 360, 100, 100, 100);
	rectMode(CENTER);
	frameRate(30);

	ox = width / 2;
	oy = height / 2;

	noFill();
	background(0);
}

function draw() {
	background(0, 10);

	objs.push(new Obj(ox, oy));

	for (var i = 0; i < objs.length; i++) {
		objs[i].display();
		if (objs[i].isFinished()) {
			objs.splice(i, 1);
		}
	}

	ox += random(-50, 50);
	oy += random(-50, 50);

	if (ox > width) {
		ox = width;
	}
	if (ox < 0) {
		ox = 0;
	}
	if (oy > height) {
		oy = height;
	}
	if (oy < 0) {
		oy = 0;
	}
}

function mousePressed() {
	background(0);
	objs = [];
}

function Obj(tmpX, tmpY) {
	this.x = tmpX;
	this.y = tmpY;
	this.w = 0;
	this.h = 0;
	this.wSpeed = random(10, 40);
	this.hSpeed = random(10, 40);
	this.finished = false;
	this.hue = random(360);
	this.stW = random(1, 10);

	this.display = function () {
		strokeWeight(this.stW);
		stroke(this.hue, 100, 100, 30);
		rect(this.x, this.y, this.w, this.h);

		this.w += this.wSpeed;
		this.h += this.hSpeed;

		if (width > height) {
			if (this.x <= width / 2) {
				if (this.w > (width - this.x) / 2) {
					finished = true;
				}
			} else {
				if (this.w > this.x * 2) {
					finished = true;
				}
			}
		} else {
			if (this.y <= height / 2) {
				if (this.h > (height - this.y) / 2) {
					finished = true;
				}
			} else {
				if (this.h > this.y * 2) {
					finished = true;
				}
			}
		}
	}

	this.isFinished = function () {
		return this.finished;
	}
}
