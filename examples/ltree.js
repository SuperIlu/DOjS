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

var allPoints;
var Y_SPREAD = 8;
var C_INC = 8;
var SPLIT_LIMITER = 5;
var ALPHA = 0.5
var SPLIT_FACTOR = 3;

function setup() {
	createCanvas(windowWidth, windowHeight);
	frameRate(10);
	noFill();
	colorMode(HSB);
	reStart();
}

function reStart() {
	allPoints = [];
	// create start point
	allPoints.push([{
		'color': 0,
		'old': {
			'x': 0,
			'y': height / 2,
		},
		'new': {
			'x': 0,
			'y': height / 2,
		}
	}]);
}

function draw() {
	background(0, 0, 0);

	for (var i = 0; i < allPoints.length; i++) {
		var p = allPoints[i];
		var lastPoint = p[p.length - 1];

		if (lastPoint.new.x < width) {
			drawPoint(p);
			p.push(iteratePoint(lastPoint));
		} else {
			reStart();
			return;
		}
	}
}

function drawPoint(p) {
	stroke(p[0].color, 255, 255, ALPHA);
	circle(p[0].new.x, p[0].new.y, 2);

	for (var i = 0; i < p.length; i++) {
		line(p[i].old.x, p[i].old.y, p[i].new.x, p[i].new.y);
	}
}

function iteratePoint(p) {
	splitPoint(p);

	return {
		'color': p.color,
		'old': {
			'x': p.new.x,
			'y': p.new.y,
		},
		'new': {
			'x': p.new.x + random(2, 5),
			'y': p.new.y + random(-Y_SPREAD, Y_SPREAD + 1),
		}
	};
}

function splitPoint(p) {
	if (random(width) + allPoints.length * SPLIT_LIMITER < p.new.x) {
		// do a split
		allPoints.push([{
			'color': p.color + C_INC,
			'old': {
				'x': p.new.x,
				'y': p.new.y,
			},
			'new': {
				'x': p.new.x,
				'y': p.new.y + random(-SPLIT_FACTOR * Y_SPREAD, SPLIT_FACTOR * Y_SPREAD + 1),
			}
		}]);
		return true;
	}

	return false;
}