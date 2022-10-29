/*
licensed under MIT license or Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0).

MIT License

Copyright (c) 2022 Andre Seidelt <superilu@yahoo.com>

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

var RECT_SIZE = 30;
var RECT_SPACING = 10;
var RECT_TOTAL = RECT_SIZE + RECT_SPACING;
var RECT_SIZE_2 = RECT_SIZE / 2;
var FIRST_ROW = 1;
var MAX_VALUE_DISTANCE = 13;
var minExponent = 0;
var maxExponent = 9;
var fields = [];
var grid = [];
var xSize = 0;
var ySize = 0;
var selectedFields = [];
var score = 0;
var lastSeen = null;
var gameOver = false;
var totalFields = 0;

/**
 * p5js setup
 */
function setup() {
	createCanvas(windowWidth, windowHeight);

	RECT_SIZE = min(windowWidth, windowHeight) / 12;
	RECT_TOTAL = RECT_SIZE + RECT_SPACING;
	RECT_SIZE_2 = RECT_SIZE / 2;
	textSize(RECT_SIZE / 4);

	xSize = int(width / RECT_TOTAL);
	ySize = int(height / RECT_TOTAL);
	totalFields = xSize * (ySize - 1);
	// print("Field size = " + xSize + "/" + ySize + ", totalFields=" + totalFields);

	for (var y = 0; y < ySize; y++) {
		grid.push([]);
		for (var x = 0; x < xSize; x++) {
			grid[y].push(null);
		}
	}
}

/**
 * p5js draw
 */
function draw() {
	if (gameOver) {
		background(32);
		textAlign(CENTER, CENTER);
		textSize(height / 6);
		colorMode(RGB);
		stroke(255, 0, 0);
		fill(128, 0, 0);
		text("Game Over", width / 2, height / 2 - height / 4);

		textSize(height / 8);
		text("no more moves", width / 2, height / 2 + height / 4);

		textSize(RECT_SIZE / 4);
	} else {
		background(32);

		if (!slideDownAndRefill()) {
			// only handle selection if no sliding was done
			for (var i = 0; i < fields.length; i++) {
				fields[i].Update();
			}
			if (!mouseIsPressed && selectedFields.length > 0) {
				processSelection();
				selectedFields = [];
				calcMaxValue();
			}
		}

		colorMode(RGB);
		stroke(255);
		strokeWeight(RECT_SPACING);
		for (var i = 1; i < selectedFields.length; i++) {
			line(
				selectedFields[i - 1].xStart + RECT_SIZE_2,
				selectedFields[i - 1].yStart + RECT_SIZE_2,
				selectedFields[i].xStart + RECT_SIZE_2,
				selectedFields[i].yStart + RECT_SIZE_2
			);
		}

		for (var i = 0; i < fields.length; i++) {
			fields[i].Draw();
		}
	}

	colorMode(RGB);
	stroke(255);
	fill(255);
	strokeWeight(1);
	textAlign(LEFT, CENTER);
	text("SCORE  " + score, RECT_SIZE_2, RECT_SIZE_2);
	text("CURRENT  " + calcCurrent(), RECT_SIZE * 5, RECT_SIZE_2);
	text("MAX  " + makeLabel(maxExponent), RECT_SIZE * 10, RECT_SIZE_2);
}

/**
 * iterate over all fields and check if there are any left that have a valid neighbor to start a selection.
 */
function checkGameOver() {
	for (var i = 0; i < fields.length; i++) {
		if (hasValidNeighbor(fields[i])) {
			gameOver = false;
			return;
		}
	}
	gameOver = true;
}

/**
 * check if one of the adjacent fields have the same value as this one.
 * 
 * @param {Field} cur the field
 * 
 * @returns {Boolean} true if a neighbor has the same value, else false.
 */
function hasValidNeighbor(cur) {
	return (getFieldValue(cur.x - 1, cur.y - 1) === cur.value) ||
		(getFieldValue(cur.x, cur.y - 1) === cur.value) ||
		(getFieldValue(cur.x + 1, cur.y - 1) === cur.value) ||
		(getFieldValue(cur.x - 1, cur.y) === cur.value) ||
		(getFieldValue(cur.x + 1, cur.y) === cur.value) ||
		(getFieldValue(cur.x - 1, cur.y + 1) === cur.value) ||
		(getFieldValue(cur.x, cur.y + 1) === cur.value) ||
		(getFieldValue(cur.x + 1, cur.y + 1) === cur.value);
}

/**
 * get field value.
 * 
 * @param {*} x y pos
 * @param {*} y x pos
 * 
 * @returns the value of the field at that coordinate or -1 if invalid coordinate.
 */
