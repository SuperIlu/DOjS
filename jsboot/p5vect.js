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
* @module p5compat.PVector
*/

/**
 * A class to describe a two or three dimensional vector, specifically
 * a Euclidean (also known as geometric) vector. A vector is an entity
 * that has both magnitude and direction. The datatype, however, stores
 * the components of the vector (x, y for 2D, and x, y, z for 3D). The magnitude
 * and direction can be accessed via the methods mag() and heading().
 * <br><br>
 * In many of the examples, you will see PVector used to describe a
 * position, velocity, or acceleration. For example, if you consider a rectangle
 * moving across the screen, at any given instant it has a position (a vector
 * that points from the origin to its location), a velocity (the rate at which
 * the object's position changes per time unit, expressed as a vector), and
 * acceleration (the rate at which the object's velocity changes per time
 * unit, expressed as a vector).
 * <br><br>
 * Since vectors represent groupings of values, we cannot simply use
 * traditional addition/multiplication/etc. Instead, we'll need to do some
 * "vector" math, which is made easy by the methods inside the PVector class.
 *
 * @class p5compat.PVector
 * @param {Number} [x] x component of the vector
 * @param {Number} [y] y component of the vector
 * @param {Number} [z] z component of the vector
 * @example
 * let v1 = createVector(40, 50);
 * let v2 = createVector(40, 50);
 *
 * ellipse(v1.x, v1.y, 50, 50);
 * ellipse(v2.x, v2.y, 50, 50);
 * v1.add(v2);
 * ellipse(v1.x, v1.y, 50, 50);
 */
exports.PVector = function PVector() {
	var x, y, z;
	x = arguments[0] || 0;
	y = arguments[1] || 0;
	z = arguments[2] || 0;
	/**
	 * The x component of the vector
	 * @property x {Number}
	 */
	this.x = x;
	/**
	 * The y component of the vector
	 * @property y {Number}
	 */
	this.y = y;
	/**
	 * The z component of the vector
	 * @property z {Number}
	 */
	this.z = z;
};

/**
 * Returns a string representation of a vector v by calling String(v)
 * or v.toString(). This method is useful for logging vectors in the
 * console.
 * @method  toString
 * @return {String}
 * @example
 * function setup() {
 *   let v = createVector(20, 30);
 *   print(String(v)); // prints "PVector Object : [20, 30, 0]"
 * }
 *
 * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(0, 0);
 *   let v1 = createVector(mouseX, mouseY);
 *   drawArrow(v0, v1, 'black');
 *
 *   noStroke();
 *   text(v1.toString(), 10, 25, 90, 75);
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.toString = function p5VectorToString() {
	return 'PVector Object : [' + this.x + ', ' + this.y + ', ' + this.z + ']';
};

/**
 * Sets the x, y, and z component of the vector using two or three separate
 * variables, the data from a PVector, or the values from a float array.
 * @method set
 * @param {Number} [x] the x component of the vector
 * @param {Number} [y] the y component of the vector
 * @param {Number} [z] the z component of the vector
 * @example
 * function setup() {
 *   let v = createVector(1, 2, 3);
 *   v.set(4, 5, 6); // Sets vector to [4, 5, 6]
 *
 *   let v1 = createVector(0, 0, 0);
 *   let arr = [1, 2, 3];
 *   v1.set(arr); // Sets vector to [1, 2, 3]
 * }
 *
 * let v0, v1;
 * function setup() {
 *   createCanvas(100, 100);
 *
 *   v0 = createVector(0, 0);
 *   v1 = createVector(50, 50);
 * }
 *
 * function draw() {
 *   background(240);
 *
 *   drawArrow(v0, v1, 'black');
 *   v1.set(v1.x + random(-1, 1), v1.y + random(-1, 1));
 *
 *   noStroke();
 *   text('x: ' + round(v1.x) + ' y: ' + round(v1.y), 20, 90);
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.set = function set(x, y, z) {
	if (x instanceof PVector) {
		this.x = x.x || 0;
		this.y = x.y || 0;
		this.z = x.z || 0;
		return this;
	}
	if (x instanceof Array) {
		this.x = x[0] || 0;
		this.y = x[1] || 0;
		this.z = x[2] || 0;
		return this;
	}
	this.x = x || 0;
	this.y = y || 0;
	this.z = z || 0;
	return this;
};

/**
 * Gets a copy of the vector, returns a PVector object.
 *
 * @method copy
 * @return {PVector} the copy of the PVector object
 * @example
 * let v1 = createVector(1, 2, 3);
 * let v2 = v1.copy();
 * print(v1.x === v2.x && v1.y === v2.y && v1.z === v2.z);
 * // Prints "true"
 */
