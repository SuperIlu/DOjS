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

var dotPos = 10;
var inc = true;
var images;
var fCount = 0;

exports.prepare = function () {
	images = [];
	images.push(loadImage("examples/DOjS/help1.bmp"));
	images.push(loadImage("examples/DOjS/help2.bmp"));

	textAlign(LEFT, TOP);
	imageMode(CORNER);

	fCount = 0;
};

exports.present = function () {
	var yPos = 10;

	background(0);

	colorMode(RGB);
	fill(222);
	textFont(fontHeader);
	text("Context Help", 10, yPos);
	yPos += fontHeader.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);

	colorMode(RGB);
	fill(0, 222, 0, 128);
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
	text("Integrated HELP / API doc", 10, yPos);

	image(images[floor(fCount / 100) % images.length], 0, 80);

	fCount++;
};

exports.keyHook = function (key) { };
exports.exitHook = function () {
	images = null;
};
