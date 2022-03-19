/*
OpenGL example code from
https://cs.lmu.edu/~ray/notes/openglexamples/

http://movievertigo.com/zyxit/

https://twitter.com/MovieVertigo/status/1483511755712126977

https://doersino.github.io/tixyz/

*/
LoadLibrary("ogl");

var TIXY_SIZE = 8;		// number of dots in x and y direction
var TIXY_DOT = 0.3;		// max radius of each dot
var TIXY_SPACING = 0.1;	// spacing between dots

var TIXY_DETAIL = 10;

var TXT_SIZE = 9;		// text height
var TXT_MAX = 85;		// max length of function

var xOffset = 0 - (((TIXY_SIZE * TIXY_DOT * 2) + (TIXY_SIZE * TIXY_SPACING)) / 2);	// x offset for the dots
var yOffset = xOffset;
var zOffset = xOffset;

var ticks = 0;

var black = [0.0, 0.0, 0.0, 1.0];
var grey = [0.8, 0.8, 0.8, 1.0];
var red = [1.0, 0, 0, 1.0];
var white = [1.0, 1.0, 1.0, 1.0];
var direction = [1.0, 1.0, 1.0, 0.0];

var currentFunction = "sin(t - hypot(x * 2 - 7.5, y * 2 - 7.5, z * 2 - 7.5))";  // current function with a start

var img;
var changedImg = true;

function Setup() {
    // copy all Math functions/constants to global context
    var mathmembers = [
        "E", "LN10", "LN2", "LOG2E", "LOG10E", "PI", "SQRT1_2", "SQRT2", "abs", "acos",
        "asin", "atan", "atan2", "ceil", "cos", "exp", "floor", "log", "max", "min",
        "pow", "random", "round", "sin", "sqrt", "tan", "hypot"
    ];

    for (var i = 0; i < mathmembers.length; i++) {
        var key = mathmembers[i];
        global[key] = Math[key];
    }

    glInit();

    glMaterial(GL.FRONT, GL.SPECULAR, white);
    glMaterial(GL.FRONT, GL.SHININESS, 30);

    glLight(GL.LIGHT0, GL.AMBIENT, black);
    glLight(GL.LIGHT0, GL.DIFFUSE, grey);
    glLight(GL.LIGHT0, GL.SPECULAR, white);
    glLight(GL.LIGHT0, GL.POSITION, direction);

    glEnable(GL.LIGHTING);                // so the renderer considers light
    glEnable(GL.LIGHT0);                  // turn LIGHT0 on
    glEnable(GL.DEPTH_TEST);              // so the renderer considers depth

    reshape(GL.WIDTH, GL.HEIGHT);

    // test image
    img = new Bitmap(Width, 20);
    SetRenderBitmap(img);
    ClearScreen(EGA.BLACK);
    Box(0, 0, img.width - 1, img.height - 1, EGA.RED);
    Line(0, 0, img.width - 1, img.height - 1, EGA.GREEN);
}

function Loop() {
    glClear(GL.COLOR_BUFFER_BIT | GL.DEPTH_BUFFER_BIT);
    glMatrixMode(GL.MODELVIEW);
    glPushMatrix();

    // Rotate the scene so we can see the tops of the shapes.
    glRotate(20 + ticks, 0, 1.0, 1.0);

    // build a valid JS function with export and evaluate it
    try {
        var content = "exports.f = function(t,i,x,y,z){return " + currentFunction + ";};"; // build JS
        var exports = { "f": null };						// build object to import into
        NamedFunction('exports', content, "f")(exports);	// evaluate JS
        exports.f(0, 0, 0, 0);								// do a testcall to check the function is valid
        f = exports.f;										// set as current function
    } catch (e) {
        f = null;
    }

    // if we have a vallid function, draw dots
    if (f) {
        var i = 0;
        var t = MsecTime() / 1000;
        for (var z = 0; z < TIXY_SIZE; z++) {
            for (var y = 0; y < TIXY_SIZE; y++) {
                for (var x = 0; x < TIXY_SIZE; x++) {
                    // get valud
                    var s = f(t, i, x, y, z);

                    // calculate pos and draw dot
                    var xPos = (x * TIXY_DOT * 2) + (x * TIXY_SPACING);
                    var yPos = (y * TIXY_DOT * 2) + (y * TIXY_SPACING);
                    var zPos = (z * TIXY_DOT * 2) + (z * TIXY_SPACING);
                    var radius = TIXY_DOT * s;
                    drawSphere(xOffset + xPos, yOffset + yPos, zOffset + zPos, radius);

                    i++; // increment i
                }
            }
        }
    }

    glPopMatrix();

    if (changedImg) {
        // update bitmap with function text
        ClearScreen(EGA.BLACK);
        TextXY(10, 0, "(t,i,x,y,z) => ", EGA.WHITE, NO_COLOR);
        TextXY(10, TXT_SIZE, currentFunction, EGA.WHITE, NO_COLOR);
        changedImg = false;
    }

    glRasterPos2(-6, -4.5);
    glDrawPixels(img);

    glFlush();

    ticks++;
}

function drawSphere(x, y, z, s) {
    // get color and invert if negative
    if (s < 0) {
        glMaterial(GL.FRONT, GL.AMBIENT_AND_DIFFUSE, red);
        s *= -1;
    } else {
        glMaterial(GL.FRONT, GL.AMBIENT_AND_DIFFUSE, white);
    }
    if (s > 1) {
        s = 1;
    }

    glPushMatrix();
    glTranslate(x, y, z);
    glutSolidSphere(s, TIXY_DETAIL, TIXY_DETAIL);
    glPopMatrix();
}

function reshape(w, h) {
    var near_far = 10;
    var ratio = 4.5;

    glViewport(0, 0, w, h);
    glMatrixMode(GL.PROJECTION);
    var aspect = w / h;
    glLoadIdentity();
    if (w <= h) {
        // width is smaller, so stretch out the height
        glOrtho(-ratio, ratio, -ratio / aspect, ratio / aspect, -near_far, near_far);
    } else {
        // height is smaller, so stretch out the width
        glOrtho(-ratio * aspect, ratio * aspect, -ratio, ratio, -near_far, near_far);
    }
}

/**
 * handle input
 * @param {*} e DOjS event object.
 */
function Input(e) {
    if (e.key != -1) {
        var key = e.key & 0xFF;
        var keyCode = e.key;
        if (keyCode >> 8 == KEY.Code.KEY_BACKSPACE) {
            // delete last character
            currentFunction = currentFunction.slice(0, currentFunction.length - 1);
        } else {
            if (key >= CharCode(" ") && (currentFunction.length < TXT_MAX)) {
                // add character if not max length and charcode at least a SPACE
                currentFunction += String.fromCharCode(key);
            }
        }
        changedImg = true;
    }
}
