/**
 * This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * http://creativecommons.org/licenses/by-nc-sa/4.0/
 * 
 * The original source can be found here:
 * https://p5js.org/examples/math-additive-wave.html
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
/**
 * Additive Wave
 * by Daniel Shiffman. 
 * 
 * Create a more complex wave by adding two waves together. 
 */
if (navigator.appName === "DOjS") {
    Include("p5");
}

xspacing = 8;   // How far apart should each horizontal location be spaced
w = 0;          // Width of entire wave
maxwaves = 4;   // total # of waves to add together

theta = 0.0;
amplitude = new Array(maxwaves);   // Height of wave
dx = new Array(maxwaves);          // Value for incrementing X, to be calculated as a function of period and xspacing
yvalues = 0;                       // Using an array to store height values for the wave (not entirely necessary)

function setup() {
    frameRate(60);
    colorMode(RGB, 255, 255, 255, 100);
    w = width + 16;

    for (var i = 0; i < maxwaves; i++) {
        amplitude[i] = (random(10, 30));
        var period = random(100, 300); // How many pixels before the wave repeats
        dx[i] = ((Math.PI * 2 / period) * xspacing);
    }

    yvalues = new Array(w / xspacing);
}

function draw() {
    background(0);
    calcWave();
    renderWave();
}

function calcWave() {
    // Increment theta (try different values for 'angular velocity' here
    theta += 0.02;

    // Set all height values to zero
    for (var i = 0; i < yvalues.length; i++) {
        yvalues[i] = 0;
    }

    // Accumulate wave height values
    for (var j = 0; j < maxwaves; j++) {
        var x = theta;
        for (var i = 0; i < yvalues.length; i++) {
            // Every other wave is cosine instead of sine
            if (j % 2 == 0) {
                yvalues[i] += Math.sin(x) * amplitude[j];
            }
            else {
                yvalues[i] += Math.cos(x) * amplitude[j];
            }
            x += dx[j];
        }
    }
}

function renderWave() {
    // A simple way to draw the wave with an ellipse at each location
    noStroke();
    fill(255, 50);
    for (var x = 0; x < yvalues.length; x++) {
        circle(x * xspacing, SizeY() / 2 + yvalues[x], 16);
    }
}