exports.PVector.prototype.copy = function copy() {
	return new PVector(this.x, this.y, this.z);
};

/**
 * Adds x, y, and z components to a vector, adds one vector to another, or
 * adds two independent vectors together. The version of the method that adds
 * two vectors together is a static method and returns a PVector, the others
 * acts directly on the vector. See the examples for more context.
 *
 * @method add
 * @param  {Number} x   the x component of the vector to be added
 * @param  {Number} [y] the y component of the vector to be added
 * @param  {Number} [z] the z component of the vector to be added
 * @example
 * let v = createVector(1, 2, 3);
 * v.add(4, 5, 6);
 * // v's components are set to [5, 7, 9]
 * // Static method
 * let v1 = createVector(1, 2, 3);
 * let v2 = createVector(2, 3, 4);
 *
 * let v3 = PVector.add(v1, v2);
 * // v3 has components [3, 5, 7]
 * print(v3);
 *
 * // red vector + blue vector = purple vector
 * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(0, 0);
 *   let v1 = createVector(mouseX, mouseY);
 *   drawArrow(v0, v1, 'red');
 *
 *   let v2 = createVector(-30, 20);
 *   drawArrow(v1, v2, 'blue');
 *
 *   let v3 = PVector.add(v1, v2);
 *   drawArrow(v0, v3, 'purple');
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.add = function add(x, y, z) {
	if (x instanceof PVector) {
		this.x += x.x || 0;
		this.y += x.y || 0;
		this.z += x.z || 0;
		return this;
	}
	if (x instanceof Array) {
		this.x += x[0] || 0;
		this.y += x[1] || 0;
		this.z += x[2] || 0;
		return this;
	}
	this.x += x || 0;
	this.y += y || 0;
	this.z += z || 0;
	return this;
};

/**
 * Subtracts x, y, and z components from a vector, subtracts one vector from
 * another, or subtracts two independent vectors. The version of the method
 * that subtracts two vectors is a static method and returns a PVector, the
 * other acts directly on the vector. See the examples for more context.
 *
 * @method sub
 * @param  {Number} x   the x component of the vector to subtract
 * @param  {Number} [y] the y component of the vector to subtract
 * @param  {Number} [z] the z component of the vector to subtract
 * @example
 * let v = createVector(4, 5, 6);
 * v.sub(1, 1, 1);
 * // v's components are set to [3, 4, 5]
 *
 * // Static method
 * let v1 = createVector(2, 3, 4);
 * let v2 = createVector(1, 2, 3);
 *
 * let v3 = PVector.sub(v1, v2);
 * // v3 has components [1, 1, 1]
 * print(v3);
 *
 * // red vector - blue vector = purple vector
 * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(0, 0);
 *   let v1 = createVector(70, 50);
 *   drawArrow(v0, v1, 'red');
 *
 *   let v2 = createVector(mouseX, mouseY);
 *   drawArrow(v0, v2, 'blue');
 *
 *   let v3 = PVector.sub(v1, v2);
 *   drawArrow(v2, v3, 'purple');
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.sub = function sub(x, y, z) {
	if (x instanceof PVector) {
		this.x -= x.x || 0;
		this.y -= x.y || 0;
		this.z -= x.z || 0;
		return this;
	}
	if (x instanceof Array) {
		this.x -= x[0] || 0;
		this.y -= x[1] || 0;
		this.z -= x[2] || 0;
		return this;
	}
	this.x -= x || 0;
	this.y -= y || 0;
	this.z -= z || 0;
	return this;
};

/**
 * Multiply the vector by a scalar. The static version of this method
 * creates a new PVector while the non static version acts on the vector
 * directly. See the examples for more context.
 *
 * @method mult
 * @param  {Number}    n the number to multiply with the vector
 * @example
 * let v = createVector(1, 2, 3);
 * v.mult(2);
 * // v's components are set to [2, 4, 6]
 *
 * // Static method
 * let v1 = createVector(1, 2, 3);
 * let v2 = PVector.mult(v1, 2);
 * // v2 has components [2, 4, 6]
 * print(v2);
 *
 * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(50, 50);
 *   let v1 = createVector(25, -25);
 *   drawArrow(v0, v1, 'red');
 *
 *   let num = map(mouseX, 0, width, -2, 2, true);
 *   let v2 = PVector.mult(v1, num);
 *   drawArrow(v0, v2, 'blue');
 *
 *   noStroke();
 *   text('multiplied by ' + num.toFixed(2), 5, 90);
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.mult = function mult(n) {
	if (!(typeof n === 'number' && isFinite(n))) {
		Println(
			'PVector.prototype.mult:',
			'n is undefined or not a finite number'
		);
		return this;
	}
	this.x *= n;
	this.y *= n;
	this.z *= n;
	return this;
};

/**
 * Divide the vector by a scalar. The static version of this method creates a
 * new PVector while the non static version acts on the vector directly.
 * See the examples for more context.
 *
 * @method div
 * @param  {number}    n the number to divide the vector by
 * @example
 * let v = createVector(6, 4, 2);
 * v.div(2); //v's components are set to [3, 2, 1]
 *
 * // Static method
 * let v1 = createVector(6, 4, 2);
 * let v2 = PVector.div(v1, 2);
 * // v2 has components [3, 2, 1]
 * print(v2);
 *
 * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(0, 100);
 *   let v1 = createVector(50, -50);
 *   drawArrow(v0, v1, 'red');
 *
 *   let num = map(mouseX, 0, width, 10, 0.5, true);
 *   let v2 = PVector.div(v1, num);
 *   drawArrow(v0, v2, 'blue');
 *
 *   noStroke();
 *   text('divided by ' + num.toFixed(2), 10, 90);
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.div = function div(n) {
	if (!(typeof n === 'number' && isFinite(n))) {
		Println(
			'PVector.prototype.div:',
			'n is undefined or not a finite number'
		);
		return this;
	}
	if (n === 0) {
		Println('PVector.prototype.div:', 'divide by 0');
		return this;
	}
	this.x /= n;
	this.y /= n;
	this.z /= n;
	return this;
};

/**
 * Calculates the magnitude (length) of the vector and returns the result as
 * a float (this is simply the equation sqrt(x*x + y*y + z*z).)
 *
 * @method mag
 * @return {Number} magnitude of the vector
 * @example
 * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(0, 0);
 *   let v1 = createVector(mouseX, mouseY);
 *   drawArrow(v0, v1, 'black');
 *
 *   noStroke();
 *   text('vector length: ' + v1.mag().toFixed(2), 10, 70, 90, 30);
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 * * * let v = createVector(20.0, 30.0, 40.0);
 * let m = v.mag();
 * print(m); // Prints "53.85164807134504"
 */
