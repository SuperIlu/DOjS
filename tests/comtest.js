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

/*
** This function is called once when the script is started.
*/
function Setup() {
    SetFramerate(1);

    Println(JSON.stringify(COM));

    dump_obj(COM.PORT);
    dump_obj(COM.BAUD);
    dump_obj(COM.BIT);
    dump_obj(COM.PARITY);
    dump_obj(COM.STOP);
    dump_obj(COM.FLOW);

    comX = new COMPort(COM.PORT.COM1, COM.BAUD.B115200, COM.BIT.BITS_8, COM.PARITY.NO_PARITY, COM.STOP.STOP_1, COM.FLOW.NO_CONTROL);
    com2 = new COMPort(COM.PORT.COM2, COM.BAUD.B115200, COM.BIT.BITS_8, COM.PARITY.NO_PARITY, COM.STOP.STOP_1, COM.FLOW.NO_CONTROL);
    Println("Ports opened");

    comX.Close();
    Println("COM1 closed");

    com1 = new COMPort(COM.PORT.COM1, COM.BAUD.B115200, COM.BIT.BITS_8, COM.PARITY.NO_PARITY, COM.STOP.STOP_1, COM.FLOW.NO_CONTROL);
    Println("COM1 re-opened");

    com3 = new COMPort(COM.PORT.COM3, COM.BAUD.B9600, COM.BIT.BITS_8, COM.PARITY.NO_PARITY, COM.STOP.STOP_1, COM.FLOW.NO_CONTROL);
    Println("COM3 opened");

    trans = false;
    
    LPTReset(0);
}

function dump_obj(o) {
    for (var key in o) {
        Print(key);
        Print(", ");
    }
    Println("");
}


/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
    TransparencyEnabled(trans);

    trans = !trans;

    ClearScreen(EGA.BLACK);
    FilledCircle(SizeX() / 2, SizeY() / 2, 100, Color(255, 0, 0, 255));
    FilledBox(SizeX() / 2, SizeY() / 2, SizeX() / 2 + 100, SizeY() / 2 + 100, Color(0, 255, 0, 128));

    Println("Wrote=" + com1.WriteString("" + trans));

    Println("Wrote=" + com3.WriteString("?\n\r"));
    while (!com3.IsInputEmpty()) {
        Print(com3.ReadByte());
    }

    Println("Wrote=" + com3.WriteString("m\n\r"));
    while (!com3.IsInputEmpty()) {
        Print(com3.ReadByte());
    }

    LPTSend(0, "TX="+trans+"\r\n");
    Println("LPTStatus=" + LPTStatus(0));
}

/*
** This function is called on any input.
*/
function Input(event) {
}
