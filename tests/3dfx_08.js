Include('tests/3dfx_tst.js');
Println("fogging");

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.RGB, GR_PARAM.Q]);
	createVertices();

	fxColorCombine(
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_LOCAL.ITERATED,
		GR_COMBINE_OTHER.NONE,
		false);
	fxFogMode(GR_FOG.WITH_TABLE_ON_W);
	fxFogColorValue(FX_GREEN);
	var fogtable = fxFogGenerateExp(0.01);
	fxFogTable(fogtable);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(FX_BLACK, 0, 0);

	v1[0] = scaleX(0.000); v1[1] = scaleY(0.0000);
	FxRGB2Vertex(v1, 2, 0x000040);
	var wDist = 20.0;
	v1[5] = (1.0 / wDist);

	v2[0] = scaleX(0.016); v2[1] = scaleY(1.0000);
	FxRGB2Vertex(v2, 2, 0x000080);
	wDist = 2000.0;
	v2[5] = (1.0 / wDist);

	v3[0] = scaleX(1.000); v3[1] = scaleY(0.0208);
	FxRGB2Vertex(v3, 2, 0x000040);
	wDist = 20.0;
	v3[5] = (1.0 / wDist);

	fxDrawTriangle(v1, v2, v3);
}

function Input(e) { }
