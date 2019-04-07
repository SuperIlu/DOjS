/**
 * This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * http://creativecommons.org/licenses/by-nc-sa/4.0/
 * 
 * The original source can be found here:
 * https://p5js.org/examples/interaction-wavemaker.html
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

t = 0; // time variable

function setup() {
	createCanvas(600, 600);
	noStroke();
	fill(40, 200, 40);
}

function draw() {
	background(10); // translucent background (creates trails)

	// make a x and y grid of ellipses
	for (var x = 0; x <= width; x = x + 30) {
		for (var y = 0; y <= height; y = y + 30) {
			// starting point of each circle depends on mouse position
			var xAngle = map(mouseX, 0, width, -4 * PI, 4 * PI, true);
			var yAngle = map(mouseY, 0, height, -4 * PI, 4 * PI, true);
			// and also varies based on the particle's location
			var angle = xAngle * (x / width) + yAngle * (y / height);

			// each particle moves in a circle
			var myX = x + 20 * cos(2 * PI * t + angle);
			var myY = y + 20 * sin(2 * PI * t + angle);

			ellipse(myX, myY, 10); // draw particle
		}
	}

	t = t + 0.01; // update time
}