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

// internal variables
exports._shapeMode = null;
exports._shape = [];

/**********************************************************************************************************************
 * 2d shapes
 */
/**
 * Draw an arc to the screen. If called with only x, y, w, h, start, and
 * stop, the arc will be drawn and filled as an open pie segment. If a mode parameter is provided, the arc
 * will be filled like an open semi-circle (OPEN) , a closed semi-circle (CHORD), or as a closed pie segment (PIE). The
 * origin may be changed with the ellipseMode() function.<br><br>
 * The arc is always drawn clockwise from wherever start falls to wherever stop falls on the ellipse.
 * Adding or subtracting TWO_PI to either angle does not change where they fall.
 * If both start and stop fall at the same place, a full ellipse will be drawn.
 *
 * @method arc
 * @param  {Number} x      x-coordinate of the arc's ellipse
 * @param  {Number} y      y-coordinate of the arc's ellipse
 * @param  {Number} w      width of the arc's ellipse by default
 * @param  {Number} h      height of the arc's ellipse by default
 * @param  {Number} start  angle to start the arc, specified in radians
 * @param  {Number} stop   angle to stop the arc, specified in radians
 * @param  {Constant} [mode] optional parameter to determine the way of drawing
 *                         the arc. either CHORD, PIE or OPEN
 *
 * @example
 * arc(50, 55, 50, 50, 0, HALF_PI);
 * noFill();
 * arc(50, 55, 60, 60, HALF_PI, PI);
 * arc(50, 55, 70, 70, PI, PI + QUARTER_PI);
 * arc(50, 55, 80, 80, PI + QUARTER_PI, TWO_PI);
 *
 * arc(50, 50, 80, 80, 0, PI + QUARTER_PI);
 *
 * arc(50, 50, 80, 80, 0, PI + QUARTER_PI, OPEN);
 *
 * arc(50, 50, 80, 80, 0, PI + QUARTER_PI, CHORD);
 *
 * arc(50, 50, 80, 80, 0, PI + QUARTER_PI, PIE);
 */
exports.arc = function (x, y, w, h, start, end, style) {
	if (w != h) {
		throw new Error("Only arcs with w==h are supported");
	}

	if (_angleMode === RADIANS) {
		var nStart = (map(start, 0, TWO_PI, 256, 0) + 256) % 256;
		var nEnd = (map(end, 0, TWO_PI, 256, 0) + 256) % 256;
	} else {
		var nStart = (map(start, 0, 360, 256, 0) + 256) % 256;
		var nEnd = (map(end, 0, 360, 256, 0) + 256) % 256;
	}
	var r = w / 2;

	if (_currentEnv._stroke != NO_COLOR) {
		if (_currentEnv._strokeWeight == 1) {
			CircleArc(x, y, r, nEnd, nStart, _currentEnv._stroke);
		} else {
			CustomCircleArc(x, y, r, nEnd, nStart, _currentEnv._strokeWeight, _currentEnv._stroke);
		}
	}
};

/**
 * Draws an ellipse (oval) to the screen. An ellipse with equal width and
 * height is a circle. By default, the first two parameters set the location,
 * and the third and fourth parameters set the shape's width and height. If
 * no height is specified, the value of width is used for both the width and
 * height. If a negative height or width is specified, the absolute value is taken.
 * The origin may be changed with the ellipseMode() function.
 *
 * @method ellipse
 * @param  {Number} x x-coordinate of the ellipse.
 * @param  {Number} y y-coordinate of the ellipse.
 * @param  {Number} w width of the ellipse.
 * @param  {Number} [h] height of the ellipse.
 * @example
 * ellipse(56, 46, 55, 55);
 */