exports.PVector.prototype.mag = function mag() {
	return Math.sqrt(this.magSq());
};

/**
 * Calculates the squared magnitude of the vector and returns the result
 * as a float (this is simply the equation <em>(x*x + y*y + z*z)</em>.)
 * Faster if the real length is not required in the
 * case of comparing vectors, etc.
 *
 * @method magSq
 * @return {number} squared magnitude of the vector
 * @example
 * // Static method
 * let v1 = createVector(6, 4, 2);
 * print(v1.magSq()); // Prints "56"
 *
 * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(0, 0);
 *   let v1 = createVector(mouseX, mouseY);
 *   drawArrow(v0, v1, 'black');
 *
 *   noStroke();
 *   text('vector length squared: ' + v1.magSq().toFixed(2), 10, 45, 90, 55);
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.magSq = function magSq() {
	var x = this.x;
	var y = this.y;
	var z = this.z;
	return x * x + y * y + z * z;
};

/**
 * Calculates the dot product of two vectors. The version of the method
 * that computes the dot product of two independent vectors is a static
 * method. See the examples for more context.
 *
 *
 * @method dot
 * @param  {Number} x   x component of the vector
 * @param  {Number} [y] y component of the vector
 * @param  {Number} [z] z component of the vector
 * @return {Number}       the dot product
 *
 * @example
 * let v1 = createVector(1, 2, 3);
 * let v2 = createVector(2, 3, 4);
 *
 * print(v1.dot(v2)); // Prints "20"
 *
 * //Static method
 * let v1 = createVector(1, 2, 3);
 * let v2 = createVector(3, 2, 1);
 * print(PVector.dot(v1, v2)); // Prints "10"
 */
