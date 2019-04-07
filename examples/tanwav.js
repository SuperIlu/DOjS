/**
 * This work is licensed under Attribution-ShareAlike 3.0 Unported.
 * https://creativecommons.org/licenses/by-sa/3.0/
 * 
 * Original by Shawn Small
 * 
 * The original source can be found here:
 * https://www.openprocessing.org/sketch/679005
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

var size = 10;
var boxes;
var vel = .004;

function setup() {
	createCanvas(windowWidth, windowHeight);
	circles = []
	for (var x = size, y = windowHeight / 2; x < windowWidth - size; x += size * 2) {
		circles.push(new Circle(x, y));
	}
	stroke(0);
}

function draw() {
	background(255);
	fill(0);
	for (var i = 0; i < circles.length; i++) {
		circles[i].create();
		if (i < circles.length - 1) {
			line(circles[i].x, circles[i].y, circles[i + 1].x, circles[i + 1].y)
		}
		circles[i].theta += circles[i].vel;
	}
}

function Circle(x, y) {
	this.x = x;
	this.y = y;
	this.theta = 0;
	this.vel = vel;
	vel += .002;
}
Circle.prototype.create = function () {
	fill(this.x - windowHeight / 2 + 100, this.y - windowHeight / 2 + 100, this.y - windowHeight / 2 + 100);
	this.y = tan(this.theta) * 100 + windowHeight / 2;
	ellipse(this.x, this.y, size * 2, size * 2);
};
