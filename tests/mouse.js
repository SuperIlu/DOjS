function Setup() {
    if (!MOUSE_AVAILABLE) {
        print("Mouse not found");
        Quit(1);
    }

    MouseSetColors(EGA.RED, EGA.BLACK);
    MouseShowCursor(true);
    ClearScreen(EGA.DARK_GRAY);

    draw = false;
    Gc(true);
    SetFramerate(50);

    mouseX = 0;
    mouseY = 0;
}

function Loop() {
    if (draw) {
        Plot(mouseX, mouseY, EGA.WHITE);
    }

    TextXY(10, 10, "rate=" + GetFramerate(), EGA.BLACK, EGA.LIGHT_BLUE);
}

function Input(event) {
    if (event.flags & MOUSE.Flags.LEFT_DOWN) {
        draw = true;

    } else if (event.flags & MOUSE.Flags.LEFT_UP) {
        draw = false;
    }
    mouseX = event.x;
    mouseY = event.y;
}
