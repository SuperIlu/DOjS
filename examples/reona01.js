/**
 * This work is licensed under Attribution-ShareAlike 3.0 Unported.
 * https://creativecommons.org/licenses/by-sa/3.0/
 * 
 * Original by @reona396, http://reona396.information.jp/
 * 
 * The original source can be found here:
 * https://www.openprocessing.org/sketch/464493
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

var shapes = [];
var shapesNum = 500;

function setup() {
	createCanvas(400, 400);
	colorMode(HSB, 360, 100, 100, 100);
	background(0);

	for (var i = 0; i < shapesNum; i++) {
		shapes.push(new Shape());
	}
}

function draw() {
	for (var i = 0; i < shapesNum; i++) {
		shapes[i].display();
	}
}

function Shape() {
	this.t = 0;
	this.r = 50;
	this.tInit = random(1, 10);
	this.tDelta = random(2, 10);
	this.hue = random(360);

	this.display = function () {
		this.x = map(noise(this.t + this.tInit), 0, 1, 0, width);
		this.y = map(noise(this.t + this.tInit * 2), 0, 1, 0, height);

		fill(this.hue, 100, 100, 18);
		noStroke();
		ellipse(this.x, this.y, this.r, this.r);

		this.t += 0.005;
		this.r -= 0.5;

		if (this.r < 0) {
			this.r = 0;
		}
	}

	this.init = function () {
		this.t = 0;
		this.r = 50;
		this.tInit = random(1, 10);
		this.tDelta = random(2, 10);
		this.hue = random(360);
	}
}

function mousePressed() {
	background(0);
	for (var i = 0; i < shapesNum; i++) {
		shapes[i].init();
	}
}