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
Include('p5');

/*
** This function is called once when the script is started.
*/
function setup() {
	colorMode(HSB);
	frameRate(30);

	centerX = width / 2;
	centerY = height / 2;
	arcs = [];
	steps = max(width, height) / 40;

	var maxSize = sqrt(width * width + height * height) / 2;

	for (var i = 0; i < maxSize; i += steps) {
		arcs.push({
			start: random(0, 3600),
			length: random(0, 3600),
			col: color(random(0, 255), 255, 255),
			r: i
		});
	}
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function draw() {
	background(0);
	for (var i = 0; i < arcs.length; i++) {
		var ca = arcs[i];
		var ai = CircleArc(centerX, centerY, ca.r, ca.start, ca.start + ca.length, ca.col);
		if (i % 2) {
			ca.start++;
			if (ca.start > 3600) {
				ca.start = 0;
			}
		} else {
			ca.start--;
			if (ca.start < 0) {
				ca.start = 3600;
			}
		}
	}
}
