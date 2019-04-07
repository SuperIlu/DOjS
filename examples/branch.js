/**
 * This work is licensed under Attribution-ShareAlike 3.0 Unported.
 * https://creativecommons.org/licenses/by-sa/3.0/
 * 
 * Original by @reona396, http://reona396.information.jp/
 * 
 * The original source can be found here:
 * https://www.openprocessing.org/sketch/177852
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

var block_val = 10;
var tmpS;

var blocks = [];

var count = 0;

function setup() {
	size(400, 400);

	tmpS = width / block_val;

	for (var i = 0; i < block_val; i++) {
		blocks.push([]);
		for (var j = 0; j < block_val; j++) {
			blocks[i].push(new Block(i * tmpS, j * tmpS, tmpS));
		}
	}

	frameRate(2);
}

function draw() {
	background(0);

	for (var i = 0; i < block_val; i++) {
		for (var j = 0; j < block_val; j++) {
			blocks[i][j].display();
			if (j == count) {
				blocks[i][count].str_c = color(255);
			} else {
				blocks[i][j].str_c = color(0);
			}
		}
	}

	count++;
	if (count > block_val) {
		count = 0;
	}
}

function mousePressed() {
	for (var i = 0; i < block_val; i++) {
		for (var j = 0; j < block_val; j++) {
			blocks[i][j].panel_num = int(random(4));
		}
	}
}

function Block(tmpX, tmpY, tmpS) {
	this.str_c = color(0);
	this.x = tmpX;
	this.y = tmpY;
	this.s = tmpS;

	this.panel_num = int(random(4));
}

Block.prototype.display = function () {
	for (var i = 0; i < 2; i++) {
		if (this.panel_num == 0) {
		} else {
			if (i == 0) {
				strokeWeight(15);
				stroke(0, 200, 0);
			} else {
				strokeWeight(5);
				stroke(this.str_c);
			}
		}

		if (this.panel_num == 0) {
		} else if (this.panel_num == 1) {
			line(this.x, this.y, this.x, this.y + this.s);
		} else if (this.panel_num == 2) {
			line(this.x, this.y, this.x + this.s, this.y + this.s);
		} else {
			line(this.x + this.s, this.y, this.x, this.y + this.s);
		}

		if (this.panel_num == 0) {
			stroke(0, 255, 0);
			strokeWeight(5);
			fill(0);
			ellipse(this.x, this.y, 20, 20);
		}
	}
}
