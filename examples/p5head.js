/* 
 PROCESSINGJS.COM HEADER ANIMATION  
 MIT License - F1lT3R/Hyper-Metrix
 Native Processing Compatible 
 */

Include('p5');

// Set number of circles
var count = 20;
// Set maximum and minimum circle size
var maxSize = 100;
var minSize = 20;
// Build float array to store circle properties
var e = [];
// Set size of dot in circle center
var ds = 2;
// Set drag switch to false
var dragging = false;
// integers showing which circle (the first index in e) that's locked, and its position in relation to the mouse
var lockedCircle;
var lockedOffsetX;
var lockedOffsetY;
// If user presses mouse...

// Set up canvas
function setup() {
	// Stroke/line/border thickness
	strokeWeight(1);
	// Initiate array with random values for circles
	for (var j = 0; j < count; j++) {
		e.push([0, 0, 0, 0, 0]);
		e[j][0] = random(width); // X 
		e[j][1] = random(height); // Y
		e[j][2] = random(minSize, maxSize); // Radius        
		e[j][3] = random(-.12, .12); // X Speed
		e[j][4] = random(-.12, .12); // Y Speed
	}
}

// Begin main draw loop (called 25 times per second)
function draw() {
	// Fill background black
	background(0);
	// Begin looping through circle array
	for (var j = 0; j < count; j++) {
		// Disable shape stroke/border
		noStroke();
		// Cache diameter and radius of current circle
		var radi = e[j][2];
		var diam = radi / 2;
		if (sq(e[j][0] - mouseX) + sq(e[j][1] - mouseY) < sq(e[j][2] / 2))
			fill(64, 187, 128, 100); // green if mouseover
		else
			fill(64, 128, 187, 100); // regular
		if ((lockedCircle == j && dragging)) {
			// Move the particle's coordinates to the mouse's position, minus its original offset
			e[j][0] = mouseX - lockedOffsetX;
			e[j][1] = mouseY - lockedOffsetY;
		}
		// Draw circle
		ellipse(e[j][0], e[j][1], radi, radi);
		// Move circle
		e[j][0] += e[j][3];
		e[j][1] += e[j][4];


		/* Wrap edges of canvas so circles leave the top
		 and re-enter the bottom, etc... */
		if (e[j][0] < -diam) {
			e[j][0] = width + diam;
		}
		if (e[j][0] > width + diam) {
			e[j][0] = -diam;
		}
		if (e[j][1] < 0 - diam) {
			e[j][1] = height + diam;
		}
		if (e[j][1] > height + diam) {
			e[j][1] = -diam;
		}

		// If current circle is selected...
		if ((lockedCircle == j && dragging)) {
			// Set fill color of center dot to white..
			fill(255, 255, 255, 255);
			// ..and set stroke color of line to green.
			stroke(128, 255, 0, 100);
		}
		else {
			// otherwise set center dot color to black.. 
			fill(0, 0, 0, 255);
			// and set line color to turquoise.
			stroke(64, 128, 128, 255);
		}

		// Loop through all circles
		for (var k = 0; k < count; k++) {
			// If the circles are close...
			if (sq(e[j][0] - e[k][0]) + sq(e[j][1] - e[k][1]) < sq(diam)) {
				// Stroke a line from current circle to adjacent circle
				line(e[j][0], e[j][1], e[k][0], e[k][1]);
			}
		}
		// Turn off stroke/border
		noStroke();
		// Draw dot in center of circle
		rect(e[j][0] - ds, e[j][1] - ds, ds * 2, ds * 2);
	}
}
