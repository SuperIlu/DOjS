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

/*
** This function is called once when the script is started.
*/
function Setup() {
    SetFramerate(1);

    for (var i = 0; i < GetNumberOfFDD(); i++) {
        Println("\nFDD" + i + " has " + GetRawSectorSize(i) + " blocks");

        var fdd = RawRead(i, 0);
        var fdd_status = GetDiskStatus(i);
        Println("FDD " + i + ":" + fdd_status);
        hexDump(fdd);
    }

    for (var i = 0; i < GetNumberOfHDD(); i++) {
        Println("\nHDD" + i + " has " + GetRawSectorSize(RAW_HDD_FLAG + i) + " blocks");

        var hdd = RawRead(i + RAW_HDD_FLAG, 0);
        var hdd_status = GetDiskStatus(i + RAW_HDD_FLAG);
        Println("HDD " + i + ":" + hdd_status);
        hexDump(hdd);
    }

    // try writing
    var writeBuffer = []
    for (var i = 0; i < 512; i++) {
        writeBuffer.push(i % 256);
    }

    Println("FDD ");
    RawWrite(0, 0, writeBuffer);
    var fdd = RawRead(0, 0);
    hexDump(fdd);

    Println("HDD ");
    RawWrite(RAW_HDD_FLAG + 1, 0, writeBuffer);
    var hdd = RawRead(1 + RAW_HDD_FLAG, 0);
    hexDump(hdd);
}

function hexLength(val, len) {
    var curStr = val.toString(16);
    while (curStr.length < len) {
        curStr = "0" + curStr;
    }
    return curStr;
}

function hexDump(data) {
    for (var l = 0; l < 32; l++) {
        Print(hexLength(l * 16, 4) + ":");
        for (var c = 0; c < 16; c++) {
            var cur = data[l * 16 + c];
            Print(hexLength(cur, 2) + " ");
        }
        Print("|");
        for (var c = 0; c < 16; c++) {
            var cur = data[l * 16 + c];
            if (cur >= 32 && cur <= 126) {
                Print(String.fromCharCode(cur));
            } else {
                Print(".");
            }
        }
        Println("");
    }
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
    Stop();
}

/*
** This function is called on any input.
*/
function Input(event) {
}
