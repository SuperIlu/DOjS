/*
OpenGL example code from
https://cs.lmu.edu/~ray/notes/openglexamples/

ported version shared with kind permission of Ray Toal (https://twitter.com/rtoal)
*/
LoadLibrary("ogl");

// Sets up global attributes like clear color and drawing color, and sets up
// the desired projection and modelview matrices.
function Setup() {
    glInit();

    // Set the current clear color to black and the current drawing color to
    // white.
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glColor3(1.0, 1.0, 1.0);

    // Set the camera lens to have a 60 degree (vertical) field of view, an
    // aspect ratio of 4/3, and have everything closer than 1 unit to the
    // camera and greater than 40 units distant clipped away.
    glMatrixMode(GL.PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 4.0 / 3.0, 1, 40);

    // Position camera at (4, 6, 5) looking at (0, 0, 0) with the vector
    // <0, 1, 0> pointing upward.
    glMatrixMode(GL.MODELVIEW);
    glLoadIdentity();
    gluLookAt(4, 6, 5, 0, 0, 0, 0, 1, 0);
}

// Clears the window and draws the torus.
function Loop() {
    glClear(GL.COLOR_BUFFER_BIT);

    // Draw a white torus of outer radius 3, inner radius 0.5 with 15 stacks
    // and 30 slices.
    glColor3(1.0, 1.0, 1.0);
    glutWireTorus(0.5, 3, 15, 30);

    // Draw a red x-axis, a green y-axis, and a blue z-axis.  Each of the
    // axes are ten units long.
    glBegin(GL.LINES);
    glColor3(1, 0, 0); glVertex3(0, 0, 0); glVertex3(10, 0, 0);
    glColor3(0, 1, 0); glVertex3(0, 0, 0); glVertex3(0, 10, 0);
    glColor3(0, 0, 1); glVertex3(0, 0, 0); glVertex3(0, 0, 10);
    glEnd();

    glFlush();
}

function Input(e) {
}
