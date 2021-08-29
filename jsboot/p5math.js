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
 * Creates a new PVector (the datatype for storing vectors). This provides a
 * two or three dimensional vector, specifically a Euclidean (also known as
 * geometric) vector. A vector is an entity that has both magnitude and
 * direction.
 *
 * @method createVector
 * @param {Number} [x] x component of the vector
 * @param {Number} [y] y component of the vector
 * @param {Number} [z] z component of the vector
 * @return {p5.Vector}
 * @example
 * function setup() {
 *   createCanvas(100, 100, WEBGL);
 *   noStroke();
 *   fill(255, 102, 204);
 * }
 *
 * function draw() {
 *   background(255);
 *   pointLight(color(255), createVector(sin(millis() / 1000) * 20, -40, -10));
 *   scale(0.75);
 *   sphere();
 * }
 */
exports.createVector = function (x, y, z) {
	return new PVector(x, y, z);
};

/**********************************************************************************************************************
 * Calculations
 */

/**
 * Calculates the absolute value (magnitude) of a number. Maps to Math.abs().
 * The absolute value of a number is always positive.
 *
 * @method abs
 * @param  {Number} n number to compute
 * @return {Number}   absolute value of given number
 * @example
 * function setup() {
 *   let x = -3;
 *   let y = abs(x);
 *
 *   print(x); // -3
 *   print(y); // 3
 * }
 */
exports.abs = Math.abs;

/**
 * Calculates the closest int value that is greater than or equal to the
 * value of the parameter. Maps to Math.ceil(). For example, ceil(9.03)
 * returns the value 10.
 *
 * @method ceil
 * @param  {Number} n number to round up
 * @return {Integer}   rounded up number
 * @example
 * function draw() {
 *   background(200);
 *   // map, mouseX between 0 and 5.
 *   let ax = map(mouseX, 0, 100, 0, 5);
 *   let ay = 66;
 *
 *   //Get the ceiling of the mapped number.
 *   let bx = ceil(map(mouseX, 0, 100, 0, 5));
 *   let by = 33;
 *
 *   // Multiply the mapped numbers by 20 to more easily
 *   // see the changes.
 *   stroke(0);
 *   fill(0);
 *   line(0, ay, ax * 20, ay);
 *   line(0, by, bx * 20, by);
 *
 *   // Reformat the float returned by map and draw it.
 *   noStroke();
 *   text(nfc(ax, 2), ax, ay - 5);
 *   text(nfc(bx, 1), bx, by - 5);
 * }
 */
exports.ceil = Math.ceil;

/**
 * Constrains a value between a minimum and maximum value.
 *
 * @method constrain
 * @param  {Number} n    number to constrain
 * @param  {Number} low  minimum limit
 * @param  {Number} high maximum limit
 * @return {Number}      constrained number
 * @example
 * function draw() {
 *   background(200);
 *
 *   let leftWall = 25;
 *   let rightWall = 75;
 *
 *   // xm is just the mouseX, while
 *   // xc is the mouseX, but constrained
 *   // between the leftWall and rightWall!
 *   let xm = mouseX;
 *   let xc = constrain(mouseX, leftWall, rightWall);
 *
 *   // Draw the walls.
 *   stroke(150);
 *   line(leftWall, 0, leftWall, height);
 *   line(rightWall, 0, rightWall, height);
 *
 *   // Draw xm and xc as circles.
 *   noStroke();
 *   fill(150);
 *   ellipse(xm, 33, 9, 9); // Not Constrained
 *   fill(0);
 *   ellipse(xc, 66, 9, 9); // Constrained
 * }
 */
exports.constrain = function (n, low, high) {
	return Math.max(Math.min(n, high), low);
};

/**
 * Calculates the distance between two points.
 *
 * @method dist
 * @param  {Number} x1 x-coordinate of the first point
 * @param  {Number} y1 y-coordinate of the first point
 * @param  {Number} x2 x-coordinate of the second point
 * @param  {Number} y2 y-coordinate of the second point
 * @return {Number}    distance between the two points
 *
 * @example
 * // Move your mouse inside the canvas to see the
 * // change in distance between two points!
 * function draw() {
 *   background(200);
 *   fill(0);
 *
 *   let x1 = 10;
 *   let y1 = 90;
 *   let x2 = mouseX;
 *   let y2 = mouseY;
 *
 *   line(x1, y1, x2, y2);
 *   ellipse(x1, y1, 7, 7);
 *   ellipse(x2, y2, 7, 7);
 *
 *   // d is the length of the line
 *   // the distance from point 1 to point 2.
 *   let d = int(dist(x1, y1, x2, y2));
 *
 *   // Let's write d along the line we are drawing!
 *   push();
 *   translate((x1 + x2) / 2, (y1 + y2) / 2);
 *   rotate(atan2(y2 - y1, x2 - x1));
 *   text(nfc(d, 1), 0, -5);
 *   pop();
 *   // Fancy!
 * }
 */
exports.dist = function () {
	if (arguments.length === 4) {
		//2D
		return hypot(arguments[2] - arguments[0], arguments[3] - arguments[1]);
	} else if (arguments.length === 6) {
		//3D
		return hypot(
			arguments[3] - arguments[0],
			arguments[4] - arguments[1],
			arguments[5] - arguments[2]
		);
	}
};

