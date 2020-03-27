/*
** This function is called once when the script is started.
*/
function Setup() {
    SetFramerate(30);

    VDebug([[1, 2, 3, 4, 5, EGA.RED], [11, 12, 13, 14, 15, EGA.GREEN]]);
    Print(JSON.stringify(GetIdentityMatrix()));
    Print(JSON.stringify(GetTranslationMatrix(1, 2, 3)));
    Print(JSON.stringify(GetScalingMatrix(1, 2, 3)));
    Print(JSON.stringify(GetXRotatateMatrix(10)));
    Print(JSON.stringify(GetYRotatateMatrix(20)));
    Print(JSON.stringify(GetZRotatateMatrix(30)));
    Print(JSON.stringify(GetRotateMatrix(10, 20, 30)));
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
    ClearScreen(EGA.BLACK);
    TextXY(SizeX() / 2, SizeY() / 2, "Hello World!", EGA.WHITE, NO_COLOR);
}

/*
** This function is called on any input.
*/
function Input(event) {
}
