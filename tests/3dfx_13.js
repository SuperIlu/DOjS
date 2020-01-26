Include('tests/3dfx_tst.js');
Println("iterated alpha test");

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.A]);
	createVertices();

	fxColorCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.CONSTANT,
		GR_COMBINE_OTHER.NONE,
		false);
	fxAlphaCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.ITERATED,
		GR_COMBINE_OTHER.NONE,
		false);
	fxAlphaBlendFunction(
		GR_BLEND.SRC_ALPHA,
		GR_BLEND.ONE_MINUS_SRC_ALPHA,
		GR_BLEND.ZERO,
		GR_BLEND.ZERO);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(FX_BLACK, 0, 0);

	v1[0] = scaleX(0.5); v1[1] = scaleY(0.1);
	v2[0] = scaleX(0.8); v2[1] = scaleY(0.9);
	v3[0] = scaleX(0.2); v3[1] = scaleY(0.9);
	v1[2] = v2[2] = v3[2] = 0xFF;

	fxConstantColorValue(FX_RED);
	fxDrawTriangle(v1, v2, v3);

	fxOrigin(GR_ORIGIN.LOWER_LEFT);

	v1[2] = 0x00;
	fxConstantColorValue(FX_BLUE);
	fxDrawTriangle(v1, v2, v3);

	fxOrigin(GR_ORIGIN.UPPER_LEFT);
}

function Input(e) { }