/**
 * Returns Euler's number e (2.71828...) raised to the power of the n
 * parameter. Maps to Math.exp().
 *
 * @method exp
 * @param  {Number} n exponent to raise
 * @return {Number}   e^n
 * @example
 * function draw() {
 *   background(200);
 *
 *   // Compute the exp() function with a value between 0 and 2
 *   let xValue = map(mouseX, 0, width, 0, 2);
 *   let yValue = exp(xValue);
 *
 *   let y = map(yValue, 0, 8, height, 0);
 *
 *   let legend = 'exp (' + nfc(xValue, 3) + ')\n= ' + nf(yValue, 1, 4);
 *   stroke(150);
 *   line(mouseX, y, mouseX, height);
 *   fill(0);
 *   text(legend, 5, 15);
 *   noStroke();
 *   ellipse(mouseX, y, 7, 7);
 *
 *   // Draw the exp(x) curve,
 *   // over the domain of x from 0 to 2
 *   noFill();
 *   stroke(0);
 *   beginShape();
 *   for (let x = 0; x < width; x++) {
 *     xValue = map(x, 0, width, 0, 2);
 *     yValue = exp(xValue);
 *     y = map(yValue, 0, 8, height, 0);
 *     vertex(x, y);
 *   }
 *
 *   endShape();
 *   line(0, 0, 0, height);
 *   line(0, height - 1, width, height - 1);
 * }
 */
exports.exp = Math.exp;

/**
 * Calculates the closest int value that is less than or equal to the
 * value of the parameter. Maps to Math.floor().
 *
 * @method floor
 * @param  {Number} n number to round down
 * @return {Integer}  rounded down number
 * @example
 * function draw() {
 *   background(200);
 *   //map, mouseX between 0 and 5.
 *   let ax = map(mouseX, 0, 100, 0, 5);
 *   let ay = 66;
 *
 *   //Get the floor of the mapped number.
 *   let bx = floor(map(mouseX, 0, 100, 0, 5));
 *   let by = 33;
 *
 *   // Multiply the mapped numbers by 20 to more easily
 *   // see the changes.
 *   stroke(0);
 *   fill(0);
 *   line(0, ay, ax * 20, ay);
 *   line(0, by, bx * 20, by);
 *
 *   // Reformat the float returned by map and draw it.
 *   noStroke();
 *   text(nfc(ax, 2), ax, ay - 5);
 *   text(nfc(bx, 1), bx, by - 5);
 * }
 */
exports.floor = Math.floor;

/**
 * Calculates a number between two numbers at a specific increment. The amt
 * parameter is the amount to interpolate between the two values where 0.0
 * equal to the first point, 0.1 is very near the first point, 0.5 is
 * half-way in between, and 1.0 is equal to the second point. If the
 * value of amt is more than 1.0 or less than 0.0, the number will be
 * calculated accordingly in the ratio of the two given numbers. The lerp
 * function is convenient for creating motion along a straight
 * path and for drawing dotted lines.
 *
 * @method lerp
 * @param  {Number} start first value
 * @param  {Number} stop  second value
 * @param  {Number} amt   number
 * @return {Number}       lerped value
 * @example
 * function setup() {
 *   background(200);
 *   let a = 20;
 *   let b = 80;
 *   let c = lerp(a, b, 0.2);
 *   let d = lerp(a, b, 0.5);
 *   let e = lerp(a, b, 0.8);
 *
 *   let y = 50;
 *
 *   strokeWeight(5);
 *   stroke(0); // Draw the original points in black
 *   point(a, y);
 *   point(b, y);
 *
 *   stroke(100); // Draw the lerp points in gray
 *   point(c, y);
 *   point(d, y);
 *   point(e, y);
 * }
 */
exports.lerp = function (start, stop, amt) {
	return amt * (stop - start) + start;
};

/**
 * Calculates the natural logarithm (the base-e logarithm) of a number. This
 * function expects the n parameter to be a value greater than 0.0. Maps to
 * Math.log().
 *
 * @method log
 * @param  {Number} n number greater than 0
 * @return {Number}   natural logarithm of n
 * @example
 * function draw() {
 *   background(200);
 *   let maxX = 2.8;
 *   let maxY = 1.5;
 *
 *   // Compute the natural log of a value between 0 and maxX
 *   let xValue = map(mouseX, 0, width, 0, maxX);
 *   let yValue, y;
 *   if (xValue > 0) {
 *   // Cannot take the log of a negative number.
 *     yValue = log(xValue);
 *     y = map(yValue, -maxY, maxY, height, 0);
 *
 *     // Display the calculation occurring.
 *     let legend = 'log(' + nf(xValue, 1, 2) + ')\n= ' + nf(yValue, 1, 3);
 *     stroke(150);
 *     line(mouseX, y, mouseX, height);
 *     fill(0);
 *     text(legend, 5, 15);
 *     noStroke();
 *     ellipse(mouseX, y, 7, 7);
 *   }
 *
 *   // Draw the log(x) curve,
 *   // over the domain of x from 0 to maxX
 *   noFill();
 *   stroke(0);
 *   beginShape();
 *   for (let x = 0; x < width; x++) {
 *     xValue = map(x, 0, width, 0, maxX);
 *     yValue = log(xValue);
 *     y = map(yValue, -maxY, maxY, height, 0);
 *     vertex(x, y);
 *   }
 *   endShape();
 *   line(0, 0, 0, height);
 *   line(0, height / 2, width, height / 2);
 * }
 */
exports.log = Math.log;

/**
 * Calculates the magnitude (or length) of a vector. A vector is a direction
 * in space commonly used in computer graphics and linear algebra. Because it
 * has no "start" position, the magnitude of a vector can be thought of as
 * the distance from the coordinate 0,0 to its x,y value. Therefore, mag() is
 * a shortcut for writing dist(0, 0, x, y).
 *
 * @method mag
 * @param  {Number} a first value
 * @param  {Number} b second value
 * @return {Number}   magnitude of vector from (0,0) to (a,b)
 * @example
 * function setup() {
 *   let x1 = 20;
 *   let x2 = 80;
 *   let y1 = 30;
 *   let y2 = 70;
 *
 *   line(0, 0, x1, y1);
 *   print(mag(x1, y1)); // Prints "36.05551275463989"
 *   line(0, 0, x2, y1);
 *   print(mag(x2, y1)); // Prints "85.44003745317531"
 *   line(0, 0, x1, y2);
 *   print(mag(x1, y2)); // Prints "72.80109889280519"
 *   line(0, 0, x2, y2);
 *   print(mag(x2, y2)); // Prints "106.3014581273465"
 * }
 */
