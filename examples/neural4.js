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

var IMG_SIZE = 32;
var MAX_ELEMENTS = 10;
var LEARNING_RATE = 0.3;
var EXTRA_TRAININGS = 1;

var inImg;
var outImg;
var ann;
var train;
var factor;

function Setup() {
	MouseShowCursor(false);

	train = true;
	inImg = new Bitmap(IMG_SIZE, IMG_SIZE);
	outImg = new Bitmap(IMG_SIZE, IMG_SIZE);

	var numPixel = (IMG_SIZE * IMG_SIZE) / 2;

	// create ANN with input, one hidden and output layer with half the number of pixels of the images
	ann = new Neural(numPixel, 1, numPixel, numPixel);
	createTestInput();

	factor = Math.floor(Height / IMG_SIZE);
}

function Loop() {
	ClearScreen(EGA.DARK_GREY);
	createSimpleInput();

	//inImg.Draw(Width / 4 - IMG_SIZE / 2, Height / 2 - IMG_SIZE / 2);
	inImg.Draw(0, 0);
	var inValues = trainNetwork();
	createOutput(inValues);
	// outImg.Draw(3 * (Width / 4) - IMG_SIZE / 2, Height / 2 - IMG_SIZE / 2);
	outImg.DrawAdvanced(
		0, 0, IMG_SIZE, IMG_SIZE,
		IMG_SIZE, IMG_SIZE, IMG_SIZE * factor, IMG_SIZE * factor);

	if (train) {
		Box(0, 0, IMG_SIZE, IMG_SIZE, EGA.GREEN);
	}
}

function Input(e) {
	if (CompareKey(e.key, ' ')) {
		train = !train;
	}
}

function createInput() {
	SetRenderBitmap(inImg);
	ClearScreen(EGA.BLACK);

	var numCircles = GetRandomInt(MAX_ELEMENTS);
	for (var i = 0; i < numCircles; i++) {
		var col = Color(GetRandomInt(255));
		var x = GetRandomInt(IMG_SIZE);
		var y = GetRandomInt(IMG_SIZE);
		var r = GetRandomInt(IMG_SIZE / 4);
		Circle(x, y, r, col);
	}

	var numRects = GetRandomInt(MAX_ELEMENTS);
	for (var i = 0; i < numRects; i++) {
		var col = Color(GetRandomInt(255));
		var x = GetRandomInt(IMG_SIZE);
		var y = GetRandomInt(IMG_SIZE);
		var w = GetRandomInt(IMG_SIZE / 4);
		var h = GetRandomInt(IMG_SIZE / 4);
		Box(x, y, x + w, y + h, col);
	}

	SetRenderBitmap(null);
}

function createTestInput() {
	SetRenderBitmap(inImg);
	ClearScreen(EGA.BLACK);

	Box(IMG_SIZE / 2 - 8, IMG_SIZE / 2 - 8, IMG_SIZE / 2 + 8, IMG_SIZE / 2 + 8, EGA.WHITE);

	SetRenderBitmap(null);
}

function createSimpleInput() {
	SetRenderBitmap(inImg);
	ClearScreen(EGA.BLACK);

	var decide = GetRandomInt(2);

	var col = Color(GetRandomInt(127) + 127);
	if (decide == 1) {
		var r = GetRandomInt(IMG_SIZE / 4) + 2;
		Circle(IMG_SIZE / 2, IMG_SIZE / 2, r, col);
	} else {
		var w = GetRandomInt(IMG_SIZE / 4) + 2;
		var h = GetRandomInt(IMG_SIZE / 4) + 2;
		Box(IMG_SIZE / 2 - w, IMG_SIZE / 2 - h, IMG_SIZE / 2 + w, IMG_SIZE / 2 + h, col);
	}

	SetRenderBitmap(null);
}

function trainNetwork() {
	var inValues = new DoubleArray();
	for (var y = 0; y < IMG_SIZE / 2; y++) {
		for (var x = 0; x < IMG_SIZE; x++) {
			inValues.Push(colorToNn(inImg.GetPixel(x, y)));
		}
	}

	if (train) {
		var outValues = new DoubleArray();
		for (var y = IMG_SIZE / 2; y < IMG_SIZE; y++) {
			for (var x = 0; x < IMG_SIZE; x++) {
				outValues.Push(colorToNn(inImg.GetPixel(x, y)));
			}
		}

		for (var i = 0; i < EXTRA_TRAININGS; i++) {
			ann.Train(inValues, outValues, LEARNING_RATE);
		}
	}

	return inValues;
}

function colorToNn(px) {
	return (px & 0xFF) / 255;
}

function nnToColor(nn) {
	return Color(nn * 255, 8, 8);
}

function createOutput(inValues) {
	var res = ann.Run(inValues);
	SetRenderBitmap(outImg);
	inImg.Draw(0, 0);

	var x = 0;
	var y = IMG_SIZE / 2;
	for (var i = 0; i < res.length; i++) {
		Plot(x, y, nnToColor(res[i]));
		x++;
		if (x >= IMG_SIZE) {
			x = 0;
			y++;
		}
	}

	SetRenderBitmap(null);
}
function createOutputTest(inValues) {
	SetRenderBitmap(outImg);
	inImg.Draw(0, 0);

	var x = 0;
	var y = IMG_SIZE / 2;
	for (var i = 0; i < inValues.length; i++) {
		Plot(x, y, nnToColor(inValues.Get(i)));
		x++;
		if (x >= IMG_SIZE) {
			x = 0;
			y++;
		}
	}

	SetRenderBitmap(null);
}
