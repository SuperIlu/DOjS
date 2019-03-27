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

/**********************************************************************************************************************
 * mouse
 */
/*
 * This is a flag which is false until the first time
 * we receive a mouse event. The pmouseX and pmouseY
 * values will match the mouseX and mouseY values until
 * this interaction takes place.
 */
exports._hasMouseInteracted = false;

/**
 * The system variable mouseX always contains the current horizontal
 * position of the mouse, relative to (0, 0) of the canvas. If touch is
 * used instead of mouse input, mouseX will hold the x value of the most
 * recent touch point.
 *
 * @property {Number} mouseX
 * @readOnly
 *
 * @example
 * // Move the mouse across the canvas
 * function draw() {
 *   background(244, 248, 252);
 *   line(mouseX, 0, mouseX, 100);
 * }
 */
exports.mouseX = 0;

/**
 * The system variable mouseY always contains the current vertical position
 * of the mouse, relative to (0, 0) of the canvas. If touch is
 * used instead of mouse input, mouseY will hold the y value of the most
 * recent touch point.
 *
 * @property {Number} mouseY
 * @readOnly
 *
 * @example
 * // Move the mouse across the canvas
 * function draw() {
 *   background(244, 248, 252);
 *   line(0, mouseY, 100, mouseY);
 * }
 */
exports.mouseY = 0;

/**
 * The system variable pmouseX always contains the horizontal position of
 * the mouse or finger in the frame previous to the current frame, relative to
 * (0, 0) of the canvas. Note: pmouseX will be reset to the current mouseX
 * value at the start of each touch event.
 *
 * @property {Number} pmouseX
 * @readOnly
 *
 * @example
 * // Move the mouse across the canvas to leave a trail
 * function setup() {
 *   //slow down the frameRate to make it more visible
 *   frameRate(10);
 * }
 *
 * function draw() {
 *   background(244, 248, 252);
 *   line(mouseX, mouseY, pmouseX, pmouseY);
 *   print(pmouseX + ' -> ' + mouseX);
 * }
 */
exports.pmouseX = 0;

/**
 * The system variable pmouseY always contains the vertical position of the
 * mouse or finger in the frame previous to the current frame, relative to
 * (0, 0) of the canvas. Note: pmouseY will be reset to the current mouseY
 * value at the start of each touch event.
 *
 * @property {Number} pmouseY
 * @readOnly
 *
 * @example
 * function draw() {
 *   background(237, 34, 93);
 *   fill(0);
 *   //draw a square only if the mouse is not moving
 *   if (mouseY === pmouseY && mouseX === pmouseX) {
 *     rect(20, 20, 60, 60);
 *   }
 *
 *   print(pmouseY + ' -> ' + mouseY);
 * }
 */
exports.pmouseY = 0;

/**
 * The system variable winMouseX always contains the current horizontal
 * position of the mouse, relative to (0, 0) of the window.
 *
 * @property {Number} winMouseX
 * @readOnly
 *
 * @example
 * let myCanvas;
 *
 * function setup() {
 *   //use a variable to store a pointer to the canvas
 *   myCanvas = createCanvas(100, 100);
 *   const body = document.getElementsByTagName('body')[0];
 *   myCanvas.parent(body);
 * }
 *
 * function draw() {
 *   background(237, 34, 93);
 *   fill(0);
 *
 *   //move the canvas to the horizontal mouse position
 *   //relative to the window
 *   myCanvas.position(winMouseX + 1, windowHeight / 2);
 *
 *   //the y of the square is relative to the canvas
 *   rect(20, mouseY, 60, 60);
 * }
 */
exports.winMouseX = 0;

/**
 * The system variable winMouseY always contains the current vertical
 * position of the mouse, relative to (0, 0) of the window.
 *
 * @property {Number} winMouseY
 * @readOnly
 *
 * @example
 * let myCanvas;
 *
 * function setup() {
 *   //use a variable to store a pointer to the canvas
 *   myCanvas = createCanvas(100, 100);
 *   const body = document.getElementsByTagName('body')[0];
 *   myCanvas.parent(body);
 * }
 *
 * function draw() {
 *   background(237, 34, 93);
 *   fill(0);
 *
 *   //move the canvas to the vertical mouse position
 *   //relative to the window
 *   myCanvas.position(windowWidth / 2, winMouseY + 1);
 *
 *   //the x of the square is relative to the canvas
 *   rect(mouseX, 20, 60, 60);
 * }
 */
