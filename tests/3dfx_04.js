Include('tests/3dfx_tst.js');
Println("draws gouraud shaded triangle");

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

	v1 = [scaleX(0.3), scaleY(0.3), 0xff, 0, 0];
	v2 = [scaleX(0.8), scaleY(0.4), 0, 0xff, 0];
	v3 = [scaleX(0.5), scaleY(0.8), 0, 0, 0xff];
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(FX_BLACK, 0, 0);

	fxDrawTriangle(v1, v2, v3);
}

function Input(e) { }
