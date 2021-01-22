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

/*
Wie sieht das ganze jetzt in DOjS aus?
Ihr seht jetzt hier den Editor von DOjS mit dem Quellcode des Sketch abwechselnd mit dem laufenden Sketch.
In dem Fall habe ich das Beispiel von p5js benutzt, eine Zeile veraendert und jetzt laeuft der Originalsketch direkt unter MS-DOS.

*/

var dotPos = 10;
var inc = true;
var fCount = 0;
var edit;

exports.prepare = function () {
	edit = loadImage("examples/DOjS/addwav.bmp");

	textAlign(LEFT, TOP);
	imageMode(CORNER);
	wavSetup();

	fCount = 0;
};

exports.present = function () {
	var yPos = 10;

	background(0);

	colorMode(RGB);
	fill(222);
	textFont(fontHeader);
	text("P5JS example", 10, yPos);
	yPos += fontHeader.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);

	colorMode(RGB);
	fill(222, 0, 0, 128);
	noStroke();
	circle(dotPos, yPos, 10);
	if (inc) {
		if (dotPos < width - 10) {
			dotPos += 2;
		} else {
			inc = false;
		}
	} else {
		if (dotPos > 10) {
			dotPos -= 2;
		} else {
			inc = true;
		}
	}
	yPos += 5;

	colorMode(RGB);
	fill(222);
	textFont(fontSubheader);
	text("Addwave", 10, yPos);

	wavDraw();
	if (floor(fCount / 200) % 2 == 0) {
		image(edit, 0, 80);
	}
	fCount++;
};

exports.keyHook = function (key) { };
exports.exitHook = function () {
	edit = null;
};

xspacing = 8;   // How far apart should each horizontal location be spaced
w = 0;              // Width of entire wave
maxwaves = 4;   // total # of waves to add together

theta = 0.0;
amplitude = new Array(maxwaves);   // Height of wave
dx = new Array(maxwaves);          // Value for incrementing X, to be calculated as a function of period and xspacing
yvalues = 0;                           // Using an array to store height values for the wave (not entirely necessary)

function wavSetup() {
	colorMode(RGB, 255, 255, 255, 100);
	w = width + 16;

	for (var i = 0; i < maxwaves; i++) {
		amplitude[i] = (random(10, 30));
		var period = random(100, 300); // How many pixels before the wave repeats
		dx[i] = ((Math.PI * 2 / period) * xspacing);
	}

	yvalues = new Array(w / xspacing);
}

function wavDraw() {
	calcWave();
	renderWave();
}

function calcWave() {
	// Increment theta (try different values for 'angular velocity' here
	theta += 0.02;

	// Set all height values to zero
	for (var i = 0; i < yvalues.length; i++) {
		yvalues[i] = 0;
	}

	// Accumulate wave height values
	for (var j = 0; j < maxwaves; j++) {
		var x = theta;
		for (var i = 0; i < yvalues.length; i++) {
			// Every other wave is cosine instead of sine
			if (j % 2 == 0) {
				yvalues[i] += Math.sin(x) * amplitude[j];
			}
			else {
				yvalues[i] += Math.cos(x) * amplitude[j];
			}
			x += dx[j];
		}
	}
}

function renderWave() {
	// A simple way to draw the wave with an ellipse at each location
	noStroke();
	fill(255, 50);
	for (var x = 0; x < yvalues.length; x++) {
		circle(x * xspacing, SizeY() / 2 + yvalues[x], 16);
	}
}
