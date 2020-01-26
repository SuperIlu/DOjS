var next = false;

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
    var joy = JoystickPoll(0);
    if (joy.calibrate) {
        TextXY(SizeX() / 2, SizeY() / 2, JoystickCalibrateName(0), EGA.WHITE);
        if (next) {
            JoystickCalibrate(0);
        }
    }
}

/*
** This function is called on any input.
*/
function Input(e) {
    if (CompareKey(e.key, ' ')) {
        next = true;
    } else {
        next = false;
    }
}
