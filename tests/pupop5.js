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
}

function draw() {
	background(128);

	ellipse(0, 50, 33, 33); // Left circle

	push(); // Start a new drawing state
	strokeWeight(10);
	fill(204, 153, 0);
	ellipse(33, 50, 33, 33); // Left-middle circle

	push(); // Start another new drawing state
	stroke(0, 102, 153);
	ellipse(66, 50, 33, 33); // Right-middle circle
	pop(); // Restore previous state

	pop(); // Restore original state

	ellipse(100, 50, 33, 33); // Right circle
}
