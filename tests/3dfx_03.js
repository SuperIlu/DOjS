Include('tests/3dfx_tst.js');
Println("draws gouraud shaded lines");

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.RGB]);
	createVertices();

	fxColorCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.ITERATED,
		GR_COMBINE_OTHER.NONE,
		false);

	v1 = [scaleX(0), scaleY(0), 0xFF, 0, 0];
	v2 = [scaleX(1), scaleY(1), 0, 0xFF, 0];
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(FX_BLACK, 0, 0);

	fxDrawLine(v1, v2);
}

function Input(e) { }
