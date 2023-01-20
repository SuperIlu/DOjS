Include('tests/3dfx_tst.js');
Println("Simple palette texture test");

// TODO: unfinished

var wrange;

var texture, light, detail;

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxOrigin(GR_ORIGIN.LOWER_LEFT);
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q, GR_PARAM.ST0]);
	createVertices();

	fxColorCombine(
		GR_COMBINE_FUNCTION.SCALE_OTHER,
		GR_COMBINE_FACTOR.ONE,
		GR_COMBINE_LOCAL.CONSTANT,
		GR_COMBINE_OTHER.TEXTURE,
		false);
	fxTexCombine(
		GR_TMU.TMU0,
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.ONE,
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.ONE,
		false,
		false);

	/* Load texture data into system ram */
	FxTexMemInit(GR_TMU.TMU0);
	FxTexMemInit(GR_TMU.TMU1);
	texture = new TexInfo("tests/testdata/decal1.3df");
	/* Download texture data to TMU */
	texture.DownloadMipMap(GR_TMU.TMU1, FxTexMemGetStartAddress(GR_TMU.TMU1, texture), GR_MIPMAPLEVELMASK.BOTH);
	texture.Source(GR_MIPMAPLEVELMASK.BOTH);

	fxTexCombine(
		GR_TMU.TMU1,
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.ONE,
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.ONE,
		false,
		false);
	fxTexCombine(
		GR_TMU.TMU0,
		GR_COMBINE_FUNCTION.BLEND_OTHER,
		GR_COMBINE_FACTOR.LOCAL,
		GR_COMBINE_FUNCTION.BLEND_OTHER,
		GR_COMBINE_FACTOR.LOCAL,
		false,
		false);


	wrange = fxGetWDepthMinMax();
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(0x00404040, 0, zrange[1]);

}

function Input(e) {
}