exports.winMouseY = 0;

/**
 * The system variable pwinMouseX always contains the horizontal position
 * of the mouse in the frame previous to the current frame, relative to
 * (0, 0) of the window. Note: pwinMouseX will be reset to the current winMouseX
 * value at the start of each touch event.
 *
 * @property {Number} pwinMouseX
 * @readOnly
 *
 * @example
 * let myCanvas;
 *
 * function setup() {
 *   //use a variable to store a pointer to the canvas
 *   myCanvas = createCanvas(100, 100);
 *   noStroke();
 *   fill(237, 34, 93);
 * }
 *
 * function draw() {
 *   clear();
 *   //the difference between previous and
 *   //current x position is the horizontal mouse speed
 *   let speed = abs(winMouseX - pwinMouseX);
 *   //change the size of the circle
 *   //according to the horizontal speed
 *   ellipse(50, 50, 10 + speed * 5, 10 + speed * 5);
 *   //move the canvas to the mouse position
 *   myCanvas.position(winMouseX + 1, winMouseY + 1);
 * }
 */
exports.pwinMouseX = 0;

/**
 * The system variable pwinMouseY always contains the vertical position of
 * the mouse in the frame previous to the current frame, relative to (0, 0)
 * of the window. Note: pwinMouseY will be reset to the current winMouseY
 * value at the start of each touch event.
 *
 * @property {Number} pwinMouseY
 * @readOnly
 *
 *
 * @example
 * let myCanvas;
 *
 * function setup() {
 *   //use a variable to store a pointer to the canvas
 *   myCanvas = createCanvas(100, 100);
 *   noStroke();
 *   fill(237, 34, 93);
 * }
 *
 * function draw() {
 *   clear();
 *   //the difference between previous and
 *   //current y position is the vertical mouse speed
 *   let speed = abs(winMouseY - pwinMouseY);
 *   //change the size of the circle
 *   //according to the vertical speed
 *   ellipse(50, 50, 10 + speed * 5, 10 + speed * 5);
 *   //move the canvas to the mouse position
 *   myCanvas.position(winMouseX + 1, winMouseY + 1);
 * }
 */
exports.pwinMouseY = 0;

/**
 * Processing automatically tracks if the mouse button is pressed and which
 * button is pressed. The value of the system variable mouseButton is either
 * LEFT, RIGHT, or CENTER depending on which button was pressed last.
 * Warning: different browsers may track mouseButton differently.
 *
 * @property {Constant} mouseButton
 * @readOnly
 *
 * @example
 * function draw() {
 *   background(237, 34, 93);
 *   fill(0);
 *
 *   if (mouseIsPressed) {
 *     if (mouseButton === LEFT) {
 *       ellipse(50, 50, 50, 50);
 *     }
 *     if (mouseButton === RIGHT) {
 *       rect(25, 25, 50, 50);
 *     }
 *     if (mouseButton === CENTER) {
 *       triangle(23, 75, 50, 20, 78, 75);
 *     }
 *   }
 *
 *   print(mouseButton);
 * }
 */
exports.mouseButton = 0;

/**
 * The boolean system variable mouseIsPressed is true if the mouse is pressed
 * and false if not.
 *
 * @property {Boolean} mouseIsPressed
 * @readOnly
 *
 * @example
 * function draw() {
 *   background(237, 34, 93);
 *   fill(0);
 *
 *   if (mouseIsPressed) {
 *     ellipse(50, 50, 50, 50);
 *   } else {
 *     rect(25, 25, 50, 50);
 *   }
 *
 *   print(mouseIsPressed);
 * }
 */
exports.mouseIsPressed = false;