exports.mag = function (x, y) {
	return hypot(x, y);
};

/**
 * Re-maps a number from one range to another.
 * <br><br>
 * In the first example above, the number 25 is converted from a value in the
 * range of 0 to 100 into a value that ranges from the left edge of the
 * window (0) to the right edge (width).
 *
 * @method map
 * @param  {Number} value  the incoming value to be converted
 * @param  {Number} start1 lower bound of the value's current range
 * @param  {Number} stop1  upper bound of the value's current range
 * @param  {Number} start2 lower bound of the value's target range
 * @param  {Number} stop2  upper bound of the value's target range
 * @param  {Boolean} [withinBounds] constrain the value to the newly mapped range
 * @return {Number}        remapped number
 * @example
 * let value = 25;
 * let m = map(value, 0, 100, 0, width);
 * ellipse(m, 50, 10, 10);
 *
 * function setup() {
 *   noStroke();
 * }
 *
 * function draw() {
 *   background(204);
 *   let x1 = map(mouseX, 0, width, 25, 75);
 *   ellipse(x1, 25, 25, 25);
 *   //This ellipse is constrained to the 0-100 range
 *   //after setting withinBounds to true
 *   let x2 = map(mouseX, 0, width, 0, 100, true);
 *   ellipse(x2, 75, 25, 25);
 * }
 */
exports.map = function (n, start1, stop1, start2, stop2, withinBounds) {
	var newval = (n - start1) / (stop1 - start1) * (stop2 - start2) + start2;
	if (!withinBounds) {
		return newval;
	}
	if (start2 < stop2) {
		return this.constrain(newval, start2, stop2);
	} else {
		return this.constrain(newval, stop2, start2);
	}
};

/**
 * Determines the largest value in a sequence of numbers, and then returns
 * that value. max() accepts any number of Number parameters, or an Array
 * of any length.
 *
 * @method max
 * @param  {Number} n0 Number to compare
 * @param  {Number} n1 Number to compare
 * @return {Number}             maximum Number
 * @example
 * function setup() {
 *   // Change the elements in the array and run the sketch
 *   // to show how max() works!
 *   let numArray = [2, 1, 5, 4, 8, 9];
 *   fill(0);
 *   noStroke();
 *   text('Array Elements', 0, 10);
 *   // Draw all numbers in the array
 *   let spacing = 15;
 *   let elemsY = 25;
 *   for (let i = 0; i < numArray.length; i++) {
 *     text(numArray[i], i * spacing, elemsY);
 *   }
 *   let maxX = 33;
 *   let maxY = 80;
 *   // Draw the Maximum value in the array.
 *   textSize(32);
 *   text(max(numArray), maxX, maxY);
 * }
 */
exports.max = function () {
	if (arguments[0] instanceof Array) {
		return Math.max.apply(null, arguments[0]);
	} else {
		return Math.max.apply(null, arguments);
	}
};

/**
 * Determines the smallest value in a sequence of numbers, and then returns
 * that value. min() accepts any number of Number parameters, or an Array
 * of any length.
 *
 * @method min
 * @param  {Number} n0 Number to compare
 * @param  {Number} n1 Number to compare
 * @return {Number}             minimum Number
 * @example
 * function setup() {
 *   // Change the elements in the array and run the sketch
 *   // to show how min() works!
 *   let numArray = [2, 1, 5, 4, 8, 9];
 *   fill(0);
 *   noStroke();
 *   text('Array Elements', 0, 10);
 *   // Draw all numbers in the array
 *   let spacing = 15;
 *   let elemsY = 25;
 *   for (let i = 0; i < numArray.length; i++) {
 *     text(numArray[i], i * spacing, elemsY);
 *   }
 *   let maxX = 33;
 *   let maxY = 80;
 *   // Draw the Minimum value in the array.
 *   textSize(32);
 *   text(min(numArray), maxX, maxY);
 * }
 */
exports.min = function () {
	if (arguments[0] instanceof Array) {
		return Math.min.apply(null, arguments[0]);
	} else {
		return Math.min.apply(null, arguments);
	}
};

/**
 * Normalizes a number from another range into a value between 0 and 1.
 * Identical to map(value, low, high, 0, 1).
 * Numbers outside of the range are not clamped to 0 and 1, because
 * out-of-range values are often intentional and useful. (See the second
 * example above.)
 *
 * @method norm
 * @param  {Number} value incoming value to be normalized
 * @param  {Number} start lower bound of the value's current range
 * @param  {Number} stop  upper bound of the value's current range
 * @return {Number}       normalized number
 * @example
 * function draw() {
 *   background(200);
 *   let currentNum = mouseX;
 *   let lowerBound = 0;
 *   let upperBound = width; //100;
 *   let normalized = norm(currentNum, lowerBound, upperBound);
 *   let lineY = 70;
 *   line(0, lineY, width, lineY);
 *   //Draw an ellipse mapped to the non-normalized value.
 *   noStroke();
 *   fill(50);
 *   let s = 7; // ellipse size
 *   ellipse(currentNum, lineY, s, s);
 *
 *   // Draw the guide
 *   let guideY = lineY + 15;
 *   text('0', 0, guideY);
 *   textAlign(RIGHT);
 *   text('100', width, guideY);
 *
 *   // Draw the normalized value
 *   textAlign(LEFT);
 *   fill(0);
 *   textSize(32);
 *   let normalY = 40;
 *   let normalX = 20;
 *   text(normalized, normalX, normalY);
 * }
 */
exports.norm = function (n, start, stop) {
	return this.map(n, start, stop, 0, 1);
};

