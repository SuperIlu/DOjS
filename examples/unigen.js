/**
 * This work is licensed under Attribution-ShareAlike 3.0 Unported.
 * https://creativecommons.org/licenses/by-sa/3.0/
 * 
 * Original by reona
 * https://twitter.com/reona396/status/1125291157162868736
 * 
 * The original source can be found here:
 * https://www.openprocessing.org/sketch/707543
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */

Include('p5');

var stars = [];
var starsNum = 50;
var starsSizeMax;
var count = 0;

function setup() {
	createCanvas(windowWidth, windowHeight);
	colorMode(HSB, 360, 100, 100, 100);
	frameRate(10);
	background(0);
	noStroke();
	starsSizeMax = width > height ? width / 20 : height / 20;

	for (var i = 0; i < starsNum; i++) {
		stars.push(new Star());
	}
}

function draw() {
	//	blendMode(BLEND);
	background(0, 30);

	//	blendMode(ADD);
	for (var i = 0; i < stars.length; i++) {
		stars[i].display();
	}

	count++;

	if (count > 25) {
		stars = [];
		for (var i = 0; i < starsNum; i++) {
			stars.push(new Star());
		}
		count = 0;
	}
}

function Star() {
	this.ox = random(width);
	this.oy = random(height);
	this.R = random(10, starsSizeMax);
	this.lightNum = floor(random(10, 60));
	this.c = random(360);
	this.cRange = random(360);

	this.display = function () {
		for (var i = 0; i < this.lightNum; i++) {
			var vertexNum = floor(random(3, 8));
			fill(random(this.c, this.c + this.cRange) % 360, random(50, 100), random(30, 100), 5);

			push();
			translate(this.ox, this.oy);
			beginShape();
			for (var j = 0; j < vertexNum; j++) {
				var r = random(this.R);
				var t = random(TWO_PI);
				vertex(r * cos(t), r * sin(t));
			}
			endShape(CLOSE);
			pop();
		}
	}
}