exports.ellipse = function (x, y, w, h) {
	h = h || w;

	var x1 = x;
	var y1 = y;

	if (_currentEnv._ellipseMode === CENTER) {
		var w1 = w / 2;
		var h1 = h / 2;
	} else if (_currentEnv._ellipseMode === RADIUS) {
		var w1 = w;
		var h1 = h;
	} else if (_currentEnv._ellipseMode === CORNER) {
		x1 = x - w;
		y1 = y - h;
		var w1 = w / 2;
		var h1 = h / 2;
	} else if (_currentEnv._ellipseMode === CORNERS) {
		var w1 = (w - x) / 2;
		var h1 = (h - y) / 2;
	} else {
		Debug("Unknown ellipseMode=" + _currentEnv._ellipseMode);
		return;
	}

	var tx = _transX(x1, y1);
	var ty = _transY(x1, y1);

	if (_currentEnv._fill != NO_COLOR) {
		FilledEllipse(tx, ty, w1, h1, _currentEnv._fill);
	}
	if (_currentEnv._stroke != NO_COLOR) {
		if (_currentEnv._strokeWeight == 1) {
			Ellipse(tx, ty, w1, h1, _currentEnv._stroke);
		} else {
			CustomEllipse(tx, ty, w1, h1, _currentEnv._strokeWeight, _currentEnv._stroke);
		}
	}
};

/**
 * Draws a circle to the screen. A circle is a simple closed shape.
 * It is the set of all points in a plane that are at a given distance from a given point, the centre.
 * This function is a special case of the ellipse() function, where the width and height of the ellipse are the same.
 * Height and width of the ellipse correspond to the diameter of the circle.
 * By default, the first two parameters set the location of the centre of the circle, the third sets the diameter of the circle.
 *
 * @method circle
 * @param  {Number} x  x-coordinate of the centre of the circle.
 * @param  {Number} y  y-coordinate of the centre of the circle.
 * @param  {Number} d  diameter of the circle.
 * @example
 * // Draw a circle at location (30, 30) with a diameter of 20.
 * circle(30, 30, 20);
 */
exports.circle = function (x, y, r) {
	ellipse(x, y, r);
};

/**
 * Draws a line (a direct path between two points) to the screen. The version
 * of line() with four parameters draws the line in 2D. To color a line, use
 * the stroke() function. A line cannot be filled, therefore the fill()
 * function will not affect the color of a line. 2D lines are drawn with a
 * width of one pixel by default, but this can be changed with the
 * strokeWeight() function.
 *
 * @method line
 * @param  {Number} x1 the x-coordinate of the first point
 * @param  {Number} y1 the y-coordinate of the first point
 * @param  {Number} x2 the x-coordinate of the second point
 * @param  {Number} y2 the y-coordinate of the second point
 * @example
 * line(30, 20, 85, 75);
 *
 * line(30, 20, 85, 20);
 * stroke(126);
 * line(85, 20, 85, 75);
 * stroke(255);
 * line(85, 75, 30, 75);
 */
exports.line = function (x1, y1, x2, y2) {
	if (_currentEnv._stroke != NO_COLOR) {
		var tx1 = _transX(x1, y1);
		var ty1 = _transY(x1, y1);
		var tx2 = _transX(x2, y2);
		var ty2 = _transY(x2, y2);

		if (_currentEnv._strokeWeight == 1) {
			Line(tx1, ty1, tx2, ty2, _currentEnv._stroke);
		} else {
			CustomLine(tx1, ty1, tx2, ty2, _currentEnv._strokeWeight, _currentEnv._stroke);
		}
	}
};

/**
 * Draws a point, a coordinate in space at the dimension of one pixel.
 * The first parameter is the horizontal value for the point, the second
 * value is the vertical value for the point. The color of the point is
 * determined by the current stroke.
 *
 * @method point
 * @param  {Number} x the x-coordinate
 * @param  {Number} y the y-coordinate
 * @param  {Number} [z] the z-coordinate (for WebGL mode)
 * @example
 * point(30, 20);
 * point(85, 20);
 * point(85, 75);
 * point(30, 75);
 */
exports.point = function (x, y) {
	if (_currentEnv._stroke != NO_COLOR) {
		Plot(_transX(x, y), _transY(x, y), _currentEnv._stroke);
	}
};

