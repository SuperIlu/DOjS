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

Include('p5const');
Include('p5color');
Include('p5env');
Include('p5input');
Include('p5math');
Include('p5shape');
Include('p5typo');
Include('p5util');
Include('p5vect');
Include('p5trans');

exports.__VERSION__ = 2;

exports._loop = true;
exports._lastButtons = 0;

exports._env = [];
exports._currentEnv = {
	_fill: EGA.WHITE,
	_stroke: EGA.BLACK,
	_colorMode: RGB,
	_colorMaxes: {
		rgb: [255, 255, 255, 255],
		hsb: [360, 100, 100, 1],
		hsl: [360, 100, 100, 1]
	},
	_txtAlignX: LEFT,
	_txtAlignY: TOP,
	_font: new Font(),
	_rectMode: CORNER,
	_ellipseMode: CENTER,
	_imageMode: CORNER,
	_strokeWeight: 1,
	_matrix: null
};

// TODO: implement matrix preservation for setup() and draw()

/**
 * deep copy the current environment.
 */
exports._cloneEnv = function () {
	if (_currentEnv._matrix) {
		// deep copy matrix
		var matrix = [
			_currentEnv._matrix[0].slice(0),
			_currentEnv._matrix[1].slice(0),
			_currentEnv._matrix[2].slice(0)
		]
	} else {
		var matrix = null;
	}

	return {
		_fill: _currentEnv._fill,
		_stroke: _currentEnv._stroke,
		_colorMode: _currentEnv._colorMode,
		_colorMaxes: {
			// deep copy color settings
			rgb: _currentEnv._colorMaxes.rgb.slice(0),
			hsb: _currentEnv._colorMaxes.hsb.slice(0),
			hsl: _currentEnv._colorMaxes.hsl.slice(0)
		},
		_txtAlignX: _currentEnv._txtAlignX,
		_txtAlignY: _currentEnv._txtAlignY,
		_font: _currentEnv._font,
		_rectMode: _currentEnv._rectMode,
		_ellipseMode: _currentEnv._ellipseMode,
		_imageMode: _currentEnv._imageMode,
		_strokeWeight: _currentEnv._strokeWeight,
		_matrix: matrix
	};
}

/*
 * not documented here.
 */
exports.Setup = function () {
	windowWidth = displayWidth = width = SizeX();
	windowHeight = displayHeight = height = SizeY();
	frameCount = 0;
	_hasMouseInteracted = false;
	mouseX = pmouseX = winMouseX = pwinMouseX = SizeX();
	mouseY = pmouseY = winMouseY = pwinMouseY = SizeY();
	MouseWarp(mouseX, mouseY);
	frameRate(60);
	setup();
};

/*
 * not documented here.
 */
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

/*
 * not documented here.
 */
