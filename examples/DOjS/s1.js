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

var fontBig = null;
var fontMedium = null;

function setup() {
	logo = loadImage("examples/DOjS.bmp");
	fontBig = loadFont(JSBOOTPATH + "fonts/cour34b.fnt");
	fontMedium = loadFont(JSBOOTPATH + "fonts/cour16b.fnt");

	textAlign(LEFT, TOP);
	imageMode(CENTER);

	fill(222);
	stroke(222);
};

function draw() {
	background(32);

	colorMode(RGB);
	fill(222);
	textFont(fontBig);
	text("DOjS", 10, 10);

	colorMode(HSB);
	stroke(frameCount % 255, 255, 255);
	//line(10, fontBig.height + 15, width - 10, fontBig.height + 15);
	line(10, 36 + 15, width - 10, 36 + 15);

	colorMode(RGB);
	fill(222);
	//textFont(fontMedium);
	//text("Creative coding on MS-DOS", 10, fontBig.height + 25);

	image(logo, width / 2, height / 2);

	TextXY(10, height - 10, str(frameCount), EGA.WHITE, NO_COLOR);
	Gc(true);
};
