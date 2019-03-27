/**
 * This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * http://creativecommons.org/licenses/by-nc-sa/4.0/
 * 
 * The original source can be found here:
 * https://p5js.org/examples/motion-non-orthogonal-reflection.html
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

//Position of left hand side of floor
base1 = 0;

//Position of right hand side of floor
base2 = 0;
//Length of floor
//var baseLength;

// Variables related to moving ball
position = 0;
velocity = 0;
r = 6;
speed = 3.5;

function setup() {
	fill(128);
	base1 = createVector(0, height - 150);
	base2 = createVector(width, height);
	//createGround();

	//start ellipse at middle top of screen
	position = createVector(width / 2, 0);

	//calculate initial random velocity
	velocity = PVector.random2D();
	velocity.mult(speed);
}

function draw() {
	//draw background
	fill(0);
	noStroke();
	rect(0, 0, width, height);

	//draw base
	fill(200);
	quad(base1.x, base1.y, base2.x, base2.y, base2.x, height, 0, height);

	//calculate base top normal
	var baseDelta = PVector.sub(base2, base1);
	baseDelta.normalize();
	var normal = createVector(-baseDelta.y, baseDelta.x);
	var intercept = PVector.dot(base1, normal);

	//draw ellipse
	noStroke();
	fill(255);
	ellipse(position.x, position.y, r * 2, r * 2);

	//move ellipse
	position.add(velocity);

	//normalized incidence vector
	incidence = PVector.mult(velocity, -1);
	incidence.normalize();

	// detect and handle collision with base
	if (PVector.dot(normal, position) > intercept) {
		//calculate dot product of incident vector and base top
		var dot = incidence.dot(normal);

		//calculate reflection vector
		//assign reflection vector to direction vector
		velocity.set(
			2 * normal.x * dot - incidence.x,
			2 * normal.y * dot - incidence.y,
			0
		);
		velocity.mult(speed);

		// draw base top normal at collision point
		stroke(255, 128, 0);
		line(
			position.x,
			position.y,
			position.x - normal.x * 100,
			position.y - normal.y * 100
		);
	}
	//}

	// detect boundary collision
	// right
	if (position.x > width - r) {
		position.x = width - r;
		velocity.x *= -1;
	}
	// left
	if (position.x < r) {
		position.x = r;
		velocity.x *= -1;
	}
	// top
	if (position.y < r) {
		position.y = r;
		velocity.y *= -1;

		//randomize base top
		base1.y = random(height - 100, height);
		base2.y = random(height - 100, height);
	}
}
