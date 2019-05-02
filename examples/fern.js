/**
 * This work is licensed under Attribution-ShareAlike 3.0 Unported.
 * https://creativecommons.org/licenses/by-sa/3.0/
 * 
 * Original by Che-Yu Wu
 * https://twitter.com/Majer666666/status/1118199946816249857
 * 
 * The original source can be found here:
 * https://www.openprocessing.org/sketch/700092
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

function setup() {
	createCanvas(windowWidth, windowHeight);
	colorMode(HSB);
	background(0);
}
function draw() {
	background(0);
	translate(width / 2, height / 2);
	stroke(255);
	noStroke();
	for (var o = 0; o < 10; o++) {
		push();
		rotate(PI / 10 * 2 * o);
		for (var i = 1; i < 50; i++) {
			var r = 100 / pow(i, 0.8);
			var deg = o + sin(i / 50 + cos(frameCount / 20) / 30 + frameCount / 50) / 5 + noise(frameCount / 50, i * 10, o * 10) + cos(frameCount / 200);
			rotate(deg);
			stroke(i, 50, 100 - o - i / 10);
			line(0, 0, r, 0);
			noStroke();
			fill(i * 3, 70, 100 - o - i / 10);
			ellipse(0, 0, log(i) * 1, 1 * log(i));
			translate(r, 0);
		}
		pop();
	}
}
