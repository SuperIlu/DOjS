/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

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
var PADDING = 10;

var OpEnum = {
	ADD: 1,
	SUB: 2,
	MUL: 3,
	DIV: 4,
	AND: 5,
	OR: 6,
	XOR: 7,

	unary: 8,
	NOT: 8,
	SHL: 9,
	SHR: 10,
	RIGHT: 11,
	LEFT: 12,

	NOP: 13
};

var OpTXT = ["", "+", "-", "*", "/", "&", "|", "^", "~", "<<<", ">>>", "-->", "<--"];

var s = [];

function setup() {
	createCanvas(windowWidth, windowHeight);
	var wd = 0;
	while (true) {
		var st = new Stack();
		wd += st.width + PADDING;
		if (wd > width) {
			break;
		} else {
			s.push(st);
		}
	}
	frameRate(2);
}

function draw() {
	// draw everything
	background(32);
	for (var i = 0; i < s.length; i++) {
		s[i].draw(i * (PADDING + s[i].width), PADDING, floor((height - 4 * PADDING) / textSize()));
	}

	// update everything
	for (var i = 0; i < s.length; i++) {
		if (s[i].peekOp()) {
			var op1, op2;
			switch (s[i].popOp()) {
				case OpEnum.ADD:
					op1 = s[i].popNum();
					op2 = s[i].popNum();
					s[i].pushNum(0xFFFFFF & (op1 + op2));
					break;
				case OpEnum.SUB:
					op1 = s[i].popNum();
					op2 = s[i].popNum();
					s[i].pushNum(0xFFFFFF & (op2 - op1));
					break;
				case OpEnum.MUL:
					op1 = s[i].popNum();
					op2 = s[i].popNum();
					s[i].pushNum(0xFFFFFF & (op1 * op2));
					break;
				case OpEnum.DIV:
					op1 = s[i].popNum();
					op2 = s[i].popNum();
					s[i].pushNum(0xFFFFFF & (op2 / op1));
					break;
				case OpEnum.AND:
					op1 = s[i].popNum();
					op2 = s[i].popNum();
					s[i].pushNum(0xFFFFFF & (op1 & op2));
					break;
				case OpEnum.OR:
					op1 = s[i].popNum();
					op2 = s[i].popNum();
					s[i].pushNum(0xFFFFFF & (op1 | op2));
					break;
				case OpEnum.XOR:
					op1 = s[i].popNum();
					op2 = s[i].popNum();
					s[i].pushNum(0xFFFFFF & (op1 ^ op2));
					break;
				case OpEnum.NOT:
					op1 = s[i].popNum();
					s[i].pushNum(0xFFFFFF & (~op1));
					break;
				case OpEnum.SHL:
					op1 = s[i].popNum();
					s[i].pushNum(0xFFFFFF & (op1 << 1));
					break;
				case OpEnum.SHR:
					op1 = s[i].popNum();
					s[i].pushNum(0xFFFFFF & (op1 >>> 1));
					break;
				case OpEnum.RIGHT:
					op1 = s[i].popNum();
					if (i + 1 < s.length) {
						s[i + 1].pushNum(op1);
					}
					break;
				case OpEnum.LEFT:
					op1 = s[i].popNum();
					if (i - 1 >= 0) {
						s[i - 1].pushNum(op1);
					}
					break;
				default:
					throw "illegal op"
			}
		} else {
			var todo = floor(random(1, OpEnum.NOP + 1));
			if (todo < OpEnum.unary && s[i].size() > 1 && s[i].peekNum() && s[i].peekNum(1)) {
				s[i].pushOp(todo);
			} else if (todo >= OpEnum.unary && todo < OpEnum.NOP && s[i].size() > 0 && s[i].peekNum()) {
				s[i].pushOp(todo);
			} else {
				s[i].pushNum(randomColor());
			}
		}
	}
}

function randomColor() {
	return floor(random(0, 255)) << 16 |
		floor(random(0, 255)) << 8 |
		floor(random(0, 255));
}

function randomOp() {
	return floow(random(1, OpEnum.NOP));
}

function num2color(n) {
	return color(n >> 16 & 0xFF, n >> 8 & 0xFF, n & 0xFF);
}

function num2invert(n) {
	return color(255 - (n >> 16 & 0xFF), 255 - (n >> 8 & 0xFF), 255 - (n & 0xFF));
}

function Stack() {
	this.stack = [];
	this.width = textWidth('#FFFFFF') + 2 * PADDING;
}
Stack.prototype.pushNum = function (e) {
	this.stack.push([1, e]);
}
Stack.prototype.pushOp = function (e) {
	this.stack.push([2, e]);
}
Stack.prototype.peekNum = function (i) {
	i = i || 0;
	return this.stack.length > 0 && this.stack[this.stack.length - 1 - i][0] == 1;
}
Stack.prototype.peekOp = function (i) {
	i = i || 0;
	return this.stack.length > 0 && this.stack[this.stack.length - 1 - i][0] == 2;
}
Stack.prototype.popNum = function () {
	if (!this.peekNum()) {
		throw 'TOP is not a number';
	}
	return this.stack.pop()[1];
}
Stack.prototype.popOp = function () {
	if (!this.peekOp()) {
		throw 'TOP is not an OP';
	}
	return this.stack.pop()[1];
}
Stack.prototype.size = function () {
	return this.stack.length;
}
Stack.prototype.draw = function (x, y, n) {
	textAlign(LEFT, TOP);
	for (var i = 0; i < n && i < this.stack.length; i++) {
		var txt;
		if (this.stack[this.stack.length - 1 - i][0] == 1) {
			txt = this.stack[this.stack.length - 1 - i][1].toString(16);
			while (txt.length < 6) {
				txt = '0' + txt;
			}
			txt = '#' + txt;
			noStroke();
			fill(num2color(this.stack[this.stack.length - 1 - i][1]));
			rect(x, y + i * textSize(), this.width, textSize());
			fill(num2invert(this.stack[this.stack.length - 1 - i][1]));
			text(txt, x + PADDING, y + i * textSize());
		} else {
			txt = OpTXT[this.stack[this.stack.length - 1 - i][1]];
			while (txt.length < 6) {
				txt = ' ' + txt + ' ';
			}
			fill(255, 0, 0);
			stroke(0, 0, 0);
			rect(x, y + i * textSize(), this.width, textSize());
			fill(0, 0, 0);
			text(txt, x + PADDING, y + i * textSize());
		}
	}
	stroke(255, 0, 0);
	noFill();
	rect(x, y, this.width, n * textSize() + 2 * PADDING);
}
