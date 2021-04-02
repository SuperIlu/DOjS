Include('p5');

function setup() {
	createCanvas(640, 480);
}

var idx = 0;

function draw() {
	background(32);
	noFill();

	stroke(255, 0, 0);
	arc(50, 55, 50, 50, 0, HALF_PI);
	stroke(0, 255, 0);
	arc(50, 55, 60, 60, HALF_PI, PI);
	stroke(60, 60, 255);
	arc(50, 55, 70, 70, PI, PI + QUARTER_PI);
	stroke(255);
	arc(50, 55, 80, 80, PI + QUARTER_PI, TWO_PI);

	stroke(255, 0, 0);
	arc(150, 55, 50, 50, idx + 0, idx + HALF_PI);
	stroke(0, 255, 0);
	arc(150, 55, 60, 60, idx + HALF_PI, idx + PI);
	stroke(60, 60, 255);
	arc(150, 55, 70, 70, idx + PI, idx + PI + QUARTER_PI);
	stroke(255);
	arc(150, 55, 80, 80, idx + PI + QUARTER_PI, idx + TWO_PI);
	idx += 0.01;
}
