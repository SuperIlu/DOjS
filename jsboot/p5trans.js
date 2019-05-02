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

// https://academo.org/demos/rotation-about-point/
// https://en.wikipedia.org/wiki/Transformation_matrix#Affine_transformations
// http://www.calcul.com/show/calculator/matrix-multiplication_;3;3;3;1?matrix1=[[%2211%22,%2212%22,%2213%22],[%2221%22,%2222%22,%2223%22],[%2231%22,%2232%22,%2233%22]]&matrix2=[[%22x%22],[%22y%22],[%221%22]]&operator=*
// https://stackoverflow.com/questions/27205018/multiply-2-matrices-in-javascript

/**
* @module p5compat
*/

/**
* make sure we have a 2x2 identity matrix.
*/
exports._ensureMatrix = function () {
	if (!_currentEnv._matrix) {
		_currentEnv._matrix = [
			[1, 0, 0],
			[0, 1, 0],
			[0, 0, 1]
		];
	}
}

exports._MaMul = function (a, b) {
	var aNumRows = a.length, aNumCols = a[0].length,
		bNumRows = b.length, bNumCols = b[0].length,
		m = new Array(aNumRows);  // initialize array of rows
	for (var r = 0; r < aNumRows; ++r) {
		m[r] = new Array(bNumCols); // initialize the current row
		for (var c = 0; c < bNumCols; ++c) {
			m[r][c] = 0;             // initialize the current cell
			for (var i = 0; i < aNumCols; ++i) {
				m[r][c] += a[r][i] * b[i][c];
			}
		}
	}
	return m;
};

/**
 * translate a point with the current matrix (if any).
 * 
 * @param {number} x point
 * @param {number} y point
 * @returns {number} the translated x coordinate.
 */
exports._transX = function (x, y) {
	if (_currentEnv._matrix) {
		return x * _currentEnv._matrix[0][0] + y * _currentEnv._matrix[0][1] + _currentEnv._matrix[0][2];
	} else {
		return x;
	}
}

/**
 * translate a point with the current matrix (if any).
 * 
 * @param {number} x point
 * @param {number} y point
 * @returns {number} the translated y coordinate.
 */
exports._transY = function (x, y) {
	if (_currentEnv._matrix) {
		return x * _currentEnv._matrix[1][0] + y * _currentEnv._matrix[1][1] + _currentEnv._matrix[1][2];
	} else {
		return y;
	}
}

/**
 * Multiplies the current matrix by the one specified through the parameters.
 * This is a powerful operation that can perform the equivalent of translate,
 * scale, shear and rotate all at once. You can learn more about transformation
 * matrices on <a href="https://en.wikipedia.org/wiki/Transformation_matrix">
 * Wikipedia.
 *
 * The naming of the arguments here follows the naming of the <a href=
 * "https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-transform">
 * WHATWG specification and corresponds to a
 * transformation matrix of the
 * form:
 *
 * > <img style="max-width: 150px" src="assets/transformation-matrix.png"
 * alt="The transformation matrix used when applyMatrix is called"/>
 *
 * @method applyMatrix
 * @param  {Number} a numbers which define the 2x3 matrix to be multiplied
 * @param  {Number} b numbers which define the 2x3 matrix to be multiplied
 * @param  {Number} c numbers which define the 2x3 matrix to be multiplied
 * @param  {Number} d numbers which define the 2x3 matrix to be multiplied
 * @param  {Number} e numbers which define the 2x3 matrix to be multiplied
 * @param  {Number} f numbers which define the 2x3 matrix to be multiplied
 * @example
 * function setup() {
 *   frameRate(10);
 *   rectMode(CENTER);
 * }
 *
 * function draw() {
 *   var step = frameCount % 20;
 *   background(200);
 *   // Equivalent to translate(x, y);
 *   applyMatrix(1, 0, 0, 1, 40 + step, 50);
 *   rect(0, 0, 50, 50);
 * }
 * 
 * function setup() {
 *   frameRate(10);
 *   rectMode(CENTER);
 * }
 *
 * function draw() {
 *   var step = frameCount % 20;
 *   background(200);
 *   translate(50, 50);
 *   // Equivalent to scale(x, y);
 *   applyMatrix(1 / step, 0, 0, 1 / step, 0, 0);
 *   rect(0, 0, 50, 50);
 * }
 * 
 * function setup() {
 *   frameRate(10);
 *   rectMode(CENTER);
 * }
 *
 * function draw() {
 *   var step = frameCount % 20;
 *   var angle = map(step, 0, 20, 0, TWO_PI);
 *   var cos_a = cos(angle);
 *   var sin_a = sin(angle);
 *   background(200);
 *   translate(50, 50);
 *   // Equivalent to rotate(angle);
 *   applyMatrix(cos_a, sin_a, -sin_a, cos_a, 0, 0);
 *   rect(0, 0, 50, 50);
 * }
 * 
 * function setup() {
 *   frameRate(10);
 *   rectMode(CENTER);
 * }
 *
 * function draw() {
 *   var step = frameCount % 20;
 *   var angle = map(step, 0, 20, -PI / 4, PI / 4);
 *   background(200);
 *   translate(50, 50);
 *   // equivalent to shearX(angle);
 *   var shear_factor = 1 / tan(PI / 2 - angle);
 *   applyMatrix(1, 0, shear_factor, 1, 0, 0);
 *   rect(0, 0, 50, 50);
 * }
 */
