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
DOjS unterstuetzt auch den Zugriff auf das Dateisystem.
Neben Lesen/Schreiben von Dateien werden auch dateisystemoperationen wie das Umbenennen von Dateien,
Erzeugen und Loeschen von Verzeichnissen usw unterstuetzt.
Des Weiteren kann ein Verzeichnislisting ausgelesen werden und Metainformationen wir Dateigroesse u.ae. ermittelt werden.

Der Zugriff auf ZIP Dateien ist moeglich und alle Ressourcen wie Fonts, Bilder oder gar Skriptdateien koennen direkt aus
ZIP Dateien gelesen werden.

Es ist sogar moeglich eigene Scripte und die mitgelieferte Lauzeitbibliothek in einem einzigen ZIP zusammenzufassen so das
eine mit DOjS erstellte Software mit drei Dateien auskommt.
*/

var wTEXT2 = [
	"* file system functions",
	"  - read/write files as text or byte wise",
	"  - create/rename/delete files/directories",
	"  - list directories & get file info",
	"* ZIP archive support",
	"  - create/extract ZIP files",
	"  - load font, bitmaps, etc from ZIP files",
	"  - load scripts and all support/resource files from",
	"    single ZIP file",
	"* run external programs"
];

exports.prepare = function () {
};

exports.present = function () {
	var yPos = 10;

	background(0);
	textAlign(LEFT, TOP);

	colorMode(RGB);
	fill(222);
	textFont(fontHeader);
	text("Files / ZIP archives", 10, 10);
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
};

exports.keyHook = function (key) { };
exports.exitHook = function () { };
