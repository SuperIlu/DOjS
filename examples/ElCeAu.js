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

// SEE https://en.wikipedia.org/wiki/Elementary_cellular_automaton

var NUM_CELLS = 256;
var CELL_WIDTH = 40;
var CELL_HEIGHT = 30;

var cells = [];
var rules = [];

var lastCell = -1;

function setup() {
	createCanvas(640, 480);
	rectMode(CORNERS);
	textSize(8);
	frameRate(10);

	// create arrays of automata
	for (var num = 0; num < NUM_CELLS; num++) {
		cells.push([]);
		rules.push([]);
		create_rule(num);
	}
}

function draw() {
	for (var num = 0; num < NUM_CELLS; num++) {
		draw_cell(num);
	}
}

/**
 * draw a cell onto the screen
 */
function draw_cell(num) {
	var screenCol = (num % 16) * CELL_WIDTH;
	var screenRow = int(num / 16) * CELL_HEIGHT;

	var rightBorder = screenCol + CELL_WIDTH;
	var lowerBorder = screenRow + CELL_HEIGHT;

	stroke(32, 32, 32);
	fill(0);
	rect(screenCol, screenRow, rightBorder, lowerBorder);

	if (mouseX > screenCol && mouseX < rightBorder && mouseY > screenRow && mouseY < lowerBorder) {
		if (lastCell != num) {
			create_cell(num);
		} else {
			step_cell(num);
		}

		var cell = cells[num];
		for (var y = 0; y < CELL_HEIGHT; y++) {
			var row = cell[y];
			for (var x = 0; x < CELL_WIDTH; x++) {
				if (row[x] == 1) {
					stroke(255);
				} else {
					stroke(0);
				}
				point(screenCol + x, screenRow + y);
			}
		}

		lastCell = num;
	} else {
		fill(0, 200, 0);
		text(str(num), screenCol + 10, screenRow + 15);
	}
}

/**
 * advance the automata one step.
 */
function step_cell(num) {
	var newRow = [];

	var row = cells[num][CELL_HEIGHT - 1];
	var rule = rules[num];

	for (var x = 0; x < CELL_WIDTH; x++) {
		var leftIdx = x - 1;
		if (x < 0) {
			x = CELL_WIDTH;
		}
		var rightIdx = x + 1 % CELL_WIDTH;

		var l = row[leftIdx];
		var m = row[x];
		var r = row[rightIdx];

		if (l && m && r) {				// 111
			newRow.push(rule[7]);
		} else if (l && m && !r) {		// 110
			newRow.push(rule[6]);
		} else if (l && !m && r) {		// 101
			newRow.push(rule[5]);
		} else if (l && !m && !r) {		// 100
			newRow.push(rule[4]);
		} else if (!l && m && r) {		// 011
			newRow.push(rule[3]);
		} else if (!l && m && !r) {		// 010
			newRow.push(rule[2]);
		} else if (!l && !m && r) {		// 001
			newRow.push(rule[1]);
		} else if (!l && !m && !r) {	// 000
			newRow.push(rule[0]);
		}
	}
	cells[num].push(newRow);
	cells[num].shift();
}

/**
 * Create the data array for a single automaton.
 * Layout: data[row][column]
 */
function create_cell(num) {
	var data = [];
	for (var y = 0; y < CELL_HEIGHT; y++) {
		var row = [];
		for (var x = 0; x < CELL_WIDTH; x++) {
			row.push(0);
		}
		data.push(row);
	}

	// create random data in last row
	var lastRow = [];
	for (var x = 0; x < CELL_WIDTH; x++) {
		lastRow.push(int(random(0, 2)));
	}
	data.shift();
	data.push(lastRow);

	// store as data array
	cells[num] = data;
}

/**
 * create the result-bit pattern for given rule.
 */
function create_rule(num) {
	var rule = [];
	for (var r = 0; r < 8; r++) {
		if (((1 << r) & num) != 0) {
			rule.push(1);
		} else {
			rule.push(0);
		}
	}

	rules[num] = rule;
}