/**
 * The >mouseMoved() function is called every time the mouse moves and a mouse
 * button is not pressed.<br><br>
 * Browsers may have different default
 * behaviors attached to various mouse events. To prevent any default
 * behavior for this event, add "return false" to the end of the method.
 *
 * @method mouseMoved
 * @param  {Object} [event] optional MouseEvent callback argument.
 * @example
 * // Move the mouse across the page
 * // to change its value
 *
 * let value = 0;
 * function draw() {
 *   fill(value);
 *   rect(25, 25, 50, 50);
 * }
 * function mouseMoved() {
 *   value = value + 5;
 *   if (value > 255) {
 *     value = 0;
 *   }
 * }
 *
 * function mouseMoved() {
 *   ellipse(mouseX, mouseY, 5, 5);
 *   // prevent default
 *   return false;
 * }
 *
 * // returns a MouseEvent object
 * // as a callback argument
 * function mouseMoved(event) {
 *   console.log(event);
 * }
 */

/**
 * The mousePressed() function is called once after every time a mouse button
 * is pressed. The mouseButton variable (see the related reference entry)
 * can be used to determine which button has been pressed.<br><br>
 * Browsers may have different default
 * behaviors attached to various mouse events. To prevent any default
 * behavior for this event, add "return false" to the end of the method.
 *
 * @method mousePressed
 * @param  {Object} [event] optional MouseEvent callback argument.
 * @example
 * // Click within the image to change
 * // the value of the rectangle
 *
 * let value = 0;
 * function draw() {
 *   fill(value);
 *   rect(25, 25, 50, 50);
 * }
 * function mousePressed() {
 *   if (value === 0) {
 *     value = 255;
 *   } else {
 *     value = 0;
 *   }
 * }
 *
 * function mousePressed() {
 *   ellipse(mouseX, mouseY, 5, 5);
 *   // prevent default
 *   return false;
 * }
 *
 * // returns a MouseEvent object
 * // as a callback argument
 * function mousePressed(event) {
 *   console.log(event);
 * }
 */

/**
* The mouseReleased() function is called every time a mouse button is
* released. <br><br>
* Browsers may have different default
* behaviors attached to various mouse events. To prevent any default
* behavior for this event, add "return false" to the end of the method.
*
*
* @method mouseReleased
* @param  {Object} [event] optional MouseEvent callback argument.
* @example
* // Click within the image to change
* // the value of the rectangle
* // after the mouse has been clicked
*
* let value = 0;
* function draw() {
*   fill(value);
*   rect(25, 25, 50, 50);
* }
* function mouseReleased() {
*   if (value === 0) {
*     value = 255;
*   } else {
*     value = 0;
*   }
* }
*
* function mouseReleased() {
*   ellipse(mouseX, mouseY, 5, 5);
*   // prevent default
*   return false;
* }
*
* // returns a MouseEvent object
* // as a callback argument
* function mouseReleased(event) {
*   console.log(event);
* }
*/

/**
 * The mouseClicked() function is called once after a mouse button has been
 * pressed and then released.<br><br>
 * Browsers handle clicks differently, so this function is only guaranteed to be
 * run when the left mouse button is clicked. To handle other mouse buttons
 * being pressed or released, see mousePressed() or mouseReleased().<br><br>
 * Browsers may have different default
 * behaviors attached to various mouse events. To prevent any default
 * behavior for this event, add "return false" to the end of the method.
 *
 * @method mouseClicked
 * @param  {Object} [event] optional MouseEvent callback argument.
 * @example
 * // Click within the image to change
 * // the value of the rectangle
 * // after the mouse has been clicked
 *
 * let value = 0;
 * function draw() {
 *   fill(value);
 *   rect(25, 25, 50, 50);
 * }
 *
 * function mouseClicked() {
 *   if (value === 0) {
 *     value = 255;
 *   } else {
 *     value = 0;
 *   }
 * }
 *
 * function mouseClicked() {
 *   ellipse(mouseX, mouseY, 5, 5);
 *   // prevent default
 *   return false;
 * }
 *
 * // returns a MouseEvent object
 * // as a callback argument
 * function mouseClicked(event) {
 *   console.log(event);
 * }
 */