/**
 * Facilitates exponential expressions. The pow() function is an efficient
 * way of multiplying numbers by themselves (or their reciprocals) in large
 * quantities. For example, pow(3, 5) is equivalent to the expression
 * 3*3*3*3*3 and pow(3, -5) is equivalent to 1 / 3*3*3*3*3. Maps to
 * Math.pow().
 *
 * @method pow
 * @param  {Number} n base of the exponential expression
 * @param  {Number} e power by which to raise the base
 * @return {Number}   n^e
 * @example
 * function setup() {
 *   //Exponentially increase the size of an ellipse.
 *   let eSize = 3; // Original Size
 *   let eLoc = 10; // Original Location
 *
 *   ellipse(eLoc, eLoc, eSize, eSize);
 *
 *   ellipse(eLoc * 2, eLoc * 2, pow(eSize, 2), pow(eSize, 2));
 *
 *   ellipse(eLoc * 4, eLoc * 4, pow(eSize, 3), pow(eSize, 3));
 *
 *   ellipse(eLoc * 8, eLoc * 8, pow(eSize, 4), pow(eSize, 4));
 * }
 */
exports.pow = Math.pow;

/**
 * Calculates the integer closest to the n parameter. For example,
 * round(133.8) returns the value 134. Maps to Math.round().
 *
 * @method round
 * @param  {Number} n number to round
 * @return {Integer}  rounded number
 * @example
 * function draw() {
 *   background(200);
 *   //map, mouseX between 0 and 5.
 *   let ax = map(mouseX, 0, 100, 0, 5);
 *   let ay = 66;
 *
 *   // Round the mapped number.
 *   let bx = round(map(mouseX, 0, 100, 0, 5));
 *   let by = 33;
 *
 *   // Multiply the mapped numbers by 20 to more easily
 *   // see the changes.
 *   stroke(0);
 *   fill(0);
 *   line(0, ay, ax * 20, ay);
 *   line(0, by, bx * 20, by);
 *
 *   // Reformat the float returned by map and draw it.
 *   noStroke();
 *   text(nfc(ax, 2), ax, ay - 5);
 *   text(nfc(bx, 1), bx, by - 5);
 * }
 */
exports.round = Math.round;

/**
 * Squares a number (multiplies a number by itself). The result is always a
 * positive number, as multiplying two negative numbers always yields a
 * positive result. For example, -1 * -1 = 1.
 *
 * @method sq
 * @param  {Number} n number to square
 * @return {Number}   squared number
 * @example
 * function draw() {
 *   background(200);
 *   let eSize = 7;
 *   let x1 = map(mouseX, 0, width, 0, 10);
 *   let y1 = 80;
 *   let x2 = sq(x1);
 *   let y2 = 20;
 *
 *   // Draw the non-squared.
 *   line(0, y1, width, y1);
 *   ellipse(x1, y1, eSize, eSize);
 *
 *   // Draw the squared.
 *   line(0, y2, width, y2);
 *   ellipse(x2, y2, eSize, eSize);
 *
 *   // Draw dividing line.
 *   stroke(100);
 *   line(0, height / 2, width, height / 2);
 *
 *   // Draw text.
 *   let spacing = 15;
 *   noStroke();
 *   fill(0);
 *   text('x = ' + x1, 0, y1 + spacing);
 *   text('sq(x) = ' + x2, 0, y2 + spacing);
 * }
 */
exports.sq = function (n) {
	return n * n;
};

/**
 * Calculates the square root of a number. The square root of a number is
 * always positive, even though there may be a valid negative root. The
 * square root s of number a is such that s*s = a. It is the opposite of
 * squaring. Maps to Math.sqrt().
 *
 * @method sqrt
 * @param  {Number} n non-negative number to square root
 * @return {Number}   square root of number
 * @example
 * function draw() {
 *   background(200);
 *   let eSize = 7;
 *   let x1 = mouseX;
 *   let y1 = 80;
 *   let x2 = sqrt(x1);
 *   let y2 = 20;
 *
 *   // Draw the non-squared.
 *   line(0, y1, width, y1);
 *   ellipse(x1, y1, eSize, eSize);
 *
 *   // Draw the squared.
 *   line(0, y2, width, y2);
 *   ellipse(x2, y2, eSize, eSize);
 *
 *   // Draw dividing line.
 *   stroke(100);
 *   line(0, height / 2, width, height / 2);
 *
 *   // Draw text.
 *   noStroke();
 *   fill(0);
 *   let spacing = 15;
 *   text('x = ' + x1, 0, y1 + spacing);
 *   text('sqrt(x) = ' + x2, 0, y2 + spacing);
 * }
 */
exports.sqrt = Math.sqrt;

// Calculate the length of the hypotenuse of a right triangle
// This won't under- or overflow in intermediate steps
// https://en.wikipedia.org/wiki/Hypot
function hypot(x, y, z) {
	// Use the native implementation if it's available
	if (typeof Math.hypot === 'function') {
		return Math.hypot.apply(null, arguments);
	}

	// Otherwise use the V8 implementation
	// https://github.com/v8/v8/blob/8cd3cf297287e581a49e487067f5cbd991b27123/src/js/math.js#L217
	var length = arguments.length;
	var args = [];
	var max = 0;
	for (var i = 0; i < length; i++) {
		var n = arguments[i];
		n = +n;
		if (n === Infinity || n === -Infinity) {
			return Infinity;
		}
		n = Math.abs(n);
		if (n > max) {
			max = n;
		}
		args[i] = n;
	}

	if (max === 0) {
		max = 1;
	}
	var sum = 0;
	var compensation = 0;
	for (var j = 0; j < length; j++) {
		var m = args[j] / max;
		var summand = m * m - compensation;
		var preliminary = sum + summand;
		compensation = preliminary - sum - summand;
		sum = preliminary;
	}
	return Math.sqrt(sum) * max;
}

