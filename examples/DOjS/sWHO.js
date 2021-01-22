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
Zu meiner Person, ich bin ein ganz normaler Nerd in den 40ern.
Ich habe mein Leben lang Computerspiele gespielt und seit einigen Jahren sammel ich alte Computer und Konsolen.
Vor ca. 13 Jahren bin ich ueber "Processing" gestolpert und wenn ich eine meiner seltenen kreativen Phasen habe,
dann mache ich damit oder aehnlichen Projekten Computerkunst.
Im wirklichen Leben entwickle ich hardwarenahe Software auf Mikrocontrollern und verdiene mir damit das geld um weitere, alte Computer zu kaufen.

Wie ist es jetzt zu DOjS gekommen?
Seit einer halben Ewigkeit will ich mal selber ein Spiel entwickeln, zuerst hatte ich den C64 im Auge,
hab aber recht schnell meine Meinung geaendert und wollte mal was fuer DOS PCs machen.
Als ich dann meine Vorrecherchen bezueglich Compiler und so weiter abgeschlossen hatte ist mir aufgefallen das ich ja noch so unwichtige Dinge wie eine Spieleidee brauche.
Da mir einfach nichts einfallen wollte habe ich entschieden das ich mir erstmal ein Framework fuer die Entwicklung schaffe.
Ich habe mich also nach Beispielcode und Bibliotheken fuer DOS Entwicklung umgesehen und bin durch ein anderes Projekt auf einen Javascript Interpreter gestossen.
Das brachte mich auf die Idee mal zu versuchen ob der auch unter DOS lauffaehig waere und dann ist dieser Versuch ein bischen eskaliert...

Und deswegen sitze ich heute hier und erzaehle euch was...
*/
var wTEXT2 = [
	"Me:",
	"- Standard nerd",
	"- Gamer",
	"- Retro computer/console collector",
	"- Creative coder",
	"- RL: Embedded software developer",
	"- Pronouns: he/his",
	"",
	"Motivation:",
	"- Develop a game on DOS?",
	"- Wait, I need an idea!",
	"- Ok, framework first...",
	"- Oh look: a Javascript intepreter",
	"- Escalation level > 9000",
];

exports.prepare = function () {
	textAlign(LEFT, TOP);
};

exports.present = function () {
	var yPos = 10;

	background(0);

	colorMode(RGB);
	noStroke();
	strokeWeight(1);

	fill(222);
	textFont(fontHeader);
	text("Who am I & motivation", 10, yPos);
	yPos += fontHeader.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);
	yPos += 5;

	colorMode(RGB);
	textFont(fontSubheader);
	fill(200, 255, 200);
	for (var l = 0; l < wTEXT2.length; l++) {
		text(wTEXT2[l], 10, yPos);
		yPos += fontSubheader.height + 4;
	}
};

exports.keyHook = function (key) { };
exports.exitHook = function () { };
