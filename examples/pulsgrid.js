/*
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
if (navigator.appName === "DOjS") {
	Include("p5");
}

var NUM_GRID;
var CORE_RADIUS;
var MAX_STATE;
var DARK_COL;

var objects = [];
var grid = [];
var xStep, yStep, xStep_2, yStep_2;
var running = false;

function setup() {
	createCanvas(min(windowWidth, windowHeight), min(windowWidth, windowHeight));

	NUM_GRID = min(10, height / 20);
	print(NUM_GRID)

	xStep = width / NUM_GRID;
	yStep = height / NUM_GRID;

	CORE_RADIUS = xStep / 8;
	MAX_STATE = int(xStep / 4);
	DARK_COL = 100 - MAX_STATE * 2;

	xStep_2 = xStep / 2;
	yStep_2 = yStep / 2;

	mouseClicked();
}

function draw() {
	running = false;
	colorMode(RGB);
	background(32);

	colorMode(HSB);
	for (var i = 0; i < objects.length; i++) {
		objects[i].Draw();
	}
	for (var i = 0; i < objects.length; i++) {
		objects[i].Tick();
	}

	if (!running) {
		mouseClicked();
	}
}

function mouseClicked() {
	objects = [];
	grid = [];
	var yPos = yStep_2;
	for (var y = 0; y < NUM_GRID; y++) {
		var xPos = xStep_2;
		grid[y] = [];
		for (var x = 0; x < NUM_GRID; x++) {
			var c = new Core(xPos, yPos);
			objects.push(c);
			grid[y].push(c);
			xPos += xStep;
		}
		yPos += yStep;
	}

	for (var y = 0; y < NUM_GRID; y++) {
		for (var x = 0; x < NUM_GRID - 1; x++) {
			if (random(4) > grid[y][x].connectors.length + grid[y][x + 1].connectors.length) {
				objects.push(new Connector(grid[y][x], grid[y][x + 1]));
			}
		}
	}
	for (var x = 0; x < NUM_GRID; x++) {
		for (var y = 0; y < NUM_GRID - 1; y++) {
			if (random(5) > grid[y][x].connectors.length + grid[y + 1][x].connectors.length) {
				objects.push(new Connector(grid[y][x], grid[y + 1][x]));
			}
		}
	}

	objects[int(random(NUM_GRID * NUM_GRID))].Activate();
}

function Core(x, y) {
	this.x = x;
	this.y = y;
	this.col = random(360);
	this.bright = 0;
	this.connectors = [];
	this.state = 0;
	this.state_2 = 0;
	this.doActivate = false;
};
Core.prototype.Addconnector = function (c) {
	this.connectors.push(c)
};
Core.prototype.Draw = function () {
	noStroke();
	var bsat = this.bright + this.state_2;
	fill(color(this.col, bsat, bsat));
	circle(this.x, this.y, CORE_RADIUS + this.state);
};
Core.prototype.Activate = function () {
	if (this.state <= 0) {
		this.state = MAX_STATE;
		this.bright = DARK_COL;
		this.doActivate = true;
	}
};
Core.prototype.Tick = function () {
	if (this.state > 0) {
		running = true;
		this.state--;
		this.state_2 = this.state * 2;
		if (this.doActivate && this.state < MAX_STATE) {
			for (var i = 0; i < this.connectors.length; i++) {
				this.connectors[i].Activate(this);
			}
			this.doActivate = false;
		}
	}
};

function Connector(c1, c2) {
	this.c1 = c1;
	this.c2 = c2;
	this.state = 0;
	this.state_2 = 0;
	this.bright = 0;
	this.activationCore = null;
	c1.Addconnector(this);
	c2.Addconnector(this);
};
Connector.prototype.Draw = function () {
	strokeWeight((CORE_RADIUS + this.state) / 10);
	noFill();
	var bsat = this.bright + this.state_2;
	if (this.c1.y === this.c2.y) {
		// horizontal line
		var deltaX = abs(this.c1.x - this.c2.x);
		var delta_75 = deltaX * .75;

		stroke(color(this.c1.col, bsat, bsat, 0.3));
		line(this.c1.x, this.c1.y, this.c1.x + delta_75, this.c2.y);

		stroke(color(this.c2.col, bsat, bsat, 0.3));
		line(this.c2.x, this.c1.y, this.c2.x - delta_75, this.c2.y);
	} else {
		// vertical line
		var deltaY = abs(this.c1.y - this.c2.y);
		var delta_75 = deltaY * .75;

		stroke(color(this.c1.col, bsat, bsat, 0.3));
		line(this.c1.x, this.c1.y, this.c1.x, this.c1.y + delta_75);

		stroke(color(this.c2.col, bsat, bsat, 0.3));
		line(this.c1.x, this.c2.y, this.c2.x, this.c2.y - delta_75);
	}
};
Connector.prototype.Activate = function (core) {
	if (this.state <= 0) {
		this.state = MAX_STATE;
		this.bright = DARK_COL;
		this.activationCore = core;
	}
};
Connector.prototype.Tick = function () {
	if (this.state > 0) {
		running = true;
		this.state--;
		this.state_2 = this.state * 2;
		if ((this.activationCore != null) && (this.state < (MAX_STATE / 2))) {
			if (this.activationCore === this.c1) {
				this.c2.Activate();
			} else {
				this.c1.Activate();
			}
			this.activationCore = null;
		}
	}
};