/**
 * Draw a quad. A quad is a quadrilateral, a four sided polygon. It is
 * similar to a rectangle, but the angles between its edges are not
 * constrained to ninety degrees. The first pair of parameters (x1,y1)
 * sets the first vertex and the subsequent pairs should proceed
 * clockwise or counter-clockwise around the defined shape.
 *
 * @method quad
 * @param {Number} x1 the x-coordinate of the first point
 * @param {Number} y1 the y-coordinate of the first point
 * @param {Number} x2 the x-coordinate of the second point
 * @param {Number} y2 the y-coordinate of the second point
 * @param {Number} x3 the x-coordinate of the third point
 * @param {Number} y3 the y-coordinate of the third point
 * @param {Number} x4 the x-coordinate of the fourth point
 * @param {Number} y4 the y-coordinate of the fourth point
 * @example
 * quad(38, 31, 86, 20, 69, 63, 30, 76);
 */
exports.quad = function (x1, y1, x2, y2, x3, y3, x4, y4) {
	beginShape();
	vertex(x1, y1);
	vertex(x2, y2);
	vertex(x3, y3);
	vertex(x4, y4);
	vertex(x1, y1);
	endShape(CLOSE);
};

/**
 * Draws a rectangle to the screen. A rectangle is a four-sided shape with
 * every angle at ninety degrees. By default, the first two parameters set
 * the location of the upper-left corner, the third sets the width, and the
 * fourth sets the height. The way these parameters are interpreted, however,
 * may be changed with the rectMode() function.
 * <br><br>
 * The fifth, sixth, seventh and eighth parameters, if specified,
 * determine corner radius for the top-left, top-right, lower-right and
 * lower-left corners, respectively. An omitted corner radius parameter is set
 * to the value of the previously specified radius value in the parameter list.
 *
 * @method rect
 * @param  {Number} x  x-coordinate of the rectangle.
 * @param  {Number} y  y-coordinate of the rectangle.
 * @param  {Number} w  width of the rectangle.
 * @param  {Number} h  height of the rectangle.
 * @example
 * // Draw a rectangle at location (30, 20) with a width and height of 55.
 * rect(30, 20, 55, 55);
 */
exports.rect = function (x, y, w, h) {
	if (_currentEnv._matrix || _currentEnv._strokeWeight > 1) {
		beginShape();
		if (_currentEnv._rectMode === CORNER) {
			vertex(x, y);
			vertex(x + w, y);
			vertex(x + w, y + h);
			vertex(x, y + h);
		} else if (_currentEnv._rectMode === CORNERS) {
			vertex(x, y);
			vertex(w, y);
			vertex(w, h);
			vertex(x, h);
		} else if (_currentEnv._rectMode === CENTER) {
			var wh = w / 2;
			var hh = h / 2;
			vertex(x - wh, y - hh);
			vertex(x + wh, y - hh);
			vertex(x + wh, y + hh);
			vertex(x - wh, y + hh);
		} else if (_currentEnv._rectMode === RADIUS) {
			vertex(x - w, y - h);
			vertex(x + w, y - h);
			vertex(x + w, y + h);
			vertex(x - w, y + h);
		} else {
			Debug("Unknown rectMode=" + _currentEnv._rectMode);
			return;
		}
		endShape(CLOSE);
	} else {
		var x1 = x;
		var y1 = y;

		if (_currentEnv._rectMode === CORNER) {
			var x2 = x + w;
			var y2 = y + h;
		} else if (_currentEnv._rectMode === CORNERS) {
			var x2 = w;
			var y2 = h;
		} else if (_currentEnv._rectMode === CENTER) {
			var wh = w / 2;
			var hh = h / 2;
			x1 = x - wh;
			y1 = y - hh;
			var x2 = x + wh;
			var y2 = y + hh;
		} else if (_currentEnv._rectMode === RADIUS) {
			x1 = x - w;
			y1 = y - h;
			var x2 = x + w;
			var y2 = y + h;
		} else {
			Debug("Unknown rectMode=" + _currentEnv._rectMode);
			return;
		}

		if (_currentEnv._fill != NO_COLOR) {
			FilledBox(x1, y1, x2, y2, _currentEnv._fill);
		}
		if (_currentEnv._stroke != NO_COLOR) {
			if (_currentEnv._strokeWeight == 1) {
				Box(x1, y1, x2, y2, _currentEnv._stroke);
			}
		}
	}
};

