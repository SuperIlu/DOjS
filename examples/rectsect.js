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

var rects, sects;

function setup() {
    createCanvas(windowWidth, windowHeight);
    colorMode(HSB, 255);
    rectMode(CORNERS);
    frameRate(10);

    rects = [];
    sects = [];
}

function draw() {
    background(60);

    // rectangles
    while (rects.length < 10 && frameCount % 3 == 0) {
        rects.push(createNormalizedRect());
    }

    for (var i = 0; i < rects.length; i++) {
        drawRect(rects[i]);
        rects[i].col = changeAlpha(rects[i].col);
    }
    rects = rects.filter(function (e) {
        return alpha(e.col) > 32;
    });

    // intersections
    if (rects.length > 3 && sects.length < 20) {
        var r1 = rects[int(random(rects.length))];
        var r2 = rects[int(random(rects.length))];

        if (r1 != r2) {
            var r1p = getPoints(r1);
            var r2p = getPoints(r2);
            var col = lerpColor(r1.col, r2.col, 0.5);

            checkSect(sects, col, intersect(r1p[0], r1p[1], r2p[0], r2p[3])); // UL.1 w/ LL.2
            checkSect(sects, col, intersect(r1p[0], r1p[1], r2p[1], r2p[2])); // UL.1 w/ LR.2
            checkSect(sects, col, intersect(r1p[3], r1p[2], r2p[0], r2p[3])); // LL.1 w/ LL.2
            checkSect(sects, col, intersect(r1p[3], r1p[2], r2p[1], r2p[2])); // LL.1 w/ LR.2
            checkSect(sects, col, intersect(r2p[0], r2p[1], r1p[0], r1p[3]));
            checkSect(sects, col, intersect(r2p[0], r2p[1], r1p[1], r1p[2]));
            checkSect(sects, col, intersect(r2p[3], r2p[2], r1p[0], r1p[3]));
            checkSect(sects, col, intersect(r2p[3], r2p[2], r1p[1], r1p[2]));
        }
    }

    for (var i = 0; i < sects.length; i++) {
        drawPoint(sects[i]);
        sects[i].size -= 3;
    }
    sects = sects.filter(function (e) {
        return e.size > 0;
    });
}

function changeAlpha(c) {
    return color(hue(c), saturation(c), brightness(c), alpha(c) - random(20));
}

function checkSect(l, c, p) {
    if (p) {
        p.col = c;
        l.push(p);
    }
}

function mouseClicked() {
    r1 = createNormalizedRect();
    r2 = createNormalizedRect();
}

function createNormalizedRect() {
    var ret = {
        s: {
            x: random(width),
            y: random(height)
        },
        e: {
            x: random(width),
            y: random(height)
        },
        col: color(random(255), 255, 255, 255)
    };

    if (ret.e.x < ret.s.x) {
        var tmp = ret.e.x;
        ret.e.x = ret.s.x;
        ret.s.x = tmp;
    }

    if (ret.e.y < ret.s.y) {
        var tmp = ret.e.y;
        ret.e.y = ret.s.y;
        ret.s.y = tmp;
    }

    return ret;
}

function getPoints(r) {
    return [{
        x: r.s.x, // UL
        y: r.s.y
    },
    {
        x: r.e.x, // UR
        y: r.s.y
    },
    {
        x: r.e.x, // LR
        y: r.e.y
    },
    {
        x: r.s.x, // LL
        y: r.e.y
    }
    ];
}

// line intercept math by Paul Bourke http://paulbourke.net/geometry/pointlineplane/
// Determine the intersection point of two line segments
// Return FALSE if the lines don't intersect
function intersect(s1, e1, s2, e2) {

    // Check if none of the lines are of length 0
    if ((s1.x === e1.x && s1.y === e1.y) || (s2.x === e2.x && s2.y === e2.y)) {
        return null;
    }

    denominator = ((e2.y - s2.y) * (e1.x - s1.x) - (e2.x - s2.x) * (e1.y - s1.y));

    // Lines are parallel
    if (denominator === 0) {
        return null;
    }

    var ua = ((e2.x - s2.x) * (s1.y - s2.y) - (e2.y - s2.y) * (s1.x - s2.x)) / denominator;
    var ub = ((e1.x - s1.x) * (s1.y - s2.y) - (e1.y - s1.y) * (s1.x - s2.x)) / denominator;

    // is the intersection along the segments
    if (ua < 0 || ua > 1 || ub < 0 || ub > 1) {
        return null;
    }

    // Return a object with the x and y coordinates of the intersection
    var x = s1.x + ua * (e1.x - s1.x);
    var y = s1.y + ua * (e1.y - s1.y);

    return {
        x: x,
        y: y,
        size: 16
    };
}

function drawRect(q) {
    noFill();
    strokeWeight(1);
    stroke(q.col);
    rect(q.s.x, q.s.y, q.e.x, q.e.y);
}

function drawPoint(p) {
    if (p) {
        noStroke();
        fill(p.col);
        circle(p.x, p.y, p.size);
    }
}