/**
 * @method dot
 * @param  {PVector} value value component of the vector or a PVector
 * @return {Number}
 */
exports.PVector.prototype.dot = function dot(x, y, z) {
	if (x instanceof PVector) {
		return this.dot(x.x, x.y, x.z);
	}
	return this.x * (x || 0) + this.y * (y || 0) + this.z * (z || 0);
};

/**
 * Calculates and returns a vector composed of the cross product between
 * two vectors. Both the static and non static methods return a new PVector.
 * See the examples for more context.
 *
 * @method cross
 * @param  {PVector} v PVector to be crossed
 * @return {PVector}   PVector composed of cross product
 * @example
 * let v1 = createVector(1, 2, 3);
 * let v2 = createVector(1, 2, 3);
 *
 * v1.cross(v2); // v's components are [0, 0, 0]
 *
 * // Static method
 * let v1 = createVector(1, 0, 0);
 * let v2 = createVector(0, 1, 0);
 *
 * let crossProduct = PVector.cross(v1, v2);
 * // crossProduct has components [0, 0, 1]
 * print(crossProduct);
 */
exports.PVector.prototype.cross = function cross(v) {
	var x = this.y * v.z - this.z * v.y;
	var y = this.z * v.x - this.x * v.z;
	var z = this.x * v.y - this.y * v.x;

	return new PVector(x, y, z);
};

