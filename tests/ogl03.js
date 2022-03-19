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

    // Set a deep purple background and draw in a greenish yellow.
    glClearColor(0.25, 0.0, 0.2, 1.0);
    glColor3(0.6, 1.0, 0.0);

    // Set up the viewing volume: 500 x 500 x 1 window with origin lower left.
    glMatrixMode(GL.PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 500.0, 0.0, 500.0, 0.0, 1.0);
}

function midpoint(p, r) {
    return {
        x: (r.x + p.x) / 2.0,
        y: (r.y + p.y) / 2.0
    };
}

var vertices = [
    { x: 0, y: 0 },
    { x: 200, y: 500 },
    { x: 500, y: 0 }
];

// Clears the window and draws the torus.
function Loop() {
    glClear(GL.COLOR_BUFFER_BIT);

    // Compute and plot 100000 new points, starting (arbitrarily) with one of
    // the vertices. Each point is halfway between the previous point and a
    // randomly chosen vertex.
    var p = { x: 0, y: 0 };
    glBegin(GL.POINTS);
    for (var k = 0; k < 100000; k++) {
        p = midpoint(p, vertices[GetRandomInt(123456) % 3]);
        glVertex2(p.x, p.y);
    }
    glEnd();
    glFlush();

}

function Input(e) {
}
