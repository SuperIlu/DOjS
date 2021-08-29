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

LoadLibrary("png");
Include('p5');
LoadLibrary('curl');

// presentation directory
var PDIR = "examples/DOjS/";

// slide index
var SLIDES = [
	"sTITLE.js",
	"sAGENDA.js",
	"sWHO.js",
	"sP5JS.js",
	"sP5ex.js",
	"sADDWAV.js",
	"sIDE.js",
	"sCnP.js",
	"sHELP.js",
	"sAPIDOC.js",
	"sLOG.js",
	"sREMLOG.js",
	"s2DGFX.js",
	"sCOLTRAN.js",
	"sTYPO.js",
	"sFILE.js",
	"sTCPIP.js",
	"s3DFX.js",
	"sFEAT.js",
	"sBYE.js"
];

var loadedSlides = {};

var currentSlide = null;
var slideIdx = 0;

var fontHeader, fontSubheader, fontText, fontSmall;

// start with first slide
function setup() {
	if (ARGS.length > 1) {
		slideIdx = ARGS[1];
	}

	setSlide();

	fontHeader = loadFont(JSBOOTPATH + "fonts/cour34b.fnt");
	fontSubheader = loadFont(JSBOOTPATH + "fonts/cour16b.fnt");
	fontText = loadFont(JSBOOTPATH + "fonts/cour16.fnt");
	fontSmall = loadFont(JSBOOTPATH + "fonts/cour12.fnt");

	SetFramerate(60);
}

var dbg = 0;

// draw current slide
function draw() {
	currentSlide.present();
	dbg++;
	if (dbg > 2) {
		Println(JSON.stringify(MemoryInfo()));
		dbg = 0;
	}
}

// switch slides
function keyPressed() {
	var key = keyCode >> 8;
	switch (key) {
		case KEY.Code.KEY_RIGHT:
		case KEY.Code.KEY_PGDN:
			if (slideIdx < SLIDES.length - 1) {
				slideIdx++;
				setSlide();
			}
			break;
		case KEY.Code.KEY_LEFT:
		case KEY.Code.KEY_PGUP:
			if (slideIdx > 0) {
				slideIdx--;
				setSlide();
			}
			break;

		case KEY.Code.KEY_F1:
			Println(JSON.stringify(MemoryInfo()));
			break;
		case KEY.Code.KEY_F2:
			Gc(true);
			break;

		default:
			if (currentSlide) {
				currentSlide.keyHook(key);
			}
			break;
	}
}

// load and prepare new slide
function setSlide() {
	if (currentSlide) {
		currentSlide.exitHook();
	}
	if (loadedSlides["s" + slideIdx]) {
		currentSlide = loadedSlides["s" + slideIdx];
		Println("Reusing slide " + SLIDES[slideIdx]);
	} else {
		currentSlide = Require(PDIR + SLIDES[slideIdx]);
		loadedSlides["s" + slideIdx] = currentSlide;
		Println("Loaded slide " + PDIR + SLIDES[slideIdx]);
	}
	currentSlide.prepare();
	Gc(true);
	Println(JSON.stringify(MemoryInfo()));
}
