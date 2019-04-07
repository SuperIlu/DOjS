/**
 * This work is licensed under Attribution-ShareAlike 3.0 Unported.
 * https://creativecommons.org/licenses/by-sa/3.0/
 * 
 * The original source by Arjo Nagelhout can be found here:
 * https://www.openprocessing.org/sketch/681621
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

function setup() {
	background(222);
	stroke(0);
	fill(255);
}

function draw() {
	t = millis() / 5e3
	for (i = 0; i < 10; i++) { v(width / 2, height / 2, i - t, 5) }
}

function v(x, y, a, l) {
	rect(x += l * sin(a) * 15, y -= l * cos(a) * 15, 4, 4)
	if (l > 1) {
		v(x, y, a + 1 + cos(t), l *= .7)
		v(x, y, a - 1 - sin(t), l)
	}
}
