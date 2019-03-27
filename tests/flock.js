/**
 * This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * http://creativecommons.org/licenses/by-nc-sa/4.0/
 * 
 * The original source can be found here:
 * https://p5js.org/examples/hello-p5-flocking.html
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5vect');

/*
** This function is called once when the script is started.
*/
function Setup() {
	width = SizeX();
	height = SizeY();

	flock = new Flock();
	// Add an initial set of boids into the system
	for (var i = 0; i < 5; i++) {
		flock.addBoid(new Boid(width / 2, height / 2));
	}

	SetFramerate(30);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	ClearScreen(EGA.BLACK);

	flock.run();

	TextXY(10, 10, "rate=" + GetFramerate(), EGA.BLACK, EGA.LIGHT_BLUE);
}

/*
** This function is called on any input.
*/
function Input(event) {
}

function random(max) {
	return Math.random() * max;
}

function radians(angle) {
	return angle * (Math.PI / 180.0);
}

//////
// class Flock
function Flock() {
	this.boids = []
}
Flock.prototype.run = function () {
	for (var i = 0; i < this.boids.length; i++) {
		this.boids[i].run(this.boids);  // Passing the entire list of boids to each boid individually
	}
}
Flock.prototype.addBoid = function (b) {
	this.boids.push(b);
}

//////
// class Boid
function Boid(x, y) {
	this.acceleration = new PVector(0, 0);

	// This is a new PVector method not yet implemented in JS
	// velocity = PVector.random2D();

	// Leaving the code temporarily this way so that this example runs in JS
	var angle = random(Math.PI * 2);
	this.velocity = new PVector(Math.cos(angle), Math.sin(angle));

	this.position = new PVector(x, y);
	this.r = 2.0;
	this.maxspeed = 2;
	this.maxforce = 0.03;
}
Boid.prototype.run = function (boids) {
	this.flock(boids);
	this.update();
	this.borders();
	this.render();
}
Boid.prototype.applyForce = function (force) {
	// We could add mass here if we want A = F / M
	this.acceleration.add(force);
}
Boid.prototype.flock = function (boids) {
	// var sep = this.separate(boids);   // Separation
	// var ali = this.align(boids);      // Alignment
	// var coh = this.cohesion(boids);   // Cohesion
	var aio = this.sealco(boids);
	var sep = aio[0];
	var ali = aio[1];
	var coh = aio[2];

	// Arbitrarily weight these forces
	sep.mult(1.5);
	ali.mult(1.0);
	coh.mult(1.0);
	// Add the force vectors to acceleration
	this.applyForce(sep);
	this.applyForce(ali);
	this.applyForce(coh);
}
Boid.prototype.update = function () {
	// Update velocity
	this.velocity.add(this.acceleration);
	// Limit speed
	this.velocity.limit(this.maxspeed);
	this.position.add(this.velocity);
	// Reset accelertion to 0 each cycle
	this.acceleration.mult(0);
}
Boid.prototype.seek = function (target) {
	var desired = PVector.sub(target, this.position);  // A vector pointing from the position to the target
	// Scale to maximum speed
	desired.normalize();
	desired.mult(this.maxspeed);

	// Above two lines of code below could be condensed with new PVector setMag() method
	// Not using this method until Processing.js catches up
	// desired.setMag(maxspeed);

	// Steering = Desired minus Velocity
	var steer = PVector.sub(desired, this.velocity);
	steer.limit(this.maxforce);  // Limit to maximum steering force
	return steer;
}
Boid.prototype.render = function () {
	//Plot(this.position.x, this.position.y, EGA.WHITE);
	Circle(this.position.x, this.position.y, 4, EGA.WHITE);
}
Boid.prototype.borders = function () {
	if (this.position.x < -this.r) this.position.x = width + this.r;
	if (this.position.y < -this.r) this.position.y = height + this.r;
	if (this.position.x > width + this.r) this.position.x = -this.r;
	if (this.position.y > height + this.r) this.position.y = -this.r;
}
Boid.prototype.sealco = function (boids) {
	var desiredseparation = 25.0;
	var neighbordist = 50;
	var sep = new PVector(0, 0, 0);
	var sum = new PVector(0, 0);
	var count = 0;
	// For every boid in the system, check if it's too close
	for (var i = 0; i < boids.length; i++) {
		var d = PVector.dist(this.position, boids[i].position);
		// If the distance is greater than 0 and less than an arbitrary amount (0 when you are yourself)
		if ((d > 0) && (d < desiredseparation)) {
			// Calculate vector pointing away from neighbor
			var diff = PVector.sub(this.position, boids[i].position);
			diff.normalize();
			diff.div(d);        // Weight by distance
			sep.add(diff);
			count++;            // Keep track of how many
		}
		if ((d > 0) && (d < neighbordist)) {
			sum.add(boids[i].velocity);
			count++;
		}
	}
	// Average -- divide by how many
	if (count > 0) {
		sep.div(count);
	}

	// As long as the vector is greater than 0
	if (sep.mag() > 0) {
		// First two lines of code below could be condensed with new PVector setMag() method
		// Not using this method until Processing.js catches up
		// steer.setMag(maxspeed);

		// Implement Reynolds: Steering = Desired - Velocity
		sep.normalize();
		sep.mult(this.maxspeed);
		sep.sub(this.velocity);
		sep.limit(this.maxforce);
	}

	var ali = new PVector(0, 0);
	var coh = new PVector(0, 0);
	if (count > 0) {
		var coh_sum = sum.copy();

		sum.div(count);
		// First two lines of code below could be condensed with new PVector setMag() method
		// Not using this method until Processing.js catches up
		// sum.setMag(maxspeed);

		// Implement Reynolds: Steering = Desired - Velocity
		sum.normalize();
		sum.mult(this.maxspeed);
		ali = PVector.sub(sum, this.velocity);
		ali.limit(this.maxforce);

		coh_sum.div(count);
		coh = this.seek(coh_sum);  // Steer towards the position
	}

	return [sep, ali, coh];
}

