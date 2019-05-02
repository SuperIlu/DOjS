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
	x = 10;
	y = 10;
	stroke(255);
	fill(128);
	beginShape();
	vertex(x + 30, y + 20);
	vertex(x + 85, y + 20);
	vertex(x + 85, y + 75);
	vertex(x + 30, y + 75);
	endShape(CLOSE);

	x = 100;
	y = 10;
	stroke(255);
	fill(128);
	beginShape(POINTS);
	vertex(x + 30, y + 20);
	vertex(x + 85, y + 20);
	vertex(x + 85, y + 75);
	vertex(x + 30, y + 75);
	endShape();

	x = 200;
	y = 10;
	stroke(255);
	fill(128);
	beginShape(LINES);
	vertex(x + 30, y + 20);
	vertex(x + 85, y + 20);
	vertex(x + 85, y + 75);
	vertex(x + 30, y + 75);
	endShape();

	x = 300;
	y = 10;
	noFill();
	beginShape();
	vertex(x + 30, y + 20);
	vertex(x + 85, y + 20);
	vertex(x + 85, y + 75);
	vertex(x + 30, y + 75);
	endShape();

	x = 400;
	y = 10;
	noFill();
	beginShape();
	vertex(x + 30, y + 20);
	vertex(x + 85, y + 20);
	vertex(x + 85, y + 75);
	vertex(x + 30, y + 75);
	endShape(CLOSE);

	x = 10;
	y = 100;
	stroke(255);
	fill(128);
	beginShape(TRIANGLES);
	vertex(x + 30, y + 75);
	vertex(x + 40, y + 20);
	vertex(x + 50, y + 75);

	vertex(x + 60, y + 20);
	vertex(x + 70, y + 75);
	vertex(x + 80, y + 20);
	endShape();

	x = 100;
	y = 100;
	stroke(255);
	fill(128);
	beginShape();
	vertex(x + 20, y + 20);
	vertex(x + 40, y + 20);
	vertex(x + 40, y + 40);
	vertex(x + 60, y + 40);
	vertex(x + 60, y + 60);
	vertex(x + 20, y + 60);
	endShape(CLOSE);
}