exports.applyMatrix = function (a, b, c, d, e, f) {
	_ensureMatrix();

	var pm = function (m) {
		for (var r = 0; r < m.length; ++r) {
			Println('  ' + m[r].join(' '));
		}
	}

	var m1 = [
		[a, c, e],
		[b, d, f],
		[0, 0, 1]
	];

	_currentEnv._matrix = _MaMul(_currentEnv._matrix, m1);
};

/**
 * Replaces the current matrix with the identity matrix.
 *
 * @method resetMatrix
 * @example
 * translate(50, 50);
 * applyMatrix(0.5, 0.5, -0.5, 0.5, 0, 0);
 * rect(0, 0, 20, 20);
 * // Note that the translate is also reset.
 * resetMatrix();
 * rect(0, 0, 20, 20);
 */
exports.resetMatrix = function () {
	_currentEnv._matrix = null;
};

/**
 * Rotates a shape the amount specified by the angle parameter. This
 * function accounts for angleMode, so angles can be entered in either
 * RADIANS or DEGREES.
 * <br><br>
 * Objects are always rotated around their relative position to the
 * origin and positive numbers rotate objects in a clockwise direction.
 * Transformations apply to everything that happens after and subsequent
 * calls to the function accumulates the effect. For example, calling
 * rotate(HALF_PI) and then rotate(HALF_PI) is the same as rotate(PI).
 * All tranformations are reset when draw() begins again.
 * <br><br>
 * Technically, rotate() multiplies the current transformation matrix
 * by a rotation matrix. This function can be further controlled by
 * the push() and pop().
 *
 * @method rotate
 * @param  {Number} angle the angle of rotation, specified in radians
 *                        or degrees, depending on current angleMode
 * @example
 * translate(width / 2, height / 2);
 * rotate(PI / 3.0);
 * rect(-26, -26, 52, 52);
 */
exports.rotate = function (angle) {
	_ensureMatrix();
	var cA = cos(angle);
	var sA = sin(angle);

	var m1 = [
		[cA, -sA, 0],
		[sA, cA, 0],
		[0, 0, 1]
	];
	_currentEnv._matrix = _MaMul(_currentEnv._matrix, m1);
};

/**
 * Shears a shape around the x-axis the amount specified by the angle
 * parameter. Angles should be specified in the current angleMode.
 * Objects are always sheared around their relative position to the origin
 * and positive numbers shear objects in a clockwise direction.
 * <br><br>
 * Transformations apply to everything that happens after and subsequent
 * calls to the function accumulates the effect. For example, calling
 * shearX(PI/2) and then shearX(PI/2) is the same as shearX(PI).
 * If shearX() is called within the draw(), the transformation is reset when
 * the loop begins again.
 * <br><br>
 * Technically, shearX() multiplies the current transformation matrix by a
 * rotation matrix. This function can be further controlled by the
 * push() and pop() functions.
 *
 * @method shearX
 * @param  {Number} angle angle of shear specified in radians or degrees,
 *                        depending on current angleMode
 * @example
 * translate(width / 4, height / 4);
 * shearX(PI / 4.0);
 * rect(0, 0, 30, 30);
 */
exports.shearX = function (angle) {
	_ensureMatrix();
	var m1 = [
		[1, tan(angle), 0],
		[0, 1, 0],
		[0, 0, 1]
	];
	_currentEnv._matrix = _MaMul(_currentEnv._matrix, m1);
};

