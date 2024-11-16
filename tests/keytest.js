if (navigator.appName === "DOjS") {
	Include('p5');
}

var xPos = 160;
var yPos = 120;
var pixelSize = 10;
var speed = 2; // Adjust the speed according to your preference

var pressedKeys = {};

function setup() {
	createCanvas(320, 240);
}

function draw() {
	background(127); // Clear the canvas

	fill('yellow');
	noStroke();
	rect(xPos, yPos, pixelSize, pixelSize);

	// Move the pixel when keys are held down
	if (keyIsDown(KEY.Code.KEY_UP)) {
		yPos -= speed;
	}
	if (keyIsDown(KEY.Code.KEY_DOWN)) {
		yPos += speed;
	}
	if (keyIsDown(KEY.Code.KEY_LEFT)) {
		xPos -= speed;
	}
	if (keyIsDown(KEY.Code.KEY_RIGHT)) {
		xPos += speed;
	}
}
