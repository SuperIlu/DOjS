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

function SimpleSlide(head, body, bigFont, image, width) {
	this.headline = head;
	this.body = body;
	this.bigFont = bigFont;
	this.image = image;
	this.imageWidth = width || 100;
}

SimpleSlide.prototype.prepare = function () {
};

SimpleSlide.prototype.present = function () {
	var yPos = 10;

	background(0);
	textAlign(LEFT, TOP);

	colorMode(RGB);
	noStroke();
	strokeWeight(1);

	fill(222);
	textFont(fontHeader);
	text(this.headline, 10, yPos);
	yPos += fontHeader.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);
	yPos += 5;

	colorMode(RGB);
	if (this.bigFont) {
		fill(222);
		textAlign(LEFT, CENTER);
		var l = 3;
		for (var l = 0; l < this.body.length; l++) {
			text(this.body[l], 20, (l + 3) * fontHeader.height);
		}
	} else {
		textFont(fontSubheader);
		fill(200, 255, 200);
		for (var l = 0; l < this.body.length; l++) {
			text(this.body[l], 10, yPos);
			yPos += fontSubheader.height + 4;
		}
	}

	if (this.image) {
		var scaleFactor = this.imageWidth / this.image.width;
		var scaledW = this.image.width * scaleFactor;
		var scaledH = this.image.height * scaleFactor;

		this.image.DrawAdvanced(
			0, 0, this.image.width, this.image.height,
			Width - scaledW, Height / 2 - scaledH / 2, scaledW, scaledH);
	}
};

SimpleSlide.prototype.keyHook = function (key) { };
SimpleSlide.prototype.exitHook = function () { };

exports.SimpleSlide = SimpleSlide;
