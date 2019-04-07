/**
 * This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * http://creativecommons.org/licenses/by-nc-sa/4.0/
 * 
 * The original source can be found here:
 * https://p5js.org/examples/control-embedded-iteration.html
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

function setup() {
	gridSize = 55;
	noStroke();
	MouseSetColors(EGA.RED, EGA.GREEN);
	//MouseShowCursor(false);
	MouseShowCursor(true);
}

function draw() {
	background(0);
	for (var x = gridSize; x <= width - gridSize; x += gridSize) {
		for (var y = gridSize; y <= height - gridSize; y += gridSize) {
			noStroke();
			fill(255);
			rect(x - 1, y - 1, 3, 3);
			stroke(255, 50);
			line(x, y, mouseX, mouseY);
		}
	}
}