/**********************************************************************************************************************
 * Random
 */

var seeded = false;
var previous = false;
var y2 = 0;

// Linear Congruential Generator
// Variant of a Lehman Generator
var lcg = (function () {
	// Set to values from http://en.wikipedia.org/wiki/Numerical_Recipes
	// m is basically chosen to be large (as it is the max period)
	// and for its relationships to a and c
	var m = 4294967296,
		// a - 1 should be divisible by m's prime factors
		a = 1664525,
		// c and m should be co-prime
		c = 1013904223,
		seed,
		z;
	return {
		setSeed: function (val) {
			// pick a random seed if val is undefined or null
			// the >>> 0 casts the seed to an unsigned 32-bit integer
			z = seed = (val == null ? Math.random() * m : val) >>> 0;
		},
		getSeed: function () {
			return seed;
		},
		rand: function () {
			// define the recurrence relationship
			z = (a * z + c) % m;
			// return a float in [0, 1)
			// if z = m then z / m = 0 therefore (z % m) / m < 1 always
			return z / m;
		}
	};
})();

/**
 * Sets the seed value for random().
 *
 * By default, random() produces different results each time the program
 * is run. Set the seed parameter to a constant to return the same
 * pseudo-random numbers each time the software is run.
 *
 * @method randomSeed
 * @param {Number} seed   the seed value
 * @example
 * randomSeed(99);
 * for (let i = 0; i < 100; i++) {
 *   let r = random(0, 255);
 *   stroke(r);
 *   line(i, 0, i, 100);
 * }
 */
exports.randomSeed = function (seed) {
	lcg.setSeed(seed);
	seeded = true;
	previous = false;
};

/**
 * Return a random floating-point number.
 *
 * Takes either 0, 1 or 2 arguments.
 *
 * If no argument is given, returns a random number from 0
 * up to (but not including) 1.
 *
 * If one argument is given and it is a number, returns a random number from 0
 * up to (but not including) the number.
 *
 * If one argument is given and it is an array, returns a random element from
 * that array.
 *
 * If two arguments are given, returns a random number from the
 * first argument up to (but not including) the second argument.
 *
 * @method random
 * @param  {Number} [min]   the lower bound (inclusive)
 * @param  {Number} [max]   the upper bound (exclusive)
 * @return {Number} the random number
 * @example
 * for (let i = 0; i < 100; i++) {
 *   let r = random(50);
 *   stroke(r * 5);
 *   line(50, i, 50 + r, i);
 * }
 * 
 * for (let i = 0; i < 100; i++) {
 *   let r = random(-50, 50);
 *   line(50, i, 50 + r, i);
 * }
 * 
 * // Get a random element from an array using the random(Array) syntax
 * let words = ['apple', 'bear', 'cat', 'dog'];
 * let word = random(words); // select random word
 * text(word, 10, 50); // draw the word
 */
exports.random = function (min, max) {
	var rand;

	if (seeded) {
		rand = lcg.rand();
	} else {
		rand = Math.random();
	}
	if (typeof min === 'undefined') {
		return rand;
	} else if (typeof max === 'undefined') {
		if (min instanceof Array) {
			return min[Math.floor(rand * min.length)];
		} else {
			return rand * min;
		}
	} else {
		if (min > max) {
			var tmp = min;
			min = max;
			max = tmp;
		}

		return rand * (max - min) + min;
	}
};

/**
 *
 * Returns a random number fitting a Gaussian, or
 * normal, distribution. There is theoretically no minimum or maximum
 * value that randomGaussian() might return. Rather, there is
 * just a very low probability that values far from the mean will be
 * returned; and a higher probability that numbers near the mean will
 * be returned.
 * <br><br>
 * Takes either 0, 1 or 2 arguments.<br>
 * If no args, returns a mean of 0 and standard deviation of 1.<br>
 * If one arg, that arg is the mean (standard deviation is 1).<br>
 * If two args, first is mean, second is standard deviation.
 *
 * @method randomGaussian
 * @param  {Number} mean  the mean
 * @param  {Number} sd    the standard deviation
 * @return {Number} the random number
 * @example
 * for (let y = 0; y < 100; y++) {
 *   let x = randomGaussian(50, 15);
 *   line(50, y, x, y);
 * }
 * 
 * let distribution = new Array(360);
 *
 * function setup() {
 *   createCanvas(100, 100);
 *   for (let i = 0; i < distribution.length; i++) {
 *     distribution[i] = floor(randomGaussian(0, 15));
 *   }
 * }
 *
 * function draw() {
 *   background(204);
 *
 *   translate(width / 2, width / 2);
 *
 *   for (let i = 0; i < distribution.length; i++) {
 *     rotate(TWO_PI / distribution.length);
 *     stroke(0);
 *     let dist = abs(distribution[i]);
 *     line(0, 0, dist, 0);
 *   }
 * }
 */
exports.randomGaussian = function (mean, sd) {
	var y1, x1, x2, w;
	if (previous) {
		y1 = y2;
		previous = false;
	} else {
		do {
			x1 = this.random(2) - 1;
			x2 = this.random(2) - 1;
			w = x1 * x1 + x2 * x2;
		} while (w >= 1);
		w = Math.sqrt(-2 * Math.log(w) / w);
		y1 = x1 * w;
		y2 = x2 * w;
		previous = true;
	}

	var m = mean || 0;
	var s = sd || 1;
	return y1 * s + m;
};

/**********************************************************************************************************************
 * noise
 */

var PERLIN_YWRAPB = 4;
var PERLIN_YWRAP = 1 << PERLIN_YWRAPB;
var PERLIN_ZWRAPB = 8;
var PERLIN_ZWRAP = 1 << PERLIN_ZWRAPB;
var PERLIN_SIZE = 4095;