/**
 * Calculates the Euclidean distance between two points (considering a
 * point as a vector object).
 *
 * @method dist
 * @param  {PVector} v the x, y, and z coordinates of a PVector
 * @return {Number}      the distance
 * @example
 * let v1 = createVector(1, 0, 0);
 * let v2 = createVector(0, 1, 0);
 *
 * let distance = v1.dist(v2); // distance is 1.4142...
 * print(distance);
 *
 * // Static method
 * let v1 = createVector(1, 0, 0);
 * let v2 = createVector(0, 1, 0);
 *
 * let distance = PVector.dist(v1, v2);
 * // distance is 1.4142...
 * print(distance);
 *
 * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(0, 0);
 *
 *   let v1 = createVector(70, 50);
 *   drawArrow(v0, v1, 'red');
 *
 *   let v2 = createVector(mouseX, mouseY);
 *   drawArrow(v0, v2, 'blue');
 *
 *   noStroke();
 *   text('distance between vectors: ' + v2.dist(v1).toFixed(2), 5, 50, 95, 50);
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.dist = function dist(v) {
	return v
		.copy()
		.sub(this)
		.mag();
};

/**
 * Normalize the vector to length 1 (make it a unit vector).
 *
 * @method normalize
 * @return {PVector} normalized PVector
 * @example
 * let v = createVector(10, 20, 2);
 * // v has components [10.0, 20.0, 2.0]
 * v.normalize();
 * // v's components are set to
 * // [0.4454354, 0.8908708, 0.089087084]
 * * * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(50, 50);
 *   let v1 = createVector(mouseX - 50, mouseY - 50);
 *
 *   drawArrow(v0, v1, 'red');
 *   v1.normalize();
 *   drawArrow(v0, v1.mult(35), 'blue');
 *
 *   noFill();
 *   ellipse(50, 50, 35 * 2);
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.normalize = function normalize() {
	var len = this.mag();
	// here we multiply by the reciprocal instead of calling 'div()'
	// since div duplicates this zero check.
	if (len !== 0) this.mult(1 / len);
	return this;
};

/**
 * Limit the magnitude of this vector to the value used for the <b>max</b>
 * parameter.
 *
 * @method limit
 * @param  {Number}    max the maximum magnitude for the vector
 * @example
 * let v = createVector(10, 20, 2);
 * // v has components [10.0, 20.0, 2.0]
 * v.limit(5);
 * // v's components are set to
 * // [2.2271771, 4.4543543, 0.4454354]
 * * * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(50, 50);
 *   let v1 = createVector(mouseX - 50, mouseY - 50);
 *
 *   drawArrow(v0, v1, 'red');
 *   drawArrow(v0, v1.limit(35), 'blue');
 *
 *   noFill();
 *   ellipse(50, 50, 35 * 2);
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.limit = function limit(max) {
	var mSq = this.magSq();
	if (mSq > max * max) {
		this.div(Math.sqrt(mSq)) //normalize it
			.mult(max);
	}
	return this;
};

/**
 * Set the magnitude of this vector to the value used for the <b>len</b>
 * parameter.
 *
 * @method setMag
 * @param  {number}    len the new length for this vector
 * @example
 * let v = createVector(10, 20, 2);
 * // v has components [10.0, 20.0, 2.0]
 * v.setMag(10);
 * // v's components are set to [6.0, 8.0, 0.0]
 *
 * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(0, 0);
 *   let v1 = createVector(50, 50);
 *
 *   drawArrow(v0, v1, 'red');
 *
 *   let length = map(mouseX, 0, width, 0, 141, true);
 *   v1.setMag(length);
 *   drawArrow(v0, v1, 'blue');
 *
 *   noStroke();
 *   text('magnitude set to: ' + length.toFixed(2), 10, 70, 90, 30);
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.setMag = function setMag(n) {
	return this.normalize().mult(n);
};

/**
 * Calculate the angle of rotation for this vector (only 2D vectors)
 *
 * @method heading
 * @return {Number} the angle of rotation
 * @example
 * * function setup() {
 *   let v1 = createVector(30, 50);
 *   print(v1.heading()); // 1.0303768265243125
 *
 *   v1 = createVector(40, 50);
 *   print(v1.heading()); // 0.8960553845713439
 *
 *   v1 = createVector(30, 70);
 *   print(v1.heading()); // 1.1659045405098132
 * }
 *
 * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(50, 50);
 *   let v1 = createVector(mouseX - 50, mouseY - 50);
 *
 *   drawArrow(v0, v1, 'black');
 *
 *   let myHeading = v1.heading();
 *   noStroke();
 *   text(
 *     'vector heading: ' +
 *       myHeading.toFixed(2) +
 *       ' radians or ' +
 *       degrees(myHeading).toFixed(2) +
 *       ' degrees',
 *     10,
 *     50,
 *     90,
 *     50
 *   );
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.heading = function heading() {
	var h = Math.atan2(this.y, this.x);
	return _fromRadians(h);
};

/**
 * Rotate the vector by an angle (only 2D vectors), magnitude remains the
 * same
 *
 * @method rotate
 * @param  {number}    angle the angle of rotation
 * @example
 * let v = createVector(10.0, 20.0);
 * // v has components [10.0, 20.0, 0.0]
 * v.rotate(HALF_PI);
 * // v's components are set to [-20.0, 9.999999, 0.0]
 *
 * let angle = 0;
 * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(50, 50);
 *   let v1 = createVector(50, 0);
 *
 *   drawArrow(v0, v1.rotate(angle), 'black');
 *   angle += 0.01;
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.rotate = function rotate(a) {
	var newHeading = this.heading() + a;
	newHeading = _toRadians(newHeading);
	var mag = this.mag();
	this.x = Math.cos(newHeading) * mag;
	this.y = Math.sin(newHeading) * mag;
	return this;
};

/**
 * Calculates and returns the angle (in radians) between two vectors.
 * @method angleBetween
 * @param  {PVector}    the x, y, and z components of a PVector
 * @return {Number}       the angle between (in radians)
 * @example
 * let v1 = createVector(1, 0, 0);
 * let v2 = createVector(0, 1, 0);
 *
 * let angle = v1.angleBetween(v2);
 * // angle is PI/2
 * print(angle);
 *
 * function draw() {
 *   background(240);
 *   let v0 = createVector(50, 50);
 *
 *   let v1 = createVector(50, 0);
 *   drawArrow(v0, v1, 'red');
 *
 *   let v2 = createVector(mouseX - 50, mouseY - 50);
 *   drawArrow(v0, v2, 'blue');
 *
 *   let angleBetween = v1.angleBetween(v2);
 *   noStroke();
 *   text(
 *     'angle between: ' +
 *       angleBetween.toFixed(2) +
 *       ' radians or ' +
 *       degrees(angleBetween).toFixed(2) +
 *       ' degrees',
 *     10,
 *     50,
 *     90,
 *     50
 *   );
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.angleBetween = function angleBetween(v) {
	var dotmagmag = this.dot(v) / (this.mag() * v.mag());
	// Mathematically speaking: the dotmagmag variable will be between -1 and 1
	// inclusive. Practically though it could be slightly outside this range due
	// to floating-point rounding issues. This can make Math.acos return NaN.
	//
	// Solution: we'll clamp the value to the -1,1 range
	var angle = Math.acos(Math.min(1, Math.max(-1, dotmagmag)));
	return _fromRadians(angle);
};

/**
 * Linear interpolate the vector to another vector
 *
 * @method lerp
 * @param  {Number}    x   the x component
 * @param  {Number}    y   the y component
 * @param  {Number}    z   the z component
 * @param  {Number}    amt the amount of interpolation; some value between 0.0
 *                         (old vector) and 1.0 (new vector). 0.9 is very near
 *                         the new vector. 0.5 is halfway in between.
 * @example
 * let v = createVector(1, 1, 0);
 *
 * v.lerp(3, 3, 0, 0.5); // v now has components [2,2,0]
 *
 * let v1 = createVector(0, 0, 0);
 * let v2 = createVector(100, 100, 0);
 *
 * let v3 = PVector.lerp(v1, v2, 0.5);
 * // v3 has components [50,50,0]
 * print(v3);
 *
 * let step = 0.01;
 * let amount = 0;
 *
 * function draw() {
 *   background(240);
 *   let v0 = createVector(0, 0);
 *
 *   let v1 = createVector(mouseX, mouseY);
 *   drawArrow(v0, v1, 'red');
 *
 *   let v2 = createVector(90, 90);
 *   drawArrow(v0, v2, 'blue');
 *
 *   if (amount > 1 || amount < 0) {
 *     step *= -1;
 *   }
 *   amount += step;
 *   let v3 = PVector.lerp(v1, v2, amount);
 *
 *   drawArrow(v0, v3, 'purple');
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.prototype.lerp = function lerp(x, y, z, amt) {
	if (x instanceof PVector) {
		return this.lerp(x.x, x.y, x.z, y);
	}
	this.x += (x - this.x) * amt || 0;
	this.y += (y - this.y) * amt || 0;
	this.z += (z - this.z) * amt || 0;
	return this;
};

/**
 * Return a representation of this vector as a float array. This is only
 * for temporary use. If used in any other fashion, the contents should be
 * copied by using the PVector.copy() method to copy into your own
 * array.
 *
 * @method array
 * @return {Number[]} an Array with the 3 values
 * @example
 * * function setup() {
 *   let v = createVector(20, 30);
 *   print(v.array()); // Prints : Array [20, 30, 0]
 * }
 *
 * let v = createVector(10.0, 20.0, 30.0);
 * let f = v.array();
 * print(f[0]); // Prints "10.0"
 * print(f[1]); // Prints "20.0"
 * print(f[2]); // Prints "30.0"
 */
