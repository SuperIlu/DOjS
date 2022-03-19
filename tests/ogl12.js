/*
OpenGL example code from
https://cs.lmu.edu/~ray/notes/openglexamples/

ported version shared with kind permission of Ray Toal (https://twitter.com/rtoal)
*/
LoadLibrary("ogl");

// A fish bitmap, size is 27x11, but all rows must have a multiple of 8 bits,
// so we define it like it is 32x11.
var fish = [
	0x00, 0x60, 0x01, 0x00,
	0x00, 0x90, 0x01, 0x00,
	0x03, 0xf8, 0x02, 0x80,
	0x1c, 0x37, 0xe4, 0x40,
	0x20, 0x40, 0x90, 0x40,
	0xc0, 0x40, 0x78, 0x80,
	0x41, 0x37, 0x84, 0x80,
	0x1c, 0x1a, 0x04, 0x80,
	0x03, 0xe2, 0x02, 0x40,
	0x00, 0x11, 0x01, 0x40,
	0x00, 0x0f, 0x00, 0xe0,
];

function Setup() {
	glInit();

	reshape(GL.WIDTH, GL.HEIGHT);
}

// On reshape, uses an orthographic projection with world coordinates ranging
// from 0 to 1 in the x and y directions, and -1 to 1 in z.
function reshape(width, height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL.PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
}

// Clears the window then plots 20 fish bitmaps in random colors at random
// positions.
function Loop() {
	glClear(GL.COLOR_BUFFER_BIT);
	for (var i = 0; i < 20; i++) {
		glColor3(Math.random(), Math.random(), Math.random());
		glRasterPos3(Math.random(), Math.random(), 0.0);
		glBitmap(27, 11, 0, 0, 0, 0, fish);
	}
	glFlush();
}

function Input(e) {
}
