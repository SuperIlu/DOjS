/*
OpenGL example code from
https://cs.lmu.edu/~ray/notes/openglexamples/

ported version shared with kind permission of Ray Toal (https://twitter.com/rtoal)
*/
LoadLibrary("ogl");

// This global variable keeps track of the current orientation of the square.
// It is better to maintain the "state" of the animation globally rather
// than trying to successively accumulate transformations.  Over a period of
// time the approach of accumulating transformation matrices generally
// degrades due to rounding errors.
var currentAngleOfRotation = 0.0;


// Performs application-specific initialization. Sets colors and sets up a
// simple orthographic projection.
function Setup() {
    glInit();

    glEnable(GL.DEPTH_TEST);

    reshape(GL.WIDTH, GL.HEIGHT);
}

// Handles the window reshape event by first ensuring that the viewport fills
// the entire drawing surface.  Then we use a simple orthographic projection
// with a logical coordinate system ranging from -50..50 in the smaller of
// the width or height, and scale the larger dimension to make the whole
// window isotropic.
function reshape(w, h) {
    glViewport(0, 0, w, h);
    var aspect = w / h;
    glMatrixMode(GL.PROJECTION);
    glLoadIdentity();
    if (w <= h) {
        // width is smaller, go from -50 .. 50 in width
        glOrtho(-50.0, 50.0, -50.0 / aspect, 50.0 / aspect, -1.0, 1.0);
    } else {
        // height is smaller, go from -50 .. 50 in height
        glOrtho(-50.0 * aspect, 50.0 * aspect, -50.0, 50.0, -1.0, 1.0);
    }
}

// Clears the window and draws the torus.
function Loop() {
    glClear(GL.COLOR_BUFFER_BIT);
    glMatrixMode(GL.MODELVIEW);
    glLoadIdentity();
    glRotate(currentAngleOfRotation, 0.0, 0.0, 1.0);
    glRect(-25.0, -25.0, 25.0, 25.0);
    glFlush();

    currentAngleOfRotation += 1.0;
    if (currentAngleOfRotation > 360.0) {
        currentAngleOfRotation -= 360.0;
    }
    reshape(GL.WIDTH, GL.HEIGHT);
}

function Input(e) {
}