Boid.prototype.separate = function (boids) {
	var desiredseparation = 25.0;
	var steer = new PVector(0, 0, 0);
	var count = 0;
	// For every boid in the system, check if it's too close
	for (var i = 0; i < boids.length; i++) {
		var d = PVector.dist(this.position, boids[i].position);
		// If the distance is greater than 0 and less than an arbitrary amount (0 when you are yourself)
		if ((d > 0) && (d < desiredseparation)) {
			// Calculate vector pointing away from neighbor
			var diff = PVector.sub(this.position, boids[i].position);
			diff.normalize();
			diff.div(d);        // Weight by distance
			steer.add(diff);
			count++;            // Keep track of how many
		}
	}
	// Average -- divide by how many
	if (count > 0) {
		steer.div(count);
	}

	// As long as the vector is greater than 0
	if (steer.mag() > 0) {
		// First two lines of code below could be condensed with new PVector setMag() method
		// Not using this method until Processing.js catches up
		// steer.setMag(maxspeed);

		// Implement Reynolds: Steering = Desired - Velocity
		steer.normalize();
		steer.mult(this.maxspeed);
		steer.sub(this.velocity);
		steer.limit(this.maxforce);
	}
	return steer;
}
Boid.prototype.align = function (boids) {
	var neighbordist = 50;
	var sum = new PVector(0, 0);
	var count = 0;
	for (var i = 0; i < boids.length; i++) {
		var d = PVector.dist(this.position, boids[i].position);
		if ((d > 0) && (d < neighbordist)) {
			sum.add(boids[i].velocity);
			count++;
		}
	}
	if (count > 0) {
		sum.div(count);
		// First two lines of code below could be condensed with new PVector setMag() method
		// Not using this method until Processing.js catches up
		// sum.setMag(maxspeed);

		// Implement Reynolds: Steering = Desired - Velocity
		sum.normalize();
		sum.mult(this.maxspeed);
		var steer = PVector.sub(sum, this.velocity);
		steer.limit(this.maxforce);
		return steer;
	}
	else {
		return new PVector(0, 0);
	}
}
Boid.prototype.cohesion = function (boids) {
	var neighbordist = 50;
	var sum = new PVector(0, 0);   // Start with empty vector to accumulate all positions
	var count = 0;
	for (var i = 0; i < boids.length; i++) {
		var d = PVector.dist(this.position, boids[i].position);
		if ((d > 0) && (d < neighbordist)) {
			sum.add(boids[i].position); // Add position
			count++;
		}
	}
	if (count > 0) {
		sum.div(count);
		return this.seek(sum);  // Steer towards the position
	}
	else {
		return new PVector(0, 0);
	}
}
