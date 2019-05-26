/*
MIT License

Copyright (c) 2019 Andre Seidelt <superilu@yahoo.com>

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

TEXT2 = [
	"Runs on MS-DOS, Win9x, FreeDOS, DOSBox and more",
	"Designed for >100MHz Pentium, but runs on 386 as well",
	"Original and p5js compatibility API",
	"RGB, HSB/HSL colors",
	"2D primitives incl. polygons and arcs",
	"Simple text rendering w/ font loading",
	"Affine transformations",
	"Sound & MIDI playback",
	"IPX networking",
	"Supports remote logging via IPX",
	"Mouse and keyboard support",
	"Based on DJGPP, MUjS, Allegro and p5js",
];

exports.prepare = function () {
	fontBig = loadFont("jsboot/fonts/cour34b.fnt");
	fontMedium = loadFont("jsboot/fonts/cour16b.fnt");
	textAlign(LEFT, TOP);

	headSetup();
};

exports.present = function () {
	var yPos = 10;

	background(0);

	colorMode(RGB);
	noStroke();
	strokeWeight(1);
	headDraw();

	fill(222);
	textFont(fontBig);
	text("Features / Infos", 10, yPos);
	yPos += fontBig.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);
	yPos += 5;

	colorMode(RGB);
	textFont(fontMedium);
	fill(255, 200, 200);
	for (var l = 0; l < TEXT2.length; l++) {
		text(TEXT2[l], 10, yPos);
		yPos += fontMedium.height + 4;
	}
};

/* ****************************************************************************************
 PROCESSINGJS.COM HEADER ANIMATION  
 MIT License - F1lT3R/Hyper-Metrix
 Native Processing Compatible 
 */

// Set number of circles
var count = 20;
// Set maximum and minimum circle size
var maxSize = 60;
var minSize = 10;
// Build float array to store circle properties
var e = [];
// Set size of dot in circle center
var ds = 2;

// Set up canvas
function headSetup() {
	// Stroke/line/border thickness
	strokeWeight(1);
	// Initiate array with random values for circles
	for (var j = 0; j < count; j++) {
		e.push([0, 0, 0, 0, 0]);
		e[j][0] = random(width); // X 
		e[j][1] = random(55, height); // Y
		e[j][2] = random(minSize, maxSize); // Radius        
		e[j][3] = random(-.12, .12); // X Speed
		e[j][4] = random(-.12, .12); // Y Speed
	}
}

// Begin main draw loop (called 25 times per second)
function headDraw() {
	// Begin looping through circle array
	for (var j = 0; j < count; j++) {
		// Disable shape stroke/border
		noStroke();
		// Cache diameter and radius of current circle
		var radi = e[j][2];
		var diam = radi / 2;
		if (sq(e[j][0] - mouseX) + sq(e[j][1] - mouseY) < sq(e[j][2] / 2))
			fill(64, 187, 128, 100); // green if mouseover
		else
			fill(64, 128, 187, 100); // regular

		// Draw circle
		ellipse(e[j][0], e[j][1], radi, radi);
		// Move circle
		e[j][0] += e[j][3];
		e[j][1] += e[j][4];


		/* Wrap edges of canvas so circles leave the top
		 and re-enter the bottom, etc... */
		if (e[j][0] < -diam) {
			e[j][0] = width + diam;
		}
		if (e[j][0] > width + diam) {
			e[j][0] = -diam;
		}
		if (e[j][1] < 55 - diam) {
			e[j][1] = height + diam;
		}
		if (e[j][1] > height + diam) {
			e[j][1] = -diam;
		}

		// otherwise set center dot color to black.. 
		fill(0, 0, 0, 255);
		// and set line color to turquoise.
		stroke(64, 128, 128, 255);

		// Loop through all circles
		for (var k = 0; k < count; k++) {
			// If the circles are close...
			if (sq(e[j][0] - e[k][0]) + sq(e[j][1] - e[k][1]) < sq(diam * 2)) {
				// Stroke a line from current circle to adjacent circle
				line(e[j][0], e[j][1], e[k][0], e[k][1]);
			}
		}
		// Turn off stroke/border
		noStroke();
		// Draw dot in center of circle
		rect(e[j][0] - ds, e[j][1] - ds, ds * 2, ds * 2);
	}
}
