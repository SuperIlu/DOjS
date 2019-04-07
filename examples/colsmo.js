/**
 * This work is licensed under Attribution-ShareAlike 3.0 Unported.
 * https://creativecommons.org/licenses/by-sa/3.0/
 * 
 * Original by eclisse92
 * 
 * The original source can be found here:
 * https://www.openprocessing.org/sketch/674040
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

var np = 300;
var startcol;

function setup() {
	createCanvas(1800, 800);
	background(0);
	noFill();
	noiseSeed(random(100));
	startcol = random(255);
	time = 0;
	randomY = random(100);
}

function draw() {
	// background(51);
	beginShape();
	time = frameCount % 1200;
	var sx;
	var sy;
	for (var i = 0; i < np; i++) {
		var angle = map(i, 0, np, 0, TWO_PI);
		var cx = time * 2 - 200;
		var cy = randomY + height / 2 + 50 * sin(time / 50);
		var xx = 100 * cos(angle + cx / 10);
		var yy = 100 * sin(angle + cx / 10);
		var v = createVector(xx, yy);
		xx = (xx + cx) / 150; yy = (yy + cy) / 150;
		v.mult(1 + 1.5 * noise(xx, yy));
		vertex(cx + v.x, cy + v.y);
		if (i == 0) {
			sx = cx + v.x;
			sy = cy + v.y;
		}
	}
	colorMode(HSB);
	var hue = cx / 10 - startcol;
	if (hue < 0) hue += 255;
	stroke(hue, 100, 120);
	strokeWeight(0.3);
	vertex(sx, sy);
	endShape();
	if (frameCount % 1200 == 0) {
		//noLoop();
		noiseSeed(random(100));
		startcol = random(255);
		randomY = random(100);
		clear();
	}
}