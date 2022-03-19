/*
OpenGL example code from
https://cs.lmu.edu/~ray/notes/openglexamples/

ported version shared with kind permission of Ray Toal (https://twitter.com/rtoal)
*/
LoadLibrary("ogl");

var moon;
var orbiter;

function Setup() {
	glInit();

	glEnable(GL.DEPTH_TEST);
	var yellow = [1.0, 1.0, 0.5, 1.0];
	glLight(GL.LIGHT0, GL.DIFFUSE, yellow);
	glEnable(GL.LIGHTING);
	glEnable(GL.LIGHT0);
	moon = new Moon();
	orbiter = new Orbiter(5.0);

	reshape(GL.WIDTH, GL.HEIGHT);
}

function Loop() {
	glClear(GL.COLOR_BUFFER_BIT | GL.DEPTH_BUFFER_BIT);
	glMatrixMode(GL.MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	var pos = orbiter.getPosition();
	gluLookAt(pos.x, pos.y, pos.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	moon.draw();
	glPopMatrix();
	glFlush();

	orbiter.advance(0.01);
}

function Input(e) {
}

// reshape() fixes up the projection matrix so that we always see a sphere
// instead of an ellipsoid.
function reshape(w, h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL.PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, w / h, 1.0, 10.0);
}


function Moon() {
	this.displayListId = glGenLists(1);
	glNewList(this.displayListId, GL.COMPILE);
	var direction = [-1.0, -1.0, -1.0, 0.0];
	glLight(GL.LIGHT0, GL.POSITION, direction);
	glutSolidSphere(1.0, 25, 25);
	glEndList();
};
Moon.prototype.draw = function () {
	glCallList(this.displayListId);
};

function Orbiter(r) {
	this.radius = r;
	this.u = 0;
};
Orbiter.prototype.advance = function (delta) {
	this.u += delta;
};
Orbiter.prototype.getPosition = function () {
	var x = this.radius * Math.cos(this.u);
	var z = this.radius * Math.sin(this.u);
	return { x: x, y: 0, z: z };
};