exports.PVector.prototype.array = function array() {
	return [this.x || 0, this.y || 0, this.z || 0];
};

/**
 * Equality check against a PVector
 *
 * @method equals
 * @param {Number} [x] the x component of the vector
 * @param {Number} [y] the y component of the vector
 * @param {Number} [z] the z component of the vector
 * @return {Boolean} whether the vectors are equals
 * @example
 * * let v1 = createVector(5, 10, 20);
 * let v2 = createVector(5, 10, 20);
 * let v3 = createVector(13, 10, 19);
 *
 * print(v1.equals(v2.x, v2.y, v2.z)); // true
 * print(v1.equals(v3.x, v3.y, v3.z)); // false
 *
 * let v1 = createVector(10.0, 20.0, 30.0);
 * let v2 = createVector(10.0, 20.0, 30.0);
 * let v3 = createVector(0.0, 0.0, 0.0);
 * print(v1.equals(v2)); // true
 * print(v1.equals(v3)); // false
 */
/**
 * @method equals
 * @param {PVector|Array} value the vector to compare
 * @return {Boolean}
 */
exports.PVector.prototype.equals = function equals(x, y, z) {
	var a, b, c;
	if (x instanceof PVector) {
		a = x.x || 0;
		b = x.y || 0;
		c = x.z || 0;
	} else if (x instanceof Array) {
		a = x[0] || 0;
		b = x[1] || 0;
		c = x[2] || 0;
	} else {
		a = x || 0;
		b = y || 0;
		c = z || 0;
	}
	return this.x === a && this.y === b && this.z === c;
};

// Static Methods

