/**
 * This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * http://creativecommons.org/licenses/by-nc-sa/4.0/
 * 
 * The original source can be found here:
 * https://p5js.org/examples/simulate-chain.html
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
lastEvent = null;

s1 = s2 = s3 = s4 = null;

gravity = 9.0;
mass = 2.0;

/*
** This function is called once when the script is started.
*/
function Setup() {
	MouseSetColors(EGA.RED, EGA.BLACK);
	SetFramerate(40);

	s1 = new Spring2D(0.0, SizeX() / 2, mass, gravity);
	s2 = new Spring2D(0.0, SizeX() / 2, mass, gravity);
	s3 = new Spring2D(0.0, SizeX() / 2, mass, gravity);
	s4 = new Spring2D(0.0, SizeX() / 2, mass, gravity);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	if (lastEvent) {
		ClearScreen(EGA.BLACK);

		s1.update(lastEvent.x, lastEvent.y);
		s1.display(lastEvent.x, lastEvent.y);
		s2.update(s1.x, s1.y);
		s2.display(s1.x, s1.y);
		s3.update(s2.x, s2.y);
		s3.display(s2.x, s2.y);
		s4.update(s3.x, s3.y);
		s4.display(s3.x, s3.y);
	}
}

/*
** This function is called on any input.
*/
function Input(event) {
	lastEvent = event;
}

//////
// class Spring2D
function Spring2D(xpos, ypos, m, g) {
	this.x = xpos; // The x- and y-coordinates
	this.y = ypos;
	this.mass = m;
	this.gravity = g;
	this.radius = 30;
	this.stiffness = 0.2;
	this.damping = 0.7;
	this.vx = 0; // The x- and y-axis velocities
	this.vy = 0;
}
Spring2D.prototype.update = function (targetX, targetY) {
	var forceX = (targetX - this.x) * this.stiffness;
	var ax = forceX / this.mass;
	this.vx = this.damping * (this.vx + ax);
	this.x += this.vx;
	var forceY = (targetY - this.y) * this.stiffness;
	forceY += this.gravity;
	var ay = forceY / this.mass;
	this.vy = this.damping * (this.vy + ay);
	this.y += this.vy;
}
Spring2D.prototype.display = function (nx, ny) {
	FilledCircle(this.x, this.y, this.radius, EGA.WHITE);
	Line(this.x, this.y, nx, ny, EGA.RED);
}
