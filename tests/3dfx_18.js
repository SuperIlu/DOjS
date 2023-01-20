Include('tests/3dfx_tst.js');
Println("alpha texture test");

var zrange;

var texture;

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q, GR_PARAM.ST0]);
	createVertices();

	fxTexCombine(
		GR_TMU.TMU0,
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		false, false);

	/* Load texture data into system ram */
	FxTexMemInit(GR_TMU.TMU0);
	texture = new TexInfo("tests/testdata/alpha.3df");
	/* Download texture data to TMU */
	texture.DownloadMipMap(GR_TMU.TMU0, FxTexMemGetStartAddress(GR_TMU.TMU0, texture), GR_MIPMAPLEVELMASK.BOTH);
	/* Select Texture As Source of all texturing operations */
	texture.Source(GR_MIPMAPLEVELMASK.BOTH);

	/* Enable Bilinear Filtering + Mipmapping */
	fxTexFilterMode(GR_TMU.TMU0, GR_TEXTUREFILTER.BILINEAR, GR_TEXTUREFILTER.BILINEAR);
	fxTexMipMapMode(GR_TMU.TMU0, GR_MIPMAP.NEAREST, false);

	zrange = fxGetZDepthMinMax();
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	fxBufferClear(FX_BLACK, 0, zrange[1]);

	/* Draw 10x10 grid of triangles */
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
		GR_BLEND.ONE,
		GR_BLEND.ZERO,
		GR_BLEND.ZERO,
		GR_BLEND.ZERO);
	for (var y = 0; y < 10; y++) {
		for (var x = 0; x < 10; x++) {
			/* 
			   A-D
			   |\|
			   B-C
			 */
			v1[0] = v2[0] = scaleX(x / 10.0);
			v1[1] = v4[1] = scaleY(y / 10.0);
			v2[1] = v3[1] = scaleY((y / 10.0) + 0.1);
			v3[0] = v4[0] = scaleX((x / 10.0) + 0.1);

			fxConstantColorValue(FX_RED);
			fxDrawTriangle(v1, v2, v3);

			fxConstantColorValue(FX_BLUE);
			fxDrawTriangle(v1, v3, v4);
		}
	}

	/*---- 
	A-B
	|\|
	C-D
	-----*/
	v1[2] = 1.0;
	v2[2] = 1.0;
	v3[2] = 1.0;
	v4[2] = 1.0;

	v1[0] = v3[0] = scaleX(0.2);
	v2[0] = v4[0] = scaleX(0.8);
	v1[1] = v2[1] = scaleY(0.2);
	v3[1] = v4[1] = scaleY(0.8);

	v1[3] = v3[3] = 0.0;
	v2[3] = v4[3] = 255.0;
	v1[4] = v2[4] = 0.0;
	v3[4] = v4[4] = 255.0;

	fxColorCombine(
		GR_COMBINE_FUNCTION.SCALE_OTHER,
		GR_COMBINE_FACTOR.ONE,
		GR_COMBINE_LOCAL.NONE,
		GR_COMBINE_OTHER.TEXTURE,
		false);
	fxAlphaCombine(
		GR_COMBINE_FUNCTION.SCALE_OTHER,
		GR_COMBINE_FACTOR.ONE,
		GR_COMBINE_LOCAL.NONE,
		GR_COMBINE_OTHER.TEXTURE,
		false);
	fxAlphaBlendFunction(
		GR_BLEND.SRC_ALPHA,
		GR_BLEND.ONE_MINUS_SRC_ALPHA,
		GR_BLEND.ZERO,
		GR_BLEND.ZERO);

	fxDrawTriangle(v1, v4, v3);
	fxDrawTriangle(v1, v2, v4);

}

function Input(e) { }