function getFieldValue(x, y) {
	var f = getField(x, y);
	if (f) {
		return f.value;
	} else {
		return -1;
	}
}

/**
 * slide down all Fields where there is an empty spot below. Keep adding new fields on top as long as there is space.
 * 
 * @returns {Boolean} true if the field was modified, else false.
 */
function slideDownAndRefill() {
	var changed = false;

	if (fields.length !== totalFields) {
		// slide down all fields that have space below them (loop works bottom up)
		for (var y = ySize - 1; y >= FIRST_ROW; y--) {
			for (var x = 0; x < xSize; x++) {
				if (getField(x, y) === null) {
					var above = getField(x, y - 1);
					if (above) {
						setField(x, y, above);
						setField(x, y - 1, null);
						changed = true;
					}
				}
			}
		}

		// add new fields if there are empty spots in the first row
		if (!changed) {
			for (var x = 0; x < xSize; x++) {
				if (getField(x, FIRST_ROW) === null) {
					addField(new Field(x, FIRST_ROW, int(random(minExponent, maxExponent - 2))));
					changed = true;
				}
			}
		}
	}

	if (!changed && lastDrawSlide) {
		// we finished sliding, now check for game over and update min/max values
		calcMaxValue();
		checkGameOver();
	}

	lastDrawSlide = changed;

	return changed;
}

/**
 * calculate the value resulting from the current selection.
 * 
 * @returns the resultingg value when assembling the current selection.
 */
function calcCurrent() {
	if (selectedFields.length > 1) {
		var newVal = 0;
		for (var i = 0; i < selectedFields.length; i++) {
			newVal += pow(2, selectedFields[i].value);
		}
		return pow(2, ceil(log2(newVal)));
	} else {
		return 0;
	}
}

/**
 * summarize the current selection because the mouse button was released.
 * Intermediate fields are removed from the field, the sum is put into the last field.
 * 
 * @returns true if the selection yielded a change, else false.
 */
function processSelection() {
	if (selectedFields.length > 1) {
		var newVal = 0;
		while (selectedFields.length > 1) {
			var first = selectedFields.shift();
			newVal += pow(2, first.value);
			remField(first);
		}
		newVal += pow(2, selectedFields[0].value);
		score += newVal;
		selectedFields[0].value = ceil(log2(newVal));
		return true;
	} else {
		return false;
	}
}

/**
 * calculate the logarith to the base of 2.
 * 
 * @param {number} x the number to calculate log2 on.
 * 
 * @returns logarithm to base 2.
 */
function log2(x) {
	return Math.log(x) / Math.log(2);
}

/**
 * calculate the highes and lowest value in the grid
 */
function calcMaxValue() {
	while (true) {
		minExponent = 0xFFFF;
		maxExponent = 0;
		fields.forEach(function (e) {
			if (e.value > maxExponent) {
				maxExponent = e.value;
			}
			if (e.value < minExponent) {
				minExponent = e.value;
			}
		});

		if (maxExponent - minExponent < MAX_VALUE_DISTANCE) {
			break;
		} else {
			fields.forEach(function (e) {
				if (e.value == minExponent) {
					e.value++;
				}
			});
		}
	}
}

/**
 * convert field value to a readable string.
 * 
 * @param {number} v the Field value
 * 
 * @returns {string} a string to display
 */
function makeLabel(v) {
	v = pow(2, v);
	if (v <= 1024) {
		return str(v);
	} else if (v <= 1048576) {
		return str(v / 1024) + "k";
	} else {
		return str(v / 1048576) + "m";
	}
}

/**
 * Add a new Field to the game.
 * 
 * @param {Field} f the new Field to add.
 */
function addField(f) {
	if (fields.indexOf(f) === -1) {
		fields.push(f);
	}
	grid[f.y][f.x] = f;
}

/**
 * remove Field from game.
 * 
 * @param {Field} f the Field to remove
 */
function remField(f) {
	var idx = fields.indexOf(f);
	if (idx !== -1) {
		fields.splice(idx, 1);
	}
	grid[f.y][f.x] = null;
}

/**
 * get Field at specified position in the grid.
 * 
 * @param {number} x x position.
 * @param {number} y y position
 * 
 * @returns {Field} the field at that coordinate or null if none or illegal grid coordinate.
 */
function getField(x, y) {
	if ((x >= 0) && (y >= FIRST_ROW) && (x < xSize) && (y < ySize)) {
		return grid[y][x];
	} else {
		return null;
	}
}

/**
 * Set a Field a a specific position in the grid. The position of the Field is updated.
 * 
 * @param {number} x new x position
 * @param {number} y new y position
 * @param {Field} f Field or null
 */
