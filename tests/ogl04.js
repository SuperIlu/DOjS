/*
OpenGL example code from
https://cs.lmu.edu/~ray/notes/openglexamples/

ported version shared with kind permission of Ray Toal (https://twitter.com/rtoal)
*/
LoadLibrary("ogl");

// Performs application-specific initialization. Sets colors and sets up a
// simple orthographic projection.
function Setup() {
    glInit();

    glEnable(GL.DEPTH_TEST);

    reshape(GL.WIDTH, GL.HEIGHT);
}

function midpoint(p, r) {
    return {
        x: (r.x + p.x) / 2.0,
        y: (r.y + p.y) / 2.0,
        z: (r.z + p.z) / 2.0
    };
}

// Handles reshape requests by setting the the viewport to exactly match the
// pixel coordinates, so we can draw in the whole window.  Then sets up a
// simple perspective viewing volume to ensure that the pyramid will never
// be distorted when the window is reshaped.  The particular settings chosen
// ensure that the vertical extent of the pyramid will always be visible.
function reshape(w, h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL.PROJECTION);
    glLoadIdentity();
    gluPerspective(100.0, w / h, 10.0, 1500.0);
}

// Draws the "next 500 points".  The function contains locally the definitions
// of the vertices and the current point as static objects so that their values
// are retained between calls.
function generateMorePoints() {

    // The tetrahedron has four vertices.  We also have to keep track of the
    // current point during the plotting.
    var vertices = [
        { x: -250, y: -225, z: -200 },
        { x: -150, y: -225, z: -700 },
        { x: 250, y: -225, z: -275 },
        { x: 0, y: 450, z: -500 }
    ];
    var lastPoint = vertices[0];

    // Here is the code to draw the "next 500 points".  The closer the point is
    // to the camera, the brighter we make it.  The coloring technique is a
    // quick hack which (unprofessionally) depends on the fact that the range
    // of z-coordinates in the tetrahedron run from -200 to -700.  By adding
    // 700 to any z-value and then dividing by 500, we get values in the range
    // 0 to 1 - just perfect for coloring.
    glBegin(GL.POINTS);
    for (var i = 0; i <= 25000; i++) {
        lastPoint = midpoint(lastPoint, vertices[GetRandomInt(123456) % 4]);
        var intensity = (700 + lastPoint.z) / 500.0;
        glColor3(intensity, intensity, 0.25);
        glVertex3(lastPoint.x, lastPoint.y, lastPoint.z);
    }
    glEnd();
    glFlush();
}

// Clears the window and draws the torus.
function Loop() {
    glClear(GL.COLOR_BUFFER_BIT | GL.DEPTH_BUFFER_BIT);

    generateMorePoints();
}

function Input(e) {
}
