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

Include('p5');

function setup() {
	frameRate(10);
	rectMode(CENTER);

	test = 0;
}

function draw() {
	// matrix tx
	if (test == 0) {
		var step = frameCount % 20;
		background(200);
		// Equivalent to translate(x, y);
		applyMatrix(1, 0, 0, 1, 40 + step, 50);
		rect(0, 0, 50, 50);
	} else if (test == 1) {
		// matrix scale
		var step = frameCount % 20;
		background(200);
		translate(50, 50);
		// Equivalent to scale(x, y);
		applyMatrix(1 / step, 0, 0, 1 / step, 0, 0);
		rect(0, 0, 50, 50);
	} else if (test == 2) {
		// matrix rot
		var step = frameCount % 20;
		var angle = map(step, 0, 20, 0, TWO_PI);
		var cos_a = cos(angle);
		var sin_a = sin(angle);
		background(200);
		translate(50, 50);
		// Equivalent to rotate(angle);
		applyMatrix(cos_a, sin_a, -sin_a, cos_a, 0, 0);
		rect(0, 0, 50, 50);
	} else if (test == 3) {
		// matrix shear
		var step = frameCount % 20;
		var angle = map(step, 0, 20, -PI / 4, PI / 4);
		background(200);
		translate(50, 50);
		// equivalent to shearX(angle);
		var shear_factor = 1 / tan(PI / 2 - angle);
		applyMatrix(1, 0, shear_factor, 1, 0, 0);
		rect(0, 0, 50, 50);
	} else if (test == 4) {
		// rotate()
		resetMatrix();
		background(200);
		translate(width / 2, height / 2);

		rotate(PI / 3.0);
		rect(-26, -26, 52, 52);
	} else if (test == 5) {
		// shearX()
		resetMatrix();
		background(200);
		translate(width / 2, height / 2);

		shearX(PI / 4.0);
		rect(0, 0, 30, 30);
	} else if (test == 6) {
		// shearY()
		resetMatrix();
		background(200);
		translate(width / 2, height / 2);

		shearY(PI / 4.0);
		rect(0, 0, 30, 30);
	} else if (test == 7) {
		// scale
		resetMatrix();
		background(200);
		translate(width / 2, height / 2);

		rect(30, 20, 50, 50);
		scale(0.5, 1.3);
		rect(30, 20, 50, 50);
	} else if (test == 8) {
		// resetMatrix
		resetMatrix();
		background(200);

		translate(50, 50);
		applyMatrix(0.5, 0.5, -0.5, 0.5, 0, 0);
		rect(0, 0, 20, 20);
		// Note that the translate is also reset.
		resetMatrix();
		rect(0, 0, 20, 20);
	}
}

function keyPressed() {
	test = int(key) - CharCode('0');
	resetMatrix();
}