/**
 * Make a new 2D vector from an angle
 *
 * @method fromAngle
 * @static
 * @param {Number}     angle the desired angle, in radians
 * @param {Number}     [length] the length of the new vector (defaults to 1)
 * @return {PVector}       the new PVector object
 * @example
 * function draw() {
 *   background(200);
 *
 *   // Create a variable, proportional to the mouseX,
 *   // varying from 0-360, to represent an angle in degrees.
 *   angleMode(DEGREES);
 *   let myDegrees = map(mouseX, 0, width, 0, 360);
 *
 *   // Display that variable in an onscreen text.
 *   // (Note the nfc() function to truncate additional decimal places,
 *   // and the "\xB0" character for the degree symbol.)
 *   let readout = 'angle = ' + nfc(myDegrees, 1) + '\xB0';
 *   noStroke();
 *   fill(0);
 *   text(readout, 5, 15);
 *
 *   // Create a PVector using the fromAngle function,
 *   // and extract its x and y components.
 *   let v = PVector.fromAngle(radians(myDegrees), 30);
 *   let vx = v.x;
 *   let vy = v.y;
 *
 *   push();
 *   translate(width / 2, height / 2);
 *   noFill();
 *   stroke(150);
 *   line(0, 0, 30, 0);
 *   stroke(0);
 *   line(0, 0, vx, vy);
 *   pop();
 * }
 */
exports.PVector.fromAngle = function fromAngle(angle, length) {
	if (typeof length === 'undefined') {
		length = 1;
	}
	return new exports.PVector(length * Math.cos(angle), length * Math.sin(angle), 0);
};

/**
 * Make a new 3D vector from a pair of ISO spherical angles
 *
 * @method fromAngles
 * @static
 * @param {Number}     theta    the polar angle, in radians (zero is up)
 * @param {Number}     phi      the azimuthal angle, in radians
 *                               (zero is out of the screen)
 * @param {Number}     [length] the length of the new vector (defaults to 1)
 * @return {PVector}          the new PVector object
 * @example
 * * function setup() {
 *   createCanvas(100, 100, WEBGL);
 *   fill(255);
 *   noStroke();
 * }
 * function draw() {
 *   background(255);
 *
 *   let t = millis() / 1000;
 *
 *   // add three point lights
 *   pointLight(color('#f00'), PVector.fromAngles(t * 1.0, t * 1.3, 100));
 *   pointLight(color('#0f0'), PVector.fromAngles(t * 1.1, t * 1.2, 100));
 *   pointLight(color('#00f'), PVector.fromAngles(t * 1.2, t * 1.1, 100));
 *
 *   sphere(35);
 * }
 */
exports.PVector.fromAngles = function (theta, phi, length) {
	if (typeof length === 'undefined') {
		length = 1;
	}
	var cosPhi = Math.cos(phi);
	var sinPhi = Math.sin(phi);
	var cosTheta = Math.cos(theta);
	var sinTheta = Math.sin(theta);

	return new exports.PVector(
		length * sinTheta * sinPhi,
		-length * cosTheta,
		length * sinTheta * cosPhi
	);
};

/**
 * Make a new 2D unit vector from a random angle
 *
 * @method random2D
 * @static
 * @return {PVector} the new PVector object
 * @example
 * let v = PVector.random2D();
 * // May make v's attributes something like:
 * // [0.61554617, -0.51195765, 0.0] or
 * // [-0.4695841, -0.14366731, 0.0] or
 * // [0.6091097, -0.22805278, 0.0]
 * print(v);
 *
 * function setup() {
 *   frameRate(1);
 * }
 *
 * function draw() {
 *   background(240);
 *
 *   let v0 = createVector(50, 50);
 *   let v1 = PVector.random2D();
 *   drawArrow(v0, v1.mult(50), 'black');
 * }
 *
 * // draw an arrow for a vector at a given base position
 * function drawArrow(base, vec, myColor) {
 *   push();
 *   stroke(myColor);
 *   strokeWeight(3);
 *   fill(myColor);
 *   translate(base.x, base.y);
 *   line(0, 0, vec.x, vec.y);
 *   rotate(vec.heading());
 *   let arrowSize = 7;
 *   translate(vec.mag() - arrowSize, 0);
 *   triangle(0, arrowSize / 2, 0, -arrowSize / 2, arrowSize, 0);
 *   pop();
 * }
 */
exports.PVector.random2D = function random2D() {
	return this.fromAngle(Math.random() * TWO_PI);
};

