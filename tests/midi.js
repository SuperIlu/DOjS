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

/*
** This function is called once when the script is started.
*/
function Setup() {
    SetFramerate(30);

    m1 = new Midi("examples/d_intro.mid");
    m2 = new Midi("examples/d_victor.mid");
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
    ClearScreen(EGA.BLACK);
    TextXY(20, 120 + 0, "name = " + m1.filename, EGA.WHITE, NO_COLOR);
    TextXY(20, 120 + 10, "length = " + m1.length + "s", EGA.WHITE, NO_COLOR);
    TextXY(20, 120 + 20, "playing = " + MidiIsPlaying(), EGA.WHITE, NO_COLOR);
    TextXY(20, 120 + 30, "elapsed = " + MidiGetTime() + "s", EGA.WHITE, NO_COLOR);

    TextXY(SizeX() / 2, SizeY() / 2, "p=Play / s=Stop / a=Pause / r=Resume", EGA.WHITE, NO_COLOR);
}

/*
** This function is called on any input.
*/
function Input(e) {
    if (CompareKey(e.key, 'p')) {
        m1.Play();
    }

    if (CompareKey(e.key, 's')) {
        MidiStop();
    }

    if (CompareKey(e.key, 'a')) {
        MidiPause();
    }

    if (CompareKey(e.key, 'r')) {
        MidiResume();
    }
}
