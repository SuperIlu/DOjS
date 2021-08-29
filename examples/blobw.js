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

var BLOB_STEPS = 50;
var BLOB_STEP_SIZE = 360 / BLOB_STEPS;
var NOISE_SCALE = 0.008;
var BLOB_SIZE = 20;
var BLOB_SIZE_10 = BLOB_SIZE / 4;
var NUM_BLOBS = 10;

var blobs = [];

function setup() {
	createCanvas(windowWidth, windowHeight);
	angleMode(DEGREES);

	stroke(255, 0, 0);
	reInit();
}

// new flock of blobs on mouseclick
function mouseClicked() {
	reInit();
	return false;
}

// create a new herd of blobs
function reInit() {
	blobs = [];
	for (var i = 0; i < NUM_BLOBS; i++) {
		createBlob();
	}
}

// create a randomly colored blob at random position with random size
function createBlob() {
	blobs.push({
		pos: createVector(random(BLOB_SIZE, width - BLOB_SIZE), random(BLOB_SIZE, height - BLOB_SIZE)),
		col: color(random(100.200), random(100.200), random(100.200), 196),
		size: BLOB_SIZE - random(BLOB_SIZE / 2)
	});
}

function draw() {
	background(32);

	// iterate over blobs
	for (var i = 0; i < blobs.length; i++) {
		var bob = blobs[i]; // current blob 'bob'
		var nearest = findNearest(bob); // find nearest blob to 'bob'
		var Vdir = p5.Vector.sub(nearest, bob.pos); // get direction vector from 'bob' to nearest blob

		drawBlob(bob, Vdir);
		moveBlob(bob, Vdir);
	}
}

// find nearest blob to 'bob'
function findNearest(bob) {
	var ret = blobs[0];
	for (var i = 1; i < blobs.length; i++) {
		var cur = blobs[i]; // get current blob

		// we ourself shall not be the nearest blob
		if (cur == bob) {
			continue;
		}

		var diffOld = abs(p5.Vector.sub(ret.pos, bob.pos).mag()); // calculate absolute distance to 'ret'
		var diffCur = abs(p5.Vector.sub(cur.pos, bob.pos).mag()); // calculate absolute distance to current blob

		// if current is nearer, switch to their position
		if (diffCur < diffOld) {
			ret = cur;
		}
	}
	return ret.pos;
}

function drawBlob(bob, Vdir) {
	// take the vector length in relation to bobs size and map it to a value between 1..2
	var magnitude = map(Vdir.mag(), 0, bob.size, 1, 2, true);

	var r_inc = bob.size / 6;
	push();
	fill(bob.col); // use bobs color
	translate(bob.pos); // center on bobs position
	rotate(Vdir.heading()); // rotate bob into the direction of Vdir
	beginShape();
	for (var a = 0; a < 360; a += BLOB_STEP_SIZE) { // go in a full circle
		// calculate sin/cos for current angle
		var sinA = sin(a);
		var cosA = cos(a);

		// calculate circle points, make it an elipse using the magnitude
		var sX = bob.size * sinA * magnitude;
		var sY = bob.size * cosA;

		// add a noise to circle points
		var n = magnitude * noise(sX + frameCount * NOISE_SCALE, sY + frameCount * NOISE_SCALE, frameCount * NOISE_SCALE * 4);
		var nX = n * sinA * r_inc;
		var nY = n * cosA * r_inc;

		vertex(sX + nX, sY + nY);
	}
	endShape();
	pop();
}

// move this blob 'bob' in direction 'Vdir'
function moveBlob(bob, Vdir) {
	// check for minimal distance (avoids further moving when the blobs are on top of each other)
	if (Vdir.mag() > 2) {
		Vdir.normalize(); // normalize vector to get baby steps :)

		// add distance onto pos of 'bob'
		bob.pos.x += Vdir.x;
		bob.pos.y += Vdir.y;
	}
}