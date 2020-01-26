Include('tests/3dfx_tst.js');
Println("texturing example, decal, rgb lit, white lit, flat lit");

var zrange;

var miro;
var textureMode = 0;

var DECAL = 0, FLATLIT = 1, RGBLIT = 2, WHITELIT = 3, SPECALPHA = 4;

/*
** This function is called once when the script is started.
*/
function Setup() {
	fxInit();
	fxVertexLayout([GR_PARAM.XY]);
	createVertices();

	fxTexCombine(
		GR_TMU.TMU0,
		GR_COMBINE_FUNCTION.LOCAL,
		GR_COMBINE_FACTOR.NONE,
		GR_COMBINE_FUNCTION.NONE,
		GR_COMBINE_FACTOR.NONE,
		false, false);

	/* Load texture data into system ram */
	FxTexMemInit(GR_TMU.TMU0);
	miro = new TexInfo("tests/miro.3df");
	/* Download texture data to TMU */
	miro.DownloadMipMap(GR_TMU.TMU0, FxTexMemGetStartAddress(GR_TMU.TMU0, miro), GR_MIPMAPLEVELMASK.BOTH);
	/* Select Texture As Source of all texturing operations */
	miro.Source(GR_MIPMAPLEVELMASK.BOTH);

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

	switch (textureMode) {
		case DECAL:
			fxResetVertexLayout();
			fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q, GR_PARAM.ST0]);
			createVertices();
			fxColorCombine(
				GR_COMBINE_FUNCTION.SCALE_OTHER,
				GR_COMBINE_FACTOR.ONE,
				GR_COMBINE_LOCAL.NONE,
				GR_COMBINE_OTHER.TEXTURE,
				false);
			break;
		case FLATLIT:
			fxResetVertexLayout();
			fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q, GR_PARAM.ST0]);
			createVertices();
			fxColorCombine(
				GR_COMBINE_FUNCTION.SCALE_OTHER,
				GR_COMBINE_FACTOR.LOCAL,
				GR_COMBINE_LOCAL.CONSTANT,
				GR_COMBINE_OTHER.TEXTURE,
				false);
			break;
		case RGBLIT:
			fxResetVertexLayout();
			fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q, GR_PARAM.ST0, GR_PARAM.RGB]);
			createVertices();
			fxColorCombine(
				GR_COMBINE_FUNCTION.SCALE_OTHER,
				GR_COMBINE_FACTOR.LOCAL,
				GR_COMBINE_LOCAL.ITERATED,
				GR_COMBINE_OTHER.TEXTURE,
				false);
			FxRGB2Vertex(v1, 5, 0xFF0000);
			FxRGB2Vertex(v2, 5, 0x00FF00);
			FxRGB2Vertex(v3, 5, 0x0000FF);
			FxRGB2Vertex(v4, 5, 0x000000);
			break;
		case WHITELIT:
			fxResetVertexLayout();
			fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q, GR_PARAM.ST0, GR_PARAM.A]);
			createVertices();
			fxColorCombine(
				GR_COMBINE_FUNCTION.SCALE_OTHER,
				GR_COMBINE_FACTOR.LOCAL_ALPHA,
				GR_COMBINE_LOCAL.NONE,
				GR_COMBINE_OTHER.TEXTURE,
				false);
			fxAlphaCombine(
				GR_COMBINE_FUNCTION.LOCAL,
				GR_COMBINE_FACTOR.NONE,
				GR_COMBINE_LOCAL.ITERATED,
				GR_COMBINE_OTHER.TEXTURE,
				false);
			v1[5] = 0xFF;
			v2[5] = 0x80;
			v3[5] = 0x80;
			v4[5] = 0x00;
			break;
		case SPECALPHA:
			fxResetVertexLayout();
			fxVertexLayout([GR_PARAM.XY, GR_PARAM.Q, GR_PARAM.ST0, GR_PARAM.A, GR_PARAM.RGB]);
			createVertices();
			fxColorCombine(
				GR_COMBINE_FUNCTION.SCALE_OTHER_ADD_LOCAL_ALPHA,
				GR_COMBINE_FACTOR.LOCAL,
				GR_COMBINE_LOCAL.ITERATED,
				GR_COMBINE_OTHER.TEXTURE,
				false);
			fxAlphaCombine(
				GR_COMBINE_FUNCTION.LOCAL,
				GR_COMBINE_FACTOR.NONE,
				GR_COMBINE_LOCAL.ITERATED,
				GR_COMBINE_OTHER.TEXTURE,
				false);
			v1[5] = 0xFF;
			v2[5] = 0x80;
			v3[5] = 0x80;
			v4[5] = 0x00;
			FxRGB2Vertex(v1, 6, 0xFF0000);
			FxRGB2Vertex(v2, 6, 0x00FF00);
			FxRGB2Vertex(v3, 6, 0x0000FF);
			FxRGB2Vertex(v4, 6, 0x000000);
			break;
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

	fxDrawTriangle(v1, v4, v3);
	fxDrawTriangle(v1, v2, v4);

	fxConstantColorValue(FX_BLUE);
}

function Input(e) {
	if (CompareKey(e.key, 'm')) {
		textureMode++;
		if (textureMode > SPECALPHA) { textureMode = DECAL; }
	}
}
