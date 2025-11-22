/*
MIT License

Copyright (c) 2019-2025 Andre Seidelt <superilu@yahoo.com>

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

var logo;

exports.prepare = function () {
	logo = loadImage(PDIR + "DOjS.png");
	imageMode(CENTER);
};

exports.present = function () {
	var yPos = 10;

	background(0);

	textAlign(LEFT, TOP);
	colorMode(RGB);
	fill(222);
	textFont(fontHeader);
	text("DOjS - Javascript for MS-DOS", 10, 10);
	yPos += fontHeader.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);
	yPos += 5;

	colorMode(RGB);
	fill(222);
	textFont(fontSubheader);
	text("How a C64 game turned into a Mastodon client for MS-DOS", 10, yPos);

	textAlign(CENTER);
	textFont(fontSmall);
	fill(222, 100, 100);
	text("VCFe 2025 / Andre \"Ilu\" Seidelt / @dec_hl", width / 2, height - 12);

	image(logo, width / 2, height / 2);
};

exports.keyHook = function (key) { };
exports.exitHook = function () {
	logo = null;
};
