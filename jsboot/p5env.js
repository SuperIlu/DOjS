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
* @module p5compat
*/

/**
 * The print() function writes to the logfile.
 * This function is often helpful for looking at the data a program is
 * producing. This function creates a new line of text for each call to
 * the function. Individual elements can be
 * separated with quotes ("") and joined with the addition operator (+).
 *
 * @method print
 * @param {Any} contents any combination of Number, String, Object, Boolean,
 *                       Array to print
 * @example
 * let x = 10;
 * print('The value of x is ' + x);
 * // prints "The value of x is 10"
 */
exports.print = function (s) {
	Print(s);
};

/**
 * The println() function writes to the logfile.
 * This function is often helpful for looking at the data a program is
 * producing. This function creates a new line of text for each call to
 * the function. Individual elements can be
 * separated with quotes ("") and joined with the addition operator (+).
 *
 * @method print
 * @param {Any} contents any combination of Number, String, Object, Boolean,
 *                       Array to print
 * @example
 * let x = 10;
 * print('The value of x is ' + x);
 * // prints "The value of x is 10"
 */
exports.println = function (s) {
	Println(s);
};

/**
 * The system variable frameCount contains the number of frames that have
 * been displayed since the program started. Inside setup() the value is 0,
 * after the first iteration of draw it is 1, etc.
 *
 * @property {Integer} frameCount
 * @readOnly
 * @example
 * function setup() {
 *   frameRate(30);
 *   textSize(30);
 *   textAlign(CENTER);
 * }
 *
 * function draw() {
 *   background(200);
 *   text(frameCount, width / 2, height / 2);
 * }
 */
exports.frameCount = 0;

/**
 * Confirms if the window a p5.js program is in is "focused," meaning that
 * the sketch will accept mouse or keyboard input. This variable is
 * "true" if the window is focused and "false" if not.
 *
 * @property {Boolean} focused
 * @readOnly
 * @example
 * // To demonstrate, put two windows side by side.
 * // Click on the window that the p5 sketch isn't in!
 * function draw() {
 *   background(200);
 *   noStroke();
 *   fill(0, 200, 0);
 *   ellipse(25, 25, 50, 50);
 *
 *   if (!focused) {
 *     // or "if (focused === false)"
 *     stroke(200, 0, 0);
 *     line(0, 0, 100, 100);
 *     line(100, 0, 0, 100);
 *   }
 * }
 */
exports.focused = true;

/**
 * Specifies the number of frames to be displayed every second. For example,
 * the function call frameRate(30) will attempt to refresh 30 times a second.
 * If the processor is not fast enough to maintain the specified rate, the
 * frame rate will not be achieved. Setting the frame rate within setup() is
 * recommended. The default frame rate is based on the frame rate of the display
 * (here also called "refresh rate"), which is set to 60 frames per second on most
 * computers. A frame rate of 24 frames per second (usual for movies) or above
 * will be enough for smooth animations
 * This is the same as setFrameRate(val).
 * <br><br>
 * Calling frameRate() with no arguments returns the current framerate. The
 * draw function must run at least once before it will return a value. This
 * is the same as getFrameRate().
 * <br><br>
 * Calling frameRate() with arguments that are not of the type numbers
 * or are non positive also returns current framerate.
 *
 * @method frameRate
 * @param  {Number} fps number of frames to be displayed every second
 * @example
 *
 * let rectX = 0;
 * let fr = 30; //starting FPS
 * let clr;
 *
 * function setup() {
 *   background(200);
 *   frameRate(fr); // Attempt to refresh at starting FPS
 *   clr = color(255, 0, 0);
 * }
 *
 * function draw() {
 *   background(200);
 *   rectX = rectX += 1; // Move Rectangle
 *
 *   if (rectX >= width) {
 *     // If you go off screen.
 *     if (fr === 30) {
 *       clr = color(0, 0, 255);
 *       fr = 10;
 *       frameRate(fr); // make frameRate 10 FPS
 *     } else {
 *       clr = color(255, 0, 0);
 *       fr = 30;
 *       frameRate(fr); // make frameRate 30 FPS
 *     }
 *     rectX = 0;
 *   }
 *   fill(clr);
 *   rect(rectX, 40, 20, 20);
 * }
 */
exports.frameRate = function (r) {
	SetFramerate(r);
};

/**
 * Returns the current framerate.
 *
 * @method getFrameRate
 * @return {Number} current frameRate
 */
