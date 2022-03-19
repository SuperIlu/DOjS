/*
OpenGL example code from
https://cs.lmu.edu/~ray/notes/openglexamples/

ported version shared with kind permission of Ray Toal (https://twitter.com/rtoal)
*/
LoadLibrary("ogl");

// The cube has opposite corners at (0,0,0) and (1,1,1), which are black and
// white respectively.  The x-axis is the red gradient, the y-axis is the
// green gradient, and the z-axis is the blue gradient.  The cube's position
// and colors are fixed.
var NUM_FACES = 6;

var vertices = [
    [0, 0, 0], [0, 0, 1], [0, 1, 0], [0, 1, 1],
    [1, 0, 0], [1, 0, 1], [1, 1, 0], [1, 1, 1]
];

var faces = [
    [1, 5, 7, 3], [5, 4, 6, 7], [4, 0, 2, 6],
    [3, 7, 6, 2], [0, 1, 3, 2], [0, 4, 5, 1]
];

var vertexColors = [
    [0.0, 0.0, 0.0], [0.0, 0.0, 1.0], [0.0, 1.0, 0.0], [0.0, 1.0, 1.0],
    [1.0, 0.0, 0.0], [1.0, 0.0, 1.0], [1.0, 1.0, 0.0], [1.0, 1.0, 1.0]
];

var u = 0;

function Setup() {
    glInit();
    glEnable(GL.CULL_FACE);
    glCullFace(GL.BACK);

    reshape(GL.WIDTH, GL.HEIGHT);
}

function Loop() {
    glClear(GL.COLOR_BUFFER_BIT);
    glBegin(GL.QUADS);
    for (var i = 0; i < NUM_FACES; i++) {
        for (var j = 0; j < 4; j++) {
            glColor3(vertexColors[faces[i][j]]);
            glVertex3(vertices[faces[i][j]]);
        }
    }
    glEnd();
    glFlush();

    u += 0.01;
    glLoadIdentity();
    gluLookAt(8 * Math.cos(u), 7 * Math.cos(u) - 1, 4 * Math.cos(u / 3) + 2, .5, .5, .5, Math.cos(u), 1, 0);
}

function reshape(w, h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL.PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, w / h, 0.5, 40.0);
    glMatrixMode(GL.MODELVIEW);
}

function Input(e) {
}
