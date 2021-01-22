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
DOjS unterstuetzt das Laden von Schriftarten im GRX Format.
Es werden ueber 300 freie Schriftwarten in unterschiedlichen Groessen mitgeliefert.
Derzeit ist leider nur einfache Textausgabe von links nach rechts moeglich, Rotation oder aehnliches wird nicht unterstuetzt.
*/

var wTEXT2 = [
	"* GRX font loading",
	"* ~390 fonts are included",
	"  - fonts from libgrx",
	"  - fonts from Linux Font Project",
	"* for now only horizontal, left to right rendering"
];

var fontList;

var FONT_DIR = JSBOOTPATH + "fonts/";

exports.prepare = function () {
	fontList = [];
	[
		"cour16.fnt",
		"helv17.fnt",
		"lucb17.fnt",
		"lucs17.fnt",
		"luct19.fnt",
		"ncen18.fnt",
		"symb16.fnt",
		"tms18.fnt"
	].forEach(function (f) {
		//Println("loading " + FONT_DIR + f);
		fontList.push(new Font(FONT_DIR + f));
	});
};

exports.present = function () {
	var yPos = 10;

	background(0);
	textAlign(LEFT, TOP);

	colorMode(RGB);
	fill(222);
	textFont(fontHeader);
	text("Typography", 10, 10);
	yPos += fontHeader.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);
	yPos += 5;

	colorMode(RGB);
	textFont(fontSubheader);
	fill(200, 255, 255);
	for (var l = 0; l < wTEXT2.length; l++) {
		text(wTEXT2[l], 10, yPos);
		yPos += fontSubheader.height + 4;
	}

	yPos += 15;

	fill(255, 255, 200);
	for (var i = 0; i < fontList.length; i++) {
		var fnt = fontList[i];
		textFont(fnt);
		text("The quick brown fox jumps over the lazy dog", 10, yPos);
		yPos += fnt.height + 4;
	}
};

exports.keyHook = function (key) { };
exports.exitHook = function () { };