/**
 * Shears a shape around the y-axis the amount specified by the angle
 * parameter. Angles should be specified in the current angleMode. Objects
 * are always sheared around their relative position to the origin and
 * positive numbers shear objects in a clockwise direction.
 * <br><br>
 * Transformations apply to everything that happens after and subsequent
 * calls to the function accumulates the effect. For example, calling
 * shearY(PI/2) and then shearY(PI/2) is the same as shearY(PI). If
 * shearY() is called within the draw(), the transformation is reset when
 * the loop begins again.
 * <br><br>
 * Technically, shearY() multiplies the current transformation matrix by a
 * rotation matrix. This function can be further controlled by the
 * push() and pop() functions.
 *
 * @method shearY
 * @param  {Number} angle angle of shear specified in radians or degrees,
 *                        depending on current angleMode
 * @example
 * translate(width / 4, height / 4);
 * shearY(PI / 4.0);
 * rect(0, 0, 30, 30);
 */
exports.shearY = function (angle) {
	_ensureMatrix();
	var m1 = [
		[1, 0, 0],
		[tan(angle), 1, 0],
		[0, 0, 1]
	];
	_currentEnv._matrix = _MaMul(_currentEnv._matrix, m1);
};

/**
 * Specifies an amount to displace objects within the display window.
 * The x parameter specifies left/right translation, the y parameter
 * specifies up/down translation.
 * <br><br>
 * Transformations are cumulative and apply to everything that happens after
 * and subsequent calls to the function accumulates the effect. For example,
 * calling translate(50, 0) and then translate(20, 0) is the same as
 * translate(70, 0). If translate() is called within draw(), the
 * transformation is reset when the loop begins again. This function can be
 * further controlled by using push() and pop().
 *
 * @method translate
 * @param  {Number} x left/right translation
 * @param  {Number} y up/down translation
 * @param  {Number} [z] forward/backward translation (webgl only)
 * @example
 * translate(30, 20);
 * rect(0, 0, 55, 55);
 *
 * rect(0, 0, 55, 55); // Draw rect at original 0,0
 * translate(30, 20);
 * rect(0, 0, 55, 55); // Draw rect at new 0,0
 * translate(14, 14);
 * rect(0, 0, 55, 55); // Draw rect at new 0,0
 *
 * function draw() {
 *   background(200);
 *   rectMode(CENTER);
 *   translate(width / 2, height / 2);
 *   translate(p5.Vector.fromAngle(millis() / 1000, 40));
 *   rect(0, 0, 20, 20);
 * }
 */
/**
 * @method translate
 * @param  {p5.Vector} vector the vector to translate by
 */
exports.translate = function (x, y, z) {
	_ensureMatrix();
	if (x instanceof PVector) {
		y = x.y;
		x = x.x;
	}
	var m1 = [
		[1, 0, x],
		[0, 1, y],
		[0, 0, 1]
	];
	_currentEnv._matrix = _MaMul(_currentEnv._matrix, m1);
};


/**
 * Increases or decreases the size of a shape by expanding and contracting
 * vertices. Objects always scale from their relative origin to the
 * coordinate system. Scale values are specified as decimal percentages.
 * For example, the function call scale(2.0) increases the dimension of a
 * shape by 200%.
 * <br><br>
 * Transformations apply to everything that happens after and subsequent
 * calls to the function multiply the effect. For example, calling scale(2.0)
 * and then scale(1.5) is the same as scale(3.0). If scale() is called
 * within draw(), the transformation is reset when the loop begins again.
 * <br><br>
 * Using this function with the z parameter is only available in WEBGL mode.
 * This function can be further controlled with push() and pop().
 *
 * @method scale
 * @param  {Number|p5.Vector|Number[]} s
 *                      percent to scale the object, or percentage to
 *                      scale the object in the x-axis if multiple arguments
 *                      are given
 * @param  {Number} [y] percent to scale the object in the y-axis
 * @example
 * rect(30, 20, 50, 50);
 * scale(0.5);
 * rect(30, 20, 50, 50);
 *
 * rect(30, 20, 50, 50);
 * scale(0.5, 1.3);
 * rect(30, 20, 50, 50);
 */
/**
 * @method scale
 * @param  {p5.Vector|Number[]} scales per-axis percents to scale the object
 */
exports.scale = function (x, y, z) {
	_ensureMatrix();
	// Only check for Vector argument type if Vector is available
	if (x instanceof PVector) {
		var v = x;
		x = v.x;
		y = v.y;
		z = v.z;
	} else if (x instanceof Array) {
		var rg = x;
		x = rg[0];
		y = rg[1];
		z = rg[2] || 1;
	}
	if (isNaN(y)) {
		y = z = x;
	} else if (isNaN(z)) {
		z = 1;
	}
	var m1 = [
		[x, 0, 0],
		[0, y, 0],
		[0, 0, 1]
	];
	_currentEnv._matrix = _MaMul(_currentEnv._matrix, m1);
};
