/*
OpenGL example code from
https://cs.lmu.edu/~ray/notes/openglexamples/

ported version shared with kind permission of Ray Toal (https://twitter.com/rtoal)
*/
LoadLibrary("ogl");

// Colors
var WHITE = [1, 1, 1];
var RED = [1, 0, 0];
var GREEN = [0, 1, 0];
var MAGENTA = [1, 0, 1];

var balls, checkerboard, camera;

function Setup() {
	glInit();

	glEnable(GL.DEPTH_TEST);
	glLight(GL.LIGHT0, GL.DIFFUSE, WHITE);
	glLight(GL.LIGHT0, GL.SPECULAR, WHITE);
	glMaterial(GL.FRONT, GL.SPECULAR, WHITE);
	glMaterial(GL.FRONT, GL.SHININESS, 30);
	glEnable(GL.LIGHTING);
	glEnable(GL.LIGHT0);
	checkerboard = new Checkerboard(8, 8);
	camera = new Camera();
	balls = [
		new Ball(1, GREEN, 7, 6, 1),
		new Ball(1.5, MAGENTA, 6, 3, 4),
		new Ball(0.4, WHITE, 5, 1, 7)
	];

	reshape(GL.WIDTH, GL.HEIGHT);
}

function Loop() {
	glClear(GL.COLOR_BUFFER_BIT | GL.DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(camera.getX(), camera.getY(), camera.getZ(),
		checkerboard.centerx(), 0.0, checkerboard.centerz(),
		0.0, 1.0, 0.0);
	checkerboard.draw();
	for (var i = 0; i < balls.length; i++) {
		balls[i].update();
	}
	glFlush();
}

// On reshape, constructs a camera that perfectly fits the window.
function reshape(w, h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL.PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, w / h, 1.0, 150.0);
	glMatrixMode(GL.MODELVIEW);
}

function Input(e) {
	var key = e.key >> 8;
	switch (key) {
		case KEY.Code.KEY_LEFT:
			camera.moveLeft();
			break;
		case KEY.Code.KEY_RIGHT:
			camera.moveRight();
			break;
		case KEY.Code.KEY_UP:
			camera.moveUp();
			break;
		case KEY.Code.KEY_DOWN:
			camera.moveDown();
			break;
	}
}

function Camera() {
	this.theta = 0;
	this.y = 3;
	this.dTheta = 0.04;
	this.dy = 0.2;
};
Camera.prototype.getX = function () { return 10 * Math.cos(this.theta); };
Camera.prototype.getY = function () { return this.y; };
Camera.prototype.getZ = function () { return 10 * Math.sin(this.theta); };
Camera.prototype.moveRight = function () { this.theta += this.dTheta; };
Camera.prototype.moveLeft = function () { this.theta -= this.dTheta; };
Camera.prototype.moveUp = function () { this.y += this.dy; };
Camera.prototype.moveDown = function () { this.y -= this.dy; };

function Ball(r, c, h, x, z) {
	this.radius = r;
	this.color = c;
	this.maximumHeight = h;
	this.direction = -1;
	this.y = h;
	this.x = x;
	this.z = z;
};
Ball.prototype.update = function () {
	this.y += this.direction * 0.05;
	if (this.y > this.maximumHeight) {
		this.y = this.maximumHeight;
		this.direction = -1;
	} else if (this.y < this.radius) {
		this.y = this.radius;
		this.direction = 1;
	}
	glPushMatrix();
	glMaterial(GL.FRONT, GL.AMBIENT_AND_DIFFUSE, this.color);
	glTranslate(this.x, this.y, this.z);
	glutSolidSphere(this.radius, 30, 30);
	glPopMatrix();
};

function Checkerboard(w, d) {
	this.width = w;
	this.depth = d;

	this.displayListId = glGenLists(1);
	glNewList(this.displayListId, GL.COMPILE);
	var lightPosition = [4, 3, 7, 1];
	glLight(GL.LIGHT0, GL.POSITION, lightPosition);
	glBegin(GL.QUADS);
	glNormal3(0, 1, 0);
	for (var x = 0; x < this.width - 1; x++) {
		for (var z = 0; z < this.depth - 1; z++) {
			glMaterial(GL.FRONT, GL.AMBIENT_AND_DIFFUSE,
				(x + z) % 2 == 0 ? RED : WHITE);
			glVertex3(x, 0, z);
			glVertex3(x + 1, 0, z);
			glVertex3(x + 1, 0, z + 1);
			glVertex3(x, 0, z + 1);
		}
	}
	glEnd();
	glEndList();
};
Checkerboard.prototype.centerx = function () { return this.width / 2; };
Checkerboard.prototype.centerz = function () { return this.depth / 2; };
Checkerboard.prototype.draw = function () { glCallList(this.displayListId); };