var perlin_octaves = 4; // default to medium smooth
var perlin_amp_falloff = 0.5; // 50% reduction/octave

var scaled_cosine = function (i) {
	return 0.5 * (1.0 - Math.cos(i * Math.PI));
};

var perlin; // will be initialized lazily by noise() or noiseSeed()

/**
 * Returns the Perlin noise value at specified coordinates. Perlin noise is
 * a random sequence generator producing a more natural ordered, harmonic
 * succession of numbers compared to the standard <b>random()</b> function.
 * It was invented by Ken Perlin in the 1980s and been used since in
 * graphical applications to produce procedural textures, natural motion,
 * shapes, terrains etc.<br /><br /> The main difference to the
 * <b>random()</b> function is that Perlin noise is defined in an infinite
 * n-dimensional space where each pair of coordinates corresponds to a
 * fixed semi-random value (fixed only for the lifespan of the program; see
 * the noiseSeed() function). p5.js can compute 1D, 2D and 3D noise,
 * depending on the number of coordinates given. The resulting value will
 * always be between 0.0 and 1.0. The noise value can be animated by moving
 * through the noise space as demonstrated in the example above. The 2nd
 * and 3rd dimension can also be interpreted as time.<br /><br />The actual
 * noise is structured similar to an audio signal, in respect to the
 * function's use of frequencies. Similar to the concept of harmonics in
 * physics, perlin noise is computed over several octaves which are added
 * together for the final result. <br /><br />Another way to adjust the
 * character of the resulting sequence is the scale of the input
 * coordinates. As the function works within an infinite space the value of
 * the coordinates doesn't matter as such, only the distance between
 * successive coordinates does (eg. when using <b>noise()</b> within a
 * loop). As a general rule the smaller the difference between coordinates,
 * the smoother the resulting noise sequence will be. Steps of 0.005-0.03
 * work best for most applications, but this will differ depending on use.
 *
 *
 * @method noise
 * @param  {Number} x   x-coordinate in noise space
 * @param  {Number} [y] y-coordinate in noise space
 * @param  {Number} [z] z-coordinate in noise space
 * @return {Number}     Perlin noise value (between 0 and 1) at specified
 *                      coordinates
 * @example
 * let xoff = 0.0;
 *
 * function draw() {
 *   background(204);
 *   xoff = xoff + 0.01;
 *   let n = noise(xoff) * width;
 *   line(n, 0, n, height);
 * }
 * 
 * let noiseScale=0.02;
 *
 * function draw() {
 *   background(0);
 *   for (let x=0; x < width; x++) {
 *     let noiseVal = noise((mouseX+x)*noiseScale, mouseY*noiseScale);
 *     stroke(noiseVal*255);
 *     line(x, mouseY+noiseVal*80, x, height);
 *   }
 * }
 */
exports.noise = function (x, y, z) {
	y = y || 0;
	z = z || 0;

	if (perlin == null) {
		perlin = new Array(PERLIN_SIZE + 1);
		for (var i = 0; i < PERLIN_SIZE + 1; i++) {
			perlin[i] = Math.random();
		}
	}

	if (x < 0) {
		x = -x;
	}
	if (y < 0) {
		y = -y;
	}
	if (z < 0) {
		z = -z;
	}

	var xi = Math.floor(x),
		yi = Math.floor(y),
		zi = Math.floor(z);
	var xf = x - xi;
	var yf = y - yi;
	var zf = z - zi;
	var rxf, ryf;

	var r = 0;
	var ampl = 0.5;

	var n1, n2, n3;

	for (var o = 0; o < perlin_octaves; o++) {
		var of = xi + (yi << PERLIN_YWRAPB) + (zi << PERLIN_ZWRAPB);

		rxf = scaled_cosine(xf);
		ryf = scaled_cosine(yf);

		n1 = perlin[of & PERLIN_SIZE];
		n1 += rxf * (perlin[(of + 1) & PERLIN_SIZE] - n1);
		n2 = perlin[(of + PERLIN_YWRAP) & PERLIN_SIZE];
		n2 += rxf * (perlin[(of + PERLIN_YWRAP + 1) & PERLIN_SIZE] - n2);
		n1 += ryf * (n2 - n1);

		of += PERLIN_ZWRAP;
		n2 = perlin[of & PERLIN_SIZE];
		n2 += rxf * (perlin[(of + 1) & PERLIN_SIZE] - n2);
		n3 = perlin[(of + PERLIN_YWRAP) & PERLIN_SIZE];
		n3 += rxf * (perlin[(of + PERLIN_YWRAP + 1) & PERLIN_SIZE] - n3);
		n2 += ryf * (n3 - n2);

		n1 += scaled_cosine(zf) * (n2 - n1);

		r += n1 * ampl;
		ampl *= perlin_amp_falloff;
		xi <<= 1;
		xf *= 2;
		yi <<= 1;
		yf *= 2;
		zi <<= 1;
		zf *= 2;

		if (xf >= 1.0) {
			xi++;
			xf--;
		}
		if (yf >= 1.0) {
			yi++;
			yf--;
		}
		if (zf >= 1.0) {
			zi++;
			zf--;
		}
	}
	return r;
};

exports.noiseDetail = function (lod, falloff) {
	if (lod > 0) {
		perlin_octaves = lod;
	}
	if (falloff > 0) {
		perlin_amp_falloff = falloff;
	}
};

