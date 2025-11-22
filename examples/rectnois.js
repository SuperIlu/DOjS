/*
MIT License

Copyright (c) 2019-2025 Andre Seidelt <superilu@yahoo.com>

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

var TIME_FACTOR = 0.01;
var COLOR_FACTOR = 0.02;
var TILE_FACTOR = 1;
var COL_FACT = 0.75;

var CONFIGS = [
	[25, 5],
	[20, 2],
	[30, 1],
	[50, 10]
];

var BG, NUM_TILES, OFFSET, MARGIN, SIZE, SIZE_2;

function setup() {
	createCanvas(min(windowWidth, windowHeight), min(windowWidth, windowHeight));

	colorMode(RGB);
	BG = color(64);

	mouseClicked();
}

function draw() {
	background(BG);

	colorMode(HSB, 1, 100, 100);
	translate(OFFSET, OFFSET);
	for (var y = 0; y < NUM_TILES; y++) {
		for (var x = 0; x < NUM_TILES; x++) {
			designSwitcher(x, y, SIZE, noise(x * TILE_FACTOR, y * TILE_FACTOR, frameCount * TIME_FACTOR));
		}
	}
}

function mouseClicked() {
	noiseSeed(millis());

	var cfg = CONFIGS[int(random(CONFIGS.length))];
	SIZE = cfg[0];
	MARGIN = cfg[1];
	SIZE_2 = SIZE / 2;
	NUM_TILES = floor(min(width, height) / SIZE);
	OFFSET = (min(width, height) - (NUM_TILES * SIZE)) / 2;
}

var NUM_TYPES = 11;

function designSwitcher(x, y, s, v) {
	// var type = int((x + y) % NUM_TYPES);

	var type = int(noise(x * TILE_FACTOR, y * TILE_FACTOR, 0) * NUM_TYPES + 1);
	var c = noise(x * COLOR_FACTOR, y * COLOR_FACTOR, 0);

	switch (type) {
		case 0:
			centerBox(x, y, s, v, c);
			break;
		case 1:
			circleBox(x, y, s, v, c);
			break;
		case 2:
			triBox(x, y, s, v, c);
			break;

		case 3:
			line_lrBox(x, y, s, v, c);
			break;
		case 4:
			line_rlBox(x, y, s, v, c);
			break;
		case 5:
			line_udBox(x, y, s, v, c);
			break;
		case 6:
			line_duBox(x, y, s, v, c);
			break;

		case 7:
			rect_ulBox(x, y, s, v, c);
			break;
		case 8:
			rect_urBox(x, y, s, v, c);
			break;
		case 9:
			rect_llBox(x, y, s, v, c);
			break;
		case 10:
			rect_lrBox(x, y, s, v, c);
			break;
	}
}

// center base figures

function centerBox(x, y, s, v, c) {
	rectMode(CENTER);
	noStroke();
	fill(c, 60, 100);
	square(SIZE_2 + x * s, SIZE_2 + y * s, s - MARGIN);
	fill(BG);
	square(SIZE_2 + x * s, SIZE_2 + y * s, (s - 2 * MARGIN) * v);
}


function circleBox(x, y, s, v, c) {
	rectMode(CENTER);
	noStroke();

	fill(c, 60, 100);
	square(SIZE_2 + x * s, SIZE_2 + y * s, s - MARGIN);

	ellipseMode(CENTER);
	fill(BG);
	circle(SIZE_2 + x * s, SIZE_2 + y * s, (s - 2 * MARGIN) * v);
}

function triBox(x, y, s, v, c) {
	rectMode(CENTER);
	noStroke();

	fill(c, 60, 100);
	square(SIZE_2 + x * s, SIZE_2 + y * s, s - MARGIN);

	ellipseMode(CENTER);
	fill(BG);
	var halfSize = SIZE_2 - MARGIN;
	var dist = halfSize - (halfSize * v);
	var xPos = x * SIZE;
	var yPos = y * SIZE;

	triangle(
		xPos + SIZE_2, yPos + MARGIN + dist,
		xPos + MARGIN + dist, yPos + SIZE - MARGIN - dist,
		xPos + SIZE - MARGIN - dist, yPos + SIZE - MARGIN - dist
	);
}

// line based figures

function line_lrBox(x, y, s, v, c) {
	rectMode(CENTER);
	noStroke();

	fill(c, 60, 100);
	square(SIZE_2 + x * s, SIZE_2 + y * s, s - MARGIN);

	rectMode(CORNER);
	fill(BG);
	rect(MARGIN + x * s, MARGIN + y * s, (s - 2 * MARGIN) * v, s - 2 * MARGIN);
}

function line_duBox(x, y, s, v, c) {
	rectMode(CENTER);
	noStroke();

	fill(c, 60, 100);
	square(SIZE_2 + x * s, SIZE_2 + y * s, s - MARGIN);

	rectMode(CORNER);
	fill(BG);
	rect(MARGIN + x * s, MARGIN + y * s, s - 2 * MARGIN, (s - 2 * MARGIN) * v);
}

function line_rlBox(x, y, s, v, c) {
	rectMode(CENTER);
	noStroke();

	fill(c, 60, 100);
	square(SIZE_2 + x * s, SIZE_2 + y * s, s - MARGIN);

	rectMode(CORNER);
	fill(BG);

	var dist = s - (s * v);
	rect(MARGIN + x * s + dist, MARGIN + y * s, s - 2 * MARGIN - dist, s - 2 * MARGIN);
}

function line_udBox(x, y, s, v, c) {
	rectMode(CENTER);
	noStroke();

	fill(c, 60, 100);
	square(SIZE_2 + x * s, SIZE_2 + y * s, s - MARGIN);

	rectMode(CORNER);
	fill(BG);

	var dist = s - (s * v);
	rect(
		MARGIN + x * s, MARGIN + y * s + dist,
		s - 2 * MARGIN, s - 2 * MARGIN - dist
	);
}

// corner based figures

function rect_ulBox(x, y, s, v, c) {
	rectMode(CENTER);
	noStroke();

	fill(c, 60, 100);
	square(SIZE_2 + x * s, SIZE_2 + y * s, s - MARGIN);

	rectMode(CORNER);
	fill(BG);
	square(MARGIN + x * s, MARGIN + y * s, (s - 2 * MARGIN) * v);
}

function rect_urBox(x, y, s, v, c) {
	rectMode(CENTER);
	noStroke();

	fill(c, 60, 100);
	square(SIZE_2 + x * s, SIZE_2 + y * s, s - MARGIN);

	rectMode(CORNER);
	fill(BG);
	var size = (s - 2 * MARGIN) * v;
	square(MARGIN + x * s + (s - size - 2 * MARGIN), MARGIN + y * s, size);
}

function rect_llBox(x, y, s, v, c) {
	rectMode(CENTER);
	noStroke();

	fill(c, 60, 100);
	square(SIZE_2 + x * s, SIZE_2 + y * s, s - MARGIN);

	rectMode(CORNER);
	fill(BG);
	var size = (s - 2 * MARGIN) * v;
	square(MARGIN + x * s, MARGIN + y * s + (s - size - 2 * MARGIN), size);
}

function rect_lrBox(x, y, s, v, c) {
	rectMode(CENTER);
	noStroke();

	fill(c, 60, 100);
	square(SIZE_2 + x * s, SIZE_2 + y * s, s - MARGIN);

	rectMode(CORNER);
	fill(BG);
	var size = (s - 2 * MARGIN) * v;
	square(MARGIN + x * s + (s - size - 2 * MARGIN), MARGIN + y * s + (s - size - 2 * MARGIN), size);
}

