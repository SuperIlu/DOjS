/*
OpenGL example code from
https://cs.lmu.edu/~ray/notes/openglexamples/

ported version shared with kind permission of Ray Toal (https://twitter.com/rtoal)
*/
LoadLibrary("ogl");

var texture;

function Setup() {
	// create texture
	texture = new Bitmap(2, 2);
	SetRenderBitmap(texture);
	ClearScreen(EGA.BLACK);
	Plot(0, 0, EGA.RED);
	Plot(1, 0, EGA.YELLOW);
	Plot(0, 1, EGA.YELLOW);
	Plot(1, 1, EGA.RED);
	SetRenderBitmap(null);

	glInit();

	reshape(GL.WIDTH, GL.HEIGHT);
}

function Loop() {
	glClear(GL.COLOR_BUFFER_BIT);
	glBegin(GL.TRIANGLES);
	glTexCoord2(0.5, 1.0); glVertex2(-3, 3);
	glTexCoord2(0.0, 0.0); glVertex2(-3, 0);
	glTexCoord2(1.0, 0.0); glVertex2(0, 0);

	glTexCoord2(4, 8); glVertex2(3, 3);
	glTexCoord2(0.0, 0.0); glVertex2(0, 0);
	glTexCoord2(8, 0.0); glVertex2(3, 0);

	glTexCoord2(5, 5); glVertex2(0, 0);
	glTexCoord2(0.0, 0.0); glVertex2(-1.5, -3);
	glTexCoord2(4, 0.0); glVertex2(1.5, -3);
	glEnd();
	glFlush();
}

// Fixes up camera and remaps texture when window reshaped.
function reshape(width, height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL.PROJECTION);
	glLoadIdentity();
	gluPerspective(80, width / height, 1, 40);
	glMatrixMode(GL.MODELVIEW);
	glLoadIdentity();
	gluLookAt(2, -1, 5, 0, 0, 0, 0, 1, 0);
	glEnable(GL.TEXTURE_2D);
	glPixelStore(GL.UNPACK_ALIGNMENT, 1);
	glTexImage2D(
		0,                    // level 0
		3,                    // use only R, G, and B components
		0,                    // no border
		texture);
	glTexParameter(GL.TEXTURE_2D, GL.TEXTURE_MIN_FILTER, GL.NEAREST);
	glTexParameter(GL.TEXTURE_2D, GL.TEXTURE_MAG_FILTER, GL.NEAREST);
}

function Input(e) {
}