/**
 * Draws a square to the screen. A square is a four-sided shape with
 * every angle at ninety degrees, and equal side size.
 * This function is a special case of the rect() function, where the width and height are the same, and the parameter is called "s" for side size.
 * By default, the first two parameters set the location of the upper-left corner, the third sets the side size of the square.
 * The way these parameters are interpreted, however,
 * may be changed with the rectMode() function.
 * <br><br>
 * The fourth, fifth, sixth and seventh parameters, if specified,
 * determine corner radius for the top-left, top-right, lower-right and
 * lower-left corners, respectively. An omitted corner radius parameter is set
 * to the value of the previously specified radius value in the parameter list.
 *
 * @method square
 * @param  {Number} x  x-coordinate of the square.
 * @param  {Number} y  y-coordinate of the square.
 * @param  {Number} s  side size of the square.
 * @example
 * // Draw a square at location (30, 20) with a side size of 55.
 * square(30, 20, 55);
 */
exports.square = function (x, y, s) {
	rect(x, y, s, s);
};

/**
 * A triangle is a plane created by connecting three points. The first two
 * arguments specify the first point, the middle two arguments specify the
 * second point, and the last two arguments specify the third point.
 *
 * @method triangle
 * @param  {Number} x1 x-coordinate of the first point
 * @param  {Number} y1 y-coordinate of the first point
 * @param  {Number} x2 x-coordinate of the second point
 * @param  {Number} y2 y-coordinate of the second point
 * @param  {Number} x3 x-coordinate of the third point
 * @param  {Number} y3 y-coordinate of the third point
 * @example
 * triangle(30, 75, 58, 20, 86, 75);
 */
exports.triangle = function (x1, y1, x2, y2, x3, y3) {
	beginShape(TRIANGLES);
	vertex(x1, y1);
	vertex(x2, y2);
	vertex(x3, y3);
	endShape();
};

/**
 * Using the beginShape() and endShape() functions allow creating more
 * complex forms. beginShape() begins recording vertices for a shape and
 * endShape() stops recording. The value of the kind parameter tells it which
 * types of shapes to create from the provided vertices. With no mode
 * specified, the shape can be any irregular polygon.
 * <br><br>
 * The parameters available for beginShape() are POINTS, LINES, TRIANGLES,
 * TRIANGLE_FAN, TRIANGLE_STRIP, QUADS, and QUAD_STRIP. After calling the
 * beginShape() function, a series of vertex() commands must follow. To stop
 * drawing the shape, call endShape(). Each shape will be outlined with the
 * current stroke color and filled with the fill color.
 * <br><br>
 * Transformations such as translate(), rotate(), and scale() do not work
 * within beginShape(). It is also not possible to use other shapes, such as
 * ellipse() or rect() within beginShape().
 *
 * @method beginShape
 * @param  {Constant} [kind] either POINTS, LINES, TRIANGLES, TRIANGLE_FAN
 *                                TRIANGLE_STRIP, QUADS, or QUAD_STRIP
 * @example
 * beginShape();
 * vertex(30, 20);
 * vertex(85, 20);
 * vertex(85, 75);
 * vertex(30, 75);
 * endShape(CLOSE);
 *
 * beginShape(POINTS);
 * vertex(30, 20);
 * vertex(85, 20);
 * vertex(85, 75);
 * vertex(30, 75);
 * endShape();
 *
 * beginShape(LINES);
 * vertex(30, 20);
 * vertex(85, 20);
 * vertex(85, 75);
 * vertex(30, 75);
 * endShape();
 *
 * noFill();
 * beginShape();
 * vertex(30, 20);
 * vertex(85, 20);
 * vertex(85, 75);
 * vertex(30, 75);
 * endShape();
 *
 * noFill();
 * beginShape();
 * vertex(30, 20);
 * vertex(85, 20);
 * vertex(85, 75);
 * vertex(30, 75);
 * endShape(CLOSE);
 *
 * beginShape(TRIANGLES);
 * vertex(30, 75);
 * vertex(40, 20);
 * vertex(50, 75);
 * vertex(60, 20);
 * vertex(70, 75);
 * vertex(80, 20);
 * endShape();
 *
 * beginShape();
 * vertex(20, 20);
 * vertex(40, 20);
 * vertex(40, 40);
 * vertex(60, 40);
 * vertex(60, 60);
 * vertex(20, 60);
 * endShape(CLOSE);
 */
