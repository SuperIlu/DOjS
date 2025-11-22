/*
Original code from https://s-macke.github.io/VoxelSpace/

MIT License

Copyright (c) 2019-2025 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
Include('p5');

var colormap;
var heightmap;

function setup() {
	colormap = loadImage("examples/C1W.bmp");
	heightmap = loadImage("examples/D1.bmp");
}

function draw() {
	// Call the render function with the camera parameters:
	// position, viewing angle, height, horizon line position,
	// scaling factor for the height, the largest distance,
	// screen width and the screen height parameter
	Render(new Point(0, 0), 0, 50, 120, 120, 300);
}

function emptyArray(x) {
	var ret = [];
	for (var i = 0; i < x; i++) {
		ret.push(0);
	}
	return ret;
}

function Render(p, phi, height, horizon, scale_height, distance) {
	// precalculate viewing angle parameters
	var sinphi = sin(phi);
	var cosphi = cos(phi);

	// initialize visibility array.Y position for each column on screen
	var ybuffer = emptyArray(width);
	for (var i = 0; i < width; i++) {
		ybuffer[i] = height;
	}

	// Draw from front to the back(low z coordinate to high z coordinate)
	var dz = 1.0;
	var z = 1.0;
	while (z < distance) {
		// Find line on map.This calculation corresponds to a field of view of 90°
		var pleft = new Point(
			(-cosphi * z - sinphi * z) + p.x,
			(sinphi * z - cosphi * z) + p.y);
		var pright = new Point(
			(cosphi * z - sinphi * z) + p.x,
			(-sinphi * z - cosphi * z) + p.y);

		// segment the line
		var dx = (pright.x - pleft.x) / width;
		var dy = (pright.y - pleft.y) / width;

		// Raster line and draw a vertical line for each segment
		for (var i = 0; i < width; i++) {
			var height_on_screen = (height - blue(heightmap.get(pleft.x, pleft.y))) / z * scale_height + horizon;
			//var height_on_screen = (height - heightmap.GetPixel(pleft.x, pleft.y)) / z * scale_height + horizon;
			DrawVerticalLine(i, height_on_screen, ybuffer[i], colormap.get(pleft.x, pleft.y))
			if (height_on_screen < ybuffer[i]) {
				ybuffer[i] = height_on_screen;
			}
			pleft.x += dx;
			pleft.y += dy;
		}

		// Go to next line and increase step size when you are far away
		z += dz;
		dz += 0.2;
	}
}

function DrawVerticalLine(x, yStart, yEnd, col) {
	stroke(col);
	line(x, yStart, x, yEnd);
}

function Point(x, y) {
	this.x = x;
	this.y = y;
};
