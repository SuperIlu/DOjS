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
LoadLibrary("neural");

var RECT_SIZE = 10;
var CLICK_INC = 8;
var HIDDEN_NEURONS = 8;
var EXTRA_TRAININGS = 15;
var LEARNING_RATE = 0.3;
var MAX_COLOR = 255;

var numX;
var numY;
var rects = [];

var mouseX = 0;
var mouseY = 0;

var ann;
var trainings = 0;

function Setup() {
	MouseShowCursor(true);

	numX = Width / RECT_SIZE;
	numY = Height / RECT_SIZE;
	var numRects = numX * numY;
	for (var i = 0; i < numRects; i++) {
		rects.push(-1);
	}

	// two inputs, 1 hidden layer with 4 neurons and 1 output
	ann = new Neural(2, 1, HIDDEN_NEURONS, 1);
}

function drawRect(x, y) {
	var bX = x * RECT_SIZE + 1;
	var bY = y * RECT_SIZE + 1;
	var bXe = bX + RECT_SIZE - 2;
	var bYe = bY + RECT_SIZE - 2;

	var prediction = ann.Run([x / numX, y / numY]);
	if (prediction < 0.5) {
		FilledBox(bX, bY, bXe, bYe, Color(MAX_COLOR * (0.5 - prediction), 0, 0));
	} else {
		FilledBox(bX, bY, bXe, bYe, Color(0, MAX_COLOR * (prediction - 0.5), 0));
	}

	if (mouseX > bX && mouseX < bXe && mouseY > bY && mouseY < bYe) {
		Box(bX, bY, bXe, bYe, EGA.YELLOW);
	} else {
		var idx = y * numX + x;
		if (rects[idx] === 1) {
			Box(bX, bY, bXe, bYe, EGA.GREEN);
		} if (rects[idx] === 0) {
			Box(bX, bY, bXe, bYe, EGA.RED);
		}
	}
}

function Loop() {
	ClearScreen(EGA.BLACK);
	for (var i = 0; i < EXTRA_TRAININGS; i++) {
		for (var x = 0; x < numX; x++) {
			for (var y = 0; y < numY; y++) {
				var idx = y * numX + x;
				if (rects[idx] === 1 || rects[idx] === 0) {	// only train selected rects...
					ann.Train([x / numX, y / numY], [rects[idx]], LEARNING_RATE);
				}
			}
		}
		trainings++;
	}

	for (var x = 0; x < numX; x++) {
		for (var y = 0; y < numY; y++) {
			drawRect(x, y);
			var idx = y * numX + x;
			if (rects[idx] === 1 || rects[idx] === 0) {	// only train selected rects...
				ann.Train([x / numX, y / numY], [rects[idx]], LEARNING_RATE);
			}
		}
	}
	trainings++;
}

function Input(e) {
	mouseX = e.x;
	mouseY = e.y;

	if (e.buttons & MOUSE.Buttons.LEFT) {
		var x = Math.floor(mouseX / RECT_SIZE);
		var y = Math.floor(mouseY / RECT_SIZE);
		var idx = y * numX + x;
		rects[idx] = 1;
		Println("rects[" + idx + "]= 1;");
	} else if (CompareKey(e.key, ' ') || (e.buttons & MOUSE.Buttons.MIDDLE)) {
		var x = Math.floor(mouseX / RECT_SIZE);
		var y = Math.floor(mouseY / RECT_SIZE);
		var idx = y * numX + x;
		rects[idx] = -1;
	} else if (e.buttons & MOUSE.Buttons.RIGHT) {
		var x = Math.floor(mouseX / RECT_SIZE);
		var y = Math.floor(mouseY / RECT_SIZE);
		var idx = y * numX + x;
		rects[idx] = 0;
	}

	var new_hLayers = 0;
	if (CompareKey(e.key, '1')) {
		new_hLayers = 1;
	} else if (CompareKey(e.key, '2')) {
		new_hLayers = 2;
	} else if (CompareKey(e.key, '3')) {
		new_hLayers = 3;
	} else if (CompareKey(e.key, '4')) {
		new_hLayers = 4;
	} else if (CompareKey(e.key, '5')) {
		new_hLayers = 5;
	} else if (CompareKey(e.key, '6')) {
		new_hLayers = 6;
	} else if (CompareKey(e.key, '7')) {
		new_hLayers = 7;
	} else if (CompareKey(e.key, 'i')) {
		var correct = 0;
		var falsePositive = 0;
		var falseNegative = 0;
		for (var x = 0; x < numX; x++) {
			for (var y = 0; y < numY; y++) {
				var prediction = ann.Run([x / numX, y / numY]);
				if (prediction < 0.5) {
					prediction = 0;
				} else {
					prediction = 1;
				}

				var idx = y * numX + x;
				if (prediction == rects[idx]) {
					correct++;
				} else {
					if (prediction) {
						falsePositive++;
					} else {
						falseNegative++;
					}
				}
			}
		}
		Println("\nModel     : " + JSON.stringify(ann));
		Println("Trainings : " + trainings);
		Println("Correct   : " + correct);
		Println("False pos : " + falsePositive);
		Println("False neg : " + falseNegative);
	}

	if (new_hLayers > 0) {
		ann.Close();
		ann = new Neural(2, new_hLayers, HIDDEN_NEURONS, 1);
		trainings = 0;
	}
}