exports.beginShape = function (m) {
	_shapeMode = m;
	_shape = [];
};

/**
 * All shapes are constructed by connecting a series of vertices. vertex()
 * is used to specify the vertex coordinates for points, lines, triangles,
 * quads, and polygons. It is used exclusively within the beginShape() and
 * endShape() functions.
 *
 * @method vertex
 * @param  {Number} x x-coordinate of the vertex
 * @param  {Number} y y-coordinate of the vertex
 * @example
 * strokeWeight(3);
 * beginShape(POINTS);
 * vertex(30, 20);
 * vertex(85, 20);
 * vertex(85, 75);
 * vertex(30, 75);
 * endShape();
 */
exports.vertex = function (x, y) {
	_shape.push([_transX(x, y), _transY(x, y)]);
};

/**
 * The endShape() function is the companion to beginShape() and may only be
 * called after beginShape(). When endshape() is called, all of image data
 * defined since the previous call to beginShape() is written into the image
 * buffer. The constant CLOSE as the value for the MODE parameter to close
 * the shape (to connect the beginning and the end).
 *
 * @method endShape
 * @param  {Constant} [mode] use CLOSE to close the shape
 * @example
 * noFill();
 *
 * beginShape();
 * vertex(20, 20);
 * vertex(45, 20);
 * vertex(45, 80);
 * endShape(CLOSE);
 *
 * beginShape();
 * vertex(50, 20);
 * vertex(75, 20);
 * vertex(75, 80);
 * endShape();
 * 
 * TODO: QUADS, QUAD_STRIP, TRIANGLE_STRIP
 */
exports.endShape = function (p) {
	if (_shapeMode === POINTS) {
		_shape.forEach(function (p) {
			Plot(p[0], p[1], _currentEnv._stroke);
		});
	} else if (_shapeMode === LINES) {
		for (var i = 0; i < _shape.length; i += 2) {
			if (_currentEnv._strokeWeight == 1) {
				Line(_shape[i][0], _shape[i][1], _shape[i + 1][0], _shape[i + 1][1], _currentEnv._stroke);
			} else {
				CustomLine(_shape[i][0], _shape[i][1], _shape[i + 1][0], _shape[i + 1][1], _strokeWeight, _currentEnv._stroke);
			}
		}
	} else if (_shapeMode === TRIANGLES) {
		for (var i = 0; i < _shape.length; i += 3) {
			var tri = [_shape[i], _shape[i + 1], _shape[i + 2]];
			if (_currentEnv._fill != NO_COLOR) {
				FilledPolygon(tri, _currentEnv._fill);
			}
			if (_currentEnv._stroke != NO_COLOR) {
				_PolyLine(tri, true);
			}
		}
	} else {
		if (_currentEnv._fill != NO_COLOR) {
			FilledPolygon(_shape, _currentEnv._fill);
		}
		if (_currentEnv._stroke != NO_COLOR) {
			_PolyLine(_shape, p === CLOSE);
		}
	}
};

/**
 * draw polygon by using lines.
 */
exports._PolyLine = function (shape, close) {
	for (var i = 0; i < shape.length - 1; i++) {
		if (_currentEnv._strokeWeight == 1) {
			Line(shape[i][0], shape[i][1], shape[i + 1][0], shape[i + 1][1], _currentEnv._stroke);
		} else {
			CustomLine(shape[i][0], shape[i][1], shape[i + 1][0], shape[i + 1][1], _currentEnv._strokeWeight, _currentEnv._stroke);
		}
	}
	if (close) {
		var last = shape.length - 1;
		if (_currentEnv._strokeWeight == 1) {
			Line(shape[0][0], shape[0][1], shape[last][0], shape[last][1], _currentEnv._stroke);
		} else {
			CustomLine(shape[0][0], shape[0][1], shape[last][0], shape[last][1], _currentEnv._strokeWeight, _currentEnv._stroke);
		}
	}
}

