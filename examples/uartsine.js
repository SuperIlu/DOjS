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

Simple sketch that read floating point numbers (-1..1) from COM1 and displays
them on screen.
The matching Arduino-Sketch is as follows:

float angle;

void setup() {
  Serial.begin(2400);

  angle = 0;
}

void loop() {
  Serial.println(sin(angle));
  angle+=M_PI/180.0f;
}

*/
Include('p5');

/*
** This function is called once when the script is started.
*/
function setup() {
    SetFramerate(30);

    buf = "";
    values = [];
    height_2 = height / 2;
    com1 = new COMPort(COM.PORT.COM1, COM.BAUD.B2400, COM.BIT.BITS_8, COM.PARITY.NO_PARITY, COM.STOP.STOP_1, COM.FLOW.NO_CONTROL);
    Println("COM1 opened");

    stroke(255, 0, 0);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function draw() {
    // drain serial FIFO
    while (!com1.IsInputEmpty()) {
        var x = com1.ReadBuffer();
        buf += x
    }

    // try to get first line from buffer and convert to number in array
    while (true) {
        var idx = buf.indexOf('\n');
        if (idx > -1) {
            var first = buf.substring(0, idx);
            buf = buf.substring(idx + 1);
            values.push(float(first));
        } else {
            break;
        }
    }

    // reduce array length to screen width
    while (values.length > width) {
        values.shift();
    }

    // draw lines on screen
    if (values.length > 2) {
        background(0);
        var lastY = height_2 + values[0] * height_2;
        for (var i = 1; i < values.length; i++) {
            var curY = height_2 + values[i] * height_2;
            line(i - 1, lastY, i, curY);
            lastY = curY;
        }
    }
}

