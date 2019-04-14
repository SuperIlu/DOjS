nodes = [];
function Setup() {
	SetFramerate(10);
	IpxSocketOpen(IPX.DEFAULT_SOCKET);
	MouseShowCursor(false);
}

function Loop() {
	ClearScreen(EGA.BLACK);

	TextXY(10, 10, 'Discovered nodes:' + nodes.length, EGA.WHITE, NO_COLOR);
	for (var l = 0; l < nodes.length; l++) {
		TextXY(10, 10 * (l + 2), l + " = " + IpxAddressToString(nodes[l]), EGA.WHITE, NO_COLOR);
	}
	var success = IpxFindNodes(3, nodes);
	if (success) {
		TextXY(SizeX() / 2, SizeY() / 2, "DONE", EGA.RED, NO_COLOR);
	}
}

function Input(e) {
}

