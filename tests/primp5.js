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
	background(0);

	// rect
	x = 10;
	y = 10;
	stroke(255);
	noFill();
	rect(x, y, 90, 90);

	x = 110;
	y = 10;
	fill(128);
	noStroke();
	rect(x, y, 90, 90);

	x = 210;
	y = 10;
	fill(128);
	stroke(255);
	rect(x, y, 90, 90);

	x = 310;
	y = 10;
	stroke(255, 0, 0, 128);
	noFill();
	rect(x, y, 90, 90);

	x = 410;
	y = 10;
	fill(0, 255, 0, 128);
	noStroke();
	rect(x, y, 90, 90);

	x = 510;
	y = 10;
	fill(0, 255, 0, 128);
	stroke(255, 0, 0, 128);
	rect(x, y, 90, 90);

	// circles
	x = 50;
	y = 150;
	stroke(255);
	noFill();
	circle(x, y, 40);

	x = 150;
	y = 150;
	fill(128);
	noStroke();
	circle(x, y, 40);

	x = 250;
	y = 150;
	fill(128);
	stroke(255);
	circle(x, y, 40);

	x = 350;
	y = 150;
	stroke(255, 0, 0, 128);
	noFill();
	circle(x, y, 40);

	x = 450;
	y = 150;
	fill(0, 255, 0, 128);
	noStroke();
	circle(x, y, 40);

	x = 550;
	y = 150;
	fill(0, 255, 0, 128);
	stroke(255, 0, 0, 128);
	circle(x, y, 40);

	// ellipses
	x = 50;
	y = 250;
	stroke(255);
	noFill();
	ellipse(x, y, 40, 20);

	x = 150;
	y = 250;
	fill(128);
	noStroke();
	ellipse(x, y, 40, 20);

	x = 250;
	y = 250;
	fill(128);
	stroke(255);
	ellipse(x, y, 40, 20);

	x = 350;
	y = 250;
	stroke(255, 0, 0, 128);
	noFill();
	ellipse(x, y, 40, 20);

	x = 450;
	y = 250;
	fill(0, 255, 0, 128);
	noStroke();
	ellipse(x, y, 40, 20);

	x = 550;
	y = 250;
	fill(0, 255, 0, 128);
	stroke(255, 0, 0, 128);
	ellipse(x, y, 40, 20);

	// triangle
	x = 10;
	y = 310;
	stroke(255);
	noFill();
	triangle(x, y, x + 50, y, x, y + 50);

	x = 110;
	y = 310;
	fill(128);
	noStroke();
	triangle(x, y, x + 50, y, x, y + 50);

	x = 210;
	y = 310;
	fill(128);
	stroke(255);
	triangle(x, y, x + 50, y, x, y + 50);

	x = 310;
	y = 310;
	stroke(255, 0, 0, 128);
	noFill();
	triangle(x, y, x + 50, y, x, y + 50);

	x = 410;
	y = 310;
	fill(0, 255, 0, 128);
	noStroke();
	triangle(x, y, x + 50, y, x, y + 50);

	x = 510;
	y = 310;
	fill(0, 255, 0, 128);
	stroke(255, 0, 0, 128);
	triangle(x, y, x + 50, y, x, y + 50);

}