exports.Input = function (e) {
	// update mouse coordinates
	pwinMouseX = pmouseX = mouseX;
	pwinMouseY = pmouseY = mouseY;

	winMouseX = mouseX = e.x;
	winMouseY = mouseY = e.y;

	_hasMouseInteracted = true;

	if (typeof global['mouseMoved'] != 'undefined') {
		mouseMoved(e);
	}
	if (mouseIsPressed) {
		if (typeof global['mouseDragged'] != 'undefined') {
			mouseDragged(e);
		}
	}

	if (e.buttons > _lastButtons) {
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
	if (e.buttons < _lastButtons) {
		mouseButton = 0;
		if (mouseIsPressed && typeof global['mouseReleased'] != 'undefined') {
			mouseReleased(e);
		}
		if (mouseIsPressed && typeof global['mouseClicked'] != 'undefined') {
			mouseClicked(e);
		}
		mouseIsPressed = false;
	}
	_lastButtons = e.buttons;

	// this does not work like with p5 as we don't get a key release
	if (e.key != -1) {
		key = e.key & 0xFF;	// TODO:
		keyCode = e.key;	// TODO:
		if (typeof global['keyPressed'] != 'undefined') {
			keyPressed(e);
		}
		if (typeof global['keyTyped'] != 'undefined') {
			keyTyped(e);
		}
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
	resetMatrix();	// TODO: fix!
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

/**
 *  Writes an array of Strings to a text file, one line per String.
 *  The file saving process and location of the saved file will
 *  vary between web browsers.
 *
 *  @method saveStrings
 *  @param  {String[]} list   string array to be written
 *  @param  {String} filename filename for output
 *  @param  {String} [extension] the filename's extension
 *  @example
 * let words = 'apple bear cat dog';
 *
 * // .split() outputs an Array
 * let list = split(words, ' ');
 *
 * function setup() {
 *   createCanvas(100, 100);
 *   background(200);
 *   text('click here to save', 10, 10, 70, 80);
 * }
 *
 * function mousePressed() {
 *   if (mouseX > 0 && mouseX < width && mouseY > 0 && mouseY < height) {
 *     saveStrings(list, 'nouns.txt');
 *   }
 * }
 *
 * // Saves the following to a file called 'nouns.txt':
 * //
 * // apple
 * // bear
 * // cat
 * // dog
 */
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

/**
 * Reads the contents of a file and creates a String array of its individual
 * lines. If the name of the file is used as the parameter, as in the above
 * example, the file must be located in the sketch directory/folder.
 * <br><br>
 * Alternatively, the file maybe be loaded from anywhere on the local
 * computer using an absolute path (something that starts with / on Unix and
 * Linux, or a drive letter on Windows), or the filename parameter can be a
 * URL for a file found on a network.
 * <br><br>
 * This method is asynchronous, meaning it may not finish before the next
 * line in your sketch is executed.
 *
 * This method is suitable for fetching files up to size of 64MB.
 * @method loadStrings
 * @param  {String}   filename   name of the file or url to load
 * @return {String[]}            Array of Strings
 * @example
 *
 * let result;
 * function preload() {
 *   result = loadStrings('assets/test.txt');
 * }

 * function setup() {
 *   background(200);
 *   let ind = floor(random(result.length));
 *   text(result[ind], 10, 10, 80, 80);
 * }
 *
 * function setup() {
 *   loadStrings('assets/test.txt', pickString);
 * }
 *
 * function pickString(result) {
 *   background(200);
 *   let ind = floor(random(result.length));
 *   text(result[ind], 10, 10, 80, 80);
 * }
 */
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

/**
 * This method is suitable for fetching files up to size of 64MB.
 * @method loadBytes
 * @param {string}   file            name of the file or URL to load
 * @returns {number[]} an object whose 'bytes' property will be the loaded buffer
 *
 * @example
 * let data;
 *
 * function preload() {
 *   data = loadBytes('assets/mammals.xml');
 * }
 *
 * function setup() {
 *   for (let i = 0; i < 5; i++) {
 *     console.log(data.bytes[i].toString(16));
 *   }
 * }
 */
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


/**
 * The push() function saves the current drawing style settings and
 * transformations, while pop() restores these settings. Note that these
 * functions are always used together. They allow you to change the style
 * and transformation settings and later return to what you had. When a new
 * state is started with push(), it builds on the current style and transform
 * information. The push() and pop() functions can be embedded to provide
 * more control. (See the second example for a demonstration.)
 * <br><br>
 * push() stores information related to the current transformation state
 * and style settings controlled by the following functions: fill(),
 * stroke(), tint(), strokeWeight(), strokeCap(), strokeJoin(),
 * imageMode(), rectMode(), ellipseMode(), colorMode(), textAlign(),
 * textFont(), textSize(), textLeading().
 *
 * @method push
 * @example
 * ellipse(0, 50, 33, 33); // Left circle
 *
 * push(); // Start a new drawing state
 * strokeWeight(10);
 * fill(204, 153, 0);
 * translate(50, 0);
 * ellipse(0, 50, 33, 33); // Middle circle
 * pop(); // Restore original state
 *
 * ellipse(100, 50, 33, 33); // Right circle
 * </code>
 * </div>
 * <div>
 * <code>
 * ellipse(0, 50, 33, 33); // Left circle
 *
 * push(); // Start a new drawing state
 * strokeWeight(10);
 * fill(204, 153, 0);
 * ellipse(33, 50, 33, 33); // Left-middle circle
 *
 * push(); // Start another new drawing state
 * stroke(0, 102, 153);
 * ellipse(66, 50, 33, 33); // Right-middle circle
 * pop(); // Restore previous state
 *
 * pop(); // Restore original state
 *
 * ellipse(100, 50, 33, 33); // Right circle
 */
exports.push = function () {
	_env.push(_cloneEnv());
};

/**
 * The push() function saves the current drawing style settings and
 * transformations, while pop() restores these settings. Note that these
 * functions are always used together. They allow you to change the style
 * and transformation settings and later return to what you had. When a new
 * state is started with push(), it builds on the current style and transform
 * information. The push() and pop() functions can be embedded to provide
 * more control. (See the second example for a demonstration.)
 * <br><br>
 * push() stores information related to the current transformation state
 * and style settings controlled by the following functions: fill(),
 * stroke(), tint(), strokeWeight(), strokeCap(), strokeJoin(),
 * imageMode(), rectMode(), ellipseMode(), colorMode(), textAlign(),
 * textFont(), textSize(), textLeading().
 *
 * @method pop
 * @example
 * ellipse(0, 50, 33, 33); // Left circle
 *
 * push(); // Start a new drawing state
 * translate(50, 0);
 * strokeWeight(10);
 * fill(204, 153, 0);
 * ellipse(0, 50, 33, 33); // Middle circle
 * pop(); // Restore original state
 *
 * ellipse(100, 50, 33, 33); // Right circle
 * </code>
 * </div>
 * <div>
 * <code>
 * ellipse(0, 50, 33, 33); // Left circle
 *
 * push(); // Start a new drawing state
 * strokeWeight(10);
 * fill(204, 153, 0);
 * ellipse(33, 50, 33, 33); // Left-middle circle
 *
 * push(); // Start another new drawing state
 * stroke(0, 102, 153);
 * ellipse(66, 50, 33, 33); // Right-middle circle
 * pop(); // Restore previous state
 *
 * pop(); // Restore original state
 *
 * ellipse(100, 50, 33, 33); // Right circle
 */
exports.pop = function () {
	if (_env.length > 0) {
		_currentEnv = _env.pop();
	} else {
		console.warn('pop() was called without matching push()');
	}
};

/**
 * Blends the pixels in the display window according to the defined mode.
 * There is a choice of the following modes to blend the source pixels (A)
 * with the ones of pixels already in the display window (B):
 * <ul>
 * <li><code>BLEND</code> - linear interpolation of colours: C =
 * A*factor + B. <b>This is the default blending mode.</b></li>
 * <li><code>ADD</code> - sum of A and B</li>
 * <li><code>DARKEST</code> - only the darkest colour succeeds: C =
 * min(A*factor, B).</li>
 * <li><code>LIGHTEST</code> - only the lightest colour succeeds: C =
 * max(A*factor, B).</li>
 * <li><code>DIFFERENCE</code> - subtract colors from underlying image.</li>
 * <li><code>EXCLUSION</code> - similar to <code>DIFFERENCE</code>, but less
 * extreme.</li>
 * <li><code>MULTIPLY</code> - multiply the colors, result will always be
 * darker.</li>
 * <li><code>SCREEN</code> - opposite multiply, uses inverse values of the
 * colors.</li>
 * <li><code>REPLACE</code> - the pixels entirely replace the others and
 * don't utilize alpha (transparency) values.</li>
 * <li><code>REMOVE</code> - removes pixels from B with the alpha strength of A.</li>
 * <li><code>OVERLAY</code> - mix of <code>MULTIPLY</code> and <code>SCREEN
 * </code>. Multiplies dark values, and screens light values. <em>(2D)</em></li>
 * <li><code>HARD_LIGHT</code> - <code>SCREEN</code> when greater than 50%
 * gray, <code>MULTIPLY</code> when lower. <em>(2D)</em></li>
 * <li><code>SOFT_LIGHT</code> - mix of <code>DARKEST</code> and
 * <code>LIGHTEST</code>. Works like <code>OVERLAY</code>, but not as harsh. <em>(2D)</em>
 * </li>
 * <li><code>DODGE</code> - lightens light tones and increases contrast,
 * ignores darks. <em>(2D)</em></li>
 * <li><code>BURN</code> - darker areas are applied, increasing contrast,
 * ignores lights. <em>(2D)</em></li>
 * <li><code>SUBTRACT</code> - remainder of A and B <em>(3D)</em></li>
 * </ul>
 *
 * @method blendMode
 * @param  {Constant} mode blend mode to set for canvas.
 *                either BLEND, DARKEST, LIGHTEST, DIFFERENCE, MULTIPLY,
 *                EXCLUSION, SCREEN, REPLACE, OVERLAY, HARD_LIGHT,
 *                SOFT_LIGHT, DODGE, BURN, ADD, REMOVE or SUBTRACT
 * @example
 * blendMode(LIGHTEST);
 * strokeWeight(30);
 * stroke(80, 150, 255);
 * line(25, 25, 75, 75);
 * stroke(255, 50, 50);
 * line(75, 25, 25, 75);
 *
 * blendMode(MULTIPLY);
 * strokeWeight(30);
 * stroke(80, 150, 255);
 * line(25, 25, 75, 75);
 * stroke(255, 50, 50);
 * line(75, 25, 25, 75);
 */
exports.blendMode = function (mode) {
	var bm = BLEND.ALPHA;
	if (mode === ADD) {
		bm = BLEND.ADD;
	} else if (mode === DARKEST) {
		bm = BLEND.DARKEST;
	} else if (mode === LIGHTEST) {
		bm = BLEND.LIGHTEST;
	} else if (mode === DIFFERENCE) {
		bm = BLEND.DIFFERENCE;
	} else if (mode === EXCLUSION) {
		bm = BLEND.EXCLUSION;
	} else if (mode === MULTIPLY) {
		bm = BLEND.MULTIPLY;
	} else if (mode === SCREEN) {
		bm = BLEND.SCREEN;
	} else if (mode === REPLACE) {
		bm = BLEND.REPLACE;
	} else if (mode === OVERLAY) {
		bm = BLEND.OVERLAY;
	} else if (mode === HARD_LIGHT) {
		bm = BLEND.HARD_LIGHT;
	} else if (mode === DODGE) {
		bm = BLEND.DODGE;
	} else if (mode === BURN) {
		bm = BLEND.BURN;
	} else if (mode === SUBTRACT) {
		bm = BLEND.SUBTRACT;
	}
	//	} else if (mode === SOFT_LIGHT) {
	//  } else if (mode === REMOVE) {
	TransparencyEnabled(bm);
};