/**********************************************************************************************************************
 * keyboard
 */

/**
 * The boolean system variable keyIsPressed is true if any key is pressed
 * and false if no keys are pressed.
 *
 * @property {Boolean} keyIsPressed
 * @readOnly
 * @example
 * function draw() {
 *   if (keyIsPressed === true) {
 *     fill(0);
 *   } else {
 *     fill(255);
 *   }
 *   rect(25, 25, 50, 50);
 * }
 */
exports.keyIsPressed = false;

/**
 * The system variable key always contains the value of the most recent
 * key on the keyboard that was typed.
 *
 * @property {String} key
 * @readOnly
 * @example
 * // Click any key to display it!
 * // (Not Guaranteed to be Case Sensitive)
 * function setup() {
 *   fill(245, 123, 158);
 *   textSize(50);
 * }
 *
 * function draw() {
 *   background(200);
 *   text(key, 33, 65); // Display last key pressed.
 * }
 */
exports.key = null;

/**
 * The variable keyCode is used to detect special keys such as BACKSPACE,
 * DELETE, ENTER, RETURN, TAB, ESCAPE, SHIFT, CONTROL, OPTION, ALT, UP_ARROW,
 * DOWN_ARROW, LEFT_ARROW, RIGHT_ARROW.
 *
 * @property {Integer} keyCode
 * @readOnly
 * @example
 * let fillVal = 126;
 * function draw() {
 *   fill(fillVal);
 *   rect(25, 25, 50, 50);
 * }
 *
 * function keyPressed() {
 *   if (keyCode === UP_ARROW) {
 *     fillVal = 255;
 *   } else if (keyCode === DOWN_ARROW) {
 *     fillVal = 0;
 *   }
 *   return false; // prevent default
 * }
 */
exports.keyCode = null;

/**
 * The keyPressed() function is called once every time a key is pressed. The
 * keyCode for the key that was pressed is stored in the keyCode variable.
 * <br><br>
 *
 * @method keyPressed
 * @example
 * let value = 0;
 * function draw() {
 *   fill(value);
 *   rect(25, 25, 50, 50);
 * }
 * function keyPressed() {
 *   if (value === 0) {
 *     value = 255;
 *   } else {
 *     value = 0;
 *   }
 * }
 *
 * let value = 0;
 * function draw() {
 *   fill(value);
 *   rect(25, 25, 50, 50);
 * }
 * function keyPressed() {
 *   if (keyCode === LEFT_ARROW) {
 *     value = 255;
 *   } else if (keyCode === RIGHT_ARROW) {
 *     value = 0;
 *   }
 * }
 */

/**
* The keyReleased() function is called once every time a key is released.
* See key and keyCode for more information.<br><br>
* Browsers may have different default
* behaviors attached to various key events. To prevent any default
* behavior for this event, add "return false" to the end of the method.
*
* @method keyReleased
* @example
* let value = 0;
* function draw() {
*   fill(value);
*   rect(25, 25, 50, 50);
* }
* function keyReleased() {
*   if (value === 0) {
*     value = 255;
*   } else {
*     value = 0;
*   }
*   return false; // prevent any default behavior
* }
*/

/**
 * The keyTyped() function is called once every time a key is pressed, but
 * action keys such as Ctrl, Shift, and Alt are ignored. The most recent
 * key pressed will be stored in the key variable.
 * <br><br>
 * Because of how operating systems handle key repeats, holding down a key
 * will cause multiple calls to keyTyped() (and keyReleased() as well). The
 * rate of repeat is set by the operating system and how each computer is
 * configured.<br><br>
 * Browsers may have different default behaviors attached to various key
 * events. To prevent any default behavior for this event, add "return false"
 * to the end of the method.
 *
 * @method keyTyped
 * @example
 * let value = 0;
 * function draw() {
 *   fill(value);
 *   rect(25, 25, 50, 50);
 * }
 * function keyTyped() {
 *   if (key === 'a') {
 *     value = 255;
 *   } else if (key === 'b') {
 *     value = 0;
 *   }
 *   // uncomment to prevent any default behavior
 *   // return false;
 * }
 */
