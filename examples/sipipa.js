/**
 * This work is licensed under Attribution-ShareAlike 3.0 Unported.
 * https://creativecommons.org/licenses/by-sa/3.0/
 * 
 * Original by Carlos Oliveira
 * https://twitter.com/vamoss/status/1121784764850040834
 * 
 * The original source can be found here:
 * https://www.openprocessing.org/sketch/703940
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

var img;

function setup() {
	background(255);
	noStroke();
	img = loadImage('examples/3dfx.tga');
	img.loadPixels();
}

function draw() {
	for (var i = 0; i < 6; i++)
		drawPill();
}

function mousePressed() {
	background(255);
}

function drawPill() {
	var x = random(width);
	var y = random(height);
	var color = img.get(x, y);
	var lum = .2126 * red(color) + .7152 * green(color) + .0722 * blue(color);
	var angle = lum / 255 * PI;
	fill(color);
	stroke(color);
	pill(x, y, random(2, 10), random(2, 20), angle);
}

function pill(x, y, width, height, rotation) {
	var size = width / 6;
	var precision = width / 600;
	push();
	translate(x, y);
	rotate(rotation);
	beginShape();
	for (var i = 0; i < TWO_PI; i += precision) {
		var s = size;
		if (i > PI) s = size * 0.7;
		vertex(cos(i + PI) * s, sin(i + PI) * s + (i < PI ? -height / 2 : height / 2));
	}
	endShape(CLOSE);
	pop();
}
