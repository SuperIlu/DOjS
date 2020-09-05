Include('p5');

function setup() {
	createCanvas(windowWidth, windowHeight);
	background(0);
	stroke(255)
	fill(128, 32, 32);

	beginShape();
	vertex(20, 20);
	vertex(45, 20);
	vertex(45, 80);
	endShape(CLOSE);

	beginShape();
	vertex(50, 20);
	vertex(75, 20);
	vertex(75, 80);
	endShape();
}

function draw() { }
