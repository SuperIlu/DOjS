if (!MOUSE_AVAILABLE) {
	print("Mouse not found");
	Quit(1);
}

MouseSetColors(EGA.RED, EGA.BLACK);
MouseShowCursor(true);
ClearScreen(EGA.DARK_GRAY);

var keep_running = true;
var draw = false;

while (keep_running) {
	event = MouseGetEvent(MOUSE.Flags.EVENT);

	if (event.key == KEY.Code.Key_Escape || event.key == KEY.Code.Key_Q || event.key == KEY.Code.Key_q) {
		Quit(0);
	}

	if (event.flags & MOUSE.Flags.LEFT_DOWN) {
		draw = true;
		Print("DOWN " + JSON.stringify(event));

	} else if (event.flags & MOUSE.Flags.LEFT_UP) {
		draw = false;
		Print("UP" + JSON.stringify(event));
	}

	if (draw) {
		Plot(event.x, event.y, EGA.WHITE);
	}
}
