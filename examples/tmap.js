/*
MIT License

Copyright (c) 2019-2020 Andre Seidelt <superilu@yahoo.com>

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

var colInc = 32;
var tree = {};
var fnt;

/**
 * load font and read dir tree.
 */
function setup() {
	fnt = new Font("jsboot\\fonts\\pc6x8.fnt");
	var cur = Stat('.');
	tree = buildTree("c:", "");
}

/**
 * redraw dir tree
 */
function draw() {
	background(0);
	colorMode(HSB);
	drawTree(tree, 0, 0, 0, width - 1, height - 1);
}

/**
 * draw one level of the tree.
 * 
 * @param {*} cur curent dir entry.
 * @param {*} level tree depth.
 * @param {*} x x start position of drawing area.
 * @param {*} y y start position of drawing area.
 * @param {*} w width of drawing area.
 * @param {*} h height of drawing area.
 */
function drawTree(cur, level, x, y, w, h) {
	stroke(level * colInc % 255, 255, 255);
	//fill(level * colInc % 255, 200, 128, 32);
	noFill();
	if (level % 2 == 0) {
		// horizontally
		var xPos = x;
		cur.entries.forEach(function (e) {
			var eWidth = map(e.size, 0, cur.size, 0, w);
			if (eWidth < 1) {
				// if we are to small just increment start position and don't draw anything
				xPos += eWidth;
				return;
			}
			rect(xPos, y, eWidth, h);
			if (w > 8) {
				fnt.DrawStringLeft(x + 1, yPos + 2, e.name, EGA.BLACK, NO_COLOR);
			}
			if (e.isDir) {
				drawTree(e, level + 1, xPos, y, eWidth, h);
			}
			xPos += eWidth;
		});
	} else {
		// vertically
		var yPos = y;
		cur.entries.forEach(function (e) {
			var eHeight = map(e.size, 0, cur.size, 0, h);
			if (eHeight < 1) {
				// if we are to small just increment start position and don't draw anything
				yPos += eHeight;
				return;
			}
			rect(x, yPos, w, eHeight);
			if (h > 8) {
				fnt.DrawStringLeft(x + 1, yPos + 2, e.name, EGA.BLACK, NO_COLOR);
			}
			if (e.isDir) {
				drawTree(e, level + 1, x, yPos, w, eHeight);
			}
			yPos += eHeight;
		});
	}
}

/**
 * recursively discover all directory entries and their sizes.
 * 
 * @param {*} pname path to element.
 * @param {*} dname element (directory) to discover.
 * 
 * @returns a directory tree.
 */
function buildTree(pname, dname) {
	var pname = pname + "\\" + dname;
	var ret = {
		'path': pname,
		'name': dname,
		'isDir': true,
		'entries': []
	};

	var list = List(pname);
	list.forEach(function (e) {
		if (e === "." || e === "..") {
			return;
		}
		var epath = ret.path + "\\" + e;

		var stat = Stat(epath);
		if (stat.is_directory) {
			var sub = buildTree(ret.path, e);
			ret.entries.push(sub);
		} else {
			ret.entries.push({
				'path': epath,
				'name': e,
				'isDir': false,
				'size': stat.size
			});
		}
	});

	// sum up size of all entries
	var size = 0;
	ret.entries.forEach(function (e) {
		size += e.size;
	});
	ret['size'] = size;

	// sort entries by size
	ret.entries.sort(function (a, b) { return a.size - b.size; });
	return ret;
}
