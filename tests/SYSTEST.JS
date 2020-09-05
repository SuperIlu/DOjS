/*
** This function is called once when the script is started.
*/
function Setup() {
    SetFramerate(30);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
    ClearScreen(EGA.BLACK);
    System("dir >dtmp.txt");
    var dout = Read("dtmp.txt");
    TextXY(10, 10, dout, EGA.WHITE);
}

/*
** This function is called on any input.
*/
function Input(event) {
}
