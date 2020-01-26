Include('tests/3dfx_tst.js');
Println("draws a diagonal line of points from top-left to bottom-right");

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY]);
	createVertices();

	fxColorCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.CONSTANT,
		GR_COMBINE_OTHER.NONE,
		false);
	fxConstantColorValue(FX_WHITE);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(FX_BLACK, 0, 0);

	for (var i = 0; i < 100; i++) {
		var pos = i / 100;

		v1[0] = scaleX(pos);
		v1[1] = scaleY(pos);

		fxDrawPoint(v1);
	}
}

function Input(e) { }
