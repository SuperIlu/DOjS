/*
MIT License

Copyright (c) 2019 Andre Seidelt <superilu@yahoo.com>

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

var PDIR = "examples/DOjS/";

var SLIDES = [
	"slide1.js",
	"slide2.js",
	"slide3.js",
	"slide4.js",
	"slide5.js",
	"slideX.js"
];

var currentSlide = null;
var slideIdx = 0;

function setup() {
	setSlide();
}

function draw() {
	currentSlide.present();
	Gc(false);
}

function keyPressed() {
	Println("key = " + key);
	Println("keyCode = " + keyCode);

	switch (keyCode >> 8) {
		case KEY.Code.KEY_RIGHT:
		case KEY.Code.KEY_DOWN:
		case KEY.Code.KEY_PGDN:
			if (slideIdx < SLIDES.length - 1) {
				slideIdx++;
				setSlide();
			}
			break;
		case KEY.Code.KEY_LEFT:
		case KEY.Code.KEY_UP:
		case KEY.Code.KEY_PGUP:
			if (slideIdx > 0) {
				slideIdx--;
				setSlide();
			}
			break;
	}
}

function setSlide() {
	currentSlide = Require(PDIR + SLIDES[slideIdx]);
	currentSlide.prepare();
}
