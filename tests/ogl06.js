/*
OpenGL example code from
https://cs.lmu.edu/~ray/notes/openglexamples/

ported version shared with kind permission of Ray Toal (https://twitter.com/rtoal)
*/
LoadLibrary("ogl");

// The robot arm is specified by (1) the angle that the upper arm makes
// relative to the x-axis, called shoulderAngle, and (2) the angle that the
// lower arm makes relative to the upper arm, called elbowAngle.  These angles
// are adjusted in 5 degree increments by a keyboard callback.
var shoulderAngle = 0, elbowAngle = 0;

// wireBox(w, h, d) makes a wireframe box with width w, height h and
// depth d centered at the origin.  It uses the GLUT wire cube function.
// The calls to glPushMatrix and glPopMatrix are essential here; they enable
// this function to be called from just about anywhere and guarantee that
// the glScalef call does not pollute code that follows a call to myWireBox.
function wireBox(width, height, depth) {
    glPushMatrix();
    glScale(width, height, depth);
    glutWireCube(1.0);
    glPopMatrix();
}

// Performs application-specific initialization. Sets colors and sets up a
// simple orthographic projection.
function Setup() {
    glInit();

    glShadeModel(GL.FLAT);
    glMatrixMode(GL.MODELVIEW);
    glLoadIdentity();
    gluLookAt(1, 2, 8, 0, 0, 0, 0, 1, 0);

    reshape(GL.WIDTH, GL.HEIGHT);
}

function reshape(w, h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL.PROJECTION);
    glLoadIdentity();
    gluPerspective(65.0, w / h, 1.0, 20.0);
}

// Displays the arm in its current position and orientation.  The whole
// function is bracketed by glPushMatrix and glPopMatrix calls because every
// time we call it we are in an "environment" in which a gluLookAt is in
// effect.  (Note that in particular, replacing glPushMatrix with
// glLoadIdentity makes you lose the camera setting from gluLookAt).
function Loop() {
    glClear(GL.COLOR_BUFFER_BIT);
    glMatrixMode(GL.MODELVIEW);
    glPushMatrix();

    // Draw the upper arm, rotated shoulder degrees about the z-axis.  Note that
    // the thing about glutWireBox is that normally its origin is in the middle
    // of the box, but we want the "origin" of our box to be at the left end of
    // the box, so it needs to first be shifted 1 unit in the x direction, then
    // rotated.
    glRotate(shoulderAngle, 0.0, 0.0, 1.0);
    glTranslate(1.0, 0.0, 0.0);
    wireBox(2.0, 0.4, 1.0);

    // Now we are ready to draw the lower arm.  Since the lower arm is attached
    // to the upper arm we put the code here so that all rotations we do are
    // relative to the rotation that we already made above to orient the upper
    // arm.  So, we want to rotate elbow degrees about the z-axis.  But, like
    // before, the anchor point for the rotation is at the end of the box, so
    // we translate <1,0,0> before rotating.  But after rotating we have to
    // position the lower arm at the end of the upper arm, so we have to
    // translate it <1,0,0> again.
    glTranslate(1.0, 0.0, 0.0);
    glRotate(elbowAngle, 0.0, 0.0, 1.0);
    glTranslate(1.0, 0.0, 0.0);
    wireBox(2.0, 0.4, 1.0);

    glPopMatrix();
    glFlush();
}

function Input(e) {
    var key = e.key >> 8;
    switch (key) {
        case KEY.Code.KEY_LEFT:
            elbowAngle += 5;
            elbowAngle %= 360;
            break;
        case KEY.Code.KEY_RIGHT:
            elbowAngle -= 5;
            elbowAngle %= 360;
            break;
        case KEY.Code.KEY_UP:
            shoulderAngle += 5;
            shoulderAngle %= 360;
            break;
        case KEY.Code.KEY_DOWN:
            shoulderAngle -= 5;
            shoulderAngle %= 360;
            break;
    }
}
