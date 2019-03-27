/*
	This file was derived from the p5.js source code at
	https://github.com/processing/p5.js

	Copyright (c) the p5.js contributors and Andre Seidelt <superilu@yahoo.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * This module provides compatibility with p5js. Use Include('p5'); on the first line of your script to activate.
 * 
 * @module p5compat
 */

Include('jsboot/p5const.js');
Include('jsboot/p5util.js');
Include('jsboot/p5math.js');
Include('jsboot/p5vect.js');
Include('jsboot/p5color.js');
Include('jsboot/p5shape.js');
Include('jsboot/p5env.js');
Include('jsboot/p5input.js');

exports._loop = true;

exports.Setup = function () {
	windowWidth = displayWidth = width = SizeX();
	windowHeight = displayHeight = height = SizeY();
	frameCount = 0;
	_hasMouseInteracted = false;
	mouseX = pmouseX = winMouseX = pwinMouseX = MaxX();
	mouseY = pmouseY = winMouseY = pwinMouseY = MaxY();
	MouseWarp(mouseX, mouseY);
	frameRate(60);
	setup();
};

exports.Loop = function () {
	if (_loop) {
		redraw();
	}
	if (keyIsPressed) {
		keyIsPressed = false;
		if (typeof global['keyReleased'] != 'undefined') {
			keyReleased();
		}
	}
};

exports.Input = function (e) {
	// update mouse coordinates
	if (e.flags & MOUSE.Flags.MOTION) {
		pwinMouseX = pmouseX = mouseX;
		pwinMouseY = pmouseY = mouseY;

		winMouseX = mouseX = e.x;
		winMouseY = mouseY = e.y;

		_hasMouseInteracted = true;

		if (typeof global['mouseMoved'] != 'undefined') {
			mouseMoved(e);
		}
	}

	if (e.flags & MOUSE.Flags.BUTTON_DOWN) {
		if (typeof global['mousePressed'] != 'undefined') {
			mousePressed(e);
		}
		mouseIsPressed = true;
		if (e.button & MOUSE.Buttons.LEFT) {
			mouseButton = LEFT;
		}
		if (e.button & MOUSE.Buttons.MIDDLE) {
			mouseButton = MIDDLE;
		}
		if (e.button & MOUSE.Buttons.RIGHT) {
			mouseButton = RIGHT;
		}
	}
	if (e.flags & MOUSE.Flags.BUTTON_UP) {
		if (mouseIsPressed && typeof global['mouseReleased'] != 'undefined') {
			mouseReleased(e);
		}
		mouseIsPressed = false;
		mouseButton = 0;
		if (mouseIsPressed && typeof global['mouseClicked'] != 'undefined') {
			mouseClicked(e);
		}
	}

	// this does not work like with p5 as we don't get a key release
	if (e.flags & MOUSE.Flags.KEYPRESS) {
		if (typeof global['keyPressed'] != 'undefined') {
			keyPressed(e);
		}
		if (typeof global['keyTyped'] != 'undefined') {
			keyTyped(e);
		}
		key = keyCode = e.key;
		keyIsPressed = true;
	}
};

/**
 * ignored
 */
exports.size = function () { };

/**
 * ignored
 */
exports.createCanvas = function () { };

/**
 * ignored
 */
exports.smooth = function () { };

/**
 * ignored
 */
exports.noSmooth = function () { };

/**
 * ignored
 */
exports.settings = function () { };

/**
 * ignored
 */
exports.colorMode = function () { };

/**
 * ignored
 */
exports.imageMode = function () { };

/**
 * ignored
 */
exports.strokeWeight = function () { };

/**
 * ignored
 */
exports.tint = function () { };

/**
 * ignored
 */
exports.noTint = function () { };

/**
 * exit the script after the current Loop().
 */
exports.exit = function () {
	Stop();
};

/**
 * By default, p5.js loops through draw() continuously, executing the code
 * within it. However, the draw() loop may be stopped by calling noLoop().
 * In that case, the draw() loop can be resumed with loop().
 *
 * Avoid calling loop() from inside setup().
 *
 * @method loop
 * @example
 * let x = 0;
 * function setup() {
 *   createCanvas(100, 100);
 *   noLoop();
 * }
 *
 * function draw() {
 *   background(204);
 *   x = x + 0.1;
 *   if (x > width) {
 *     x = 0;
 *   }
 *   line(x, 0, x, height);
 * }
 *
 * function mousePressed() {
 *   loop();
 * }
 *
 * function mouseReleased() {
 *   noLoop();
 * }
 */
exports.loop = function () {
	_loop = true;
};

