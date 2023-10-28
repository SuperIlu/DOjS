if (navigator.appName === "DOjS") {
	Include('p5');
}

function setup() {
	cX = width / 2;
	cY = height / 2;

	rot = 0;

	direction = 1;
	frameRate(60);
}

function draw() {
	background(0);

	stroke(255, 32, 42);
	line(2, 2, 12, 12);

	stroke(255, 0, 0);
	line(0, 0, width - 1, height);

	stroke(0, 255, 0);
	line(0, height, width - 1, 0);

	fill(0, 0, 255);
	stroke(0, 255, 255);
	ellipse(cX, cY, 8, 12);

	cX += direction;
	if (cX > width / 2 + 12) {
		direction = -1;
	}
	if (cX < width / 2 - 12) {
		direction = 1;
	}

	push();
	fill(0, 255, 0);
	stroke(255, 255, 0);
	translate(13, 13);
	rotate(rot);
	rect(0, 0, 10, 10);
	pop();

	rot += 0.01;
}

