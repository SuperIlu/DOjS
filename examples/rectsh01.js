/**
 * This work is licensed under Attribution-ShareAlike 3.0 Unported.
 * https://creativecommons.org/licenses/by-sa/3.0/
 * 
 * Original by @reona396, http://reona396.information.jp/
 * 
 * The original source can be found here:
 * https://www.openprocessing.org/sketch/177852
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

function setup() {
	size(400, 400);
	rectMode(CENTER);
}

function draw() {
	background(255);

	drawingRects(width / 2, height / 2, 200, 20, 6);
}

function drawingRects(x, y, w, h, num) {
	noStroke();
	fill(0);

	rect(x, y, w, h);
	rect(x - w / 2, y, h, w);
	rect(x + w / 2, y, h, w);

	if (num > 0) {
		drawingRects(x - w / 2, y - w / 2, w / 2, h / 2, num - 1);
		drawingRects(x + w / 2, y - w / 2, w / 2, h / 2, num - 1);
		drawingRects(x - w / 2, y + w / 2, w / 2, h / 2, num - 1);
		drawingRects(x + w / 2, y + w / 2, w / 2, h / 2, num - 1);
	}
}

