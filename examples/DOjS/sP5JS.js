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
Ok, was ist dieses Processing und Creative Coding jetzt eigentlich?
Mit Creative Coding bezeichnet man alles wo Programmieren als Gestaltungsmittel genutzt wird und nicht um eine bestimmt Funktion zu erfuellen.
Das kann Computerkunst sein, interaktive Installationen, auditive Programme oder generative Kunst z.B. auf Basis von mathematischen Funktionen.

Wie bereits erwaehnt benutze ich seit Jahren Processing um Daten zu visualisieren oder einfach nur mehr oder weniger schoene Bilder und Effekte zu erzeugen.
Auf dem Bildschirm seht ihr die Projektvorstellung von der Webseite.

Processing selber basiert auf Java, es gibt allerdings ein seit einiger Zeit ein Schwesterprojekt namens p5js welches in jedem modernen Webbrowser laeuft
und auf Javascript basiert. Auch hier habe ich mal den Einleitungstext von der Webseite kopiert.

Beide Projekte zeichnen sich dadurch aus das sie Anfaengefreundlich sind und deswegen auch von Menschen ohne tiefergehende Programmierkentnisse genutzt werden koennen.

*/

var pTEXT = [
	"[...]",
	"Processing is an open-source graphical library and integrated",
	"development environment (IDE) / playground built for the",
	"electronic arts, new media art, and visual design communities",
	"with the purpose of teaching non-programmers the fundamentals",
	"of computer programming in a visual context.",
	"[...]"
];

var pSRC = "From: https://en.wikipedia.org/wiki/Processing_(programming_language)";

var pTEXT2 = [
	"Hello! p5.js is a JavaScript library that starts with the",
	"original goal of Processing, to make coding accessible for",
	"artists, designers, educators, and beginners, and",
	"reinterprets this for today's web."
];

var pSRC2 = "From: http://p5js.org/";

var numChars = 0;
var showSecond = false;
var fCount = 0;

exports.prepare = function () {
	textAlign(LEFT, TOP);
	colorMode(RGB);
	green = color(0, 200, 0);
	lightGreen = color(20, 255, 20);

	blue = color(222, 222, 40);
	lightBlue = color(255, 255, 60);

	for (var l = 0; l < pTEXT.length; l++) {
		for (var x = 0; x < pTEXT[l].length; x++) {
			numChars++;
		}
	}
	fCount = 0;
};

exports.present = function () {
	var yPos = 10;
	background(0);
	colorMode(RGB);
	fill(222);
	textFont(fontHeader);
	text("Creative coding", 10, yPos);
	yPos += fontHeader.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);
	yPos += 5;

	colorMode(RGB);
	fill(222);
	textFont(fontSubheader);
	text("Inspired by Processing / p5js", 10, yPos);
	yPos += fontSubheader.height + 15;

	var chPos = 0;
	for (var l = 0; l < pTEXT.length; l++) {
		var xPos = 10;
		for (var x = 0; x < pTEXT[l].length; x++) {
			chPos++;

			if (chPos == (fCount % numChars)) {
				fill(lightGreen);
				textFont(fontSubheader);
			} else {
				fill(green);
				textFont(fontText);
			}

			text(pTEXT[l][x], xPos, yPos);
			xPos += fontText.StringWidth(pTEXT[l][x]);
		}
		yPos += fontText.height + 4;
	}
	fill(lightGreen);
	textFont(fontSmall);
	text(pSRC, 10, yPos);
	yPos += fontSmall.height + 30;

	if (showSecond) {
		textFont(fontSubheader);
		for (var l = 0; l < pTEXT2.length; l++) {
			if (l == (floor(fCount / 10) % pTEXT2.length)) {
				fill(lightBlue);
			} else {
				fill(blue);
			}
			text(pTEXT2[l], 10, yPos);
			yPos += fontSubheader.height + 4;
		}
		fill(lightBlue);
		textFont(fontSmall);
		text(pSRC2, 10, yPos);
		yPos += fontSmall.height + 30;
	}
	fCount++;
};

exports.keyHook = function (key) {
	if (key == KEY.Code.KEY_DOWN) {
		showSecond = true;
	} else if (key == KEY.Code.KEY_UP) {
		showSecond = false;
	}
};

exports.exitHook = function () { };
