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

function setup() {
	//createCanvas(windowWidth, windowHeight);
	createCanvas(640, 480);
	background(100);
	fill(0);

	HOLE_SIZE = 20;
	light = null;
	hole = createVector(width / 2, height / 2);
	frameRate(200);
	colorMode(HSB);
}

function draw() {
	if (light) {
		light.update();
		light.draw();
		if (light.isDead) {
			light = null;
		}
	} else {
		light = new Light(random(height));
	}
	noStroke();
	circle(hole.x, hole.y, HOLE_SIZE);
}

function Light(y) {
	this.location = createVector(width, y);
	this.velocity = createVector(-1, 0);
	this.acceleration = createVector(0, 0);
	this.col = y;
};

Light.prototype.update = function () {
	var dist = hole.dist(this.location);
	var dir = p5.Vector.sub(hole, this.location);
	dir.normalize();
	dir.mult(HOLE_SIZE / sq(dist));
	this.acceleration = dir;
	this.velocity.add(this.acceleration);
	this.location.add(this.velocity);

	stroke(dist, 255, 255);

	this.isDead = this.location.x <= 0 ||
		this.location.y <= 0 ||
		this.location.x > width ||
		this.location.y > height ||
		dist < HOLE_SIZE / 2;
};

Light.prototype.draw = function () {
	point(this.location.x, this.location.y);
};
