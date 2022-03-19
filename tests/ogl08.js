/*
OpenGL example code from
https://cs.lmu.edu/~ray/notes/openglexamples/
*/
LoadLibrary("ogl");

function Setup() {
    glInit();

    var black = [0.0, 0.0, 0.0, 1.0];
    var yellow = [1.0, 1.0, 0.0, 1.0];
    var cyan = [0.0, 1.0, 1.0, 1.0];
    var white = [1.0, 1.0, 1.0, 1.0];
    var direction = [1.0, 1.0, 1.0, 0.0];

    glMaterial(GL.FRONT, GL.AMBIENT_AND_DIFFUSE, cyan);
    glMaterial(GL.FRONT, GL.SPECULAR, white);
    glMaterial(GL.FRONT, GL.SHININESS, 30);

    glLight(GL.LIGHT0, GL.AMBIENT, black);
    glLight(GL.LIGHT0, GL.DIFFUSE, yellow);
    glLight(GL.LIGHT0, GL.SPECULAR, white);
    glLight(GL.LIGHT0, GL.POSITION, direction);

    glEnable(GL.LIGHTING);                // so the renderer considers light
    glEnable(GL.LIGHT0);                  // turn LIGHT0 on
    glEnable(GL.DEPTH_TEST);              // so the renderer considers depth

    reshape(GL.WIDTH, GL.HEIGHT);
}

// Clears the window and depth buffer and draws three solids.
//
// The solids are placed so that they either sit or float above the x-z plane;
// therefore note one of the first things that is done is to rotate the whole
// scene 20 degrees about x to turn the top of the scene toward the viewer.
// This lets the viewer see how the torus goes around the cone.
function Loop() {
    glClear(GL.COLOR_BUFFER_BIT | GL.DEPTH_BUFFER_BIT);
    glMatrixMode(GL.MODELVIEW);
    glPushMatrix();

    // Rotate the scene so we can see the tops of the shapes.
    glRotate(-20.0, 1.0, 0.0, 0.0);

    // Make a torus floating 0.5 above the x-z plane.  The standard torus in
    // the GLUT library is, perhaps surprisingly, a stack of circles which
    // encircle the z-axis, so we need to rotate it 90 degrees about x to
    // get it the way we want.
    glPushMatrix();
    glTranslate(-0.75, 0.5, 0.0);
    glRotate(90.0, 1.0, 0.0, 0.0);
    glutSolidTorus(0.275, 0.85, 16, 40);
    glPopMatrix();

    // Make a cone.  The standard cone "points" along z; we want it pointing
    // along y, hence the 270 degree rotation about x.
    glPushMatrix();
    glTranslate(-0.75, -0.5, 0.0);
    glRotate(270.0, 1.0, 0.0, 0.0);
    glutSolidCone(1.0, 2.0, 70, 12);
    glPopMatrix();

    // Add a sphere to the scene.
    glPushMatrix();
    glTranslate(0.75, 0.0, -1.0);
    glutSolidSphere(1.0, 30, 30);
    glPopMatrix();

    glPopMatrix();
    glFlush();
}

// We don't want the scene to get distorted when the window size changes, so
// we need a reshape callback.  We'll always maintain a range of -2.5..2.5 in
// the smaller of the width and height for our viewbox, and a range of -10..10
// for the viewbox depth.
function reshape(w, h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL.PROJECTION);
    var aspect = w / h;
    glLoadIdentity();
    if (w <= h) {
        // width is smaller, so stretch out the height
        glOrtho(-2.5, 2.5, -2.5 / aspect, 2.5 / aspect, -10.0, 10.0);
    } else {
        // height is smaller, so stretch out the width
        glOrtho(-2.5 * aspect, 2.5 * aspect, -2.5, 2.5, -10.0, 10.0);
    }
}

function Input(e) {
}
