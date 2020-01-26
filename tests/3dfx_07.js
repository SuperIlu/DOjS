Include('tests/3dfx_tst.js');
Println("alpha blending test");

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
	fxAlphaCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.CONSTANT,
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

	v1[0] = scaleX(0.25); v1[1] = scaleY(0.21);
	v2[0] = scaleX(0.75); v2[1] = scaleY(0.21);
	v3[0] = scaleX(0.50); v3[1] = scaleY(0.79);

	fxConstantColorValue(FX_RED);
	fxDrawTriangle(v1, v2, v3);

	v1[0] = scaleX(0.86); v1[1] = scaleY(0.21);
	v2[0] = scaleX(0.86); v2[1] = scaleY(0.79);
	v3[0] = scaleX(0.14); v3[1] = scaleY(0.5);

	fxConstantColorValue(FX_BLUE_TRANS);
	fxDrawTriangle(v1, v2, v3);
}

function Input(e) { }
