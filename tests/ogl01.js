/*
OpenGL example code from
https://cs.lmu.edu/~ray/notes/openglexamples/

ported version shared with kind permission of Ray Toal (https://twitter.com/rtoal)
*/
LoadLibrary("ogl");

var count = 0;
for (var e in global) {
    if (typeof global[e] === "function") {
        if (e.startsWith("gl")) {
            count++;
        }
    }
}
Println("GL members        : " + Object.keys(GL).length);
Println("Open GL functions : " + count);

// Sets up global attributes like clear color and drawing color, enables and
// initializes any needed modes (in this case we want backfaces culled), and
// sets up the desired projection and modelview matrices. It is cleaner to
// define these operations in a function separate from main().
function Setup() {
    glInit();

    // Set the current clear color to sky blue and the current drawing color to
    // white.
    glClearColor(0.1, 0.39, 0.88, 1.0);
    glColor3(1.0, 1.0, 1.0);

    // Tell the rendering engine not to draw backfaces.  Without this code,
    // all four faces of the tetrahedron would be drawn and it is possible
    // that faces farther away could be drawn after nearer to the viewer.
    // Since there is only one closed polyhedron in the whole scene,
    // eliminating the drawing of backfaces gives us the realism we need.
    // THIS DOES NOT WORK IN GENERAL.
    glEnable(GL.CULL_FACE);
    glCullFace(GL.BACK);

    // Set the camera lens so that we have a perspective viewing volume whose
    // horizontal bounds at the near clipping plane are -2..2 and vertical
    // bounds are -1.5..1.5.  The near clipping plane is 1 unit from the camera
    // and the far clipping plane is 40 units away.
    glMatrixMode(GL.PROJECTION);
    glLoadIdentity();
    glFrustum(-2, 2, -1.5, 1.5, 1, 40);

    // Set up transforms so that the tetrahedron which is defined right at
    // the origin will be rotated and moved into the view volume.  First we
    // rotate 70 degrees around y so we can see a lot of the left side.
    // Then we rotate 50 degrees around x to "drop" the top of the pyramid
    // down a bit.  Then we move the object back 3 units "into the screen".
    glMatrixMode(GL.MODELVIEW);
    glLoadIdentity();
    glTranslate(0, 0, -3);
    glRotate(50, 1, 0, 0);
    glRotate(70, 0, 1, 0);
}

// Clears the window and draws the tetrahedron.  The tetrahedron is  easily
// specified with a triangle strip, though the specification really isn't very
// easy to read.
function Loop() {
    glClear(GL.COLOR_BUFFER_BIT);

    // Draw a white grid "floor" for the tetrahedron to sit on.
    glColor3(1.0, 1.0, 1.0);
    glBegin(GL.LINES);
    for (var i = -2.5; i <= 2.5; i += 0.25) {
        glVertex3(i, 0, 2.5);
        glVertex3(i, 0, -2.5);
        glVertex3(2.5, 0, i);
        glVertex3(-2.5, 0, i);
    }
    glEnd();

    // Draw the tetrahedron.  It is a four sided figure, so when defining it
    // with a triangle strip we have to repeat the last two vertices.
    glBegin(GL.TRIANGLE_STRIP);
    glColor3(1, 1, 1);
    glVertex3(0, 2, 0);
    glColor3(1, 0, 0);
    glVertex3(-1, 0, 1);
    glColor3(0, 1, 0);
    glVertex3(1, 0, 1);
    glColor3(0, 0, 1);
    glVertex3(0, 0, -1.4);
    glColor3(1, 1, 1);
    glVertex3(0, 2, 0);
    glColor3(1, 0, 0);
    glVertex3(-1, 0, 1);
    glEnd();

    glFlush();
}

function Input(e) {
}