function setField(x, y, f) {
	if ((x >= 0) && (y >= FIRST_ROW) && (x < xSize) && (y < ySize)) {
		if (f) {
			f.x = x;
			f.y = y;
			f.CalcCoordinates();
		}
		grid[y][x] = f;
	}
}

/**
 * check if a field can be added to the list of selected fields according to game rules.
 * 
 * @param {Field} cur the field considered for adding
 * 
 * @returns {Boolean} true if the field shall be added, else false.
 */
function checkRules(cur) {
	if (selectedFields.length === 0) {
		// if the list ist empty, just add
		return true;
	} else {
		var prev = selectedFields[selectedFields.length - 1];
		if (selectedFields.length === 1) {
			// if there is already a field in the list, make sure the new one is a neighbor and of the same value as the one already in the list
			return isNeighbor(cur, prev) && (prev.value === cur.value);
		} else {
			// in all other cases: make sure the new one is a neighbor and of the same value or one more as the one already in the list
			return isNeighbor(cur, prev) && ((prev.value === cur.value) || (prev.value === cur.value - 1));
		}
	}
}

/**
 * check if the two Fields are adjacent.
 * 
 * @param {Field} cur the currently added field
 * @param {Field} prev the last added field
 * 
 * @returns {Boolean} true if they are neighbors, else false.
 */
function isNeighbor(cur, prev) {
	return (getField(cur.x - 1, cur.y - 1) === prev) ||
		(getField(cur.x, cur.y - 1) === prev) ||
		(getField(cur.x + 1, cur.y - 1) === prev) ||
		(getField(cur.x - 1, cur.y) === prev) ||
		(getField(cur.x + 1, cur.y) === prev) ||
		(getField(cur.x - 1, cur.y + 1) === prev) ||
		(getField(cur.x, cur.y + 1) === prev) ||
		(getField(cur.x + 1, cur.y + 1) === prev);
}

/**
 * create a new field with given value and position.
 * 
 * @param {number} x x pos
 * @param {number} y y pos
 * @param {number} v the value.
 */
function Field(x, y, v) {
	this.x = x;
	this.y = y;
	this.value = v;
	this.selected = false;
	this.mouseOver = false;
	this.CalcCoordinates();
}

/**
 * helper to alculate screen position from grid position and store it.
 */
Field.prototype.CalcCoordinates = function () {
	this.xStart = this.x * RECT_TOTAL;
	this.yStart = this.y * RECT_TOTAL;
	this.xEnd = this.xStart + RECT_SIZE;
	this.yEnd = this.yStart + RECT_SIZE;
};

/**
 * handles mouse-over and selection of a field.
 */
Field.prototype.Update = function () {
	// highlight field if mouseover
	this.mouseOver = (mouseX > this.xStart) && (mouseX < this.xEnd) && (mouseY > this.yStart) && (mouseY < this.yEnd);

	this.selected = selectedFields.indexOf(this) !== -1;
	if (this.mouseOver) {
		if (this.mouseOver && mouseIsPressed && (selectedFields.indexOf(this) === -1)) {
			// the mouse is over this field, it is NOT in the selectedFields and the mouse is still pressed
			var prev = selectedFields[selectedFields.length - 1];
			if (checkRules(this)) {
				selectedFields.push(this);
				this.selected = true;
			}
		} else if ((lastSeen != this) && (selectedFields.length > 1) && (selectedFields[selectedFields.length - 2] == this)) {
			// check if second to last is this, then deselect the last
			selectedFields.pop();
		}
		lastSeen = this;
	}
};

/**
 * draws a field to the screen
 */
Field.prototype.Draw = function () {
	var xPos = this.x * RECT_TOTAL;
	var yPos = this.y * RECT_TOTAL;

	colorMode(HSB);
	var hue = map(this.value, minExponent, maxExponent, 0, 300);
	var colB = color(hue, 100, 100);
	var colH = color(hue, 50, 50);

	if (this.mouseOver) {
		stroke("white");
		strokeWeight(2);
	} else {
		stroke(colB);
		strokeWeight(1);
	}
	fill(colH);
	rect(xPos, yPos, RECT_SIZE, RECT_SIZE, RECT_SIZE / 4);

	colorMode(RGB);
	if (this.selected) {
		stroke(255, 0, 0);
		fill(255, 0, 0);
	} else {
		stroke(255);
		fill(255);
	}
	strokeWeight(1);
	textAlign(CENTER, CENTER);
	text(makeLabel(this.value), xPos + RECT_SIZE_2, yPos + RECT_SIZE_2);
};