/**
 * Modifies the location from which rectangles are drawn by changing the way
 * in which parameters given to rect() are interpreted.
 * <br><br>
 * The default mode is rectMode(CORNER), which interprets the first two
 * parameters of rect() as the upper-left corner of the shape, while the
 * third and fourth parameters are its width and height.
 * <br><br>
 * rectMode(CORNERS) interprets the first two parameters of rect() as the
 * location of one corner, and the third and fourth parameters as the
 * location of the opposite corner.
 * <br><br>
 * rectMode(CENTER) interprets the first two parameters of rect() as the
 * shape's center point, while the third and fourth parameters are its
 * width and height.
 * <br><br>
 * rectMode(RADIUS) also uses the first two parameters of rect() as the
 * shape's center point, but uses the third and fourth parameters to specify
 * half of the shapes's width and height.
 * <br><br>
 * The parameter must be written in ALL CAPS because Javascript is a
 * case-sensitive language.
 *
 * @method rectMode
 * @param  {Constant} mode either CORNER, CORNERS, CENTER, or RADIUS
 * @example
 * rectMode(CORNER); // Default rectMode is CORNER
 * fill(255); // Set fill to white
 * rect(25, 25, 50, 50); // Draw white rect using CORNER mode
 *
 * rectMode(CORNERS); // Set rectMode to CORNERS
 * fill(100); // Set fill to gray
 * rect(25, 25, 50, 50); // Draw gray rect using CORNERS mode
 *
 * rectMode(RADIUS); // Set rectMode to RADIUS
 * fill(255); // Set fill to white
 * rect(50, 50, 30, 30); // Draw white rect using RADIUS mode
 *
 * rectMode(CENTER); // Set rectMode to CENTER
 * fill(100); // Set fill to gray
 * rect(50, 50, 30, 30); // Draw gray rect using CENTER mode
 */
exports.rectMode = function (m) {
	if (
		m === CORNER ||
		m === CORNERS ||
		m === RADIUS ||
		m === CENTER
	) {
		_currentEnv._rectMode = m;
	}
	return this;
};


/**
 * Modifies the location from which ellipses are drawn by changing the way
 * in which parameters given to ellipse() are interpreted.
 * <br><br>
 * The default mode is ellipseMode(CENTER), which interprets the first two
 * parameters of ellipse() as the shape's center point, while the third and
 * fourth parameters are its width and height.
 * <br><br>
 * ellipseMode(RADIUS) also uses the first two parameters of ellipse() as
 * the shape's center point, but uses the third and fourth parameters to
 * specify half of the shapes's width and height.
 * <br><br>
 * ellipseMode(CORNER) interprets the first two parameters of ellipse() as
 * the upper-left corner of the shape, while the third and fourth parameters
 * are its width and height.
 * <br><br>
 * ellipseMode(CORNERS) interprets the first two parameters of ellipse() as
 * the location of one corner of the ellipse's bounding box, and the third
 * and fourth parameters as the location of the opposite corner.
 * <br><br>
 * The parameter must be written in ALL CAPS because Javascript is a
 * case-sensitive language.
 *
 * @method ellipseMode
 * @param  {Constant} mode either CENTER, RADIUS, CORNER, or CORNERS
 * @example
 * ellipseMode(RADIUS); // Set ellipseMode to RADIUS
 * fill(255); // Set fill to white
 * ellipse(50, 50, 30, 30); // Draw white ellipse using RADIUS mode
 *
 * ellipseMode(CENTER); // Set ellipseMode to CENTER
 * fill(100); // Set fill to gray
 * ellipse(50, 50, 30, 30); // Draw gray ellipse using CENTER mode
 *
 * ellipseMode(CORNER); // Set ellipseMode is CORNER
 * fill(255); // Set fill to white
 * ellipse(25, 25, 50, 50); // Draw white ellipse using CORNER mode
 *
 * ellipseMode(CORNERS); // Set ellipseMode to CORNERS
 * fill(100); // Set fill to gray
 * ellipse(25, 25, 50, 50); // Draw gray ellipse using CORNERS mode
 */
