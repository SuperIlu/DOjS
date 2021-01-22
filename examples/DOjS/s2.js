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

TEXT = [
	"[...]",
	"Processing is an open-source graphical library and integrated",
	"development environment (IDE) / playground built for the",
	"electronic arts, new media art, and visual design communities",
	"with the purpose of teaching non-programmers the fundamentals",
	"of computer programming in a visual context.",
	"[...]"
];

numChars = -1;

function setup() {
	font = loadFont(JSBOOTPATH + "fonts/cour16b.fnt");
	fontBold = loadFont(JSBOOTPATH + "fonts/cour16b.fnt");

	textAlign(LEFT, TOP);

	colorMode(RGB);
	green = color(0, 200, 0);
	lightGreen = color(20, 255, 20);
};

function draw() {
	background(0);
	var chPos = 0;
	var yPos = 10;
	var cnt = 0;
	for (var l = 0; l < TEXT.length; l++) {
		var xPos = 10;
		for (var x = 0; x < TEXT[l].length; x++) {
			chPos++;

			if (numChar != -1 && chPos == (frameCount % numChars)) {
				fill(lightGreen);
				textFont(fontBold);
			} else {
				fill(green);
				textFont(font);
			}

			text(TEXT[l][x], xPos, yPos);
			xPos += font.StringWidth(TEXT[l][x]);
			cnt++;
		}
		yPos += font.height + 4;
	}
	numChars = cnt;

	TextXY(10, height - 10, str(frameCount), EGA.WHITE, NO_COLOR);
	Gc(true);
};
