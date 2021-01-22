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
Werfen wir mal einen Blick auf die Agenda:
Zuerst ein paar Worte zu mir und wie es zu dem Projekt gekommen ist.
Dann moechte ich kurz etwas zum Creative Coding und dem Vorbild "Processing" erzaehlen.
Danach zeige ich Euch die Entwicklungsumgebung mit dem integrierten Editor und dann stelle ich die restlichen Features des Systems  vor.
Ganz am Ende machen wir noch eine kurze Fragerunde.
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
	text("Agenda", 10, 10);
	yPos += fontHeader.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);
	yPos += 5;

	colorMode(RGB);
	fill(222);
	textAlign(LEFT, CENTER);
	var l = 3;
	text("* Who am I & motivation", 20, l * fontHeader.height); l++;
	text("* Creative coding / ", 20, l * fontHeader.height); l++;
	text("  Processing", 20, l * fontHeader.height); l++;
	text("* IDE / Editor", 20, l * fontHeader.height); l++;
	text("* Features", 20, l * fontHeader.height); l++;
	text("* Questions", 20, l * fontHeader.height); l++;
};

exports.keyHook = function (key) { };
exports.exitHook = function () { };