exports.ellipseMode = function (m) {
	if (
		m === CORNER ||
		m === CORNERS ||
		m === RADIUS ||
		m === CENTER
	) {
		_currentEnv._ellipseMode = m;
	}
	return this;
};

/**********************************************************************************************************************
 * images
 */

/**
 * Loads an image from a path and creates a Image from it.
 * <br><br>
 *
 * @method loadImage
 * @param  {String} path Path of the image to be loaded
 * @return {Bitmap}             the Image object
 * @example
 * let img;
 * function setup() {
 *   img = loadImage('assets/laDefense.jpg');
 *   image(img, 0, 0);
 * }
 */
exports.loadImage = function (path) {
	var ret = function (p) {
		this.bm = new Bitmap(p);
		this.width = this.bm.width;
		this.height = this.bm.height;
	};
	ret.prototype.loadPixels = function () { };
	ret.prototype.get = function (x, y) {	// TODO: check!
		var px = this.bm.GetPixel(x, y);
		return color(GetRed(px), GetGreen(px), GetBlue(px), 255);
	};

	return new ret(path);
};


/**
 * Creates a new <a href="#/p5.Image">p5.Image</a> (the datatype for storing images). This provides a
 * fresh buffer of pixels to play with. Set the size of the buffer with the
 * width and height parameters.
 * <br><br>
 * .<a href="#/p5.Image/pixels">pixels</a> gives access to an array containing the values for all the pixels
 * in the display window.
 * These values are numbers. This array is the size (including an appropriate
 * factor for the <a href="#/p5/pixelDensity">pixelDensity</a>) of the display window x4,
 * representing the R, G, B, A values in order for each pixel, moving from
 * left to right across each row, then down each column. See .<a href="#/p5.Image/pixels">pixels</a> for
 * more info. It may also be simpler to use <a href="#/p5.Image/set">set()</a> or <a href="#/p5.Image/get">get()</a>.
 * <br><br>
 * Before accessing the pixels of an image, the data must loaded with the
 * <a href="#/p5.Image/loadPixels">loadPixels()</a> function. After the array data has been modified, the
 * <a href="#/p5.Image/updatePixels">updatePixels()</a> function must be run to update the changes.
 *
 * @method createImage
 * @param  {Number} width  width in pixels
 * @param  {Number} height height in pixels
 * @return {Bitmap}       the Image object
 * @example
 * let img = createImage(66, 66);
 * img.loadPixels();
 * for (let i = 0; i < img.width; i++) {
 *   for (let j = 0; j < img.height; j++) {
 *     img.set(i, j, color(0, 90, 102));
 *   }
 * }
 * img.updatePixels();
 * image(img, 17, 17);
 *
 * let img = createImage(66, 66);
 * img.loadPixels();
 * for (let i = 0; i < img.width; i++) {
 *   for (let j = 0; j < img.height; j++) {
 *     img.set(i, j, color(0, 90, 102, (i % img.width) * 2));
 *   }
 * }
 * img.updatePixels();
 * image(img, 17, 17);
 * image(img, 34, 34);
 *
 * let pink = color(255, 102, 204);
 * let img = createImage(66, 66);
 * img.loadPixels();
 * let d = pixelDensity();
 * let halfImage = 4 * (img.width * d) * (img.height / 2 * d);
 * for (let i = 0; i < halfImage; i += 4) {
 *   img.pixels[i] = red(pink);
 *   img.pixels[i + 1] = green(pink);
 *   img.pixels[i + 2] = blue(pink);
 *   img.pixels[i + 3] = alpha(pink);
 * }
 * img.updatePixels();
 * image(img, 17, 17);
 */
exports.createImage = function (width, height) {
	var ret = function (p) {
		this.bm = new Bitmap(width, height);
		this.width = this.bm.width;
		this.height = this.bm.height;
	};
	ret.prototype.loadPixels = function () { };
	ret.prototype.get = function (x, y) {	// TODO: check!
		var px = this.bm.GetPixel(x, y);
		return color(GetRed(px), GetGreen(px), GetBlue(px), 255);
	};

	return new ret(path);
};

