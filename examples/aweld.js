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
Include('p5');


var MAX_ROTATION_SPEED = 0.5; // max rotation speed in degrees per frame
var STEP_SIZE = 150; // distance between arcs
var ARC_WIDTH = 3; // stroke weight of the arcs
var EASE_FRAMES = 200; // number of frames to ease into new start/stop

// all known ease functions
var EASE_FUNCS = [
	easeInOutSine,
	easeInOutQuad,
	easeInOutCubic,
	easeInOutQuart,
	easeInOutQuint,
	easeInOutExpo,
	easeInOutCirc,
	easeInOutBack,
	easeInOutElastic,
	easeInOutBounce,
];

// contains all arcs
var allArcs = [];

function setup() {
	createCanvas(windowWidth, windowHeight);

	colorMode(HSB);
	reInit();
}

function mousePressed() {
	reInit();
	return false;
}

function draw() {
	background(0);

	for (var i = 0; i < allArcs.length; i++) {
		allArcs[i].update();
		allArcs[i].draw();
	}
}

// setup new random arcs
function reInit() {
	allArcs = [];

	// make 4 clusters of arcs
	for (var r = STEP_SIZE; r < width; r += random(STEP_SIZE / 2, STEP_SIZE)) {
		allArcs.push(new EasingArc((1 * width) / 4, (1 * height) / 4, r));
	}
	for (var r = STEP_SIZE; r < width; r += random(STEP_SIZE / 2, STEP_SIZE)) {
		allArcs.push(new EasingArc((3 * width) / 4, (1 * height) / 4, r));
	}
	for (var r = STEP_SIZE; r < width; r += random(STEP_SIZE / 2, STEP_SIZE)) {
		allArcs.push(new EasingArc((1 * width) / 4, (3 * height) / 4, r));
	}
	for (var r = STEP_SIZE; r < width; r += random(STEP_SIZE / 2, STEP_SIZE)) {
		allArcs.push(new EasingArc((3 * width) / 4, (3 * height) / 4, r));
	}
}

// create a new arc with given center and radius
function EasingArc(x, y, r) {
	this.x = x;
	this.y = y;
	this.r = r;
	this.color = color(map(r, 0, width, 0, 360), 100, 100, 0.5); // map color to radius

	this.newStart = this.start = this.currentStart = random(TWO_PI); // init positions
	this.newEnd = this.end = this.currentEnd = random(TWO_PI);

	this.easeFunction = null; // mark this arc as "needs new random values"
}

// draw the arc
EasingArc.prototype.draw = function () {
	noFill();

	// draw 'rails'
	strokeWeight(0.1);
	stroke(255);
	circle(this.x, this.y, this.r);

	// draw the arc
	strokeWeight(ARC_WIDTH);
	stroke(this.color);
	arc(this.x, this.y, this.r, this.r, this.currentStart, this.currentEnd, OPEN);
};

// make sure start <= end for all variables
EasingArc.prototype.normalize = function () {
	if (this.start > this.end) {
		var tmp = this.start;
		this.start = this.end;
		this.end = tmp;
	}

	if (this.newStart > this.newEnd) {
		var tmp = this.newStart;
		this.newStart = this.newEnd;
		this.newEnd = tmp;
	}

	this.currentStart = this.start;
	this.currentEnd = this.end;
};

// update the arcs start/end postion by easing function
EasingArc.prototype.update = function () {
	if (this.easeFunction === null) {
		// no easing function, chose random values for this run
		this.easeFunction = EASE_FUNCS[int(random(EASE_FUNCS.length))]; // choose ease function for the next run
		this.count = 0;
		this.easeFrames = random(EASE_FRAMES / 2, EASE_FRAMES);
		this.newStart = random(TWO_PI);
		this.newEnd = random(TWO_PI);
		this.normalize();
	} else {
		// ease arc into new start/end values
		var percent = this.count / this.easeFrames;

		if (percent < 1) {
			var p = this.easeFunction(percent);
			this.currentStart = this.start + (this.newStart - this.start) * p;
			this.currentEnd = this.end + (this.newEnd - this.end) * p;

			this.count++;
		} else {
			this.start = this.currentStart;
			this.end = this.currentEnd;

			this.easeFunction = null;
		}
	}
};

//////
// easing functions are from https://easings.net/
function easeInOutSine(x) {
	return -(cos(PI * x) - 1) / 2;
}

function easeInOutQuad(x) {
	return x < 0.5 ? 2 * x * x : 1 - pow(-2 * x + 2, 2) / 2;
}

function easeInOutCubic(x) {
	return x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
}

function easeInOutQuart(x) {
	return x < 0.5 ? 8 * x * x * x * x : 1 - pow(-2 * x + 2, 4) / 2;
}

function easeInOutQuint(x) {
	return x < 0.5 ? 16 * x * x * x * x * x : 1 - pow(-2 * x + 2, 5) / 2;
}

function easeInOutExpo(x) {
	return x === 0 ?
		0 :
		x === 1 ?
			1 :
			x < 0.5 ?
				pow(2, 20 * x - 10) / 2 :
				(2 - pow(2, -20 * x + 10)) / 2;
}

function easeInOutCirc(x) {
	return x < 0.5 ?
		(1 - sqrt(1 - pow(2 * x, 2))) / 2 :
		(sqrt(1 - pow(-2 * x + 2, 2)) + 1) / 2;
}

function easeInOutBack(x) {
	var c1 = 1.70158;
	var c2 = c1 * 1.525;

	return x < 0.5 ?
		(pow(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2 :
		(pow(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
}

function easeInOutElastic(x) {
	var c5 = (2 * Math.PI) / 4.5;

	return x === 0 ?
		0 :
		x === 1 ?
			1 :
			x < 0.5 ?
				-(pow(2, 20 * x - 10) * sin((20 * x - 11.125) * c5)) / 2 :
				(pow(2, -20 * x + 10) * sin((20 * x - 11.125) * c5)) / 2 + 1;
}

function easeInOutBounce(x) {
	return x < 0.5 ?
		(1 - easeOutBounce(1 - 2 * x)) / 2 :
		(1 + easeOutBounce(2 * x - 1)) / 2;
}

function easeOutBounce(x) {
	var n1 = 7.5625;
	var d1 = 2.75;

	if (x < 1 / d1) {
		return n1 * x * x;
	} else if (x < 2 / d1) {
		return n1 * (x -= 1.5 / d1) * x + 0.75;
	} else if (x < 2.5 / d1) {
		return n1 * (x -= 2.25 / d1) * x + 0.9375;
	} else {
		return n1 * (x -= 2.625 / d1) * x + 0.984375;
	}
}