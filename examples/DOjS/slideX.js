/*
MIT License

Copyright (c) 2019 Andre Seidelt <superilu@yahoo.com>

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

exports.prepare = function () {
	logo = loadImage("examples/DOjS.bmp");
	fontBig = loadFont("jsboot/fonts/cour34b.fnt");
	fontMedium = loadFont("jsboot/fonts/cour16b.fnt");

	sqSetup();
};

exports.present = function () {
	var yPos = 10;

	background(0);
	textAlign(LEFT, TOP);

	colorMode(RGB);
	fill(222);
	textFont(fontBig);
	text("Contact / Website", 10, 10);
	yPos += fontBig.height + 5;

	colorMode(HSB);
	strokeWeight(2);
	stroke(frameCount % 255, 255, 255);
	line(10, yPos, width - 10, yPos);
	yPos += 5;

	sqDraw();

	colorMode(RGB);
	fill(222);
	textAlign(CENTER, CENTER);
	text("Twitter:", width / 2, height / 2 - 2 * fontBig.height);
	text("@dec_hl", width / 2, height / 2 - fontBig.height);

	text("Github:", width / 2, height / 2 + fontBig.height)
	textFont(fontMedium);
	text("https://github.com/SuperIlu/DOjS", width / 2, height / 2 + 2 * fontBig.height)
};

var rectSize = 20;
var MAX_RECTS = 10;
var rects = [];

function sqSetup() {

	for (var i = 0; i < MAX_RECTS; i++) {
		rects.push([random(width), random(55, height)]);
	}
}

function sqDraw() {
	strokeWeight(1);
	angleMode(DEGREES);
	colorMode(HSB);
	noFill();
	var new_rects = [];
	for (var i = 0; i < rects.length; i++) {
		drawRect(rects[i][0], rects[i][1]);
		rects[i][1] += 1;
		if (rects[i][1] < height) {
			new_rects.push(rects[i]);
		}
	}

	if (new_rects.length < MAX_RECTS && int(random(2)) == 1) {
		new_rects.push([random(width), 55]);
	}

	rects = new_rects;
}

function drawRect(x, y) {
	var c = {
		x: x,
		y: y
	};
	var ul = {
		x: x - rectSize,
		y: y - rectSize
	};
	var ur = {
		x: x + rectSize,
		y: y - rectSize
	};
	var ll = {
		x: x - rectSize,
		y: y + rectSize
	};
	var lr = {
		x: x + rectSize,
		y: y + rectSize
	};

	var a = map(y, 0, height, 0, 90);

	ul = rot(ul, a, c);
	ur = rot(ur, a, c);
	ll = rot(ll, a, c);
	lr = rot(lr, a, c);

	stroke(map(y, 0, height, 0, 255), 255, 255);
	beginShape();
	vertex(ul.x, ul.y);
	vertex(ur.x, ur.y);
	vertex(lr.x, lr.y);
	vertex(ll.x, ll.y);
	endShape(CLOSE);
}

// https://academo.org/demos/rotation-about-point/
function rot(p, a, c) {
	var cA = cos(a);
	var sA = sin(a);

	var x = p.x - c.x;
	var y = p.y - c.y;

	return {
		x: (x * cA - y * sA) + c.x,
		y: (y * cA + x * sA) + c.y
	};
}
