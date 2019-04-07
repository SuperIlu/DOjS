/**
 * This work is licensed under Attribution-ShareAlike 3.0 Unported.
 * https://creativecommons.org/licenses/by-sa/3.0/
 * 
 * Original by @reona396, http://reona396.information.jp/
 * 
 * The original source can be found here:
 * https://www.openprocessing.org/sketch/648552
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

var waves = [];
var wavesNum = 60;

var colors = [];

function setup() {
	createCanvas(windowWidth, windowHeight);
	background(255);
	noStroke();

	for (var i = 0; i < wavesNum; i++) {
		waves.push(new Wave(i));
	}

	var alpha = 50;

	colors[0] = color(217, 49, 49, alpha);
	colors[1] = color(34, 116, 204, alpha);
	colors[2] = color(243, 213, 79, alpha);
	colors[3] = color(156, 64, 152, alpha);
}

function draw() {
	for (var i = 0; i < wavesNum; i++) {
		waves[i].display();
	}
}

function Wave(tmpIndex) {
	this.index = tmpIndex;
	this.x = width / 2;
	this.y = height / 2;
	this.R = 0;
	this.Rs = random(0.5, 3);
	this.r = 5;
	this.rn = 0.0;
	this.rns = random(0.1, 0.5);
	this.t = 0;
	this.tn = 0.0;
	this.tns = random(0.001, 0.005);
	this.tr = random(0.00, 360.00);
	this.c = this.index % 4;
	this.Rmax = random(100, width);

	this.display = function () {
		this.t = map(noise(this.tn + this.index), 0, 1, 0, 360);
		this.tn += this.tns;
		this.r = 15 * noise(this.rn + this.index);
		this.rn += this.rns;

		this.x = this.R * cos(radians(this.t)) + width;
		this.y = this.R * sin(radians(this.t)) + height / 2;

		fill(colors[this.c]);
		ellipse(this.x, this.y, this.r, this.r);

		this.R += this.Rs;

		if (this.R > this.Rmax) {
			this.R = 0;
			this.Rmax = random(100, width);
		}
	}
}