/**
 *
 * Adjusts the character and level of detail produced by the Perlin noise
 * function. Similar to harmonics in physics, noise is computed over
 * several octaves. Lower octaves contribute more to the output signal and
 * as such define the overall intensity of the noise, whereas higher octaves
 * create finer grained details in the noise sequence.
 * <br><br>
 * By default, noise is computed over 4 octaves with each octave contributing
 * exactly half than its predecessor, starting at 50% strength for the 1st
 * octave. This falloff amount can be changed by adding an additional function
 * parameter. Eg. a falloff factor of 0.75 means each octave will now have
 * 75% impact (25% less) of the previous lower octave. Any value between
 * 0.0 and 1.0 is valid, however note that values greater than 0.5 might
 * result in greater than 1.0 values returned by <b>noise()</b>.
 * <br><br>
 * By changing these parameters, the signal created by the <b>noise()</b>
 * function can be adapted to fit very specific needs and characteristics.
 *
 * @method noiseDetail
 * @param {Number} lod number of octaves to be used by the noise
 * @param {Number} falloff falloff factor for each octave
 * @example
 * let noiseVal;
 * let noiseScale = 0.02;
 *
 * function setup() {
 *   createCanvas(100, 100);
 * }
 *
 * function draw() {
 *   background(0);
 *   for (let y = 0; y < height; y++) {
 *     for (let x = 0; x < width / 2; x++) {
 *       noiseDetail(2, 0.2);
 *       noiseVal = noise((mouseX + x) * noiseScale, (mouseY + y) * noiseScale);
 *       stroke(noiseVal * 255);
 *       point(x, y);
 *       noiseDetail(8, 0.65);
 *       noiseVal = noise(
 *         (mouseX + x + width / 2) * noiseScale,
 *         (mouseY + y) * noiseScale
 *       );
 *       stroke(noiseVal * 255);
 *       point(x + width / 2, y);
 *     }
 *   }
 * }
 */
exports.noiseSeed = function (seed) {
	// Linear Congruential Generator
	// Variant of a Lehman Generator
	var lcg = (function () {
		// Set to values from http://en.wikipedia.org/wiki/Numerical_Recipes
		// m is basically chosen to be large (as it is the max period)
		// and for its relationships to a and c
		var m = 4294967296;
		// a - 1 should be divisible by m's prime factors
		var a = 1664525;
		// c and m should be co-prime
		var c = 1013904223;
		var seed, z;
		return {
			setSeed: function (val) {
				// pick a random seed if val is undefined or null
				// the >>> 0 casts the seed to an unsigned 32-bit integer
				z = seed = (val == null ? Math.random() * m : val) >>> 0;
			},
			getSeed: function () {
				return seed;
			},
			rand: function () {
				// define the recurrence relationship
				z = (a * z + c) % m;
				// return a float in [0, 1)
				// if z = m then z / m = 0 therefore (z % m) / m < 1 always
				return z / m;
			}
		};
	})();

	lcg.setSeed(seed);
	perlin = new Array(PERLIN_SIZE + 1);
	for (var i = 0; i < PERLIN_SIZE + 1; i++) {
		perlin[i] = lcg.rand();
	}
};

/**********************************************************************************************************************
 * trigonometry
 */

/*
 * all DEGREES/RADIANS conversion should be done in the p5 instance
 * if possible, using the p5._toRadians(), p5._fromRadians() methods.
 */
exports._angleMode = RADIANS;

/**
 * The inverse of cos(), returns the arc cosine of a value. This function
 * expects the values in the range of -1 to 1 and values are returned in
 * the range 0 to PI (3.1415927).
 *
 * @method acos
 * @param  {Number} value the value whose arc cosine is to be returned
 * @return {Number}       the arc cosine of the given value
 *
 * @example
 * let a = PI;
 * let c = cos(a);
 * let ac = acos(c);
 * // Prints: "3.1415927 : -1.0 : 3.1415927"
 * print(a + ' : ' + c + ' : ' + ac);
 *
 * let a = PI + PI / 4.0;
 * let c = cos(a);
 * let ac = acos(c);
 * // Prints: "3.926991 : -0.70710665 : 2.3561943"
 * print(a + ' : ' + c + ' : ' + ac);
 */
exports.acos = function (ratio) {
	return _fromRadians(Math.acos(ratio));
};

/**
 * The inverse of sin(), returns the arc sine of a value. This function
 * expects the values in the range of -1 to 1 and values are returned
 * in the range -PI/2 to PI/2.
 *
 * @method asin
 * @param  {Number} value the value whose arc sine is to be returned
 * @return {Number}       the arc sine of the given value
 *
 * @example
 * let a = PI + PI / 3;
 * let s = sin(a);
 * let as = asin(s);
 * // Prints: "1.0471976 : 0.86602545 : 1.0471976"
 * print(a + ' : ' + s + ' : ' + as);
 *
 * let a = PI + PI / 3.0;
 * let s = sin(a);
 * let as = asin(s);
 * // Prints: "4.1887903 : -0.86602545 : -1.0471976"
 * print(a + ' : ' + s + ' : ' + as);
 *
 */
exports.asin = function (ratio) {
	return _fromRadians(Math.asin(ratio));
};

/**
 * The inverse of tan(), returns the arc tangent of a value. This function
 * expects the values in the range of -Infinity to Infinity (exclusive) and
 * values are returned in the range -PI/2 to PI/2.
 *
 * @method atan
 * @param  {Number} value the value whose arc tangent is to be returned
 * @return {Number}       the arc tangent of the given value
 *
 * @example
 * let a = PI + PI / 3;
 * let t = tan(a);
 * let at = atan(t);
 * // Prints: "1.0471976 : 1.7320509 : 1.0471976"
 * print(a + ' : ' + t + ' : ' + at);
 *
 * let a = PI + PI / 3.0;
 * let t = tan(a);
 * let at = atan(t);
 * // Prints: "4.1887903 : 1.7320513 : 1.0471977"
 * print(a + ' : ' + t + ' : ' + at);
 *
 */
exports.atan = function (ratio) {
	return _fromRadians(Math.atan(ratio));
};

