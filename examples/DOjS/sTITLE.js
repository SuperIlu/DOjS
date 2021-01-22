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
Title:
	DOjS - Javascript auf MS-DOS
Teaser:
	DOjS ist eine Javascript (ES2015) basierte Laufzeitumgebung mit integrierter Entwicklungsumgebung (IDE) für MS-DOS.
	Sie bietet Schnittstellen für Hardware die üblicherweise in DOS-PCs zu finden ist wie z.B. SVGA-Grafik, Maus, Tastatur, Soundkarten oder gar 3D-Beschleunigung mit den damals üblichen Voodoo-Grafikkarten.
	Obwohl der primäre Fokus bei der Entwicklung auf elektronische Kunstwerke (Creative Coding) gelegt wurde kann sie ebenso für die Entwicklung von Spielen verwendet werden.

---
Hallo, ich bin Ilu und moechte Euch in den naechsten Minuten etwas darueber erzaehlen was Javascript auf MS-DOS zu suchen hat...
Mein kleines Spassprojekt heisst DOjS und die Folien die ich Euch jetzt zeigen werde sind auch damit erstellt.
Das ganze laeuft jetzt grade auf einem AMD-K6 mit FreeDOS der hier neben mir unter dem Schreibtisch steht.
*/

var logo;

exports.prepare = function () {
	logo = loadImage("examples/DOjS.bmp");
	imageMode(CENTER);
};

exports.present = function () {
	var yPos = 10;

	background(0);

	textAlign(LEFT, TOP);
	colorMode(RGB);
	fill(222);
	textFont(fontHeader);
	text("DOjS", 10, 10);
	yPos += fontHeader.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);
	yPos += 5;

	colorMode(RGB);
	fill(222);
	textFont(fontSubheader);
	text("Creative coding with Javascript on MS-DOS", 10, yPos);

	textAlign(CENTER);
	textFont(fontSmall);
	fill(222, 100, 100);
	text("Andre \"Ilu\" Seidelt / @dec_hl", width / 2, height - 12);

	image(logo, width / 2, height / 2);
};

exports.keyHook = function (key) { };
exports.exitHook = function () {
	logo = null;
};