/**
 * Make a new random 3D unit vector.
 *
 * @method random3D
 * @static
 * @return {PVector} the new PVector object
 * @example
 * let v = PVector.random3D();
 * // May make v's attributes something like:
 * // [0.61554617, -0.51195765, 0.599168] or
 * // [-0.4695841, -0.14366731, -0.8711202] or
 * // [0.6091097, -0.22805278, -0.7595902]
 * print(v);
 */
exports.PVector.random3D = function random3D() {
	var angle = Math.random() * constants.TWO_PI;
	var vz = Math.random() * 2 - 1;
	var vzBase = Math.sqrt(1 - vz * vz);
	var vx = vzBase * Math.cos(angle);
	var vy = vzBase * Math.sin(angle);
	return new exports.PVector(vx, vy, vz);
};

// Adds two vectors together and returns a new one.
/**
 * @method add
 * @static
 * @param  {PVector} v1 a PVector to add
 * @param  {PVector} v2 a PVector to add
 * @param  {PVector} target the vector to receive the result
 */
exports.PVector.add = function add(v1, v2, target) {
	if (!target) {
		target = v1.copy();
	} else {
		target.set(v1);
	}
	target.add(v2);
	return target;
};

/**
 * Subtracts one PVector from another and returns a new one.  The second
 * vector (v2) is subtracted from the first (v1), resulting in v1-v2.
 * 
 * @method sub
 * @static
 * @param  {PVector} v1 a PVector to subtract from
 * @param  {PVector} v2 a PVector to subtract
 * @param  {PVector} target if undefined a new vector will be created
 */
exports.PVector.sub = function sub(v1, v2, target) {
	if (!target) {
		target = v1.copy();
	} else {
		target.set(v1);
	}
	target.sub(v2);
	return target;
};

/**
 * Multiplies a vector by a scalar and returns a new vector.
 * 
 * @method mult
 * @static
 * @param  {PVector} v the vector to multiply
 * @param  {Number}  n
 * @param  {PVector} target if undefined a new vector will be created
 */
exports.PVector.mult = function mult(v, n, target) {
	if (!target) {
		target = v.copy();
	} else {
		target.set(v);
	}
	target.mult(n);
	return target;
};

/**
 * Divides a vector by a scalar and returns a new vector.
 * 
 * @method div
 * @static
 * @param  {PVector} v the vector to divide
 * @param  {Number}  n
 * @param  {PVector} target if undefined a new vector will be created
 */
exports.PVector.div = function div(v, n, target) {
	if (!target) {
		target = v.copy();
	} else {
		target.set(v);
	}
	target.div(n);
	return target;
};

/**
 * Calculates the dot product of two vectors.
 * 
 * @method dot
 * @static
 * @param  {PVector} v1 the first PVector
 * @param  {PVector} v2 the second PVector
 * @return {Number}     the dot product
 */
exports.PVector.dot = function dot(v1, v2) {
	return v1.dot(v2);
};

/**
 * Calculates the cross product of two vectors.
 * 
 * @method cross
 * @static
 * @param  {PVector} v1 the first PVector
 * @param  {PVector} v2 the second PVector
 * @return {Number}     the cross product
 */
exports.PVector.cross = function cross(v1, v2) {
	return v1.cross(v2);
};

/**
 * Calculates the Euclidean distance between two points (considering a
 * point as a vector object).
 * 
 * @method dist
 * @static
 * @param  {PVector} v1 the first PVector
 * @param  {PVector} v2 the second PVector
 * @return {Number}     the distance
 */
exports.PVector.dist = function dist(v1, v2) {
	return v1.dist(v2);
};

/**
 * Linear interpolate a vector to another vector and return the result as a
 * new vector.
 * 
 * @method lerp
 * @static
 * @param {PVector} v1
 * @param {PVector} v2
 * @param {Number} amt
 * @return {Number}      the lerped value
 */
exports.PVector.lerp = function lerp(v1, v2, amt, target) {
	if (!target) {
		target = v1.copy();
	} else {
		target.set(v1);
	}
	target.lerp(v2, amt);
	return target;
};

/**
 * @method mag
 * @param {PVector} vecT the vector to return the magnitude of
 * @return {Number}        the magnitude of vecT
 * @static
 */
exports.PVector.mag = function mag(vecT) {
	var x = vecT.x,
		y = vecT.y,
		z = vecT.z;
	var magSq = x * x + y * y + z * z;
	return Math.sqrt(magSq);
};

exports.p5 = {};
exports.p5.Vector = exports.PVector;
