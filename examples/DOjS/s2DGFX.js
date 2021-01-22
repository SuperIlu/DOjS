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
Jetzt schauen wir uns mal die Features an.

Als erstes haben wir hier die einfachen 2D Grafikprimitive wie Punkte, Linien, Vierecke und so weiter.
Diese koennen als Umriss oder gefuellt gezeichnet werden.

Zusaetzlich gibt es Funktionen zum Zeichnen von gefuellten Polygonen.
*/

exports.prepare = function () {
};

exports.present = function () {
	var yPos = 10;

	background(0);
	textAlign(LEFT, TOP);

	colorMode(RGB);
	fill(222);
	textFont(fontHeader);
	text("2D Graphics", 10, 10);
	yPos += fontHeader.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);
	yPos += 5;

	// test lines
	Line(10, yPos + 10, 100, yPos + 10, EGA.RED);
	CustomLine(110, yPos + 10, 200, yPos + 10, 5, EGA.RED);

	// test boxes
	Box(10, yPos + 20, 100, yPos + 100, EGA.GREEN);
	FilledBox(110, yPos + 20, 200, yPos + 100, EGA.GREEN);

	// test circles
	Circle(300, yPos + 60, 40, EGA.BLUE);
	FilledCircle(400, yPos + 60, 40, EGA.BLUE);
	CustomCircle(500, yPos + 60, 40, 5, EGA.BLUE);

	// test ellipses
	Ellipse(300, yPos + 160, 40, 20, EGA.CYAN);
	FilledEllipse(400, yPos + 160, 40, 20, EGA.CYAN);
	CustomEllipse(500, yPos + 160, 40, 20, 5, EGA.CYAN);

	// try fill
	Box(10, yPos + 110, 100, yPos + 200, EGA.MAGENTA);
	FloodFill(15, yPos + 115, EGA.WHITE);

	var arc = CircleArc(110, yPos + 140, 40, 0, 32, EGA.BROWN);
	Line(arc.centerX, arc.centerY, arc.startX, arc.startY, EGA.BROWN);
	Line(arc.centerX, arc.centerY, arc.endX, arc.endY, EGA.BROWN);

	var arc = CustomCircleArc(130, yPos + 200, 40, 32, 64, 5, EGA.BROWN);
	Line(arc.centerX, arc.centerY, arc.startX, arc.startY, EGA.BROWN);
	Line(arc.centerX, arc.centerY, arc.endX, arc.endY, EGA.BROWN);


	FilledPolygon(
		[
			[20 + mX(), 20 + mY()],
			[40 + mX(), 20 + mY()],
			[40 + mX(), 40 + mY()],
			[60 + mX(), 40 + mY()],
			[60 + mX(), 60 + mY()],
			[20 + mX(), 60 + mY()]
		], Color(255, 0, 0, 128));
	FilledPolygon(
		[
			[20 + mX(), 20 + mY()],
			[40 + mX(), 20 + mY()],
			[40 + mX(), 40 + mY()],
			[60 + mX(), 40 + mY()],
			[60 + mX(), 60 + mY()],
			[20 + mX(), 60 + mY()]
		], Color(0, 255, 0, 128));
};

function Loop() {
	ClearScreen(EGA.BLACK);

}

function mX() {
	return Math.floor(Math.random() * 21) - 10 + SizeX() / 2;
}

function mY() {
	return Math.floor(Math.random() * 21) - 10 + 270;
}

exports.keyHook = function (key) { };
exports.exitHook = function () { };
