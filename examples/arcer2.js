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
	centerX = width / 2;
	centerY = height / 2;
	colorMode(HSB);
	frameRate(20);
	start = 0;
	r = min(width, height) / 2;
	lastAI = null;
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function draw() {
	var size = random(0, 255);
	var c = random(0, 255);
	var cCol = color(c, 255, 255);
	var lCol = color(c, 128, 255);

	var ai = CircleArc(centerX, centerY, r, start, start + size, cCol.toAllegro());
	if (lastAI) {
		Line(lastAI.endX, lastAI.endY, ai.startX, ai.startY, lCol.toAllegro());
	}

	start += size;
	lastAI = ai;
	r -= 5;
	if (r <= 0) {
		background(0);
		r = min(width, height) / 2;
		lastAI = null;
	}
}