/**
 * Calculates the angle (in radians) from a specified point to the coordinate
 * origin as measured from the positive x-axis. Values are returned as a
 * float in the range from PI to -PI. The atan2() function is most often used
 * for orienting geometry to the position of the cursor.
 * <br><br>
 * Note: The y-coordinate of the point is the first parameter, and the
 * x-coordinate is the second parameter, due the the structure of calculating
 * the tangent.
 *
 * @method atan2
 * @param  {Number} y y-coordinate of the point
 * @param  {Number} x x-coordinate of the point
 * @return {Number}   the arc tangent of the given point
 *
 * @example
 * function draw() {
 *   background(204);
 *   translate(width / 2, height / 2);
 *   let a = atan2(mouseY - height / 2, mouseX - width / 2);
 *   rotate(a);
 *   rect(-30, -5, 60, 10);
 * }
 */
exports.atan2 = function (y, x) {
	return _fromRadians(Math.atan2(y, x));
};

/**
 * Calculates the cosine of an angle. This function takes into account the
 * current angleMode. Values are returned in the range -1 to 1.
 *
 * @method cos
 * @param  {Number} angle the angle
 * @return {Number}       the cosine of the angle
 *
 * @example
 * let a = 0.0;
 * let inc = TWO_PI / 25.0;
 * for (let i = 0; i < 25; i++) {
 *   line(i * 4, 50, i * 4, 50 + cos(a) * 40.0);
 *   a = a + inc;
 * }
 */
exports.cos = function (angle) {
	return Math.cos(_toRadians(angle));
};

/**
 * Calculates the sine of an angle. This function takes into account the
 * current angleMode. Values are returned in the range -1 to 1.
 *
 * @method sin
 * @param  {Number} angle the angle
 * @return {Number}       the sine of the angle
 *
 * @example
 * let a = 0.0;
 * let inc = TWO_PI / 25.0;
 * for (let i = 0; i < 25; i++) {
 *   line(i * 4, 50, i * 4, 50 + sin(a) * 40.0);
 *   a = a + inc;
 * }
 */
exports.sin = function (angle) {
	return Math.sin(_toRadians(angle));
};

/**
 * Calculates the tangent of an angle. This function takes into account
 * the current angleMode. Values are returned in the range -1 to 1.
 *
 * @method tan
 * @param  {Number} angle the angle
 * @return {Number}       the tangent of the angle
 *
 * @example
 * let a = 0.0;
 * let inc = TWO_PI / 50.0;
 * for (let i = 0; i < 100; i = i + 2) {
 *   line(i, 50, i, 50 + tan(a) * 2.0);
 *   a = a + inc;
 * }
 */
exports.tan = function (angle) {
	return Math.tan(_toRadians(angle));
};


/**
 * Converts a radian measurement to its corresponding value in degrees.
 * Radians and degrees are two ways of measuring the same thing. There are
 * 360 degrees in a circle and 2*PI radians in a circle. For example,
 * 90° = PI/2 = 1.5707964. This function does not take into account the
 * current angleMode.
 *
 * @method degrees
 * @param  {Number} radians the radians value to convert to degrees
 * @return {Number}         the converted angle
 *
 *
 * @example
 * let rad = PI / 4;
 * let deg = degrees(rad);
 * print(rad + ' radians is ' + deg + ' degrees');
 * // Prints: 0.7853981633974483 radians is 45 degrees
 */
exports.degrees = function (angle) {
	return angle * RAD_TO_DEG;
};

/**
 * Converts a degree measurement to its corresponding value in radians.
 * Radians and degrees are two ways of measuring the same thing. There are
 * 360 degrees in a circle and 2*PI radians in a circle. For example,
 * 90° = PI/2 = 1.5707964. This function does not take into account the
 * current angleMode.
 *
 * @method radians
 * @param  {Number} degrees the degree value to convert to radians
 * @return {Number}         the converted angle
 *
 * @example
 * let deg = 45.0;
 * let rad = radians(deg);
 * print(deg + ' degrees is ' + rad + ' radians');
 * // Prints: 45 degrees is 0.7853981633974483 radians
 */
exports.radians = function (angle) {
	return angle * DEG_TO_RAD;
};

/**
 * Sets the current mode of p5 to given mode. Default mode is RADIANS.
 *
 * @method angleMode
 * @param {Constant} mode either RADIANS or DEGREES
 *
 * @example
 * function draw() {
 *   background(204);
 *   angleMode(DEGREES); // Change the mode to DEGREES
 *   let a = atan2(mouseY - height / 2, mouseX - width / 2);
 *   translate(width / 2, height / 2);
 *   push();
 *   rotate(a);
 *   rect(-20, -5, 40, 10); // Larger rectangle is rotating in degrees
 *   pop();
 *   angleMode(RADIANS); // Change the mode to RADIANS
 *   rotate(a); // variable a stays the same
 *   rect(-40, -5, 20, 10); // Smaller rectangle is rotating in radians
 * }
 */
exports.angleMode = function (mode) {
	if (mode === DEGREES || mode === RADIANS) {
		_angleMode = mode;
	}
};

/**
 * converts angles from the current angleMode to RADIANS
 *
 * @method _toRadians
 * @private
 * @param {Number} angle
 * @returns {Number}
 */
exports._toRadians = function (angle) {
	if (_angleMode === DEGREES) {
		return angle * DEG_TO_RAD;
	}
	return angle;
};

/**
 * converts angles from the current angleMode to DEGREES
 *
 * @method _toDegrees
 * @private
 * @param {Number} angle
 * @returns {Number}
 */
exports._toDegrees = function (angle) {
	if (_angleMode === RADIANS) {
		return angle * RAD_TO_DEG;
	}
	return angle;
};

/**
 * converts angles from RADIANS into the current angleMode
 *
 * @method _fromRadians
 * @private
 * @param {Number} angle
 * @returns {Number}
 */
exports._fromRadians = function (angle) {
	if (_angleMode === DEGREES) {
		return angle * RAD_TO_DEG;
	}
	return angle;
};