exports.getFrameRate = function () {
	return GetFramerate();
};

/**
 * System variable that stores the width of the screen display according to The
 * default pixelDensity. This is used to run a
 * full-screen program on any display size. To return actual screen size,
 * multiply this by pixelDensity.
 *
 * @property {Number} displayWidth
 * @readOnly
 * @example
 * createCanvas(displayWidth, displayHeight);
 */
exports.displayWidth = 0;

/**
 * System variable that stores the height of the screen display according to The
 * default pixelDensity. This is used to run a
 * full-screen program on any display size. To return actual screen size,
 * multiply this by pixelDensity.
 *
 * @property {Number} displayHeight
 * @readOnly
 * @example
 * createCanvas(displayWidth, displayHeight);
 */
exports.displayHeight = 0;

/**
 * System variable that stores the width of the inner window.
 *
 * @property {Number} windowWidth
 * @readOnly
 * @example
 * createCanvas(windowWidth, windowHeight);
 */
exports.windowWidth = 0;

/**
 * System variable that stores the height of the inner window.
 *
 * @property {Number} windowHeight
 * @readOnly
 * @example
 * createCanvas(windowWidth, windowHeight);
 */
exports.windowHeight = 0;

/**
 * System variable that stores the width of the drawing canvas.
 *
 * @property {Number} width
 * @readOnly
 */
exports.width = 0;

/**
 * System variable that stores the height of the drawing canvas.
 *
 * @property {Number} height
 * @readOnly
 */
exports.height = 0;

/**
 * If argument is given, sets the sketch to fullscreen or not based on the
 * value of the argument. If no argument is given, returns the current
 * fullscreen state. Note that due to browser restrictions this can only
 * be called on user input, for example, on mouse press like the example
 * below.
 *
 * @method fullscreen
 * @param  {Boolean} [val] whether the sketch should be in fullscreen mode
 * or not
 * @return {Boolean} current fullscreen state
 * @example
 * // Clicking in the box toggles fullscreen on and off.
 * function setup() {
 *   background(200);
 * }
 * function mousePressed() {
 *   if (mouseX > 0 && mouseX < 100 && mouseY > 0 && mouseY < 100) {
 *     let fs = fullscreen();
 *     fullscreen(!fs);
 *   }
 * }
 */
exports.fullScreen = function () {
	return true;
};

/**
 * Sets the pixel scaling for high pixel density displays. By default
 * pixel density is set to match display density, call pixelDensity(1)
 * to turn this off. Calling pixelDensity() with no arguments returns
 * the current pixel density of the sketch.
 *
 * @method pixelDensity
 * @param  {Number} val whether or how much the sketch should scale
 * @example
 * function setup() {
 *   pixelDensity(1);
 *   createCanvas(100, 100);
 *   background(200);
 *   ellipse(width / 2, height / 2, 50, 50);
 * }
 * 
 * function setup() {
 *   pixelDensity(3.0);
 *   createCanvas(100, 100);
 *   background(200);
 *   ellipse(width / 2, height / 2, 50, 50);
 * }
 */
exports.pixelDensity = function () {
	return 1;
};

/**
 * Gets the current URL.
 * @method getURL
 * @return {String} url
 * @example
 * let url;
 * let x = 100;
 *
 * function setup() {
 *   fill(0);
 *   noStroke();
 *   url = getURL();
 * }
 *
 * function draw() {
 *   background(200);
 *   text(url, x, height / 2);
 *   x--;
 * }
 */
exports.getURL = function () {
	return "";
};

/**
 * Gets the current URL path as an array.
 * @method getURLPath
 * @return {String[]} path components
 * @example
 * function setup() {
 *   let urlPath = getURLPath();
 *   for (let i = 0; i < urlPath.length; i++) {
 *     text(urlPath[i], 10, i * 20 + 20);
 *   }
 * }
 */
exports.getURLPath = function () {
	return "";
};

/**
 * Gets the current URL params as an Object.
 * @method getURLParams
 * @return {Object} URL params
 * @example
 * // Example: http://p5js.org?year=2014&month=May&day=15
 *
 * function setup() {
 *   let params = getURLParams();
 *   text(params.day, 10, 20);
 *   text(params.month, 10, 40);
 *   text(params.year, 10, 60);
 * }
 */
exports.getURLParams = function () {
	return "";
};
