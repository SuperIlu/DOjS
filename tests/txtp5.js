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
	f = loadFont("jsboot/fonts/tms18.fnt");
	textFont(f);

	stroke(255, 0, 0);
}

function draw() {
	// textAlign 1
	textAlign(RIGHT, TOP);
	text('ABCD', 150, 30);
	textAlign(CENTER, TOP);
	text('EFGH', 150, 50);
	textAlign(LEFT, TOP);
	text('IJKL', 150, 70);
	line(150, 30, 150, 90);

	// textAlign 2
	var y = 100;
	strokeWeight(1);

	textAlign(CENTER, TOP);
	text('TOP', 100, y);
	line(80, y, 120, y);
	y += 30;

	textAlign(CENTER, CENTER);
	text('CENTER', 100, y);
	line(80, y, 120, y);
	y += 30;

	textAlign(CENTER, BASELINE);
	text('BASELINE', 100, y);
	line(80, y, 120, y);
	y += 30;

	textAlign(CENTER, BOTTOM);
	text('BOTTOM', 100, y);
	line(80, y, 120, y);
	y += 30;

	// textWidth
	textAlign(LEFT, TOP);
	var aChar = 'P';
	var cWidth = textWidth(aChar) + 100;
	text(aChar, 100, y);
	line(cWidth, y, cWidth, y + textSize());
	y += 30;

	var aString = 'p5.js';
	var sWidth = textWidth(aString) + 100;
	text(aString, 100, y);
	line(sWidth, y, sWidth, y + textSize());
}
