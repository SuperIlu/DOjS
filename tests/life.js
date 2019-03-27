/**
 * This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * http://creativecommons.org/licenses/by-nc-sa/4.0/
 * 
 * The original source can be found here:
 * https://p5js.org/examples/simulate-game-of-life.html
 * 
 * It was modified to run with DOjS by Andre Seidelt <superilu@yahoo.com>.
 */
Include('p5');

w = 20;
columns = null;
rows = null;
board = null;
next = null;

function setup() {
    // Calculate columns and rows
    columns = floor(width / w);
    rows = floor(height / w);
    // Wacky way to make a 2D array is JS
    board = new Array(columns);
    for (var i = 0; i < columns; i++) {
        board[i] = new Array(rows);
    }
    // Going to use multiple 2D arrays and swap them
    next = new Array(columns);
    for (i = 0; i < columns; i++) {
        next[i] = new Array(rows);
    }
    init();
}

function draw() {
    background(255);
    generate();
    for (var i = 0; i < columns; i++) {
        for (var j = 0; j < rows; j++) {
            if ((board[i][j] == 1)) {
                fill(0);
            } else {
                fill(255);
            }
            stroke(0);
            rect(i * w, j * w, w - 1, w - 1);
        }
    }
}

// reset board when mouse is pressed
function mousePressed() {
    init();
}

// Fill board randomly
function init() {
    for (var i = 0; i < columns; i++) {
        for (var j = 0; j < rows; j++) {
            // Lining the edges with 0s
            if (i == 0 || j == 0 || i == columns - 1 || j == rows - 1) {
                board[i][j] = 0;
                // Filling the rest randomly
            } else {
                board[i][j] = floor(random(2));
            }
            next[i][j] = 0;
        }
    }
}

// The process of creating the new generation
function generate() {
    // Loop through every spot in our 2D array and check spots neighbors
    for (var x = 1; x < columns - 1; x++) {
        for (var y = 1; y < rows - 1; y++) {
            // Add up all the states in a 3x3 surrounding grid
            var neighbors = 0;
            for (var i = -1; i <= 1; i++) {
                for (var j = -1; j <= 1; j++) {
                    neighbors += board[x + i][y + j];
                }
            }

            // A little trick to subtract the current cell's state since
            // we added it in the above loop
            neighbors -= board[x][y];
            // Rules of Life
            if ((board[x][y] == 1) && (neighbors < 2)) next[x][y] = 0;           // Loneliness
            else if ((board[x][y] == 1) && (neighbors > 3)) next[x][y] = 0;      // Overpopulation
            else if ((board[x][y] == 0) && (neighbors == 3)) next[x][y] = 1;     // Reproduction
            else next[x][y] = board[x][y]; // Stasis
        }
    }

    // Swap!
    var temp = board;
    board = next;
    next = temp;
}
