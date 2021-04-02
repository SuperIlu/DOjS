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
Include("console");
Include("evchain");

var con;
var chain;
var fontList;
var fontSize;
var fontSizeSquared;
var letterList = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
var dataset;
var trainingSet;
var validationSet;
var hiddenNeurons;

var ann;
var LEARNING_RATE = 0.3;
var REPETITIONS = 20;
var TRAIN_SET = 0.8;
var VAL_SUCCESS = 0.9;

function Setup() {
	MouseShowCursor(false);
	con = new Console();
	chain = new EvalChain();

	msg("neural2 starting...");
	chain.Add(function () {
		var allFontSizes = getFontSizes();
		chain.Add(function () {
			fontSize = findMostFonts(allFontSizes);
			fontSizeSquared = fontSize * fontSize;
			fontList = allFontSizes[fontSize];
			var tmp = Math.floor(Math.sqrt((fontSizeSquared + letterList.length) / 2));
			hiddenNeurons = tmp * tmp;
			msg("Created ANN[" + fontSizeSquared + "/" + hiddenNeurons + "/" + letterList.length + "]");
			ann = new Neural(fontSizeSquared, 1, hiddenNeurons, letterList.length);
			chain.Add(function () {
				createDataset();
				chain.Add(function () {
					splitDataset();
					trainLoop();
				});
			});
		});
	});
}

function trainLoop() {
	chain.Add(function () {
		var sw = new StopWatch();
		sw.Start();
		for (var r = 0; r < REPETITIONS; r++) {
			train();
		}
		sw.Stop();
		msg(REPETITIONS + " training repetitions: " + sw.Result());
	});
	chain.Add(function () {
		ann.Save("ann." + REPETITIONS);
		msg("ANN saved...");
	});
	chain.Add(function () {
		var succ = evaluateNetwork();
		if (succ < VAL_SUCCESS) {
			trainLoop();
		} else {
			drawNetwork(0);
		}
	});
}

function drawArrayData(dat, xPos, yPos) {
	var size = Math.sqrt(dat.length);
	for (var y = 0; y < size; y++) {
		for (var x = 0; x < size; x++) {
			Plot(xPos + x, yPos + y, dat[y * size + x]);
		}
	}
}

function drawNetwork(idx) {
	ClearScreen(EGA.BLACK);
	if (idx >= validationSet.length) {
		idx = 0;
	}

	var res = ann.Run(validationSet[idx][0]);
	var internals = ann.GetAllData();
	var greys = internals.map(function (x) { return Color(x * 255); });
	var inp = greys.slice(0, fontSizeSquared);
	var hid = greys.slice(fontSizeSquared, fontSizeSquared + hiddenNeurons);
	// var outp = greys.slice(fontSizeSquared + hiddenNeurons);

	drawArray(inp, Width / 2 - 20, 100);
	drawArray(hid, Width / 2 + 20, 100);
	// drawArray(outp);

	var maxIdx = indexOfMax(res);

	//TextXY(10, Height / 2, letterList.charAt(validationSet[idx][2]) + ":", EGA.YELLOW);
	validationSet[idx][3].Draw(0, Height / 2);
	for (var l = 0; l < letterList.length; l++) {
		var col = Color(res[l] * 255);
		if (l === maxIdx) {
			if (validationSet[idx][2] === maxIdx) {
				col = EGA.GREEN;
			} else {
				col = EGA.RED;
			}
		}
		TextXY(20 + l * 10, Height / 2, letterList.charAt(l), col);
	}

	Sleep(1000);
	chain.Add(function () { drawNetwork(idx + 1); });
}

function Loop() {
	con.Draw(true);
	chain.Step();
}

function Input(e) { }