/**
 * Draw an image to the p5.js canvas.
 *
 * This function can be used with different numbers of parameters. The
 * simplest use requires only three parameters: img, x, and y—where (x, y) is
 * the position of the image. Two more parameters can optionally be added to
 * specify the width and height of the image.
 *
 * This function can also be used with all eight Number parameters. To
 * differentiate between all these parameters, p5.js uses the language of
 * "destination rectangle" (which corresponds to "dx", "dy", etc.) and "source
 * image" (which corresponds to "sx", "sy", etc.) below. Specifying the
 * "source image" dimensions can be useful when you want to display a
 * subsection of the source image instead of the whole thing. 
 *
 * @method image
 * @param  {Bitmap} img    the image to display
 * @param  {Number}   x     the x-coordinate of the top-left corner of the image
 * @param  {Number}   y     the y-coordinate of the top-left corner of the image
 * @example
 * let img;
 * function setup() {
 *   img = loadImage('assets/laDefense.jpg');
 *   // Top-left corner of the img is at (0, 0)
 *   // Width and height are the img's original width and height
 *   image(img, 0, 0);
 * }
 */
exports.image = function (img, x, y) {
	var x1 = x;
	var y1 = y;
	var w = img.bm.width;
	var h = img.bm.height;

	if (_currentEnv._imageMode === CORNER) {
	} else if (_currentEnv._imageMode === CORNERS) {
	} else if (_currentEnv._imageMode === CENTER) {
		var wh = w / 2;
		var hh = h / 2;
		x1 = x - wh;
		y1 = y - hh;
	} else {
		Debug("Unknown imageMode=" + _currentEnv._imageMode);
		return;
	}

	img.bm.Draw(_transX(x1, y1), _transY(x1, y1));
};


/**
 * Set image mode. Modifies the location from which images are drawn by
 * changing the way in which parameters given to image() are interpreted.
 * The default mode is imageMode(CORNER), which interprets the second and
 * third parameters of image() as the upper-left corner of the image. If
 * two additional parameters are specified, they are used to set the image's
 * width and height.
 * <br><br>
 * imageMode(CORNERS) interprets the second and third parameters of image()
 * as the location of one corner, and the fourth and fifth parameters as the
 * opposite corner.
 * <br><br>
 * imageMode(CENTER) interprets the second and third parameters of image()
 * as the image's center point. If two additional parameters are specified,
 * they are used to set the image's width and height.
 *
 * @method imageMode
 * @param {Constant} mode either CORNER, CORNERS, or CENTER
 * @example
 *
 * let img;
 * function setup() {
 *   img = loadImage('assets/bricks.jpg');
 *   imageMode(CORNER);
 *   image(img, 10, 10, 50, 50);
 * }
 *
 * let img;
 * function setup() {
 *   img = loadImage('assets/bricks.jpg');
 *   imageMode(CORNERS);
 *   image(img, 10, 10, 90, 40);
 * }
 *
 * let img;
 * function setup() {
 *   img = loadImage('assets/bricks.jpg');
 *   imageMode(CENTER);
 *   image(img, 50, 50, 80, 80);
 * }
 */
exports.imageMode = function (m) {
	if (
		m === CORNER ||
		m === CORNERS ||
		m === CENTER
	) {
		_currentEnv._imageMode = m;
	}
};

/**
 * Sets the width of the stroke used for lines, points, and the border
 * around shapes. All widths are set in units of pixels.
 *
 * @method strokeWeight
 * @param  {Number} weight the weight (in pixels) of the stroke
 * @example
 * strokeWeight(1); // Default
 * line(20, 20, 80, 20);
 * strokeWeight(4); // Thicker
 * line(20, 40, 80, 40);
 * strokeWeight(10); // Beastly
 * line(20, 70, 80, 70);
 */
exports.strokeWeight = function (w) {
	_currentEnv._strokeWeight = w;
};