/**
 * Stops p5.js from continuously executing the code within draw().
 * If loop() is called, the code in draw() begins to run continuously again.
 * If using noLoop() in setup(), it should be the last line inside the block.
 * <br><br>
 * When noLoop() is used, it's not possible to manipulate or access the
 * screen inside event handling functions such as mousePressed() or
 * keyPressed(). Instead, use those functions to call redraw() or loop(),
 * which will run draw(), which can update the screen properly. This means
 * that when noLoop() has been called, no drawing can happen, and functions
 * like saveFrame() or loadPixels() may not be used.
 * <br><br>
 * Note that if the sketch is resized, redraw() will be called to update
 * the sketch, even after noLoop() has been specified. Otherwise, the sketch
 * would enter an odd state until loop() was called.
 *
 * @method noLoop
 * @example
 * function setup() {
 *   createCanvas(100, 100);
 *   background(200);
 *   noLoop();
 * }
 * 
 * function draw() {
 *   line(10, 10, 90, 90);
 * }
 *
 * let x = 0;
 * function setup() {
 *   createCanvas(100, 100);
 * }
 *
 * function draw() {
 *   background(204);
 *   x = x + 0.1;
 *   if (x > width) {
 *     x = 0;
 *   }
 *   line(x, 0, x, height);
 * }
 *
 * function mousePressed() {
 *   noLoop();
 * }
 *
 * function mouseReleased() {
 *   loop();
 * }
 */
exports.noLoop = function () {
	_loop = false;
};

/**
 *
 * Executes the code within draw() one time. This functions allows the
 * program to update the display window only when necessary, for example
 * when an event registered by mousePressed() or keyPressed() occurs.
 * <br><br>
 * In structuring a program, it only makes sense to call redraw() within
 * events such as mousePressed(). This is because redraw() does not run
 * draw() immediately (it only sets a flag that indicates an update is
 * needed).
 * <br><br>
 * The redraw() function does not work properly when called inside draw().
 * To enable/disable animations, use loop() and noLoop().
 * <br><br>
 * In addition you can set the number of redraws per method call. Just
 * add an integer as single parameter for the number of redraws.
 *
 * @method redraw
 * @param  {Integer} [n] Redraw for n-times. The default value is 1.
 * @example
 * let x = 0;
 *
 * function setup() {
 *   createCanvas(100, 100);
 *   noLoop();
 * }
 *
 * function draw() {
 *   background(204);
 *   line(x, 0, x, height);
 * }
 *
 * function mousePressed() {
 *   x += 1;
 *   redraw();
 * }
 *
 * let x = 0;
 *
 * function setup() {
 *   createCanvas(100, 100);
 *   noLoop();
 * }
 *
 * function draw() {
 *   background(204);
 *   x += 1;
 *   line(x, 0, x, height);
 * }
 *
 * function mousePressed() {
 *   redraw(5);
 * }
 */
exports.redraw = function () {
	draw();
	frameCount++;
};

/**
 * Returns the pixel density of the current display the sketch is running on (always 1 for DOjS).
 *
 * @method displayDensity
 * @returns {Number} current pixel density of the display
 * @example
 * function setup() {
 *   let density = displayDensity();
 *   pixelDensity(density);
 *   createCanvas(100, 100);
 *   background(200);
 *   ellipse(width / 2, height / 2, 50, 50);
 * }
 */
exports.displayDensity = function () {
	return 1;
};


exports.cursor = function () {
	MouseShowCursor(true);
};

/**
 * Hides the cursor from view.
 *
 * @method noCursor
 * @example
 * function setup() {
 *   noCursor();
 * }
 *
 * function draw() {
 *   background(200);
 *   ellipse(mouseX, mouseY, 10, 10);
 * }
 */
exports.noCursor = function () {
	MouseShowCursor(false);
};

exports.delay = function (ms) {
	Sleep(ms);
};

exports.printArray = function (what) {
	for (var i = 0; i < what.length; i++) {
		Println("[" + i + "] " + what[i]);
	}
};

exports.saveStrings = function (fname, data) {
	var f = new File();
	data.forEach(function (d) {
		f.WriteLine(d);
	});
	f.Close();
};

exports.saveBytes = function (fname, data) {
	var f = new File();
	data.forEach(function (d) {
		f.WriteByte(d);
	});
	f.Close();
};

exports.loadStrings = function (fname) {
	try {
		var ret = [];
		var f = new File(fname);
		var l = f.ReadLine();
		while (l != null) {
			ret.push(l);
			l = f.ReadLine();
		}
		f.Close();
		return ret;
	} catch (e) {
		Println(e);
		return null;
	}
};

exports.loadBytes = function (fname) {
	try {
		var ret = [];
		var f = new File(fname);
		var ch = g.ReadByte();
		while (ch != null) {
			ret.push(ch);
			ch = f.ReadByte();
		}
		f.Close();
		return ret;
	} catch (e) {
		Println(e);
		return null;
	}
};

// exports. = function () {
// };

