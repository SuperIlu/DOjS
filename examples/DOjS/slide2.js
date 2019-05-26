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

var TEXT = [
	"[...]",
	"Processing is an open-source graphical library and integrated",
	"development environment (IDE) / playground built for the",
	"electronic arts, new media art, and visual design communities",
	"with the purpose of teaching non-programmers the fundamentals",
	"of computer programming in a visual context.",
	"[...]"
];

SRC = "From: https://en.wikipedia.org/wiki/Processing_(programming_language)";

TEXT2 = [
	"Hello! p5.js is a JavaScript library that starts with the",
	"original goal of Processing, to make coding accessible for",
	"artists, designers, educators, and beginners, and",
	"reinterprets this for today's web."
];

SRC2 = "From: http://p5js.org/";

var numChars = 0;
var fCount = 0;

var showSecond = false;

exports.prepare = function () {
	fontBig = loadFont("jsboot/fonts/cour34b.fnt");
	fontSmall = loadFont("jsboot/fonts/cour12.fnt");
	font = loadFont("jsboot/fonts/cour16.fnt");
	fontBold = loadFont("jsboot/fonts/cour16b.fnt");

	textAlign(LEFT, TOP);
	colorMode(RGB);
	green = color(0, 200, 0);
	lightGreen = color(20, 255, 20);

	blue = color(40, 40, 222);
	lightBlue = color(60, 60, 255);

	for (var l = 0; l < TEXT.length; l++) {
		for (var x = 0; x < TEXT[l].length; x++) {
			numChars++;
		}
	}
};

exports.present = function () {
	var yPos = 10;
	background(0);
	colorMode(RGB);
	fill(222);
	textFont(fontBig);
	text("Inspired by Processing / p5js", 10, 10);
	yPos += fontBig.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);
	yPos += 5;

	colorMode(RGB);
	fill(222);
	textFont(fontBold);
	text("running on MS-DOS / Win 9x / FreeDOS", 10, yPos);
	yPos += fontBold.height + 15;

	var chPos = 0;
	for (var l = 0; l < TEXT.length; l++) {
		var xPos = 10;
		for (var x = 0; x < TEXT[l].length; x++) {
			chPos++;

			if (chPos == (fCount % numChars)) {
				fill(lightGreen);
				textFont(fontBold);
			} else {
				fill(green);
				textFont(font);
			}

			text(TEXT[l][x], xPos, yPos);
			xPos += font.StringWidth(TEXT[l][x]);
		}
		yPos += font.height + 4;
	}
	fill(lightGreen);
	textFont(fontSmall);
	text(SRC, 10, yPos);
	yPos += fontSmall.height + 30;

	if (showSecond) {
		textFont(fontBold);
		for (var l = 0; l < TEXT2.length; l++) {
			if (l == (fCount % TEXT2.length)) {
				fill(lightBlue);
			} else {
				fill(blue);
			}
			text(TEXT2[l], 10, yPos);
			yPos += fontBold.height + 4;
		}
		fill(lightBlue);
		textFont(fontSmall);
		text(SRC2, 10, yPos);
		yPos += fontSmall.height + 30;
	}
	if (mouseIsPressed) {
		showSecond = true;
	}

	fCount++;
};
