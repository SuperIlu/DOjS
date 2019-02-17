scale = 5;

W = SizeX() / scale;
H = SizeY() / scale;

c = [EGA.BLACK, EGA.WHITE];
old = [];
cur = [];

function Setup() {
    for (var y = 0; y < H; y++) {
        old[y] = []
        cur[y] = [];
        for (var x = 0; x < W; x++) {
            cur[y][x] = old[y][x] = Math.random() <= 0.5 ? 1 : 0;
            FilledBox(x * scale, y * scale, x * scale + scale, y * scale + scale, c[old[y][x]]);
        }
    }
}

function Loop() {
    for (var y = 0; y < H; y++) {
        for (var x = 0; x < W; x++) {
            var cell = cur[y][x];
            // Calculate above/below/left/right row/column values
            var row_above = (y - 1 >= 0) ? y - 1 : H - 1; // If current cell is on first row, cell "above" is the last row (stitched)
            var row_below = (y + 1 <= H - 1) ? y + 1 : 0; // If current cell is in last row, then cell "below" is the first row
            var column_left = (x - 1 >= 0) ? x - 1 : W - 1; // If current cell is on first row, then left cell is the last row
            var column_right = (x + 1 <= W - 1) ? x + 1 : 0; // If current cell is on last row, then right cell is in the first row

            var alive_count =
                cur[row_above][column_left] +
                cur[row_above][x] +
                cur[row_above][column_right] +
                cur[y][column_left] +
                cur[y][column_right] +
                cur[row_below][column_left] +
                cur[row_below][x] +
                cur[row_below][column_right];

            var dead_count = 8 - alive_count;

            // Set new state to current state, but it may change below
            var new_state = cur[y][x];
            if (cur[y][x] == 1) {
                if (alive_count < 2 || alive_count > 3) {
                    // new state: dead, overpopulation/ underpopulation
                    new_state = 0;
                } else {
                    // lives on to next generation
                    new_state = 1;
                }
            } else {
                if (alive_count == 3) {
                    // new state: live, reproduction
                    new_state = 1;
                }
            }
            old[y][x] = new_state;
            FilledBox(x * scale, y * scale, x * scale + scale, y * scale + scale, c[new_state]);
        }
    }
    var tmp = old;
    old = cur;
    cur = tmp;

    if (KeyPressed()) {
        while (KeyPressed()) { KeyRead(); }
        Stop();
    }
}
