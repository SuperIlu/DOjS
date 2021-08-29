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

var STEP_SIZE = 3;
var NOISE_FACTOR = 1;
var NUM_SEEDS = 100;
var BOOST_FACTOR = 0.05;
var ANGLE_FACTOR = 2;
var STEM_STROKE = 1;
var BLOOM_SIZE = 3;
var BLOOM_THRESHOLD = 0.3;

var seeds = [];

function setup() {
	createCanvas(windowWidth, windowHeight);
	noFill();
	strokeWeight(STEM_STROKE);

	seeed();
}

function draw() {
	background(23, 17, 7); // background is dirt :)

	// draw all plants
	for (var i = 0; i < seeds.length; i++) {
		drawSeed(seeds[i]);
	}

	// grow a plant each frame
	growSeed(seeds[frameCount % seeds.length]);

	// randomly boost a plant
	for (var i = 0; i < NUM_SEEDS * BOOST_FACTOR; i++) {
		growSeed(seeds[floor(random(seeds.length))]);
	}
}

function mouseClicked() {
	seeds = [];
	seeed();
}

// draw a single seed
function drawSeed(seed) {
	push();
	stroke(seed.c); // set color
	translate(seed.v[0].x, seed.v[0].y); // move to start point

	// draw all points
	for (var i = 1; i < seed.v.length; i++) {
		var v = seed.v[i];
		line(0, 0, v.x, v.y);
		translate(v.x, v.y);
	}
	if (seed.f) {
		fill(seed.f);
		noStroke();
		circle(0, 0, BLOOM_SIZE);
	}
	pop();
}

// add an additional point with noise() generated heading
function growSeed(seed) {
	if (!seed.f) {
		var v = createVector(0, -STEP_SIZE); // create vector pointing UP
		var n = (noise(seed.v[0].x, seed.v[0].y, frameCount * NOISE_FACTOR) - 0.5);
		if (n < -BLOOM_THRESHOLD) {
			seed.f = color(random(200, 255), random(0, 75), random(0, 66)); // red
		} else if (n > BLOOM_THRESHOLD) {
			seed.f = color(random(0, 75), random(0, 66), random(200, 255)); // blue
		} else {
			v.rotate(ANGLE_FACTOR * n); // rotate it slightly based on noise
			seed.v.push(v);
		}
	}
}
// seed the field
function seeed() {
	for (var i = 0; i < random(NUM_SEEDS - NUM_SEEDS / 10, NUM_SEEDS + NUM_SEEDS / 10); i++) {
		seeds.push({
			c: color(random(0, 75), random(100, 255), random(0, 66), 200),
			v: [createVector(random(0, width), random(0, height))],
			f: null
		});
	}
}