function evaluateNetwork() {
	var trainSucc = 0;
	for (var i = 0; i < trainingSet.length; i++) {
		var res = ann.Run(trainingSet[i][0]);
		var idx = indexOfMax(res);
		if (idx === trainingSet[i][2]) {
			trainSucc++;
		}
	}
	msg("Data: Network detected " + trainSucc + "/" + trainingSet.length + " := " + (trainSucc / trainingSet.length * 100).toFixed(2) + "%", EGA.YELLOW);

	var valSucc = 0;
	for (var i = 0; i < validationSet.length; i++) {
		var res = ann.Run(validationSet[i][0]);
		var idx = indexOfMax(res);
		if (idx === validationSet[i][2]) {
			valSucc++;
		}
	}
	msg("Validaton: Network detected " + valSucc + "/" + validationSet.length + " := " + (valSucc / validationSet.length * 100).toFixed(2) + "%", EGA.RED);

	return valSucc / validationSet.length;
}

function indexOfMax(arr) {
	if (arr.length === 0) {
		return -1;
	}

	var max = arr[0];
	var maxIndex = 0;

	for (var i = 1; i < arr.length; i++) {
		if (arr[i] > max) {
			maxIndex = i;
			max = arr[i];
		}
	}

	return maxIndex;
}

function train() {
	for (var d = 0; d < trainingSet.length; d++) {
		ann.Train(trainingSet[d][0], trainingSet[d][1], LEARNING_RATE);
	}
}

function splitDataset() {
	var sw = new StopWatch();
	sw.Start();
	var trainSize = Math.ceil(dataset.length * TRAIN_SET);
	trainingSet = [];
	while (trainingSet.length < trainSize) {
		// randomly move an element from dataset to trainingSet
		trainingSet.push(dataset.splice(getRandomInt(dataset.length), 1)[0]);
	}
	validationSet = dataset;
	sw.Stop();
	msg("Randomized training set: " + sw.Result());

	msg("Training size  : " + trainingSet.length);
	msg("Validation size: " + validationSet.length);
}

function getRandomInt(max) {
	return Math.floor(Math.random() * Math.floor(max));
}

function createDataset() {
	var sw = new StopWatch();
	dataset = [];

	sw.Start();
	for (var f = 0; f < fontList.length; f++) {
		for (var l = 0; l < letterList.length; l++) {
			//         A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z
			var res = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
			res[l] = 1;
			var bm = createLetter(fontList[f], letterList.charAt(l));
			var ar = letterToArray(bm);
			dataset.push([ar, res, l, bm]);
		}
	}
	sw.Stop();
	msg("Training set size: " + dataset.length);
	msg("Create training set: " + sw.Result());
}

function letterToArray(bm) {
	var ret = new DoubleArray();

	for (var y = 0; y < bm.height; y++) {
		for (var x = 0; x < bm.width; x++) {
			if (bm.GetPixel(x, y) & 0xFF) {
				ret.Push(0);
			} else {
				ret.Push(1);
			}
		}
	}

	return ret;
}

function createLetter(font, letter) {
	var bm = new Bitmap(fontSize, fontSize);
	SetRenderBitmap(bm);
	ClearScreen(EGA.WHITE);
	font.DrawStringCenter(fontSize / 2, 0, letter, EGA.BLACK, EGA.WHITE);
	SetRenderBitmap(null);

	return bm;
}

function findMostFonts(fontSizes) {
	var num = 0;
	var size = 0;

	for (e in fontSizes) {
		if (fontSizes[e].length > num) {
			num = fontSizes[e].length;
			size = e;
		}
	}
	msg("Most (" + num + ") fonts are size " + size);
	return parseInt(size);
}

function getFontSizes() {
	var ret = {};

	msg("Determining font sizes...");
	var jsboot = new Zip("jsboot.zip", ZIPFILE.READ);
	var entries = jsboot.GetEntries();
	jsboot.Close();

	for (var i = 0; i < entries.length; i++) {
		if (entries[i].name.endsWith(".fnt") || entries[i].name.endsWith(".FNT")) {
			var fnt = new Font("jsboot.zip=" + entries[i].name);
			var fntHeight = fnt.height.toString();

			if (fntHeight in ret) {
				ret[fntHeight].push(fnt);
			} else {
				ret[fntHeight] = [fnt];
			}
		}
	}

	return ret;
}

function msg(s, c) {
	Println(s);
	con.Log(s, c);
}

function dump(s) {
	Println(JSON.stringify(s));
}
