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

var COLOR_INCREASE = 32;
var NUM_CHILDREN = 0.8; // higher values result in more children
var NEW_TREE = 100;
var BORDER_SIZE = 2;
var CHILD_PROB = 10; // higher values increase propability for children
var tree = null;

function setup() {
	createCanvas(windowWidth, windowHeight);
	background(100);
}

function draw() {
	if (!tree || frameCount % NEW_TREE == 0) {
		// CHILD_PROB = 15 * Math.random();
		// NUM_CHILDREN = 0.8 * Math.random();
		// BORDER_SIZE = random(1, 5);
		tree = createTree(1.0, true, 0);
	}

	background(0);
	colorMode(HSB);
	drawTree(tree, 0, 0, 0, width - 1, height - 1);
}

function createTree(amount, createChildren, level) {
	var ret = {
		'size': amount,
		'entries': [],
		'isDir': createChildren
	};

	if (createChildren) {
		var rest = amount;
		var keepRunning = true;
		while (keepRunning) {
			var chSize = Math.random() / (NUM_CHILDREN * level);
			if (chSize > rest) {
				chSize = rest;
				keepRunning = false;
			}

			ret.entries.push(createTree(chSize, Math.random() / CHILD_PROB < 1 / level, level + 1));

			rest -= chSize
		}
	}

	return ret;
}

function drawTree(cur, level, x, y, w, h) {
	stroke(level * COLOR_INCREASE % 255, 255, 255);
	fill(level * COLOR_INCREASE % 255, 32, 32, 32);
	//noFill();
	if (level % 2 == 0) {
		// horizontally
		var xPos = x;
		cur.entries.forEach(function (e) {
			var eWidth = map(e.size, 0, cur.size, 0, w);
			if (eWidth < 1) {
				// if we are to small just increment start position and don't draw anything
				xPos += eWidth;
				return;
			}
			rect(xPos, y, eWidth, h);
			if (e.isDir) {
				drawTree(e, level + 1, xPos + BORDER_SIZE, y + BORDER_SIZE, eWidth - 2 * BORDER_SIZE, h - 2 * BORDER_SIZE);
			}
			xPos += eWidth;
		});
	} else {
		// vertically
		var yPos = y;
		cur.entries.forEach(function (e) {
			var eHeight = map(e.size, 0, cur.size, 0, h);
			if (eHeight < 1) {
				// if we are to small just increment start position and don't draw anything
				yPos += eHeight;
				return;
			}
			rect(x, yPos, w, eHeight);
			if (e.isDir) {
				drawTree(e, level + 1, x + BORDER_SIZE, yPos + BORDER_SIZE, w - 2 * BORDER_SIZE, eHeight - 2 * BORDER_SIZE);
			}
			yPos += eHeight;
		});
	}
}
